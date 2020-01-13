#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

enum PACKET_INDEX
{
	ACT_STX		= 0,
	ACT_CMD1,
	ACT_CMD2,
	ACT_DATA1,
	ACT_DATA2,
	ACT_DATA3,
	ACT_DATA4,
	ACT_DATA5,
	ACT_DATA6,
	CHECKSUM
};

typedef struct act_status_{
	unsigned char act_direction;
	unsigned char act_max_stroke;
	unsigned char act_speed;
	unsigned char act_l_limit;
	unsigned char act_r_limit;
	unsigned char act_origin_offset_msb;
	unsigned char act_origin_offset_lsb;
}act_status;

typedef struct act_position_{
	unsigned char act_cur_position_msb;
	unsigned char act_cur_position_lsb;
	unsigned char act_prev_position_msb;
	unsigned char act_prev_position_lsb;
}act_position;

#define MAX_PACKET_LENGTH 		10

#define SPARE					0x00
#define FOWARD					0x01
#define REVERSE					0x00
#define DIRECTION_MASK			0x80 /* left, right */
#define MOVE_VALUE_1MM			0x0064

#define STX_SEND				0xCA
#define STX_ACK					0xCB
#define CMD1_GET_ACT_PARAM		0x01
#define CMD1_GET_ACT_POSITION	0x02

#define CMD1_SET_ACT_POSITION	0x40
#define CMD2_ACT_MOVE			0x01
#define CMD2_ACT_MOVE_ORG		0x02
#define CMD2_ACT_STOP			0x04
#define CMD2_ACT_RESET			0x06

#define CMD1_SET_ACT_PARAM		0x80
#define CMD2_ACT_DIRECTION		0x01
#define CMD2_ACT_MAX_STROKE		0x02
#define CMD2_ACT_MAX_SPEED		0x04
#define CMD2_ACT_L_LIMIT		0x08
#define CMD2_ACT_R_LIMIT		0x10
#define CMD2_ACT_ORG_OFFSET		0x20
#define MAX_STATUS_PARAM		7

/* Get parameter status */
#define ACK_ACT_DIRECTION		ACT_CMD2
#define ACK_ACT_MAX_STROKE		ACT_DATA1
#define ACK_ACT_SPEED			ACT_DATA2
#define ACK_ACT_L_LIMIT			ACT_DATA3
#define ACK_ACT_R_LIMIT			ACT_DATA4
#define ACK_ACT_ORG_OFFSET_MSB		ACT_DATA5
#define ACK_ACT_ORG_OFFSET_LSB		ACT_DATA6

/* Set parameter */
#define DATA_ACT_DIRECTION				ACT_DATA1
#define DATA_ACT_MAX_STROKE				ACT_DATA1
#define DATA_ACT_MAX_SPEED				ACT_DATA1
#define DATA_ACT_L_LIMIT				ACT_DATA1
#define DATA_ACT_R_LIMIT				ACT_DATA1
#define DATA_ACT_ORG_OFFSET_MSB			ACT_DATA1
#define DATA_ACT_ORG_OFFSET_LSB			ACT_DATA2

/* Get actuator position */
#define ACK_ACT_CUR_POSITION_MSB	ACT_DATA1
#define ACK_ACT_CUR_POSITION_LSB	ACT_DATA2
#define ACK_ACT_PREV_POSITION_MSB	ACT_DATA3
#define ACK_ACT_PREV_POSITION_LSB	ACT_DATA4

/* Set actuator */
#define DATA_ACT_MOVE_VALUE_MSB			ACT_DATA1
#define DATA_ACT_MOVE_VALUE_LSB			ACT_DATA2

#define ACT_MOVE_1MM 0x64

#define ACTUATOR_PORT	5000

int actuator_init(const char *ip_addr);
int actuator_get_status(act_status *p);
int actuator_set_status(act_status *p);
int actuator_get_current_postion(act_position *p);
int actuator_set_current_position(act_position *p, int cmd);

#endif

