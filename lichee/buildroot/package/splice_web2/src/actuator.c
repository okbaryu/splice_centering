#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#include "splice_libs.h"
#include "actuator.h"

#define MAX_IP_LENGTH 20
#define IP_FILE "/data/ipaddr"

#define STANDALONE
#define RX_DUMP
#define TX_DUMP
#define STATUS_DUMP

int socket_fd;
static char ip_addr[MAX_IP_LENGTH];

int actuator_init(const char *p)
{
	int fd;
	ssize_t rc;
	struct sockaddr_in server;

	fd = open(IP_FILE, O_RDWR);
	if(fd < 0 && p == NULL)
	{
		perror("failed to open");
		return -1;
	}

	rc = read(fd, ip_addr, MAX_IP_LENGTH);
	if(rc < 0)
	{
		perror("read fail");
	}
	ip_addr[rc-1] = 0;

#if 0
	if(strncmp(ip_addr, p, MAX_IP_LENGTH))
	{
		printf("%s:%s\n", ip_addr, p);
		write(fd, p, MAX_IP_LENGTH);
		strncpy(ip_addr, p, MAX_IP_LENGTH);
		printf("ip changed!!\n");
	}
#endif

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		printf("Could not create socket");
	}

	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(ACTUATOR_PORT);

	if (connect(socket_fd , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error");
		close(socket_fd);
		return -1;
	}
	return 0;
}

static void make_base_packet(unsigned char *buf, int cmd)
{
	buf[ACT_STX] = STX_SEND;
	buf[ACT_CMD1] = cmd;

	buf[ACT_CMD2] = SPARE;
	buf[ACT_DATA1] = SPARE;
	buf[ACT_DATA2] = SPARE;
	buf[ACT_DATA3] = SPARE;
	buf[ACT_DATA4] = SPARE;
	buf[ACT_DATA5] = SPARE;
	buf[ACT_DATA6] = SPARE;
	buf[CHECKSUM]  = SPARE;
}

static unsigned char make_checksum(unsigned char *buf)
{
	int i;
	unsigned char checksum = 0;

	for(i=0; i<MAX_PACKET_LENGTH-1; i++)
		checksum ^= buf[i];

	return checksum;
}

static int send_receive_packet(unsigned char *buf, unsigned char *ack)
{
	int i, rc;

#ifdef TX_DUMP
	for(i=0; i<MAX_PACKET_LENGTH;i++)
		printf("buf[%d] = %x\n", i, buf[i]);
#endif

	rc = send(socket_fd, buf, MAX_PACKET_LENGTH, 0);
	if(rc < 0)
	{
		close(socket_fd);
		perror("Send failed");
		return -1;
	}

	if(buf[ACT_CMD1] == CMD1_GET_ACT_PARAM || buf[ACT_CMD1] == CMD1_GET_ACT_POSITION)
	{
		if(recv(socket_fd, ack, MAX_PACKET_LENGTH, MSG_WAITALL) < 0)
		{
			close(socket_fd);
			printf("recv failed");
			return -1;
		}

#ifdef RX_DUMP
		for(i=0; i<MAX_PACKET_LENGTH;i++)
			printf("ack[%d] = %x\n", i, ack[i]);
#endif

		if(ack[ACT_STX] != STX_ACK || ack[CHECKSUM] != make_checksum(ack))
		{
			close(socket_fd);
			printf("ACK checksum error\n");
			return -1;
		}
	}
	return 0;
}

int actuator_get_status(act_status *p)
{

	unsigned char buf[MAX_PACKET_LENGTH];
	unsigned char ack[MAX_PACKET_LENGTH];

	if(p == NULL)
	{
		printf("Invalid parameter\n");
		return -1;
	}

	make_base_packet(buf, CMD1_GET_ACT_PARAM);
	buf[CHECKSUM] = make_checksum(buf);

	if(send_receive_packet(buf, ack) != 0)
	{
		printf("send_receive_packet failed");
		return -1;
	}

	p->act_direction = ack[ACK_ACT_DIRECTION];
	p->act_max_stroke = ack[ACK_ACT_MAX_STROKE];
	p->act_speed = ack[ACK_ACT_SPEED];
	p->act_l_limit = ack[ACK_ACT_L_LIMIT];
	p->act_r_limit = ack[ACK_ACT_R_LIMIT];
	p->act_origin_offset_msb = ack[ACK_ACT_ORG_OFFSET_MSB];
	p->act_origin_offset_lsb = ack[ACK_ACT_ORG_OFFSET_LSB];

// #ifdef STATUS_DUMP
	printf("ACTUATOR DIRECTION : %d\n", ack[ACK_ACT_DIRECTION]);
	printf("ACTUATOR MAX STROKE : %d\n", ack[ACK_ACT_MAX_STROKE]);
	printf("ACTUATOR SPEED : %d\n", ack[ACK_ACT_SPEED]);
	printf("ACTUATOR LEFT LIMIT : %d\n", ack[ACK_ACT_L_LIMIT]);
	printf("ACTUATOR RIGHT LIMIT : %d\n", ack[ACK_ACT_R_LIMIT]);
	printf("ACTUATOR ORG OFFSET MSB : %d\n", ack[ACK_ACT_ORG_OFFSET_MSB]);
	printf("ACTUATOR ORG OFFSET LSB : %d\n", ack[ACK_ACT_ORG_OFFSET_LSB]);
// #endif

	return 0;
}

