#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>

#include "json-c/json.h"
#include "centering_libs.h"
#include "splice_libs.h"
#include "actuator.h"
#include "sys_trace.h"

#define BUF_SIZE 4096

#define CENTERING_PORT 7076
#define JSON_STRING_SIZE 256

#define MAX_TIP_DIRECTION_STRING 10
#define FILE_TIP_DIRECTION "/data/tip_direction"
#define FILE_WHOLE_PROFILE "/mnt/extsd/whole_area_profile"
#define FILE_LEADING_PROFILE "/mnt/extsd/leading_area_profile"
//#define VIEW_POS

static int centering_fd;
static int isCentering;
static int offsetCoeff;
static int CPCRatio = DEFAULT_CPC_RATIO;
static volatile int cnt_enc;
static act_status ACT;
float rWidth[4];

static struct leadingProfile lp[MAX_LEADING_PROFILE];
static struct wholeProfile wp[MAX_WHOLE_PROFILE];
static int leadingProfileCnt;
static int wholeProfileCnt;
static int profileOnOff=1;

int getIsCentering(void)
{
	return isCentering;
}

void setIsCentering(char status)
{
	isCentering = status;
}

int setOffsetCoeff(int coeff)
{
	if(coeff > 100 || coeff < -100)
	{
		PrintError("Seems look like abnormal coeff value %d\n", coeff);
		return -1;
	}

	offsetCoeff = coeff;

	return 0;
}

int setCPCRatio(int ratio)
{
	if(ratio > 100 || ratio < 80)
	{
		PrintError("Seems look like abnormal ratio value %d\n", ratio);
		return -1;
	}

	CPCRatio = ratio;

	return 0;
}

int getCPCRatio(void)
{
	return CPCRatio;
}

int startProfile(char onOff)
{
	profileOnOff = onOff;

	return 0;
}

int isProfileOn(void)
{
	return profileOnOff;
}

void resetProfile(void)
{
	memset(lp, 0, sizeof(struct leadingProfile) * MAX_LEADING_PROFILE);
	memset(wp, 0, sizeof(struct wholeProfile) * MAX_WHOLE_PROFILE);
	leadingProfileCnt = 0;
	wholeProfileCnt = 0;
}

void saveProfile(void)
{
	int fd_wp, fd_lp;

	if(isProfileOn() == FALSE) return;

	fd_wp = open(FILE_WHOLE_PROFILE, O_CREAT|O_RDWR);
	if(fd_wp < 0)
	{
		perror("open whole profile error\n");
		return;
	}

	fd_lp = open(FILE_LEADING_PROFILE, O_CREAT|O_RDWR);
	if(fd_lp < 0)
	{
		perror("open leading profile error\n");
		return;
	}

	write(fd_wp, (void *)&wholeProfileCnt, sizeof(int));
	write(fd_wp, (void *)wp, sizeof(struct wholeProfile) * wholeProfileCnt);
	write(fd_lp, (void *)&leadingProfileCnt, sizeof(int));
	write(fd_lp, (void *)lp, sizeof(struct leadingProfile) * leadingProfileCnt);

	close(fd_wp);
	close(fd_lp);
}

void viewProfile(char area)
{
	int fd_wp, fd_lp, cnt, i;
	float width = 0, prev_enc_cnt = 0, a_tangent;
	struct leadingProfile p;
	RRegister R;

	if(isProfileOn() == FALSE)
	{
		printf("No profiling data to show\n");
		return;
	}

	fd_wp = open(FILE_WHOLE_PROFILE, O_RDONLY);
	if(fd_wp < 0)
	{
		perror("open whole profile error\n");
		return;
	}

	fd_lp = open(FILE_LEADING_PROFILE, O_RDONLY);
	if(fd_lp < 0)
	{
		perror("open leading profile error\n");
		return;
	}

	readRRegister(FALSE, &R);

	if(area == PROFILE_AREA_LEADING_DIVIDED && getAlgorithm() > ALGORITHM1)
	{
		read(fd_lp, (void *)&cnt, sizeof(int));
		printf("total cnt=%d\n", cnt);
		for(i=0; i<cnt; i++)
		{
			read(fd_lp, (void *)&p, sizeof(struct leadingProfile));
			if(width != p.RWidth)
			{
				a_tangent = atanf((p.RWidth - width) / ((p.enc_cnt - prev_enc_cnt) * R.mm_per_pulse));
				printf("Area %d : %f : %d : %f : %f\n", p.area, p.RWidth, p.enc_cnt, a_tangent, a_tangent * (180/3.14));
				width = p.RWidth;
				prev_enc_cnt = p.enc_cnt;
			}
		}
	}

	close(fd_wp);
	close(fd_lp);
}

