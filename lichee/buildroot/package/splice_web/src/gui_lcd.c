#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <stdint.h>
#include <json.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#define BUF_SIZE        4096
#define WIDTH_SCREEN	272
#define HEIGHT_SCREEN	480
#define WIDTH_IMAGE     1920

#define DEV_CAM_0	"/dev/ttyS0"
#define DEV_CAM_1	"/dev/ttyS2"

extern int uart_open(char *devname, int baud);
extern void  uart_close( int fd );

#define MODE_RUNNING 		0
#define MODE_CALIBRATION 	1
#define MODE_LOADING		2
#define MODE_ALERT		3

#define STR_RUN_MODE		"0"
#define STR_CAL_MODE		"1"

#define IMAGE_LOADING	0
#define IMAGE_ALERT_0	1
#define IMAGE_ALERT_1	2
#define IMAGE_BG	3
#define IMAGE_BG_CALI	4
#define IMAGE_BG_RUN	5
#define IMAGE_SIZE	6	

#define FILE_PATH_IMAGE_LOADING		"./images/loading.bmp"
#define FILE_PATH_IMAGE_ALERT_0		"./images/alert0.bmp"
#define FILE_PATH_IMAGE_ALERT_1		"./images/alert1.bmp"
#define FILE_PATH_IMAGE_BG		"./images/bg.bmp"
#define FILE_PATH_IMAGE_BG_CALI		"./images/bg_cali.bmp"
#define FILE_PATH_IMAGE_BG_RUN		"./images/bg_run.bmp"

BITMAP bitmap[IMAGE_SIZE];

#define true	1
#define false	0
#define EDGE_BLACK_TO_WHITE	0
#define EDGE_WHITE_TO_BLACK	1

int gMode = MODE_LOADING;
int gPrevMode = MODE_LOADING;

int nomalInputCounter0 = 0;
int nomalInputCounter1 = 0;

char gVf0[BUF_SIZE] = "";
char gVf1[BUF_SIZE] = "";

char g_frame0[4096];
int g_index0 = 0;

char g_frame1[4096];
int g_index1 = 0;

int gUartPort0=0;
int gUartPort1=0;

char gThreshold0 = 128;
char gThreshold1 = 128;

int gLeftTrim0 = 0;
int gRightTrim0 = 1919;

int gLeftTrim1 = 0;
int gRightTrim1 = 1919;

int gLeftPosition = 0;
int gRightPosition = 0;

double widthmaterial =0;
int centerarrow=0;
int arrow_Y=0;

#define FONT_WIDTH	16
#define FONT_HEIGHT	21
#define FONT_COUNT	10
unsigned char gFont[FONT_COUNT][FONT_HEIGHT][FONT_WIDTH];

int gCount0 = 12;
int gCount1 = 13;

pthread_mutex_t gMutex0;
pthread_mutex_t gMutex1;

#define SET_CAM_ALL	0
#define SET_CAM_0	1
#define SET_CAM_1	2

#define STR_SET_MODE	"setMode"
#define STR_RUNNING	"running"
#define STR_CALI	"calibration"

#define STR_SET_CAMERA	"setCamera"
#define STR_CAM_ALL	"all"
#define STR_CAM_0	"cam0"
#define STR_CAM_1	"cam1"

#define STR_SET_THRESHOLD		"setThreshold"
#define STR_SET_LEFT_TRIM		"setLeftTrim"
#define STR_SET_RIGHT_TRIM		"setRightTrim"
#define STR_SAVE_CALIBRATION_DATA	"saveCalibrationData"
#define STR_GET_CALIBRATION_DATA	"getCalibrationData"
#define STR_ERASE_CALIBRATION_DATA	"eraseCalibrationData"
#define STR_GET_IMAGE			"getImage"
#define STR_SET_STREAMING		"setStreaming"
#define STR_ON				"on"
#define STR_OFF				"off"
#define STR_LPOS02			"LPos02"
#define STR_LPOS01			"LPos01"
#define STR_RPOS01			"RPos01"
#define STR_RPOS02			"RPos02"
#define STR_GET_POSITIONS	"getPositions"

#define FILE_PATH		"/mnt/extsd/calibration_data.json"
#define KEY_CAM0_CENTER 	"cam0Center"
#define KEY_CAM1_CENTER  	"cam1Center"
#define KEY_CAM0_LEFT_TRIM  	"cam0LeftTrim"
#define KEY_CAM0_RIGHT_TRIM  	"cam0RightTrim"
#define KEY_CAM1_LEFT_TRIM  	"cam1LeftTrim"
#define KEY_CAM1_RIGHT_TRIM  	"cam1RightTrim"
#define KEY_CAM0_THRESHOLD  	"cam0Threshold"
#define KEY_CAM1_THRESHOLD  	"cam1Threshold"
#define KEY_PIXEL_LEN	 	"pixelLen"

int gSetCam = SET_CAM_ALL;

#define STREAM_MODE_INFINITE	0
#define STREAM_MODE_ONESHOT	1
#define STREAM_MODE_STOP	2

int gStreamMode = STREAM_MODE_STOP;

#define BUF_CAM_SIZE	1920
char bufCamAll[BUF_CAM_SIZE] = {0};
char bufCam0[BUF_CAM_SIZE] = {0};
char bufCam1[BUF_CAM_SIZE] = {0};

typedef struct {
	unsigned char startCode;
	int cam0Center;
	int cam1Center;
	int cam0LeftTrim;
	int cam0RightTrim;
	int cam1LeftTrim;
	int cam1RightTrim;
	unsigned char cam0Threshold;
	unsigned char cam1Threshold;
	double pixelLen[3840];
	unsigned char errCheck;
} CalibrationData;	
	
CalibrationData gCalibrationData;

struct {
	int flag;
	const char *flag_str;
} json_flags[] = {
	{ JSON_C_TO_STRING_PLAIN, "JSON_C_TO_STRING_PLAIN" },
	{ JSON_C_TO_STRING_SPACED, "JSON_C_TO_STRING_SPACED" },
	{ JSON_C_TO_STRING_PRETTY, "JSON_C_TO_STRING_PRETTY" },
	{ JSON_C_TO_STRING_NOZERO, "JSON_C_TO_STRING_NOZERO" },
	{ JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY, 
		"JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY" },
	{ -1, NULL }
}; // Create an anonymous struct, instanciate an array and fill it


void drawImage(HDC vdc, int x,int y,int w, int h, int indexImage);
int loadCalibrationData();
void setMode(int uartPort, int mode, int threshold);
int getPositions(int sock, int LPos, int RPos, int center);
int getIpParse(char * buf);

int min(int a, int b) { return a<b?a:b;}
int max(int a, int b) { return a>b?a:b;}

void clearScreen(HDC vdc, int w, int h)
{
	//drawImage(vdc, 0, 0, w, h, IMAGE_BG);
	if(gMode==MODE_CALIBRATION) drawImage(vdc, 0, 0, w, h, IMAGE_BG_CALI);
	else if(gMode==MODE_RUNNING) drawImage(vdc, 0, 0, w, h, IMAGE_BG_RUN);

}

long long GetNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (long long)tv.tv_sec * 1000000ll + tv.tv_usec;
}

void fetchFrameCalibration(char * vf, const char * buf, int size)
{
	memcpy(vf, buf, size);
}

Uint8 getPixelR(Uint8 * framebuffer, int x, int y, int pitch)
{
        int row = y*pitch;
        int offset = row+x*4;

        return framebuffer[offset+2];//R
}

Uint8 getPixelG(Uint8 * framebuffer, int x, int y, int pitch)
{
        int row = y*pitch;
        int offset = row+x*4;

        return framebuffer[offset+1];//G
}


Uint8 getPixelB(Uint8 * framebuffer, int x, int y, int pitch)
{
        int row = y*pitch;
        int offset = row+x*4;

        return framebuffer[offset];//B
}

void putPixelRGB(Uint8 * framebuffer, int x, int y, int pitch, char r, char g, char b)
{
	int row = y*pitch;
	int offset = row+x*4;

	framebuffer[offset]  =b;//B
	framebuffer[offset+1]=g;//G
	framebuffer[offset+2]=r;//R
	framebuffer[offset+3]=255;//A
}

void drawImage(HDC vdc, int x,int y,int w, int h, int indexImage)
{
	//printf("(i) drawImage called: x=%d, y=%d, w=%d, h=%d, index=%d\n",
	//		x, y, w, h, indexImage);
	FillBoxWithBitmap(vdc, x, y, w, h, &bitmap[indexImage]);
}

void drawVLine(Uint8 * framebuffer, int x, int pitch, char r, char g, char b )
{
	int i=0;
	for(i=156; i<324; i++)
	{
		putPixelRGB(framebuffer, x, i, pitch, r, g, b);
	}
}

void putPixel(Uint8 * framebuffer, int x, int y, int pitch, char grey)
{
	int row = y*pitch;
	int offset = row+x*4;

	framebuffer[offset]  =grey;//B
	framebuffer[offset+1]=grey;//G
	framebuffer[offset+2]=grey;//R
	framebuffer[offset+3]=255;//A
}


char temp[1920];
void dilation(char * bin, int size)
{
	memcpy(temp, bin, size);
	memset(bin, 0, size);

	int i=0;
	for(i=0; i<size; i++)
	{
		char src = temp[i];
		if(src==1)
		{
			if(i-1>=0) bin[i-1] = 1;
			bin[i] = 1;
			if(i+1<size) bin[i+1] = 1;
		}
	}
}

