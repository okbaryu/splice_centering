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
#include "splice_utils.h"
#include "actuator.h"
#include "centering.h"
#include "plc.h"
#include "cmdtool.h"
#include "osal_init.h"
#include "cmd_parser_init.h"
#include "sys_trace.h"

#define MAXHOSTNAME                  80
#define PERF_FILE_FULL_PATH_LEN      64
#define SATA_USB_FILE_FULL_PATH_LEN  64
#define LINUX_TOP_FILE_FULL_PATH_LEN 64

static unsigned char Quit = 0;

static bool                    initializationDone = false;
static splice_irq_data       g_savedIrqData;
static unsigned long int       g_ContextSwitchesPrev  = 0;
static unsigned long int       g_ContextSwitchesDelta = 0;

typedef struct
{
	char deviceName[32];
} splice_device_name;


typedef struct
{
	long int          uptime;
	unsigned int      major;
	unsigned int      minor;
	unsigned long int readsCompletedSuccessfully;
	unsigned long int readsMerged;
	unsigned long int readsSectors;
	unsigned long int readsMilliseconds;
	unsigned long int writesCompletedSuccessfully;
	unsigned long int writesMerged;
	unsigned long int writesSectors;
	unsigned long int writesMilliseconds;
} splice_device_data;

typedef struct
{
	float readMbps;
	float writeMbps;
} splice_device_mbps;

#define SPLICE_SATA_USB_HISTORY_MAX 10
static pthread_t         gSataUsbThreadId           = 0;
static bool              gSataUsbFirstPassAfterInit = false;
static unsigned long int gSataUsbTimeout            = 0;   /* gather function increments it; request function clears it out; if we reach 5, stop gathering */
splice_device_name     gSataUsbDeviceNames[SPLICE_SATA_USB_MAX];
splice_device_data     gSataUsbDataPrev[SPLICE_SATA_USB_MAX];
splice_device_data     gSataUsbDataNow[SPLICE_SATA_USB_MAX];
splice_device_mbps     gSataUsbMbps[SPLICE_SATA_USB_MAX];
splice_device_mbps     gSataUsbMbpsHistory[SPLICE_SATA_USB_MAX][SPLICE_SATA_USB_HISTORY_MAX];
unsigned int             gSataUsbMbpsHistoryIdx = 0;
pthread_mutex_t          gSataUsbMutex;

/**
 *  Function: This function will initialize the specified mutex variable.
 **/
int Splice_Server_InitMutex(
		pthread_mutex_t *mutex
		)
{
	if (pthread_mutex_init( mutex, NULL ))
	{
		printf( "%s: pthread_mutex_init failure; \n", __FUNCTION__ );
		return( 1 );
	}
	else
	{
		return( 0 );
	}
}

/**
 *  Function: This function will lock the specified mutex variable.
 **/
int Splice_Server_LockMutex(
		pthread_mutex_t *mutex
		)
{
	int rc = 0;

	rc = pthread_mutex_lock( mutex );
	return( rc );
}

/**
 *  Function: This function will unlock the specified mutex variable.
 **/
void Splice_Server_UnlockMutex(
		pthread_mutex_t *mutex
		)
{
	if (pthread_mutex_unlock( mutex ))
	{
		PrintError("pthread_mutex_unlock failed\n" );
	}
	return;
}

/**
 *  Function: This function will uninit the specified mutex variable.
 **/
void Splice_Server_UninitMutex(
		pthread_mutex_t *mutex
		)
{
	pthread_mutex_destroy( mutex );
	return;
}

/**
 *  Function: This function is the prototype for Splice_ReadRequest() API.
 **/
static int Splice_ReadRequest(
		int                psd,
		struct sockaddr_in from,
		splice_request  *pRequest,
		splice_response *pResponse
		);

/**
 *  Function: This function will close the specified socket and exit the app.
 **/
static int CloseAndExit(
		int         socketFd,
		const char *reason
		)
{
	printf( "FAILURE: socket %d; reason (%s)\n", socketFd, reason );
	if (socketFd>0) {close( socketFd ); }
	exit( 0 );
}

/**
 *  Function: This function will open a connection to a specific port that will be used to receive and send data
 *  from the user's browser.
 **/
