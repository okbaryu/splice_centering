#include "d500n3437v0.h"

#define Hsync_Display_Period    480	
#define Vsync_Display_Period    854
#define pixel_clock   33			//单位 	M
#define TFT_panel_data_width  1     // 0 18bit  // 1 24bit
#define Dclk_polarity 	1           // 0 Data latch in rising edge  //  1 Data latch in falling edge
#define Hsync_polarity 	0           // 0 Active low //1 Active high
#define Vsync_polarity 	0           // 0 Active low //1 Active high
#define TFT_type   0                //0,1 TFT mode  //2 Serial RGB mode  //3 Serial RGB+dummy mode 
#define Even_line_RGB_sequence 0    //0 RGB //1 RBG //2 GRB //3 GBR //4 BRG //5 BGR       
#define	Odd_line_RGB_sequence  0    //0 RGB //1 RBG //2 GRB //3 GBR //4 BRG //5 BGR
#define	Hsync_Back_Porh   43
#define	Hsync_Front_Porh  8
#define	Hsync_Pulse_width 2	
#define	Vsync_Back_Porh   42
#define	Vsync_Front_Porh  4
#define	Vsync_Pulse_width 10
#define	Display_pixel_format 0x70     //0x50 16-bit/pixel //0x60 18-bit/pixel  //0x70 24-bit/pixel  //0x10 3-bit/pixel  //0x20 8-bit/pixel  //0x30 12-bit/pixel
#define Pixel_Data_Interface_Format 5 //0 8-bit //1	12-bit //2 16-bit packed  //3  16-bit 565  //4 18-bit  //5  24-bit  //6  9-bit



#define d500n3437v0_spi_scl_1	sunxi_lcd_gpio_set_value(0,1,1)
#define d500n3437v0_spi_scl_0	sunxi_lcd_gpio_set_value(0,1,0)
#define d500n3437v0_spi_sdi_1	sunxi_lcd_gpio_set_value(0,2,1)
#define d500n3437v0_spi_sdi_0	sunxi_lcd_gpio_set_value(0,2,0)
#define d500n3437v0_spi_cs_1  	sunxi_lcd_gpio_set_value(0,0,1)
#define d500n3437v0_spi_cs_0  	sunxi_lcd_gpio_set_value(0,0,0)

#define d500n3437v0_spi_reset_1  	sunxi_lcd_gpio_set_value(0,3,1)
#define d500n3437v0_spi_reset_0  	sunxi_lcd_gpio_set_value(0,3,0)

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
	LCD_OPEN_FUNC(sel, LCD_power_on, 30);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 50);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(u32 sel)
{
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
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

static void SPI_writedat(u32 value)
{
	u32 i;
	d500n3437v0_spi_cs_0;
	d500n3437v0_spi_sdi_1;
	d500n3437v0_spi_scl_0;
	sunxi_lcd_delay_us(10);
	d500n3437v0_spi_scl_1;
	//sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		//d500n3437v0_spi_scl_0;
		if(value & 0x80)
			d500n3437v0_spi_sdi_1;
		else
			d500n3437v0_spi_sdi_0;
		value <<= 1;
		d500n3437v0_spi_scl_0;
		sunxi_lcd_delay_us(10);
		d500n3437v0_spi_scl_1;
	}
	sunxi_lcd_delay_us(10);
	d500n3437v0_spi_cs_1;
}

static void SPI_writecom(u32 value)
{
	u32 i;
	d500n3437v0_spi_cs_0;
	d500n3437v0_spi_sdi_0;
	d500n3437v0_spi_scl_0;
	sunxi_lcd_delay_us(10);
	d500n3437v0_spi_scl_1; 
	//sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		//d500n3437v0_spi_scl_0;
		if(value & 0x80)
			d500n3437v0_spi_sdi_1;
		else
			d500n3437v0_spi_sdi_0;
		
		d500n3437v0_spi_scl_0; 
		sunxi_lcd_delay_us(10);
		d500n3437v0_spi_scl_1;
		value <<= 1;
		
	}
	sunxi_lcd_delay_us(10);
	d500n3437v0_spi_cs_1;
}


