/*
 * Header files
 */
#include <stdio.h>
#include <time.h>
#include "cmdtool.h"
#include "centering_libs.h"
#include "cmd_parser_centering.h"

/*
 * Definitions
 */

/*
 * Data Types
 */

/*
 * Static Variables
 */

/*
 * Prototypes for Static Functions
 */

/*
 * Static Functions
 */

/*
 * Global Functions
 */
int CMD_PARSER_Centering(void *arg)
{
	GET_ARGS;

	int param, ret;

	if (CMD_IS("coeff"))
	{
		iResult = CMD_OK;

		READABLE_IN_DEC(HWTEST_PARAM1, param);

		ret = setOffsetCoeff(param);
		if (ret != PI_OK)
		{
			iResult = CMD_ERR;
		}
		else
		{
			iResult = CMD_OK;
		}
	}
	else if (CMD_IS("CPCRatio"))
	{
		iResult = CMD_OK;

		READABLE_IN_DEC(HWTEST_PARAM1, param);

		ret = setCPCRatio(param);
		if (ret != PI_OK)
		{
			iResult = CMD_ERR;
		}
		else
		{
			iResult = CMD_OK;
		}
	}

	return iResult;
}

int CMD_PARSER_Calibration(void *arg)
{
	GET_ARGS;
	int ret;

	if (CMD_IS("start"))
	{
		calibrationSetMode(MODE_CALIBRATION);

		iResult = CMD_OK;
	}
	if (CMD_IS("end"))
	{
		calibrationSetMode(MODE_RUNNING);

		iResult = CMD_OK;
	}
	else if (CMD_IS("setCam0"))
	{
		iResult = CMD_OK;

		ret = calibrationSetCam(CAM0);
		if(ret < 0)
		{
			iResult = CMD_ERR;
		}
	}
	else if (CMD_IS("setCam1"))
	{
		iResult = CMD_OK;

		ret = calibrationSetCam(CAM1);
		if(ret < 0)
		{
			iResult = CMD_ERR;
		}
	}
	else if (CMD_IS("setCamAll"))
	{
		iResult = CMD_OK;

		ret = calibrationSetCam(CAMALL);
		if(ret < 0)
		{
			iResult = CMD_ERR;
		}
	}
	else if (CMD_IS("save"))
	{
		calibrationSave();

		iResult = CMD_OK;
	}

	return iResult;
}

