#ifndef _SYS_TRACE_H_	/* It's an example. It should be changed. */
#define _SYS_TRACE_H_
 
/*
 * Header Files
 */
#include <stdio.h>
#include "datatypes.h"

/*
 * Definitions
 */
#define	ENTER_STRING		"(): ENTER(+)\n"
#define	EXIT_STRING			"(): EXIT(-)\n"

typedef enum
{
	TRACE_ENTER	= 0x00000001,
	TRACE_EXIT	= 0x00000002,
	TRACE_DEBUG	= 0x00000004,
	TRACE_ERR	= 0x00000008,
	TRACE_WARN	= 0x00000010
} TRACE_LEVEL;

#define PrintEnter()	do \
						{ \
							if(SYS_TRACE_CheckLevel(TRACE_ENTER)) \
							{ \
								printf("%c[1;35;40m", 0x1b); \
								printf("%s%s", __func__, ENTER_STRING); \
								printf("\x1b[0m"); \
							} \
						} while (0)

#define PrintExit()		do \
						{ \
							if(SYS_TRACE_CheckLevel(TRACE_EXIT)) \
							{ \
								printf("%c[1;35;40m", 0x1b); \
								printf("%s%s", __func__, EXIT_STRING); \
								printf("\x1b[0m"); \
							} \
						} while (0)

#define PrintDebug(X, ...)	do \
							{ \
								if(SYS_TRACE_CheckLevel(TRACE_DEBUG)) \
								{ \
									printf("%c[1;37;40m", 0x1b); \
									printf("[%s:%d] ", __func__, __LINE__); \
									printf(X, ##__VA_ARGS__); \
									printf("\x1b[0m"); \
								} \
							} while (0)

#define PrintError(X, ...)	do \
							{ \
								if(SYS_TRACE_CheckLevel(TRACE_ERR)) \
								{ \
									printf("%c[1;31;40m", 0x1b); \
									printf("[%s:%d] ", __func__, __LINE__); \
									printf(X, ##__VA_ARGS__); \
									printf("\x1b[0m"); \
								} \
							} while (0)

#define PrintWarn(X, ...)	do \
							{ \
								if(SYS_TRACE_CheckLevel(TRACE_WARN)) \
								{ \
									printf("%c[1;33;40m", 0x1b); \
									printf("[%s:%d] ", __func__, __LINE__); \
									printf(X, ##__VA_ARGS__); \
									printf("\x1b[0m"); \
								} \
							} while (0)

/*
 * Data Types
 */

/*
 * Function Prototypes
 */
uint_t SYS_TRACE_CheckLevel(TRACE_LEVEL param);
uint_t SYS_TRACE_GetState(void);
void SYS_TRACE_EnableLevel(uint_t param);
void SYS_TRACE_DisableLevel(uint_t param);

#endif
