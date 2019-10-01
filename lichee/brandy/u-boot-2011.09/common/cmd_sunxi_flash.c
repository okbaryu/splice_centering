/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Ported 'dynenv' to 'nand env.oob' command
 * (C) 2010 Nanometrics, Inc.
 * 'dynenv' -- Dynamic environment offset in NAND OOB
 * (C) Copyright 2006-2007 OpenMoko, Inc.
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 *
 * Copyright 2010 Freescale Semiconductor
 * The portions of this file whose copyright is held by Freescale and which
 * are not considered a derived work of GPL v2-only code may be distributed
 * and/or modified under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <fastboot.h>
#include <sys_partition.h>
#include <net.h>

//#define  NOR_FLASH_DEBUG  1
#ifdef NOR_FLASH_DEBUG
#define FLASH_DEBUG(fmt,args...)	printf(fmt ,##args)
#else
#define FLASH_DEBUG(fmt,args...) do {} while(0)
#endif

#ifdef NET_UPGRADE
extern void *malloc (size_t len);
extern void free(void*);
extern int spinor_erase_one_block(uint index);
extern int spinor_net_datafinish(uint sector_start, uint total_write_bytes, void *buf);
extern int spinor_read_nocache(uint start, uint nblock, void *buffer);
extern int tftp_netboot_common (proto_t proto, cmd_tbl_t *cmdtp, int argc, char * const arg1,char *const arg2);
#endif

#define  SUNXI_FLASH_READ_FIRST_SIZE      (32 * 1024)
#define  UID_OFFSET                       (24)

#ifdef CONFIG_SUNXI_GETH
#ifdef NET_UPGRADE
enum set {
	BOOT = 1,
	ROOTFS ,
	CONFIG ,
};

static int net_upgrade_reset = 0;
uchar is_update(int type)
{
	char *config = NULL;
//	char s_config_netupdate_dat[512]="mac=10:10:10:10:10:10;uid=12345678;ver=1.00.1000.1000;";
    uint  start_private ;
	config = (char *)malloc(512);
    char *verflash;
    char *verbuf;

	start_private= sunxi_partition_get_offset_byname("private");
	FLASH_DEBUG("===enter update func\n");
    
    spinor_read_nocache(start_private+((256 - 16) * 1024/512),1,(void *)config);

    if((config !=NULL)&&(config[0]=='m'))
	{
			FLASH_DEBUG(" is_update:from flash =%s\n",config);
	}
	
	if (config ==NULL)
	{
		FLASH_DEBUG(" malloc fail update warning \n");
		return 0 ;  // not need upgrade 
	}

	if ( type == BOOT )
	{
		if ( g_config_buffer == NULL)
		{
				return 0;
		}
        verflash    = strstr(config, "ver=");
	    verbuf      = strstr(g_config_buffer, "ver=");

	    if((verflash!=NULL)&&(verbuf!=NULL))    // compare boot 
		{
			if (strncmp(verflash,verbuf,13)==0)
			{
				
			    FLASH_DEBUG("boot  is same \n");
            	free( config ); 
				return 0;  // no need  upgrade
			}
				
	    } 
		else
		{
				free( config );
				config = NULL;
        }
    	 
		 FLASH_DEBUG("boot  not same \n");
		 
     
     }
	 else if (type == ROOTFS)
	 {
			if ( g_config_buffer_bak == NULL)
			{
				return 0;
			}
			verflash    = strstr(config, "ver=");
	        verbuf      = strstr(g_config_buffer_bak, "ver=");
			
            if((verflash!=NULL)&&(verbuf!=NULL)) 
            {
				if(strncmp(verflash+14,verbuf+14,4)==0) 
            	{
					FLASH_DEBUG("bootfs  is same \n") ;
            		free( config );
					config = NULL;
					return 0;    // no need  upgrade
				}
            }
			else
	        {
	            free( config );
				config = NULL;
            }
		
			FLASH_DEBUG("bootfs not same \n") ;
		
	}
	 
	return 1 ;  // need upgrade 
}

