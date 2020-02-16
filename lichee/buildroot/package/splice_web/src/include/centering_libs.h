#include "plc.h"

#define TIP_LEFT 0x1
#define TIP_RIGHT 0x0

#define ACT_MOVE_LEFT 0
#define ACT_MOVE_RIGHT 1

#define LPos02 0
#define LPos01 1
#define RPos01 2
#define RPos02 3

int centering_libs_init(void);
char getTipDirection(void);
void readRRegister(char dump, RRegister *R);
void enableReadPos(char onOff);
int getIsCentering(void);
int getEncoderCnt(void);
void resetEncoder(void);
void act_move(char direction, int pos);
int setOffsetCoeff(int coeff);
void setIsCentering(char status);