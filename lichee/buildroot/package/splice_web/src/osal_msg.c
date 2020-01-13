/*
 * Filename : template.c
 * Description : Template file for C code
 */

/*
 * Header files
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <sys/queue.h>
#include <unistd.h>
#include <time.h>
#include "osal_msg.h"

/*
 * Definitions
 */
#define list_size(list) ((list)->curr_msg_count_)

#define list_bitmap_size(list, i) ((list)->bitmap_count[(i)])

#define list_head(list, i) ((list)->head[i])

#define list_tail(list, i) ((list)->tail[i])

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)

#define list_data(element) ((element)->data)

#define list_next(element) ((element)->next)

/*
 * Data Types
 */
struct  osal_msg_element
{
	void             *data;
	struct osal_msg_element  *next;
	struct osal_msg_element  *prev;
};

typedef enum
{
	SUSPENDTYPE_FIFO,
	SUSPENDTYPE_PRIORITY,
	SUSPENDTYPE_MAX
}SUSPEND_TYPE;

typedef enum
{
	MSG_PRIORITY0,
	MSG_PRIORITY1,
	MSG_PRIORITY2,
	MSG_PRIORITY3,
	MSG_PRIORITY4,
	MSG_PRIORITY5,
	MSG_PRIORITY6,
	MSG_PRIORITY7,
	MSG_PRIORITY8,
	MSG_PRIORITY9,
	MSG_PRIORITY10,
	MSG_PRIORITY11,
	MSG_PRIORITY12,
	MSG_PRIORITY13,
	MSG_PRIORITY14,
	MSG_PRIORITY15,
	MSG_PRIORITY16,
	MSG_PRIORITY_MAX
}OSAL_MSG_PRIORITY;

struct osal_msg_queue
{
	char name[32];

	pthread_mutex_t     q_lock_;
	pthread_cond_t      q_not_empty_;
	pthread_cond_t      q_not_full_;
	unsigned long       curr_msg_count_;
	unsigned long       msg_size_;
	unsigned long       msg_count_;
	unsigned long       bitmap_count[MSG_PRIORITY_MAX];
	unsigned char       bitmap[MSG_PRIORITY_MAX];
	unsigned char       suspend_type_;
	void                *pPool;
	void                *pDataPool;

	int writer_count_;
	int reader_count_;

	struct osal_msg_element          *temp_head;
	struct osal_msg_element          *temp_tail;
	struct osal_msg_element          *head[MSG_PRIORITY_MAX];
	struct osal_msg_element          *tail[MSG_PRIORITY_MAX];

};

/*
 * Static Variables
 */

/*
 * Prototypes for Static Functions
 */
static void list_init(struct osal_msg_queue *list, unsigned long ulSize, unsigned long ulCount, unsigned char ucSuspendType);

static void list_destroy(struct osal_msg_queue *list);

int list_ins_next(struct osal_msg_queue *list, unsigned long prio, const void *data, unsigned long size, unsigned char timeoutflag, struct timespec *timeo);

int list_rem_next(struct osal_msg_queue *list, void *data, unsigned long size, unsigned char timeoutflag, struct timespec *timeo);

void list_set_bitmap(struct osal_msg_queue *list, unsigned long prio);

void list_clear_bitmap(struct osal_msg_queue *list, unsigned long prio);

unsigned long list_find_highest_priority_bitmap(struct osal_msg_queue *list);

void list_bitmap_count_increase(struct osal_msg_queue *list, unsigned long prio);

void list_bitmap_count_decrease(struct osal_msg_queue *list, unsigned long prio);

void q_insert_temp_tail_element(struct osal_msg_queue *q, struct osal_msg_element *element);

void q_remove_temp_tail_element(struct osal_msg_queue *q, 	struct osal_msg_element **element);

void q_destory_temp_tail(struct osal_msg_queue *q);

static int P_MSG_Check(struct osal_msg_queue *q, unsigned long q_id);

/*
 * Static Functions
 */