void *startServer(
		void *data
		)
{
	int                fromlen;
	int                struct_length;
	int                pn;
	int                sd           = 0, psd = 0;
	unsigned long int  requestCount = 0;
	struct sockaddr_in server;
	struct sockaddr_in from;
	splice_request   request;
	splice_response  response;

	/* Construct name of socket */
	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = htonl( INADDR_ANY );
	pn                     = htons( SPLICE_SERVER_PORT );
	server.sin_port        = pn;

	/* Create socket on which to send and receive */

	sd = socket( AF_INET, SOCK_STREAM, 0);

	if (sd < 0)
	{
		perror( "opening stream socket" );
		exit( -1 );
	}

	/* this allows the server to re-start quickly instead of fully wait for TIME_WAIT which can be as large as 2 minutes */
	reusePort( sd );

	if (bind( sd, (struct sockaddr *)&server, sizeof( server )) < 0)
	{
		CloseAndExit( sd, "binding name to stream socket" );
	}

	/* get port information and  prints it out */
	struct_length = sizeof( server );

	if (getsockname( sd, (struct sockaddr *)&server, (unsigned int *)&struct_length ))
	{
		CloseAndExit( sd,  "getting socket name" );
	}
	PrintInfo( "%s - Server Port is: %d\n", __FUNCTION__, ntohs( server.sin_port ));

	/* accept TCP connections from clients and fork a process to serve each */
	listen( sd, 4 );
	fromlen = sizeof( from );

	while (Quit == 0) {
		PrintInfo( "%s - Waiting to accept socket connections; Quit %u; requestCount %lu\n", __FUNCTION__, Quit, requestCount );
		psd = accept( sd, (struct sockaddr *)&from, (unsigned int *)&fromlen );
		requestCount++;
		memset( &request, 0, sizeof( request ));
		memset( &response, 0, sizeof( response ));
		Splice_ReadRequest( psd, from, &request, &response );
	}

	CloseAndExit( sd, "quit detected" );

	return( 0 );
}


/**
 *  Function: This function will call get_interrupt_counts() API to copy the interrupt counts to a local
 *  structure. This local structure will be used to determine the delta counts from the last pass through.
 **/
