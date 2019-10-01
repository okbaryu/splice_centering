/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <private_boot0.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include <asm/arch/uart.h>
#include <asm/arch/dram.h>

extern const boot0_file_head_t  BT0_head;
extern int pmu_set_vol(void);


static void print_version(void);

void __attribute__((weak)) bias_calibration(void)
{
	return;
}

#define ss_write(value,addr)     *((volatile unsigned int *)(addr))  = value
#define ss_read_w(n)   		 (*((volatile unsigned int *)(n)))          
#define SS_BASE				 (0x01c15000)

#define BUS_CLK_GATING_REG   (0x01c20000+0X60)        // bit5 -> ss
#define BUS_RESET_SOFT_REG   (0x01c20000+0x2c0)
#define SS_CLK_REG           (0x01c20000+0x9c)          // bit31 -> ss

unsigned int get_ifm(void)
{
	unsigned int reg_val;

	ss_write(ss_read_w(BUS_CLK_GATING_REG)|(1<<5),BUS_CLK_GATING_REG);
//	printf("BUS_CLK_GATING_REG =[0x%x]\n",ss_read_w(BUS_CLK_GATING_REG));

	ss_write(ss_read_w(BUS_RESET_SOFT_REG)|(1<<5),BUS_RESET_SOFT_REG);
//	printf("BUS_RESET_SOFT_REG =[0x%x]\n",ss_read_w(BUS_RESET_SOFT_REG));

	ss_write(ss_read_w(SS_CLK_REG)|(1<<31)|(1<<24),SS_CLK_REG);
//	printf("SS_CLK_REG =[0x%x]\n",ss_read_w(SS_CLK_REG));

	reg_val = ss_read_w(SS_BASE);
	reg_val >>=16;
	reg_val &=0x7;

	printf("boot0: ifm = %d\n", reg_val);

	return reg_val;
}

/*******************************************************************************
*函数名称: Boot0_C_part
*函数原型：void Boot0_C_part( void )
*函数功能: Boot0中用C语言编写的部分的主流程
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void main( void )
{
	__u32 status;
	__s32 dram_size;
	int	index = 0;
	int   ddr_aotu_scan = 0;
	int ss_value = 0; // 0 -> V3 ; 3 or 7 -> v3s

    __u32 fel_flag;

	bias_calibration();
    timer_init();

    sunxi_serial_init( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6 );
	if( BT0_head.prvt_head.enable_jtag )
    {
    	boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 6, 1);
    }
	printf("boot0: Hello! BOOT0 is starting!\n");
	ss_value = get_ifm();
	if((ss_value == 0)||(ss_value == 7)||(ss_value == 3)) {
	} else {
		printf("get_ifm warning ...\n");
		boot0_jump(FEL_BASE);
	}
	//print_version();

#ifdef CONFIG_ARCH_SUN7I
	reset_cpux(1);
#endif
    fel_flag = rtc_region_probe_fel_flag();
    if(fel_flag == SUNXI_RUN_EFEX_FLAG)
    {
        rtc_region_clear_fel_flag();
    	printf("eraly jump fel\n");
    	reset_pll();
    	__msdelay(10);

    	boot0_jump(FEL_BASE);
    }

	mmu_setup();
	
#if 1	
	if(ss_value != 0)
	{
		if(pmu_set_vol()) {
	 	printf("set pmu vol failed,maybe no pmu \n");
//		goto __boot0_entry_err;        
		}
	}
#endif	
	
	ddr_aotu_scan = 0;
//	dram_para_display();
	dram_size = init_DRAM(ddr_aotu_scan, (void *)BT0_head.prvt_head.dram_para);
	if(dram_size)
	{
	    //mdfs_save_value();
		//printf("dram size =%d\n", dram_size);
	}
	else
	{
		printf("initializing SDRAM Fail.\n");
		mmu_turn_off( );

		reset_pll();
		boot0_jump(FEL_BASE);
	}
#if defined(CONFIG_ARCH_SUN9IW1P1)
	__msdelay(100);
#endif

#ifdef CONFIG_ARCH_SUN7I
    check_super_standby_flag();
#endif

	status = load_boot1();

	//printf("Ready to disable icache.\n");

	mmu_turn_off( );                               // disable instruction cache

	if( status == 0 )
	{
		//跳转之前，把所有的dram参数写到boot1中
		set_dram_para((void *)&BT0_head.prvt_head.dram_para, dram_size);
		printf("Jump to secend Boot.\n");

		boot0_jump(CONFIG_SYS_TEXT_BASE);		  // 如果载入Boot1成功，跳转到Boot1处执行
	}
	else
	{
//		disable_watch_dog( );                     // disable watch dog
		reset_pll();
		printf("Jump to Fel.\n");
		boot0_jump(FEL_BASE);                     // 如果载入Boot1失败，将控制权交给Fel
	}
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void print_version(void)
{
	printf("boot0 version : %s\n", BT0_head.boot_head.platform + 2);

	return;
}

