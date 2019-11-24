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

#include "json-c-inc/json.h"
#include "splice_libs.h"
#include "splice_utils.h"
#include "actuator.h"
#include "osal_msg.h"
#include "plc.h"

#define CENTERING_PORT 7076

#define STATE_INIT 1
#define STATE_TIP_DETECT 2
#define STATE_CENTERING 3
#define STATE_ETC 4

#define BUF_SIZE 256

//#define VIEW_POS
#define R_DUMP

extern unsigned char PLCIO;
static int centering_fd;
static double rWidth[4];
static RRegister R;
static act_status ACT;
static int actLLimit;
static int actRLimit;

typedef struct
{
	unsigned int type;
	unsigned int value;
}centeringMsg;

void *readPosTask(void * data)
{
	json_object *pos, *pos_val, *cam;
	char setStreamOn[32], buf[BUF_SIZE];
	int i;

	memset(buf, 0, BUF_SIZE);
	memset(setStreamOn, 0, 32);
	strncpy(setStreamOn, "{\"setStreaming\":\"on\"}", 32);
	send(centering_fd, setStreamOn, 32, 0);

	while(1)
	{
		if(recv(centering_fd, buf, BUF_SIZE, 0) < 0)
		{
			close(centering_fd);
			printf("recv failed");
		}

		cam = json_tokener_parse(buf);
		pos = json_object_object_get(cam, "getImage");

#ifdef VIEW_POS
		printf("\n");
#endif

		for(i=0; i<json_object_array_length(pos); i++)
		{
			pos_val = json_object_array_get_idx(pos, i);
			rWidth[i] = json_object_get_double(pos_val);
#ifdef VIEW_POS
			printf("pos=%f ", json_object_get_double(pos_val));
#endif
		}
	}
}