static int splice_computeIrqData(
		unsigned int       numActiveCpus,
		splice_irq_data *pIrqData
		)
{
	unsigned int             cpu = 0;
	unsigned int             irq = 0;
	splice_irq_data        irqDataNow;
	float                    uptimeDelta = 0;
	unsigned long int        irqTotal    = 0;
	static float             irqAvg      = 0;
	static unsigned long int irqAvgCount = 0;
	static unsigned long int irqAvgFirst = 0;
	splice_proc_stat_info  lProcStatInfo;

	memset( &irqDataNow, 0, sizeof( irqDataNow ));
	memset( &lProcStatInfo, 0, sizeof( lProcStatInfo ));

	getUpTime( &irqDataNow.uptime );
	PrintInfo( "%5.3f - %5.3f = %5.3f\n", irqDataNow.uptime, g_savedIrqData.uptime, irqDataNow.uptime - g_savedIrqData.uptime );

	/* calculate the cummulative number of interrupts that happened since we booted up */
	get_interrupt_counts( &irqDataNow );

	PrintInfo( "%s: numCpus %u: ", __FUNCTION__, numActiveCpus );
	/* subtract off the number of interrupts that we saw the last pass through */
	for (cpu = 0; cpu < numActiveCpus; cpu++)
	{
		PrintInfo( "%u:", cpu );
		/* if no data has been saved previously, return 0; otherwise, the returned value could be a very very large number */
		if (g_savedIrqData.irqCount[cpu] == 0)
		{
			pIrqData->irqCount[cpu] = 0;
		}
		else
		{
			if (g_savedIrqData.uptime > 0)
			{
				uptimeDelta = 1.0 * irqDataNow.irqCount[cpu];
				/*printf("%5.1f ", uptimeDelta );*/
				uptimeDelta -= 1.0 * g_savedIrqData.irqCount[cpu];
				/*printf("%5.1f ", uptimeDelta );*/
				/*uptimeDelta *= ( irqDataNow.uptime - g_savedIrqData.uptime ); */ /* adjust the value to account for 99 jiffies this second versus 100 this second */
				PrintInfo( "(%5.1f)", uptimeDelta );
				pIrqData->irqCount[cpu] = uptimeDelta;
				PrintInfo( "->%lu (%lu)\t", pIrqData->irqCount[cpu], irqDataNow.irqCount[cpu] - g_savedIrqData.irqCount[cpu] );
			}
			else
			{
				pIrqData->irqCount[cpu] = irqDataNow.irqCount[cpu] - g_savedIrqData.irqCount[cpu];
			}
		}
		PrintInfo( "%-5lu ", pIrqData->irqCount[cpu] );
	}

	/* read interrupt data and context switch data from /proc/stat file */
	get_proc_stat_info( &lProcStatInfo );

	PrintInfo( "%s: delta (%lu) = now (%lu) - prev (%lu)\n", __FUNCTION__,  g_ContextSwitchesDelta, lProcStatInfo.contextSwitches, g_ContextSwitchesPrev );
	g_ContextSwitchesDelta = lProcStatInfo.contextSwitches - g_ContextSwitchesPrev;
	g_ContextSwitchesPrev  = lProcStatInfo.contextSwitches;

	/* the irqTotal is computed from the file /proc/stat and not from adding entries from /proc/interrupts */
	irqDataNow.irqTotal = irqTotal = lProcStatInfo.irqTotal;

	/* adjust value to account for 99 jiffies per second versus 100 or 101 or 102 etc. */
	uptimeDelta = ( irqDataNow.uptime - g_savedIrqData.uptime );

	pIrqData->irqTotal = irqDataNow.irqTotal - g_savedIrqData.irqTotal;
	if (irqAvgFirst == 0)
	{
		irqAvgFirst = irqDataNow.irqTotal;
	}
	/* don't adjust the delta count if the uptime delta is way off */
	if (( uptimeDelta > 0 ) && ( uptimeDelta < 1.5 ))
	{
		float temp = 1.0 /*uptimeDelta*/;
		PrintInfo( "irqTotal was %-5lu (%-5lu - %-5lu); ", pIrqData->irqTotal, irqDataNow.irqTotal, g_savedIrqData.irqTotal );
		temp              *= pIrqData->irqTotal;
		pIrqData->irqTotal = temp;
		PrintInfo( "mul %5.2f is %-5lu;    ", 1.0 /*uptimeDelta*/, pIrqData->irqTotal );
		{
			float             delta    = pIrqData->irqTotal;
			float             oldAvg   = irqAvg;
			unsigned long int oldCount = irqAvgCount;

			irqAvg = ( irqAvg*irqAvgCount ) + delta;
			irqAvgCount++;
			irqAvg /= irqAvgCount;

			PrintInfo( " 2222 avg (%5.1f * %lu) + %5.1f = %5.1f; first (%lu - %lu)/%lu = %lu\n", oldAvg, oldCount, delta, irqAvg, irqDataNow.irqTotal, irqAvgFirst, irqAvgCount,
					( irqDataNow.irqTotal - irqAvgFirst )/irqAvgCount );
		}
	}
	PrintInfo( "irqTotal %-3lu (%lu - %lu); adj %5.1f\n", pIrqData->irqTotal, irqDataNow.irqTotal, g_savedIrqData.irqTotal, uptimeDelta );

	/* copy the IRQ details to the response buffer */
	PrintInfo( "memcpy to irqDetails ... %lu bytes 1; %lu bytes 2\n", sizeof( pIrqData->irqDetails ), sizeof( pIrqData->irqDetails ));
	memcpy( &pIrqData->irqDetails, &irqDataNow.irqDetails, sizeof( pIrqData->irqDetails ));

	/* copy the previous counts into the response buffer */
	for (cpu = 0; cpu < numActiveCpus; cpu++)
	{
		for (irq = 0; irq < SPLICE_IRQ_MAX_TYPES; irq++)
		{
			pIrqData->irqDetails[irq].irqCountPrev[cpu] = g_savedIrqData.irqDetails[irq].irqCount[cpu];
		}
	}
	/* save the current values to be used to compute the delta the next pass through */
	g_savedIrqData = irqDataNow;

	return( 0 );
}

static int splice_sata_usb_array_update(
		splice_device_data *lSataUsbDataNow,
		const char           *lDeviceName
		)
{
	unsigned int idx;

	for (idx = 0; idx<SPLICE_SATA_USB_MAX; idx++)
	{
		/* if we find a slot that does not have a device name in it, the one we are searching for does not exist in the array */
		if (strlen( gSataUsbDeviceNames[idx].deviceName ) == 0)
		{
			break;
		}
		else if (strcmp( gSataUsbDeviceNames[idx].deviceName, lDeviceName ) == 0)
		{
			/*printf( "adding sata_usb data to idx %u for (%s)\n", idx, lSataUsbDataNow->deviceName );*/
			gSataUsbDataNow[idx] = *lSataUsbDataNow;
			return( 0 );
		}
	}

	PrintInfo( "Could not find deviceName (%s) in gSataUsbDeviceNames.\n", lDeviceName );
	return( 0 );
}


/**
 *  Function: This function will use the 'df' utility to scan for currently mounted devices. Only these devices
 *  will be searched for in the /proc/diskstats file during the one-second gathering.
 **/
