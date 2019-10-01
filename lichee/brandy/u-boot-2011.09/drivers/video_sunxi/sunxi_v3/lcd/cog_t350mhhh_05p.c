/* 
 **********************************************************************************************************
 * 
 * Initial setting for gd9161 panel
 * 
 * cog_t350mhhh_05p.c module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description			   Email
 * 
 *   1.0		richard.liu       2015/11/15     first Version                liubaihao@sina.com.cn
 ***********************************************************************************************************
 */

#include "cog_t350mhhh_05p.h"

#define cog_t350mhhh_05p_spi_sdi_1	sunxi_lcd_gpio_set_value(0,3,1)
#define cog_t350mhhh_05p_spi_sdi_0	sunxi_lcd_gpio_set_value(0,3,0)
#define cog_t350mhhh_05p_spi_rst_1	sunxi_lcd_gpio_set_value(0,2,1)
#define cog_t350mhhh_05p_spi_rst_0	sunxi_lcd_gpio_set_value(0,2,0)
#define cog_t350mhhh_05p_spi_scl_1	sunxi_lcd_gpio_set_value(0,1,1)
#define cog_t350mhhh_05p_spi_scl_0	sunxi_lcd_gpio_set_value(0,1,0)

#define cog_t350mhhh_05p_spi_cs_1	sunxi_lcd_gpio_set_value(0,0,1)
#define cog_t350mhhh_05p_spi_cs_0	sunxi_lcd_gpio_set_value(0,0,0)

#define Delayms sunxi_lcd_delay_ms

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, LCD_power_on, 50);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 5);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 100);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms
	printf("turn off lcd power!!!\n");
	return 0;
}

static void LCD_power_on(u32 sel)
{
	sunxi_lcd_power_enable(sel, 3);//config lcd_power pin to open lcd power3
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 3);//config lcd_power pin to close lcd power3
}

static void LCD_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
}

static void write_spidat(u32 value)
{
	u32 i;
	cog_t350mhhh_05p_spi_cs_0;
	cog_t350mhhh_05p_spi_sdi_1;
	cog_t350mhhh_05p_spi_scl_0;
	sunxi_lcd_delay_us(10);
	cog_t350mhhh_05p_spi_scl_1;
	sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		cog_t350mhhh_05p_spi_scl_0;
		if(value & 0x80)
			cog_t350mhhh_05p_spi_sdi_1;
		else
			cog_t350mhhh_05p_spi_sdi_0;
		value <<= 1;
		sunxi_lcd_delay_us(10);
		cog_t350mhhh_05p_spi_scl_0;
		cog_t350mhhh_05p_spi_scl_1;
	}
	sunxi_lcd_delay_us(10);
	cog_t350mhhh_05p_spi_cs_1;
}

static void write_spicom(u32 value)
{
	u32 i;
	cog_t350mhhh_05p_spi_cs_0;
	cog_t350mhhh_05p_spi_sdi_0;
	cog_t350mhhh_05p_spi_scl_0;
	sunxi_lcd_delay_us(10);
	cog_t350mhhh_05p_spi_scl_1; 
	sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		cog_t350mhhh_05p_spi_scl_0;
		if(value & 0x80)
			cog_t350mhhh_05p_spi_sdi_1;
		else
			cog_t350mhhh_05p_spi_sdi_0;
		
		cog_t350mhhh_05p_spi_scl_0; 
		sunxi_lcd_delay_us(10);
		cog_t350mhhh_05p_spi_scl_1;
		value <<= 1;
		
	}
	sunxi_lcd_delay_us(10);
	cog_t350mhhh_05p_spi_cs_1;
}

