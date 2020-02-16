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
#include "datatypes.h"
#include "splice_libs.h"
#include "splice_utils.h"
#include "actuator.h"
#include "centering.h"
#include "plc.h"
#include "sys_trace.h"

#define CENTERING_PORT 7076

#define BUF_SIZE 4096
#define STREAM_ONOFF_SIZE 32

#define FILE_TIP_DIRECTION "/data/tip_direction"
#define FILE_TIP_OFFSET_DIVICE "/data/offset_divide"
#define MAX_TIP_DIRECTION_STRING 10
#define TIP_LEFT 0x1
#define TIP_RIGHT 0x0

#define LEADING_TIP_SECTION  0xAA
#define LEADING_EPC_SECTION  0xAB
#define CPC_SECTION          0xAC
#define TRAILING_EPC_SECTION 0xAD
#define TRAILING_TIP_SECTION 0xAE

#define LPos02 0
#define LPos01 1
#define RPos01 2
#define RPos02 3

#define LPos02_R_FLAG 0x1
#define LPos01_R_FLAG 0x2
#define RPos01_R_FLAG 0x4
#define RPos02_R_FLAG 0x8

//#define VIEW_POS
//#define VIEW_ENCODER_CNT
#define R_DUMP

extern unsigned char PLCIO;
static int centering_fd;
static float rWidth[4];
static RRegister R;
static act_status ACT;
static int isCentering;
static float mm_per_pulse;
static int tip_offset_cnt, tip_offset_flag;
static char tip_direction;
static int cnt_enc;
static char current_section;
static int RWidth_FLAG;

typedef struct
{
	unsigned int type;
	unsigned int value;
}centeringMsg;

void readRRegister(char dump);