static void splice_sata_usb_scan(
		void
		)
{
	FILE *cmd = NULL;
	char  line[MAX_LENGTH_CPU_STATUS];
	int   gSataUsbDeviceNamesIdx = 0;

	strncpy( line, "df | grep \"^/dev/\" ", sizeof ( line ) - 1 );
	cmd = popen( line, "r" );

	do
	{
		memset( line, 0, sizeof( line ));

		fgets( line, MAX_LENGTH_CPU_STATUS, cmd );         /* /dev/sda4            958852568    917028 909919940   0% /data */

		if (strlen( line ))
		{
			char *pos = strchr( &line[1], '/' );           /* find the beginning of the unique device name (skipping over the first slash) */
			if (pos)
			{
				char *posspace = NULL;
				pos++;
				posspace = strchr( pos, ' ' );             /* find the first space after the device name */
				if (posspace)
				{
					*posspace = '\0';                      /* null-terminate the device name */
					/* if there is space to add this device name to the list */
					if (gSataUsbDeviceNamesIdx < SPLICE_SATA_USB_MAX)
					{
						strncpy( gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName, pos, sizeof( gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName ) - 1 );
						PrintInfo( "%s: added gSataUsbDeviceNames[%d]:(%s)\n", __FUNCTION__, gSataUsbDeviceNamesIdx, gSataUsbDeviceNames[gSataUsbDeviceNamesIdx].deviceName );
						gSataUsbDeviceNamesIdx++;
					}
				}
			}
		}
	} while (strlen( line ));
	PrintInfo( "\n" );

	pclose( cmd );
	return;
}

/**
 *  Function: This function will read the /proc/diskstats file to gather statistics for SATA and USB drives.
 **/
