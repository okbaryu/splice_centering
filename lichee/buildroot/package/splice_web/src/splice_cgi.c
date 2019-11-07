#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>

#include "splice_libs.h"
#include "splice_utils.h"
#include "actuator.h"

#define MAXHOSTNAME       80
#define CONTENT_TYPE_HTML "Content-type: text/html\n\n"
#define MYUUID_LEN 16

#define WEBSPLICE

typedef struct
{
	unsigned long long int rxErrors;
	unsigned long long int txErrors;
	unsigned long long int rxBytes;
	unsigned long long int txBytes;
	char                   name[16];
	char                   ipAddress[32];
} splice_netStatistics;

#define NET_STATS_MAX       10
#define SPLICE_VALUE_BASE 10
splice_netStatistics g_netStats[NET_STATS_MAX];
int g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

char outString[32];

/**
 *  Function: This function will format an integer to output an indicator for kilo, mega, or giga.
 **/
char *formatul(
		unsigned long long int value
		)
{
	float fValue = value;

	if (value < 1024)
	{
		sprintf( outString, "%llu", value );
	}
	else if (value < 1024 * 1024)
	{
		sprintf( outString, "%5.1f K", fValue / 1024 );
	}
	else if (value < 1024 * 1024 * 1024)
	{
		sprintf( outString, "%6.2f M", fValue / 1024 / 1024 );
	}
	else
	{
		sprintf( outString, "%7.3f G", fValue / 1024 / 1024 / 1024 );
	}

	return( outString );
}

/**
 *  Function: This function will look in the provided string for the specified tag string. If the tag string is
 *  found on the line, the converted integer value will be returned.
 **/
int scan_for_tag(
		const char             *line,
		const char             *tag,
		unsigned long long int *value
		)
{
	int                     rc     = -1;
	char                   *pos    = NULL;
	unsigned long long int  lvalue = 0;
	char                    valueStr[64];

	pos = strstr( line, tag );
	if (pos)
	{
		pos   += strlen( tag );
		lvalue = strtoull( pos, NULL, SPLICE_VALUE_BASE );
		if (strstr( tag, "RX packets:" ) && ( lvalue > 0 ))
		{
			valueStr[0] = 0;
			convert_to_string_with_commas( lvalue, valueStr, sizeof( valueStr ));
			PrintInfo( "%s %llu (%s) (%s) (addr -> %p)<br>\n", tag, lvalue, valueStr, pos, (void *)value );
		}

		if (value)
		{
			*value = lvalue;
		}

		rc = 0;
	}
	/* -1 is used to indicate the tag was not found; rc=0 indicates the tag was found and some values were scanned */
	return( rc );
}

int get_netstat_data(
		splice_netStatistics *pNetStats
		)
{
	char *pos = NULL;
	FILE *cmd = NULL;
	char  line[MAX_LINE_LENGTH];

	if (pNetStats == NULL)
	{
		return( -1 );
	}

	/* clear out the array */
	memset( pNetStats, 0, sizeof( *pNetStats ));

	sprintf( line, "%s", IFCONFIG_UTILITY );
	cmd = popen( line, "r" );

	do {
		memset( line, 0, sizeof( line ));
		fgets( line, MAX_LINE_LENGTH, cmd );
		PrintInfo( "got len %lu: line (%s)<br>\n", strlen( line ), line );
		if (strlen( line ))
		{
			/* if something is in column 1, it must be a name of another interface */
			if (( 'a' <= line[0] ) && ( line[0] <= 'z' ))
			{
				/* if there is room for another interface */
				if (g_netStatsIdx < NET_STATS_MAX)
				{
					g_netStatsIdx++;

					/* look for a space; that marks the end of the i/f name */
					pos = strchr( line, ' ' );
					if (pos)
					{
						*pos = '\0';                       /* null-terminate the i/f name */
						strncpy( pNetStats[g_netStatsIdx].name, line, sizeof( pNetStats[g_netStatsIdx].name ) - 1 );
						PrintInfo( "g_netStats[%u].name is (%s)<br>\n", g_netStatsIdx, pNetStats[g_netStatsIdx].name );
					}
				}
				else
				{
					PrintError( "not enough room for new interface (%s) in array; max'ed out at %u\n",  line, g_netStatsIdx );
				}
			}

			/* if we haven't found the first interface name yet, keep looking for the next line */
			if (g_netStatsIdx >= 0)
			{
				int rc = 0;

				/* if line has IP address on it */
				if (( pos = strstr( line, "inet addr:" )))
				{
					char *ipAddrStart = NULL;

					pos += strlen( "inet addr:" );

					ipAddrStart = pos;

					/* look for a space; that marks the end of the i/f name */
					pos = strchr( ipAddrStart, ' ' );
					if (pos)
					{
						*pos = '\0';                       /* null-terminate the IP address */
						strncpy( pNetStats[g_netStatsIdx].ipAddress, ipAddrStart, sizeof( pNetStats[g_netStatsIdx].ipAddress ) - 1 );
						PrintInfo( "IF (%s) has IP addr (%s)<br>\n", pNetStats[g_netStatsIdx].name, pNetStats[g_netStatsIdx].ipAddress );
					}
				}
				/*
				   eth0      Link encap:Ethernet  HWaddr 00:10:18:D2:C3:C9
				   inet addr:10.14.244.188  Bcast:10.14.245.255  Mask:255.255.254.0
				   UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
				   RX packets:11832 errors:0 dropped:0 overruns:0 frame:0
				   TX packets:2150 errors:0 dropped:0 overruns:0 carrier:0
collisions:0 txqueuelen:1000
RX bytes:2429458 (2.3 MiB)  TX bytes:632148 (617.3 KiB)
*/

				rc = scan_for_tag( line, "RX packets:", &pNetStats[g_netStatsIdx].rxErrors );
				/* if the RX packets tag was found, now scan for errors tag on the same line */
				if (rc == 0)
				{
					rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].rxErrors );
				}

				rc = scan_for_tag( line, "TX packets:", &pNetStats[g_netStatsIdx].txErrors );
				/* if the TX packets tag was found, now scan for errors tag on the same line */
				if (rc == 0)
				{
					rc = scan_for_tag( line, " errors:", &pNetStats[g_netStatsIdx].txErrors );
				}
				scan_for_tag( line, "RX bytes:", &pNetStats[g_netStatsIdx].rxBytes );
				scan_for_tag( line, "TX bytes:", &pNetStats[g_netStatsIdx].txBytes );
				/*printf("~DEBUG~ip_addr (%s) ... TX bytes (%lld)~", pNetStats[g_netStatsIdx].ipAddress, pNetStats[g_netStatsIdx].txBytes );*/
			}
		}
	} while (strlen( line ));
	PrintInfo( "\n" );

	pclose( cmd );

	return( 0 );
}


