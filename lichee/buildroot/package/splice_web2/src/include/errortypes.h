 /*
 * Filename : errortypes.h
 * Description : errortypes file for C header
 */

#ifndef _ERROR_TYPES_H_
#define _ERROR_TYPES_H_

/*
 * Header Files
 */

/* CID 25428 (#1 of 1): Recursion in included headers (PW.INCLUDE_RECURSION)
 *    1. include_recursion: #include file "/work/coverity/S500-Mid-Head-Unit/source/service/ha/include/datatypes.h" includes itself: datatypes.h -> errortypes.h -> datatypes.h
 *    2. primary_file: During compilation of file '/work/coverity/S500-Mid-Head-Unit/source/service/ha/apps/test_app.c'
 */
/* #include "datatypes.h" */


/*
 * Definitions
 */
#define ERR_OK						(0)

/* Common errors (-1 ~ -50) */
#define ERR_ERROR					(-1)
#define ERR_INVALID_PARAM			(-2)
#define ERR_BUSY					(-3)
#define ERR_OUT_OF_MEMORY			(-4)
#define ERR_FEATURE_NOT_SUPPORTED	(-5)
#define ERR_TIMEOUT					(-6)

/*
 * Data Types
 */
typedef int ErrorType;

#endif