void update_version(int type)
{
	char *config = NULL;
	//	char s_config_netupdate_dat[512]="mac=10:10:10:10:10:10;uid=12345678;ver=1.00.1000.1000;";
	uint  start_private ;
	u32 net_addr_start ;
	u32 index_begin ;
	config = (char *)malloc(512);
    char mac_tmp[]="mac=00:00:00:00:00:00";
	char uid_tmp[38]="uid=00000000000000000000000000000000";
	char uid_len = 0;
    char * uidflash;
	char *verflash;
    char *verbuf;
	
	start_private= sunxi_partition_get_offset_byname("private");

    spinor_read_nocache(start_private+((256 - 16) * 1024/512),1,config);

    if(config ==NULL)
	{
		FLASH_DEBUG("malloc fail\n");
		return ;
	}
	
    if((config[0]=='m'))
	{
		FLASH_DEBUG(" before flash update_version =%s\n",config);
	}
   
	if (type == BOOT) 
	{
		if ( g_config_buffer == NULL)
		{
				free(config);
				config =NULL;
				return ;
		}

		verflash	=   strstr(config, "ver=");
		verbuf      =   strstr(g_config_buffer, "ver=");
		if((verflash !=NULL)&&((verbuf !=NULL)))
		{
			strncpy(verbuf+14,verflash+14,4);  // bak rootfs ver data from flash

		}
		if(((verbuf !=NULL))&&(verflash ==NULL))
		{
			strncpy(verbuf+14,"0000",4);
		}
		if( strncmp(&g_config_buffer[0],&mac_tmp[0],21 ) ==0)
        {  
            if(config[0] == 'm')
			strncpy(&g_config_buffer[0],&config[0],21);
			FLASH_DEBUG("config not burn mac \n");
		}

		if( strncmp(&g_config_buffer[22],&uid_tmp[0],36 ) ==0)
		{
		    uidflash = strstr(config, "uid=");
			verflash = strstr(config, "ver=");
			verbuf   =  strstr(g_config_buffer, "ver=");
			if(uidflash&&verflash&&verbuf)
			{
				uid_len = verflash - uidflash;
				strncpy(&g_config_buffer[22],&config[22],uid_len);
				strncpy(&g_config_buffer[22+uid_len],verbuf,18);
			}
			
			FLASH_DEBUG("config not burn uid \n");
		}

//        start_private = sunxi_partition_get_offset_byname("private");
		u32 net_addr_start  = start_private+((256 - 16) * 1024/512) ;
        u32 index_begin= (256*1024+(15*1024*1024/2)+128*1024)/(64*1024) ;

		index_begin =( start_private * 512+(256 - 16) * 1024 )/(64*1024);
        spinor_erase_one_block(index_begin);
		
	    spinor_net_datafinish(net_addr_start,512,&g_config_buffer[0]);
		
        spinor_read_nocache(start_private+((256 - 16) * 1024/512),1,config);
        
    	FLASH_DEBUG(" update_version:from flash :%s\n",config);

		if( g_config_buffer != NULL)
		{
			 free(g_config_buffer);
			 g_config_buffer = NULL ;
		}
	}
	else if ( type == ROOTFS)   // rootfs 
	{
		if ( g_config_buffer_bak == NULL)
		{
				free(config);
				config = NULL;
				return ;
		}

		verflash	= strstr(config, "ver=");
		verbuf      = strstr(g_config_buffer_bak, "ver=");

       if((verflash !=NULL)&&((verbuf !=NULL)))
       {
			strncpy(verbuf+9,verflash+9,4); // bak boot ver data from flash
       }
		
        if( strncmp(&g_config_buffer_bak[0],&mac_tmp[0],21 ) ==0)
        {
            if(config[0] == 'm')
				strncpy(&g_config_buffer_bak[0],&config[0],21);
			FLASH_DEBUG(" config not burn mac \n");
		}
		
		if( strncmp(&g_config_buffer_bak[22],&uid_tmp[0],36 ) ==0)
		{

			uidflash= strstr(config, "uid=");
			verflash	= strstr(config, "ver=");
			verbuf =  strstr(g_config_buffer_bak, "ver=");
			if(uidflash&&verflash&&verbuf)
			{
				uid_len = verflash - uidflash;
				strncpy(&g_config_buffer_bak[22],&config[22],uid_len);
				strncpy(&g_config_buffer_bak[22+uid_len],verbuf,18);
			}
			FLASH_DEBUG(" config not burn uid \n");
		}

//          start_private = sunxi_partition_get_offset_byname("private");
		net_addr_start  = start_private+((256 - 16) * 1024/512) ;
//		index_begin= (256*1024+(15*1024*1024/2)+128*1024)/(64*1024) ;
        index_begin =( start_private * 512+(256 - 16) * 1024 )/(64*1024);
        spinor_erase_one_block(index_begin);
		spinor_net_datafinish(net_addr_start,512,&g_config_buffer_bak[0]);
		spinor_read_nocache(start_private+((256 - 16) * 1024/512),1,config);
        FLASH_DEBUG(" step 1update_version config =%s\n",config);

        if(g_config_buffer_bak !=NULL) 
		{
			free(g_config_buffer_bak);
			g_config_buffer_bak = NULL;
		}
	}
	else if ( type == CONFIG)
	{
        if ( g_config_buffer == NULL)
		{
				free(config);
				config = NULL;
				return ;
		}
	
		if((g_config_buffer[0]=='m')&&(strncmp(&g_config_buffer[0],&config[0],21 ) !=0)) 
		{
		    if( strncmp(&g_config_buffer[0],&mac_tmp[0],21 ) ==0)
            {
                if(config[0] == 'm')
				strncpy(&g_config_buffer[0],&config[0],21);
				FLASH_DEBUG("config not burn mac \n");
		    }
		}
		if((strncmp(&g_config_buffer[22],&config[0],36 ) !=0)&&(g_config_buffer[22]=='u') ) 
		{
			FLASH_DEBUG("CONFIG\n");
			if( strncmp(&g_config_buffer[22],&uid_tmp[0],36 ) ==0)
            {

                uidflash =    strstr(config, "uid=");
				verflash =    strstr(config, "ver=");
				verbuf   =    strstr(g_config_buffer, "ver=");
				if(uidflash&&verflash&&verbuf)
				{
					uid_len = verflash - uidflash;
					#if 0
					strncpy(&g_config_buffer[22],&config[22],uid_len);
					strncpy(&g_config_buffer[22+uid_len],verbuf,18);
					#else
					strncpy(&g_config_buffer[22],&config[22],uid_len+19);
					#endif
				}
				FLASH_DEBUG("config not burn uid \n");
		    }

		}
		

//        start_private = sunxi_partition_get_offset_byname("private");
		net_addr_start  = start_private+((256 - 16) * 1024/512) ;
        FLASH_DEBUG(" CONFIG buf:%s \n",g_config_buffer);
        index_begin =( start_private * 512+(256 - 16) * 1024 )/(64*1024);
#if 1		
        spinor_erase_one_block(index_begin);
		spinor_net_datafinish(net_addr_start,512,&g_config_buffer[0]);
		spinor_read_nocache(start_private+((256 - 16) * 1024/512),1,config);
#endif		
        printf(" CONFIG flash: %s\n",config);

		if (g_config_buffer !=NULL)
		{
			free(g_config_buffer);
			g_config_buffer = NULL ;
		}
		if(g_config_buffer_bak !=NULL) 
		{
			free(g_config_buffer_bak);
			g_config_buffer_bak = NULL;
		}
	
	}
	
	free(config);
}
#endif
#endif
static int sunxi_flash_read_all(u32 start, ulong buf, const char *part_name)
{
	u32 rbytes, rblock;
	u32 start_block = start;
	void *addr;
	struct fastboot_boot_img_hdr *fb_hdr;
	int ret =0;
	addr = (void *)buf;

#ifdef CONFIG_SUNXI_GETH
#ifdef NET_UPGRADE
	
	int tftp_size;
    int update = 0;
    tftp_netboot_common(4,NULL, 3, "40007800","dhcp" );  /*4 ->dhcp*/

	if (dhcp_netupdate_status ==1) {
    	 dhcp_netupdate_status = 0; 
         tftp_netboot_common(3,NULL, 3, "40007800","config.fex" );   /*3-> TFTP */

		 if(g_config_down_load == 1)
		 	update = is_update(BOOT);
		
		 if ((g_config_down_load == 1) &&  (update==1) ) {
   	     	tftp_size = tftp_netboot_common(3,NULL, 3, "40007800","boot.fex" );   /*3-> TFTP */
			if ( tftp_size > 0)
			{
            	update_version(BOOT) ;
				FLASH_DEBUG("update boot.fex  success \n");
				net_upgrade_reset= 1;
			}
			
		}    

		 if(update == 1) {
			printf("update boot.fex\n");
		 }
		 else {
			printf(" boot.fex same \n");
		 }
#if 1		 
		 if(g_config_down_load == 1)
        	 update = is_update(ROOTFS);
		 if ( (g_config_down_load ==1) && (update==1 )) {
		 	
	     	tftp_size = tftp_netboot_common(3,NULL, 3, "40007800","rootfs.fex" );   /*3-> TFTP */
			if ( tftp_size > 0)
			{
			   update_version(ROOTFS) ;
			   net_upgrade_reset= 1;
			   
			   FLASH_DEBUG("update rootfs.fex success\n");
			}
		 }
#endif		 

		  if(update == 1) {
			printf("update rootfs.fex\n");
		 }
		 else {
			printf(" rootfs.fex is same \n");
		 }

		
	}

    if((g_config_down_load ==1)&&(net_upgrade_reset == 0)) {

		 net_upgrade_reset = 1;
		 printf(" update config now \n");
		 update_version(CONFIG);
	}

	g_config_down_load = 0;
    if( net_upgrade_reset == 1) {
		net_upgrade_reset = 0;
	//	FLASH_DEBUG(" net upgrade now \n");
    //	do_reset (NULL, 0, 0, NULL);    /*reboot now */
    }
	
#endif	
#endif
	ret = sunxi_flash_read(start_block, SUNXI_FLASH_READ_FIRST_SIZE/512, addr);
	if(!ret)
	{
		printf("read all error: start=%x, addr=%x\n", start_block, (u32)addr);

		return 1;
	}
	fb_hdr = (struct fastboot_boot_img_hdr *)addr;
	if (memcmp(fb_hdr->magic, FASTBOOT_BOOT_MAGIC, 8))
	{
		debug("boota: bad boot image magic, maybe not a boot.img?\n");
		printf("try to read all\n");
		debug("part name=%s\n", part_name);
		rbytes = sunxi_partition_get_size_byname(part_name) * 512;
	}
	else
	{
#ifdef CONFIG_SUNXI_SPINOR_PLATFORM	
		rbytes = sunxi_partition_get_size_byname(part_name) * 512 + 511;	
#else
		rbytes = fb_hdr->kernel_size + fb_hdr->ramdisk_size + fb_hdr->second_size + 1024 * 1024 + 511;
#endif		
		
	}
	rblock = rbytes/512 - SUNXI_FLASH_READ_FIRST_SIZE/512;
	debug("rblock=%d, start=%d\n", rblock, start_block);
	start_block += SUNXI_FLASH_READ_FIRST_SIZE/512;
	addr = (void *)(buf + SUNXI_FLASH_READ_FIRST_SIZE);

	ret = sunxi_flash_read(start_block, rblock, addr);

	tick_printf("sunxi flash read :offset %x, %d bytes %s\n", start<<9, rbytes,
		       ret ? "OK" : "ERROR");

	return ret == 0 ? 1 : 0;

}

