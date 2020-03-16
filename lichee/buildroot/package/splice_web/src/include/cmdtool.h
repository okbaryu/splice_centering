#ifndef _CMD_TOOL_H_	/* It's an example. It should be changed. */
#define _CMD_TOOL_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Header Files
 */
#include "datatypes.h"
#include "osal_stdlib.h"

/*
 * Definitions
 */

/* return value of command function */
#define CMD_OK					0
#define CMD_ERR					1

/* for keyword structure */
#define	MAX_CMD_GROUP			3
#define MAX_CMD_WORD			(128 * 2)
#define MAX_WORD_LEN			16
#define MAX_PROMPT_LEN 			64
#define MAX_HELP_LEN			(512 * 2)
#define MAX_USAGE_LEN			(512 * 2)

#define	PARAM_NOT_USED(x)			(x = x)

#define HWTEST_CMD					szCmd
#define HWTEST_PARAM1				szParam
#define HWTEST_PARAM2				szParam1
#define HWTEST_PARAM3				szParam2
#define HWTEST_PARAM4				szParam3
#define HWTEST_PARAM5				szParam4
#define GET_ARGS					int iResult=CMD_ERR; \
									char_t *HWTEST_CMD=CMD_NextArg((char_t **)&arg); \
									char_t *HWTEST_PARAM1=CMD_NextArg((char_t **)&arg); \
									char_t *HWTEST_PARAM2=CMD_NextArg((char_t **)&arg); \
									char_t *HWTEST_PARAM3=CMD_NextArg((char_t **)&arg); \
									char_t *HWTEST_PARAM4=CMD_NextArg((char_t **)&arg); \
									char_t *HWTEST_PARAM5=CMD_NextArg((char_t **)&arg);	\
									PARAM_NOT_USED(HWTEST_PARAM1);	\
									PARAM_NOT_USED(HWTEST_PARAM2);	\
									PARAM_NOT_USED(HWTEST_PARAM3);	\
									PARAM_NOT_USED(HWTEST_PARAM4);	\
									PARAM_NOT_USED(HWTEST_PARAM5);
						
#define CMD_IS(x)					(HWTEST_CMD!=NULL && OSAL_Strcmp(HWTEST_CMD, x) == 0)
#define PARAM_1_IS(x)				(HWTEST_PARAM1!=NULL && OSAL_Strcmp(HWTEST_PARAM1, x) == 0)
#define PARAM_2_IS(x)				(HWTEST_PARAM2!=NULL && OSAL_Strcmp(HWTEST_PARAM2, x) == 0)
#define PARAM_3_IS(x)				(HWTEST_PARAM3!=NULL && OSAL_Strcmp(HWTEST_PARAM3, x )== 0)
#define PARAM_4_IS(x)				(HWTEST_PARAM4!=NULL && OSAL_Strcmp(HWTEST_PARAM4, x )== 0)
#define PARAM_5_IS(x)				(HWTEST_PARAM5!=NULL && OSAL_Strcmp(HWTEST_PARAM5, x )== 0)
#define READABLE_IN_HEX(sz,val)		CMD_ReadAsHex(sz, &val)
#define READABLE_IN_DEC(sz,val)		CMD_ReadAsDec(sz, &val)
	
#define GET_NEXT_ARG(variable)		char_t *variable = CMD_NextArg((char_t **)&arg)



/* command category structure */
typedef	struct tagCmdGroup
{
	int_t	nGroupId;
	char_t	szGroupName[MAX_WORD_LEN];
	char_t	szPromptName[MAX_PROMPT_LEN];
/* CID 20931 (#1 of 1): Buffer not null terminated (BUFFER_SIZE_WARNING)
 *      6. buffer_size_warning: Calling strncpy with a maximum size argument of 1024 bytes on destination array 
 * pstCmdGroup->szHelp of size 1024 bytes might leave the destination string unterminated
 */	
	char_t	szHelp[MAX_HELP_LEN+1];

	struct tagCmdGroup	*pstParentGroup;
} CMDGROUP_WORD;

/* command structure using keyword typing */
typedef struct {
	char_t	szWord[MAX_WORD_LEN];		/* keyword */
/* CID 20931 (#1 of 1): Buffer not null terminated (BUFFER_SIZE_WARNING)
 *      6. buffer_size_warning: Calling strncpy with a maximum size argument of 1024 bytes on destination array 
 * pstCmdGroup->szHelp of size 1024 bytes might leave the destination string unterminated
 */	
	char_t	szHelp[MAX_HELP_LEN+1];		/* help string */
	char_t	szUsage[MAX_USAGE_LEN];	/* usage string */
	int_t	(*fnToDo)(void *arg);			/* function pointer */

	//	group
	CMDGROUP_WORD	*pstGroupPtr;
} CMD_WORD, *pCMD_WORD;


/* command structure using key press */
typedef struct {
	uint8_t		ucKeyCode;					/* key code */
/* CID 20931 (#1 of 1): Buffer not null terminated (BUFFER_SIZE_WARNING)
 *      6. buffer_size_warning: Calling strncpy with a maximum size argument of 1024 bytes on destination array 
 * pstCmdGroup->szHelp of size 1024 bytes might leave the destination string unterminated
 */	
	char_t		szHelp[MAX_HELP_LEN+1];		/* help string */
	char_t		szUsage[MAX_USAGE_LEN];	/* usage string */
	void		(*fnToDo)(void);				/* function pointer */
} CMD_KEY, *pCMD_KEY;

/*
 * Data Types
 */

/*
 * Function Prototypes
 */
void CMD_Init(void);
void CMD_Deinit(void);
void CMD_RegisterGroup(char_t *szGroupName, char_t *szPrompt, char_t *szHelp);
void CMD_ReleaseRegisterGroup(void);
void CMD_RegisterWord(int_t (*fnToDo)(void *arg), char_t *szWord, char_t *szHelp, char_t *szUsage);
void CMD_SetCurrentGroup(char_t *szGroupName);
void CMD_ShowPrompt(void);

char_t *CMD_NextArg(char_t **szLine);
int_t CMD_ReadAsDec(char_t *pString, int_t *puiDec_value);
int_t CMD_ReadAsHex(char_t *pString, uint_t *puiHex_value);

#if defined(__cplusplus)
}
#endif

#endif
