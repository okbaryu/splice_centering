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
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <dirent.h>
#include <time.h>
#include <poll.h>

#include "splice_libs.h"
#include "splice_utils.h"

static splice_cpu_percent g_cpuData;

static float gUptime = 0.0;

/**
 *  Function: This function will return a string that contains the current date and time.
 **/
char *DateYyyyMmDdHhMmSs(
		void
		)
{
	static char    fmt [64];
	struct timeval tv;
	struct tm     *tm;

	memset( fmt, 0, sizeof( fmt ));
	gettimeofday( &tv, NULL );
	if (( tm = localtime( &tv.tv_sec )) != NULL)
	{
		strftime( fmt, sizeof fmt, "%Y%m%d-%H%M%S", tm );
	}

	return( fmt );
}

char *DayMonDateYear(
		unsigned long int timestamp
		)
{
	static char    fmt[64];
	struct timeval tv;
	struct tm     *tm;

	memset( fmt, 0, sizeof( fmt ));

	if (timestamp == 0)
	{
		gettimeofday( &tv, NULL );
	}
	else
	{
		tv.tv_sec = timestamp;
	}

	if (( tm = localtime( &tv.tv_sec )) != NULL)
	{
		strftime( fmt, sizeof fmt, "%a %b %d, %Y %H:%M:%S", tm );
	}

	return( fmt );
}

int getUpTime(
		float *uptime
		)
{
	if (uptime == NULL)
	{
		return( 0 );
	}

	*uptime = gUptime;

	return( 0 );
}  

int setUpTime(
		void
		)
{
	size_t numBytes     = 0;
	FILE  *fpProcUptime = NULL;
	char   bufProcStat[256];

	fpProcUptime = fopen( "/proc/uptime", "r" );
	if (fpProcUptime==NULL)
	{
		PrintError( "could not open /proc/uptime\n" );
		return( -1 );
	}

	numBytes = fread( bufProcStat, 1, sizeof( bufProcStat ), fpProcUptime );
	fclose( fpProcUptime );

	if (numBytes)
	{
		sscanf( bufProcStat, "%f.2", &gUptime );
#ifdef DEBUG
		{
			static float uptimePrev = 0;
			PrintInfo( "uptime: %8.2f (delta %8.2f)\n", gUptime, ( gUptime - uptimePrev ));
			uptimePrev = gUptime;
		}
#endif
	}
	return( 0 );
}/* setUptime */


int getHostByAddr(
		const char *HostName,
		int         port,
		char       *HostAddr,
		int         HostAddrLen
		)
{
	char            portStr[8];
	struct addrinfo hints, *res, *p;
	int             status = 0;

	if (HostAddr == NULL)
	{
		PrintError("HostAddr is null\n" );
		return( 2 );
	}

	memset( &portStr, 0, sizeof portStr );
	memset( &hints, 0, sizeof hints );
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	sprintf( portStr, "%u", port );
	if (( status = getaddrinfo( HostName, portStr, &hints, &res )) != 0)
	{
		PrintError("etaddrinfo: %s\n", gai_strerror( status ));
		return( 2 );
	}

	for (p = res; p != NULL; p = p->ai_next)
	{
		void *addr;

		PrintInfo( "family is %u; AF_INET is %u\n", p->ai_family, AF_INET );
		if (p->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &( ipv4->sin_addr );

			/* convert the IP to a string and print it: */
			inet_ntop( p->ai_family, addr, HostAddr, HostAddrLen );

			PrintInfo( "Hostname: %s; IP Address: %s\n", HostName, HostAddr );
			break;
		}
		else
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &( ipv6->sin6_addr );
		}
	}

	return( 0 );
}

int scanForInt(
		const char *queryRequest,
		const char *formatStr,
		int        *returnValue
		)
{
	char  newstr[32];
	char *pos = strstr( queryRequest, formatStr );

	if (pos)
	{
		strncpy( newstr, formatStr, sizeof( newstr ));
		strncat( newstr, "%d", sizeof( newstr ));
		sscanf( pos, newstr, returnValue );
		PrintInfo( "%s is %d\n", formatStr, *returnValue );
	}
	return( 0 );
}