void erosion(char * bin, int size)
{
	memcpy(temp, bin, size);
	memset(bin, 0, size);

	int i=0;
	for(i=0; i<size; i++)
	{
		char left=0, center=0, right=0;
		if(i-1>=0) left = temp[i-1];
		center = temp[i];
		if(i+1<size) right = temp[i+1];

		if(left==1 && center==1 && right==1)
			bin[i] = 1;
	}
}

char bin[1920];

void removeNoise(char * bin, int size)
{
	// dilation
	dilation(bin, size);

	// erosion
	erosion(bin, size);

	// erosion
	erosion(bin, size);

	// dilation
	dilation(bin, size );
}
	
int getGridSize(const char* buf, int size, int threshold)
{
	int i=0, j=0;
	for(i=0; i<size; i++)
		if(buf[i]>threshold) bin[i] = 1;
		else bin[i] = 0;

	removeNoise(bin, size);

	char arr[10];
	int index = 0;
	memset(arr,0,10);

	int center = size/2;

	int current = center;
	
	// move to left start
	while(bin[current]==bin[current-1])
		current--;
	current--;

	for(i=0; i<5; i++)
	{
		int combo=1;
		while(bin[current]==bin[current-1]) 
		{
			combo++;
			current--;
		}

		arr[index++]= combo;
		current--;
	}


	current = center;
	
	// move to right start
	while(bin[current]==bin[current+1])
		current++;
	current++;

	for(i=0; i<5; i++)
	{
		int combo=1;
		while(bin[current]==bin[current+1]) 
		{
			combo++;
			current++;
		}

		arr[index++]= combo;
		current++;
	}


	// sort
	for(i=0; i<10; i++)
		for(j=i+1; j<10; j++)
			if(arr[j]>arr[i]) 
			{
				char temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
	
/*	printf("(i) arr=");
	for(i=0; i<10; i++) printf("%d ", arr[i]);
	printf("\n");
*/

	int sum = 0;
	for(i=0; i<4; i++)
		sum+=arr[i];
	
	return sum/4;
}

int getCenter(int gridSize, char threshold)
{
	int i=0;
	int combo = 0;
	int minDiff = 128;
	int minIndex = 0;
	int centerWidth = 0;

	int leftTrim = 0;
	int rightTrim = 0;

	if(gSetCam==SET_CAM_0)
	{
		leftTrim = gLeftTrim0;
		rightTrim = gRightTrim0;
	}
	else if(gSetCam==SET_CAM_1)
	{
		leftTrim = gLeftTrim1;
		rightTrim = gRightTrim1;
	}
	else {}

	//printf("(i) Trim left=%d, right=%d\n", leftTrim, rightTrim);

	for(i=leftTrim; i<rightTrim; i++)
	{
		char value = bin[i];
		if(value==1)
		{
			combo++;
		}
		else
		{
			int difference = abs(gridSize*2 - combo);
			if(difference<minDiff)
			{
				minDiff = difference;
				minIndex = i;
				centerWidth = combo;
			}
			combo=0;
		}
	}

//	int min =0; 
//	for(i=0; i<1920; i++)
//		if(difference[i]<difference[min]) min = i;

	return minIndex-centerWidth/2;
}


int haveToMakeBorder(int current, int next, int edge )
{
	if(edge==EDGE_BLACK_TO_WHITE)
	{
		if(current==0 && next==1) return true;
		else return false;
	}
	else // white to black
	{
		if(current==1 && next==0) return true;
		else return false;
	}
}


int comboTable[1920];
void generateComboTable(int center)
{
	int i = 0;
	int combo = 1;
	memset(comboTable,0,1920*sizeof(int));

	while(i<1919)
	{
		//char currentColor = bin[i];
		//char nextColor = bin[i+1];
		char color = bin[i];

		if(color==bin[i+1])
		{
			combo++;
		}	
		else
		{
			comboTable[i] = combo;
			combo=1;
		}
		i++;
	}
	//printf("center = %d\n", center);
		
	// center bar has double size width on the others. 
	int centerBarSize = 0;
	int indexCenterBar = 0;
	
	for(i=center; i<1920; i++)
	{
		if(comboTable[i]>0) 
		{
			centerBarSize=comboTable[i];
			indexCenterBar = i;
			break;
		}
	}
	
	int halfSize = (int)(centerBarSize/2.0);
	comboTable[center] = halfSize;
	comboTable[indexCenterBar] = halfSize;

/*
	printf("(i)comboTable:");
	for(i=0; i<1920; i++){ if(comboTable[i]>0) printf("%d,", comboTable[i]); }
	printf("\n");
*/

}

void printNumber1(int baseX, int baseY,int pitch, int n, Uint8 * fb)
{
/*
	int x=0, y=0;
	for(y=0; y<FONT_HEIGHT; y++) 
	{
		for(x=0; x<FONT_WIDTH; x++)
		{
			putPixel(fb, baseX+x, baseY+y, pitch, gFont[n][y][x]);
		}
	}
*/
}

void printNumber(int x, int y, int n, Uint8 * fb)
{
/*
	int n1 = n%10;
	int n10 = (int)(((n%100) - n1)/10);	

	printNumber1(x, y, n10, fb);
	printNumber1(x+16, y, n1, fb);
*/
}

void printDot(int baseX, int baseY, Uint8 * fb)
{
/*
	int x=0, y=0;
	for(y=0; y<FONT_HEIGHT; y++)
	{
		for(x=0; x<FONT_WIDTH; x++)
		{
			if( (x==7 && y==19) || (x==8 && y==19) 
				|| (x==7 && y==20) || (x==8 && y==20) )
				putPixel(fb, baseX+x, baseY+y, pitch,0xFF);
			else putPixel(fb, baseX+x, baseY+y, pitch,0x00);
		}
	}
*/
}

void printDouble(int x, int y, double d, Uint8 * fb)
{
/*
	int quotient = ((int)d)%1000;
	int q1 = quotient%10;
	int q10 = (int)((quotient%100)/10);
	int q100 = (int)((quotient%1000)/100);

	int decimal = ((int)(d*1000))%1000;
	//int d1 = decimal%10;
	int d10 = (int)((decimal%100)/10);
	int d100 = (int)((decimal%1000)/100);

	//if(q100>0) 
	printNumber1(x-16*3-16, y, q100, fb);
	printNumber1(x-16*2-16, y, q10, fb);
	printNumber1(x-16*1-16, y, q1, fb);

	printDot(x-16, y, fb);
	
	printNumber1(x, y, d100, fb);
	printNumber1(x+16*1, y, d10, fb);
	//printNumber1(x+16*2, y, d1, fb);
*/
}

void drawLine(Uint8 * fb, int x1, int y1, int x2, int y2, int pitch, char red, char green, char blue)
{
	int x = 0;
	int y = 0;
	int a = 0;
	int min = 0;
	int max = 0;

	if(x1==x2)
	{
		if(y1<y2)
		{
			min = y1;
			max = y2;
		}
		else
		{
			min = y2;
			max = y1;
		}
		for(y=min; y<=max; y++)
			putPixelRGB(fb, x1, y, pitch, red, green, blue);
	}
	else
	{
		a= (y2-y1)/(x2-x1);
		int b = y1-a*x1;

		if(x1<x2)
		{
			min = x1;
			max = x2;
		}
		else
		{
			min = x2;
			max = x1;
		}
		for(x=min; x<=max; x++)
		{
			y = a*x+b;
			putPixelRGB(fb, x, y, pitch, red, green, blue);
		}
	}

}

void render(Uint8 * framebuffer, const char * buf, int size, int pitch)
{
//	printf("(i) render called~~\n");
	int baseLine = 312; //312
	if(gMode==MODE_RUNNING) baseLine = 280; //280

	int gridSize = 0;
	int center = 0;
	if(gMode==MODE_CALIBRATION)
	{
//		clearScreen();
		int threshold = gThreshold0;
		if(gSetCam == SET_CAM_1) threshold = gThreshold1;

		gridSize = getGridSize(buf, size, threshold);
		//printf("(i) grid size = %d\n", gridSize);
	
		center = getCenter(gridSize, threshold);
		//printf("(i) center = %d\n", center);
	}
	
	int x, y;
	char prev = 0;
	int maxHighWidth = 0;
	int counter = 0;
	int risingEdge=0;
	int maxRisingEdge=0;
	int maxFallingEdge=0;

	for(x=0; x<WIDTH_SCREEN; x++)
	{
		int index = (int)(x*7.0588);
		if(index+1>=size) break;

		char value1 = buf[index];
		char value2 = buf[index+1];
		char average = (value1+value2)/2;

		if(gMode==MODE_CALIBRATION)
		{
			for(y=baseLine; y>baseLine-(average*0.6); y--) {
				putPixel(framebuffer, x, y, pitch, 0xFF);
			}

			if(gSetCam==SET_CAM_0)
				putPixelRGB(framebuffer, x, baseLine-(gThreshold0*0.6), pitch, 255, 255, 0);
			else if(gSetCam==SET_CAM_1) 
				putPixelRGB(framebuffer, x, baseLine-(gThreshold1*0.6), pitch, 170, 170, 0);
			else {
				putPixelRGB(framebuffer, x, baseLine-(gThreshold0*0.6), pitch, 255, 255, 0);
				putPixelRGB(framebuffer, x, baseLine-(gThreshold1*0.6), pitch, 170, 170, 0);
			}

		}
		else // running mode screen rendering
		{
			if(x!=0 && prev != average) // draw an edge line
			{
				for(y=baseLine; y>baseLine-80; y--) {
					putPixel(framebuffer, x, y, pitch, 0xFF);
				}
				
			}
			else if(average>128) // draw bottom line
			{
				for(y=baseLine; y>baseLine-80; y--) {
					if(y==baseLine) putPixel(framebuffer, x, y, pitch, 0xFF);
					else putPixel(framebuffer, x, y, pitch, 0x0);
				}
			}
			else	// draw top line
			{
				for(y=baseLine; y>baseLine-80; y--) {
					if(y==baseLine-80+1) putPixel(framebuffer, x, y, pitch, 0xFF);
					else putPixel(framebuffer, x, y, pitch, 0x0);
				}
			}
			if(x==0) {} // do nothing
			else if(prev>average) // rising(light has to be bottom line)
			{
				//printf("risingEdge = %d, counter = %d\n", risingEdge, counter);
				risingEdge = x; 
				counter=1;
			}

			else if(prev==average)
				counter++;
			else // falling edge
			{
				if(counter>maxHighWidth) 
				{
					maxHighWidth = counter;
					maxRisingEdge = risingEdge;
					maxFallingEdge = x;

				}
				counter = 0;
			}
			prev = average;
		}
	}

	// draw running mode screen meta data
	if(gMode==MODE_RUNNING)
	{
		// draw arrow from left rising edge to right falling edge in red color
		int arrowY = baseLine-25;
	
		drawLine(framebuffer, maxRisingEdge, arrowY, maxFallingEdge, arrowY, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, maxRisingEdge, arrowY, maxRisingEdge+5, arrowY-5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, maxRisingEdge, arrowY, maxRisingEdge+5, arrowY+5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, maxFallingEdge, arrowY, maxFallingEdge-5, arrowY-5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, maxFallingEdge, arrowY, maxFallingEdge-5, arrowY+5, pitch, 0xFF, 0x00, 0x00);

		// get material size in mm  
		double widthMaterial = 0;
		int i=0;
		int risingEdge1920 = (int)(maxRisingEdge*7.0588);
		int fallingEdge1920 = (int)(maxFallingEdge*7.0588);

//		printf("(i) risingEdge1920:%d, fallingEdge1920:%d\n", risingEdge1920, fallingEdge1920);
	
		for(i=risingEdge1920; i<fallingEdge1920; i++)
		{
			double len=gCalibrationData.pixelLen[i*2];
			if(len>0) widthMaterial+=len;
			if(i*2+1<3840)
			{
				len = gCalibrationData.pixelLen[i*2+1];
				if(len>0) widthMaterial+=len;
			}
		}

//		printf("(i) widthMaterial:%lf\n", widthMaterial);
		widthmaterial =	widthMaterial;

		// draw material size in mm  on the arrow line upside center
		int centerArrow = (maxFallingEdge-maxRisingEdge)/2+maxRisingEdge;
		printDouble(centerArrow, arrowY-5-21, widthMaterial, framebuffer);
		centerarrow = centerArrow;
		arrow_Y = arrowY-5-21;
		

		/*
		int risingEdge1920 = -1;
		int fallingEdge1920 = -1;
		int i=0;
		for(i=1; i<1919; i++)
		{
			if(buf[i]>buf[i+1]) // rising(light=low, dark=high)
			{
				risingEdge1920 = i;
				break;
			}
		}

		for(i=risingEdge1920+1; i<1919; i++)
		{
			if(buf[i]<buf[i+1]) // falling
			{
				fallingEdge1920 = i;
				break;
			}
		}

		// convert to 800pixel width
		int risingEdge = (int)(risingEdge1920/4);
		int fallingEdge = (int)(fallingEdge1920/4);

		// draw signal line in white color
		drawLine(framebuffer,0, baseLine, risingEdge, baseLine, pitch, 0xFF, 0xFF, 0xFF);
		drawLine(framebuffer,risingEdge, baseLine, risingEdge, baseLine-80, pitch, 0xFF, 0xFF, 0xFF);
		drawLine(framebuffer,risingEdge, baseLine-80, fallingEdge, baseLine-80, pitch, 0xFF, 0xFF, 0xFF);
		drawLine(framebuffer,fallingEdge, baseLine-80, fallingEdge, baseLine, pitch, 0xFF, 0xFF, 0xFF);
		drawLine(framebuffer,fallingEdge, baseLine, 799, baseLine, pitch, 0xFF, 0xFF, 0xFF);

		// draw arrow from left rising edge to right falling edge in red color
		int arrowY = baseLine-25;
	
		drawLine(framebuffer, risingEdge, arrowY, fallingEdge, arrowY, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, risingEdge, arrowY, risingEdge+5, arrowY-5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, risingEdge, arrowY, risingEdge+5, arrowY+5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, fallingEdge, arrowY, fallingEdge-5, arrowY-5, pitch, 0xFF, 0x00, 0x00);
		drawLine(framebuffer, fallingEdge, arrowY, fallingEdge-5, arrowY+5, pitch, 0xFF, 0x00, 0x00);

		// get material size in mm  
		double widthMaterial = -1;
		for(i=risingEdge1920; i<fallingEdge1920; i++)
			widthMaterial+=gCalibrationData.pixelLen[i];
		//printf("(i) width material = %lf\n", widthMaterial);

		
		// draw material size in mm  on the arrow line upside center
		printNumber(480-20-16*2, baseLine+20, gCount0, framebuffer);
		*/
	}

	// draw meta line of calibration mode
	if(gMode==MODE_CALIBRATION)
	{
	    if(gSetCam==SET_CAM_0)
	    {
		x = (int)(gLeftTrim0/7.0588);
		drawVLine(framebuffer, x, pitch, 0, 0, 255);

		x = (int)(gRightTrim0/7.0588);
		drawVLine(framebuffer, x, pitch, 0, 0, 255);

		gCalibrationData.cam0Center = center;
		gCalibrationData.cam0LeftTrim = gLeftTrim0;
		gCalibrationData.cam0RightTrim = gRightTrim0;
		gCalibrationData.cam0Threshold = gThreshold0;

		// generate combo table
		generateComboTable(center);

		// right side of merged image.
		int from = center;
		int to = 1919;
		int combo = 0;
		int i=0;
		//printf("(i) setcam0 center=%d\n", center);
		for(i=to; i>=from; i--)
		{
			if(comboTable[i]>0) combo = comboTable[i];
			if(combo>0)
			{
				gCalibrationData.pixelLen[1920+i-center]=5.0/combo; // 1cell = 5mm
			}
		}
		for(i=0; i<center; i++) gCalibrationData.pixelLen[3840+i-center] = -1;
				
	    }
	    else if(gSetCam==SET_CAM_1)
	    {
		x = (int)(gLeftTrim1/7.0588);
		drawVLine(framebuffer, x, pitch, 0, 0, 255);

		x = (int)(gRightTrim1/7.0588);
		drawVLine(framebuffer, x, pitch, 0, 0, 255);

		gCalibrationData.cam1Center = center;
		gCalibrationData.cam1LeftTrim = gLeftTrim1;
		gCalibrationData.cam1RightTrim = gRightTrim1;
		gCalibrationData.cam1Threshold = gThreshold1;

		// generate combo table
		generateComboTable(center);

		// left side of merged image.
		int from = center; 
		int to = 0; 
		int combo = 0;
		int i=0;
		for(i=from; i>=to; i--)
		{
			if(comboTable[i]>0) combo = comboTable[i];
			if(combo>0)
			{
				gCalibrationData.pixelLen[i+(1919-center)]=5.0/combo; // 1cell = 5mm
			}
		}
		for(i=0; i<1919-center; i++) gCalibrationData.pixelLen[i] = -1;
	    }
	    else 
	    {
		drawVLine(framebuffer, 136, pitch, 255, 0, 0); //calibration center line
	    }
	    x = (int)(center/7.0588);
	   	drawVLine(framebuffer, x, pitch, 255, 0, 0);
	}
	/*		
	if(gMode==MODE_CALIBRATION)
	{
		if(gSetCam == SET_CAM_ALL || gSetCam==SET_CAM_0) {
			printNumber(272-20-16*2, baseLine+20, gCount0, framebuffer);
		}

		if(gSetCam == SET_CAM_ALL || gSetCam==SET_CAM_1)
			printNumber(20, baseLine+20, gCount1, framebuffer);
	}
	*/
}


void fetchFrameRunning(char * vf, 
	const char * buf, int size, int *pIndex, char* frame, char threshold)
{
    char startOfFrame = buf[0];
    char colorData = buf[1];
    int count = ( (buf[2] & 0x7F)<<8) | buf[3]; // MSB not used. 15bit values max

    if(*pIndex>0 && startOfFrame==0x02)
    {
        //printf("(i) frame will be rendereomboTable
        memcpy(vf, frame, 1920);
        *pIndex=0;
	//int i=0;
	//for(i=0; i<1920; i++) printf("%X ", vf[i]);
	//printf("\n");
        return;
    }

    // put pixel on frame
    int i=0;
    for(i=0; i<count; i++)
    {
        if(*pIndex>1920) break;
        else 
	{
		if(colorData> threshold)
			frame[(*pIndex)++] = 0xFF;
		else
			frame[(*pIndex)++] = 0x00;
	}
    }
}

void * thread_function_uart0(void* arg)
{
    int i = 0;
    int index = 0;

    gUartPort0 = uart_open(DEV_CAM_0, 115200);
    if(gUartPort0<0) {
        fprintf(stderr, "Fail to open serial port0\n");
        exit(1);
    }
    printf("Serial Port0 open success!\n");

    setMode(gUartPort0, gMode, gThreshold0);

    char buf[BUF_SIZE] = "";
    char temp[BUF_SIZE] = "";
    memset(buf,0,BUF_SIZE);
    memset(temp,0,BUF_SIZE);

    memset(gVf0,0,BUF_SIZE);

    while(1)
    {
	//printf("uart0\n");
	ssize_t len = read(gUartPort0, temp, BUF_SIZE);
	//printf("len = %d\n", len);
        if(len==-1) {
            fprintf(stderr,"Fail to read serial port0\n");
	    //printf("Serial port0 error!\n");
   	    //nomalInputCounter0 = 0;
            break;
        }
	//else nomalInputCounter0 = 1; 
	//printf("nomal = %d\n", nomalInputCounter0);

        for(i=0; i<len; i++)
        {
            if(temp[i]==0) // new packet 
            {
		pthread_mutex_lock(&gMutex0);

		if(gMode==MODE_RUNNING) 
			fetchFrameRunning(gVf0, 
				buf, index, &g_index0, g_frame0, gThreshold0);
		else fetchFrameCalibration(gVf0, buf, index);

		pthread_mutex_unlock(&gMutex0);

                index = 0;
                continue;
            }

            buf[index++] = temp[i];
        }
    }

    uart_close(gUartPort0);
    printf("(i) UART Port0 closed\n");

    return 0;
}

void * thread_function_uart1(void* arg)
{
    int i = 0;
    int index = 0;

    gUartPort1 = uart_open(DEV_CAM_1, 115200);
    if(gUartPort1<0) {
        fprintf(stderr, "Fail to open serial port1\n");
        exit(1);
    }
    printf("Serial Port1 open success!\n");

    setMode(gUartPort1, gMode, gThreshold1);

    char buf[BUF_SIZE] = "";
    char temp[BUF_SIZE] = "";
    memset(buf,0,BUF_SIZE);
    memset(temp,0,BUF_SIZE);

    memset(gVf1,0,BUF_SIZE);

    while(1)
    {
	ssize_t len = read(gUartPort1, temp, BUF_SIZE);
        if(len==-1) {
            fprintf(stderr,"Fail to read serial port1\n");
   	    nomalInputCounter1 = 0;	
            //break;
        }
	else nomalInputCounter1 = 1; 

        for(i=0; i<len; i++)
        {
            if(temp[i]==0) // new packet 
            {
		pthread_mutex_lock(&gMutex1);

		if(gMode==MODE_RUNNING) 
			fetchFrameRunning(gVf1, buf, 
				index, &g_index1, g_frame1, gThreshold1);
		else fetchFrameCalibration(gVf1, buf, index);

		pthread_mutex_unlock(&gMutex1);

                index = 0;
                continue;
            }

            buf[index++] = temp[i];
        }
    }

    uart_close(gUartPort1);
    printf("(i) UART Port1 closed\n");

    return 0;
}

int countBar(char * buf, int from, int to, unsigned threshold)
{
	int count = 0;
	int i = 0;

	int rised = 0;

	for(i=from; i<to; i++)
	{
		if(buf[i]<threshold && buf[i+1]>=threshold && rised==0) rised=1;

		if(buf[i]>=threshold && buf[i+1]<threshold && rised==1) 
		{
			rised = 0;
			count++;
		}
	}

	return count;
}

void dilationGrey(char * vf, int size)
{
	char tempBuf[size];
	memcpy(tempBuf, vf, size);
	memset(vf, 0, size);

	int i=0;
	for(i=0; i<size; i++)
	{
		char src = tempBuf[i];
		if(src>vf[i]) vf[i] = src;

		if(i-1>=0)
			if(src>vf[i-1])
				vf[i-1] = src;
		
		if(i+1<size)
			if(src>vf[i+1])
				vf[i+1] = src;
	}
}

int getMin(int a, int b, int c)
{
	int min = a<b?a:b;
	return min<c?min:c;
}

void erosionGrey(char * vf, int size)
{
	char tempBuf[1920];
	memcpy(tempBuf, vf, size);
	memset(vf, 0, size);

	int i=0;
	for(i=0; i<size; i++)
	{
		char left=0, center=0, right=0;
		if(i-1>=0) left = tempBuf[i-1];
		center = tempBuf[i];
		if(i+1<size) right = tempBuf[i+1];

		vf[i] = getMin(left,center,right);
	}
}

void filterNoise(char * vf, int size)
{
	dilationGrey(vf, size);
	
	erosionGrey(vf, size);
}

void outputDc(HDC dstDc, HDC srcDc)
{
        int widthSrc, heightSrc, pitchSrc;
        RECT rcSrc = {0, 0, 272, 480};

        Uint8* srcBuf = LockDC (srcDc, &rcSrc, &widthSrc, &heightSrc, &pitchSrc);
        Uint8 r, g, b;
        int x = 0;
        int y = 0;

        static Uint8 table[480][272][4];

        for(y = 0; y<heightSrc; y++)
        {
                for (x = 0; x < widthSrc; x++)
                {
                        r = getPixelR(srcBuf, x, y, pitchSrc);
                        g = getPixelG(srcBuf, x, y, pitchSrc);
                        b = getPixelB(srcBuf, x, y, pitchSrc);
                        table[y][x][0] = r;
                        table[y][x][1] = g;
                        table[y][x][2] = b;
                }
        }
        UnlockDC (srcDc);

        int widthDst, heightDst, pitchDst;
        RECT rcDst = {0, 0, 480, 272};

        Uint8* dstBuf = LockDC (dstDc, &rcDst, &widthDst, &heightDst, &pitchDst);
        int xDst = 0;
        int yDst = 0;

        // Copy the pixels from srcDc to dstDc with clockwise 90 degree rotation
        for(y = 0; y<480; y++)
        {
                for (x = 0; x < 272; x++)
                {
                        r = table[y][x][0];
                        g = table[y][x][1];
                        b = table[y][x][2];

                        yDst = x;
                        xDst = 479-y;

                        putPixelRGB(dstBuf, xDst, yDst, pitchDst, r, g, b);
                }
        }
        UnlockDC (dstDc);
}

void current_Time(char * buff)
{
	struct tm *t;
	time_t now = time(NULL);
	t = localtime(&now);
	sprintf(buff, "%d/%d/%d %d:%d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec);

/*
	time_t tm_time;
	struct tm *st_time;
	
	time(&tm_time);
	st_time = localtime(&tm_time);
	strftime(buff, 255, "%Y/%m/%d %l:%M:%S",st_time);
*/	
}

void getTextout(HDC hdc, HDC vdc)
{
	char buf_count[255];
    	char buf_widthmaterial[255];
	char buff[255];
	char bufIp[255];
	int len = getIpParse(bufIp);
	//printf("ip(%d):%s\n",len, bufIp);
	
	if(gMode==MODE_CALIBRATION)
	{
		if(gSetCam==SET_CAM_0)
		{
			memset(buf_count,0,255);
			sprintf(buf_count,"%d", gCount0);
			TextOut(vdc, 272-20-16, 312+20, buf_count);
			
			SetTextColor(vdc, RGBA2Pixel(hdc, 0xFF, 0xFF, 0xFF, 0xFF));
			TextOut(vdc, 10, 75, bufIp);
			TextOut(vdc, 200, 75, "cam:0");
			
			SetTextColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
			current_Time(buff);
			TextOut(vdc, 10, 455, buff);
		}
		else if(gSetCam==SET_CAM_1)
		{
			memset(buf_count,0,255);
			sprintf(buf_count,"%d", gCount1);
			TextOut(vdc, 20, 312+20, buf_count);

			SetTextColor(vdc, RGBA2Pixel(hdc, 0xFF, 0xFF, 0xFF, 0xFF));
			TextOut(vdc, 10, 75, bufIp);
			TextOut(vdc, 200, 75, "cam:1");

			SetTextColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
			current_Time(buff);
			TextOut(vdc, 10, 455, buff);
		}
		else 
		{
			memset(buf_count,0,255);
			sprintf(buf_count,"%d", gCount0);
			TextOut(vdc, 272-20-16, 312+20, buf_count);

			memset(buf_count,0,255);
			sprintf(buf_count,"%d", gCount1);
			TextOut(vdc, 20, 312+20, buf_count);
			
			SetTextColor(vdc, RGBA2Pixel(hdc, 0xFF, 0xFF, 0xFF, 0xFF));
			TextOut(vdc, 10, 75, bufIp);
			TextOut(vdc, 200, 75, "cam:all");
			
			SetTextColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
			current_Time(buff);
			TextOut(vdc, 10, 455, buff);
		}
	}
	else if(gMode==MODE_RUNNING)
	{
		memset(buf_widthmaterial,0,255);
		sprintf(buf_widthmaterial,"%.3lfmm", widthmaterial);
		SetTextColor(vdc, RGBA2Pixel(hdc, 0xFF, 0xFF, 0x00, 0xFF));
		TextOut(vdc, centerarrow-30, arrow_Y, buf_widthmaterial);

		SetTextColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
		current_Time(buff);
		TextOut(vdc, 10, 455, buff);
	}
}

/* 
void getFillbox(HDC hdc, HDC vdc)
{
	char buff[255];
	memset(buff,0,255);

	SetBrushColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));	
	FillBox(vdc, 0, 0, 272, 80);
	SetBrushColor(vdc, RGBA2Pixel(hdc, 0x6C, 0x6C, 0x6C, 0xFF));	
	FillBox(vdc, 0, 80, 272, 40);
	FillBox(vdc, 0, 480-35, 272-98, 480);
	
	//FillBoxWithBitmap(vdc, 272-98, 480-35, 272, 480, IMAGE_AFACHE);

	SetBrushColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));	
}
*/

void receiveFrame(HWND hWnd)
{

    unsigned long current		= GetNowUs();
    unsigned long prev			= current;
    unsigned long elapsedTime		= 0;	
    unsigned long renderingCounter	= 0;
    unsigned long loadingCounter	= 0;
    unsigned long alertCounter		= 0;

    // Create Mutex for shared image buffer
    pthread_t t_id0, t_id1;
    pthread_mutex_init(&gMutex0, NULL);
    pthread_mutex_init(&gMutex1, NULL);

    // start a thread works on UART 0
    pthread_create(&t_id0, NULL, thread_function_uart0, NULL);
    pthread_detach(t_id0);

    // start a thread works on UART 1
    pthread_create(&t_id1, NULL, thread_function_uart1, NULL);
    pthread_detach(t_id1);

    char vf0[1920];
    char vf1[1920];
    char vfCombined[3840];
    char vfBin[3840];
    char vfResized[1920];

    memset(vf0, 0, 1920);
    memset(vf1, 0, 1920);
    memset(vfCombined, 0 ,3840);
    memset(vfBin, 0 ,3840);
    memset(vfResized, 0 ,1920);

    HDC hdc = CreatePrivateDC(hWnd);
    SetBkMode(hdc, BM_TRANSPARENT);
    SetTextColor(hdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
    SetBrushColor(hdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));

    // Create mem DC
    HDC vdc = CreateMemDC(272, 480, 32, MEMDC_FLAG_HWSURFACE | MEMDC_FLAG_SRCALPHA,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

    SetBkMode(vdc, BM_TRANSPARENT);
    SetTextColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));
    SetBrushColor(vdc, RGBA2Pixel(hdc, 0x00, 0x00, 0x00, 0xFF));

    int width, height, pitch;
    RECT rc = {0, 0, 272, 480};

    while(1)
    {
	current = GetNowUs();
	elapsedTime = current - prev;

	if(	(gMode==MODE_RUNNING	&& elapsedTime>16667)	||
		(gMode==MODE_CALIBRATION&& elapsedTime>1000000)	||
		(gMode==MODE_LOADING	&& elapsedTime>1000000)	||
		(gMode==MODE_ALERT      && elapsedTime>1000000) )
	{
		prev = current;

		if( gMode == MODE_LOADING)
		{
    			drawImage(vdc, 0, 0, 272, 480, IMAGE_LOADING);
			outputDc(hdc, vdc);
			UpdateWindow(hWnd, FALSE);
			loadingCounter++;
			if(loadingCounter>2) {
				gMode = MODE_RUNNING;
			  	if(loadCalibrationData()<0)
      				{
   			        	gMode = MODE_CALIBRATION;
           				gSetCam = SET_CAM_1;
   	    			}
			}
			continue;
		}

		if( gMode == MODE_ALERT)
		{
    			if(alertCounter==0) drawImage(vdc, 0, 0, 272, 480, IMAGE_ALERT_0);
    			else drawImage(vdc, 0, 0, 272, 480, IMAGE_ALERT_1);
			outputDc(hdc, vdc);
			UpdateWindow(hWnd, FALSE);
			alertCounter++;
			if(alertCounter>1) alertCounter = 0;
			continue;
		}

		if( gSetCam == SET_CAM_0 && MODE_CALIBRATION )
		{
			pthread_mutex_lock(&gMutex0);
			memcpy(vf0, gVf0, 1920);
			pthread_mutex_unlock(&gMutex0);

			filterNoise(vf0, 1920);
			gCount0 = countBar(vf0, 0, 1919, gThreshold0);
			
			if(gMode==MODE_RUNNING) renderingCounter++;
			if( gMode==MODE_CALIBRATION || (gMode==MODE_RUNNING&&renderingCounter>=6) )
			{
				renderingCounter=0;
				clearScreen(vdc, 272, 480);
				Uint8* frame_buffer = LockDC (vdc, &rc, &width, &height, &pitch);
				render(frame_buffer, vf0, 1920, pitch);
				UnlockDC (vdc);
				getTextout(hdc, vdc);
				outputDc(hdc, vdc);
			}
		}
		else if( gSetCam == SET_CAM_1 && gMode == MODE_CALIBRATION )
		{
			pthread_mutex_lock(&gMutex1);
			memcpy(vf1, gVf1, 1920);
			pthread_mutex_unlock(&gMutex1);

			filterNoise(vf1, 1920);
			gCount1 = countBar(vf1, 0, 1919, gThreshold1);

			if(gMode==MODE_RUNNING) renderingCounter++;
			if( gMode==MODE_CALIBRATION || (gMode==MODE_RUNNING&&renderingCounter>=6) )
			{
				renderingCounter=0;
				clearScreen(vdc, 272, 480);
				Uint8* frame_buffer = LockDC (vdc, &rc, &width, &height, &pitch);
				render(frame_buffer, vf1, 1920, pitch);
				UnlockDC (vdc);
				getTextout(hdc, vdc);
				outputDc(hdc, vdc);
			}
		}
		else
		{
			pthread_mutex_lock(&gMutex0);
			memcpy(vf0, gVf0, 1920);
			pthread_mutex_unlock(&gMutex0);

			pthread_mutex_lock(&gMutex1);
			memcpy(vf1, gVf1, 1920);
			pthread_mutex_unlock(&gMutex1);

			// generate combined image from vf0 and vf1

			// copy cam1 left side image
			int i=0;
			int center = gCalibrationData.cam1Center;
			//printf("(i) cam1 center=%d\n", center);

			for(i=0; i<1920; i++)
			{
				if(center-i<0) break;

				vfCombined[1919-i] = vf1[center-i];

				if(gMode==MODE_RUNNING)
				{
					if(vf1[center-i]>gCalibrationData.cam1Threshold)
						vfBin[1919-i] = 1;
					else vfBin[1919-i] = 0;
				}
			}
	
			// copy cam0 right side image
			center = gCalibrationData.cam0Center;
			//printf("(i) cam0 center=%d\n", center);
			for(i=0; i<1920; i++)
			{
				if(center+i>=1920) break;

				vfCombined[1920+i] = vf0[center+i];

				if(gMode==MODE_RUNNING)
				{
					if(vf0[center+i]>gCalibrationData.cam0Threshold)
						vfBin[1920+i] = 1;
					else vfBin[1920+i] = 0;
				}
			}

			// get Left Edge Position
			center=gCalibrationData.cam1Center;
			int startIndex = 1920-center+1;
			for(i=startIndex; i<3839; i++)
			{
				if(vfBin[i]==1 && vfBin[i+1]==0) // the material make some shadows.
				{
					gLeftPosition = i+1;
					break;
				}
			}
			//printf("(i) gLeftPosition = %d\n", gLeftPosition);
			
			// get Right Edge Position
			center=gCalibrationData.cam0Center;
			startIndex = 3839-center-1;
			for(i=startIndex; i>0; i--)
			{
				if(vfBin[i]==1 && vfBin[i-1]==0)
				{
					gRightPosition = i-1;
					break;
				}
			}
			//printf("(i) gRightPosition=%d\n", gRightPosition);
			
			// resize to 1/2 scale
			for(i=0; i<1920; i++)
				vfResized[i] = (vfCombined[i*2] + vfCombined[i*2+1])/2.0;

			//printf("(i) vf resized\n");
			
			gCount0 = countBar(vfResized, 1920/2, 1919, gThreshold0);
			gCount1 = countBar(vfResized, 0, 1920/2-1, gThreshold1);

			//printf("(i) render function will be called\n");
			if(gMode==MODE_RUNNING) renderingCounter++;
			if( gMode==MODE_CALIBRATION || (gMode==MODE_RUNNING&&renderingCounter>=6) )
			{
				renderingCounter=0;
				clearScreen(vdc, 272, 480);
				Uint8* frame_buffer = LockDC (vdc, &rc, &width, &height, &pitch);
				render(frame_buffer, vfResized, 1920, pitch);
				UnlockDC (vdc);
				getTextout(hdc, vdc);
				outputDc(hdc, vdc);
			}
		}
	}
	else
	{
		usleep(2000);
	}
    }

    pthread_mutex_destroy(&gMutex0);
    pthread_mutex_destroy(&gMutex1);

    return;
}

void setMode(int uartPort, int mode, int threshold)
{
//	clearScreen();

	printf("(i) setMode function called(%d).\n", mode);
	int len = 0;
	char buf[100];

	buf[0] = 0; // start byte

	if(mode==MODE_RUNNING)
	{
		buf[1] = 1; // run mode
	
	}
	else
	{
		buf[1] = 2; // cal mode
	}

	if(threshold==0) buf[2] = 1; // do not use the start byte
	else buf[2] = threshold;

	len = write(uartPort,buf,3); // send 

	if(len<=0) 
	{
		printf("(!) send control on uart(%d) failed(%d).\n", uartPort, len);
	}
	else 
	{
		gMode = mode;
		printf("(i) send control on user(%d) success(%d).\n", uartPort, len);
	}
}

int getMode()
{
	return gMode;
}

void saveCalibrationData()
{
	json_object * json = json_object_new_object();
	json_object * temp = NULL;
	
	temp = json_object_new_int(gCalibrationData.cam0Center);
	json_object_object_add(json,KEY_CAM0_CENTER, temp); 

	temp = json_object_new_int(gCalibrationData.cam1Center);
	json_object_object_add(json,KEY_CAM1_CENTER, temp); 

	temp = json_object_new_int(gCalibrationData.cam0LeftTrim);
	json_object_object_add(json,KEY_CAM0_LEFT_TRIM, temp); 

	temp = json_object_new_int(gCalibrationData.cam0RightTrim);
	json_object_object_add(json,KEY_CAM0_RIGHT_TRIM, temp); 

	temp = json_object_new_int(gCalibrationData.cam1LeftTrim);
	json_object_object_add(json,KEY_CAM1_LEFT_TRIM, temp); 

	temp = json_object_new_int(gCalibrationData.cam1RightTrim);
	json_object_object_add(json,KEY_CAM1_RIGHT_TRIM, temp); 

	temp = json_object_new_int(gCalibrationData.cam0Threshold);
	json_object_object_add(json,KEY_CAM0_THRESHOLD, temp); 

	temp = json_object_new_int(gCalibrationData.cam1Threshold);
	json_object_object_add(json,KEY_CAM1_THRESHOLD, temp); 

	json_object * array = json_object_new_array();
 	int i=0;	
	for(i=0; i<1920*2; i++)
	{
		temp = json_object_new_double(gCalibrationData.pixelLen[i]);
		json_object_array_add(array, temp);
	}
	json_object_object_add(json,KEY_PIXEL_LEN, array);

	FILE * fp = fopen(FILE_PATH, "w+");
	if(fp!=NULL)
	{
		fprintf(fp, "%s", json_object_to_json_string(json));
		fclose(fp);
	}
		
	json_object_put(json); // delete the new object.
}


void responseUserRequest(int sock, char * msg, int len)
{
	msg[len] = 0;

	// parse JSON packet
	json_object *rootObj;
	json_object *dval;

	/* JSON type의 데이터를 읽는다. */
	rootObj = json_tokener_parse(msg);


	// execute user's request	

	// key existance test
	int exists = 0;
	char buf[BUF_SIZE];
	json_object *response = json_object_new_object();

	// 1. setMode
	exists=json_object_object_get_ex(rootObj,STR_SET_MODE,0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, STR_SET_MODE);
		strcpy(buf,json_object_get_string(dval));
		printf("%s : %s\n", STR_SET_MODE, buf);
		
		if(strcmp(buf,STR_RUNNING)==0)
		{
			setMode(gUartPort0, MODE_RUNNING, gThreshold0);
			setMode(gUartPort1, MODE_RUNNING, gThreshold1);
		}
		else if(strcmp(buf,STR_CALI)==0)
		{
			setMode(gUartPort0, MODE_CALIBRATION, gThreshold0);
			setMode(gUartPort1, MODE_CALIBRATION, gThreshold1);
		}
		else
			printf("(!) setMode value is wrong!\n");
		
		if(getMode()==MODE_RUNNING)	
			json_object_object_add(response,
				STR_SET_MODE, json_object_new_string(STR_RUNNING) );
		else 
			json_object_object_add(response,
				STR_SET_MODE, json_object_new_string(STR_CALI) );
	}

	// 2. getMode
	exists=json_object_object_get_ex(rootObj,"getMode",0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, "getMode");
		printf("getMode : %s\n", json_object_get_string(dval));
	}

	// 3. getCalibrationData
	exists=json_object_object_get_ex(rootObj,STR_GET_CALIBRATION_DATA,0);
	if(exists!=0) {
		printf("%s\n", STR_GET_CALIBRATION_DATA);
		
		FILE * fp = fopen(FILE_PATH, "r");
		if(fp!=NULL)
		{
			fclose(fp);
			json_object_object_add(response,
				STR_GET_CALIBRATION_DATA, json_object_new_string(FILE_PATH));
		}
		else
		{
			json_object_object_add(response,
				STR_GET_CALIBRATION_DATA, json_object_new_string("no file"));
		}
	}

	// 4. saveCalibrationData
	exists=json_object_object_get_ex(rootObj,STR_SAVE_CALIBRATION_DATA,0);
	if(exists!=0) {
		// parse value.
		printf("%s\n", STR_SAVE_CALIBRATION_DATA);
		
		if(getMode()==MODE_CALIBRATION)	
		{
			saveCalibrationData();

			json_object_object_add(response,
				STR_SAVE_CALIBRATION_DATA, json_object_new_string("saved"));
		}
	}

	// 5. eraseCalibrationData
	exists=json_object_object_get_ex(rootObj,STR_ERASE_CALIBRATION_DATA,0);
	if(exists!=0) {
		// parse value.
		printf("%s\n", STR_ERASE_CALIBRATION_DATA);
		
		remove(FILE_PATH);

		json_object_object_add(response,
			STR_ERASE_CALIBRATION_DATA, json_object_new_string("erased"));
	}

	// 6. getImage
	exists=json_object_object_get_ex(rootObj,STR_GET_IMAGE,0);
	if(exists!=0) {
		printf("%s\n", STR_GET_IMAGE);
		gStreamMode = STREAM_MODE_ONESHOT;	

		json_object_put(response); // delete the new object.

		return;
	}

	// 7. setStreaming
	exists=json_object_object_get_ex(rootObj,STR_SET_STREAMING,0);
	if(exists!=0) {
		dval = json_object_object_get(rootObj, STR_SET_STREAMING);
		strcpy(buf, json_object_get_string(dval));

		printf("%s : %s\n", STR_SET_STREAMING, buf);

		if(strcmp(buf, STR_ON) == 0)
			gStreamMode = STREAM_MODE_INFINITE;
		else if(strcmp(buf, STR_OFF) == 0)
			gStreamMode = STREAM_MODE_STOP;
		else 
			printf("(!) %s value is wrong!\n", STR_SET_STREAMING);

		json_object_put(response); // delete the new object.

		return;
	}

	// 8. setCamera
	exists=json_object_object_get_ex(rootObj,"setCamera",0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, STR_SET_CAMERA);
		strcpy(buf, json_object_get_string(dval));

		printf("%s : %s\n", STR_SET_CAMERA, buf);

		if(strcmp(buf, STR_CAM_ALL) == 0)
			gSetCam = SET_CAM_ALL;
		else if( strcmp(buf, STR_CAM_0) == 0 )
			gSetCam = SET_CAM_0;
		else if(strcmp(buf,STR_CAM_1) == 0)
			gSetCam = SET_CAM_1;
		else 
			printf("(!) setCamera value is wrong!\n");

		if(gSetCam==SET_CAM_ALL)	
			json_object_object_add(response,
			    STR_SET_CAMERA, json_object_new_string(STR_CAM_ALL) );
		else if(gSetCam==SET_CAM_0)	
			json_object_object_add(response,
			    STR_SET_CAMERA, json_object_new_string(STR_CAM_0) );
		else 
			json_object_object_add(response,
			    STR_SET_CAMERA, json_object_new_string(STR_CAM_1) );
	
	}

	// 9. setThreashold
	exists=json_object_object_get_ex(rootObj,"setThreshold",0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, STR_SET_THRESHOLD);
		strcpy(buf, json_object_get_string(dval));

		printf("%s : %s\n", STR_SET_THRESHOLD, buf);

		int threshold = atoi(buf);
		if(threshold>=0 && threshold<=255)
		{
			if(gSetCam == SET_CAM_0)
				gThreshold0 = threshold;
			else
				gThreshold1 = threshold;
		}
		else printf("(!) setThreshold value is wrong!\n");

		if(gSetCam == SET_CAM_0)
			sprintf(buf, "%d", gThreshold0);
		else
			sprintf(buf, "%d", gThreshold1);

		json_object_object_add(response,
			STR_SET_THRESHOLD, json_object_new_string(buf) );	
	}

	// 10. setLeftTrim
	exists=json_object_object_get_ex(rootObj,STR_SET_LEFT_TRIM,0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, STR_SET_LEFT_TRIM);
		strcpy(buf, json_object_get_string(dval));

		printf("%s : %s\n", STR_SET_LEFT_TRIM, buf);

		int leftTrim = atoi(buf);
		if(leftTrim>=0 && leftTrim<1920)
		{
			if(gSetCam == SET_CAM_0)
				gLeftTrim0 = leftTrim;
			else
				gLeftTrim1 = leftTrim;
		}
		else printf("(!) setLeftTrim value is wrong!\n");

		if(gSetCam == SET_CAM_0)
			sprintf(buf, "%d", gLeftTrim0);
		else
			sprintf(buf, "%d", gLeftTrim1);

		json_object_object_add(response,
			STR_SET_LEFT_TRIM, json_object_new_string(buf) );	
	}

	// 11. setRightTrim
	exists=json_object_object_get_ex(rootObj,STR_SET_RIGHT_TRIM,0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, STR_SET_RIGHT_TRIM);
		strcpy(buf, json_object_get_string(dval));

		printf("%s : %s\n", STR_SET_RIGHT_TRIM, buf);

		int rightTrim = atoi(buf);
		if(rightTrim>=0 && rightTrim<1920)
		{
			if(gSetCam == SET_CAM_0)
				gRightTrim0 = rightTrim;
			else
				gRightTrim1 = rightTrim;
		}
		else printf("(!) setRightTrim value is wrong!\n");

		if(gSetCam == SET_CAM_0)
			sprintf(buf, "%d", gRightTrim0);
		else
			sprintf(buf, "%d", gRightTrim1);

		json_object_object_add(response,
			STR_SET_RIGHT_TRIM, json_object_new_string(buf) );	
	}
	
	// 12. getPositions
	exists=json_object_object_get_ex(rootObj, STR_GET_POSITIONS, 0);
	if(exists!=0) {

		int LPos = gLeftPosition;
		int RPos = gRightPosition;
		int center = 1919; // center index on pixel_length array in calibration_data.json
		
		getPositions(sock, LPos, RPos, center);
		
		return;
	}


	strcpy(buf, json_object_to_json_string_ext(response, json_flags[0].flag) );
	len = write(sock,buf,strlen(buf)+1);
	if(len>0) printf("(i) server: send response success(%d). %s\n", len, buf);
	else printf("(!) server: send response failed(%d). %s\n", len, buf);

	json_object_put(response); // delete the new object.

	return;
}


