/*
 * Copyright 1994, 1995, 2000 Neil Russell.
 * (See License)
 * Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 * Copyright 2011 Comelit Group SpA,
 *                Luca Ceresoli <luca.ceresoli@comelit.it>
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "tftp.h"
#include "bootp.h"
#include <malloc.h>

//#define  TFTP_UPGRADE_DEBUG 1
#ifdef TFTP_UPGRADE_DEBUG
#define TFTP_DEBUG(fmt,args...)	printf(fmt ,##args)
#else
#define TFTP_DEBUG(fmt,args...) do {} while(0)
#endif

//#define  TFTP_UPGRADE_DEBUG2 1
#ifdef TFTP_UPGRADE_DEBUG2
#define TFTP_DEBUG2(fmt,args...)	printf(fmt ,##args)
#else
#define TFTP_DEBUG2(fmt,args...) do {} while(0)
#endif
/* Well known TFTP port # */
#define WELL_KNOWN_PORT	69
/* Millisecs to timeout for lost pkt */
#define TIMEOUT		1000UL // richard modify
#ifndef	CONFIG_NET_RETRY_COUNT
/* # of timeouts before giving up */
# define TIMEOUT_COUNT	10
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 2)
#endif
/* Number of "loading" hashes per line (for checking the image size) */
#define HASHES_PER_LINE	65

#define  NOR_ERASE_SECTOR   (64*1024)

char *flash_burn_buffer=NULL ;
char *g_config_buffer = NULL ;
char *g_config_buffer_bak = NULL ;

static ulong g_flash_burn_len = 0;
static ulong g_flash_block_cnt = 0;
uint  verison_flag = 0;
extern uint sunxi_partition_get_size_byname(const char *part_name);
extern uint sunxi_partition_get_offset_byname(const char *part_name);
extern int spinor_net_datafinish(uint sector_start, uint total_write_bytes, void *buf);
extern int spinor_erase_one_block(uint index);


/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

static ulong TftpTimeoutMSecs = TIMEOUT;
static int TftpTimeoutCountMax = TIMEOUT_COUNT;

/*
 * These globals govern the timeout behavior when attempting a connection to a
 * TFTP server. TftpRRQTimeoutMSecs specifies the number of milliseconds to
 * wait for the server to respond to initial connection. Second global,
 * TftpRRQTimeoutCountMax, gives the number of such connection retries.
 * TftpRRQTimeoutCountMax must be non-negative and TftpRRQTimeoutMSecs must be
 * positive. The globals are meant to be set (and restored) by code needing
 * non-standard timeout behavior when initiating a TFTP transfer.
 */
ulong TftpRRQTimeoutMSecs = TIMEOUT;
int TftpRRQTimeoutCountMax = TIMEOUT_COUNT;

enum {
	TFTP_ERR_UNDEFINED           = 0,
	TFTP_ERR_FILE_NOT_FOUND      = 1,
	TFTP_ERR_ACCESS_DENIED       = 2,
	TFTP_ERR_DISK_FULL           = 3,
	TFTP_ERR_UNEXPECTED_OPCODE   = 4,
	TFTP_ERR_UNKNOWN_TRANSFER_ID  = 5,
	TFTP_ERR_FILE_ALREADY_EXISTS = 6,
};

static IPaddr_t TftpRemoteIP;
/* The UDP port at their end */
static int	TftpRemotePort;
/* The UDP port at our end */
static int	TftpOurPort;
static int	TftpTimeoutCount;
/* packet sequence number */
static ulong	TftpBlock;
/* last packet sequence number received */
static ulong	TftpLastBlock;
/* count of sequence number wraparounds */
static ulong	TftpBlockWrap;
/* memory offset due to wrapping */
static ulong	TftpBlockWrapOffset;
static int	TftpState;
#ifdef CONFIG_TFTP_TSIZE
/* The file size reported by the server */
static int	TftpTsize;
/* The number of hashes we printed */
static short	TftpNumchars;
#endif

#define STATE_SEND_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4
#define STATE_OACK	5
#define STATE_RECV_WRQ	6

/* default TFTP block size */
#define TFTP_BLOCK_SIZE		512
/* sequence number is 16 bit */
#define TFTP_SEQUENCE_SIZE	((ulong)(1<<16))

#define DEFAULT_NAME_LEN	(8 + 4 + 1)
static char default_filename[DEFAULT_NAME_LEN];

