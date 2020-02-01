#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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

#include "json-c/json.h"
#include "splice_libs.h"
#include "splice_utils.h"
#include "actuator.h"
#include "osal_msg.h"
#include "centering.h"
#include "plc.h"

#define CENTERING_PORT 7076

#define STATE_INIT 1
#define STATE_TIP_DETECT 2
#define STATE_CENTERING 3
#define STATE_ETC 4

#define BUF_SIZE 256
#define STREAM_ONOFF_SIZE 32

#define FILE_TIP_DIRECTION "/data/tip_direction"
#define FILE_TIP_OFFSET_DIVICE "/data/offset_divide"
#define MAX_TIP_DIRECTION_STRING 10
#define TIP_LEFT 0x1
#define TIP_RIGHT 0x0

#define LPos02 0
#define LPos01 1
#define RPos01 2
#define RPos02 3

//#define VIEW_POS
//#define VIEW_ENCODER_CNT
#define R_DUMP

extern unsigned char PLCIO;
static int centering_fd;
static float rWidth[4];
static RRegister R;
static act_status ACT;
static int actLLimit;
static int actRLimit;
static int g_onOff;
static int isCentering;
static float mm_per_pulse;

static int cnt_enc;

typedef struct
{
	unsigned int type;
	unsigned int value;
}centeringMsg;

void readRRegister(char dump);

static void enableReadPos(char onOff)
{
	char setStream[STREAM_ONOFF_SIZE];

	if(g_onOff == onOff) return;

	memset(setStream, 0, STREAM_ONOFF_SIZE);
	if(onOff)
	{
		strncpy(setStream, "{\"setStreaming\":\"on\"}", 32);
	}
	else
	{
		strncpy(setStream, "{\"setStreaming\":\"off\"}", 32);
	}

	g_onOff = onOff;
	send(centering_fd, setStream, STREAM_ONOFF_SIZE, 0);
}

void *readPosTask(void * data)
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

#ifdef VIEW_POS
		printf("\n");
#endif

		for(i=0; i<json_object_array_length(pos); i++)
		{
			pos_val = json_object_array_get_idx(pos, i);
			rWidth[i] = (float)json_object_get_double(pos_val); //LPos02, LPos01, RPos01, RPos02
			if(i == LPos02) rWidth[LPos02] *= -1;
			if(i == LPos01) rWidth[LPos01] *= -1;
#ifdef VIEW_POS
			printf("pos=%f ", json_object_get_double(pos_val));
#endif
		}
	}
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
		printf("Tip direction seems look like normal(%s)\n", buf);
		td = TIP_LEFT;
	}
	else if(strncmp("right", buf, MAX_TIP_DIRECTION_STRING) == 0)
	{
		printf("Tip direction seems look like normal(%s)\n", buf);
		td = TIP_RIGHT;
	}
	else
	{
		printf("Tip direction seems look like abnormal(%s)\n", buf);
		td = TIP_LEFT;
	}

	close(fd);
	return td;
}

int getIsCentering(void)
{
	return isCentering;
}

void setIsCentering(char status)
{
	isCentering = status;
}

#define ACT_MOVE_LEFT 0
#define ACT_MOVE_RIGHT 1

void act_move(char direction, int pos)
{
	act_position act_p;


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
		printf("actuator set wrong direction\n");
	}
}

#define TIP_DETECT_POS_DIR_LEFT  (rWidth[LPos02] - rWidth[LPos01])
#define TIP_DETECT_POS_DIR_RIGHT (rWidth[RPos02] - rWidth[RPos01])
#define TIP_OFFSET_DIVIDE_COUNT 5

#define EPC_POS_DIR_LEFT (rWidth[RPos02] - rWidth[RPos01])
#define EPC_POS_DIR_RIGHT (rWidth[LPos02] - rWidth[LPos01])