int do_sunxi_flash(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	ulong addr;
	char *cmd;
	char *part_name;

	/* at least four arguments please */
	if ((argc != 4) && (argc != 5))
		goto usage;

	cmd = argv[1];
	part_name = argv[3];
/*
************************************************
*************  read only   *********************
************************************************
*/

	if (strncmp(cmd, "read", 4) == 0)
	{
		u32 start_block;
		u32 rblock;
		int readall_flag = 0;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

		if((!strncmp(part_name, "boot", 4)) || (!strncmp(part_name, "recovery", 8)))
		{
			readall_flag = 1;
		}
		start_block = sunxi_partition_get_offset_byname((const char *)part_name);
		if(!start_block)
		{
			printf("cant find part named %s\n", (char *)part_name);

			goto usage;
		}
		if(argc == 4)
		{
			if(readall_flag)
			{
				puts("read boot or recovery all\n");

				return sunxi_flash_read_all(start_block, addr, (const char *)part_name);
			}
			rblock = sunxi_partition_get_size_byname((const char *)part_name);
		}
		else
		{
			rblock = (u32)simple_strtoul(argv[4], NULL, 16)/512;
		}
#ifdef DEBUG
		printf("part name   = %s\n", part_name);
		printf("start block = %x\n", start_block);
		printf("     nblock = %x\n", rblock);
#endif
		ret = sunxi_flash_read(start_block, rblock, (void *)addr);

		tick_printf("sunxi flash read :offset %x, %d bytes %s\n", start_block<<9, rblock<<9,
		       ret ? "OK" : "ERROR");

		return ret == 0 ? 1 : 0;
	}
	else if(strncmp(cmd, "log_read", strlen("log_read")) == 0)
	{
#if 0	
		u32 start_block;
		u32 rblock;

        printf("read logical\n");

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		start_block = (ulong)simple_strtoul(argv[3], NULL, 16);
		rblock = (ulong)simple_strtoul(argv[4], NULL, 16);

		ret = sunxi_flash_read(start_block, rblock, (void *)addr);

		tick_printf("sunxi flash log_read :offset %x, %d sectors %s\n", start_block, rblock,
		       ret ? "OK" : "ERROR");

		return ret == 0 ? 1 : 0;
#endif		
	}
	else if(strncmp(cmd, "phy_read", strlen("phy_read")) == 0)
	{
#if 0	
		u32 start_block;
		u32 rblock;

        printf("read physical\n");

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		start_block = (ulong)simple_strtoul(argv[3], NULL, 16);
		rblock = (ulong)simple_strtoul(argv[4], NULL, 16);

		ret = sunxi_flash_phyread(start_block, rblock, (void *)addr);

		tick_printf("sunxi flash phy_read :offset %x, %d sectors %s\n", start_block, rblock,
		       ret ? "OK" : "ERROR");

		return ret == 0 ? 1 : 0;
#endif		
	}

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	sunxi_flash, CONFIG_SYS_MAXARGS, 1, do_sunxi_flash,
	"sunxi_flash sub-system",
	"read command parmeters : \n"
	"parmeters 0 : addr to load(hex only)\n"
	"parmeters 1 : the name of the part to be load\n"
	"[parmeters 2] : the number of bytes to be load(hex only)\n"
	"if [parmeters 2] not exist, the number of bytes to be load "
	"is the size of the part indecated on partemeter 1"
);
