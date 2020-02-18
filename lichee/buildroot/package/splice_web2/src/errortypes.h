/*
 * (c) 2006-2014 Humax Automotive Co., Ltd.
 * This program is produced by Humax Automotive Co., Ltd. ("Humax Automotive") and
 * the proprietary Software of Humax Automotive and its licensors. Humax Automotive provides you, as an Authorized Licensee,
 * non-assignable, non-transferable and non-exclusive license to use this Software.
 * You acknowledge that this Software contains valuable trade secrets of Humax Automotive and by using this Software
 * you agree to the responsibility to take all reasonable efforts to protect the any information
 * you receive from Humax Automotive. You are not permitted to duplicate, modify, distribute, sell or lease and
 * reverse engineer or extract the source code of this Software unless you have Humax Automotive's written permission to do so.
 * If you have no authorized license, discontinue using this Software immediately.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND Humax Automotive MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS,
 * IMPLIED OR STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
 * IN NO EVENT SHALL Humax Automotive BE LIABLE FOR LOST PROFITS, REVENUES, OR DATA, FINANCIAL LOSSES OR INDIRECT, SPECIAL,
 * CONSEQUENTIAL, EXEMPLARTY OR PUNITIVE DAMAGES WHATSOEVER RELATING TO YOUR USE OR INABILITY TO USE THE SOFTWARE.

 * This License is effective until terminated. You may terminate this License at any time by destroying all copies
 * of the Software including all documentation. This License will terminate immediately without notice from Humax Automotive
 * to you if you fail to comply with any provision of this License. Upon termination, you must destroy all copies
 * of the Software and all documentation.

 * The laws of the Republic of Korea will apply to any disputes arising out of or relating to this Copyright Notice.
 * All claims arising out of or relating to this Copyright Notice will be litigated in the Seoul Central District Court,
 * in the Republic of Korea.
 */

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