#define AUTO_MODE !(PLCIO & 0x1)
#define MANUAL_MODE ((PLCIO & 0x7) == 0x7)
#define PLC_RD_WIDTH ((PLCIO & 0x7) == 0x2)
#define PLC_RD_EPC (!(PLCIO & 0x2))
#define PLC_RD_CPC (isCPC && (PLCIO & 0x2))

void edgeCentering(char tip_direction)
{
	float diff;
	int pos;

	if(tip_direction == TIP_LEFT)
	{
		diff = rWidth[LPos01] - (R.GetSWidth/2);
	}
	else
	{
		diff = rWidth[RPos01] - (R.GetSWidth/2);
	}

	if(abs(diff) > 10)
	{
		pos = ACT_MOVE_1MM;
	}
	else
	{
		pos = ACT_MOVE_1MM / 2;
	}

	if(diff > 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_RIGHT, pos);
		}
		else
		{
			act_move(ACT_MOVE_LEFT, pos);
		}
	}
	else if(diff < 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_LEFT, pos);
		}
		else
		{
			act_move(ACT_MOVE_RIGHT, pos);
		}
	}
}

void EPCCentering(char tip_direction, float avgWidth)
{
	float diff;
	int pos;

	if(tip_direction == TIP_LEFT)
	{
		if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
		{
			//printf("1 %f\n", rWidth[RPos01]);
			diff = rWidth[RPos01] - (avgWidth/2);
		}
		else if(rWidth[LPos02] == 0 && rWidth[LPos01] == 0 && rWidth[RPos01] > 0 && rWidth[RPos02] > 0)
		{
			//printf("2 %f\n", rWidth[RPos02]);
			diff = rWidth[RPos02] - (avgWidth/2);
		}
		else
		{
			printf("couldn't be here, left, %f:%f:%f:%f\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
		}
	}
	else
	{
		if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
		{
			//printf("3 %f\n", rWidth[LPos01]);
			diff = rWidth[LPos01] - (avgWidth/2);
		}
		else if(rWidth[LPos02] > 0 && rWidth[LPos01] > 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
		{
			//printf("4 %f\n", rWidth[LPos02]);
			diff = rWidth[LPos02] - (avgWidth/2);
		}
		else
		{
			printf("couldn't be here, right, %f:%f:%f:%f\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
		}
	}

	if(abs(diff) > 10)
	{
		pos = ACT_MOVE_1MM;
	}
	else
	{
		pos = ACT_MOVE_1MM / 2;
	}

	if(diff > 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_RIGHT, pos);
		}
		else
		{
			act_move(ACT_MOVE_LEFT, pos);
		}
	}
	else if(diff < 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_LEFT, pos);
		}
		else
		{
			act_move(ACT_MOVE_RIGHT, pos);
		}
	}
}