int sort_on_irq0(
		const void *a,
		const void *b
		)
{
	int rc = 0;

	splice_irq_details *client_a = (splice_irq_details *)a;
	splice_irq_details *client_b = (splice_irq_details *)b;

	rc = client_b->irqCount[0] - client_a->irqCount[0];

	return( rc );
}

int create_uuid( char * strUuid )
{
	unsigned int  idx;
	unsigned long int myUuid[4];
	unsigned char     *uuid = (unsigned char *)myUuid;
	unsigned long int temp = 0;
	struct timespec time1;
	char   two_digits[3];

	memset ( myUuid, 0, MYUUID_LEN );
	memset ( &time1, 0, sizeof(time1) );

	clock_gettime( CLOCK_REALTIME, &time1 );

	srandom ( (unsigned int) time1.tv_nsec/1000 );

	/*printf("~DEBUG~uuid %p~\n", uuid );*/
	for (idx=0; idx<4; idx++)
	{
		myUuid[idx] = temp = random();
		/*printf("~DEBUG~random(%u) returned %08lx; uuid %p~\n", idx, temp, myUuid[idx] );*/
	}

	/*printf("~DEBUG~%s: myUuid: ", __FUNCTION__ );*/
	for (idx=0; idx<MYUUID_LEN; idx++)
	{
		/*printf( "%02x ", uuid[idx] );*/
		snprintf( two_digits, sizeof(two_digits), "%02x", uuid[idx] );
		strcat( strUuid, two_digits);
	}
	/*printf("~\n");*/
	/*printf("strUuid:(%s)\n", strUuid );*/

	return 0;
}