int scanForStr(
		const char  *queryRequest,
		const char  *formatStr,
		unsigned int returnValuelen,
		char        *returnValue
		)
{
	char *pos = strstr( queryRequest, formatStr );

	/*printf( "looking for (%s) in (%s); pos is (%s)<br>\n", formatStr, queryRequest, pos );*/
	if (pos)
	{
		char *delimiter = NULL;

		delimiter = strstr( pos+strlen( formatStr ), "&" );
		if (delimiter)
		{
			unsigned int len = delimiter - pos - strlen( formatStr );
			PrintInfo( "delimiter (%s)\n", delimiter );
			strncpy( returnValue, pos+strlen( formatStr ), len );
			returnValue[len] = '\0';
		}
		else
		{
			strncpy( returnValue, pos+strlen( formatStr ), returnValuelen );
		}
		PrintInfo( "%s is (%s)\n", formatStr, returnValue );
	}
	return( 0 );
}                                                          /* scanForStr */


/**
  This private function will compute the CPU utilization for each active CPU in the system. The percentage ranges
  from 0 to 100. This is a private function that gets called once a second in the dataFetchThread thread.
  It uses two files: (1) /proc/uptime and (2) /proc/stat

 **/
int P_getCpuUtilization(
		void
		)
{
	FILE                *fpProcStat = NULL;
	char                 bufProcStat[1024];
	int                  numCpusConf = 0;
	unsigned long int    clk         = 0;
	unsigned char        cpu         = 0;
	char                *pos         = NULL;
	char                 cpuTag[6];
	float                cpuPercent = 0.0;
	splice_cpu_percent gCpuDataNow;

	memset( &bufProcStat, 0, sizeof( bufProcStat ));
	memset( &gCpuDataNow, 0, sizeof( gCpuDataNow ));

	clk         = sysconf( _SC_CLK_TCK );
	numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
	if (numCpusConf > SPLICE_MAX_NUM_CPUS)
	{
		numCpusConf = SPLICE_MAX_NUM_CPUS;
	}

#ifdef DEBUG
	PrintInfo( "configured %u; clk_tck %lu\n", numCpusConf, clk );
#else
	UNUSED( clk );
#endif

	fpProcStat = fopen( "/proc/stat", "r" );
	if (fpProcStat == NULL)
	{
		PrintError( "could not open /proc/stat\n" );
	}
	else
	{
		getUpTime( &gCpuDataNow.uptime );

		fread( bufProcStat, 1, sizeof( bufProcStat ), fpProcStat );

		fclose( fpProcStat );

#ifdef DEBUG
		{
			char *posIntr = NULL;
			PrintInfo( "%s: uptime now (%8.2f); prev (%8.2f); delta (%8.2f)", __FUNCTION__, gCpuDataNow.uptime, g_cpuData.uptime,
					gCpuDataNow.uptime - g_cpuData.uptime );
			/* look for line that starts with "intr" */
			posIntr = strstr( bufProcStat, "intr" );       /* DEBUG */
			if (posIntr)
			{
				*posIntr = '\0';
				PrintInfo( "; /proc/stat\n(%s)", bufProcStat );
				*posIntr = 'i';
			}
			PrintInfo( "\n" );                                /* DEBUG */
		}
#endif /* ifdef  SPLICE_CGI_DEBUG */

		pos = bufProcStat;

		g_cpuData.numActiveCpus = sysconf( _SC_NPROCESSORS_ONLN ); /* will be lower than SC_NPROCESSORS_CONF if some CPUs are disabled */

		for (cpu = 0; cpu<numCpusConf; cpu++)
		{
			cpuPercent = 0.0;

			sprintf( cpuTag, "cpu%u ", cpu );
			pos = strstr( pos, cpuTag );
			if (pos)
			{
				pos += strlen( cpuTag );
				sscanf( pos, "%lu %lu %lu %lu %lu %lu %lu %lu ",
						&gCpuDataNow.user[cpu], &gCpuDataNow.nice[cpu], &gCpuDataNow.system[cpu], &gCpuDataNow.idle[cpu],
						&gCpuDataNow.cpu_iowait[cpu], &gCpuDataNow.cpu_irq[cpu], &gCpuDataNow.cpu_softirq[cpu], &gCpuDataNow.cpu_steal[cpu] );
				if (gCpuDataNow.uptime > g_cpuData.uptime)
				{
					/* compute the cpu percentage based on the current numbers minus the previous numbers */
					cpuPercent = ( gCpuDataNow.user[cpu] - g_cpuData.user[cpu] +
							gCpuDataNow.system[cpu] - g_cpuData.system[cpu] +
							gCpuDataNow.nice[cpu] - g_cpuData.nice[cpu] +
							gCpuDataNow.cpu_iowait[cpu] - g_cpuData.cpu_iowait[cpu] +
							gCpuDataNow.cpu_irq[cpu] - g_cpuData.cpu_irq[cpu] +
							gCpuDataNow.cpu_softirq[cpu] - g_cpuData.cpu_softirq[cpu] +
							gCpuDataNow.cpu_steal[cpu] - g_cpuData.cpu_steal[cpu]  ) /
						( gCpuDataNow.uptime-g_cpuData.uptime ) + .5 /* round */;
#if 0
					PrintInfo( "cpu %u: (%lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu + %lu - %lu ) / (%8.2f) + .5 = %6.2f;\n",
							cpu,
							gCpuDataNow.user[cpu], g_cpuData.user[cpu],
							gCpuDataNow.system[cpu], g_cpuData.system[cpu],
							gCpuDataNow.nice[cpu], g_cpuData.nice[cpu],
							gCpuDataNow.cpu_iowait[cpu], g_cpuData.cpu_iowait[cpu],
							gCpuDataNow.cpu_irq[cpu], g_cpuData.cpu_irq[cpu],
							gCpuDataNow.cpu_softirq[cpu], g_cpuData.cpu_softirq[cpu],
							gCpuDataNow.cpu_steal[cpu], g_cpuData.cpu_steal[cpu],
							gCpuDataNow.uptime-g_cpuData.uptime, cpuPercent );
#endif /* if 0 */
				}
				if (cpuPercent > 100.0)
				{
					cpuPercent = 100.0;
				}
				if (cpuPercent < 0.0)
				{
					cpuPercent = 0.0;
				}

				/* save the current data to the global structure to use during the next pass */
				gCpuDataNow.idlePercentage[cpu] = g_cpuData.idlePercentage[cpu] = (unsigned char) cpuPercent;
#ifdef DEBUG
				PrintInfo( "%3u ", gCpuDataNow.idlePercentage[cpu] );
#endif

				g_cpuData.user[cpu]        = gCpuDataNow.user[cpu];
				g_cpuData.nice[cpu]        = gCpuDataNow.nice[cpu];
				g_cpuData.system[cpu]      = gCpuDataNow.system[cpu];
				g_cpuData.cpu_iowait[cpu]  = gCpuDataNow.cpu_iowait[cpu];
				g_cpuData.cpu_irq[cpu]     = gCpuDataNow.cpu_irq[cpu];
				g_cpuData.cpu_softirq[cpu] = gCpuDataNow.cpu_softirq[cpu];
				g_cpuData.cpu_steal[cpu]   = gCpuDataNow.cpu_steal[cpu];
			}
			else
			{
#ifdef DEBUG
				PrintInfo( "cpu %u is offline\n", cpu );
#endif
				gCpuDataNow.idlePercentage[cpu] = g_cpuData.idlePercentage[cpu] = 255;
				pos = bufProcStat;                         /* pos is NULL at this point; reset it to start searching back at the beginning of the stat buffer */
			}
		}
#ifdef DEBUG
		PrintInfo( "\n" );
#endif

		g_cpuData.uptime = gCpuDataNow.uptime;
	}

	return( 0 );
}