void *centeringTask(void *data)
{
	float diff, MWidth, EPCWidth, avgWidth = 0;
	char tip_detect = 0, tip_direction, act_need_reset_flag = TRUE;
	act_position act_p;
	int isCPC = FALSE, avgWidthCnt = 1;
	int i, pos, tip_offset_cnt, tip_offset_flag;
	float leading_tip_width[TIP_OFFSET_DIVIDE_COUNT], trailing_tip_width[TIP_OFFSET_DIVIDE_COUNT];
	int leading_tip_offset[TIP_OFFSET_DIVIDE_COUNT], trailing_tip_offset[TIP_OFFSET_DIVIDE_COUNT];
	float tip_width_enc[TIP_OFFSET_DIVIDE_COUNT];
	int profile_cnt[5];

	while(1)
	{
		if(MANUAL_MODE) // Manual mode, Mask Input 0,1,2 bit, ignore cutting error check
		{
			if(act_need_reset_flag == TRUE)
			{
				tip_direction = getTipDirection();
				actuator_set_current_position(NULL, CMD2_ACT_MOVE_ORG);
				act_need_reset_flag = FALSE;
				readRRegister(TRUE);

				printf("Actuator reset!\n");
				printf("enc = %d, avgWidth=%f, cnt=%d\n", cnt_enc, avgWidth/(avgWidthCnt-1), avgWidthCnt-1);

				avgWidthCnt = 1;
				avgWidth = 0;
			//	test_cnt=0;
			}

			isCPC = FALSE;
			tip_detect = FALSE;
			tip_offset_flag = TRUE;

			enableReadPos(FALSE);
			sendPlcIO(PLC_WR_RESET);
			setIsCentering(FALSE);

			TASK_Sleep(10);

			continue;
		}

		if(AUTO_MODE) // Auto mode
		{
			enableReadPos(TRUE);
			setIsCentering(TRUE);
			act_need_reset_flag = TRUE;
		}

		//printf("%f:%f:%f:%f, PLCIO=%x\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02], (~PLCIO) & 0xF);

		if((rWidth[LPos01] + rWidth[RPos01]) > R.CPCStart) isCPC = TRUE;

		if(tip_direction == TIP_LEFT)
		{
			MWidth = TIP_DETECT_POS_DIR_LEFT;
			EPCWidth = EPC_POS_DIR_LEFT;
		}
		else
		{
			MWidth = TIP_DETECT_POS_DIR_RIGHT;
			EPCWidth = EPC_POS_DIR_RIGHT;
		}

		/* Check whether if SWidthIn is in boundary of MWidth or not*/
		if(R.SWidthIn > (R.GetSWidth * 40 / 100))
		{
			printf("R.SWidthIn is too big to control!\n");
		}

		//printf("%f:%f:%f:%f, %f:%f\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02], MWidth, isCPC);

		if(MWidth >= R.MWidth && !isCPC) //tip detect and tip offset guiding
		{
			if(tip_detect == FALSE)
			{
				printf("Tip detect~!!\n");
				sendPlcIO(PLC_WR_TIP_DETECT);
				sendPlcIO(PLC_WR_CENTERING);
				tip_detect = TRUE;
				cnt_enc = 0;
				tip_offset_cnt = 0;

				for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
				{
					/* Divide leading tip offset area by TIP_OFFSET_DIVIDE_COUNT */
					leading_tip_width[i] = MWidth + ((i+1) * R.SWidthIn/TIP_OFFSET_DIVIDE_COUNT);
					if(R.OffsetIn < TIP_OFFSET_DIVIDE_COUNT)
					{
						printf("OffsetIn is smaller than divide count(%d)\n", TIP_OFFSET_DIVIDE_COUNT);
						leading_tip_offset[i] = 1;
					}
					else
					{
						leading_tip_offset[i] = R.OffsetIn - (i * (R.OffsetIn / TIP_OFFSET_DIVIDE_COUNT));
					}
					printf("ltip %d : [%f][%d]\n", i, leading_tip_width[i], leading_tip_offset[i]);
				}

				for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
				{
					/* Divide trailing tip offset area by TIP_OFFSET_DIVIDE_COUNT */
					trailing_tip_width[i] = R.SWidthIn - ((i+1) * R.SWidthIn/TIP_OFFSET_DIVIDE_COUNT);
				}

				for(i=TIP_OFFSET_DIVIDE_COUNT-1; i >=0; i--)
				{
					if(R.OffsetOut < TIP_OFFSET_DIVIDE_COUNT)
					{
						printf("OffsetOut is smaller than divide count(%d)\n", TIP_OFFSET_DIVIDE_COUNT);
						trailing_tip_offset[i] = 1;
					}
					else
					{
						trailing_tip_offset[i] = R.OffsetOut - (i * (R.OffsetOut / TIP_OFFSET_DIVIDE_COUNT));
					}
					printf("ttip %d : [%f][%d]\n", i, trailing_tip_width[i], trailing_tip_offset[i]);
				}
			}

			if(tip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
			{
				if(leading_tip_width[tip_offset_cnt] > MWidth)
				{
					if(tip_direction == TIP_LEFT && tip_offset_flag)
					{
						act_move(ACT_MOVE_LEFT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM);
						tip_offset_flag = FALSE;
						//printf("tol %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], MWidth);
					}
					else if(tip_direction == TIP_RIGHT && tip_offset_flag)
					{
						act_move(ACT_MOVE_RIGHT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM);
						tip_offset_flag = FALSE;
						//printf("tor %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], MWidth);
					}
				}
				else
				{
					tip_offset_cnt++;
					tip_offset_flag = TRUE;
				}
			}
			else
			{
				edgeCentering(tip_direction);
			}

		}
		else if(PLC_RD_CPC) // masking PLC_RD_CPC_TO_EPC
		{
			edgeCentering(tip_direction);
		}
		else if(PLC_RD_EPC)
		{
			if(EPCWidth >= R.SWidthOut)
			{
				EPCCentering(tip_direction, avgWidth/avgWidthCnt);
				tip_offset_cnt = 0;
				tip_offset_flag = TRUE;
			}
			else
			{
				if(tip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
				{
					if(trailing_tip_width[tip_offset_cnt] <= EPCWidth)
					{
						if(tip_direction == TIP_LEFT && tip_offset_flag)
						{
							act_move(ACT_MOVE_LEFT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM);
							tip_offset_flag = FALSE;
							//printf("tol %d, %f:%f\n", tip_offset_cnt, trailing_tip_width[tip_offset_cnt], EPCWidth);
						}
						else if(tip_direction == TIP_RIGHT && tip_offset_flag)
						{
							act_move(ACT_MOVE_RIGHT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM);
							tip_offset_flag = FALSE;
							//printf("tor %d, %f:%f\n", tip_offset_cnt, trailing_tip_width[tip_offset_cnt], EPCWidth);
						}
					}
					else
					{
						tip_offset_cnt++;
						tip_offset_flag = TRUE;
					}
				}
			}
		}

		if(PLC_RD_WIDTH)
		{
			avgWidth += (rWidth[LPos01] + rWidth[RPos01]);
			avgWidthCnt++;
		}

		TASK_Sleep(10);
	}
}

void readRRegister(char dump)
{
	char plc_buf[108];
	int i, j;

	/* Though R register size is 54byte, plc_buf should bigger twice than EEP_R_Register_Size.
	 * This is for PLC internal implementation */
	if(sendPlc(EEP_Data_Start_R_Register, plc_buf, EEP_R_Register_Size, true) != PLC_COMM_ACK)
	{
		printf("failed to get R register from PLC\n");
		return;
	}

	PLC_BIN_to_LONG(&plc_buf[0], &R.MWidth);
	PLC_BIN_to_LONG(&plc_buf[4], &R.SWidthIn);
	PLC_BIN_to_LONG(&plc_buf[8], &R.OffsetIn);
	PLC_BIN_to_LONG(&plc_buf[12], &R.GetSWidth);
	PLC_BIN_to_LONG(&plc_buf[16], &R.TolPos);
	PLC_BIN_to_LONG(&plc_buf[20], &R.TolNeg);
	PLC_BIN_to_LONG(&plc_buf[24], &R.SWidthOut);
	PLC_BIN_to_LONG(&plc_buf[28], &R.OffsetOut);
	PLC_BIN_to_LONG(&plc_buf[32], &R.POffset);
	PLC_BIN_to_LONG(&plc_buf[36], &R.MOffset);
	PLC_BIN_to_LONG(&plc_buf[40], &R.LeadingOffsetEnable);
	PLC_BIN_to_LONG(&plc_buf[44], &R.TrailingOffsetEnable);
	for(i=48, j=0; i<76; i+=4, j++)
	{
		PLC_BIN_to_LONG(&plc_buf[i], &(R.LeadingOffset[j]));
		PLC_BIN_to_LONG(&plc_buf[i+28], &(R.TrailingOffset[j]));
	}
	PLC_BIN_to_LONG(&plc_buf[104], &R.ActReset);

	R.MWidth /= EEP_Scale_Num_for_PLC;
	R.SWidthIn /= EEP_Scale_Num_for_PLC;
	R.OffsetIn /= EEP_Scale_Num_for_PLC;
	R.GetSWidth /= EEP_Scale_Num_for_PLC;
	//R.GetSWidth = 180;//for demo
	R.TolPos /= EEP_Scale_Num_for_PLC;
	R.TolNeg /= EEP_Scale_Num_for_PLC;
	R.SWidthOut /= EEP_Scale_Num_for_PLC;
	R.OffsetOut /= EEP_Scale_Num_for_PLC;
	R.POffset /= EEP_Scale_Num_for_PLC;
	R.MOffset /= EEP_Scale_Num_for_PLC;
	for(i=0; i<7; i++)
	{
		R.LeadingOffset[i] /= EEP_Scale_Num_for_PLC;
		R.TrailingOffset[i] /= EEP_Scale_Num_for_PLC;
	}

	mm_per_pulse = 0.091993;
	R.CPCStart = R.GetSWidth * 90 / 100; // if width is 90% of GetSWidth, assume CPC started

#ifdef R_DUMP
	if(dump == TRUE)
	{
		printf("R.MWidth = %ld\n", R.MWidth);
		printf("R.SWidthIn = %ld\n", R.SWidthIn);
		printf("R.OffsetIn= %ld\n", R.OffsetIn);
		printf("R.GetSWidth= %ld\n", R.GetSWidth);
		printf("R.TolPos= %ld\n", R.TolPos);
		printf("R.TolNeg= %ld\n", R.TolNeg);
		printf("R.SWidthOut= %ld\n", R.SWidthOut);
		printf("R.OffsetOut= %ld\n", R.OffsetOut);
		printf("R.POffset= %ld\n", R.POffset);
		printf("R.MOffset= %ld\n", R.MOffset);
		printf("R.LeadingOffsetEnable = %ld\n", R.LeadingOffsetEnable);
		printf("R.TrailingOffsetEnable = %ld\n", R.TrailingOffsetEnable);
		for(i=0; i<7; i++)
		{
			printf("R.LeadingOffset[%d] = %ld\n", i, R.LeadingOffset[i]);
		}
		for(i=0; i<7; i++)
		{
			printf("R.TrailingOffset[%d] = %ld\n", i, R.TrailingOffset[i]);
		}
		printf("mm_per_pulse = %f\n", mm_per_pulse);
		printf("CPCStart = %f\n", R.CPCStart);
	}
#endif
}

void *encoderTask(void *data)
{
	struct pollfd enc_fd;
	struct input_event ev;
	int i;
	char buf[8];

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
		printf("enc=%d\n", cnt_enc);
#endif
	}
}

int centering_init(void)
{
	struct sockaddr_in server;
	pthread_t centeringTaskId, readPosTaskId, encoderTaskId;

	centering_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (centering_fd == -1)
	{
		printf("Could not create socket");
	}

	printf("Centering 0.60\n");

	/* do not remove since CPCStart in centering task should be calculated R.GetSWidth. */
	readRRegister(FALSE);
	actuator_get_status(&ACT);
	actLLimit = ACT.act_l_limit * 1000;
	actRLimit = ACT.act_r_limit * 1000;

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

	if (pthread_create( &encoderTaskId, NULL, encoderTask, (void *)NULL ))
	{
		PrintError( "could not create thread for encoder %s\n", strerror( errno ));
	}
	if (pthread_create( &centeringTaskId, NULL, centeringTask, (void *)NULL ))
	{
		PrintError( "could not create thread for centering %s\n", strerror( errno ));
	}
#if 1
	if (pthread_create( &readPosTaskId, NULL, readPosTask, (void *)NULL ))
	{
		PrintError( "could not create thread for read position from cam %s\n", strerror( errno ));
	}
#endif


	return 0;
}