int main(
		int   argc,
		char *argv[]
		)
{
	int rc=0;
	splice_request      request;
	splice_response     response;
	splice_version_info versionInfo;

	char *queryString      = NULL;
	int   epochSeconds     = 0;
	int   tzOffset         = 0;
	int   systemInfo       = 0;
	int   getTime          = 0;
	int   actPos           = 0;
	int   getActStatus     = 0;
	int   setActStatus     = 0;
	int   ACTDIR           = 0;
	int   ACTSTROKE        = 0;
	int   ACTSPEED         = 0;
	int   ACTLLIMIT        = 0;
	int   ACTRLIMIT        = 0;
	int   ACTORG           = 0;
	char  strUuid[MYUUID_LEN*2+1];

	struct utsname uname_info;

	if (argc > 1) {PrintError( "%s: no arguments are expected\n", argv[0] ); }

	memset( &versionInfo, 0, sizeof( versionInfo ));
	memset( &response, 0, sizeof( response ));
	memset( strUuid, 0, sizeof(strUuid) );
	memset( &uname_info, 0, sizeof(uname_info));

	queryString   = getenv( "QUERY_STRING" );

	if (queryString && strlen( queryString ))
	{
		scanForInt( queryString, "datetime=", &epochSeconds );
		scanForInt( queryString, "tzoffset=", &tzOffset );
		scanForStr( queryString, "uuid=", sizeof( strUuid ), strUuid );
		scanForInt( queryString, "systemInfo=", &systemInfo);
		scanForInt( queryString, "getTime=", &getTime);
		scanForInt( queryString, "actPos=", &actPos);
		scanForInt( queryString, "getActStatus=", &getActStatus);
		scanForInt( queryString, "setActStatus=", &setActStatus);
		scanForInt( queryString, "ACTDIR=", &ACTDIR);
		scanForInt( queryString, "ACTSTROKE=", &ACTSTROKE);
		scanForInt( queryString, "ACTSPEED=", &ACTSPEED);
		scanForInt( queryString, "ACTLLIMIT=", &ACTLLIMIT);
		scanForInt( queryString, "ACTRLIMIT=", &ACTRLIMIT);
		scanForInt( queryString, "ACTORG=", &ACTORG);
	}
	else
	{
		PrintHTML( CONTENT_TYPE_HTML );
		PrintError( "~ERROR: QUERY_STRING is not defined~" );
		return( -1 );
	}

	PrintHTML( CONTENT_TYPE_HTML );

	/* if browser provided a new date/time value; this only happens once during initialization */
	if (epochSeconds)
	{
		struct timeval now          = {1400000000, 0};

		now.tv_sec = epochSeconds - ( tzOffset * 60 );
		settimeofday( &now, NULL );
		usleep( 200 );
	}

	if(systemInfo)
	{
		versionInfo.webMjVersion = WEB_MAJOR_VERSION;
		versionInfo.webMnVersion = WEB_MINOR_VERSION;
		versionInfo.spliceMnVersion = SPLICE_MINOR_VERSION;
		versionInfo.spliceMjVersion = SPLICE_MAJOR_VERSION;
		versionInfo.cameraMnVersion = CAMERA_MINOR_VERSION;
		versionInfo.cameraMjVersion = CAMERA_MAJOR_VERSION;
		versionInfo.sizeOfResponse = sizeof( response );

		PrintHTML( "~WEBVERSION~%u.%u\n", versionInfo.webMjVersion, versionInfo.webMnVersion);
		PrintHTML( "~SPLICEVERSION~%u.%u\n", versionInfo.spliceMjVersion, versionInfo.spliceMnVersion);
		PrintHTML( "~CAMERAVERSION~%u.%u\n", versionInfo.cameraMjVersion, versionInfo.cameraMnVersion);
		uname(&uname_info);
		PrintHTML("~UNAME~%d-bit %s %s~\n", (sizeof(char*) == 8)?64:32, uname_info.machine , uname_info.release );

		get_netstat_data( &g_netStats[0] );
		PrintHTML( "~SERVERIP~ %s\n", g_netStats[0].ipAddress);
	}

	if(getActStatus)
	{
		rc = send_request_read_response((unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), SPLICE_SERVER_PORT, SPLICE_CMD_GET_ACTUATOR_STATUS);

		PrintHTML( "~ACTDIR~%d~", response.data[0]);
		PrintHTML( "~ACTSTROKE~%d~", response.data[1]);
		PrintHTML( "~ACTSPEED~%d~", response.data[2]);
		PrintHTML( "~ACTLLIMIT~%d~", response.data[3] * 100);
		PrintHTML( "~ACTRLIMIT~%d~", response.data[4] * 100);
		if(response.data[5] & 0x80)
		{
			PrintHTML( "~ACTORG~%d~", ((response.data[5] & 0x7F) << 8 | response.data[6]) * -1);
		}
		else
		{
			PrintHTML( "~ACTORG~%d~", response.data[5] << 8 | response.data[6]);
		}
	}

	if(setActStatus)
	{
		request.request.strCmdLine[0] = ACTDIR;
		request.request.strCmdLine[1] = ACTSTROKE;
		request.request.strCmdLine[2] = ACTSPEED;
		request.request.strCmdLine[3] = ACTLLIMIT;
		request.request.strCmdLine[4] = ACTRLIMIT;
		request.request.strCmdLine[5] = 0;
		request.request.strCmdLine[6] = 0;
		if(ACTORG < 0)
		{
			request.request.strCmdLine[5] = 0x80;
		}
		request.request.strCmdLine[5] |= (ACTORG & 0x7F00) >> 8;
		request.request.strCmdLine[6] = ACTORG & 0xFF;

		rc = send_request_read_response((unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), SPLICE_SERVER_PORT, SPLICE_CMD_SET_ACTUATOR_STATUS);
	}

	if(actPos)
	{
		request.cmdSecondary = CMD2_ACT_MOVE;
		request.cmdSecondaryOption = actPos;

		rc = send_request_read_response((unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), SPLICE_SERVER_PORT, SPLICE_CMD_SET_ACTUATOR_POSITION);

	}

	if(getTime)
		PrintHTML( "~SPLICETIME~%s~", DayMonDateYear( 0 ));

	PrintHTML( "~ALLDONE~" );

	return( 0 );
}