int actuator_set_status(act_status *p)
{
	unsigned char buf[MAX_PACKET_LENGTH];
	unsigned char ack[MAX_PACKET_LENGTH];
	unsigned char param[MAX_STATUS_PARAM];
	int i, j, k, cmd2 = 0;
	act_status cur_p;

	if(p == NULL)
	{
		printf("Invalid parameter\n");
		return -1;
	}

	if(actuator_get_status(&cur_p) != 0)
	{
		printf("Get actuator status failed\n");
		return -1;
	}

	if(p->act_direction != cur_p.act_direction)
		{ cmd2 |= 0x1; param[0] = p->act_direction; }
	if(p->act_max_stroke != cur_p.act_max_stroke)
		{ cmd2 |= 0x2; param[1] = p->act_max_stroke; }
	if(p->act_speed != cur_p.act_speed)
		{ cmd2 |= 0x4; param[2] = p->act_speed; }
	if(p->act_l_limit != cur_p.act_l_limit)
		{ cmd2 |= 0x8; param[3] = p->act_l_limit; }
	if(p->act_r_limit != cur_p.act_r_limit)
		{ cmd2 |= 0x10; param[4] = p->act_r_limit; }
	if(p->act_origin_offset_msb != cur_p.act_origin_offset_msb ||
		p->act_origin_offset_lsb != cur_p.act_origin_offset_lsb)
	{
		cmd2 |= 0x20;
		param[5] = p->act_origin_offset_msb;
		param[6] = p->act_origin_offset_lsb;
	}

	if(cmd2 == 0) return 0;

	for(i=1, j=0; i<=0x20; i<<=1, j++)
	{
		if(cmd2 & i)
		{
			make_base_packet(buf, CMD1_SET_ACT_PARAM);
			buf[ACT_CMD2] = i;
			buf[ACT_DATA1] = param[j];
			if(i == CMD2_ACT_ORG_OFFSET) buf[ACT_DATA2] = param[j+1];
			buf[CHECKSUM] = make_checksum(buf);
			printf("set status : cmd 0x%x\n", i);
			printf("====== Set Data Start ======\n");
			for(k = 0; k < MAX_PACKET_LENGTH; k++){
				printf("%x ", buf[k]);
			}
			printf("====== Set Data End ======\n");
			if(send_receive_packet(buf, ack) != 0) return -1;
		}
	}




	return 0;
}

int actuator_get_current_postion(act_position *p)
{

	unsigned char buf[MAX_PACKET_LENGTH];
	unsigned char ack[MAX_PACKET_LENGTH];

	make_base_packet(buf, CMD1_GET_ACT_POSITION);
	buf[CHECKSUM] = make_checksum(buf);

	if(send_receive_packet(buf, ack) != 0) return -1;

	p->act_cur_position_msb = ack[ACK_ACT_CUR_POSITION_MSB];
	p->act_cur_position_lsb = ack[ACK_ACT_CUR_POSITION_LSB];
	p->act_prev_position_msb = ack[ACK_ACT_PREV_POSITION_MSB];
	p->act_prev_position_lsb = ack[ACK_ACT_PREV_POSITION_LSB];

#ifdef STATUS_DUMP
	printf("ACTUATOR CURRENT POSTION MSB = 0x%x(%d)\n",ack[ACK_ACT_CUR_POSITION_MSB], ack[ACK_ACT_CUR_POSITION_MSB]);
	printf("ACTUATOR CURRENT POSTION LSB = 0x%x(%d)\n",ack[ACK_ACT_CUR_POSITION_LSB], ack[ACK_ACT_CUR_POSITION_LSB]);
	printf("ACTUATOR PREV POSTION MSB = 0x%x(%d)\n",ack[ACK_ACT_PREV_POSITION_MSB], ack[ACK_ACT_PREV_POSITION_MSB]);
	printf("ACTUATOR PREV POSTION LSB = 0x%x(%d)\n",ack[ACK_ACT_PREV_POSITION_LSB], ack[ACK_ACT_PREV_POSITION_LSB]);
#endif

	return 0;
}

int actuator_set_current_position(act_position *p, int cmd)
{

	unsigned char buf[MAX_PACKET_LENGTH];
	unsigned char ack[MAX_PACKET_LENGTH];

	make_base_packet(buf, CMD1_SET_ACT_POSITION);

	if(cmd == CMD2_ACT_MOVE)
	{
		buf[ACT_CMD2] = 0x01;
		buf[DATA_ACT_MOVE_VALUE_MSB] = p->act_cur_position_msb;
		buf[DATA_ACT_MOVE_VALUE_LSB] = p->act_cur_position_lsb;
	}
	else
	{
		buf[ACT_CMD2] = cmd;
	}

	buf[CHECKSUM] = make_checksum(buf);

	if(send_receive_packet(buf, ack) != 0)
	{
		printf("%s: actuator comm error\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

#ifdef STANDALONE
int main(int argc , char *argv[])
{
	act_status status;
	act_position pos;
	int i;

	if(argc != 2)
	{
		printf("Usage : act ip_addr\n");
		return -1;
	}

	if(actuator_init(argv[1]) != 0)
	{
		printf("Initialize actuator failed\n");
		return -1;
	}

	actuator_get_status(&status);
	//actuator_get_current_postion(&pos);

	status.act_direction = 0;
	status.act_max_stroke = 50;
	status.act_origin_offset_msb = 0x84;
	status.act_origin_offset_lsb = 0xb0;
	//actuator_set_status(&status);

	pos.act_cur_position_msb = 0x00;
	pos.act_cur_position_lsb = 0x00;
	actuator_set_current_position(&pos, CMD2_ACT_MOVE_ORG);

	close(socket_fd);

	return 0;
}
#endif