#ifndef CONFIG_TFTP_FILE_NAME_MAX_LEN
#define MAX_LEN 128
#else
#define MAX_LEN CONFIG_TFTP_FILE_NAME_MAX_LEN
#endif

static char tftp_filename[MAX_LEN];

#ifdef CONFIG_SYS_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[];
#endif

/* 512 is poor choice for ethernet, MTU is typically 1500.
 * Minus eth.hdrs thats 1468.  Can get 2x better throughput with
 * almost-MTU block sizes.  At least try... fall back to 512 if need be.
 * (but those using CONFIG_IP_DEFRAG may want to set a larger block in cfg file)
 */
#ifdef CONFIG_TFTP_BLOCKSIZE
#define TFTP_MTU_BLOCKSIZE CONFIG_TFTP_BLOCKSIZE
#else
//#define TFTP_MTU_BLOCKSIZE 1468  //for richard debug
#define TFTP_MTU_BLOCKSIZE 512 

#endif

static unsigned short TftpBlkSize = TFTP_BLOCK_SIZE;
static unsigned short TftpBlkSizeOption = TFTP_MTU_BLOCKSIZE;

#ifdef CONFIG_MCAST_TFTP
#include <malloc.h>
#define MTFTP_BITMAPSIZE	0x1000
static unsigned *Bitmap;
static int PrevBitmapHole, Mapsize = MTFTP_BITMAPSIZE;
static uchar ProhibitMcast, MasterClient;
static uchar Multicast;
extern IPaddr_t Mcast_addr;
static int Mcast_port;
static ulong TftpEndingBlock; /* can get 'last' block before done..*/

static void parse_multicast_oack(char *pkt, int len);

static void
mcast_cleanup(void)
{
	if (Mcast_addr)
		eth_mcast_join(Mcast_addr, 0);
	if (Bitmap)
		free(Bitmap);
	Bitmap = NULL;
	Mcast_addr = Multicast = Mcast_port = 0;
	TftpEndingBlock = -1;
}

#endif	/* CONFIG_MCAST_TFTP */


/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void * tftp_memcpy(void *dest, const void *src, size_t count)
{
	char *d8, *s8;
    if((dest==NULL) ||(src==NULL)){
		TFTP_DEBUG("tftp_memcpy arg null \n");
		return NULL;
	}
	if (src == dest)
		return dest;
    /* copy the reset one byte at a time */
	d8 = (char *)dest;
	s8 = (char *)src;
	while (count--)
		*d8++ = *s8++;

	return dest;
}