void wholeAreaProfile(char section, float RWidth, float *rWidth)
{
	if(section && wholeProfileCnt < MAX_WHOLE_PROFILE)
	{
		wp[wholeProfileCnt].rWidth[LPos02] = rWidth[LPos02];
		wp[wholeProfileCnt].rWidth[LPos01] = rWidth[LPos01];
		wp[wholeProfileCnt].rWidth[RPos01] = rWidth[RPos01];
		wp[wholeProfileCnt].rWidth[RPos02] = rWidth[RPos02];
		wp[wholeProfileCnt].RWidth = RWidth;
		wp[wholeProfileCnt].current_section = section;
		wp[wholeProfileCnt].encoder = getEncoderCnt();
		wholeProfileCnt++;
	}
}

void leadingOffsetProfile(float RWidth, float *leading_tip_width)
{
	if(leadingProfileCnt > MAX_LEADING_PROFILE) return;

	if(leading_tip_width[0] <= RWidth && RWidth < leading_tip_width[1])
	{
		lp[leadingProfileCnt].area = 1;
	}
	else if(leading_tip_width[1] <= RWidth  && RWidth < leading_tip_width[2])
	{
		lp[leadingProfileCnt].area = 2;
	}
	else if(leading_tip_width[2] <= RWidth && RWidth < leading_tip_width[3])
	{
		lp[leadingProfileCnt].area = 3;
	}
	else if(leading_tip_width[3] <= RWidth && RWidth < leading_tip_width[4])
	{
		lp[leadingProfileCnt].area = 4;
	}
	else if(leading_tip_width[4] <= RWidth && RWidth < leading_tip_width[5])
	{
		lp[leadingProfileCnt].area = 5;
	}
	else if(leading_tip_width[5] <= RWidth && RWidth < leading_tip_width[6])
	{
		lp[leadingProfileCnt].area = 6;
	}

	lp[leadingProfileCnt].RWidth = RWidth;
	lp[leadingProfileCnt].enc_cnt = getEncoderCnt();
	leadingProfileCnt++;
}

int getAlgorithm(void)
{
	int i;
	RRegister R;

	readRRegister(FALSE, &R);

	if(R.LeadingOffsetEnable)
	{
		for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
		{
			if(R.LeadingOffset[i]) return ALGORITHM2;
		}
		return ALGORITHM3;
	}
	
	return ALGORITHM1;
}

void enableReadPos(char onOff)
{
	char setStream[JSON_STRING_SIZE];

	memset(setStream, 0, JSON_STRING_SIZE);
	if(onOff)
	{
		strncpy(setStream, "{\"setStreaming\":\"on\"}", JSON_STRING_SIZE);
	}
	else
	{
		strncpy(setStream, "{\"setStreaming\":\"off\"}", JSON_STRING_SIZE);
	}

	send(centering_fd, setStream, JSON_STRING_SIZE, 0);
}

