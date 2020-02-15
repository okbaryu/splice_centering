 /*
 * Filename : osal_stdlib.h
 * Description : osal_stdlib.h file for C header
 */

#ifndef _OSAL_STDLIB_H_
#define _OSAL_STDLIB_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Header Files
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include "datatypes.h"

/*
 * Definitions
 */
/* stdio.h */
#define	OSAL_Scanf			scanf
#define	OSAL_Sscanf			sscanf
#define OSAL_Sprintf		sprintf
#define OSAL_Snprintf		snprintf
#define	OSAL_Vsprintf		vsprintf

/* string.h */
#define OSAL_Memset			memset
#define OSAL_Memcpy			memcpy
#define OSAL_Strncpy		strncpy
#define OSAL_Strcmp			strcmp
#define OSAL_Strncmp		strncmp
#define OSAL_Strlen			strlen
#define OSAL_Strcasecmp		strcasecmp
#define OSAL_Strtok			strtok

/* math.h */
#define	OSAL_Rand			rand
#define	OSAL_Srand			srand
#define	OSAL_Pow			pow
#define OSAL_Sqrt			sqrt

/* ctype.h */
#define OSAL_Isalnum		isalnum
#define OSAL_Isalpha		isalpha
#define	OSAL_Isdigit		isdigit
#define OSAL_Isxdigit		isxdigit
#define OSAL_Islower		islower
#define OSAL_Isupper		isupper
#define	OSAL_Tolower		tolower
#define OSAL_Toupper		toupper

/* stdlib.h */
#define OSAL_Malloc			malloc
#define OSAL_Free			free

/*
 * Data Types
 */

/*
 * Function Prototypes
 */

#if defined(__cplusplus)
}
#endif

#endif /* _OSAL_STDLIB_H_ */

