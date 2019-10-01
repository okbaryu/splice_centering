/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <mmc.h>
#include <search.h>
#include <errno.h>
#include <nand.h>
#ifdef CONFIG_ALLWINNER
#include <boot_type.h>
#include <sys_partition.h>
#endif
extern uchar default_environment[];

char * env_name_spec = "SUNXI";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = NULL;
#endif /* ENV_IS_EMBEDDED */

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

const uchar sunxi_sprite_environment[] = {
#ifdef  CONFIG_SUNXI_SPRITE_ENV_SETTINGS
	CONFIG_SUNXI_SPRITE_ENV_SETTINGS
#endif
	"\0"
};


#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

//loff_t env_offset = (loff_t)CONFIG_ENV_ADDR;
size_t env_size = (size_t)CONFIG_ENV_SIZE;

uchar env_get_char_spec(int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
#error No support for redundant environment on sunxi nand yet!
#endif

static void flash_use_efex_env(void)
{
	if (himport_r(&env_htab, (char *)sunxi_sprite_environment,
		    sizeof(sunxi_sprite_environment), '\0', 0) == 0) {
		error("Environment import failed: errno = %d\n", errno);
	}
	gd->flags |= GD_FLG_ENV_READY;

	return ;
}

static int flash_saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res;
	u32     start;

	start = sunxi_partition_get_offset_byname(CONFIG_SUNXI_ENV_PARTITION);
	if(!start){
		printf("fail to find part named %s\n", CONFIG_SUNXI_ENV_PARTITION);
		return -1;
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc   = crc32(0, env_new.data, ENV_SIZE);

	return sunxi_flash_write(start, env_size/512, &env_new);
}

int saveenv(void)
{
	printf("saveenv storage_type = %d\n", uboot_spare_head.boot_data.storage_type);
	return flash_saveenv();
}

static void flash_env_relocate_spec(int workmode)
{
#if !defined(ENV_IS_EMBEDDED)
	char buf[CONFIG_ENV_SIZE];
	u32 start;
	int value, ret;

	if((workmode & WORK_MODE_PRODUCT) && (!(workmode & WORK_MODE_UPDATE)))
	{
		flash_use_efex_env();
	}
	else
	{
		start = sunxi_partition_get_offset_byname(CONFIG_SUNXI_ENV_PARTITION);
		if (!start) {
			printf("fail to find part named %s\n", CONFIG_SUNXI_ENV_PARTITION);
			use_default();
			return;
		}

		if (!sunxi_flash_read(start, CONFIG_ENV_SIZE/512, buf)) {
			use_default();
			return;
		}
		ret = script_parser_fetch("env_crc", "env_crc_check_no_used", &value, 1);
		if (!ret && (value == 1)) {
			printf("not check env\n");
			env_import(buf, 0);
		} else {
			env_import(buf, 1);
		}

#ifdef CONFIG_FLASH_NOT_BURN_ALL 
#define UUID_OFFSET  24
	char config_dat[1024] ="mac_addr=30:10:10:10:10:30;uid=12345678;ver=1.0.0;" ;
    char mac_dat[18]={0};
	char uid_dat[38]={0};
	char ver_dat[18]={0};
//	memset(config_dat,'c',1024);
	u32 start_private= sunxi_partition_get_offset_byname("private");
	u32 private_size_len=sunxi_partition_get_size_byname("private");
	
	sunxi_flash_read(start_private,1,config_dat);
	if(strncmp("mac",config_dat,3)==0){
		strncpy(mac_dat,&config_dat[4],17);
		printf("mac=%s\n",mac_dat);
		if(strncmp("mac=00:00:00:00:00:00",config_dat,21)!=0)
		  setenv("mac",mac_dat);
		if(strncmp("uid",&config_dat[22],3)==0){
			strncpy(uid_dat,&config_dat[26],(8+UUID_OFFSET));
			setenv("uid",uid_dat);
			TFTP_ENV_DEBUG("uid=%s\n",uid_dat);
	    }
		if(strncmp("ver",&config_dat[(35+UUID_OFFSET)],3)==0){
			strncpy(ver_dat,&config_dat[(39+UUID_OFFSET)],5);
			setenv("ver",ver_dat);
			TFTP_ENV_DEBUG("ver=%s\n",ver_dat);
//			TFTP_ENV_DEBUG("read config_dat =%s\n",config_dat);
	    }
	}
	else{
		printf(" flash_env_relocate_spec \n");
//		setenv("mac","20:20:20:20:30:40");
//		setenv("uid","32345679");
	}
#endif
	}
#endif
}


void env_relocate_spec(void)
{
	debug("env_relocate_spec storage_type = %d\n", uboot_spare_head.boot_data.storage_type);
	flash_env_relocate_spec(uboot_spare_head.boot_data.work_mode);
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	set_default_env(NULL);
}
#endif

/*
 *  This is called before nand_init() so we can't read NAND to
 *  validate env data.
 */