static void *splice_sata_usb_thread(
		void *data
		)
{
	if (data == NULL)
	{
		PrintError( "%s: arg1 cannot be NULL\n", __FUNCTION__ );
		exit( EXIT_FAILURE );
	}

	Splice_Server_LockMutex( &gSataUsbMutex );

	PrintInfo( "%s: clearing gSataUsb arrays. \n", __FUNCTION__ );
	memset( &gSataUsbDataPrev, 0, sizeof( gSataUsbDataPrev ));
	memset( &gSataUsbDeviceNames, 0, sizeof( gSataUsbDeviceNames ));
	memset( &gSataUsbMbpsHistory, 0, sizeof( gSataUsbMbpsHistory ));
	gSataUsbMbpsHistoryIdx     = 0;
	gSataUsbFirstPassAfterInit = true;

	/* scan for currently-mounted device names */
	splice_sata_usb_scan();                              /* function updates gSataUsbDeviceNames[] array */

	Splice_Server_UnlockMutex( &gSataUsbMutex );

	/* collect data until no one has requested USB/SATA for 5 seconds */
	while (gSataUsbTimeout <  5)
	{
		unsigned int         idx      = 0;
		char                *pos      = NULL;
		char                *posEol   = NULL;
		char                *posBol   = NULL;
		char                *contents = NULL;
		char                 cmd[128];
		unsigned char        differenceFound = 0;
		splice_device_data lSataUsbDataNow;
		char                 sataUsbFilename[SATA_USB_FILE_FULL_PATH_LEN];

		gSataUsbTimeout++;                                 /* request function should zero this out every time a request comes in; if we reach time, stop gathering */
		if (gSataUsbTimeout > 1)
		{
			PrintError( "%s: gSataUsbTimeout (%ld)\n", __FUNCTION__, gSataUsbTimeout );
		}

		PrependTempDirectory( sataUsbFilename, sizeof( sataUsbFilename ), SATA_USB_OUTPUT_FILE );
		PrintInfo( "%s: filename (%s)\n", __FUNCTION__, sataUsbFilename );

		differenceFound = 0;

		memset( &gSataUsbDataNow, 0, sizeof( gSataUsbDataNow ));
		memset( &lSataUsbDataNow, 0, sizeof( lSataUsbDataNow ));

		sprintf( cmd, "cp -f /proc/diskstats %s", sataUsbFilename );
		/*PrintInfo( "issuing cmd (%s)\n", cmd );*/
		system( cmd );

		contents = GetFileContents( sataUsbFilename );

		if (contents == NULL)
		{
			PrintInfo( "file (%s) had no contents\n", sataUsbFilename );
			continue;
		}

		/* loop through known devices and find the matching line in the diskstats contents */
		for (idx = 0; idx<SPLICE_SATA_USB_MAX; idx++)
		{
			if (strlen( gSataUsbDeviceNames[idx].deviceName ) == 0)
			{
				/* we have reached the end of known devices */
				break;
			}

			pos = strstr( contents, gSataUsbDeviceNames[idx].deviceName );
			/*PrintInfo("%s: found device (%s) at offset %d\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, pos - contents );*/

			/* if the device name was found in diskstats file */
			if (pos)
			{
				char lDeviceName[32];

				posEol = strchr( pos, '\n' );
				/*PrintInfo("%s: for device (%s) posEol %p\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, posEol );*/
				if (posEol)
				{
					posEol[0] = '\0';                      /* null-terminate the line */
				}

				posBol = pos - 12;

				/*PrintInfo("%s: for device (%s) line (%s)\n", __FUNCTION__, gSataUsbDeviceNames[idx].deviceName, posBol );*/
				/*
				   8       0 sda 70715 13758 8161780 6120 4 1 5 0 0 2950 6120
				   8       1 sda1 70714 13758 8161772 6090 4 1 5 0 0 2920 6090
				   31      10 mtdblock10 0 0 0 0 0 0 0 0 0 0 0
				   179       0 mmcblk0 170 9048 9257 610 0 0 0 0 0 330 610
				   179       1 mmcblk0p1 169 9048 9249 600 0 0 0 0 0 320 600
				   */

				sscanf( posBol, "%u %u %s %lu %lu %lu %lu %lu %lu %lu %lu ", &lSataUsbDataNow.major, &lSataUsbDataNow.minor, &lDeviceName[0],
						&lSataUsbDataNow.readsCompletedSuccessfully, &lSataUsbDataNow.readsMerged, &lSataUsbDataNow.readsSectors, &lSataUsbDataNow.readsMilliseconds,
						&lSataUsbDataNow.writesCompletedSuccessfully, &lSataUsbDataNow.writesMerged, &lSataUsbDataNow.writesSectors, &lSataUsbDataNow.writesMilliseconds );
				splice_sata_usb_array_update( &lSataUsbDataNow, lDeviceName );

				/* debug */
				if (( lSataUsbDataNow.readsSectors != gSataUsbDataPrev[idx].readsSectors ) || ( lSataUsbDataNow.writesSectors != gSataUsbDataPrev[idx].writesSectors ))
				{
					PrintInfo( "%s: /dev/%s: rd %-8lu, wr %-8lu\n", __FUNCTION__, lDeviceName, lSataUsbDataNow.readsSectors, lSataUsbDataNow.writesSectors );
				}
				if (posEol)
				{
					posEol[0] = '\n';                      /* restore the newline character */
				}
			}
		}                                                  /* end for each device name */

		Free( contents );

		Splice_Server_LockMutex( &gSataUsbMutex );
		/* for each I/O device */
		for (idx = 0; idx<sizeof( gSataUsbDataNow )/sizeof( gSataUsbDataNow[0] ); idx++)
		{
			unsigned long int deltaSectors = 0;
			if (( gSataUsbDataNow[idx].readsSectors > gSataUsbDataPrev[idx].readsSectors ) || ( gSataUsbDataNow[idx].writesSectors > gSataUsbDataPrev[idx].writesSectors ))
			{
				differenceFound            = 1;
				deltaSectors               = gSataUsbDataNow[idx].readsSectors - gSataUsbDataPrev[idx].readsSectors;
				gSataUsbMbps[idx].readMbps = deltaSectors * 512.0 * 8.0 / 1024.0 / 1024.0;
				/*if (strlen( gSataUsbDataPrev[idx].deviceName ))*/
				{
					PrintInfo( "device[%u] (%-4s) %6.2f Mbps ", idx, gSataUsbDeviceNames[idx].deviceName, gSataUsbMbps[idx].readMbps );
				}

				deltaSectors                = gSataUsbDataNow[idx].writesSectors - gSataUsbDataPrev[idx].writesSectors;
				gSataUsbMbps[idx].writeMbps = deltaSectors * 512.0 * 8.0 / 1024.0 / 1024.0;
				/*if (strlen( gSataUsbDataPrev[idx].deviceName ))*/
				{
					PrintInfo( "%6.2f Mbps\n", gSataUsbMbps[idx].writeMbps );
				}
			}
			else
			{
				gSataUsbMbps[idx].readMbps  = 0;
				gSataUsbMbps[idx].writeMbps = 0;
				PrintInfo( "device[%u] zero; ", idx );
			}

			/* save for 10-second history */
			if (gSataUsbFirstPassAfterInit == false)
			{
				gSataUsbMbpsHistory[idx][gSataUsbMbpsHistoryIdx].readMbps  = gSataUsbMbps[idx].readMbps;
				gSataUsbMbpsHistory[idx][gSataUsbMbpsHistoryIdx].writeMbps = gSataUsbMbps[idx].writeMbps;
			}
		}

		/* the first pass through, the previous numbers are zero but the current readSectors/writeSectors are very large; skip the first pass */
		gSataUsbFirstPassAfterInit = false;

		gSataUsbMbpsHistoryIdx++;
		if (gSataUsbMbpsHistoryIdx >= SPLICE_SATA_USB_HISTORY_MAX) /* wrap around to zero if it's time to */
		{
			gSataUsbMbpsHistoryIdx = 0;
		}

		Splice_Server_UnlockMutex( &gSataUsbMutex );
		PrintInfo( "\n" );

		if (differenceFound)
		{
			Splice_Server_LockMutex( &gSataUsbMutex );
			memcpy( &gSataUsbDataPrev, &gSataUsbDataNow, sizeof( gSataUsbDataNow ));
			Splice_Server_UnlockMutex( &gSataUsbMutex );
		}

		/*remove( sataUsbFilename );*/

		sleep( 1 );
	}

	Splice_Server_LockMutex( &gSataUsbMutex );
	PrintInfo( "%s: clearing gSataUsb arrays. \n", __FUNCTION__ );
	memset( &gSataUsbDataPrev, 0, sizeof( gSataUsbDataPrev ));
	memset( &gSataUsbDeviceNames, 0, sizeof( gSataUsbDeviceNames ));
	memset( &gSataUsbMbpsHistory, 0, sizeof( gSataUsbMbpsHistory ));
	gSataUsbMbpsHistoryIdx     = 0;
	gSataUsbFirstPassAfterInit = true;
	Splice_Server_UnlockMutex( &gSataUsbMutex );

	gSataUsbThreadId = 0;

	PrintInfo( "%s: complete.\n", __FUNCTION__ );

	pthread_exit( 0 );
}