//#define write_spicom write_spicom
//#define	write_spidat write_spidat

static void LCD_panel_init(u32 sel)
{
	
	printk("  LCD_panel_init\n");
	d500n3437v0_spi_reset_1;
	sunxi_lcd_delay_ms(10);
	d500n3437v0_spi_reset_0;
	sunxi_lcd_delay_ms(100);
	d500n3437v0_spi_reset_1;
	sunxi_lcd_delay_ms(200);
	printk("  reset finish\n");

	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x00);		 // Change to Page 0	
//	sunxi_lcd_delay_ms(30);
	SPI_writecom(0x11);	sunxi_lcd_delay_ms(200);	// Sleep-Out
	printk("  sleep out 1 \n");
	
	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x00);		 // Change to Page 0	
	//sunxi_lcd_delay_ms(30);
	SPI_writecom(0x01);
	sunxi_lcd_delay_ms(300);	// Sleep-Out
	printk("  software reset \n");
	
	SPI_writecom(0x11);	sunxi_lcd_delay_ms(200);	// Sleep-Out
	printk("  sleep out 2 \n");

	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x01);		 // Change to Page 1
//	sunxi_lcd_delay_ms(30);
	SPI_writecom(0x08);SPI_writedat(0x10);		// output SDA
	SPI_writecom(0x21);SPI_writedat(0x01);		// DE = 1 Active
//	SPI_writecom(0x22);SPI_writedat(0x03);		// GS SS

