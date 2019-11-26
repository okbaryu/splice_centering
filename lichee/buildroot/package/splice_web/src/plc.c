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
#include "osal_msg.h"
#include "plc.h"

//#define STANDALONE
//#define RX_DUMP

#define MAX_BUF_LENGTH 256
#define PLC_MSG_Q_CNT 64
#define PLC_MSG_NAME "PLCIO"

#define PLC_TRUE 0
#define PLC_FALSE 1

int plc_fd;
int plc_wr_fd[4];
int plc_rd_fd[4];
volatile unsigned char PLCIO;
unsigned long plc_msg_id;

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
						//printf("data_length = %d\n", data_length);
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

	cnt = recv(plc_fd, buf, MAX_BUF_LENGTH, 0);
	return parseBuf(buf, head_data_address, cnt);
}

void sendPlcIO(int data)
{
	plc_wr_fd[PLC_WR_CENTERING] = open("/sys/class/gpio/gpio138/value", O_RDWR);
	plc_wr_fd[PLC_WR_COMM_ERROR] = open("/sys/class/gpio/gpio139/value", O_RDWR);

	char buf[3] = {0, };

	if(data == PLC_WR_TIP_DETECT)
	{
		plc_wr_fd[PLC_WR_TIP_DETECT] = open("/sys/class/gpio/gpio137/value", O_RDWR);
		buf[0] = '0';
		write(plc_wr_fd[PLC_WR_TIP_DETECT], buf, sizeof(buf));
		close(plc_wr_fd[PLC_WR_TIP_DETECT]);
	}
	else if(data == PLC_WR_CENTERING)
	{
		plc_wr_fd[PLC_WR_TIP_DETECT] = open("/sys/class/gpio/gpio137/value", O_RDWR);
		plc_wr_fd[PLC_WR_CENTERING] = open("/sys/class/gpio/gpio138/value", O_RDWR);
		buf[0] = '0';
		write(plc_wr_fd[PLC_WR_TIP_DETECT], buf, sizeof(buf));
		write(plc_wr_fd[PLC_WR_CENTERING], buf, sizeof(buf));
		close(plc_wr_fd[PLC_WR_TIP_DETECT]);
		close(plc_wr_fd[PLC_WR_CENTERING]);

	}
	else if(data == PLC_WR_COMM_ERROR)
	{
		plc_wr_fd[PLC_WR_COMM_ERROR] = open("/sys/class/gpio/gpio139/value", O_RDWR);
		buf[0] = '0';
		write(plc_wr_fd[PLC_WR_COMM_ERROR], buf, sizeof(buf));
		close(plc_wr_fd[PLC_WR_COMM_ERROR]);
	}
	else if(data == PLC_WR_RESET)
	{
		plc_wr_fd[PLC_WR_TIP_DETECT] = open("/sys/class/gpio/gpio137/value", O_RDWR);
		plc_wr_fd[PLC_WR_CENTERING] = open("/sys/class/gpio/gpio138/value", O_RDWR);
		plc_wr_fd[PLC_WR_COMM_ERROR] = open("/sys/class/gpio/gpio139/value", O_RDWR);
		buf[0] = '1';
		write(plc_wr_fd[PLC_WR_TIP_DETECT], buf, sizeof(buf));
		write(plc_wr_fd[PLC_WR_CENTERING], buf, sizeof(buf));
		write(plc_wr_fd[PLC_WR_COMM_ERROR], buf, sizeof(buf));
		close(plc_wr_fd[PLC_WR_TIP_DETECT]);
		close(plc_wr_fd[PLC_WR_CENTERING]);
		close(plc_wr_fd[PLC_WR_COMM_ERROR]);
	}
	else
	{
		printf("Unknown data to send PLC\n");
	}

}

void *plcIO(void *data)
{
	unsigned char prevPLCIO = 0xAA;
	char buf[4][3];

	while(1)
	{
		plc_rd_fd[PLC_RD_AUTO_MANUAL] = open("/sys/class/gpio/gpio128/value", O_RDONLY);
		plc_rd_fd[PLC_RD_CPC_TO_EPC] = open("/sys/class/gpio/gpio129/value", O_RDONLY);
		plc_rd_fd[PLC_RD_SAVE_WIDTH] = open("/sys/class/gpio/gpio130/value", O_RDONLY);
		plc_rd_fd[PLC_RD_CHECK_CUTTING] = open("/sys/class/gpio/gpio131/value", O_RDONLY);

		read(plc_rd_fd[PLC_RD_AUTO_MANUAL], buf[0], sizeof(buf[0]));
		read(plc_rd_fd[PLC_RD_CPC_TO_EPC], buf[1], sizeof(buf[1]));
		read(plc_rd_fd[PLC_RD_SAVE_WIDTH], buf[2], sizeof(buf[2]));
		read(plc_rd_fd[PLC_RD_CHECK_CUTTING], buf[3], sizeof(buf[3]));
		PLCIO = (atoi(buf[3]) << 3 | atoi(buf[2]) << 2 | atoi(buf[1]) << 1 | atoi(buf[0])) & 0x0F;

		if(PLCIO != prevPLCIO)
		{
			prevPLCIO = PLCIO;
		}

		close(plc_rd_fd[0]);
		close(plc_rd_fd[1]);
		close(plc_rd_fd[2]);
		close(plc_rd_fd[3]);

		TASK_Sleep(50);
	}

}

int plc_init(void)
{
	struct sockaddr_in server;
	pthread_t plcIOid= 0;

	plc_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (plc_fd == -1)
	{
		printf("Could not create socket");
	}

	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = inet_addr("192.168.0.100");
	server.sin_family = AF_INET;
	server.sin_port = htons(PLC_PORT);

	if (connect(plc_fd, (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error to plc");
		close(plc_fd);
		return -1;
	}

	//plc_wr_fd[PLC_WR_SYSTEM_READY] = open("/sys/class/gpio/gpio136/value",O_RDWR);
	plc_wr_fd[PLC_WR_TIP_DETECT] = open("/sys/class/gpio/gpio137/value", O_RDWR);
	plc_wr_fd[PLC_WR_CENTERING] = open("/sys/class/gpio/gpio138/value", O_RDWR);
	plc_wr_fd[PLC_WR_COMM_ERROR] = open("/sys/class/gpio/gpio139/value", O_RDWR);

	//if (OSAL_MSG_Create(PLC_MSG_Q_CNT, 1024, PLC_MSG_NAME, &plc_msg_id) < 0)
	//{
		//printf("MSG create fail for centering\n");
		//return -1;
	//}

	if (pthread_create( &plcIOid, NULL, plcIO, (void *)NULL ))
	{
		PrintError( "could not create thread for data gathering; %s\n", strerror( errno ));
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