static void list_init(struct osal_msg_queue *list, unsigned long ulSize, unsigned long ulCount, unsigned char ucSuspendType)
{
	int i;

	list->curr_msg_count_ = 0;
	list->msg_size_ = ulSize;
	list->msg_count_ = ulCount;
	list->writer_count_ = 0;
	list->reader_count_ = 0;
	list->suspend_type_ = ucSuspendType;
	pthread_mutex_init(&list->q_lock_, NULL);
	pthread_cond_init(&list->q_not_empty_, NULL);
	pthread_cond_init(&list->q_not_full_, NULL);

	list->temp_head = NULL;
	list->temp_tail = NULL;

	for(i=0; i<MSG_PRIORITY_MAX; i++)
	{
		list->head[i] = NULL;
		list->tail[i] = NULL;
		list->bitmap_count[i] = 0;
		list->bitmap[i] = 0;
	}

	return;
}

static void list_destroy(struct osal_msg_queue *list)
{
	void               *data;

	data = (char *)malloc(list->msg_size_);
	if (data == NULL)
	{
		return;
	}

	while (list_size(list) > 0)
	{
		if ( list_rem_next(list, (void *)data, list->msg_size_, FALSE, FALSE) != 0 )
		{
			printf("list_rem_next Error!!\n");
		}
	}

	free(data);
	return;
}

int list_ins_next(struct osal_msg_queue *list, unsigned long prio, const void *data, unsigned long size, unsigned char timeoutflag, struct timespec *timeo)
{
	struct osal_msg_element           *new_element;
	int rc = 0;

	list->writer_count_++;

	while (list->msg_count_ == list->curr_msg_count_ && 0 == rc)
	{
		if(TRUE == timeoutflag)
		{
			rc = pthread_cond_timedwait(&list->q_not_full_, &list->q_lock_, timeo);
		}
		else
		{
			rc = pthread_cond_wait(&list->q_not_full_, &list->q_lock_);
		}
	}

	list->writer_count_--;

	if(rc != 0)
	{
		printf("rc[%d] !!\n", rc);
		return rc;
	}

	q_remove_temp_tail_element(list, &new_element);

	memcpy(new_element->data, data, size);

	if(list->tail[prio] == NULL)
	{
		list->head[prio] = new_element;
		new_element->next = NULL;
		new_element->prev = NULL;
		list->tail[prio] = new_element;
	}
	else
	{
		list->tail[prio]->next = new_element;
		new_element->next = NULL;
		new_element->prev = list->tail[prio];
		list->tail[prio] = new_element;
	}

	list->curr_msg_count_++;
	list_bitmap_count_increase(list, prio);
	list_set_bitmap(list, prio);

	if (list->reader_count_ > 0)
	{
		pthread_cond_signal(&list->q_not_empty_);
	}

	return 0;
}

int list_rem_next(struct osal_msg_queue *list, void *data, unsigned long size, unsigned char timeoutflag, struct timespec *timeo)
{
	struct osal_msg_element           *old_element;
	unsigned long prio;
	int rc = 0;

	list->reader_count_++;

	if ((list_size(list) == 0 || list->reader_count_ > 1) && 0 == rc)
	{
		if(TRUE == timeoutflag)
		{
			rc = pthread_cond_timedwait(&list->q_not_empty_, &list->q_lock_, timeo);
		}
		else
		{
			rc = pthread_cond_wait(&list->q_not_empty_, &list->q_lock_);
		}
	}

	list->reader_count_--;

	if (rc != 0)
	{
		//		printf("rc[%d] !!\n", rc);
		return rc;
	}

	prio = list_find_highest_priority_bitmap(list);

	memcpy(data, list->head[prio]->data, size);
	old_element = list->head[prio];
	if(NULL == list->head[prio]->next)
	{
		list->head[prio] = NULL;
		list->tail[prio] = NULL;
	}
	else
	{
		list->head[prio] = list->head[prio]->next;
	}

	q_insert_temp_tail_element(list, old_element);
	list->curr_msg_count_--;
	list_bitmap_count_decrease(list, prio);

	if(list->bitmap_count[prio] == 0)
	{
		list_clear_bitmap(list, prio);
	}

	if (list->writer_count_ > 0)
	{
		pthread_cond_signal(&list->q_not_full_);
	}
	return rc;
}