float getRelativeMM(int indexCenter, int indexPos)
{
	// exception handling
	if(indexPos==0 || indexCenter==0) return 0;

	float lengthMM = 0;

	// Sumation of all pixels from center to the position
	int from = min(indexCenter, indexPos);
	int to = max(indexCenter, indexPos);

	int i=0;
	for(i=from; i<=to; i++)
	{
		float pixelLengthMM = gCalibrationData.pixelLen[i];	
		lengthMM+=pixelLengthMM;
	}

	// Make decision on the signal
	if(indexPos<indexCenter)
		lengthMM*=-1;

	return lengthMM;	
}

int getPositions(int sock, int LPos, int RPos, int center)
{
//	printf("(i) sendPosition called, sock=%d, LPos=%d, RPos=%d, center=%d\n", sock, LPos, RPos, center);

	int LPos02 = 0;
	int LPos01 = 0;
	int RPos01 = 0;
	int RPos02 = 0;
	
	// exception handling
	if(gMode != MODE_RUNNING)
	{
//		printf("(!)gMode != MODE_RUNNING\n");
		return -1;
	}
	
	if(gSetCam != SET_CAM_ALL)
	{
//		printf("(!)gSetCam != SET_CAM_ALL\n");
		return -1;
	}
	
	if(LPos >= RPos)
	{
//		printf("(!)LPos(%d) >= RPos(%d)\n", LPos, RPos);
		return -1;
	}
	
	// 4 case branch
	if(LPos <= center && RPos <= center)
	{
		LPos02 = LPos;
		LPos01 = RPos;
	}
	else if(LPos<=center && RPos>center)
	{
		LPos01 = LPos;
		RPos01 = RPos;
	}
	else if(LPos>center && RPos>center)
	{
		RPos01 = LPos;
		RPos02 = RPos;
	}
//	printf("(i) 4 positions have been prepared\n");

	// Convert pixel array index to mm length based on center pixel
	double fLPos02=0;
	double fLPos01=0;
	double fRPos01=0;
	double fRPos02=0;

	fLPos02 = getRelativeMM(center, LPos02);
	fLPos01 = getRelativeMM(center, LPos01);
	fRPos01 = getRelativeMM(center, RPos01);
	fRPos02 = getRelativeMM(center, RPos02);
	
	// Packing on JSON and sending
	json_object *response = json_object_new_object();
	
	json_object_object_add(response, STR_LPOS02, json_object_new_double(fLPos02) );
	json_object_object_add(response, STR_LPOS01, json_object_new_double(fLPos01) );
	json_object_object_add(response, STR_RPOS01, json_object_new_double(fRPos01) );
	json_object_object_add(response, STR_RPOS02, json_object_new_double(fRPos02) );

	char buf[256];
	sprintf(buf,"%s\n",json_object_to_json_string_ext(response, json_flags[0].flag) );
	int len = write(sock,buf,strlen(buf)+1);

	json_object_put(response); // delete the new object.

//	if(len>0) printf("(i) server: send response success(%d).n=%d, %s\n", len, strlen(buf), buf);
	if(len<=0)
	{
//		printf("(!) server: getPositions response failed(%d).\n", len);
		return -1;
	}

	return 0;
}


