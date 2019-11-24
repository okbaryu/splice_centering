/*
* Filename : template.h
* Description : Template file for C header
*/
#ifndef _OSAL_MSG_H_
#define _OSAL_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * Header Files
 */
#include "datatypes.h"
 
/*
 * Definitions
 */

/*
 * Data Types
 */

/*
 * Function Prototypes
 */

int OSAL_MSG_Init(void);
int OSAL_MSG_Create(unsigned long q_count, unsigned long msg_size, const char *q_name, unsigned long *q_id);
int OSAL_MSG_Destroy(unsigned long q_id);
int OSAL_MSG_Clear(unsigned long q_id);
int OSAL_MSG_Send(unsigned long q_id, const void *msg, unsigned long size);
int OSAL_MSG_SendTimeout(unsigned long q_id, const void *msg, unsigned long size, unsigned long timeout);
int OSAL_MSG_Receive(unsigned long q_id, void *msg, unsigned long size);
int OSAL_MSG_ReceiveTimeout(unsigned long q_id, void *msg, unsigned long size, unsigned long timeout);

#ifdef __cplusplus
}
#endif

#endif