//	SPI_writecom(0x25);SPI_writedat(4);         //vfp = 4
//	SPI_writecom(0x26);SPI_writedat(42);        //vbp = 42
//	SPI_writecom(0x27);SPI_writedat(43);        //hbp = 43 
//	SPI_writecom(0x28);SPI_writedat(0);          

	SPI_writecom(0x30);SPI_writedat(0x01);		// 480 X 854
	SPI_writecom(0x31);SPI_writedat(0x02);		// 2-dot Inversion
	SPI_writecom(0x40);SPI_writedat(0x16);		// BT	+2.5/-2.5 pump for DDVDH-L
	SPI_writecom(0x41);SPI_writedat(0x77);		// DVDDH DVDDL clamp
	SPI_writecom(0x42);SPI_writedat(0x02);		// VGH/VGL
	SPI_writecom(0x44);SPI_writedat(0x16);		// VGH/VGL

	SPI_writecom(0x50);SPI_writedat(0x67);		// VGMP	  69
	SPI_writecom(0x51);SPI_writedat(0x67);		// VGMN	  69

	SPI_writecom(0x52);SPI_writedat(0x00);		//Flicker
	SPI_writecom(0x53);SPI_writedat(0x38);	    //Flicker 40
											
	SPI_writecom(0x54);SPI_writedat(0x00);      //Flicker 36H=03时调VCOM
	SPI_writecom(0x55);SPI_writedat(0x38);      //Flicker 36H=03时调VCOM


	SPI_writecom(0x57);SPI_writedat(0x50);		//
	SPI_writecom(0x60);SPI_writedat(0x07);	    // SDTI
	SPI_writecom(0x61);SPI_writedat(0x00);		// CRTI
	SPI_writecom(0x62);SPI_writedat(0x08);		// EQTI
	SPI_writecom(0x63);SPI_writedat(0x00);		// PCTI

	//++++++++++++++++++ Gamma Setting ++++++++++++++++++//);
	SPI_writecom(0xA0);SPI_writedat(0x00);		// Gamma 0
	SPI_writecom(0xA1);SPI_writedat(0x0B);		// Gamma 4
	SPI_writecom(0xA2);SPI_writedat(0x14);		// Gamma 8
	SPI_writecom(0xA3);SPI_writedat(0x0A);		// Gamma 16
	SPI_writecom(0xA4);SPI_writedat(0x03);		// Gamma 24
	SPI_writecom(0xA5);SPI_writedat(0x0C);		// Gamma 52
	SPI_writecom(0xA6);SPI_writedat(0x07);		// Gamma 80
	SPI_writecom(0xA7);SPI_writedat(0x03);		// Gamma 108
	SPI_writecom(0xA8);SPI_writedat(0x0B);		// Gamma 147
	SPI_writecom(0xA9);SPI_writedat(0x0D);		// Gamma 175
	SPI_writecom(0xAA);SPI_writedat(0x0C);		// Gamma 203
	SPI_writecom(0xAB);SPI_writedat(0x08);		// Gamma 231
	SPI_writecom(0xAC);SPI_writedat(0x0B);		// Gamma 239
	SPI_writecom(0xAD);SPI_writedat(0x1B);		// Gamma 247
	SPI_writecom(0xAE);SPI_writedat(0x08);  	// Gamma 251
	SPI_writecom(0xAF);SPI_writedat(0x00);		// Gamma 255
	///==============Nagitive
	SPI_writecom(0xC0);SPI_writedat(0x00);		// Gamma 0
	SPI_writecom(0xC1);SPI_writedat(0x0D);	    // Gamma 4
	SPI_writecom(0xC2);SPI_writedat(0x12);		// Gamma 8
	SPI_writecom(0xC3);SPI_writedat(0x10);		// Gamma 16
	SPI_writecom(0xC4);SPI_writedat(0x08);		// Gamma 24
	SPI_writecom(0xC5);SPI_writedat(0x0B);		// Gamma 52
	SPI_writecom(0xC6);SPI_writedat(0x05);		// Gamma 80
	SPI_writecom(0xC7);SPI_writedat(0x07);		// Gamma 108
	SPI_writecom(0xC8);SPI_writedat(0x03);		// Gamma 147
	SPI_writecom(0xC9);SPI_writedat(0x07);		// Gamma 175
	SPI_writecom(0xCA);SPI_writedat(0x0E);		// Gamma 203
	SPI_writecom(0xCB);SPI_writedat(0x04);		// Gamma 231
	SPI_writecom(0xCC);SPI_writedat(0x0C);	    // Gamma 239
	SPI_writecom(0xCD);SPI_writedat(0x1C);		// Gamma 247
	SPI_writecom(0xCE);SPI_writedat(0x0A);		// Gamma 251
	SPI_writecom(0xCF);SPI_writedat(0x00);		// Gamma 255
	
	//+++++++++++++++++++++++++++++++++++++++++++++++++++//
	//****************************************************************************//
	//****************************** Page 6 Command ******************************//
	//****************************************************************************//
	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x06);	// Change to Page 6