static void LCD_panel_init(u32 sel)
{
	int width, height;
	int rotate;
	unsigned char madctl;
	unsigned short col, page;

	printf("=====cog_t350mhhh_05p=========\n");

	width = 320;
	height = 480;
	rotate = 90;

	cog_t350mhhh_05p_spi_rst_1;
	Delayms(2);
	cog_t350mhhh_05p_spi_rst_0;
	Delayms(20);
	cog_t350mhhh_05p_spi_rst_1;
	Delayms(240);

	write_spicom(0x11); //Sleep Out
	sunxi_lcd_delay_ms(200);

	// Sleep Out CMD will reload OTP
	// If any CMD has been programmed OTP, and it need to set again in the initial
	//code. The CMD should be set after Sleep Out
	write_spicom(0xB9); // SET password
	write_spidat(0xFF);
	write_spidat(0x83);
	write_spidat(0x57);

	if (rotate > 0) {
		write_spicom(0xB0); // SETOSC
		write_spidat(0x66);
		write_spidat(0x01);
	}

	write_spicom(0xB1); //SETPower
	write_spidat(0x00); //STB
	write_spidat(0x12); //VGH
	write_spidat(0x18); //VSPR
	write_spidat(0x18); //VSNR
	write_spidat(0xC3); //AP
	write_spidat(0x31); //FS

#define SETRGB_DM		0x01
#define SETRGB_BYPASS	0x40

	write_spicom(0xB3);	// SETRGB
	if (rotate) {
		// Memory, SDO_EN=1, BYPASS=0, RM=1, DM=0(Internal OSC)
		write_spidat(0x82);
		// Memory, SDO_EN=1, BYPASS=0, RM=1, DM=1(DPI Interface)
		// write_spidat(0x82 | SETRGB_DM);
	} else {
		// Bypass, SDO_EN=1, BYPA0S=1, RM=1, DM=1(DPI Interface)
		write_spidat(0x82 | SETRGB_BYPASS | SETRGB_DM);
	}
	// TODO: dclk/sync polarity
	write_spidat(0x08);	// DPL=1, HSPL=0, VSPL=0, EPL=0
	// TODO: hbp
	write_spidat(0x86);	// RCM=1, HBP=6
	// TODO: vbp
	write_spidat(0x06);	// VBP=6

	write_spicom(0xB4); //SETCYC
	write_spidat(0x02); //2-dot
	write_spidat(0x40); //RTN
	write_spidat(0x00); //DIV
	write_spidat(0x2A); //N_DUM
	write_spidat(0x2A); //I_DUM
	write_spidat(0x35); //GDON
	write_spidat(0x4E); //GDOFF

	write_spicom(0xB5); //SETBGP
	write_spidat(0x06);
	write_spidat(0x06);

	write_spicom(0xB6); //VCOMDC
	write_spidat(0x10);

	write_spicom(0xC0); //SETSTBA
	write_spidat(0x20); //N_OPON
	write_spidat(0x20); //I_OPON
	write_spidat(0x00); //STBA
	write_spidat(0x08); //STBA
	write_spidat(0X1C); //STBA
	write_spidat(0x08); //GENON

	write_spicom(0xC2); // Set Gate EQ
	write_spidat(0x00);
	write_spidat(0x04);
	write_spidat(0x04);

	write_spicom(0xCC); //Set Panel
	write_spidat(0x05);

	write_spicom(0xE0); // SETGamma: set gamma curve
	write_spidat(0x00);
	write_spidat(0x00);
	write_spidat(0x08);
	write_spidat(0x10);
	write_spidat(0x18);
	write_spidat(0x3B);
	write_spidat(0x47);
	write_spidat(0x50);
	write_spidat(0x48);
	write_spidat(0x42);
	write_spidat(0x3A);
	write_spidat(0x32);
	write_spidat(0x2E);
	write_spidat(0x29);
	write_spidat(0x24);
	write_spidat(0x19);
	write_spidat(0x00);
	write_spidat(0x00);
	write_spidat(0x08);
	write_spidat(0x10);
	write_spidat(0x18);
	write_spidat(0x3B);
	write_spidat(0x47);
	write_spidat(0x50);
	write_spidat(0x48);
	write_spidat(0x42);
	write_spidat(0x3A);
	write_spidat(0x32);
	write_spidat(0x2E);
	write_spidat(0x29);
	write_spidat(0x24);
	write_spidat(0x19);
	write_spidat(0x00);
	write_spidat(0x01);

#define MADCTL_MV	0x20

	write_spicom(0x36); // Memory Access Control
	switch (rotate) {
		case 0:
		default:
			// Normal
			madctl = 0x00;
			break;

		case 90:
			// X-Y Exchange, Y-invert
			madctl = 0x60;
			break;
	}
	// write_spidat(0x80); // flip? Y-invert
	// write_spidat(0x40); // flip? X-invert
	// write_spidat(0xC0);
	// write_spidat(0xA0); // X-Y Exchange, X-invert
	write_spidat(madctl);

	if (madctl & MADCTL_MV) {
		col = height - 1;
		page = width - 1;
	} else {
		col = width - 1;
		page = height - 1;
	}
	write_spicom(0x2A);
	write_spidat(0x00);
	write_spidat(0x00);
	write_spidat(col >> 8);
	write_spidat(col);

	write_spicom(0x2B);
	write_spidat(0x00);
	write_spidat(0x00);
	write_spidat(page >> 8);
	write_spidat(page);

	write_spicom(0x3A); // COLMOD
	write_spidat(0x66); // RGB666
	sunxi_lcd_delay_ms(10);

	write_spicom(0x29); //Display on
	sunxi_lcd_delay_ms(10);
}

static void LCD_panel_exit(u32 sel)
{
	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t  cog_t350mhhh_05p_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "cog_t350mhhh_05p",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