#define __SEND_LENGTH_ONLY__
int sendImage(int sock)
{
//	printf("(i) sendImage called, sock=%d, gSetCam=%d\n", sock, gSetCam);

	json_object *response = json_object_new_object();
	char buf[1920*4];
	char vfb[1920];
	int bufsize = 1920;

	int left, right;
	double lengthmm[4];
	
	memset(vfb, 0 ,1920);


	if( gSetCam == SET_CAM_0 )
	{


		pthread_mutex_lock(&gMutex0);
		memcpy(vfb, gVf0, 1920);
		pthread_mutex_unlock(&gMutex0);

		filterNoise(vfb, 1920);
	}
	else if( gSetCam == SET_CAM_1 )
	{
		

		pthread_mutex_lock(&gMutex1);
		memcpy(vfb, gVf1, 1920);
		pthread_mutex_unlock(&gMutex1);

		filterNoise(vfb, 1920);
	}
	else
	{
		bufsize = 1920;
	    
		char vf0[1920];
		char vf1[1920];
    		memset(vf0, 0 ,1920);
    		memset(vf1, 0 ,1920);
		

		pthread_mutex_lock(&gMutex0);
		memcpy(vf0, gVf0, 1920);
		pthread_mutex_unlock(&gMutex0);

		pthread_mutex_lock(&gMutex1);
		memcpy(vf1, gVf1, 1920);
		pthread_mutex_unlock(&gMutex1);

		// generate combined image from vf0 and vf1

		// copy cam1 left side image
		int i=0;
		int center = gCalibrationData.cam1Center;
		
		for(i=0; i<1920/2; i++)
		{
			if(center-i*2-1<0) break;
			vfb[1920/2-i] = (vf1[center-i*2] + vf1[center-i*2-1])/2;
		}

		// copy cam0 right side image
		center = gCalibrationData.cam0Center;
		for(i=0; i<1920/2; i++)
		{
			if(center+i*2+1>=1920) break;
			vfb[1920/2+i] = (vf0[center+i*2] + vf0[center+i*2+1])/2;
		}

		filterNoise(vfb, 1920);

	}

	#ifdef __SEND_LENGTH_ONLY__

	lengthmm[0] = 0;
	lengthmm[1] = 0;
	lengthmm[2] = 0;
	lengthmm[3] = 0;

	int state = 0;

	bufsize = 4;
	//cpc가 아닐 때
	
	if(vfb[960] > 128)
	{
		state = 0;

		//scan left
		for(left=0; left < 960; left++)
		{
			if(state == 0)
			{
				if(vfb[960 - left] > 128) 
					lengthmm[2] += gCalibrationData.pixelLen[960 - left];
				else
					state = 1;
			}

			if (state == 1)
			{
				if(vfb[960 - left] < 128) 
					lengthmm[3] += gCalibrationData.pixelLen[960 - left];
				else
					break;
			}			
		}

		state = 0;
		
		if( lengthmm[2] > 90 ) //no material on left side, scan right
		{
			
			lengthmm[2] = 0;
			lengthmm[3] = 0;

			for(right=0; right < 960; right++)
			{
				if(state == 0)
				{
					if(vfb[960 + right] > 128) 
						lengthmm[2] += gCalibrationData.pixelLen[960 + right];
					else
						state = 1;
				}

				if (state == 1)
				{
					if(vfb[960 + right] < 128) 
						lengthmm[3] += gCalibrationData.pixelLen[960 + right];
					else
						break;
				}			
			}
		}

		if(lengthmm[2] > 90) // nothing both side
		{
			lengthmm[2] = 0;
			lengthmm[3] = 0;
		}
			
	}
	else // cpc
	{
		for(left=0; left < 960; left++)
		{
			if(vfb[960 - left] > 128) break;
			lengthmm[0] += gCalibrationData.pixelLen[960 - left];	
		}

		
		for(right=0; right < 960; right++)
		{
			if(vfb[960 + right] > 128) break;
			lengthmm[1] +=  gCalibrationData.pixelLen[960 + right ];
		}
	}

	lengthmm[0] *= 2;
	lengthmm[1] *= 2;
	lengthmm[2] *= 2;
	lengthmm[3] *= 2;

	
	json_object * array = json_object_new_array();
 	int i=0;	
	for(i=0; i<bufsize; i++)
	{
		json_object *temp = json_object_new_double(lengthmm[i]);
		json_object_array_add(array, temp);
	}
	json_object_object_add(response,STR_GET_IMAGE, array);


	
	#else

	

//	printf("(i) vfb prepared\n");

	json_object * array = json_object_new_array();
 	int i=0;	
	for(i=0; i<bufsize; i++)
	{
		json_object * temp = json_object_new_int(vfb[i]);
		json_object_array_add(array, temp);
	}
	json_object_object_add(response, STR_GET_IMAGE, array);

	#endif

	sprintf(buf,"%s\n",json_object_to_json_string_ext(response, json_flags[0].flag) );
	int len = write(sock,buf,strlen(buf)+1);

	json_object_put(response); // delete the new object.

//	if(len>0) printf("(i) server: send response success(%d).n=%d, %s\n", len, strlen(buf), buf);
	if(len<=0) 
	{
		printf("(!) server: sendImage response failed(%d).\n", len);
		return -1;
	}

	return 0;
}