//	sunxi_lcd_delay_ms(30);
	SPI_writecom(0x00);SPI_writedat(0xA0);		//1
	SPI_writecom(0x01);SPI_writedat(0x05);		//2
	SPI_writecom(0x02);SPI_writedat(0x00);		//3
	SPI_writecom(0x03);SPI_writedat(0x00);		//4
	SPI_writecom(0x04);SPI_writedat(0x01);		//5
	SPI_writecom(0x05);SPI_writedat(0x01);		//6
	SPI_writecom(0x06);SPI_writedat(0x88);		//7
	SPI_writecom(0x07);SPI_writedat(0x04);		//8
	SPI_writecom(0x08);SPI_writedat(0x01);	
	SPI_writecom(0x09);SPI_writedat(0x90);	
	SPI_writecom(0x0A);SPI_writedat(0x03);	
	SPI_writecom(0x0B);SPI_writedat(0x01);
	SPI_writecom(0x0C);SPI_writedat(0x01);	
	SPI_writecom(0x0D);SPI_writedat(0x01);	
	SPI_writecom(0x0E);SPI_writedat(0x00);	
	SPI_writecom(0x0F);SPI_writedat(0x00);	
	SPI_writecom(0x10);SPI_writedat(0x55);	
	SPI_writecom(0x11);SPI_writedat(0x50);	
	SPI_writecom(0x12);SPI_writedat(0x01);	
	SPI_writecom(0x13);SPI_writedat(0x84);	
	SPI_writecom(0x14);SPI_writedat(0x83);	
	SPI_writecom(0x15);SPI_writedat(0xc0);	
	SPI_writecom(0x16);SPI_writedat(0x0B);	
	SPI_writecom(0x17);SPI_writedat(0x00);	
	SPI_writecom(0x18);SPI_writedat(0x00);	
	SPI_writecom(0x19);SPI_writedat(0x00);	
	SPI_writecom(0x1A);SPI_writedat(0x00);	
	SPI_writecom(0x1B);SPI_writedat(0x00);	
	SPI_writecom(0x1C);SPI_writedat(0x00);	
	SPI_writecom(0x1D);SPI_writedat(0x00);	

	SPI_writecom(0x20);SPI_writedat(0x01);	
	SPI_writecom(0x21);SPI_writedat(0x23);	
	SPI_writecom(0x22);SPI_writedat(0x45);	
	SPI_writecom(0x23);SPI_writedat(0x67);	
	SPI_writecom(0x24);SPI_writedat(0x01);	
	SPI_writecom(0x25);SPI_writedat(0x23);
	SPI_writecom(0x26);SPI_writedat(0x45);	
	SPI_writecom(0x27);SPI_writedat(0x67);	

	SPI_writecom(0x30);SPI_writedat(0x02);
	SPI_writecom(0x31);SPI_writedat(0x22);	
	SPI_writecom(0x32);SPI_writedat(0x11);	
	SPI_writecom(0x33);SPI_writedat(0xAA);
	SPI_writecom(0x34);SPI_writedat(0xBB);	
	SPI_writecom(0x35);SPI_writedat(0x66);	
	SPI_writecom(0x36);SPI_writedat(0x00);	
	SPI_writecom(0x37);SPI_writedat(0x22);	
	SPI_writecom(0x38);SPI_writedat(0x22);	
	SPI_writecom(0x39);SPI_writedat(0x22);	
	SPI_writecom(0x3A);SPI_writedat(0x22);	
	SPI_writecom(0x3B);SPI_writedat(0x22);	
	SPI_writecom(0x3C);SPI_writedat(0x22);	
	SPI_writecom(0x3D);SPI_writedat(0x22);	
	SPI_writecom(0x3E);SPI_writedat(0x22);
	SPI_writecom(0x3F);SPI_writedat(0x22);	
	SPI_writecom(0x40);SPI_writedat(0x22);	
	SPI_writecom(0x52);SPI_writedat(0x10);	
	SPI_writecom(0x53);SPI_writedat(0x10);	

	//****************************************************************************//
	//****************************** Page 7 Command ******************************//
	//****************************************************************************//
	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x07);		 // Change to Page 7
//	sunxi_lcd_delay_ms(30);
	SPI_writecom(0x18);SPI_writedat(0x1D);
	SPI_writecom(0x02);SPI_writedat(0x77);	

	//****************************************************************************//
	//****************************** Page 0 Command ******************************//
	//****************************************************************************//

	SPI_writecom(0xFF);SPI_writedat(0xFF);SPI_writedat(0x98);SPI_writedat(0x06);SPI_writedat(0x04);SPI_writedat(0x00);		 // Change to Page 0	
	sunxi_lcd_delay_ms(30);
	//SPI_writecom(0x11);	sunxi_lcd_delay_ms(120);	// Sleep-Out
	SPI_writecom(0x29);	sunxi_lcd_delay_ms(300);	// Display On
	printk("  display on \n");

	return;
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

__lcd_panel_t d500n3437v0_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "d500n3437v0",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
