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
int CMD_PARSER_PI_Centering(void *arg)
{
	GET_ARGS;

	int coeff, ret;

	if (CMD_IS("coeff"))
	{
		iResult = CMD_OK;

		READABLE_IN_DEC(HWTEST_PARAM1, coeff);

		ret = setOffsetCoeff(coeff);
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