/**
 *  Function: This function will use the output from the ifconfig utility to determine what our IP4 address is.
 **/
unsigned int get_my_ip4_addr( void )
{
	char  line[200];
	FILE *cmd = NULL;
	struct in_addr sin_temp_addr;

	memset( &sin_temp_addr, 0, sizeof( sin_temp_addr ) );

	sprintf( line, "/sbin/ifconfig | /bin/egrep 'Link encap|inet addr' | /usr/bin/awk '{if ( $1 == \"inet\"  && substr($2,6,3) != \"127\" ) { printf substr($2,6) \" \";} }'" );

	cmd = popen( line, "r" );

	memset( line, 0, sizeof( line ));
	fgets( line, sizeof(line), cmd );

	pclose( cmd );

	if ( strlen(line) > 1 )
	{
		int len = strlen(line);
		if ( line[len-1] == '\n') line[len-1] = 0;
	}

	inet_aton( line, &sin_temp_addr);
	PrintInfo("%s (0x%lx)~\n", line, (unsigned long int) sin_temp_addr.s_addr );

	return( sin_temp_addr.s_addr );
}

char *GetFileContents(
		const char *filename
		)
{
	char       *contents = NULL;
	FILE       *fpInput  = NULL;
	struct stat statbuf;

	if (lstat( filename, &statbuf ) == -1)
	{
		PrintError( "Could not stat (%s)\n", filename );
		return( NULL );
	}

	contents = malloc( statbuf.st_size + 1 );
	if (contents == NULL) {return( NULL ); }

	memset( contents, 0, statbuf.st_size + 1 );

	if (statbuf.st_size)
	{
		fpInput = fopen( filename, "r" );
		fread( contents, 1, statbuf.st_size, fpInput );
		fclose( fpInput );
	}

	return( contents );
}