void *centeringTask(void *data)
{
	static char tip_detect = 0;
	double diff;
	unsigned char plcIn = 0;
	act_position act_p, prev_p;
	int CPCStart = R.GetSWidth * 90 / 100; // if width is 98% of GetSWidth, assume CPC started
	int i, isCPC = FALSE, state = STATE_INIT, cnt = 1;
	int pos, avgWidth = 0;

	while(1)
	{
		//OSAL_Memset(&msg, 0, sizeof(PI_EXTSIG_Message_t));
		//if (OSAL_MSG_ReceiveTimeout(plc_msg_id, (void *)&plcIn,  sizeof(unsigned char), 10) < 0)

		if((rWidth[0] + rWidth[1]) > CPCStart) isCPC = TRUE;

		if(rWidth[3] >= R.MWidth && !isCPC)
		{
			if(tip_detect == FALSE)
			{
				sendPlcIO(PLC_WR_TIP_DETECT);
				TASK_Sleep(500);
				sendPlcIO(PLC_WR_CENTERING);
				tip_detect = TRUE;
			}

			diff = (rWidth[2] + rWidth[3]) - (R.GetSWidth / 2); //demo web's width is about 180mm
			if(abs(diff) > 10)
			{
				pos = ACT_MOVE_1MM * diff;
			}
			else
			{
				pos = ACT_MOVE_1MM / 10;
			}

			//printf("tip, diff = %f, %f:%f, pos=%d\n", diff, rWidth[2], rWidth[3], pos);
			if(diff > 0 && pos  > actLLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_lsb = ACT.act_l_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff > 0 && pos < actLLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_msb |= pos >> 8;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0 && pos > actRLimit)
			{
				act_p.act_cur_position_msb = 0x00;
				act_p.act_cur_position_lsb = ACT.act_r_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0.0 && pos < actRLimit)
			{
				act_p.act_cur_position_msb = 0x00;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
		}
		else if(isCPC && (PLCIO & 0x2)) // masking PLC_RD_CPC_TO_EPC
		{
			int sWidth, median;
			sWidth = rWidth[0] + rWidth[1];
			median = sWidth / 2;
			diff = rWidth[0] - median;

			if(abs(diff) > 10)
			{
				pos = ACT_MOVE_1MM;
			}
			else
			{
				pos = ACT_MOVE_1MM / 2;
			}
			//printf("cpc, diff = %f, %f:%f, pos=%d, plc=%d\n", diff, rWidth[0], rWidth[1], pos, PLCIO);

			if(diff > 0 && pos > actLLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_lsb = ACT.act_l_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff > 0 && pos < actLLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_msb |= pos >> 8;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0 && pos > actRLimit)
			{
				act_p.act_cur_position_msb = 0x00;
				act_p.act_cur_position_lsb = ACT.act_r_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0.0 && pos < actRLimit)
			{
				act_p.act_cur_position_msb = 0x00;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
		}
		else if(!(PLCIO & 0x2)) //Going to EPC
		{
			if(rWidth[2] > 0 && rWidth[3])
			{
				diff = (rWidth[2] + rWidth[3]) - (R.GetSWidth / 2); //demo web's width is about 180mm
			}
			else
			{
				diff = 0;
			}
			if(abs(diff) > 10)
			{
				pos = ACT_MOVE_1MM * diff;
			}
			else
			{
				pos = ACT_MOVE_1MM / 10;
			}

			//printf("epc, diff = %f, %f:%f, pos=%d, plc=%d\n", diff, rWidth[2], rWidth[3], pos, PLCIO);
			if(diff > 0 && pos  > actLLimit)
			{
				act_p.act_cur_position_msb = 0x0;
				act_p.act_cur_position_lsb = ACT.act_l_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff > 0 && pos < actLLimit)
			{
				act_p.act_cur_position_msb = 0x0;
				act_p.act_cur_position_msb |= pos >> 8;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0 && pos > actRLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_lsb = ACT.act_r_limit;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}
			else if(diff < 0.0 && pos < actRLimit)
			{
				act_p.act_cur_position_msb = 0x80;
				act_p.act_cur_position_lsb = pos;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
			}

			if((PLCIO & 0x1))
			{
				sendPlcIO(PLC_WR_RESET);
				tip_detect = FALSE;
			}
		}

		if(!(PLCIO & 0x4) && (PLCIO & 0x2)) //PLC_RD_SAVE_WIDTH
		{
			avgWidth += rWidth[1];
			avgWidth /= cnt;
			cnt++;
			printf("avgWidth = %d, cnt = %d\n", avgWidth, cnt);
		}

#if 0
		switch(state)
		{
			case STATE_INIT:
				if(rWidth[3] >= 3)
				{
					sendPlcIO(PLC_WR_TIP_DETECT);
					printf("tip detect %f:%f\n", rWidth[2], rWidth[3]);
					if(rWidth[3] > 0 && rWidth[2] > 0)
					{
						diff = rWidth[2] + rWidth[3];
						
						diff -= 90; //demo web's width is about 180mm
						printf("diff = %f\n", diff);
						if(diff > 0)
						{
							act_p.act_cur_position_lsb |= (int)diff & 0xff;
							act_p.act_cur_position_msb |= ((int)diff >> 8) & 0x7f;
							act_p.act_cur_position_msb |= 0x80;
							actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
						}
					}
					state = STATE_TIP_DETECT;

					//tip_detect = true;
					//TASK_Sleep(2000);
				}
				break;

			case STATE_TIP_DETECT:
				sendPlcIO(PLC_WR_CENTERING);
				state = STATE_CENTERING;
				break;

			case STATE_CENTERING:
				//actuator_get_current_postion(&prev_p);
				//cur = (prev_p.act_cur_position_msb & 0x7F) << 8 | prev_p.act_cur_position_lsb;
				//if(prev_p.act_cur_position_msb & 0x80) cur *= -1;

				//diff = (rWidth[0] - rWidth[1]) - cur;
				if(rWidth[0] > 0 && rWidth[1] > 0)
				{
					/* under current setting, rWidth[0] = 77.53, rWidth[1] = 100
					 * diff is too big to control with actuator, compensate position */
					rWidth[0] += 10;
					rWidth[1] -= 10;
					diff = (rWidth[0] - rWidth[1]);
				}
				if(diff < 0)
				{
					act_p.act_cur_position_msb = 0x80;
					diff *= -1;
				}

				printf("%f:%f:%d:%d\n", rWidth[0], rWidth[1], cur, diff);
				if(abs((int)diff) < 10) continue;

				//snprintf(log_buf, 128, "%f:%f:%d:%d\n", rWidth[0], rWidth[1], cur, diff);
				act_p.act_cur_position_lsb |= (int)diff & 0xff;
				act_p.act_cur_position_msb |= ((int)diff >> 8) & 0x7f;
				actuator_set_current_position(&act_p, CMD2_ACT_MOVE);
				break;
		}
#endif
		//memset(buf, 0, BUF_SIZE);
		TASK_Sleep(50);
	}
}

void readRRegister(void)
{
	char plc_buf[108];
	int i, j;
	long tmp;

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
	//R.GetSWidth /= EEP_Scale_Num_for_PLC;
	R.GetSWidth = 180;//for demo
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


#ifdef R_DUMP
	printf("R.MWidth = %ld\n", R.MWidth);
	printf("R.SWidthIn = %ld\n", R.SWidthIn);
	printf("R.OffsetIn= %ld\n", R.OffsetIn);
	printf("R.GetSWIdth= %ld\n", R.GetSWidth);
	printf("R.TolPos= %ld\n", R.TolPos);
	printf("R.TolNeg= %ld\n", R.TolNeg);
	printf("R.SWidthOut= %ld\n", R.SWidthOut);
	printf("R.OffsetOut= %ld\n", R.OffsetOut);
	printf("R.POffset= %ld\n", R.POffset);
	printf("R.MOffset= %ld\n", R.MOffset);
	printf("R.LeadingOffsetEnablei = %ld\n", R.LeadingOffsetEnable);
	printf("R.TrailingOffsetEnable = %ld\n", R.TrailingOffsetEnable);
	for(i=0; i<7; i++)
	{
		printf("R.LeadingOffset[%d] = %ld\n", i, R.LeadingOffset[i]);
	}
	for(i=0; i<7; i++)
	{
		printf("R.TrailingOffset[%d] = %ld\n", i, R.TrailingOffset[i]);
	}
#endif
}

int centering_init(void)
{
	struct sockaddr_in server;
	pthread_t centeringTaskId= 0, readPosTaskId;

	centering_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (centering_fd == -1)
	{
		printf("Could not create socket");
	}

	readRRegister();
	actuator_get_status(&ACT);
	actLLimit = ACT.act_l_limit * 1000;
	actRLimit = ACT.act_r_limit * 1000;
	actuator_set_current_position(NULL, CMD2_ACT_MOVE_ORG);

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
