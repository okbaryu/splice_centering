/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
#ifndef  _SUNXI_SPINAND_H
#define  _SUNXI_SPINAND_H


extern int spinand_get_mbr(char* buffer, uint len);

extern int spinand_uboot_init(int boot_mode);
extern int spinand_uboot_exit(int force);

extern int spinand_uboot_probe(void);

extern uint spinand_uboot_read(uint start, uint sectors, void *buffer);

extern uint spinand_uboot_write(uint start, uint sectors, void *buffer);


extern int spinand_download_boot0(uint length, void *buffer);
extern int spinand_download_uboot(uint length, void *buffer);

extern int spinand_force_download_uboot(uint length ,void *buffer);
extern int spinand_uboot_erase(int user_erase);

extern uint spinand_uboot_get_flash_info(void *buffer, uint length);

extern uint spinand_uboot_set_flash_info(void *buffer, uint length);

extern uint spinand_uboot_get_flash_size(void);

extern int spinand_uboot_flush(void);

extern int SPINAND_Uboot_Force_Erase(void);

uint spinand_upload_boot0(uint length, void *buf);

int spinand_download_boot0_simple(uint length, void *buffer);

#endif