void calibrationSetMode(unsigned char mode)
{
	char setCaliMode[JSON_STRING_SIZE];

	memset(setCaliMode, 0, JSON_STRING_SIZE);
	if(mode == MODE_CALIBRATION)
	{
		strncpy(setCaliMode, "{\"setMode\":\"calibration\"}", JSON_STRING_SIZE);
	}
	else if(mode == MODE_RUNNING)
	{
		strncpy(setCaliMode, "{\"setMode\":\"running\"}", JSON_STRING_SIZE);
	}
	else
	{
		PrintError("Invalid calibration mdoe %d\n", mode);
	}

	send(centering_fd, setCaliMode, JSON_STRING_SIZE, 0);
}

int calibrationSetCam(unsigned char cam)
{
	char setCam[JSON_STRING_SIZE];

	if(cam > CAMALL)
	{
		PrintError("Invalid camera number %d\n", cam);
		return -1;
	}

	memset(setCam, 0, JSON_STRING_SIZE);
	if(cam == CAMALL)
	{
		snprintf(setCam, JSON_STRING_SIZE, "{\"setCamera\":\"all\"}");
	}
	else
	{
		snprintf(setCam, JSON_STRING_SIZE, "{\"setCamera\":\"cam%d\"}", cam);
	}

	send(centering_fd, setCam, JSON_STRING_SIZE, 0);

	return 0;
}

void calibrationSave(void)
{
	char setCaliSave[JSON_STRING_SIZE];

	memset(setCaliSave, 0, JSON_STRING_SIZE);
	strncpy(setCaliSave, "{\"saveCalibrationData\":null}", JSON_STRING_SIZE);

	send(centering_fd, setCaliSave, JSON_STRING_SIZE, 0);
}

char getTipDirection(void)
{
	int fd, td, cnt;
	char buf[MAX_TIP_DIRECTION_STRING];

	fd = open(FILE_TIP_DIRECTION, O_RDONLY);
	if(fd < 0)
	{
		perror("failed to open tip direction file\n");
		return TIP_LEFT;
	}

	cnt = read(fd, (void *)buf, MAX_TIP_DIRECTION_STRING);
	buf[cnt - 1] = 0;

	if(strncmp("left", buf, MAX_TIP_DIRECTION_STRING) == 0)
	{
		PrintDebug("Tip direction seems look like normal(%s)\n", buf);
		td = TIP_LEFT;
	}
	else if(strncmp("right", buf, MAX_TIP_DIRECTION_STRING) == 0)
	{
		PrintDebug("Tip direction seems look like normal(%s)\n", buf);
		td = TIP_RIGHT;
	}
	else
	{
		PrintError("Tip direction seems look like abnormal(%s)\n", buf);
		td = TIP_LEFT;
	}

	close(fd);
	return td;
}

int getEncoderCnt(void)
{
	return cnt_enc;
}

void resetEncoder(void)
{
	cnt_enc = 0;
}

void act_move(char direction, int pos)
{
	act_position act_p;
	int actLLimit = ACT.act_l_limit * 1000;
	int actRLimit = ACT.act_r_limit * 1000;

	if(pos == 0) return;

	if(direction == ACT_MOVE_LEFT)
	{
		if(pos  > actLLimit)
		{
			act_p.act_cur_position_msb = 0x0;
			act_p.act_cur_position_lsb = ACT.act_l_limit;
			actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
		}
		else if(pos < actLLimit)
		{
			act_p.act_cur_position_msb = 0x0;
			act_p.act_cur_position_msb |= (pos >> 8) & 0x0F;
			act_p.act_cur_position_lsb = pos & 0xFF;
			actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
		}
	}
	else if(direction == ACT_MOVE_RIGHT)
	{
		if(pos > actRLimit)
		{
			act_p.act_cur_position_msb = 0x80;
			act_p.act_cur_position_lsb = ACT.act_r_limit;
			actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
		}
		else if(pos < actRLimit)
		{
			act_p.act_cur_position_msb = 0x80;
			act_p.act_cur_position_msb |= (pos >> 8) & 0x0F;
			act_p.act_cur_position_lsb = pos & 0xFF;
			actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
		}
	}
	else
	{
		PrintError("actuator set wrong direction\n");
	}
}

