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

#include "datatypes.h"
#include "splice_libs.h"
#include "splice_utils.h"
#include "actuator.h"
#include "centering.h"
#include "centering_libs.h"
#include "plc.h"
#include "sys_trace.h"

#define LPos02_R_FLAG 0x1
#define LPos01_R_FLAG 0x2
#define RPos01_R_FLAG 0x4
#define RPos02_R_FLAG 0x8

#define AUTO_MODE !(PLCIO & 0x1)
#define MANUAL_MODE ((PLCIO & 0x7) == 0x7)
#define PLC_RD_WIDTH ((PLCIO & 0x7) == 0x2)
#define PLC_RD_EPC (!(PLCIO & 0x2))
#define PLC_RD_CPC (isCPC && (PLCIO & 0x2))

#define WIDTH_ERR_CNT_THRESHOLD 100 // depends on CPCRatio
//#define VIEW_ENCODER_CNT

extern unsigned char PLCIO;
extern float rWidth[4];
static RRegister R;
static int ltip_offset_cnt, ttip_offset_cnt, tip_offset_flag;
static char tip_direction;
static char current_section;
static char width_err_cnt;
static int RWidth_FLAG;

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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
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
				sendPlcError(PLC_ERR_DUST_DETECT, 0);
			}
		}
	}
	else
	{
		PrintError("Unrecognized tip direction %d\n", tip_direction);
	}

	return -1;
}

void do_centering(char section, float pos_diff)
{
	int act_pos;

	switch(section)
	{
		case LEADING_TIP_SECTION :
		case LEADING_EPC_SECTION :
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
			break;

		case CPC_SECTION :
			if(pos_diff > 5 || pos_diff < -5)
			{
				act_pos = ACT_MOVE_1MM / 2;
			}
			else if(pos_diff > 3 || pos_diff < -3)
			{
				act_pos = ACT_MOVE_1MM / 4;
			}
			else
			{
				act_pos = ACT_MOVE_1MM / 10;
			}
			break;

		case TRAILING_EPC_SECTION :
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
			break;

		case TRAILING_TIP_SECTION :
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
			break;

		default :
			PrintWarn("Unrecognized section %d\n", section);
			break;
	}

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

void tipEdgeCentering(char tip_edge_guide, long OffsetIn)
{
	float pos_diff, ref_pos, ref_width;
	int act_pos;

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
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
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
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
		}

	}

	do_centering(current_section, pos_diff);
}

void CPCCentering(float RWidth)
{
	do_centering(current_section, RWidth);
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
		}
		else if((RWidth_FLAG & RPos01_R_FLAG) && (RWidth_FLAG & RPos02_R_FLAG))
		{
			diff = rWidth[RPos02] - (avgWidth/2);
		}
		else
		{
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
		}
	}
	else
	{
		if(rWidth[LPos02] == 0 && rWidth[LPos01] > 0 && rWidth[RPos01] > 0 && rWidth[RPos02] == 0)
		{
			diff = rWidth[LPos01] - (avgWidth/2);
		}
		else if(rWidth[LPos02] > 0 && rWidth[LPos01] > 0 && rWidth[RPos01] == 0 && rWidth[RPos02] == 0)
		{
			diff = rWidth[LPos02] - (avgWidth/2);
		}
		else
		{
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
		}
	}

	//printf("%f, %f:%d\n", diff, getRWidth(tip_direction, current_section), current_section);

	/* trailing diff value should multiple by -1 to make opposite actuator operation */
	do_centering(current_section, diff * -1);
}

void trailingTipGuide(long OffsetOut)
{
	float pos_diff, ref_pos, ref_width;
	int act_pos;

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
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
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
<<<<<<< HEAD
			// PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
=======
			 PrintError("Abnormal RWidth_FLAG 0x%x\n", RWidth_FLAG);
>>>>>>> 049f8372e2bb5db237e1f6aba025a4c9841573f0
		}
	}

	//printf("%f, %f:%d\n", pos_diff, getRWidth(tip_direction, current_section), current_section);
	do_centering(current_section, pos_diff);
}

static void tip_offset_divide(float RWidth, float *leading_tip_width, float *trailing_tip_width, int *leading_tip_offset, int *trailing_tip_offset)
{
	int i;
	float divide_factor = (float)(R.GetSWidth * getCPCRatio() / 100) / TIP_OFFSET_DIVIDE_COUNT;

	for(i=0; i<TIP_OFFSET_DIVIDE_COUNT; i++)
	{
		/* Divide leading tip offset area by TIP_OFFSET_DIVIDE_COUNT */
		leading_tip_width[i] = (i+1) * divide_factor;
		leading_tip_offset[i] = R.LeadingOffset[i];
		/* Divide trailing tip offset area by TIP_OFFSET_DIVIDE_COUNT */
		trailing_tip_width[i] = R.GetSWidth - ((i+1) * divide_factor);
		trailing_tip_offset[i] = R.TrailingOffset[i];

		PrintDebug("tip width %d : [%f][%f]\n", i, leading_tip_width[i], trailing_tip_width[i]);
	}
}

