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

#define BUF_SIZE        4096
#define WIDTH_SCREEN    800
#define HEIGHT_SCREEN   480
#define WIDTH_IMAGE     1920

#define DEV_CAM_0	"/dev/ttyS0"
#define DEV_CAM_1	"/dev/ttyS2"

extern int uart_open(char *devname, int baud);
extern void  uart_close( int fd );

#define MODE_RUNNING 		0
#define MODE_CALIBRATION 	1
#define STR_RUN_MODE		"0"
#define STR_CAL_MODE		"1"

char * gFb = NULL;
int gMode = MODE_RUNNING;
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
int gStartStream = 0;

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
	double pixelLen[1920];
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

void setMode(int uartPort, int mode, int threshold);

void clearScreen()
{
	memset(gFb, 0, 800*480*4);// clear screen
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

void putPixelRGB(char * framebuffer, int x, int y, char r, char g, char b)
{
	int offset = (WIDTH_SCREEN * y + x)*4;
	framebuffer[offset]  =b;//B
	framebuffer[offset+1]=g;//G
	framebuffer[offset+2]=r;//R
	framebuffer[offset+3]=255;//A
}

void drawVLine(char * framebuffer, int x, char r, char g , char b )
{
	int i=0;
	for(i=0; i<480; i++)
	{
		putPixelRGB(framebuffer, x, i, r, g, b);
	}
}

void putPixel(char * framebuffer, int x, int y, char grey)
{
	int offset = (WIDTH_SCREEN * y + x)*4;
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
	int i = 0, j=0;
	for(i=0; i<1920; i++)
		if(buf[i]>threshold) bin[i] = 1;
		else bin[i] = 0;

	removeNoise(bin, 1920);

	char arr[10];
	int index = 0;
	memset(arr,0,10);

	int center = 1920/2;

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

int comboTable[1920]; 
void generateComboTable()
{
	int i = 0;
	int combo = 1;
	memset(comboTable,0,1920*sizeof(int));
	while(i<1920)
	{
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
}

void printNumber1(int baseX, int baseY, int n, char * fb)
{
	int x=0, y=0;
	for(y=0; y<FONT_HEIGHT; y++)
	{
		for(x=0; x<FONT_WIDTH; x++)
		{
			putPixel(fb, baseX+x, baseY+y, gFont[n][y][x]);
		}
	}
}

void printNumber(int x, int y, int n, char * fb)
{
	int n1 = n%10;
	int n10 = (int)(((n%100) - n1)/10);	

	printNumber1(x, y, n10, fb);
	printNumber1(x+16, y, n1, fb);
}


void render(char * framebuffer, const char * buf, int size)
{
	int baseLine = 368;
	if(gMode==MODE_RUNNING) baseLine = 280;

	int gridSize = 0;
	int center = 0;
	if(gMode==MODE_CALIBRATION)
	{
		clearScreen();
		int threshold = gThreshold0;
		if(gSetCam == SET_CAM_1) threshold = gThreshold1;

		gridSize = getGridSize(buf, size, threshold);
		//printf("(i) grid size = %d\n", gridSize);
	
		center = getCenter(gridSize, threshold);
		//printf("(i) center = %d\n", center);
	}

	int x, y;
	for(x=0; x<WIDTH_SCREEN; x++)
	{
		int index = (int)(x*2.4);
		if(index+1>=size) break;

		char value1 = buf[index];
		char value2 = buf[index+1];
		char average = (value1+value2)/2;

		if(gMode==MODE_CALIBRATION)
		{
			for(y=baseLine; y>baseLine-average; y--)
				putPixel(framebuffer, x, y, 0xFF);

			if(gSetCam==SET_CAM_0)
				putPixelRGB(framebuffer, x, baseLine-gThreshold0, 255, 255, 0);
			else if(gSetCam==SET_CAM_1)
				putPixelRGB(framebuffer, x, baseLine-gThreshold1, 170, 170, 0);
			else {
				putPixelRGB(framebuffer, x, baseLine-gThreshold0, 255, 255, 0);
				putPixelRGB(framebuffer, x, baseLine-gThreshold1, 170, 170, 0);
			}
		}
		else
		{
			for(y=baseLine; y>baseLine-80; y--)
				putPixel(framebuffer, x, y, average);

			/*
			if(average>128)
			{
				for(y=baseLine; y>baseLine-80; y--)
					putPixel(framebuffer, x, y, 0xFF);
			}
			else
			{
				for(y=baseLine; y>baseLine-80; y--)
					putPixel(framebuffer, x, y, 0x00);
			}
			*/
		}
	}

	if(gMode==MODE_CALIBRATION)
	{
	    if(gSetCam==SET_CAM_0)
	    {
		x = (int)(gLeftTrim0/2.4);
		drawVLine(framebuffer, x, 0, 0, 255);

		x = (int)(gRightTrim0/2.4);
		drawVLine(framebuffer, x, 0, 0, 255);

		gCalibrationData.cam0Center = center;
		gCalibrationData.cam0LeftTrim = gLeftTrim0;
		gCalibrationData.cam0RightTrim = gRightTrim0;
		gCalibrationData.cam0Threshold = gThreshold0;

		// generate combo table
		generateComboTable();

		// right side of merged image.
		int from = center;
		int to = 1919;
		int combo = 0;
		int i=0;
		for(i=to; i>=from; i--)
		{
			if(comboTable[i]>0) combo = comboTable[i];
			if(combo>0)
			{
				gCalibrationData.pixelLen[i]=5.0/combo; // 1cell = 5mm
			}
		}
				
	    }
	    else if(gSetCam==SET_CAM_1)
	    {
		x = (int)(gLeftTrim1/2.4);
		drawVLine(framebuffer, x, 0, 0, 255);

		x = (int)(gRightTrim1/2.4);
		drawVLine(framebuffer, x, 0, 0, 255);

		gCalibrationData.cam1Center = center;
		gCalibrationData.cam1LeftTrim = gLeftTrim1;
		gCalibrationData.cam1RightTrim = gRightTrim1;
		gCalibrationData.cam1Threshold = gThreshold1;

		// generate combo table
		generateComboTable();

		// right side of merged image.
		int from = center; 
		int to = 0; 
		int combo = 0;
		int i=0;
		for(i=from; i>=to; i--)
		{
			if(comboTable[i]>0) combo = comboTable[i];
			if(combo>0)
			{
				gCalibrationData.pixelLen[i]=5.0/combo; // 1cell = 5mm
			}
		}
	    }
	    else 
	    {
		drawVLine(framebuffer, 400, 255, 0, 0);
	    }

	    x = (int)(center/2.4);
	    drawVLine(framebuffer, x, 255, 0, 0);
	}

	if(gMode==MODE_CALIBRATION)
	{
		if(gSetCam == SET_CAM_ALL || gSetCam==SET_CAM_0)
			printNumber(800-20-16*2, baseLine+20, gCount0, framebuffer);

		if(gSetCam == SET_CAM_ALL || gSetCam==SET_CAM_1)
			printNumber(20, baseLine+20, gCount1, framebuffer);
	}
}


void fetchFrameRunning(char * vf, 
	const char * buf, int size, int *pIndex, char* frame, char threshold)
{
    char startOfFrame = buf[0];
    char colorData = buf[1];
    int count = ( (buf[2] & 0x7F)<<8) | buf[3]; // MSB not used. 15bit values max

    if(*pIndex>0 && startOfFrame==0x02)
    {
        //printf("(i) frame will be rendered\n");
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
        ssize_t len = read(gUartPort0, temp, BUF_SIZE);
        if(len==-1) {
            fprintf(stderr,"Fail to read serial port0\n");
            break;
        }

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
            break;
        }

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

	erosionGrey(vf, size);
	
	dilationGrey(vf, size);
}

void receiveFrame(char* fb)
{

    unsigned long current = GetNowUs();
    unsigned long prev = current;
    unsigned long elapsedTime = 0;	

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
    char vfCombined[1920];

    memset(vf0, 0, 1920);
    memset(vf1, 0, 1920);
    memset(vfCombined, 0 ,1920);



    while(1)
    {
	current = GetNowUs();
	elapsedTime = current - prev;
	if( (gMode==MODE_RUNNING && elapsedTime>16667) ||
		(gMode==MODE_CALIBRATION && elapsedTime>1000000) )
	{
		prev = current;

		if( gSetCam == SET_CAM_0 )
		{
			pthread_mutex_lock(&gMutex0);
			memcpy(vf0, gVf0, 1920);
			pthread_mutex_unlock(&gMutex0);

			filterNoise(vf0, 1920);
			gCount0 = countBar(vf0, 0, 1919, gThreshold0);
		
			render(fb, vf0, 1920);
		}
		else if( gSetCam == SET_CAM_1 )
		{
			pthread_mutex_lock(&gMutex1);
			memcpy(vf1, gVf1, 1920);
			pthread_mutex_unlock(&gMutex1);

			filterNoise(vf1, 1920);
			gCount1 = countBar(vf1, 0, 1919, gThreshold1);

			render(fb, gVf1, 1920);
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
			for(i=0; i<1920/2; i++)
			{
				if(center-i*2-1<0) break;
				vfCombined[1920/2-i] = (vf1[center-i*2] + vf1[center-i*2-1])/2;
			}

			// copy cam0 right side image
			center = gCalibrationData.cam0Center;
			for(i=0; i<1920/2; i++)
			{
				if(center+i*2+1>=1920) break;
				vfCombined[1920/2+i] = (vf0[center+i*2] + vf0[center+i*2+1])/2;
			}

			filterNoise(vfCombined, 1920);
			gCount0 = countBar(vfCombined, 1920/2, 1919, gThreshold0);
			gCount1 = countBar(vfCombined, 0, 1920/2-1, gThreshold1);

			render(fb, vfCombined, 1920);
		}
	}
    }

    pthread_mutex_destroy(&gMutex0);
    pthread_mutex_destroy(&gMutex1);

    return;
}

void setMode(int uartPort, int mode, int threshold)
{
	memset(gFb, 0, 800*480*4);// clear screen

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
	for(i=0; i<1920; i++)
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
	exists=json_object_object_get_ex(rootObj,"getImage",0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, "getImage");
		printf("getImage : %s\n", json_object_get_string(dval));
	}

	// 7. setStreaming
	exists=json_object_object_get_ex(rootObj,"setStreaming",0);
	if(exists!=0) {
		// parse value.
		dval = json_object_object_get(rootObj, "setStreaming");
		printf("setStreaming : %s\n", json_object_get_string(dval));
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

	strcpy(buf, json_object_to_json_string_ext(response, json_flags[0].flag) );
	len = write(sock,buf,strlen(buf));
	if(len>0) printf("(i) server: send response success(%d). %s\n", len, buf);
	else printf("(!) server: send response failed(%d). %s\n", len, buf);

	json_object_put(response); // delete the new object.

	return;
}

int sendImage(int sock, int cam)
{
	int len = 0;

	if(cam==SET_CAM_ALL)
		len = write(sock, bufCamAll, BUF_CAM_SIZE);
	else if(cam==SET_CAM_0)
		len = write(sock, bufCam0, BUF_CAM_SIZE);
	else if(cam==SET_CAM_1)
		len = write(sock, bufCam1, BUF_CAM_SIZE);
	else
		fprintf(stderr,"(!) cam number is wrong.\n");
	return len;
}

void* thread_function_client(void* arg)
{
	fprintf(stderr,"(i) client  thread started.\n");

	int sock = *((int*)arg);
	int str_len = 0;
	char msg[BUF_SIZE];
	memset(msg,0,BUF_SIZE);

	while( (str_len=read(sock, msg, BUF_SIZE))!=0)
	{
		fprintf(stderr,"(i)read(%d):%s\n", sock, msg);
		responseUserRequest(sock,  msg, str_len);
	}
	close(sock);
	
	return 0;
}

void* thread_function_streaming(void* arg)
{
	fprintf(stderr,"(i) streaming thread started.\n");

	int sock = *((int*)arg);

	while(1)
	{
		sleep(1);
		if(gStartStream==0) continue;

		sendImage(sock, gSetCam);
	}

	return 0;
}

void* thread_function_server(void * arg)
{
	fprintf(stderr,"(i) server thread started.\n");

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id1, t_id2;

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

		pthread_create(&t_id2, NULL,
			thread_function_streaming, (void*)&clnt_sock);
		pthread_detach(t_id2);
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
	FILE * fp = fopen(FILE_PATH, "r+");
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
		sprintf(strPath, "./font/%d.dat", n);
		FILE* fp = fopen(strPath, "r+");
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

int main( int argc, char* argv[] )  
{  
/*
    if(argc!=2) {
        perror("Useage: lcd <Serial Port path>\n");
        perror("Example: lcd /dev/ttyS2\n");
        exit(1);
    }
*/

    loadFont();


    memset(&gCalibrationData, 0, sizeof(CalibrationData));

    if(loadCalibrationData()<0)
    {
	gMode = MODE_CALIBRATION;
	gSetCam = SET_CAM_1;
    }


    char strFb[255]="/dev/fb0";

    int framebuffer_fd = 0;  
    struct fb_var_screeninfo framebuffer_variable_screeninfo;  
    struct fb_fix_screeninfo framebuffer_fixed_screeninfo;  
                    
    framebuffer_fd = open( strFb, O_RDWR );  
    if ( framebuffer_fd <  0 ){  
        perror( "Error: cannot open framebuffer device\n" );  
        exit(1);  
    }  
    printf("frame buffer open success!:%s\n", strFb);


    if ( ioctl(framebuffer_fd, FBIOGET_VSCREENINFO,   
        &framebuffer_variable_screeninfo) )  
    {  
        perror( "Error: reading variable screen infomation\n" );  
        exit(1);  
    }  
    printf("(i) ioctl get v screen info success");


    printf("(i) xres = %d\n", framebuffer_variable_screeninfo.xres);
    printf("(i) yres = %d\n", framebuffer_variable_screeninfo.yres);
    printf("(i) xres_virtual = %d\n", framebuffer_variable_screeninfo.xres_virtual);
    printf("(i) yres_virtual = %d\n", framebuffer_variable_screeninfo.yres_virtual);
    printf("(i) bits_per_pixel = %d\n", framebuffer_variable_screeninfo.bits_per_pixel );
    printf("(i) height = %d\n", framebuffer_variable_screeninfo.height);
    printf("(i) width = %d\n", framebuffer_variable_screeninfo.width);

    printf("(i) ioctl put v screen info success");

    if ( ioctl(framebuffer_fd, FBIOGET_FSCREENINFO,   
        &framebuffer_fixed_screeninfo) )  
    {  
        perror( "Error: reading fixed screen infomation\n" );  
        exit(1);  
    }  

    printf("(i) smem_len = %d\n", framebuffer_fixed_screeninfo.smem_len);
    printf("(i) type = %d\n", framebuffer_fixed_screeninfo.type);
    printf("(i) mmio_len = %d\n", framebuffer_fixed_screeninfo.mmio_len);
               
    printf( "framebuffer Display information\n" );  
    printf( " %d x %d  %d bpp\n", 
        framebuffer_variable_screeninfo.xres,  
        framebuffer_variable_screeninfo.yres,   
        framebuffer_variable_screeninfo.bits_per_pixel );  
                                                   
    int width  = framebuffer_variable_screeninfo.xres;  
    int height = framebuffer_variable_screeninfo.yres;  
    int bpp     = framebuffer_variable_screeninfo.bits_per_pixel/8;  
    int xoffset = framebuffer_variable_screeninfo.xoffset;  
    int yoffset = framebuffer_variable_screeninfo.yoffset;  

    printf( " width:%d , height: %d , bpp: %d, xoffset: %d, yoffset:%d \n",
            width, height, bpp, xoffset, yoffset);

    long int screensize = width*height*bpp;  
                                                                            
    char *framebuffer_pointer = (char*)mmap( 0, screensize,  
        PROT_READ|PROT_WRITE,  MAP_SHARED,  framebuffer_fd, 0 );  
                                                                
    if ( framebuffer_pointer == MAP_FAILED )  
    {  
        perror( "Error : mmap\n" );  
        exit(1);  
    }  
    else  
    {  
	gFb = framebuffer_pointer;
	clearScreen();

	startServerThread(); // start TCP Server on a thread.	
        receiveFrame(framebuffer_pointer); // infinite loop
    }   
    munmap( framebuffer_pointer, screensize );   

    close( framebuffer_fd );  

    return 0;
}  