/**
 *  Function: This function will compute the 10-second average in Mbps for data reads and writes to the
 *  various SATA and USB devices.
 **/
float splice_sata_usb_compute_average(
		unsigned int deviceIdx,
		unsigned int readOrWrite                               /* 0: readMbps; 1: writeMbps */
		)
{
	unsigned int idx     = 0;
	float        average = 0.0;

	/* if the deviceName is invalid, stop computing */
	if (gSataUsbDeviceNames[deviceIdx].deviceName[0] == '\0')
	{
		return( average );
	}

#ifdef DEBUG
	if (deviceIdx == 0)
	{
		PrintInfo( "%s: device[%u].%-5s: idx %d: %-5s ", __FUNCTION__, deviceIdx,
				gSataUsbDeviceNames[deviceIdx].deviceName, gSataUsbMbpsHistoryIdx,
				( readOrWrite==0 ) ? "read" : "write" );
	}
#endif /* ifdef DEBUG */
	for (idx = 0; idx<SPLICE_SATA_USB_HISTORY_MAX; idx++)
	{
		if (readOrWrite == 0 /* user requested readMbps */)
		{
			average += gSataUsbMbpsHistory[deviceIdx][idx].readMbps;
#ifdef DEBUG
			if (deviceIdx == 0) {printf( "%-5.2f ", gSataUsbMbpsHistory[deviceIdx][idx].readMbps ); }
#endif
		}
		else
		{
			average += gSataUsbMbpsHistory[deviceIdx][idx].writeMbps;
#ifdef DEBUG
			if (deviceIdx == 0) {printf( "%-5.2f ", gSataUsbMbpsHistory[deviceIdx][idx].writeMbps ); }
#endif
		}
	}
	average /= SPLICE_SATA_USB_HISTORY_MAX;
#ifdef DEBUG
	if (deviceIdx == 0) {printf( "... avg %5.2f Mbps \n", average ); }
#endif
	return( average );
}

/**
 *  Function: This function will gather the SATA and USB data that is collected by a separate thread and save
 *  the data into the response buffer being prepared to send back to the client.
 **/
static int splice_sata_usb_gather(
		splice_response *pResponse
		)
{
	unsigned int idx = 0;

	if (pResponse == NULL)
	{
		return( -1 );
	}

	gSataUsbTimeout = 0;

	for (idx = 0; idx<sizeof( gSataUsbDeviceNames )/sizeof( gSataUsbDeviceNames[0] ); idx++)
	{
		/* the first time we reach an empty device name, stop looking for data */
		if (gSataUsbDeviceNames[idx].deviceName[0] == '\0')
		{
			break;
		}
		strncpy( pResponse->response.cpuIrqData.sataUsbData[idx].deviceName, gSataUsbDeviceNames[idx].deviceName,
				sizeof( pResponse->response.cpuIrqData.sataUsbData[0].deviceName ) - 1 );
#ifdef INSTANT_RESPONSE
		pResponse->response.cpuIrqData.sataUsbData[idx].readMbps  = gSataUsbMbps[idx].readMbps;
		pResponse->response.cpuIrqData.sataUsbData[idx].writeMbps = gSataUsbMbps[idx].writeMbps;
#else /* use 10-second average */
		pResponse->response.cpuIrqData.sataUsbData[idx].readMbps  = splice_sata_usb_compute_average( idx, 0 /* compute readMbps */ );
		pResponse->response.cpuIrqData.sataUsbData[idx].writeMbps = splice_sata_usb_compute_average( idx, 1 /* compute writeMbps */ );
#endif /* if defined INSTANT_RESPONE */
	}

	return( 0 );
}