int width_check(float width, RRegister *r)
{
	if(r->TolPos == 0 && r->TolNeg == 0)
	{
		return 0;
	}

	if(width > r->TolPos || width < r->TolNeg)
	{
		return 1;
	}

	return 0;
}

void readRRegister(char dump, RRegister *R)
{
	char plc_buf[108];
	int i, j;

	/* Though R->register size is 54byte, plc_buf should bigger twice than EEP_R_Register_Size.
	 * This is for PLC internal implementation */
	if(sendPlc(EEP_Data_Start_R_Register, plc_buf, EEP_R_Register_Size, TRUE) != PLC_COMM_ACK)
	{
		PrintError("failed to get R->register from PLC\n");
		return;
	}

	PLC_BIN_to_LONG(&plc_buf[0], &(R->MWidth));
	PLC_BIN_to_LONG(&plc_buf[4], &(R->SWidthIn));
	PLC_BIN_to_LONG(&plc_buf[8], &(R->OffsetIn));
	PLC_BIN_to_LONG(&plc_buf[12], &(R->GetSWidth));
	PLC_BIN_to_LONG(&plc_buf[16], &(R->TolPos));
	PLC_BIN_to_LONG(&plc_buf[20], &(R->TolNeg));
	PLC_BIN_to_LONG(&plc_buf[24], &(R->SWidthOut));
	PLC_BIN_to_LONG(&plc_buf[28], &(R->OffsetOut));
	PLC_BIN_to_LONG(&plc_buf[32], &(R->POffset));
	PLC_BIN_to_LONG(&plc_buf[36], &(R->MOffset));
	PLC_BIN_to_LONG(&plc_buf[40], &(R->LeadingOffsetEnable));
	PLC_BIN_to_LONG(&plc_buf[44], &(R->TrailingOffsetEnable));
	for(i=48, j=0; i<76; i+=4, j++)
	{
		PLC_BIN_to_LONG(&plc_buf[i], &(R->LeadingOffset[j]));
		PLC_BIN_to_LONG(&plc_buf[i+28], &(R->TrailingOffset[j]));
	}
	PLC_BIN_to_LONG(&plc_buf[104], &(R->ActReset));

	R->MWidth /= EEP_Scale_Num_for_PLC;
	R->SWidthIn /= EEP_Scale_Num_for_PLC;
	R->OffsetIn /= EEP_Scale_Num_for_PLC;
	R->GetSWidth /= EEP_Scale_Num_for_PLC;
	R->TolPos /= EEP_Scale_Num_for_PLC;
	R->TolNeg /= EEP_Scale_Num_for_PLC;
	R->SWidthOut /= EEP_Scale_Num_for_PLC;
	R->OffsetOut /= EEP_Scale_Num_for_PLC;
	R->POffset /= EEP_Scale_Num_for_PLC;
	R->MOffset /= EEP_Scale_Num_for_PLC;
	for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
	{
		R->LeadingOffset[i] /= EEP_Scale_Num_for_PLC;
		R->TrailingOffset[i] /= EEP_Scale_Num_for_PLC;
	}

	R->mm_per_pulse = 0.091993;
	R->CPCStart = R->GetSWidth * CPCRatio / 100; // if width is CPCRatio% of GetSWidth, assume CPC started

	if(dump == TRUE)
	{
		PrintDebug("R->MWidth = %ld\n", R->MWidth);
		PrintDebug("R->SWidthIn = %ld\n", R->SWidthIn);
		PrintDebug("R->OffsetIn= %ld\n", R->OffsetIn);
		PrintDebug("R->GetSWidth= %ld\n", R->GetSWidth);
		PrintDebug("R->TolPos= %ld\n", R->TolPos);
		PrintDebug("R->TolNeg= %ld\n", R->TolNeg);
		PrintDebug("R->SWidthOut= %ld\n", R->SWidthOut);
		PrintDebug("R->OffsetOut= %ld\n", R->OffsetOut);
		PrintDebug("R->POffset= %ld\n", R->POffset);
		PrintDebug("R->MOffset= %ld\n", R->MOffset);
		PrintDebug("R->LeadingOffsetEnable = %ld\n", R->LeadingOffsetEnable);
		PrintDebug("R->TrailingOffsetEnable = %ld\n", R->TrailingOffsetEnable);
		for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
		{
			PrintDebug("R->LeadingOffset[%d] = %ld\n", i, R->LeadingOffset[i]);
		}
		for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
		{
			PrintDebug("R->TrailingOffset[%d] = %ld\n", i, R->TrailingOffset[i]);
		}
		PrintDebug("mm_per_pulse = %f\n", R->mm_per_pulse);
		PrintDebug("CPCStart = %f\n", R->CPCStart);
	}

	if(offsetCoeff != 0)
	{
		if(R->OffsetIn > 0)
		{
			R->OffsetIn += offsetCoeff;
			PrintDebug("OffsetIn is compensate by %ld\n", R->OffsetIn);
		}
		else if(R->OffsetIn < 0)
		{
			R->OffsetIn += (offsetCoeff * -1);
			PrintDebug("OffsetIn is compensate by %ld\n", R->OffsetIn);
		}
	}
}