void* thread_function_streaming(void* arg)
{
	fprintf(stderr,"(i) streaming thread started.\n");

	int sock = *((int*)arg);
	int LPos = 0;
	int RPos = 0;
	int center = 1919;	

	long prev = GetNowUs();
	while(1)
	{
		LPos = gLeftPosition;
		RPos = gRightPosition;
		long current = GetNowUs();
		long elapsedTime = current - prev;
		if( (gMode==MODE_RUNNING && elapsedTime>16667) ||
			(gMode==MODE_CALIBRATION && elapsedTime>1000000) )
		{
			prev = current;

			if(gStreamMode==STREAM_MODE_STOP) continue;

			// if(sendImage(sock) == -1)  break;
			getPositions(sock,LPos,RPos,center);

			if(gStreamMode==STREAM_MODE_ONESHOT)
				gStreamMode = STREAM_MODE_STOP;

		}
		else
		{
			usleep(2000);
		}
	}
	
	printf("(i) thread_function_stream will be returned\n");

	return 0;
}


void* thread_function_client(void* arg)
{
	fprintf(stderr,"(i) client  thread started.\n");

	int sock = *((int*)arg);

	pthread_t t_id;
	pthread_create(&t_id, NULL, thread_function_streaming, arg);
	pthread_detach(t_id);

	int str_len = 0;
	char msg[BUF_SIZE];
	memset(msg,0,BUF_SIZE);

	while( (str_len=read(sock, msg, BUF_SIZE))!=0)
	{
		fprintf(stderr,"(i)read(%d):%s\n", sock, msg);
		responseUserRequest(sock,  msg, str_len);
	}

	gStreamMode = STREAM_MODE_STOP;

	fprintf(stderr,"(i) client connection closed.\n");

	close(sock);
	fprintf(stderr,"(i) client thread will be returned.\n");
	
	return 0;
}

