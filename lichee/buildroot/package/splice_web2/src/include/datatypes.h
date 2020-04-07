/*
 * Filename : datatypes.h
 * Description : Global datatypes
 */

#ifndef _DATATYPES_H_
#define _DATATYPES_H_

/*
 * Header Files
 */
#include "errortypes.h"
#include <stdint.h>

/*
 * Definitions
 */
#if !defined(FALSE)
#define FALSE       		(0)
#endif

#if !defined(TRUE)
#define TRUE        		(1)
#endif

#if !defined(NULL)
#define	NULL				(0)
#endif

#define __FUNCTION__	__func__

#define	PI_OK				(0)

/*
 * Data Types
 */

typedef char			char_t;
typedef unsigned int	uint_t;
typedef signed int		int_t;
typedef unsigned int   bool_t;
/*
 * Function Prototypes
 */


#endif
