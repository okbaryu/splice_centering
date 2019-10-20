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

#define MAXHOSTNAME       80
#define CONTENT_TYPE_HTML "Content-type: text/html\n\n"
#define MYUUID_LEN 16

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

static int outputDashTickLines( const char *svg_text_id )
{
	int            tickidx      = 0;
	int            penoff       = 0;
	char           l_text_id[32];

	/* make a vertical tick mark every 10% */
	for (tickidx = 1; tickidx<10; tickidx++)
	{
		penoff = ( 1-tickidx%2 ) * 5;          /* dashed line for 20,40,60,80; solid line for 10,30,50,70,90 */
		/* for dasharray: how many pixels will the pen be on ... how many pixels will the pen be off. 5 on 5 off is a dash; 5 on 0 off is solid */
		PrintHTML( "<path d=\"M0 %d L500 %d\" stroke=lightgray stroke-width=1 stroke-dasharray=\"5,%d\" />", tickidx*10, tickidx*10, penoff );
		if (( tickidx%2 ) == 1)                                                           /* output text for 10, 30, 50, 70, 90 */
		{
			l_text_id[0] = '\0';
			if ( svg_text_id ) sprintf( l_text_id, "id=%s_%u", svg_text_id, tickidx/2 ); /* if user provided a string id, append idx to end of string and use it as text id */
			PrintHTML( "<text x=2 y=%d %s >%d</text>\n", tickidx*10+3,  /* offset 3 pixels to drop the number into the middle of the tickmark */
					l_text_id, ( 100 - ( tickidx*10 )));
		}
	}

	return ( 0 );
}

