#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "splice_libs.h"
#include "plc.h"

//#define STANDALONE
#define RX_DUMP
#define MAX_BUF_LENGTH 256

#define TRUE 1
#define FALSE 0

int plc_fd;

static int parseBuf(char *src, char *dst, int cnt)
{
	int data_src, data_length, i;

	data_src = src[0] << 8 | src[1];
	if(data_src == 0xD000) // sub-header
	{
		data_src = src[2] << 8 | src[3];
		if(data_src == 0x00FF)
		{
			data_src = src[5] << 8 | src[4];
			if(data_src == 0x03FF)
			{
				if(src[6] == 0x00)
				{
					PLC_BIN_to_INT(&src[7], &data_src);
					data_length = data_src - 2;

					if(data_length > MAX_BUF_LENGTH)
					{
						printf("data_length is bigger than srcfer %d\n", data_length);
						return -1;
					}
					else
					{
						printf("data_length = %d\n", data_length);
					}

					PLC_BIN_to_INT(&src[9], &data_src);
					if(data_src == 0x00)
					{
						if(data_length && dst)
						{
							memcpy((void *)dst, (void *)&src[11], data_length);
#ifdef RX_DUMP
							printf("R value\n");
							for(i=0; i<data_length; i++)
							{
								printf("0x%x ", src[i+11]);
							}
							printf("\n");
#endif
						}
						else if(data_length && dst == NULL)
						{
							printf("dest buf is NULL\n");
							return -1;
						}
					}
					else
					{
						printf("PLC Comm error %d\n", data_src);
						return PLC_COMM_ERR;
					}
					return PLC_COMM_ACK;
				}
				return PLC_COMM_ERR;
			}
			return PLC_COMM_ERR;
		}
		return PLC_COMM_ERR;
	}
	return PLC_COMM_ERR;
}

/* If readCmd is TRUE, it's for request to PLC send back R register value.
 * If readCmd is FALSE, it's for request to write data to PLC and expect ACK.
 * head_data_Address is used for write data buffer when readCmd is FALSE,
 * and used for receive buffer for R register when readCmd is TRUE
 */
int sendPlc(unsigned int r_head_address, char *head_data_address, char size, char readCmd)
{
	int Send_length, cnt;
	unsigned char i, Send_data[MAX_BUF_LENGTH], send_cnt=0;
	char buf[MAX_BUF_LENGTH];

	memset(Send_data, 0, MAX_BUF_LENGTH);
	memset(buf, 0x0, MAX_BUF_LENGTH);

	if(r_head_address == 0 || head_data_address == NULL)
	{
		printf("invalid parameter\n");
		return -1;
	}

	Send_length = ((int)size & 0x00FF) << 2;				// Long (4byte) * Size(보낼 점 수) ==> Size * 4(byte)
	// Shift연산으로 곱하기 4와 같은 효과
	//-------- sub header -------------
	Send_data[send_cnt++] = 0x50;

	Send_data[send_cnt++] = 0x00;

	//-------- network number ---------
	Send_data[send_cnt++] = 0x00;

	//-------- PLC number -------------
	Send_data[send_cnt++] = 0xFF;

	//-- request destination module I/O No.--
	Send_data[send_cnt++] = 0xFF;		// L

	Send_data[send_cnt++] = 0x03;		// H

	//-- request destination station No.--
	Send_data[send_cnt++] = 0x00;

	//-------- request data length ----------
	if(readCmd)
	{
		Send_data[send_cnt++] = 0x0C;
	}
	else
	{
		Send_data[send_cnt++] = 0x0C + Send_length;		// L	( 기본 Packet(12byte) + (4byte * Size) )
	}

	Send_data[send_cnt++] = 0x00;					// H

	//------ cpu Monitoring timer --------
	if(readCmd)
	{
		Send_data[send_cnt++] = 0x10;		// L
	}
	else
	{
		Send_data[send_cnt++] = 0x08;		// L		// 2000ms (0x08 x 250ms)
	}
	//		Send_data[send_cnt++] = 0x00;		// L		// 무한대기					//180205

	Send_data[send_cnt++] = 0x00;		//H

	//----------- command ------------      // 일괄쓰기 Command ==> 1401
	Send_data[send_cnt++] = 0x01;		// L

	if(readCmd)
	{
		Send_data[send_cnt++] = 0x04;		// H
	}
	else
	{
		Send_data[send_cnt++] = 0x14;		// H
	}

	//----------- sub command ------------	// word 단위 쓰기
	Send_data[send_cnt++] = 0x00;		// L

	Send_data[send_cnt++] = 0x00;		// H

	//----------- head device ------------
	Send_data[send_cnt++] = (unsigned char)(r_head_address & 0xff);      // L

	r_head_address >>= 8;
	Send_data[send_cnt++] = (unsigned char)(r_head_address & 0xff);

	Send_data[send_cnt++] = 0x00;                                   	// H

	//----------- device code ------------
	Send_data[send_cnt++] = 0xAF;	//  R* 에 해당하는 디바이스 코드(바이너리 코드 사용시)

	if(readCmd)
	{
		//------ number of device points -----
		Send_data[send_cnt++] = (size & 0xff); 				// L
		size >>= 8;
		Send_data[send_cnt++] = (size & 0xff);   				// H
	}
	else
	{
		//------ number of device points -----
		Send_data[send_cnt++] = (size & 0xff) << 1; 	// L			// word(2byte) 단위로 보내므로, Size * 2
		// shift 연산으로 Size * 2와 같은 효과
		Send_data[send_cnt++] = 0x00;   				// H
	}

	//- data for number of device points -

	if(head_data_address && readCmd == FALSE)
	{
		for(i=0;i<Send_length;i++)
		{
			Send_data[send_cnt++] = *(head_data_address++);
		}
	}

	cnt = send(plc_fd, Send_data, send_cnt, 0);
	printf("send plc, length=%d\n", cnt);

	cnt = recv(plc_fd, buf, MAX_BUF_LENGTH, 0);
	return parseBuf(buf, head_data_address, cnt);
}

int plc_init(void)
{
	struct sockaddr_in server;

	plc_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (plc_fd == -1)
	{
		printf("Could not create socket");
	}

	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = inet_addr("192.168.29.6");
	server.sin_family = AF_INET;
	server.sin_port = htons(PLC_PORT);

	if (connect(plc_fd, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error to plc");
		close(plc_fd);
		return -1;
	}

	return 0;
}

#if defined(STANDALONE)
int main(int argc, char **argv)
{
	unsigned char Send_Data_Array[10] = {0,};
	char buf[MAX_BUF_LENGTH] = {0,};
	int rc;

	printf("plc comm test 0.6\n");

	if(plc_init() != 0)
	{
		printf("failed to init plc\n");
		return -1;
	}

	TASK_Sleep(1000);

	printf("Send cmd to PLC for read R-register\n");
	rc = sendPlc(EEP_Data_Start_R_Register, buf, EEP_R_Register_Size, TRUE);
	printf("rc = %d\n",rc);
	printf("Send width 20 to PLC\n");
	LONG_to_PLC_BIN_ARRAY(20, Send_Data_Array);
	rc = sendPlc(EEP_Width_R_Register, Send_Data_Array, 1, FALSE);
	printf("rc = %d\n",rc);
	printf("Send width error to PLC\n");
	LONG_to_PLC_BIN_ARRAY(2, Send_Data_Array);
	rc = sendPlc(EEP_Err_Bit_R_Register, Send_Data_Array, 1, FALSE);
	printf("rc = %d\n",rc);


	while(1)
	{
		TASK_Sleep(1000);
	}


	return 0;
}
#endif