/**
 *  Function: This function will return to the user the known temporary path name. In Linux system, this
 *  file will be /tmp/.
 **/
char *GetTempDirectoryStr(
		void
		)
{
	static char tempDirectory[TEMP_FILE_FULL_PATH_LEN] = "empty";
	char       *contents     = NULL;
	char       *posErrorLog  = NULL;
	char       *posLastSlash = NULL;
	char       *posEol       = NULL;

	PrintInfo( "tempDirectory (%s)\n~", tempDirectory );
	/* if the boa.conf file has no yet been scanned for the temporary directory, do it now */
	if (strncmp( tempDirectory, "empty", 5 ) == 0)
	{
		contents = GetFileContents( "boa.conf" );

		/* if the contents of boa.conf were successfully read */
		if (contents)
		{
			posErrorLog = strstr( contents, "\nErrorLog " );
			PrintInfo( "posErrorLog (%p)\n~", posErrorLog );
			if (posErrorLog != NULL)
			{
				posErrorLog += strlen( "\nErrorLog " );
				/* look for the end of the ErrorLog line */
				posEol = strstr( posErrorLog, "\n" );

				PrintInfo( "posErrorLog (%p); posEol (%p)\n~", posErrorLog, posEol );
				/* if end of ErrorLog line found */
				if (posEol)
				{
					posEol[0] = '\0';                      /* terminate the end of the line so that the strrchr() call works just on this line */

					posLastSlash = strrchr( posErrorLog, '/' );
					PrintInfo( "posLastSlash (%p)(%s)\n~", posLastSlash, posErrorLog );
				}
			}
			else
			{
				PrintInfo( "could not find ErrorLog line in boa.conf\n~");
			}
		}
	}

	/* if the last forward slash was found on the ErrorLog line */
	if (posErrorLog && posLastSlash)
	{
		posLastSlash[1] = '\0';
		PrintInfo( "detected temp directory in boa.conf of (%s)\n~", posErrorLog );
		strncpy( tempDirectory, posErrorLog, sizeof( tempDirectory ) -1 );
	}
	/* if the temp directory is already set to something previously */
	else if (strncmp( tempDirectory, "empty", 5 ) != 0)
	{
		/* use the previous setting */
	}
	else
	{
		strncpy( tempDirectory, "/tmp/", sizeof( tempDirectory ) -1 );
		PrintInfo( "using default temp directory of (%s)\n~", tempDirectory );
	}

	Free( contents );

	PrintInfo( "returning (%s)\n~", tempDirectory );
	return( tempDirectory );
}

int get_proc_stat_info(
		splice_proc_stat_info *pProcStatInfo
		)
{
	unsigned long int irqTotal = 0;
	FILE             *fpStats  = NULL;
	char              buf[1024];
	char             *pos = NULL;

	if (pProcStatInfo == NULL)
	{
		return( -1 );                                      /* invalid parameter */
	}

	fpStats = fopen( PROC_STAT_FILE, "r" );
	if (fpStats != NULL)
	{
		while (fgets( buf, sizeof( buf ), fpStats ))
		{
			pos = strchr( buf, '\n' );
			if (pos)
			{
				*pos = 0;                                  /* null-terminate the line */
			}
			if (strncmp( buf, "intr ", 5 ) == 0)
			{
				/* just read the first number. it contains the total number of interrupts since boot */
				sscanf( buf + 5, "%lu", &( pProcStatInfo->irqTotal ));
				PrintInfo( "irqTotal %lu\n", pProcStatInfo->irqTotal );
			}
			else if (strncmp( buf, "ctxt ", 5 ) == 0)
			{
				sscanf( buf + 5, "%lu", &( pProcStatInfo->contextSwitches ));
				PrintInfo( "contextSwitches %lu\n", pProcStatInfo->contextSwitches );
				break;                                     /* after finding context switches, do not need to look any further in the file */
			}
		}

		fclose( fpStats );
	}
	else
	{
		PrintError("Could not open %s\n", PROC_STAT_FILE );
	}
	return( irqTotal );
}  