/**
 *  Function: This function will start a new thread that will gather the SATA and USB bandwidth statistics.
 **/
static int splice_sata_usb_start(
		unsigned long int option
		)
{
	void                    *(*threadFunc)( void * );
	static unsigned long int threadOption = 0;

	/* if the thread has not already been started, start it now */
	if (( gSataUsbThreadId == 0 ) && ( option == SPLICE_CMD_START_SATA_USB ))
	{
		threadFunc   = splice_sata_usb_thread;
		threadOption = 0;

		if (pthread_create( &gSataUsbThreadId, NULL, threadFunc, (void *)&threadOption ))
		{
			PrintError( "could not create thread for sataUsb; %s\n", strerror( errno ));
		}
		else
		{
			PrintError( "%s: Thread for SATA/USB started successfully; id %lx\n", __FUNCTION__, (long int) gSataUsbThreadId );
		}
	}
	else
	{
		PrintInfo( "Thread for SATA/USB already started; id %lx\n", (long int) gSataUsbThreadId );
		if (option == SPLICE_CMD_STOP_SATA_USB)
		{
			/* gSataUsbThreadId = 0; thread will timeout by itself after 5 seconds of inactivity */
		}
	}

	return( 0 );
}


/**
 *  Function: This function will read the user's request coming in from the browser
 **/
