/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Young <guoyingyang@allwinnertech.com>
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

#include "common.h"
#include "asm/armv7.h"
#include <private_boot0.h>
#include <power/axp20_reg.h>
#include <i2c.h> 


extern const boot0_file_head_t  BT0_head;
extern int debug_mode;

static int axp20_set_dcdc2(int set_vol, int onoff)
{ 	
    u32 vol, tmp;
	volatile u32 i;  
	u8  reg_addr, value;
	if(set_vol == -1)	
	{	
		set_vol = 1400;
	}
	//PMU is AXP209   
	reg_addr = BOOT_POWER20_DC2OUT_VOL;
	if(i2c_read(AXP20_ADDR, reg_addr,1, &value,1))
	{        return -1;    }
	tmp     = value & 0x3f;
	vol     = tmp * 25 + 700;
	//�����ѹ���ߣ������
	while(vol > set_vol) {
		tmp -= 1;       
		value &= ~0x3f;
		value |= tmp;
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_write(AXP20_ADDR, reg_addr,1, &value,1))
		{       
			return -1;      
		}
		for(i=0;i<2000;i++);
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_read(AXP20_ADDR, reg_addr,1, &value,1))
		{
			return -1;
		}
		tmp     = value & 0x3f;
		vol     = tmp * 25 + 700;
    }
	//�����ѹ���ͣ�����ߣ������ȵ����ٵ��ߵĹ��̣���֤��ѹ����ڵ����û��趨��ѹ+    
	while(vol < set_vol)
	{     
		tmp += 1;
		value &= ~0x3f;
		value |= tmp;
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_write(AXP20_ADDR, reg_addr, 1,&value,1))
		{            
			return -1;        
		}
		for(i=0;i<2000;i++);
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_read(AXP20_ADDR, reg_addr, 1,&value,1))
		{           
			return -1;        
		}
		tmp     = value & 0x3f;
		vol     = tmp * 25 + 700;
	}	
	printf("after set, dcdc2 =%dmv\n",vol);

	return 0;
}

int pmu_set_vol(void){

	u8 pmu_type = 0;
	i2c_init(CONFIG_SYS_I2C_SPEED,CONFIG_SYS_I2C_SLAVE);
	if(i2c_read(AXP20_ADDR,BOOT_POWER20_VERSION,1,&pmu_type,1))
	{
		printf("axp read fail, maybe no pmu \n");   
		return -1; 
	}
	pmu_type &= 0x0f;
	if(pmu_type & 0x01)	{
		printf("PMU: axp version ok \n");
	} 
	else 
	{
		printf("try pmu axp fail, maybe no pmu \n");
		return -1; 
	}       //set sys vol +      
	if(axp20_set_dcdc2(1100,1))
	{
			printf("axp20 set dcdc2 vol fail, maybe no pmu \n");
			return -1;
	}
	printf("axp20 set dcdc2 success \n");


    return 0;
}


/*
************************************************************************************************************
*
*                                             function
*
*    name          :set_debugmode_flag
*
*    parmeters     :void
*
*    return        :
*
*    note          :if BT0_head.prvt_head.debug_mode_off = 1,do not print any message to uart 
*
*
************************************************************************************************************
*/

void set_debugmode_flag(void)
{
#if 0
        char c = 0;
        int i = 0;
        for( i = 0 ; i < 3 ; i++)
        {
                __msdelay(10);
                if(sunxi_serial_tstc())
                {
                        printf("key_press  \n");
                        c = sunxi_serial_getc();
                        printf("0x%x \n",c);
                        break;
                }
        }
        if(c  == 's')
        {
                debug_mode = 1;
                return ;
        }
	if(BT0_head.prvt_head.debug_mode)
		debug_mode = 1;
	else
		debug_mode = 0;
	return ;
#endif	
		
}