int get_interrupt_counts(
		splice_irq_data *irqData
		)
{
	struct stat       statbuf;
	char             *contents      = NULL;
	size_t            numBytes      = 0;
	FILE             *fpText        = NULL;
	char             *posstart      = NULL;
	char             *posend        = NULL;
	unsigned long int lineNum       = 0;
	unsigned long int cpu           = 0;
	unsigned int      numProcessors = sysconf( _SC_NPROCESSORS_ONLN );
	unsigned int      numCpusActive = 0;
	unsigned int      numLines      = 0;
	char              cmd[64+TEMP_FILE_FULL_PATH_LEN];
	char              tempFilename[TEMP_FILE_FULL_PATH_LEN];

	PrependTempDirectory( tempFilename, sizeof( tempFilename ), TEMP_INTERRUPTS_FILE );

	setlocale( LC_NUMERIC, "" );

	memset( &statbuf, 0, sizeof( statbuf ));

	/* create a shell command to concatinate the cumuulative interrupt counts file to the temp directory */
	sprintf( cmd, "cat %s > %s", PROC_INTERRUPTS_FILE, tempFilename );
	system( cmd );

	if (lstat( tempFilename, &statbuf ) == -1)
	{
		PrintError( "(%s); lstat failed; %s\n", tempFilename, strerror( errno ));
		return( -1 );
	}

	/*printf("size of (%s) %lu\n", tempFilename, statbuf.st_size);*/

	if (statbuf.st_size == 0)
	{
		PrintError( "could not determine interrupts file (%s) size.\n", tempFilename );
		return( -1 );
	}

	contents = (char *) malloc( statbuf.st_size + 1 );
	if (contents == NULL)
	{
		PrintError( "could not malloc(%lu+1) bytes\n", (unsigned long int) statbuf.st_size );
		return( -1 );
	}

	memset( contents, 0, statbuf.st_size + 1 );

	fpText = fopen( tempFilename, "r" );
	if (fpText == NULL)
	{
		PrintError( "could not fopen(%s)\n", tempFilename );
		return( -1 );
	}
	numBytes = fread( contents, 1, statbuf.st_size, fpText );

	fclose( fpText );

	/* we are done with the temp file; delete it. */
	remove( tempFilename );

	if (numBytes != (unsigned int) statbuf.st_size)
	{
		PrintError( "tried to fread %lu bytes but got %lu\n", (unsigned long int) statbuf.st_size, (unsigned long int) numBytes );
		return( -1 );
	}

	posstart = contents;      /* start parsing the file at the beginning of the file */

	/* step through the file line by line, searching for interrupt counts for each CPU */
	do {
		posend = strstr( posstart, "\n" );
		if (posend)
		{
			*posend = '\0';
			posend++;
		}
		/*PrintInfo("next line (%s); posend %p\n", posstart, posend );*/

		numLines++;

		if (lineNum == 0)
		{
			char *cp, *restOfLine;
			restOfLine = posstart;

			/*PrintInfo("numProcessors %u\n", numProcessors);*/
			while (( cp = strstr( restOfLine, "CPU" )) != NULL && numCpusActive < numProcessors)
			{
				cpu = strtoul( cp + 3, &restOfLine, SPLICE_IRQ_VALUE_LENGTH );
				numCpusActive++;
			}
			/*PrintInfo("found %u cpus in header; numProcessors %u\n", numCpusActive, numProcessors );*/
		}
		else if (strlen( posstart ))
		{
			char        *cp         = NULL;
			char        *restOfLine = NULL;
			char        *pos        = NULL;
			unsigned int irqIdx     = lineNum - 1;         /* the first line has the header on it */

			if (irqIdx < SPLICE_IRQ_MAX_TYPES)
			{
				/* Skip over "IRQNAME:" */
				cp = strchr( posstart, ':' );
				if (!cp)
				{
					continue;
				}

				cp++;

				pos  = cp;
				pos += numCpusActive * ( SPLICE_IRQ_VALUE_LENGTH + 1 ); /* each number is 10 digits long separated by 1 space */
				pos += 2;                                                 /* after all of the numbers are listed, the name is separated by 2 more spaces */

				/* some names have a few spaces at the beginning of them; advance the pointer past all of the beginning spaces */
				while (*pos == ' ')
				{
					pos++;
				}

				/* the line is long enough to have a name at the end of it */
				if (pos < ( cp + strlen( cp )))
				{
					strncpy( irqData->irqDetails[irqIdx].irqName, pos, sizeof( irqData->irqDetails[irqIdx].irqName ));
					//PrintInfo( "%s: added new irq to idx %2u (%s)\n", __FUNCTION__, irqIdx, pos );
				}

				//PrintInfo( "line %3u: (%s) ", numLines, cp );
				for (cpu = 0; cpu < numCpusActive; cpu++)
				{
					unsigned long int value = 0;

					/* parse the next value from the current line */
					value = strtoul( cp, &restOfLine, SPLICE_IRQ_VALUE_LENGTH );

					/* add interrupt count to the running value for this CPU */
					irqData->irqCount[cpu] += value;

					/* save the value for this specific IRQ */
					irqData->irqDetails[irqIdx].irqCount[cpu] = value;
					//PrintInfo( "%lu ", value );

					cp = restOfLine;
					if (cp == NULL)
					{
						break;
					}
				}
				//PrintInfo( "\n" );
			}
			else
			{
				PrintError("irqIdx (%u) exceeded SPLICE_IRQ_MAX_TYPES\n", irqIdx );
			}
		}

		posstart = posend;

		lineNum++;
	} while (posstart != NULL);

	Free( contents );

	/* add up all of the cpu's interrupts into one big total */
	for (cpu = 0; cpu < numCpusActive; cpu++)
	{
		irqData->irqTotal += irqData->irqCount[cpu];
	}
	//PrintInfo( "after %u lines, total %lu\n\n\n", numLines, irqData->irqTotal );

	return( 0 );
}