int main(
		int   argc,
		char *argv[]
		)
{
	int                   idx = 0;
	int                   rc  = 0;
	int                   cpu = 0;
	int                   irq = 0;
	struct sockaddr_in    server;
	char                  ThisHost[80];
	splice_request      request;
	splice_response     response;
	splice_version_info versionInfo;
	char                  valueStr[64];
	int                   numCpusConf =  sysconf( _SC_NPROCESSORS_CONF );
	unsigned long int     irqTotal    = 0;
	unsigned long int     irqTotal2   = 0;
	char                  irqTotalStr[64];

	char *queryString      = NULL;
	int   epochSeconds     = 0;
	int   tzOffset         = 0;
	int   cpuInfo          = 0;
	int   netStatsInit     = 0; // when true, send to browser the Net Stats html table structure
	int   netStatsUpdate   = 0; // when true, send network interface values from last second; browser will populate the html table structure
	int   irqInfo          = 0;
	int   sataUsb          = 0;
	char  strUuid[MYUUID_LEN*2+1];

	struct utsname uname_info;

	if (argc > 1) {PrintError( "%s: no arguments are expected\n", argv[0] ); }

	memset( &versionInfo, 0, sizeof( versionInfo ));
	memset( &response, 0, sizeof( response ));
	memset( &irqTotalStr, 0, sizeof( irqTotalStr ));
	memset( strUuid, 0, sizeof(strUuid) );
	memset( &uname_info, 0, sizeof(uname_info));

	queryString   = getenv( "QUERY_STRING" );

	if (queryString && strlen( queryString ))
	{
		scanForInt( queryString, "datetime=", &epochSeconds );
		scanForInt( queryString, "tzoffset=", &tzOffset );
		scanForInt( queryString, "cpuinfo=", &cpuInfo );
		scanForInt( queryString, "netStatsInit=", &netStatsInit );
		scanForInt( queryString, "netStatsUpdate=", &netStatsUpdate );
		scanForInt( queryString, "irqinfo=", &irqInfo );
		scanForInt( queryString, "satausb=", &sataUsb );
		scanForStr( queryString, "uuid=", sizeof( strUuid ), strUuid );
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
		int            cpupair      = 0;
		int            leftright    = 0;
		int            leftrightmax = 1;
		struct timeval now          = {1400000000, 0};

		strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
		strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
		versionInfo.majorVersion   = MAJOR_VERSION;
		versionInfo.minorVersion   = MINOR_VERSION;
		versionInfo.sizeOfResponse = sizeof( response );
		PrintHTML( "~PLATFORM~%s\n", versionInfo.platform );
		PrintHTML( "~PLATVER~%s\n", versionInfo.platVersion );
		PrintHTML( "~VERSION~Ver: %u.%u~\n", versionInfo.majorVersion, versionInfo.minorVersion );

		uname(&uname_info);
		PrintHTML("~UNAME~%d-bit %s %s~\n", (sizeof(char*) == 8)?64:32, uname_info.machine , uname_info.release );

		if (numCpusConf % 2 == 0)
		{
			leftrightmax = 2;
		}

		now.tv_sec = epochSeconds - ( tzOffset * 60 );
		settimeofday( &now, NULL );
		usleep( 200 );

		PrintHTML( "~CPUPERCENTS~" );
		PrintHTML( "<table cols=2 border=0 id=cpugraphs style=\"border-collapse:collapse;\" >\n" );

		for (cpupair = 0; cpupair < numCpusConf; cpupair += 2) {
			PrintHTML( "<tbody>\n" );
			PrintHTML( "  <tr id=row%02ua style=\"visibility:hidden;\" >\n", cpupair + 1 );
			for (leftright = 0; leftright < leftrightmax; leftright++) {
				PrintHTML( "    <th id=%scol%02ua  align=center valign=bottom >\n", ( leftright == 0 ) ? "left" : "right", cpupair + 1 );
				PrintHTML( "      <table cols=3 border=0 style=\"border-collapse:collapse;\" ><tr>\n" );
				PrintHTML( "          <th id=cpuoverall align=left  width=230 style=\"color:red;\" >&nbsp;</th>\n" );
				PrintHTML( "          <th align=center width=208 ><span id=cputitle%02u >CPU %u</span></th>",
						cpupair + leftright, cpupair + leftright );
				PrintHTML( "          <th width=50 align=center id=ChangeCpuTag%u title=\"Click to disable/enable CPU %u\" >",
						cpupair + leftright, cpupair + leftright );
				PrintHTML( "&nbsp;" );
				/*
				   Javascript will insert something similar to this based on which CPU comes back enabled and which is disabled
				   <svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick="MyClick(event);" id=ChangeCpuState3 fill=lime /></svg>
				   */
				PrintHTML( "      </tr></table>\n" );
				PrintHTML( "    </th>\n" );
			}
			PrintHTML( "  </tr>\n" );
			PrintHTML( "  <tr id=row%02ub style=\"visibility:hidden;\" >\n", cpupair + 1 );
			for (leftright = 0; leftright < leftrightmax; leftright++) {
				PrintHTML( "    <td id=%scol%02ub  >\n", ( leftright == 0 ) ? "left" : "right", cpupair + 1 );
				PrintHTML( "      <svg id=svg%02u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >"
						"<polyline id=polyline%02u style=\"fill:none;stroke:blue;stroke-width:2\" />\n", cpupair + 1, cpupair + 1 + leftright );
				/* for the very first one, create a polyline to be used for the average cpu utilization ... polyline00 */
				if (( cpupair == 0 ) && ( leftright == 0 ))
				{
					PrintHTML( "<polyline id=polyline00 style=\"fill:none;stroke:red;stroke-width:2\" />\n" );
					/* add a polyline for the limegreen 5-second CPU utilization average */
					PrintHTML( "<polyline id=polyline0%u style=\"fill:none;stroke:limegreen;stroke-width:2\" />\n", numCpusConf + 1 );
				}

				outputDashTickLines( NULL );

				PrintHTML( "</svg>\n" );
				PrintHTML( "      </td>\n" );
			}
			PrintHTML( "  </tr>\n" );
			PrintHTML( "  <tr id=row%02uc style=\"visibility:hidden;\" >\n", cpupair + 1 );
			for (leftright = 0; leftright < leftrightmax; leftright++) {
				PrintHTML( "    <td id=%scol%02uc align=left valign=bottom ><textarea rows=1 id=cpudata%02u style=\"border:solid 1px black;width:100%%;\" >&nbsp;</textarea></td>\n",
						( leftright == 0 ) ? "left" : "right", cpupair + 1, cpupair + leftright );
			}
			PrintHTML( "  </tr>\n" );
			PrintHTML( "</tbody>\n" );
		}
		PrintHTML( "</table>~" );
	}

	/* if the checkbox for CPU Utilization OR Network Stats OR IRQ Counts is checked (any one of these needs to request data from splice_server) */
	if (cpuInfo || netStatsUpdate || irqInfo || sataUsb)
	{
		struct in_addr sin_temp_addr;

		strncpy( ThisHost, "localhost", sizeof( ThisHost ));
		getservbyname( "echo", "tcp" );

		strncpy( ThisHost, "localhost", sizeof( ThisHost ));

		sin_temp_addr.s_addr = get_my_ip4_addr();

		if ( sin_temp_addr.s_addr == 0 )
		{
			PrintHTML( "~FATAL~get_my_ip4_addr() failed to determine IP address.~" );
			return( -1 );
		}
		bcopy( &sin_temp_addr.s_addr, &( server.sin_addr ), sizeof( sin_temp_addr.s_addr ) );
		/*printf( "~TCP/Client running at HOST (%s) at INET ADDRESS : (%s)~", ThisHost, inet_ntoa( server.sin_addr ));*/

		/* Construct name of socket to send to. */
		server.sin_family = AF_INET;
		server.sin_port   = htons( SPLICE_SERVER_PORT );

		if (sataUsb)
		{
			if (sataUsb == 1)                              /* user requested we start data collection */
			{
				PrintInfo( "~Sending SPLICE_CMD_START_SATA_USB request~" );
				request.cmdSecondary = SPLICE_CMD_START_SATA_USB;
			}
			else                                           /* user requested we stop data collection */
			{
				PrintInfo( "~Sending SPLICE_CMD_STOP_SATA_USB request~" );
				request.cmdSecondary = SPLICE_CMD_STOP_SATA_USB;
			}
			request.cmdSecondaryOption = request.cmdSecondary;
		}


		PrintInfo( "~DEBUG~Sending request ... cmdSecondary 0x%x ... option 0x%lx ~", request.cmdSecondary, request.cmdSecondaryOption );
		rc = send_request_read_response( &server, (unsigned char*) &request, sizeof(request), (unsigned char*) &response, sizeof(response), SPLICE_SERVER_PORT, SPLICE_CMD_GET_CPU_IRQ_INFO );
		if (rc < 0)
		{
			PrintError( "error sending SPLICE_CMD_GET_CPU_IRQ_INFO request; rc %d \n", rc );
			PrintHTML( "~FATAL~ERROR connecting to server. Make sure splice_server is running.~" );
			return( -1 );
		}
		PrintInfo( "Received from server: cmd (%d)\n", response.cmd );
		irqTotal2 = response.response.cpuIrqData.irqData.irqTotal;

	}

	/* if the checkbox for CPU Utilization is checked */
	if (cpuInfo)
	{
		/* output the CPU utilization data */
		PrintHTML( "~CPUINFO~%u ", numCpusConf );
		for (cpu = 0; cpu < numCpusConf; cpu++) {
			/* if the CPU is not active */
			if (response.response.cpuIrqData.cpuData.idlePercentage[cpu] == 255)
			{
				PrintHTML( "%06u ", response.response.cpuIrqData.cpuData.idlePercentage[cpu] );
			}
			else
			{
				PrintHTML( "%06u ", 100 - response.response.cpuIrqData.cpuData.idlePercentage[cpu] );
			}
		}
		PrintHTML( "~" );

		if(response.profile == 0xFFAA){
			PrintHTML( "~BACKGROUND");
		}

		output_cpu_frequencies();
	}                                                      /* cpuInfo */

	/* if the checkbox for IRQ Stats is checked */
	if (irqInfo)
	{
		/* output the IRQ data */
		PrintHTML( "~IRQINFO~%u ", response.response.cpuIrqData.cpuData.numActiveCpus );
		for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
			PrintHTML( "%06lu ", response.response.cpuIrqData.irqData.irqCount[cpu] );
			irqTotal += response.response.cpuIrqData.irqData.irqCount[cpu];
		}
		PrintHTML( "~" );

		qsort( &response.response.cpuIrqData.irqData.irqDetails[0], SPLICE_IRQ_MAX_TYPES,
				sizeof( response.response.cpuIrqData.irqData.irqDetails[0] ), sort_on_irq0 );

		PrintHTML( "~DEBUG~irqTotal (%lu); irqTotal2 (%lu)~", irqTotal, irqTotal2 );
		/* convert 999999 to 999,999 ... e.g. add commas */
		convert_to_string_with_commas( irqTotal,  irqTotalStr, sizeof( irqTotalStr ));

		PrintHTML( "~IRQDETAILS~" );
		PrintHTML( "<table cols=%u style=\"border-collapse:collapse;\" border=1 cellpadding=3 >", response.response.cpuIrqData.cpuData.numActiveCpus + 1 );

		/* the first pass through, output the table description and table header row */
		PrintHTML( "<tr><th colspan=%u class=whiteborders18 align=left >%s</th></tr>", response.response.cpuIrqData.cpuData.numActiveCpus + 1,
				"Interrupt Counts Since Bootup" );
		PrintHTML( "<tr><th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" width=50 >%s</th><th class=whiteborders18 colspan=%u "
				"style=\"border-left:solid thin black;\" ></th></tr>", irqTotalStr, response.response.cpuIrqData.cpuData.numActiveCpus );

		PrintHTML( "<tr bgcolor=lightgray >" );

		/* output the header line for the cpus */
		for (cpu = 0; cpu < numCpusConf; cpu++) {
			/* if the cpu is not inactive */
			if (response.response.cpuIrqData.cpuData.idlePercentage[cpu] != 255)
			{
				PrintHTML( "<td width=130 >CPU %u (delta)</td>", cpu );
			}
		}
		PrintHTML( "<td>IRQ Description</td></tr>\n" );
		for (irq = 0; irq < SPLICE_IRQ_MAX_TYPES; irq++) {
			/* if the entry has a valid IRQ name */
			if (strlen( response.response.cpuIrqData.irqData.irqDetails[irq].irqName ))
			{
				unsigned long int summation = 0;
				/* determine if something on the row is non-zero */
				for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
					summation += response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu];
				}

				/* if at least one of the numbers on the row is non-zero, then display the row */
				if (summation)
				{
					PrintHTML( "<tr>" );
					for (cpu = 0; cpu < response.response.cpuIrqData.cpuData.numActiveCpus; cpu++) {
						valueStr[0] = 0;
						convert_to_string_with_commas( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu], valueStr, sizeof( valueStr ));
						PrintHTML( "<td>%s", formatul( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] ));
						/* if there is a non-zero delta from the previous pass, display the delta in parens; otherwise no need to display a zero in parens */
						if (( response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] - response.response.cpuIrqData.irqData.irqDetails[irq].irqCountPrev[cpu] ) > 0)
						{
							valueStr[0] = 0;
							convert_to_string_with_commas(
									response.response.cpuIrqData.irqData.irqDetails[irq].irqCount[cpu] - response.response.cpuIrqData.irqData.irqDetails[irq].irqCountPrev[cpu],
									valueStr, sizeof( valueStr ));
							PrintHTML( " (%s)", valueStr );
						}
						PrintHTML( "</td>" );
					}
					PrintHTML( "<td>%s</td></tr>", response.response.cpuIrqData.irqData.irqDetails[irq].irqName );
				}
				else
				{
					/*printf("<tr><td colspan=5> all numbers are zero</td></tr>\n");*/
				}
			}
		}

		PrintHTML( "</table>~" );                             /* end IRQDETAILS */
	}

	/* if the checkbox for Network Stats was just checked */
	if ( netStatsInit )
	{
		char dash_line_id[32];

		get_netstat_data( &g_netStats[0] );

		PrintHTML( "~netStatsInit~" );
		PrintHTML( "<table cols=9 width=\"1024\" style=\"border-collapse:collapse;\" border=0 cellpadding=0 >" );

		PrintHTML( "<tr><td class=whiteborders18 align=left style=\"font-weight:bold;width:320px;\" >Network Interface Statistics</td>" );
		//PrintHTML( "<td style=\"width:20px;\" ><input type=checkbox id=checkboxiperfrow onclick=\"MyClick(event);\" ></td>");
		//PrintHTML( "<td style=\"width:50px;\" align=left >iperf</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "<td>&nbsp;</td>" );
		PrintHTML( "</tr>" );
		PrintHTML( "<tr><td colspan=9><table width=\"100%%\" style=\"border-collapse:collapse;\" border=1 cellpadding=3 >\n" );
		//PrintHTML( "<tr bgcolor=lightgray ><th>Name</th><th>IP Addr</th><th>Rx Bytes</th><th>Tx Bytes</th><th>Rx Errors</th><th>Tx Errors</th>"
		//		"<th>Rx Mbps (Avg)</th><th>Tx Mbps (Avg)</th><th>Graph</th></tr>\n" );
		PrintHTML( "<tr bgcolor=lightgray ><th>Name</th><th>IP Addr</th><th>Rx Bytes</th><th>Tx Bytes</th><th>Rx Errors</th><th>Tx Errors</th>"
				"<th>Rx Mbps (Avg)</th><th>Tx Mbps (Avg)</th></tr>\n" );
		for (idx = 0; idx <= g_netStatsIdx; idx++) {
			PrintHTML( "<tr><td id=ethname%d>%s</td> <td align=center >%s</td> <td align=center id=netif_rxBytes_%d >%s</td>",
					idx, g_netStats[idx].name, g_netStats[idx].ipAddress, idx, formatul( g_netStats[idx].rxBytes ));
			PrintHTML( "<td id=netif_txBytes_%d align=center >%s</td>", idx, formatul( g_netStats[idx].txBytes ));
			PrintHTML( "<td id=netif_rxError_%d align=center >%s</td>", idx, formatul( g_netStats[idx].rxErrors ));
			PrintHTML( "<td id=netif_txError_%d align=center >%s</td>", idx, formatul( g_netStats[idx].txErrors ));
			PrintHTML( "<td id=netif%urx align=center ><!-- value inserted via javascript --></td>", idx );
			PrintHTML( "<td id=netif%utx align=center ><!-- value inserted via javascript --></td>", idx );
			//PrintHTML( "<td align=center ><input type=checkbox id=checkbox_netgfx%u onclick=\"MyClick(event);\" ></td>", idx );
			PrintHTML( "</tr>\n" );

			/* add in the histogram for Rx and Tx */
			PrintHTML( "<tr id=row_netgfxsvg%u style=\"visibility:hidden;\" ><td colspan=9 align=left "
					"style=\"border-right: 1pt solid white;border-left: 1pt solid white;\" ><table style=\"border-collapse:collapse;\" border=0 ><tr>", idx );
			PrintHTML( "<td width=500 align=left ><svg id=svg_netgfx_rx_%u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >", idx );
			PrintHTML( "<polyline id=polyline_netgfx_rx_%u style=\"fill:none;stroke:red;stroke-width:2\" />\n", idx );
			sprintf(dash_line_id, "dash_rx_%u", idx );
			outputDashTickLines( dash_line_id );
			PrintHTML( "<text x=30 y=13>RX</text>" );
			PrintHTML( "</svg></td>" );
			PrintHTML( "<td width=500 align=left ><svg id=svg_netgfx_tx_%u height=\"100\" width=\"500\" style=\"border:solid 1px black;font-size:8pt;\" >", idx );
			PrintHTML( "<polyline id=polyline_netgfx_tx_%u style=\"fill:none;stroke:turquoise;stroke-width:2\" />\n", idx );
			sprintf(dash_line_id, "dash_tx_%u", idx );
			outputDashTickLines( dash_line_id );
			PrintHTML( "<text x=30 y=13>TX</text>" );
			PrintHTML( "</svg></td>" );
			PrintHTML( "</tr>" );
			PrintHTML( "<tr id=row_netgfxtxt%u style=\"visibility:hidden;\" >", idx );
			PrintHTML( "<td width=500 align=left ><textarea id=txt_netgfx_rx_%u rows=1 style=\"border:solid 1px black;width:100%%;\" ></textarea></td>", idx );
			PrintHTML( "<td width=500 align=left ><textarea id=txt_netgfx_tx_%u rows=1 style=\"border:solid 1px black;width:100%%;\" ></textarea></td>", idx );
			PrintHTML( "</tr>" );
			PrintHTML( "</table></td>" );
		}
		PrintHTML( "</table></td></tr>\n" );
		PrintHTML( "</table>~" ); /* end netStatsInit */
	}

	/* if the checkbox for Network Stats is checked */
	if ( netStatsUpdate )
	{
		get_netstat_data( &g_netStats[0] );

		PrintHTML( "~netStatsUpdate~%d~", g_netStatsIdx + 1 );
		for (idx = 0; idx <= g_netStatsIdx; idx++) {
			PrintHTML( "%s|", formatul( g_netStats[idx].rxBytes ));
			PrintHTML( "%s|", formatul( g_netStats[idx].txBytes ));
			PrintHTML( "%s|", formatul( g_netStats[idx].rxErrors ));
			PrintHTML( "%s~", formatul( g_netStats[idx].txErrors ));
		}
		/* end netStatsUpdate */

		/* output some of the above information again in an unformatted way to make it easier to compute bits per second */
		PrintHTML( "~NETBYTES~" );
		for (idx = 0; idx <= g_netStatsIdx; idx++) {
			PrintHTML( "%llu ", g_netStats[idx].rxBytes );
			PrintHTML( "%llu,", g_netStats[idx].txBytes );
		}
		PrintHTML( "~" );
	}

	/* if the checkbox for SATA/USB is checked */
	if (sataUsb)
	{
		unsigned int idx = 0;
		char         debugBuffer[1024];
		memset( debugBuffer, 0, sizeof( debugBuffer ));

		PrintHTML( "~SATAUSB~" );
		PrintHTML( "<table id=satauabtable cols=3 border=0 style=\"border-collapse:collapse;\" cellpadding=5 >" );
		PrintHTML( "  <tr id=satausbrow1222 ><th align=left colspan=3 style=\"font-size:18pt;\" ><b>SATA/USB Information</b></th></tr>" );
		PrintHTML( "<tr><th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Device</th>"
				"<th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Read&nbsp;Mbps</th>"
				"<th class=allborders50 style=\"background-color:lightgray;font-size:12pt;\" >Write&nbsp;Mbps</th>"
				"</tr>" );
		for (idx = 0; idx<sizeof( response.response.cpuIrqData.sataUsbData )/sizeof( response.response.cpuIrqData.sataUsbData[0] ); idx++)
		{
			if (response.response.cpuIrqData.sataUsbData[idx].deviceName[0] != '\0')
			{
				PrintHTML( " <tr id=satausbrow2222 ><td class=allborders50 >%s</td><td class=allborders50 align=center >",
						response.response.cpuIrqData.sataUsbData[idx].deviceName );
				if (response.response.cpuIrqData.sataUsbData[idx].readMbps > 0.0)
				{
					PrintHTML( "%6.1f", response.response.cpuIrqData.sataUsbData[idx].readMbps );
				}
				PrintHTML( "</td><td class=allborders50 align=center >" );
				if (response.response.cpuIrqData.sataUsbData[idx].writeMbps > 0.0)
				{
					PrintHTML( "%6.1f", response.response.cpuIrqData.sataUsbData[idx].writeMbps );
				}
				PrintHTML( "</td></tr>" );
				{
					char line[64];
					sprintf( line, "~SATADEBUG~%s rd %6.1f wr %6.1f~", response.response.cpuIrqData.sataUsbData[idx].deviceName,
							response.response.cpuIrqData.sataUsbData[idx].readMbps,
							response.response.cpuIrqData.sataUsbData[idx].writeMbps  );
					strncat( debugBuffer, line, sizeof( debugBuffer ) - strlen( debugBuffer ) - 1 );
				}
			}
		}
		PrintHTML( "</table>" );
		PrintHTML( "%s", debugBuffer );
	}

	PrintHTML( "~SPLICETIME~%s~", DayMonDateYear( 0 ));

	PrintHTML( "~ALLDONE~" );

	return( 0 );
}
