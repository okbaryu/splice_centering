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

#define ALGORITHM1 1
#define ALGORITHM2 2
#define ALGORITHM3 3

#define MODE_CALIBRATION 0
#define MODE_RUNNING     1
#define CAM0             0
#define CAM1             1
#define CAMALL           2

#define DEFAULT_CPC_RATIO 90

#define PROFILE_AREA_LEADING_DIVIDED 0
#define PROFILE_AERA_WHOLE 1

#define MAX_LEADING_PROFILE 1000
#define MAX_WHOLE_PROFILE 3000

struct leadingProfile
{
	char area;
	float RWidth;
	int enc_cnt;
}leadingProfile;

struct wholeProfile{
	float rWidth[4];
	float RWidth;
	unsigned char current_section;
	int encoder;
	int tip_detect_cnt;
}wholeProfile;

int centering_libs_init(void);
char getTipDirection(void);
void readRRegister(char dump, RRegister *R);
void enableReadPos(char onOff);
int getIsCentering(void);
int getEncoderCnt(void);
void resetEncoder(void);
void act_move(char direction, int pos);
int setOffsetCoeff(int coeff);
int setCPCRatio(int ratio);
int getCPCRatio(void);
void setIsCentering(char status);
int getAlgorithm(void);
int width_check(float width, RRegister *r);

int startProfile(char onOff);
int isProfileOn(void);
void resetProfile(void);
void saveProfile(void);
void viewProfile(char area);
void wholeAreaProfile(char section, float RWidth, float *rWidth);
void leadingOffsetProfile(float RWidth, float *leading_tip_width);

/* Calibraion function */
void calibrationSetMode(unsigned char mode);
int calibrationSetCam(unsigned char cam);
void calibrationSave(void);

#endif
