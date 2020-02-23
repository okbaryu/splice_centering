#ifndef __CENTERING_LIBS_H__
#define __CENTERING_LIBS_H__

#include "plc.h"

#define TIP_LEFT 0x1
#define TIP_RIGHT 0x0

#define ACT_MOVE_LEFT 0
#define ACT_MOVE_RIGHT 1

#define LPos02 0
#define LPos01 1
#define RPos01 2
#define RPos02 3

#define MODE_CALIBRATION 0
#define MODE_RUNNING     1
#define CAM0             0
#define CAM1             1
#define CAMALL           2

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

/* Calibraion function */
void calibrationSetMode(unsigned char mode);
int calibrationSetCam(unsigned char cam);
void calibrationSave(void);

#endif