static void leading_tip_guide(float RWidth, float *leading_tip_width, int *leading_tip_offset)
{
	if(ltip_offset_cnt < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
	{
		if(leading_tip_width[ltip_offset_cnt] > RWidth)
		{
			if(tip_direction == TIP_LEFT && tip_offset_flag)
			{
				if(leading_tip_offset[ltip_offset_cnt] > 0)
					act_move(ACT_MOVE_LEFT, leading_tip_offset[ltip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[ltip_offset_cnt] * -1 * ACT_MOVE_1MM_HALF);

				if(ltip_offset_cnt == 0) tip_offset_flag = FALSE;
				//PrintDebug("ltol %d, %f:%d\n", ltip_offset_cnt, leading_tip_width[ltip_offset_cnt], leading_tip_offset[ltip_offset_cnt]);
			}
			else if(tip_direction == TIP_RIGHT && tip_offset_flag)
			{
				if(leading_tip_offset[ltip_offset_cnt] > 0)
					act_move(ACT_MOVE_RIGHT, leading_tip_offset[ltip_offset_cnt] * ACT_MOVE_1MM_HALF);
				else
					act_move(ACT_MOVE_LEFT, leading_tip_offset[ltip_offset_cnt] * -1 * ACT_MOVE_1MM_HALF);

				if(ltip_offset_cnt == 0) tip_offset_flag = FALSE;
				//PrintDebug("ltor %d, %f:%d\n", ltip_offset_cnt, leading_tip_width[ltip_offset_cnt], leading_tip_offset[ltip_offset_cnt]);
			}
		}
		else
		{
			ltip_offset_cnt++;
			tip_offset_flag = TRUE; //Used for move actuator once when section is in leading_tip_width[0]
		}
	}
}

static void trailing_tip_guide(float RWidth, float *trailing_tip_width, int *trailing_tip_offset)
{
	if(ttip_offset_cnt+1 < TIP_OFFSET_DIVIDE_COUNT) //leading tip offset guide
	{
		if(RWidth < trailing_tip_width[ttip_offset_cnt] && RWidth > trailing_tip_width[ttip_offset_cnt + 1])
		{
			if(tip_direction == TIP_LEFT)
			{
				if(trailing_tip_offset[ttip_offset_cnt] > 0)
					act_move(ACT_MOVE_LEFT, trailing_tip_offset[ttip_offset_cnt] * ACT_MOVE_1MM);
				else
					act_move(ACT_MOVE_RIGHT, trailing_tip_offset[ttip_offset_cnt] * -1 * ACT_MOVE_1MM);

				//PrintDebug("ttol %d, %f:%f\n", ttip_offset_cnt, trailing_tip_width[ttip_offset_cnt], RWidth);
			}
			else if(tip_direction == TIP_RIGHT)
			{
				if(trailing_tip_offset[ttip_offset_cnt] > 0)
					act_move(ACT_MOVE_RIGHT, trailing_tip_offset[ttip_offset_cnt] * ACT_MOVE_1MM);
				else
					act_move(ACT_MOVE_LEFT, trailing_tip_offset[ttip_offset_cnt] * -1 * ACT_MOVE_1MM);

				//PrintDebug("ttor %d, %f:%f\n", ttip_offset_cnt, trailing_tip_width[ttip_offset_cnt], RWidth);
			}
		}
		else
		{
			ttip_offset_cnt++;
		}
	}
}

void *centeringTask(void *data)
{
	float RWidth, avgWidth = 0;
	char tip_detect = 0, act_need_reset_flag = TRUE, rregister_need_read_flag = TRUE, flag = TRUE;
	int isCPC = FALSE, avgWidthCnt = 1, leading_alg, trailing_alg;
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
				PrintDebug("enc = %d, Trail EPC avgWidth=%f, width_err_cnt=%d\n", getEncoderCnt(), avgWidth/(avgWidthCnt-1), width_err_cnt);

				avgWidthCnt = 1;
				avgWidth = 0;
				flag = TRUE;

				enableReadPos(FALSE); //need to call only once per centering process
				sendPlcIO(PLC_WR_RESET);

				rWidth[LPos02] = rWidth[LPos01] = rWidth[RPos01] = rWidth[RPos02] = 0;

				current_section = 0;
				width_err_cnt = 0;

				saveProfile();
				resetProfile();
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
				readRRegister(TRUE, &R);
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

		//printf("%f:%f:%f:%f, %f:%d\n", rWidth[LPos02], rWidth[LPos01], rWidth[RPos01], rWidth[RPos02], getRWidth(tip_direction, current_section), current_section);

		if(RWidth >= R.MWidth && !isCPC) //tip detect and tip offset guiding
		{
			if(tip_detect == FALSE)
			{
				PrintDebug("Tip detect~!!\n");
				sendPlcIO(PLC_WR_TIP_DETECT);
				sendPlcIO(PLC_WR_CENTERING);
				tip_detect = TRUE;
				resetEncoder();
				ltip_offset_cnt = 0;
				ttip_offset_cnt = 0;
				current_section = LEADING_TIP_SECTION;

				leading_alg = getAlgorithm(LEADING_TIP_SECTION);
				trailing_alg = getAlgorithm(TRAILING_TIP_SECTION);

				if(leading_alg > ALGORITHM1 || trailing_alg > ALGORITHM1)
				{
					tip_offset_divide(RWidth, leading_tip_width, trailing_tip_width, leading_tip_offset, trailing_tip_offset);
				}
			}

			if(leading_alg == ALGORITHM2)
			{
				current_section = LEADING_TIP_SECTION;
				RWidth = getRWidth(tip_direction, current_section);
				leading_tip_guide(RWidth, leading_tip_width, leading_tip_offset);

				leadingOffsetProfile(RWidth, leading_tip_width);
			}
			else if(leading_alg == ALGORITHM3)
			{
				leadingOffsetProfile(RWidth, leading_tip_width);
			}
			else // ALGORITHM1
			{
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
		}
		else if(PLC_RD_CPC) // masking PLC_RD_CPC_TO_EPC
		{
			current_section = CPC_SECTION;
			RWidth = getRWidth(tip_direction, current_section);

			if(width_check(RWidth, &R))
			{
				width_err_cnt++;
			}

			if(width_err_cnt > WIDTH_ERR_CNT_THRESHOLD)
			{
				PrintError("Width Error %f\n", RWidth);
				sendPlcError(PLC_ERR_WIDTH_ERROR, RWidth);
				width_err_cnt = 0;
			}

			CPCCentering(RWidth);
		}
		else if(PLC_RD_EPC)
		{
			RWidth = getRWidth(tip_direction, TRAILING_EPC_SECTION);

			if(trailing_alg == ALGORITHM2)
			{
				current_section = TRAILING_TIP_SECTION;
				RWidth = getRWidth(tip_direction, current_section);
				if(RWidth < trailing_tip_width[0])
				{
					trailing_tip_guide(RWidth, trailing_tip_width, trailing_tip_offset);

					trailingOffsetProfile(RWidth, trailing_tip_width);
				}
				else
				{
					/* Since EPC transfer signal caught, it needs some time to make decision if RWidth
					 * is smaller than trailing_tip_width[0] or not. If this time is too long, it needs
					 * tune EPC transfer timing in PLC */
				}

			}
			else if(trailing_alg == ALGORITHM3)
			{
				if(flag)
				{
					calLeadingProfile(trailing_tip_offset);
					flag = FALSE;
				}

				current_section = TRAILING_TIP_SECTION;
				RWidth = getRWidth(tip_direction, current_section);
				if(RWidth < trailing_tip_width[0])
				{
					trailing_tip_guide(RWidth, trailing_tip_width, trailing_tip_offset);

					trailingOffsetProfile(RWidth, trailing_tip_width);
				}
			}
			else // ALGORITHM 1
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
			}
		}

		if(PLC_RD_WIDTH)
		{
			avgWidth += (rWidth[LPos01] + rWidth[RPos01]);
			avgWidthCnt++;
		}

		wholeAreaProfile(current_section, RWidth, rWidth);

		TASK_Sleep(10);
	}
}

int centering_init(void)
{
	pthread_t centeringTaskId;

	printf("Centering 0.71\n");

	/* do not remove since CPCStart in centering task should be calculated R.GetSWidth. */
	readRRegister(FALSE, &R);

	centering_libs_init();

	if (pthread_create( &centeringTaskId, NULL, centeringTask, (void *)NULL ))
	{
		PrintError( "could not create thread for centering %s\n", strerror( errno ));
	}

	return 0;
}