void list_set_bitmap(struct osal_msg_queue *list, unsigned long prio)
{
	list->bitmap[prio] = 1;
}

void list_clear_bitmap(struct osal_msg_queue *list, unsigned long prio)
{
	list->bitmap[prio] = 0;
}

unsigned long list_find_highest_priority_bitmap(struct osal_msg_queue *list)
{
	int i;

	for(i=MSG_PRIORITY_MAX-1; i>=0; i--)
	{
		if(list->bitmap[i] == 1)
			return i;
	}

	return 0;
}

void list_bitmap_count_increase(struct osal_msg_queue *list, unsigned long prio)
{
	list->bitmap_count[prio]++;
}

void list_bitmap_count_decrease(struct osal_msg_queue *list, unsigned long prio)
{
	list->bitmap_count[prio]--;
}

void q_insert_temp_tail_element(struct osal_msg_queue *q, struct osal_msg_element *element)
{
	if(q->temp_tail == NULL)
	{
		q->temp_head = element;
		element->next = NULL;
		element->prev = NULL;
		q->temp_tail = element;
	}
	else
	{
		q->temp_tail->next = element;
		element->next = NULL;
		element->prev = q->temp_tail;
		q->temp_tail = element;
	}
}

void q_remove_temp_tail_element(struct osal_msg_queue *q, 	struct osal_msg_element **element)
{

	*element = q->temp_tail;

	if(q->temp_head->next == NULL) //element 1개 남은 경우.
	{
		q->temp_head = NULL;
		q->temp_tail = NULL;
	}
	else
	{
		q->temp_tail->prev->next = NULL;
		q->temp_tail = q->temp_tail->prev;
	}

}

void q_destory_temp_tail(struct osal_msg_queue *q)
{
	do
	{
		q->temp_tail = q->temp_tail->prev;
	} while(q->temp_tail != NULL);

	if (q->pPool)
	{
		free(q->pPool);
		q->pPool = NULL;
	}
	if (q->pDataPool)
	{
		free(q->pDataPool);
		q->pDataPool = NULL;
	}


}

/*
 * Global Functions
 */
int OSAL_MSG_Init(void)
{
	return 0;
}

int OSAL_MSG_Create(unsigned long q_count, unsigned long msg_size,
		const char *q_name, unsigned long *q_id)
{
	struct osal_msg_queue               *q;
	struct osal_msg_element           *q_element;
	unsigned long					i;
	struct osal_msg_element           *q_element_pool;
	unsigned char					*data_pool;

	q = (struct osal_msg_queue *)malloc(sizeof(struct osal_msg_queue));
	if (q == NULL)
	{
		printf("malloc Error\n");
		return -1;
	}

	memset(q, 0, sizeof(struct osal_msg_queue));

	list_init(q, msg_size, q_count, SUSPENDTYPE_FIFO);
	q_element_pool = (struct osal_msg_element *)malloc(sizeof(struct osal_msg_element)*q_count);
	data_pool = (unsigned char *)malloc(msg_size*q_count);

	for (i=0; i<q_count; i++)
	{
		q_element = &q_element_pool[i];
		q_element->data = &data_pool[i*msg_size];
		q_insert_temp_tail_element(q, q_element);
	}

	q->pDataPool = data_pool;
	q->pPool = q_element_pool;

	*q_id = (unsigned long)q;

	if (q_name)
		strncpy(q->name, q_name, sizeof(q->name)-1);
	else
		q->name[0] = 0;

	return 0;
}

int OSAL_MSG_Destroy(unsigned long q_id)
{
	struct osal_msg_queue *q;

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_Destroy Error\n");
		return -1;
	}

	pthread_cond_destroy(&q->q_not_full_);
	pthread_cond_destroy(&q->q_not_empty_);
	pthread_mutex_destroy(&q->q_lock_);

	list_destroy(q);
	q_destory_temp_tail(q);

	if(q != NULL)
	{
		free(q);
	}

	return 0;
}