static __inline__ void
store_block(unsigned block, uchar *src, unsigned len)
{
	ulong offset = block * TftpBlkSize + TftpBlockWrapOffset;
	ulong newsize = offset + len;
    static ulong block_offset = 0;
    uint start_block = 0;
    int i = 0;

	uint index_cnt   = 0;	
	uint net_addr_start= 0;
	uint flash_burn_length = 0;
	uint boot_size = 0;
	uint index_begin = 0 ;
	
#ifdef CONFIG_SYS_DIRECT_FLASH_TFTP
	int i, rc = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		/* start address in flash? */
		if (flash_info[i].flash_id == FLASH_UNKNOWN)
			continue;
		if (load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { /* Flash is destination for this packet */
		rc = flash_write((char *)src, (ulong)(load_addr+offset), len);
		if (rc) {
			flash_perror(rc);
			NetState = NETLOOP_FAIL;
			return;
		}
	}
	else
#endif /* CONFIG_SYS_DIRECT_FLASH_TFTP */
	{
        g_flash_burn_len += len ;
     	TFTP_DEBUG("\nstorebuf[0x%x],len=[%d],block[%d]\n",flash_burn_buffer,src,len,block);
		(void)tftp_memcpy((uchar *)(flash_burn_buffer+block_offset), src, len);
	    block_offset += 512;
	    if ( len< 512) {
			 NetState =  NETLOOP_SUCCESS ;
			 if(strncmp(tftp_filename,"roo",3)==0 || strncmp(tftp_filename,"boo",3)==0  ) {
                  if(len < 512) {

                    block_offset = 0;
					net_addr_start= 0;

                    if ( strncmp(tftp_filename,"roo",3)==0 ) {
						
                        index_cnt   = sunxi_partition_get_size_byname("system")*512/(NOR_ERASE_SECTOR);
						start_block = sunxi_partition_get_offset_byname("system");
						boot_size = sunxi_partition_get_size_byname("boot")*512;
					    index_begin= (256*1024+boot_size)/(64*1024) ;
						
                    } else {
                    
                    	start_block = sunxi_partition_get_offset_byname("boot");
						index_cnt = sunxi_partition_get_size_byname("boot")*512/(NOR_ERASE_SECTOR); 
						index_begin = (256*1024)/(64*1024) ;
					}
					
                    net_addr_start  = start_block+((256 - 16) * 1024/512) ;
					TFTP_DEBUG("index_begin =%d,index_cnt=%d,g_flash_burn_len=%d\n",index_begin,index_cnt,g_flash_burn_len);

					for ( i = index_begin;i< (index_begin + index_cnt); i++) {
						spinor_erase_one_block(i);
					}

					flash_burn_length = g_flash_burn_len+511 ;
                    spinor_net_datafinish(net_addr_start,flash_burn_length,flash_burn_buffer);
                    g_flash_burn_len = 0;
				    if (flash_burn_buffer !=NULL) {
						free(flash_burn_buffer);
                        flash_burn_buffer=NULL;
                    }
					
		    }

			}
			
			else if(strncmp(tftp_filename,"con",3)==0) {

				g_config_buffer = (char *)malloc(512);
				g_config_buffer_bak = (char *)malloc(512);
				(void)tftp_memcpy((char *)(g_config_buffer), src, len);
				(void)tftp_memcpy((char *)(g_config_buffer_bak), src, len);
				printf(" g_config_buffer = %s\n",g_config_buffer);
				g_config_down_load = 1 ;
                g_flash_burn_len = 0;
				block_offset = 0 ;
				if (flash_burn_buffer !=NULL){
					free(flash_burn_buffer);
					flash_burn_buffer = NULL;
			    }

		  }
		

	    }
     }
#ifdef CONFIG_MCAST_TFTP
	if (Multicast)
		ext2_set_bit(block, Bitmap);
#endif

	if (NetBootFileXferSize < newsize)
		NetBootFileXferSize = newsize;
}

static void TftpSend(void);
static void TftpTimeout(void);

/**********************************************************************/

static void
TftpSend(void)
{
	volatile uchar *pkt;
	volatile uchar *xp;
	int		len = 0;
	volatile ushort *s;

#ifdef CONFIG_MCAST_TFTP
	/* Multicast TFTP.. non-MasterClients do not ACK data. */
	if (Multicast
	 && (TftpState == STATE_DATA)
	 && (MasterClient == 0))
		return;
#endif
	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;

	switch (TftpState) {

	case STATE_SEND_RRQ:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_RRQ);
		pkt = (uchar *)s;
		strcpy((char *)pkt, tftp_filename);
		pkt += strlen(tftp_filename) + 1;
		strcpy((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		strcpy((char *)pkt, "timeout");
		pkt += 7 /*strlen("timeout")*/ + 1;
		sprintf((char *)pkt, "%lu", TftpTimeoutMSecs / 1000);
		debug("send option \"timeout %s\"\n", (char *)pkt);
		TFTP_DEBUG2("send option \"timeout %s\"\n", (char *)pkt);
		pkt += strlen((char *)pkt) + 1;
#ifdef CONFIG_TFTP_TSIZE
		memcpy((char *)pkt, "tsize\0000\0", 8);
		pkt += 8;
#endif
		/* try for more effic. blk size */
		pkt += sprintf((char *)pkt, "blksize%c%d%c",
				0, TftpBlkSizeOption, 0);
#ifdef CONFIG_MCAST_TFTP
		/* Check all preconditions before even trying the option */
		if (!ProhibitMcast
		 && (Bitmap = malloc(Mapsize))
		 && eth_get_dev()->mcast) {
			free(Bitmap);
			Bitmap = NULL;
			pkt += sprintf((char *)pkt, "multicast%c%c", 0, 0);
		}
#endif /* CONFIG_MCAST_TFTP */
		len = pkt - xp;
		break;

	case STATE_OACK:
#ifdef CONFIG_MCAST_TFTP
		/* My turn!  Start at where I need blocks I missed.*/
		if (Multicast)
			TftpBlock = ext2_find_next_zero_bit(Bitmap,
							    (Mapsize*8), 0);
		/*..falling..*/
#endif

	case STATE_RECV_WRQ:
	case STATE_DATA:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ACK);
		*s++ = htons(TftpBlock);
		pkt = (uchar *)s;
		len = pkt - xp;
		break;

	case STATE_TOO_LARGE:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(3);
		pkt = (uchar *)s;
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_BAD_MAGIC:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (uchar *)s;
		strcpy((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}

	NetSendUDPPacket(NetServerEther, TftpRemoteIP, TftpRemotePort,
			 TftpOurPort, len);
}


static void
TftpHandler(uchar *pkt, unsigned dest, IPaddr_t sip, unsigned src,
	    unsigned len)
{
	ushort proto;
	ushort *s;
	int i;

	if (dest != TftpOurPort) {
#ifdef CONFIG_MCAST_TFTP
		if (Multicast
		 && (!Mcast_port || (dest != Mcast_port)))
#endif
			return;
	}
	if (TftpState != STATE_SEND_RRQ && src != TftpRemotePort &&
	    TftpState != STATE_RECV_WRQ)
		return;

	if (len < 2)
		return;
	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	s = (ushort *)pkt;
	proto = *s++;
	pkt = (uchar *)s;
	switch (ntohs(proto)) {

	case TFTP_RRQ:
	case TFTP_ACK:
		break;
	default:
		break;

#ifdef CONFIG_CMD_TFTPSRV
	case TFTP_WRQ:
		debug("Got WRQ\n");
		TftpRemoteIP = sip;
		TftpRemotePort = src;
		TftpOurPort = 1024 + (get_timer(0) % 3072);
		TftpLastBlock = 0;
		TftpBlockWrap = 0;
		TftpBlockWrapOffset = 0;
		TftpSend(); /* Send ACK(0) */
		break;
#endif

	case TFTP_OACK:
		debug("Got OACK: %s %s\n",
			pkt,
			pkt + strlen((char *)pkt) + 1);
        TFTP_DEBUG2("Got OACK: %s %s\n",
			pkt,
			pkt + strlen((char *)pkt) + 1);
		
		TftpState = STATE_OACK;
		TftpRemotePort = src;
		/*
		 * Check for 'blksize' option.
		 * Careful: "i" is signed, "len" is unsigned, thus
		 * something like "len-8" may give a *huge* number
		 */
		for (i = 0; i+8 < len; i++) {
			if (strcmp((char *)pkt+i, "blksize") == 0) {
				TftpBlkSize = (unsigned short)
					simple_strtoul((char *)pkt+i+8, NULL,
						       10);
				debug("Blocksize ack: %s, %d\n",
					(char *)pkt+i+8, TftpBlkSize);
			}
#ifdef CONFIG_TFTP_TSIZE
			if (strcmp((char *)pkt+i, "tsize") == 0) {
				TftpTsize = simple_strtoul((char *)pkt+i+6,
							   NULL, 10);
				debug("size = %s, %d\n",
					 (char *)pkt+i+6, TftpTsize);
			}
#endif
		}
#ifdef CONFIG_MCAST_TFTP
		parse_multicast_oack((char *)pkt, len-1);
		if ((Multicast) && (!MasterClient))
			TftpState = STATE_DATA;	/* passive.. */
		else
#endif
		TftpSend(); /* Send ACK */
		break;
	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);

		/*
		 * RFC1350 specifies that the first data packet will
		 * have sequence number 1. If we receive a sequence
		 * number of 0 this means that there was a wrap
		 * around of the (16 bit) counter.
		 */
		if (TftpBlock == 0) {
			TftpBlockWrap++;
			TftpBlockWrapOffset +=
				TftpBlkSize * TFTP_SEQUENCE_SIZE;
			printf("\n\t %lu MB received\n\t ",
				TftpBlockWrapOffset>>20);
		}
#ifdef CONFIG_TFTP_TSIZE
		else if (TftpTsize) {
			while (TftpNumchars <
			       NetBootFileXferSize * 50 / TftpTsize) {
				putc('#');
				TftpNumchars++;
			}
		}
#endif
		else {
			if (((TftpBlock - 1) % 10) == 0)
				putc('#');
			else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0)
				puts("\n\t ");
		}

		if (TftpState == STATE_SEND_RRQ)
		{
			debug("Server did not acknowledge timeout option!\n");
			TFTP_DEBUG2("Server did not acknowledge timeout option!\n");
		}

		if (TftpState == STATE_SEND_RRQ || TftpState == STATE_OACK ||
		    TftpState == STATE_RECV_WRQ) {
			/* first block received */
			TftpState = STATE_DATA;
			TftpRemotePort = src;
			TftpLastBlock = 0;
			TftpBlockWrap = 0;
			TftpBlockWrapOffset = 0;

#ifdef CONFIG_MCAST_TFTP
			if (Multicast) { /* start!=1 common if mcast */
				TftpLastBlock = TftpBlock - 1;
			} else
#endif
			if (TftpBlock != 1) {	/* Assertion */
				printf("\nTFTP error: "
				       "First block is not block 1 (%ld)\n"
				       "Starting again\n\n",
					TftpBlock);
				NetStartAgain();
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		TftpTimeoutCountMax = TIMEOUT_COUNT;
		NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);

		store_block(TftpBlock - 1, pkt + 2, len);

		/*
		 *	Acknowledge the block just received, which will prompt
		 *	the remote for the next one.
		 */
#ifdef CONFIG_MCAST_TFTP
		/* if I am the MasterClient, actively calculate what my next
		 * needed block is; else I'm passive; not ACKING
		 */
		if (Multicast) {
			if (len < TftpBlkSize)  {
				TftpEndingBlock = TftpBlock;
			} else if (MasterClient) {
				TftpBlock = PrevBitmapHole =
					ext2_find_next_zero_bit(
						Bitmap,
						(Mapsize*8),
						PrevBitmapHole);
				if (TftpBlock > ((Mapsize*8) - 1)) {
					printf("tftpfile too big\n");
					/* try to double it and retry */
					Mapsize <<= 1;
					mcast_cleanup();
					NetStartAgain();
					return;
				}
				TftpLastBlock = TftpBlock;
			}
		}
#endif
		TftpSend();

#ifdef CONFIG_MCAST_TFTP
		if (Multicast) {
			if (MasterClient && (TftpBlock >= TftpEndingBlock)) {
				puts("\nMulticast tftp done\n");
				mcast_cleanup();
				NetState = NETLOOP_SUCCESS;
			}
		}
		else
#endif
		if (len < TftpBlkSize) {
			/*
			 *	We received the whole thing.  Try to
			 *	run it.
			 */
#ifdef CONFIG_TFTP_TSIZE
			/* Print hash marks for the last packet received */
			while (TftpTsize && TftpNumchars < 49) {
				putc('#');
				TftpNumchars++;
			}
#endif
			puts("\ndone\n");
			NetState = NETLOOP_SUCCESS;
		}
		break;

	case TFTP_ERROR:
		printf("\nTFTP error: '%s' (%d)\n",
		       pkt + 2, ntohs(*(ushort *)pkt));

		switch (ntohs(*(ushort *)pkt)) {
		case TFTP_ERR_FILE_NOT_FOUND:
		case TFTP_ERR_ACCESS_DENIED:
			puts("Not retrying...\n");
			eth_halt();
			NetState = NETLOOP_FAIL;
			break;
		case TFTP_ERR_UNDEFINED:
		case TFTP_ERR_DISK_FULL:
		case TFTP_ERR_UNEXPECTED_OPCODE:
		case TFTP_ERR_UNKNOWN_TRANSFER_ID:
		case TFTP_ERR_FILE_ALREADY_EXISTS:
		default:
			puts("Starting again\n\n");
#ifdef CONFIG_MCAST_TFTP
			mcast_cleanup();
#endif
			NetStartAgain();
			break;
		}
		break;
	}
}


static void
TftpTimeout(void)
{
	if (++TftpTimeoutCount > TftpTimeoutCountMax) {
		puts("\nRetry count exceeded; starting again\n");
#ifdef CONFIG_MCAST_TFTP
		mcast_cleanup();
#endif
		NetStartAgain();
	} else {
		puts("T ");
		NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);
		if (TftpState != STATE_RECV_WRQ)
			printf("TftpTime\n");
			TftpSend();
	}
}


void
TftpStart(void)
{
	char *ep;             /* Environment pointer */

   // TFTP_DEBUG(" TftpStart begin   \n");
    if( flash_burn_buffer != NULL)
    {
		free( flash_burn_buffer );
	}
	flash_burn_buffer = (char *)malloc(7*1024*1024);
	if ( flash_burn_buffer == NULL ){
	    TFTP_DEBUG("TftpStart malloc err \n");
		return ;
     }

     g_flash_burn_len = 0;
	 g_flash_block_cnt = 0;
	 verison_flag = 0;

	/*
	 * Allow the user to choose TFTP blocksize and timeout.
	 * TFTP protocol has a minimal timeout of 1 second.
	 */
	ep = getenv("tftpblocksize");
	if (ep != NULL)
		TftpBlkSizeOption = simple_strtol(ep, NULL, 10);

	ep = getenv("tftptimeout");
	if (ep != NULL)
		TftpTimeoutMSecs = simple_strtol(ep, NULL, 10);

	if (TftpTimeoutMSecs < 1000) {
		debug("TFTP timeout (%ld ms) too low, "
			"set minimum = 1000 ms\n",
			TftpTimeoutMSecs);
		TftpTimeoutMSecs = 1000;
	}

	debug("TFTP blocksize = %i, timeout = %ld ms\n",
		TftpBlkSizeOption, TftpTimeoutMSecs);
	TFTP_DEBUG2("TFTP blocksize = %i, timeout = %ld ms\n",
		TftpBlkSizeOption, TftpTimeoutMSecs);

	TftpRemoteIP = NetServerIP;
	//printf(" NetServerIP = 0x%x\n",TftpRemoteIP);
    TftpRemoteIP = g_NetServerIP;   // add by richard debug 
//	printf(" TftpRemoteIP = 0x%x\n",TftpRemoteIP);
	if (BootFile[0] == '\0') {
		sprintf(default_filename, "%02lX%02lX%02lX%02lX.img",
			NetOurIP & 0xFF,
			(NetOurIP >>  8) & 0xFF,
			(NetOurIP >> 16) & 0xFF,
			(NetOurIP >> 24) & 0xFF);

		strncpy(tftp_filename, default_filename, MAX_LEN);
		tftp_filename[MAX_LEN-1] = 0;

		printf("*** Warning: no boot file name; using '%s'\n",
			tftp_filename);
	} else {
		char *p = strchr(BootFile, ':');

		if (p == NULL) {
			strncpy(tftp_filename, BootFile, MAX_LEN);
			tftp_filename[MAX_LEN-1] = 0;
		} else {
			TftpRemoteIP = string_to_ip(BootFile);
			strncpy(tftp_filename, p + 1, MAX_LEN);
			tftp_filename[MAX_LEN-1] = 0;
		}
	}

#if defined(CONFIG_NET_MULTI)
	TFTP_DEBUG("Using %s device\n", eth_get_name());
#endif
	printf("TFTP from server %pI4"
		"; our IP address is %pI4", &TftpRemoteIP, &NetOurIP);

	/* Check if we need to send across this subnet */
	if (NetOurGatewayIP && NetOurSubnetMask) {
		IPaddr_t OurNet	= NetOurIP    & NetOurSubnetMask;
		IPaddr_t RemoteNet	= TftpRemoteIP & NetOurSubnetMask;

		if (OurNet != RemoteNet)
			printf("; sending through gateway %pI4",
			       &NetOurGatewayIP);
	}
	putc('\n');

	printf("Filename '%s'.", tftp_filename);

	if (NetBootFileSize) {
		printf(" Size is 0x%x Bytes = ", NetBootFileSize<<9);
		print_size(NetBootFileSize<<9, "");
	}

	putc('\n');

	printf("Load address: 0x%lx\n", load_addr);
#if 0
	puts("Loading: *\b");
#endif

	TftpTimeoutCountMax = TftpRRQTimeoutCountMax;

	NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);
	NetSetHandler(TftpHandler);

	TftpRemotePort = WELL_KNOWN_PORT;
	TftpTimeoutCount = 0;
	TftpState = STATE_SEND_RRQ;
	/* Use a pseudo-random port unless a specific port is set */
	TftpOurPort = 1024 + (get_timer(0) % 3072);

#ifdef CONFIG_TFTP_PORT
	ep = getenv("tftpdstp");
	if (ep != NULL)
		TftpRemotePort = simple_strtol(ep, NULL, 10);
	ep = getenv("tftpsrcp");
	if (ep != NULL)
		TftpOurPort = simple_strtol(ep, NULL, 10);
#endif
	TftpBlock = 0;

	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
	/* Revert TftpBlkSize to dflt */
	TftpBlkSize = TFTP_BLOCK_SIZE;
#ifdef CONFIG_MCAST_TFTP
	mcast_cleanup();
#endif
#ifdef CONFIG_TFTP_TSIZE
	TftpTsize = 0;
	TftpNumchars = 0;
#endif

	TftpSend();
}