static void enableReadPos(char onOff)
{
	char setStream[STREAM_ONOFF_SIZE];

	memset(setStream, 0, STREAM_ONOFF_SIZE);
	if(onOff)
	{
		strncpy(setStream, "{\"setStreaming\":\"on\"}", 32);
	}
	else
	{
		strncpy(setStream, "{\"setStreaming\":\"off\"}", 32);
	}

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
			if(i == LPos02 && rWidth[LPos02] < 0) rWidth[LPos02] *= -1;
			if(i == LPos01 && rWidth[LPos01] < 0) rWidth[LPos01] *= -1;
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

#define TIP_OFFSET_DIVIDE_COUNT 5

#define AUTO_MODE !(PLCIO & 0x1)
#define MANUAL_MODE ((PLCIO & 0x7) == 0x7)
#define PLC_RD_WIDTH ((PLCIO & 0x7) == 0x2)
#define PLC_RD_EPC (!(PLCIO & 0x2))
#define PLC_RD_CPC (isCPC && (PLCIO & 0x2))

float getRWidth(char tip_direction, char section)
{
	if(tip_direction == TIP_LEFT)
	{
		if(section == LEADING_TIP_SECTION || section == LEADING_EPC_SECTION)
		{
			if(rWidth[LPos02] > 0 && rWidth[LPos01] > 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos02_R_FLAG | LPos01_R_FLAG;
				return rWidth[LPos02] - rWidth[LPos01];
			}
			else if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos01_R_FLAG | RPos01_R_FLAG;
				return rWidth[LPos01] + rWidth[RPos01];
			}
			else if(rWidth[LPos02] == 0 && rWidth[LPos01] == 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = 0;
				return 0;
			}
			else
			{
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
		else if(section == CPC_SECTION)
		{
			if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos01_R_FLAG | RPos01_R_FLAG;
				return rWidth[LPos01] - rWidth[RPos01];
			}
			else
			{
				RWidth_FLAG = 0;
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
		else if(section == TRAILING_TIP_SECTION || section == TRAILING_EPC_SECTION)
		{
			if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos01_R_FLAG | RPos01_R_FLAG;
				return rWidth[RPos01] + rWidth[LPos01];
			}
			else if(rWidth[LPos02] == 0 && rWidth[LPos01] == 0 && rWidth[RPos01] > 0 && rWidth[RPos02] > 0)
			{
				RWidth_FLAG = RPos01_R_FLAG | RPos02_R_FLAG;
				return rWidth[RPos02] - rWidth[RPos01];
			}
			else
			{
				RWidth_FLAG = 0;
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
	}
	else if(tip_direction == TIP_RIGHT)
	{
		if(section == LEADING_TIP_SECTION || section == LEADING_EPC_SECTION)
		{
			if(rWidth[LPos02] == 0 && rWidth[LPos01] == 0 && rWidth[RPos01] > 0 && rWidth[RPos02] > 0)
			{
				RWidth_FLAG = RPos02_R_FLAG | RPos01_R_FLAG;
				return rWidth[RPos02] - rWidth[RPos01];
			}
			else if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = RPos01_R_FLAG | LPos01_R_FLAG;
				return rWidth[RPos01] + rWidth[LPos01];
			}
			else if(rWidth[LPos02] == 0 && rWidth[LPos01] == 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = 0;
				return 0;
			}
			else
			{
				RWidth_FLAG = 0;
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
		else if(section == CPC_SECTION)
		{
			if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos01_R_FLAG | RPos01_R_FLAG;
				return rWidth[LPos01] - rWidth[RPos01];
			}
			else
			{
				RWidth_FLAG = 0;
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
		else if(section == TRAILING_TIP_SECTION || section == TRAILING_EPC_SECTION)
		{
			if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos01_R_FLAG | RPos01_R_FLAG;
				return rWidth[LPos01] + rWidth[RPos01];
			}
			else if(rWidth[LPos02] > 0 && rWidth[LPos01] > 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
			{
				RWidth_FLAG = LPos02_R_FLAG | LPos01_R_FLAG;
				return rWidth[LPos02] - rWidth[LPos01];
			}
			else
			{
				RWidth_FLAG = 0;
				PrintError("Error on getRWidth, %d:%d, %f:%f:%f:%f\n", tip_direction, section, rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);
				//send dust error to PLC
			}
		}
	}
	else
	{
		PrintError("Unrecognized tip direction %d\n", tip_direction);
	}

	return -1;
}

void tipEdgeCentering(char tip_edge_guide, long OffsetIn)
{
	float pos_diff, ref_pos, ref_width;
	int act_pos, coeff = 3; //determine how fast set to OffsetIn position

	if(tip_edge_guide == TRUE)
	{
		ref_pos = (R.GetSWidth/2) + OffsetIn;
	}
	else
	{
		OffsetIn = 0;
		ref_pos = (R.GetSWidth/2);
	}

	if(tip_direction == TIP_LEFT)
	{
		if((RWidth_FLAG & LPos02_R_FLAG) && (RWidth_FLAG & LPos01_R_FLAG))
		{
			pos_diff = rWidth[LPos02] - ref_pos;
		}
		else if((RWidth_FLAG & LPos01_R_FLAG) && (RWidth_FLAG & RPos01_R_FLAG))
		{
			pos_diff = rWidth[LPos01] - ref_pos;
		}
		else
		{
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
		}
	}
	else
	{
		if((RWidth_FLAG & RPos02_R_FLAG) && (RWidth_FLAG & RPos01_R_FLAG))
		{
			pos_diff = rWidth[RPos02] - ref_pos;
		}
		else if((RWidth_FLAG & RPos01_R_FLAG) && (RWidth_FLAG & LPos01_R_FLAG))
		{
			pos_diff = rWidth[RPos01] - ref_pos;
		}
		else
		{
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
		}

	}

	/* Divide more specifically to prevent meaningless movement of actuator at tip detect position */
	if(pos_diff > 3 || pos_diff < -3)
	{
			act_pos = ACT_MOVE_1MM;
	}
	else if(pos_diff > 1 || pos_diff < -1)
	{
			act_pos = ACT_MOVE_1MM / 4;
	}
	else
	{
			act_pos = ACT_MOVE_1MM / 10;
	}

	//printf("%d:%f:%d:%f:%f\n", tip_edge_guide, pos_diff, act_pos, rWidth[LPos02], rWidth[LPos01]);

	if(pos_diff > 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
	}
	else
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
	}
}

void CPCCentering(float RWidth)
{
	int act_pos;

	if(RWidth > 5 || RWidth < -5)
	{
		act_pos = ACT_MOVE_1MM / 2;
	}
	else if(RWidth > 3 || RWidth < -3)
	{
		act_pos = ACT_MOVE_1MM / 4;
	}
	else if(RWidth > 1 || RWidth < -1)
	{
		act_pos = ACT_MOVE_1MM / 10;
	}
	else
	{
		act_pos = ACT_MOVE_1MM / 10;
	}

	if(RWidth > 0)
	{
		if(tip_direction == TIP_LEFT)
			act_move(ACT_MOVE_RIGHT, act_pos);
		else
			act_move(ACT_MOVE_LEFT, act_pos);
	}
	else
	{
		if(tip_direction == TIP_LEFT)
			act_move(ACT_MOVE_LEFT, act_pos);
		else
			act_move(ACT_MOVE_RIGHT, act_pos);
	}
}

void trailingEPCCentering(float avgWidth)
{
	float diff;
	int act_pos;

	if(tip_direction == TIP_LEFT)
	{
		if((RWidth_FLAG & LPos01_R_FLAG) && (RWidth_FLAG & RPos01_R_FLAG))
		{
			diff = rWidth[RPos01] - (avgWidth/2);
			//printf("1 %f\n", diff);
		}
		else if((RWidth_FLAG & RPos01_R_FLAG) && (RWidth_FLAG & RPos02_R_FLAG))
		{
			diff = rWidth[RPos02] - (avgWidth/2);
			//printf("2 %f\n", diff);
		}
		else
		{
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
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
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
		}
	}

	if(diff > 3 || diff < -3)
	{
			act_pos = ACT_MOVE_1MM;
	}
	else if(diff > 1 || diff < -1)
	{
			act_pos = ACT_MOVE_1MM / 4;
	}
	else
	{
			act_pos = ACT_MOVE_1MM / 10;
	}

	if(diff > 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
	}
	else
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
	}
}

void trailingTipGuide(long OffsetOut)
{
	float pos_diff, ref_pos, ref_width;
	int act_pos, coeff = 3; //determine how fast set to OffsetIn position

	ref_pos = (R.GetSWidth/2) + OffsetOut;

	if(tip_direction == TIP_LEFT)
	{
		if((RWidth_FLAG & LPos01_R_FLAG) && (RWidth_FLAG & RPos01_R_FLAG))
		{
			pos_diff = rWidth[RPos01] - ref_pos;
		}
		else if((RWidth_FLAG & RPos01_R_FLAG) && (RWidth_FLAG & RPos02_R_FLAG))
		{
			pos_diff = rWidth[RPos02] - ref_pos;
		}
		else
		{
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
		}
	}
	else
	{
		if((RWidth_FLAG & LPos01_R_FLAG) && (RWidth_FLAG & RPos01_R_FLAG))
		{
			pos_diff = rWidth[LPos01] - ref_pos;
		}
		else if((RWidth_FLAG & LPos02_R_FLAG) && (RWidth_FLAG & LPos01_R_FLAG))
		{
			pos_diff = rWidth[LPos02] - ref_pos;
		}
		else
		{
			PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
		}
	}

	//printf("3 %f\n", pos_diff);
	if(pos_diff > 3 || pos_diff < -3)
	{
			act_pos = ACT_MOVE_1MM;
	}
	else if(pos_diff > 1 || pos_diff < -1)
	{
			act_pos = ACT_MOVE_1MM / 4;
	}
	else
	{
			act_pos = ACT_MOVE_1MM / 10;
	}

	//printf("%d:%f:%d:%f:%f\n", tip_edge_guide, pos_diff, act_pos, rWidth[LPos02], rWidth[LPos01]);

	if(pos_diff > 0)
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
	}
	else
	{
		if(tip_direction == TIP_LEFT)
		{
			act_move(ACT_MOVE_LEFT, act_pos);
		}
		else
		{
			act_move(ACT_MOVE_RIGHT, act_pos);
		}
	}
}
static void tip_offset_divide(float RWidth, float *leading_tip_width, float *trailing_tip_width, int *leading_tip_offset, int *trailing_tip_offset)
{
	int i;

	for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
	{
		/* Divide leading tip offset area by TIP_OFFSET_DIVIDE_COUNT */
		leading_tip_width[i] = RWidth + ((i+1) * R.SWidthIn/TIP_OFFSET_DIVIDE_COUNT);
		if(R.OffsetIn != 0 && abs(R.OffsetIn) < TIP_OFFSET_DIVIDE_COUNT)
		{
			PrintDebug("OffsetIn is smaller than divide count(%d)\n", TIP_OFFSET_DIVIDE_COUNT);
			leading_tip_offset[i] = 1;
		}
		else if(R.OffsetIn == 0)
		{
			leading_tip_offset[i] = 0;
		}
		else
		{
			leading_tip_offset[i] = abs(R.OffsetIn) - (i * (abs(R.OffsetIn) / TIP_OFFSET_DIVIDE_COUNT));
		}

		PrintDebug("ltip %d : [%f][%d]\n", i, leading_tip_width[i], leading_tip_offset[i]);

		/* Divide trailing tip offset area by TIP_OFFSET_DIVIDE_COUNT */
		trailing_tip_width[i] = R.SWidthOut - ((i+1) * R.SWidthOut/TIP_OFFSET_DIVIDE_COUNT);
	}

	for(i=TIP_OFFSET_DIVIDE_COUNT-1; i >=0; i--)
	{
		if(R.OffsetOut != 0 && abs(R.OffsetOut) < TIP_OFFSET_DIVIDE_COUNT)
		{
			PrintDebug("OffsetOut is smaller than divide count(%d)\n", TIP_OFFSET_DIVIDE_COUNT);
			trailing_tip_offset[i] = 1;
		}
		else if(R.OffsetOut == 0)
		{
			trailing_tip_offset[i] = 0;
		}
		else
		{
			trailing_tip_offset[i] = abs(R.OffsetOut) - (i * (abs(R.OffsetOut) / TIP_OFFSET_DIVIDE_COUNT));
		}

		PrintDebug("ttip %d : [%f][%d]\n", i, trailing_tip_width[i], trailing_tip_offset[i]);
	}
}

static void divided_tip_guide(float RWidth, float *leading_tip_width, int *leading_tip_offset)
{
	if(tip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
	{
		if(leading_tip_width[tip_offset_cnt] > RWidth)
		{
			if(tip_direction == TIP_LEFT && tip_offset_flag && R.OffsetIn != 0)
			{
				if(R.OffsetIn > 0)
					act_move(ACT_MOVE_LEFT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);

				tip_offset_flag = FALSE;
				PrintDebug("ltol %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], RWidth);
			}
			else if(tip_direction == TIP_RIGHT && tip_offset_flag && R.OffsetIn != 0)
			{
				if(R.OffsetIn > 0)
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_LEFT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);

				//tip_offset_flag = FALSE;
				PrintDebug("ltor %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], RWidth);
			}
			else if(tip_offset_flag && R.OffsetIn == 0)//If R.OffsetIn is 0, apply edge centering
			{
				tipEdgeCentering(TRUE, R.OffsetIn);
				//tip_offset_flag = FALSE;
				//printf("ledge\n");
			}
		}
		else
		{
			tip_offset_cnt++;
			tip_offset_flag = TRUE; //Used for move actuator once per each leading_tip_width
		}
	}
	else
	{
		tipEdgeCentering(FALSE, 0);
	}
}

static void leading_tip_guide(float RWidth, float *leading_tip_width, int *leading_tip_offset)
{
	if(tip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
	{
		if(leading_tip_width[tip_offset_cnt] > RWidth)
		{
			if(tip_direction == TIP_LEFT && tip_offset_flag && R.OffsetIn != 0)
			{
				if(R.OffsetIn > 0)
					act_move(ACT_MOVE_LEFT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);

				tip_offset_flag = FALSE;
				PrintDebug("ltol %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], RWidth);
			}
			else if(tip_direction == TIP_RIGHT && tip_offset_flag && R.OffsetIn != 0)
			{
				if(R.OffsetIn > 0)
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_LEFT, leading_tip_offset[tip_offset_cnt] * ACT_MOVE_1MM_HALF);

				//tip_offset_flag = FALSE;
				PrintDebug("ltor %d, %f:%f\n", tip_offset_cnt, leading_tip_width[tip_offset_cnt], RWidth);
			}
			else if(tip_offset_flag && R.OffsetIn == 0)//If R.OffsetIn is 0, apply edge centering
			{
				tipEdgeCentering(TRUE, R.OffsetIn);
				//tip_offset_flag = FALSE;
				//printf("ledge\n");
			}
		}
		else
		{
			tip_offset_cnt++;
			tip_offset_flag = TRUE; //Used for move actuator once per each leading_tip_width
		}
	}
	else
	{
		tipEdgeCentering(FALSE, 0);
	}
}

static void trailing_epc_tip_guide(float EPCWidth, float avgWidth, float *trailing_tip_width, int *trailing_tip_offset)
{
	if(EPCWidth >= R.SWidthOut)
	{
		trailingEPCCentering(avgWidth);
		tip_offset_cnt = 0;
		tip_offset_flag = TRUE;
	}
	else
	{
		if(tip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
		{
			if(trailing_tip_width[tip_offset_cnt] <= EPCWidth)
			{
				if(tip_direction == TIP_LEFT && tip_offset_flag && R.OffsetOut != 0)
				{
					if(R.OffsetOut > 0)
						act_move(ACT_MOVE_LEFT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_2MM);
					else
						act_move(ACT_MOVE_RIGHT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_2MM);

					tip_offset_flag = FALSE;
					PrintDebug("ttol %d, %f:%f\n", tip_offset_cnt, trailing_tip_width[tip_offset_cnt], EPCWidth);
				}
				else if(tip_direction == TIP_RIGHT && tip_offset_flag && R.OffsetOut != 0)
				{
					if(R.OffsetOut > 0)
						act_move(ACT_MOVE_RIGHT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_2MM);
					else
						act_move(ACT_MOVE_LEFT, trailing_tip_offset[tip_offset_cnt] * ACT_MOVE_2MM);

					tip_offset_flag = FALSE;
					PrintDebug("ttor %d, %f:%f\n", tip_offset_cnt, trailing_tip_width[tip_offset_cnt], EPCWidth);
				}
				else if(tip_offset_flag && R.OffsetOut == 0)
				{
					trailingEPCCentering(avgWidth);
					tip_offset_flag = FALSE;
					PrintDebug("epc\n");
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

void *centeringTask(void *data)
{
	float RWidth, avgWidth = 0;
	char tip_detect = 0, act_need_reset_flag = TRUE, rregister_need_read_flag = TRUE;
	int isCPC = FALSE, avgWidthCnt = 1;
	float leading_tip_width[TIP_OFFSET_DIVIDE_COUNT], trailing_tip_width[TIP_OFFSET_DIVIDE_COUNT];
	int leading_tip_offset[TIP_OFFSET_DIVIDE_COUNT], trailing_tip_offset[TIP_OFFSET_DIVIDE_COUNT];

	while(1)
	{
		if(MANUAL_MODE) // Manual mode, Mask Input 0,1,2 bit, ignore cutting error check
		{
			if(act_need_reset_flag == TRUE)
			{
				tip_direction = getTipDirection();
				actuator_set_current_position(NULL, CMD2_ACT_MOVE_ORG);
				act_need_reset_flag = FALSE;

				PrintDebug("Actuator reset!\n");
				PrintDebug("enc = %d, avgWidth=%f, cnt=%d\n", cnt_enc, avgWidth/(avgWidthCnt-1), avgWidthCnt-1);

				avgWidthCnt = 1;
				avgWidth = 0;

				enableReadPos(FALSE); //need to call only once per centering process
				sendPlcIO(PLC_WR_RESET);

				rWidth[LPos02] = rWidth[LPos01] = rWidth[RPos01] = rWidth[RPos02] = 0;
				current_section = 0;
			}

			isCPC = FALSE;
			tip_detect = FALSE;
			tip_offset_flag = TRUE;
			rregister_need_read_flag = TRUE;
			setIsCentering(FALSE);

			TASK_Sleep(10);

			continue;
		}

		if(AUTO_MODE) // Auto mode
		{
			if(rregister_need_read_flag == TRUE)
			{
				readRRegister(TRUE);
				rregister_need_read_flag = FALSE;
				enableReadPos(TRUE); //need to call only once per centering process
				current_section = LEADING_TIP_SECTION;
			}
			setIsCentering(TRUE);
			act_need_reset_flag = TRUE;
			RWidth = getRWidth(tip_direction, current_section);
		}

		//printf("%f:%f:%f:%f, PLCIO=%x\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02], (~PLCIO) & 0xF);

		if((rWidth[LPos01] + rWidth[RPos01]) > R.CPCStart)
		{
			isCPC = TRUE;
			current_section = CPC_SECTION;
		}

		/* Check whether if SWidthIn is in boundary of RWidth or not*/
		if(R.SWidthIn > (R.GetSWidth * 40 / 100))
		{
			PrintWarn("R.SWidthIn is too big to control!\n");
		}

		/* Check whether if SWidthOut is in boundary of RWidth or not*/
		if(R.SWidthOut > (R.GetSWidth * 40 / 100))
		{
			PrintWarn("R.SWidthOut is too big to control!\n");
		}

		printf("%f:%f:%f:%f, %f:%d\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02], getRWidth(tip_direction, current_section), current_section);

		if(RWidth >= R.MWidth && !isCPC) //tip detect and tip offset guiding
		{
			if(tip_detect == FALSE)
			{
				PrintDebug("Tip detect~!!\n");
				sendPlcIO(PLC_WR_TIP_DETECT);
				sendPlcIO(PLC_WR_CENTERING);
				tip_detect = TRUE;
				cnt_enc = 0;
				tip_offset_cnt = 0;

				tip_offset_divide(RWidth, leading_tip_width, trailing_tip_width, leading_tip_offset, trailing_tip_offset); // alg1
				current_section = LEADING_TIP_SECTION;
			}

			//leading_tip_guide(RWidth, leading_tip_width, leading_tip_offset); // alg1
			//if(RWidth < leading_tip_width[TIP_OFFSET_DIVIDE_COUNT - 1])
//		printf("%f:%f:%f:%f\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02]);

			if(RWidth < R.SWidthIn)
			{
				current_section = LEADING_TIP_SECTION;
				tipEdgeCentering(TRUE, R.OffsetIn);
			}
			else
			{
				current_section = LEADING_EPC_SECTION;
				tipEdgeCentering(FALSE, R.OffsetIn);
			}
		}
		else if(PLC_RD_CPC) // masking PLC_RD_CPC_TO_EPC
		{
			current_section = CPC_SECTION;
			RWidth = getRWidth(tip_direction, current_section);
			CPCCentering(RWidth);
		}
		else if(PLC_RD_EPC)
		{
			if(RWidth >= R.SWidthOut)
			{
				current_section = TRAILING_EPC_SECTION;
				RWidth = getRWidth(tip_direction, current_section);
				trailingEPCCentering(avgWidth/avgWidthCnt);
			}
			else
			{
				current_section = TRAILING_TIP_SECTION;
				RWidth = getRWidth(tip_direction, current_section);
				trailingTipGuide(R.OffsetOut);
			}
			//trailing_epc_tip_guide(RWidth, avgWidth/avgWidthCnt, trailing_tip_width, trailing_tip_offset); //alg1
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
		PrintError("failed to get R register from PLC\n");
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
		PrintDebug("R.MWidth = %ld\n", R.MWidth);
		PrintDebug("R.SWidthIn = %ld\n", R.SWidthIn);
		PrintDebug("R.OffsetIn= %ld\n", R.OffsetIn);
		PrintDebug("R.GetSWidth= %ld\n", R.GetSWidth);
		PrintDebug("R.TolPos= %ld\n", R.TolPos);
		PrintDebug("R.TolNeg= %ld\n", R.TolNeg);
		PrintDebug("R.SWidthOut= %ld\n", R.SWidthOut);
		PrintDebug("R.OffsetOut= %ld\n", R.OffsetOut);
		PrintDebug("R.POffset= %ld\n", R.POffset);
		PrintDebug("R.MOffset= %ld\n", R.MOffset);
		PrintDebug("R.LeadingOffsetEnable = %ld\n", R.LeadingOffsetEnable);
		PrintDebug("R.TrailingOffsetEnable = %ld\n", R.TrailingOffsetEnable);
		for(i=0; i<7; i++)
		{
			PrintDebug("R.LeadingOffset[%d] = %ld\n", i, R.LeadingOffset[i]);
		}
		for(i=0; i<7; i++)
		{
			PrintDebug("R.TrailingOffset[%d] = %ld\n", i, R.TrailingOffset[i]);
		}
		PrintDebug("mm_per_pulse = %f\n", mm_per_pulse);
		PrintDebug("CPCStart = %f\n", R.CPCStart);
	}
#endif
}

void *encoderTask(void *data)
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
}

int centering_init(void)
{
	struct sockaddr_in server;
	pthread_t centeringTaskId, readPosTaskId, encoderTaskId;

	centering_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (centering_fd == -1)
	{
		PrintError("Could not create socket");
		return -1;
	}

	printf("Centering 0.60\n");

	/* do not remove since CPCStart in centering task should be calculated R.GetSWidth. */
	readRRegister(FALSE);
	actuator_get_status(&ACT);

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
	if (pthread_create( &readPosTaskId, NULL, readPosTask, (void *)NULL ))
	{
		PrintError( "could not create thread for read position from cam %s\n", strerror( errno ));
	}

	return 0;
}