void* thread_function_server(void * arg)
{
	fprintf(stderr,"(i) server thread started.\n");

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id1;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(7076);
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr,
		sizeof(serv_adr)) == -1)
	{
		fprintf(stderr,"(!) bind error!\n");
		return 0;
	}

	if(listen(serv_sock, 5) ==-1) 
	{
		fprintf(stderr,"(!) listen error!\n");
		return 0;
	}

	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, 
			(struct sockaddr*)&clnt_adr, (void*)&clnt_adr_sz);

		fprintf(stderr,"(i) accepted.\n");
		
		pthread_create(&t_id1, NULL,
			thread_function_client, (void*)&clnt_sock);
		pthread_detach(t_id1);

	}

	return 0;
}

void startServerThread() // start TCP Server on a thread.	
{
	pthread_t t_id;

	// make a thread and run
	pthread_create(&t_id, NULL, thread_function_server, NULL);

	// detach the thread
	pthread_detach(t_id);
}

int loadCalibrationData()
{
	FILE * fp = fopen(FILE_PATH, "r");
	if(fp==NULL) return -1;

	char buf[100000];

	fread(buf, 100000, 1, fp);	

	json_object * json = json_tokener_parse(buf);
	json_object * temp = NULL;
	
	temp= json_object_object_get(json,KEY_CAM0_CENTER); 
	gCalibrationData.cam0Center = json_object_get_int(temp);

	temp= json_object_object_get(json,KEY_CAM1_CENTER); 
	gCalibrationData.cam1Center = json_object_get_int(temp);

	temp= json_object_object_get(json,KEY_CAM0_LEFT_TRIM); 
	gCalibrationData.cam0LeftTrim = json_object_get_int(temp);
	gLeftTrim0 = gCalibrationData.cam0LeftTrim;

	temp= json_object_object_get(json,KEY_CAM0_RIGHT_TRIM); 
	gCalibrationData.cam0RightTrim = json_object_get_int(temp);
	gRightTrim0 = gCalibrationData.cam0RightTrim;

	temp= json_object_object_get(json,KEY_CAM1_LEFT_TRIM); 
	gCalibrationData.cam1LeftTrim = json_object_get_int(temp);
	gLeftTrim1 = gCalibrationData.cam1LeftTrim;

	temp= json_object_object_get(json,KEY_CAM1_RIGHT_TRIM); 
	gCalibrationData.cam1RightTrim = json_object_get_int(temp);
	gRightTrim1 = gCalibrationData.cam1RightTrim;

	temp= json_object_object_get(json,KEY_CAM0_THRESHOLD); 
	gCalibrationData.cam0Threshold = json_object_get_int(temp);
	gThreshold0 = gCalibrationData.cam0Threshold;

	temp= json_object_object_get(json,KEY_CAM1_THRESHOLD); 
	gCalibrationData.cam1Threshold = json_object_get_int(temp);
	gThreshold1 = gCalibrationData.cam1Threshold;

	temp = json_object_object_get(json, KEY_PIXEL_LEN);
	int len = json_object_array_length(temp);

 	int i=0;	
	for(i=0; i<len; i++)
	{
		json_object * element = json_object_array_get_idx(temp, i);
		gCalibrationData.pixelLen[i] = json_object_get_double(element);
	}

	fclose(fp);
	return 0;
}