static int Splice_ReadRequest(
		int                psd,
		struct sockaddr_in from,
		splice_request  *pRequest,
		splice_response *pResponse
		)
{
	int         rc;
	static bool bFirstPass = true;

	PrintInfo( "%u Started new thread serving %s\n", ntohs( from.sin_port ), inet_ntoa( from.sin_addr ));

	if (initializationDone == false)
	{
		PrintInfo( "memset(g_client_list to 0xff\n");
		initializationDone = true;
	}
	/* get data from  clients and send it back */

	PrintInfo( "%u Server is reading request (%lu bytes) from client ... \n", ntohs( from.sin_port ), sizeof( *pRequest ));

	if (( rc = recv( psd, pRequest, sizeof( *pRequest ), 0 )) < 0)
	{
		PrintInfo( "%u ", ntohs( from.sin_port ));
		CloseAndExit( psd, "receiving stream  message" );
	}

	if (rc > 0)
	{
		PrintInfo( "%u %s: Received: from TCP/Client (%s); cmd (%u) \n", ntohs( from.sin_port ), DateYyyyMmDdHhMmSs(), inet_ntoa( from.sin_addr ), pRequest->cmd );
		PrintInfo( "cmd %u; cmdSecondary %u; cmdOption %lu\n", pRequest->cmd, pRequest->cmdSecondary, pRequest->cmdSecondaryOption );
		switch (pRequest->cmd) {
			case SPLICE_CMD_GET_ACTUATOR_STATUS:
				{
					act_status p;

					if(getIsCentering())
					{
						PrintInfo( "In middle of centering\n");
						break;
					}

					PrintInfo( " SPLICE_CMD_GET_ACTUATOR_STATUS\n");

					actuator_get_status(&p);

					pResponse->data[0] = p.act_direction;
					pResponse->data[1] = p.act_max_stroke;
					pResponse->data[2] = p.act_speed;
					pResponse->data[3] = p.act_l_limit;
					pResponse->data[4] = p.act_r_limit;
					pResponse->data[5] = p.act_origin_offset_msb;
					pResponse->data[6] = p.act_origin_offset_lsb;

					if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
					{
						CloseAndExit( psd, "sending response cmd" );
					}

					break;
				}

			case SPLICE_CMD_SET_ACTUATOR_STATUS:
				{
					act_status p;

					if(getIsCentering())
					{
						PrintInfo( "In middle of centering\n");
						break;
					}

					PrintInfo( "SPLICE_CMD_SET_ACTUATOR_STATUS\n");

					p.act_direction = pRequest->request.strCmdLine[0];
					p.act_max_stroke = pRequest->request.strCmdLine[1];
					p.act_speed = pRequest->request.strCmdLine[2];
					p.act_l_limit = pRequest->request.strCmdLine[3];
					p.act_r_limit = pRequest->request.strCmdLine[4];
					p.act_origin_offset_msb = pRequest->request.strCmdLine[5];
					p.act_origin_offset_lsb = pRequest->request.strCmdLine[6];

					actuator_set_status(&p);

					if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
					{
						CloseAndExit( psd, "sending response cmd" );
					}

					break;
				}

			case SPLICE_CMD_GET_ACTUATOR_POSITION:
				{
					act_position p;

					if(getIsCentering())
					{
						PrintInfo( "In middle of centering\n");
						break;
					}

					PrintInfo( " SPLICE_CMD_GET_ACTUATOR_POSITION\n");

					actuator_get_current_postion(&p);

					pResponse->data[0] = p.act_cur_position_msb;
					pResponse->data[1] = p.act_cur_position_lsb;
					pResponse->data[2] = p.act_prev_position_msb;
					pResponse->data[3] = p.act_prev_position_lsb;

					if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
					{
						CloseAndExit( psd, "sending response cmd" );
					}

					break;
				}

			case SPLICE_CMD_SET_ACTUATOR_POSITION:
				{
					act_position p;
					p.act_cur_position_msb = 0;
					p.act_cur_position_lsb = 0;
					int val;

					if(getIsCentering())
					{
						PrintInfo( "In middle of centering\n");
						break;
					}

					PrintInfo( " SPLICE_CMD_SET_ACTUATOR_POSITION\n");

					pResponse->cmd = pRequest->cmdSecondary;
					if(pRequest->cmdSecondary == CMD2_ACT_MOVE)
					{
						if(pRequest->cmdSecondaryOption < 0)
						{
							p.act_cur_position_msb |= 0x80;
							pRequest->cmdSecondaryOption *= -1;
						}

						val = pRequest->cmdSecondaryOption;
						p.act_cur_position_lsb |= val & 0xff;
						p.act_cur_position_msb |= (val >> 8) & 0xff;
						actuator_set_current_position(&p, CMD2_ACT_MOVE);

						if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
						{
							CloseAndExit( psd, "sending response cmd" );
						}
					}
					else if(pRequest->cmdSecondary == CMD2_ACT_MOVE_ORG)
					{
						actuator_set_current_position(&p, CMD2_ACT_MOVE_ORG);

						if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
						{
							CloseAndExit( psd, "sending response cmd" );
						}
					}

					break;
				}

			case SPLICE_CMD_GET_RREGISTER_PLC:
				{
					pResponse->cmd = pRequest->cmd;
					if(sendPlc(EEP_Data_Start_R_Register, pResponse->data, EEP_R_Register_Size, true) != PLC_COMM_ACK)
					{
						printf("failed to get R register from PLC\n");
					}

					if (send( psd, pResponse, sizeof( *pResponse ), 0 ) < 0)
					{
						CloseAndExit( psd, "sending response cmd" );
					}
					break;
				}

			case SPLICE_CMD_QUIT:
				{
					Quit = 1;
					break;
				}
			default:
				{
					printf( "Server: unknown request %d\n", pRequest->cmd );
					break;
				}
		}                                                  /* switch */
	}
	else
	{
		PrintInfo( "%u TCP/Client: %s\n", ntohs( from.sin_port ), inet_ntoa( from.sin_addr ));
		PrintInfo( "%u Disconnected..\n", ntohs( from.sin_port ));
	}

	close( psd );

	return( 0 );
}

void handler(int signum)
{
	printf("broken pipe signal~!~!  0x%x\n", signum);
}

/**
 *  Function: This function is the main function that controls the logic of the app.
 **/
int main(
		int   argc,
		char *argv[]
		)
{

	UNUSED( argc );

	pthread_t serverTaskId;

	memset( &g_savedIrqData, 0, sizeof( g_savedIrqData ));
	memset( &gSataUsbMbps, 0, sizeof( gSataUsbMbps ));

	Splice_Server_InitMutex( &gSataUsbMutex );

	OSAL_Init();
	CMD_Init();
	CMD_HS_Init();

	SYS_TRACE_EnableLevel(TRACE_ERR | TRACE_WARN | TRACE_DEBUG | TRACE_INFO);

	signal(SIGPIPE, handler);
	plc_init();
	actuator_init();
	TASK_Sleep(1000);
	centering_init();

	if (pthread_create( &serverTaskId, NULL, startServer, (void *)NULL ))
	{
		PrintError( "could not create thread for splice_server %s\n", strerror( errno ));
	}

	while(1)
	{
		TASK_Sleep(1000);
	}

	PrintInfo( "%s exiting.\n", argv[0] );

	return( 0 );
}