int OSAL_MSG_Clear(unsigned long q_id)
{
	struct osal_msg_queue 			  *q;
	int rc = 0;
	long long exp_nsec;
	struct timespec timeo, cur_time;
	unsigned long size;
	void			*pBuf;

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_Clear Error\n");
		return -1;
	}

	size = q->msg_size_;
	pBuf = malloc(size);
	if (pBuf == NULL)
	{
		printf("OSAL_MSG_Clear Error\n");
		return -1;
	}

	(void)pthread_mutex_lock(&q->q_lock_);
	do
	{
		clock_gettime(CLOCK_REALTIME, &cur_time);
		exp_nsec = (long long)cur_time.tv_sec * (long long)1000 * (long long)1000 * (long long)1000 + (long long)cur_time.tv_nsec;
		timeo.tv_sec = exp_nsec / (1000 * 1000 * 1000);
		timeo.tv_nsec = exp_nsec % (1000 * 1000 * 1000);

		rc = list_rem_next(q, pBuf, size, TRUE, &timeo);
		if (0 != rc)
		{
			if(ETIMEDOUT == rc)
			{
				(void)pthread_mutex_unlock(&q->q_lock_);
				free(pBuf);
				return 0;
			}
		}
	} while (1);

	(void)pthread_mutex_unlock(&q->q_lock_);

	free(pBuf);

	return 0;
}

int OSAL_MSG_Send(unsigned long q_id, const void *msg, unsigned long size)
{
	struct osal_msg_queue *q;
	int rc = 0;
	unsigned long prio;

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_Send Error\n");
		return -1;
	}

	if (size > q->msg_size_)
	{
		printf("size %ld Error\n", size);
		return -1;
	}

	rc = P_MSG_Check(q, q_id);
	if (0 != rc)
	{
		return rc;
	}

	prio = MSG_PRIORITY0;

	(void)pthread_mutex_lock(&q->q_lock_);

	rc = list_ins_next(q, prio, msg, size, FALSE, NULL);
	if (0 != rc)
	{
		printf("list_ins_next() rc[%d] Error!!\n", rc);
		(void)pthread_mutex_unlock(&q->q_lock_);
		return rc;
	}

	(void)pthread_mutex_unlock(&q->q_lock_);

	return 0;
}

int OSAL_MSG_SendTimeout(unsigned long q_id, const void *msg, unsigned long size, unsigned long timeout)
{
	struct osal_msg_queue *q;
	int rc = 0;
	unsigned long prio = 0;
	struct timespec timeo;
	struct timespec cur_time;
	long long exp_nsec;

	if (timeout == 0xFFFFFFFF)
	{
		return OSAL_MSG_Send(q_id, msg, size);
	}

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_SendTimeout Error\n");
		return -1;
	}

	if (size > q->msg_size_)
	{
		printf("size %ld Error\n", size);
		return -1;
	}

	rc = P_MSG_Check(q, q_id);
	if (0 != rc)
	{
		printf("P_MSG_Check : OSAL_MSG_FULL!! queue name : %s\n", q->name);
		return rc;
	}

	prio = MSG_PRIORITY0;

	if(timeout == 0)
	{
		timeo.tv_sec = 0;
		timeo.tv_nsec = 0;
	}
	else
	{
		clock_gettime(CLOCK_REALTIME, &cur_time);
		exp_nsec = (long long)cur_time.tv_sec * (long long)1000 * (long long)1000 * (long long)1000 + (long long)cur_time.tv_nsec;
		exp_nsec += (long long)(timeout * (long long)1000 - 999) * (long long)1000;
		timeo.tv_sec = exp_nsec / (1000 * 1000 * 1000);
		timeo.tv_nsec = exp_nsec % (1000 * 1000 * 1000);
	}

	(void)pthread_mutex_lock(&q->q_lock_);

	rc = list_ins_next(q, prio, msg, size, TRUE, &timeo);
	if (0 != rc)
	{
		(void)pthread_mutex_unlock(&q->q_lock_);

		if(ETIMEDOUT == rc)
		{
			printf("list_ins_next() rc[%d] VK_TIMEOUT!!\n", rc);
			return -1;
		}
		else
		{
			printf("list_ins_next() rc[%d] Error!!\n", rc);
			return -1;
		}
	}

	(void)pthread_mutex_unlock(&q->q_lock_);

	return 0;
}