void loadFont()
{
	memset(gFont, 0, FONT_COUNT*FONT_HEIGHT*FONT_WIDTH);

	int n, x, y;
	for(n=0; n<FONT_COUNT; n++)
	{
		char strPath[255];
		sprintf(strPath, "/root/font/%d.dat", n);
		FILE* fp = fopen(strPath, "r");
		if(fp==NULL)
		{
			printf("(!) font file not found!\n");
			return;
		}

		for(y=0; y<FONT_HEIGHT; y++)
		{
			for(x=0; x<FONT_WIDTH; x++)
			{
				unsigned char ch = fgetc(fp);
				gFont[n][y][x] = ch;
			}
		}
		fclose(fp);
	}
}

void* thread_function_receiver(void * arg)
{
    HWND hWnd =*((HWND *)arg);
    receiveFrame(hWnd); // infinite loop

    return 0;
}

int startReceiverThread(void * arg) // start Serial Receiver on a thread.	
{
	pthread_t t_id;

	// make a thread and run
	pthread_create(&t_id, NULL, thread_function_receiver, arg);

	// detach the thread
	pthread_detach(t_id);
	
	return 0;
}

static int LcdWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_PAINT:
        break;

        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}


static void InitCreateInfo (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_VISIBLE;
    pCreateInfo->dwExStyle = 0;
    pCreateInfo->spCaption = "splice centering";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor (0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = LcdWinProc;
    pCreateInfo->lx = 0; 
    pCreateInfo->ty = 0; 
    pCreateInfo->rx = 480; 
    pCreateInfo->by = 272; 
    pCreateInfo->iBkColor = PIXEL_black; 
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

int loadBitmap(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);

	if(LoadBitmap(hdc, &bitmap[IMAGE_LOADING], FILE_PATH_IMAGE_LOADING)) return -1;
	if(LoadBitmap(hdc, &bitmap[IMAGE_ALERT_0], FILE_PATH_IMAGE_ALERT_0)) return -1;
	if(LoadBitmap(hdc, &bitmap[IMAGE_ALERT_1], FILE_PATH_IMAGE_ALERT_1)) return -1;
	if(LoadBitmap(hdc, &bitmap[IMAGE_BG], FILE_PATH_IMAGE_BG)) return -1;
	if(LoadBitmap(hdc, &bitmap[IMAGE_BG_CALI], FILE_PATH_IMAGE_BG_CALI)) return -1;
	if(LoadBitmap(hdc, &bitmap[IMAGE_BG_RUN], FILE_PATH_IMAGE_BG_RUN)) return -1;

	ReleaseDC(hdc);
	
	return 0;
}

int getIpParse(char * buf)
{
	FILE* fp = popen("ifconfig", "r");

	if(fp==NULL)
	{
		printf("(i)ifconfig error\n");
	}

	char temp[1024];
	int len = fread( temp, 1, 1024, fp );

	pclose(fp);

	char *start = strstr(temp, "inet addr:");
	if(start==NULL) {
		printf("(i)IP address not found!\n");
		return 0;
	}
	
	start+=10;
	char *end = strstr(start, "  ");
	*end = 0;

	strcpy(buf,start);
	
	//printf("(getIpPsrase)IP:%s\n", buf);

	return len;
}

void unloadBitmap(HWND hWnd)
{
	UnloadBitmap(&bitmap[IMAGE_LOADING]);
	UnloadBitmap(&bitmap[IMAGE_ALERT_0]);
	UnloadBitmap(&bitmap[IMAGE_ALERT_1]);
	UnloadBitmap(&bitmap[IMAGE_BG_CALI]);
	UnloadBitmap(&bitmap[IMAGE_BG_RUN]);
	UnloadBitmap(&bitmap[IMAGE_BG]);
}

int MiniGUIMain(int args, const char* argp[])
{  
    memset(&gCalibrationData, 0, sizeof(CalibrationData));
/*
    if(loadCalibrationData()<0)
    {
	gMode = MODE_CALIBRATION;
	gSetCam = SET_CAM_1;
    }
*/
    // init GUI Window  

    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER, arg[0], 0, 0);
#endif

    InitCreateInfo (&CreateInfo);

    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;

    if(loadBitmap(hMainWnd)==-1) 
    {
	printf("(!) bitmap files not found!\n");
        MainWindowThreadCleanup (hMainWnd);
	return -1;
    }

    startServerThread();	// start TCP Server on a thread
    startReceiverThread((void*)&hMainWnd); // start Serial Receiver on a thread 
 
    while (GetMessage (&Msg, hMainWnd)) {
        DispatchMessage (&Msg);
    }

    unloadBitmap(hMainWnd);

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}  



