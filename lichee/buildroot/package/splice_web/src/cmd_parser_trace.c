/*
 * Header files
 */
#include <stdio.h>
#include "cmdtool.h"
#include "sys_trace.h"

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
int CMD_PARSER_PI_Trace(void *arg)
{
	GET_ARGS;
	uint_t level = 0;
	
	if (CMD_IS("state"))
	{
		printf("gTraceLevel (0x%X)\n", SYS_TRACE_GetState());
		PrintEnter();
		PrintExit();
		PrintDebug("PrintDebug\n");
		PrintError("PrintError\n");
		PrintWarn("PrintWarn\n");
		iResult = CMD_OK;
	}

	if (CMD_IS("on"))
	{
		READABLE_IN_HEX(HWTEST_PARAM1, level);

		switch(level)
		{
			case 0:
				SYS_TRACE_EnableLevel(TRACE_ENTER | TRACE_EXIT | TRACE_DEBUG | TRACE_ERR | TRACE_WARN);
				break;
				
			case 1:
				SYS_TRACE_EnableLevel(TRACE_ENTER | TRACE_EXIT);
				break;

			case 2:
				SYS_TRACE_EnableLevel(TRACE_DEBUG);
				break;

			case 3:
				SYS_TRACE_EnableLevel(TRACE_ERR);
				break;

			case 4:
				SYS_TRACE_EnableLevel(TRACE_WARN);
				break;

			default:
				return CMD_ERR;
		}
		PrintEnter();
		PrintExit();
		PrintDebug("PrintDebug\n");
		PrintError("PrintError\n");
		PrintWarn("PrintWarn\n");

		iResult = CMD_OK;
	}

	if (CMD_IS("off"))
	{
		READABLE_IN_HEX(HWTEST_PARAM1, level);

		switch(level)
		{
			case 0:
				SYS_TRACE_DisableLevel(0);
				break;
				
			case 1:
				SYS_TRACE_DisableLevel(TRACE_ENTER | TRACE_EXIT);
				break;

			case 2:
				SYS_TRACE_DisableLevel(TRACE_DEBUG);
				break;

			case 3:
				SYS_TRACE_DisableLevel(TRACE_ERR);
				break;

			case 4:
				SYS_TRACE_DisableLevel(TRACE_WARN);
				break;
				
			default:
				return CMD_ERR;
				
		}
		PrintEnter();
		PrintExit();
		PrintDebug("PrintDebug\n");
		PrintError("PrintError\n");
		PrintWarn("PrintWarn\n");
		
		iResult = CMD_OK;
	}

	return iResult;
}