/**
 *  Function: This function will prepend the specified file name with the known temporary path name.
 **/
void PrependTempDirectory(
		char       *filenamePath,
		int         filenamePathLen,
		const char *filename
		)
{
	if (filenamePath)
	{
		strncpy( filenamePath, GetTempDirectoryStr(), filenamePathLen -1 );
		strncat( filenamePath, filename,              filenamePathLen -1 );

		PrintInfo( "returning (%s)\n~", filenamePath );
	}

	return;
}

/**
 *  Function: This function returns to the caller the number of bytes contained in the specified file.
 **/
int getFileSize(
		const char *filename
		)
{
	struct stat file_stats;

	if (filename == NULL)
	{
		return( 0 );
	}

	if (stat( filename, &file_stats ) != 0)
	{
		PrintError( "<!-- ERROR getting stats for file (%s) -->\n", filename );
		return( 0 );
	}

	return( file_stats.st_size );
}

int Close(
		int socketFd
		)
{
	PrintInfo( "socket %d\n", socketFd );
	if (socketFd>0) {close( socketFd ); }
	return( 0 );
}

int send_request_read_response(
		unsigned char      *request,
		int                 request_len,
		unsigned char      *response,
		int                 response_len,
		int                 server_port,
		int                 cmd
		)
{
	int                rc = 0;
	int                sd = 0;
	int                fromlen;
	struct sockaddr_in from;
	struct in_addr sin_temp_addr;
	struct sockaddr_in    server;
	struct pollfd sock_pfd;

	sin_temp_addr.s_addr = get_my_ip4_addr();
	if ( sin_temp_addr.s_addr == 0 )
	{
		PrintHTML( "~FATAL~get_my_ip4_addr() failed to determine IP address.~" );
		return( -1 );
	}
	bcopy( &sin_temp_addr.s_addr, &( server.sin_addr ), sizeof( sin_temp_addr.s_addr ) );

	/* Construct name of socket to send to. */
	server.sin_family = AF_INET;
	server.sin_port   = htons( SPLICE_SERVER_PORT );

	PrintInfo( "reqlen %d; reslen %d; server_port %d\n", request_len, response_len, server_port );
	/*   Create socket on which to send and receive */
	sd = socket( AF_INET, SOCK_STREAM, 0 );

	if (sd<0)
	{
		PrintError( "ERROR creating socket\n");
		return( -1 );
	}

	/* Connect to server */
	PrintInfo( "connect(sock %u; port %u)\n", sd, server_port );
	if (connect( sd, (struct sockaddr *)&server, sizeof( server )) < 0)
	{
		Close( sd );
		PrintError( "ERROR connecting to server\n");
		return( -1 );
	}

	fromlen = sizeof( from );
	if (getpeername( sd, (struct sockaddr *)&from, (unsigned int *) &fromlen )<0)
	{
		Close( sd );
		PrintError( "ERROR could not get peername\n");
		return( -1 );
	}

	PrintInfo( "Connected to TCPServer1: " );
	PrintInfo( "%s:%d\n", inet_ntoa( from.sin_addr ), ntohs( from.sin_port ));

	memcpy( request, &cmd, sizeof( cmd ));

	PrintInfo( "Sending request cmd (%d) to server; sizeof(request) %u\n", cmd, request_len );
	if (send( sd, request, request_len, 0 ) < 0)
	{
		PrintError( "failed to send cmd %u to socket %u\n", cmd, sd );
		return( -1 );
	}

	PrintInfo( "Reading response from server; sizeof(response) %u\n", response_len );
	sock_pfd.fd = sd;
	sock_pfd.events = POLLIN;

	poll(&sock_pfd, 1, 1000);
	if(sock_pfd.revents & POLLIN)
	{
		rc = recv( sd, response, response_len, 0);
	}
	else
	{
		PrintInfo("poll timeout, event=0x%x\n", sock_pfd.revents);
	}

	Close( sd );
	return( 0 );
}