#ifdef CONFIG_CMD_TFTPSRV
void
TftpStartServer(void)
{
	tftp_filename[0] = 0;

#if defined(CONFIG_NET_MULTI)
	printf("Using %s device\n", eth_get_name());
#endif
	printf("Listening for TFTP transfer on %pI4\n", &NetOurIP);
	printf("Load address: 0x%lx\n", load_addr);

	puts("Loading: *\b");

	TftpTimeoutCountMax = TIMEOUT_COUNT;
	TftpTimeoutCount = 0;
	TftpTimeoutMSecs = TIMEOUT;
	NetSetTimeout(TftpTimeoutMSecs, TftpTimeout);

	/* Revert TftpBlkSize to dflt */
	TftpBlkSize = TFTP_BLOCK_SIZE;
	TftpBlock = 0;
	TftpOurPort = WELL_KNOWN_PORT;

#ifdef CONFIG_TFTP_TSIZE
	TftpTsize = 0;
	TftpNumchars = 0;
#endif

	TftpState = STATE_RECV_WRQ;
	NetSetHandler(TftpHandler);
}
#endif /* CONFIG_CMD_TFTPSRV */

#ifdef CONFIG_MCAST_TFTP
/* Credits: atftp project.
 */

/* pick up BcastAddr, Port, and whether I am [now] the master-client. *
 * Frame:
 *    +-------+-----------+---+-------~~-------+---+
 *    |  opc  | multicast | 0 | addr, port, mc | 0 |
 *    +-------+-----------+---+-------~~-------+---+
 * The multicast addr/port becomes what I listen to, and if 'mc' is '1' then
 * I am the new master-client so must send ACKs to DataBlocks.  If I am not
 * master-client, I'm a passive client, gathering what DataBlocks I may and
 * making note of which ones I got in my bitmask.
 * In theory, I never go from master->passive..
 * .. this comes in with pkt already pointing just past opc
 */