int OSAL_MSG_Receive(unsigned long q_id, void *msg, unsigned long size)
{
	struct osal_msg_queue               *q;
	int rc = 0;

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_Receive Error\n");
		return -1;
	}

	if(size > q->msg_size_)
	{
		printf("size %ld Error!!\n", size);
		return -1;
	}

	(void)pthread_mutex_lock(&q->q_lock_);

	rc = list_rem_next(q, msg, size, FALSE, NULL);
	if(0 != rc)
	{
		printf("list_rem_next() rc[%d] Error!!\n", rc);
		(void)pthread_mutex_unlock(&q->q_lock_);
		return rc;
	}

	(void)pthread_mutex_unlock(&q->q_lock_);

	return 0;
}

int OSAL_MSG_ReceiveTimeout(unsigned long q_id, void *msg, unsigned long size, unsigned long timeout)
{
	struct osal_msg_queue               *q;
	int rc = 0;
	long long exp_nsec;
	struct timespec timeo, cur_time;

	if (timeout == 0xFFFFFFFF)
	{
		return OSAL_MSG_Receive(q_id, msg, size);
	}

	q = (struct osal_msg_queue *)q_id;
	if (q == NULL)
	{
		printf("OSAL_MSG_ReceiveTimeout Error\n");
		return -1;
	}

	if(size > q->msg_size_)
	{
		printf("size %ld Error!!\n", size);
		return -1;
	}

	if(timeout == 0)
	{
		timeo.tv_sec = 0;
		timeo.tv_nsec = 0;
	}
	else
	{
		clock_gettime(CLOCK_REALTIME, &cur_time);
		exp_nsec = (long long)cur_time.tv_sec * (long long)1000 * (long long)1000 * (long long)1000 + (long long)cur_time.tv_nsec;
		exp_nsec += (long long)(timeout * (long long)1000 - 999) * (long long)1000;
		timeo.tv_sec = exp_nsec / (1000 * 1000 * 1000);
		timeo.tv_nsec = exp_nsec % (1000 * 1000 * 1000);
	}

	(void)pthread_mutex_lock(&q->q_lock_);

	rc = list_rem_next(q, msg, size, TRUE, &timeo);
	if (0 != rc)
	{
		(void)pthread_mutex_unlock(&q->q_lock_);

		if(ETIMEDOUT == rc)
		{
			//            printf("list_rem_next() rc[%d] VK_TIMEOUT!!\n", rc);
			return -1;
		}
		else
		{
			printf("list_rem_next() rc[%d] Error!!\n", rc);
			return -1;
		}
	}

	(void)pthread_mutex_unlock(&q->q_lock_);

	return 0;
}

static int P_MSG_Check(struct osal_msg_queue *q, unsigned long q_id)
{
	unsigned long ulNearlyFullCount;

	ulNearlyFullCount = (q->msg_count_ * 3);
	ulNearlyFullCount = ulNearlyFullCount/4;

	if (q->msg_count_ == q->curr_msg_count_)
	{
		printf("queue_id %ld, full !!!\n", q_id);
		return -1;
	}
	else if( q->curr_msg_count_ > ulNearlyFullCount )
	{
		printf("MsgQue is nearly full - SendTask:%ld, QueId:%ld, (%ld, %ld), name:%s\r\n",
				(unsigned long)pthread_self(), q_id, q->msg_count_, q->curr_msg_count_, q->name);
	}

	return 0;
}