/**
 *  Function: This function will return the frequency for the specified CPU.
 *            You can use this command to list the current frequency:
 *            $ cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
 **/
int get_cpu_frequency(
		unsigned int cpuId
		)
{
	long int      freq = 0;
	FILE         *fp  = NULL;
	struct stat   statbuf;
	char          cpuinfo_cur_freq[64];

	memset( &statbuf, 0, sizeof( statbuf ));

	sprintf( cpuinfo_cur_freq, "/sys/devices/system/cpu/cpu%u/cpufreq/cpuinfo_cur_freq", cpuId );

	PrintInfo( "trying lstat(%s)\n", cpuinfo_cur_freq );
	if ( (lstat( cpuinfo_cur_freq, &statbuf ) == -1) || (statbuf.st_size == 0) )
	{
		PrintInfo( "lstat(%s) failed; %s\n", cpuinfo_cur_freq, strerror( errno ));
		return( freq );
	}

	fp = fopen( cpuinfo_cur_freq, "r" );
	if ( fp )
	{
		int  num_bytes = 0;
		char freq_buffer[32];
		memset(freq_buffer, 0, sizeof(freq_buffer));

		num_bytes = fread(freq_buffer, 1, sizeof(freq_buffer) - 1, fp );
		PrintInfo( "fread() returned num_bytes %d \n", num_bytes );
		if ( num_bytes )
		{
			sscanf( freq_buffer, "%ld", &freq );
			PrintInfo( "sscanf(%s) returned freq %ld \n", freq_buffer, freq );
			freq /= 1000;
		}
	}

	return ( freq );
}

/**
 *  Function: This function will loop through all processors in the system
 *  and look for the CPU frequency associated each one. If at least one
 *  non-zero frequency is found, then all of the discovered frequencies will
 *  be output using the CPUFREQUENCY tag.
 **/
int output_cpu_frequencies( void )
{
	int numCpusConf = 0;
	int cpu = 0;
	char CPUFREQUENCY_line[512];
	char cpu_freq_str[32];
	int  cpu_freq_int;
	bool cpu_freq_nonzero = false;

	memset(CPUFREQUENCY_line, 0, sizeof(CPUFREQUENCY_line));

	numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
	if (numCpusConf > SPLICE_MAX_NUM_CPUS)
	{
		numCpusConf = SPLICE_MAX_NUM_CPUS;
	}

	for (cpu = 0; cpu < numCpusConf; cpu++)
	{
		cpu_freq_int = get_cpu_frequency(cpu);

		/* if at least one non-zero cpu frequency is found, remember this fact */
		if ( cpu_freq_nonzero == false && cpu_freq_int > 0 ) cpu_freq_nonzero = true;

		sprintf( cpu_freq_str, "%06u ", cpu_freq_int );
		strncat( CPUFREQUENCY_line, cpu_freq_str, sizeof(CPUFREQUENCY_line) - strlen(CPUFREQUENCY_line) - 1 );
	}

	if ( cpu_freq_nonzero )
	{
		/* output the CPU frequency data */
		PrintHTML( "~CPUFREQUENCY~%u %s~", numCpusConf, CPUFREQUENCY_line );
	}

	return ( 0 );
}

