#ifndef __SPLICE_LIBS_H__
#define __SPLICE_LIBS_H__
#define WEB_MAJOR_VERSION                  0
#define WEB_MINOR_VERSION                  60
#define SPLICE_MAJOR_VERSION			0
#define SPLICE_MINOR_VERSION			50
#define CAMERA_MAJOR_VERSION			0
#define CAMERA_MINOR_VERSION			0

#define SPLICE_MAX_NUM_CLIENT 128
#define SPLICE_SERVER_PORT  6001
#define SPLICE_MAX_NUM_CPUS          8
#define SPLICE_IRQ_NAME_LENGTH  64
#define SPLICE_IRQ_MAX_TYPES    120 /* number of different interrupts listed in /proc/interrupts */
#define SPLICE_IRQ_VALUE_LENGTH 10  /* each interrupt count is 10 digits long in /proc/interrupts */
#define PROC_INTERRUPTS_FILE         "/proc/interrupts"
#define TEMP_INTERRUPTS_FILE         "interrupts"
#define PROC_STAT_FILE               "/proc/stat"
#define MAX_LENGTH_GETPIDOF_CMD      128

#define Free(buffer)    if(buffer){free(buffer); buffer=0;}
#define false 0
#define true 1

//#define DEBUG
#define PrintHTML printf

typedef unsigned char bool;

typedef struct
{
	unsigned long int irqCount[SPLICE_MAX_NUM_CPUS];
	unsigned long int irqCountPrev[SPLICE_MAX_NUM_CPUS];
	char              irqName[SPLICE_IRQ_NAME_LENGTH];
} splice_irq_details;

typedef struct
{
	unsigned char     numActiveCpus;
	float             uptime;
	unsigned char     idlePercentage[SPLICE_MAX_NUM_CPUS];
	unsigned long int user[SPLICE_MAX_NUM_CPUS];
	unsigned long int nice[SPLICE_MAX_NUM_CPUS];
	unsigned long int system[SPLICE_MAX_NUM_CPUS];
	unsigned long int idle[SPLICE_MAX_NUM_CPUS];
	unsigned long int cpu_iowait[SPLICE_MAX_NUM_CPUS];
	unsigned long int cpu_irq[SPLICE_MAX_NUM_CPUS];
	unsigned long int cpu_softirq[SPLICE_MAX_NUM_CPUS];
	unsigned long int cpu_steal[SPLICE_MAX_NUM_CPUS];
} splice_cpu_percent;

typedef struct
{
	unsigned long int irqTotal;
	unsigned long int contextSwitches;
} splice_proc_stat_info;

typedef struct
{
	float                uptime;
	unsigned long int    irqCount[SPLICE_MAX_NUM_CPUS];
	unsigned long int    irqTotal;
	splice_irq_details irqDetails[SPLICE_IRQ_MAX_TYPES];
} splice_irq_data;

char *getPlatformVersion(
		void
		);

char *getPlatform(
		void
		);

int Close(
		int socketFd
		);

int send_request_read_response(
		unsigned char      *request,
		int                 request_len,
		unsigned char      *response,
		int                 response_len,
		int                 server_port,
		int                 cmd
		);

int gethostbyaddr2(
		const char *HostName,
		int         port,
		char       *HostAddr,
		int         HostAddrLen
		);

int scanForInt(
		const char *queryRequest,
		const char *formatStr,
		int        *returnValue
		);

int scanForStr(
		const char  *queryRequest,
		const char  *formatStr,
		unsigned int returnValuelen,
		char        *returnValue
		);

int convert_to_string_with_commas(
		unsigned long int value,
		char             *valueStr,
		unsigned int      valueStrLen
		);

char *DateYyyyMmDdHhMmSs( void );
char *HhMmSs( unsigned long int timestamp );
char *DayMonDateYear( unsigned long int timestamp );
char *GetFileContents( const char *filename );
char *GetTempDirectoryStr( void );
int get_proc_stat_info( splice_proc_stat_info *pProcStatInfo );
int get_interrupt_counts( splice_irq_data *irqData );
void PrependTempDirectory( char *filenamePath, int filenamePathLen, const char *filename );
int set_cpu_utilization( void );
int splice_getCpuUtilization( splice_cpu_percent *pCpuData );
int P_getCpuUtilization( void );

unsigned int get_my_ip4_addr( void );
int get_cpu_frequency( unsigned int cpuId);
int output_cpu_frequencies( void );
int getFileSize( const char *filename);
int getUpTime(float *uptime);
int setUpTime(void);
void reusePort(int s);
int TASK_Sleep(unsigned long ultime);
void LONG_to_PLC_BIN_ARRAY(long n, unsigned char *str);
void PLC_BIN_to_LONG( char *SData, long  *DData);
void PLC_BIN_to_INT( char *SData, int  *DData);
#endif