static void parse_multicast_oack(char *pkt, int len)
{
	int i;
	IPaddr_t addr;
	char *mc_adr, *port,  *mc;

	mc_adr = port = mc = NULL;
	/* march along looking for 'multicast\0', which has to start at least
	 * 14 bytes back from the end.
	 */
	for (i = 0; i < len-14; i++)
		if (strcmp(pkt+i, "multicast") == 0)
			break;
	if (i >= (len-14)) /* non-Multicast OACK, ign. */
		return;

	i += 10; /* strlen multicast */
	mc_adr = pkt+i;
	for (; i < len; i++) {
		if (*(pkt+i) == ',') {
			*(pkt+i) = '\0';
			if (port) {
				mc = pkt+i+1;
				break;
			} else {
				port = pkt+i+1;
			}
		}
	}
	if (!port || !mc_adr || !mc)
		return;
	if (Multicast && MasterClient) {
		printf("I got a OACK as master Client, WRONG!\n");
		return;
	}
	/* ..I now accept packets destined for this MCAST addr, port */
	if (!Multicast) {
		if (Bitmap) {
			printf("Internal failure! no mcast.\n");
			free(Bitmap);
			Bitmap = NULL;
			ProhibitMcast = 1;
			return ;
		}
		/* I malloc instead of pre-declare; so that if the file ends
		 * up being too big for this bitmap I can retry
		 */
		Bitmap = malloc(Mapsize);
		if (!Bitmap) {
			printf("No Bitmap, no multicast. Sorry.\n");
			ProhibitMcast = 1;
			return;
		}
		memset(Bitmap, 0, Mapsize);
		PrevBitmapHole = 0;
		Multicast = 1;
	}
	addr = string_to_ip(mc_adr);
	if (Mcast_addr != addr) {
		if (Mcast_addr)
			eth_mcast_join(Mcast_addr, 0);
		Mcast_addr = addr;
		if (eth_mcast_join(Mcast_addr, 1)) {
			printf("Fail to set mcast, revert to TFTP\n");
			ProhibitMcast = 1;
			mcast_cleanup();
			NetStartAgain();
		}
	}
	MasterClient = (unsigned char)simple_strtoul((char *)mc, NULL, 10);
	Mcast_port = (unsigned short)simple_strtoul(port, NULL, 10);
	printf("Multicast: %s:%d [%d]\n", mc_adr, Mcast_port, MasterClient);
	return;
}

#endif /* Multicast TFTP */
