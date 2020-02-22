/*
 * Header files
 */
#include <stdio.h>
#include "cmdtool.h"
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
static void CMD_PARSER_PI_Init(void);

/*
 * Static Functions
 */
static void CMD_PARSER_PI_Init(void)
{
	CMD_RegisterWord(CMD_PARSER_PI_Trace,
	/* keyword */		(char_t *)"trace",
	/* help */		(char_t *)"trace [state/on/off] [n]",
	/* usage */ 		(char_t *)"trace state\n"
								"\t trace on [n]\n"
								"\t trace off [n]\n"
								"\t n is 0(ALL), 1(enter/exit), 2(debug), 3(error), 4(warning)\n");

	CMD_RegisterWord(CMD_PARSER_PI_Centering,
	/* keyword */		(char_t *)"centering",
	/* help */			(char_t *)"centering",
	/* usage */ 		(char_t *)"centering\n");
}

/*
 * Global Functions
 */
void CMD_PI_Init(void)
{
	//CMD_RegisterGroup("CENTERING", "CENTERING", "Service commands");
	CMD_PARSER_PI_Init();
	//CMD_ReleaseRegisterGroup();
}

