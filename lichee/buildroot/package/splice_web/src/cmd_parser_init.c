/*
 * Header files
 */
#include <stdio.h>
#include "cmdtool.h"
#include "cmd_parser_init.h"
#include "cmd_parser_trace.h"
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
static void CMD_PARSER_Init(void);

/*
 * Static Functions
 */
static void CMD_PARSER_Init(void)
{
	CMD_RegisterWord(CMD_PARSER_Trace,
	/* keyword */		(char_t *)"trace",
	/* help */		(char_t *)"trace [state/on/off] [n]",
	/* usage */ 		(char_t *)"trace state\n"
								"\t trace on [n]\n"
								"\t trace off [n]\n"
								"\t n is 0(ALL), 1(enter/exit), 2(debug), 3(error), 4(warning)\n");

	CMD_RegisterWord(CMD_PARSER_Centering,
	/* keyword */		(char_t *)"centering",
	/* help */			(char_t *)"centering",
	/* usage */ 		(char_t *)"centering\n");

	CMD_RegisterWord(CMD_PARSER_Calibration,
	/* keyword */		(char_t *)"cali",
	/* help */			(char_t *)"cali [start/end/setCam0/setCam1/setCamAll/save]",
	/* usage */ 		(char_t *)"cali start\n"
								"\t cali setCam0\n"
								"\t cali save\n"
								"\t cali setCam1\n"
								"\t cali save\n"
								"\t cali setCamAll\n"
								"\t cali end\n");
}

/*
 * Global Functions
 */
void CMD_HS_Init(void)
{
	//CMD_RegisterGroup("CENTERING", "CENTERING", "Service commands");
	CMD_PARSER_Init();
	//CMD_ReleaseRegisterGroup();
}