static void *readPosTask(void * data)
{
	json_object *pos, *pos_val, *cam;
	char buf[BUF_SIZE];
	int i;

	memset(buf, 0, BUF_SIZE);

	while(1)
	{
		if(recv(centering_fd, buf, BUF_SIZE, 0) < 0)
		{
			close(centering_fd);
			printf("recv failed");
		}

		cam = json_tokener_parse(buf);
		pos = json_object_object_get(cam, "Pos");

		if(pos == NULL) continue;

#ifdef VIEW_POS
		printf("\n");
#endif

		for(i=0; i<json_object_array_length(pos); i++)
		{
			pos_val = json_object_array_get_idx(pos, i);
			rWidth[i] = (float)json_object_get_double(pos_val); //LPos02, LPos01, RPos01, RPos02
			if(i == LPos02 && rWidth[LPos02] < 0) rWidth[LPos02] *= -1;
			if(i == LPos01 && rWidth[LPos01] < 0) rWidth[LPos01] *= -1;
#ifdef VIEW_POS
			printf("pos=%f ", json_object_get_double(pos_val));
#endif
		}
	}

	close(centering_fd);
	return NULL;
}

static void *encoderTask(void *data)
{
	struct pollfd enc_fd;
	struct input_event ev;

	enc_fd.fd = open("/dev/input/event0", O_RDONLY);
	enc_fd.events = POLL_IN;

	while(1)
	{
		poll(&enc_fd, 1, -1);

		if(enc_fd.revents & POLL_IN)
		{
			read(enc_fd.fd, &ev, sizeof(struct input_event));
			if(ev.type == 2 && ev.value > 0) cnt_enc++;
		}

#ifdef VIEW_ENCODER_CNT
		PrintDebug("enc=%d\n", cnt_enc);
#endif
	}

	close(enc_fd.fd);
	return NULL;
}

int centering_libs_init(void)
{
	struct sockaddr_in server;
	pthread_t readPosTaskId, encoderTaskId;

	actuator_get_status(&ACT);

	centering_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (centering_fd == -1)
	{
		PrintError("Could not create socket");
		return -1;
	}

	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(CENTERING_PORT);

	if (connect(centering_fd, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error to cam");
		close(centering_fd);
		return -1;
	}

	if (pthread_create( &readPosTaskId, NULL, readPosTask, (void *)NULL ))
	{
		PrintError( "could not create thread for read position from cam %s\n", strerror( errno ));
	}
	if (pthread_create( &encoderTaskId, NULL, encoderTask, (void *)NULL ))
	{
		PrintError( "could not create thread for encoder %s\n", strerror( errno ));
	}

	return 0;
}
