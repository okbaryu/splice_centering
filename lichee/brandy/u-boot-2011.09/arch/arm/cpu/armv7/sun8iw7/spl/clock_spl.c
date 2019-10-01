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
#include "common.h"
#include "asm/io.h"
#include "asm/armv7.h"
#include "asm/arch/cpu.h"
#include "asm/arch/ccmu.h"
#include "asm/arch/timer.h"
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
static int clk_set_divd(void)
{
	unsigned int reg_val;

	//config axi
	reg_val = readl(CCM_CPU_L2_AXI_CTRL);
	reg_val &= ~(0x03 << 0);
	reg_val |=  (0x02 << 0);
	writel(reg_val, CCM_CPU_L2_AXI_CTRL);
	//config ahb
	reg_val = readl(CCM_AHB1_APB1_CTRL);;
	reg_val &= ~((0x03 << 12) | (0x03 << 8) | (0x03 << 6) | (0x03 << 4));
	reg_val |=   (0x03 << 12);
	reg_val |=  (2 << 6);
	reg_val |=  (1 << 8);

	writel(reg_val, CCM_AHB1_APB1_CTRL);

	return 0;
}
/*******************************************************************************
*��������: set_pll
*����ԭ�ͣ�void set_pll( void )
*��������: ����CPUƵ��
*��ڲ���: void
*�� �� ֵ: void
*��    ע:
*******************************************************************************/
void set_pll( void )
{
    unsigned int reg_val;
    unsigned int i;
    //����ʱ��ΪĬ��408M

    //�л���24M
    reg_val = readl(CCM_CPU_L2_AXI_CTRL);
    reg_val &= ~(0x03 << 16);
    reg_val |=  (0x01 << 16);
    writel(reg_val, CCM_CPU_L2_AXI_CTRL);
    //��ʱ���ȴ�ʱ���ȶ�
    for(i=0; i<0x400; i++);
	//��дPLL1
    reg_val = readl(CCM_PLL1_CPUX_CTRL);
    reg_val &= ~((0x03 << 16) | (0x1f << 8) | (0x03 << 4) | (0x03 << 0));
	reg_val |=  (16<<8);
    writel(reg_val, CCM_PLL1_CPUX_CTRL);
    //��ʱ���ȴ�ʱ���ȶ�
#ifndef CONFIG_FPGA
	do
	{
		reg_val = readl(CCM_PLL1_CPUX_CTRL);
	}
	while(!(reg_val & (0x1 << 28)));
#endif
    //�޸�AXI,AHB,APB��Ƶ
    clk_set_divd();
    //�л�ʱ�ӵ�COREPLL��
    reg_val = readl(CCM_CPU_L2_AXI_CTRL);
    reg_val &= ~(0x03 << 16);
    reg_val |=  (0x02 << 16);
    writel(reg_val, CCM_CPU_L2_AXI_CTRL);

    return  0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void reset_pll( void )
{
    //�л�CPUʱ��ԴΪ24M
    writel(0x00000000, CCM_CPU_L2_AXI_CTRL);
    //��ԭPLL1ΪĬ��ֵ
	writel(0x00001000, CCM_PLL1_CPUX_CTRL);

	return ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void set_gpio_gate(void)
{
	writel(readl(CCM_APB1_GATE0_CTRL)   | (1 << 5), CCM_APB1_GATE0_CTRL);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
void set_ccmu_normal(void)
{
	writel(7, CCM_SECURITY_REG);
}