int convert_to_string_with_commas(
		unsigned long int value,
		char             *valueStr,
		unsigned int      valueStrLen
		)
{
	unsigned long int numCommas = 0;
	unsigned long int numDigits = 0;
	char             *newString = NULL;
	unsigned long int newLen    = 0;

	numCommas = log10( value )/3;
	numDigits = trunc( log10( value ))+1;

	newLen    = numDigits + numCommas + 1 /* null terminator */;
	newString = (char *) malloc( newLen );
	if (newString)
	{
		unsigned int comma      = 0;
		unsigned int groupStart = 0;

		memset( newString, 0, newLen );

		sprintf( newString, "%lu", value );
		/*printf("%s: newString (%s); len %lu; space %lu\n", __FUNCTION__, newString, strlen(newString), newLen );*/

		/* 999999999 -> 999,999,999 */
		for (comma = 1; comma<=numCommas; comma++) {
			groupStart = numDigits - ( 3*comma );
			/*           1 ...           1111 */ /* 8,9,10 -> 11,12,13 */ /* 5,6,7 -> 7,8,9 */ /* 2,3,4 -> 3,4,5 */
			/* 01234567890 ... 01234567890123 */
			/* 11222333444 ... 11 222 333 444 */
			/*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 2 + numCommas- (comma - 1) , groupStart + 2);*/
			newString[groupStart + 2 + numCommas - ( comma - 1 )] = newString[groupStart + 2];
			/*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 1 + numCommas- (comma - 1) , groupStart + 1);*/
			newString[groupStart + 1 + numCommas - ( comma - 1 )] = newString[groupStart + 1];
			/*printf("%s: [%lu] <- [%u]\n", __FUNCTION__, groupStart + 0 + numCommas- (comma - 1) , groupStart + 0);*/
			newString[groupStart + 0 + numCommas - ( comma - 1 )] = newString[groupStart + 0];
			/*printf("%s: [%lu] <- ','\n", __FUNCTION__, groupStart - 1 + numCommas- (comma - 1)  );*/
			newString[groupStart - 1 + numCommas - ( comma - 1 )] = ',';
			/*printf("%s: comma %u; groupStart %u; (%s)\n", __FUNCTION__, comma, groupStart, newString);*/
		}

		if (valueStr && ( valueStrLen>1 ))
		{
			strncpy( valueStr, newString, valueStrLen -1 );
		}
		/*printf("%s", newString );*/

		Free( newString );
	}

	return( 0 );
}

int set_cpu_utilization(void)
{
	splice_getCpuUtilization( &g_cpuData );

	return( 0 );
}

/**
  This public function will return the CPU utilization numbers that are calculated once a second in
  a separate 1 hertz thread (dataFetchThread).
 **/
int splice_getCpuUtilization(
		splice_cpu_percent *pCpuData
		)
{
	if (pCpuData)
	{
		PrintInfo( "memcpy() %lu bytes; numActiveCpus %u\n", sizeof( *pCpuData ), g_cpuData.numActiveCpus );
		memcpy( pCpuData, &g_cpuData, sizeof( *pCpuData ));
	}

	return( 0 );
}

/**
 *  Function: This function will configure the socket so that the address can be re-used without a long timeout.
 **/
void reusePort(
		int s
		)
{
	int one = 1;

	if (setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof( one )) == -1)
	{
		PrintError( "error in setsockopt, SO_REUSEADDR(%d) (%s) \n", SO_REUSEADDR, strerror(errno) );
	}
}

int TASK_Sleep(unsigned long ultime)
{
	struct timespec delay;
	struct timespec rem;
	int rc;

	delay.tv_sec = ultime / 1000;
	delay.tv_nsec = (ultime % 1000) * 1000000;

	for (;;) {
		rc = nanosleep(&delay, &rem);
		if (0 != rc) {
			if (EINTR == errno) {
				delay = rem;
				continue;
			}
			return -1;
		}
		break;
	}

	return 0;
}

void LONG_to_PLC_BIN_ARRAY(long n, unsigned char *str)
{

	unsigned int High_int, Low_int;

	High_int = (unsigned int) ((n & 0xFFFF0000) >> 16);
	Low_int = (unsigned int) (n & 0x0000FFFF);

	*str++ = (unsigned char) (Low_int & 0x00FF);
	*str++ = (unsigned char) ((Low_int & 0xFF00) >> 8);
	*str++ = (unsigned char) (High_int & 0x00FF);
	*str++ = (unsigned char) ((High_int & 0xFF00) >> 8);

	*str = 0;
}

void PLC_BIN_to_LONG( char *SData, long  *DData)
{
	long i=0;
	unsigned int High_int, Low_int;

	Low_int = (unsigned int) SData[1] << 8;
	Low_int |= (unsigned int) SData[0];

	High_int = (unsigned int) SData[3] << 8;
	High_int |= (unsigned int) SData[2];

	i = ((long) High_int << 16);
	i |=  Low_int;

	*DData=i;
}

void PLC_BIN_to_INT( char *SData, int  *DData)
{
	int i=0;

	i =  (unsigned int)SData[1] << 8;
	i |=  SData[0];

	*DData=i;
}


