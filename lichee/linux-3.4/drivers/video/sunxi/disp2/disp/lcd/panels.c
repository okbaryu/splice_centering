#include "panels.h"

struct sunxi_lcd_drv g_lcd_drv;

extern __lcd_panel_t default_panel;
extern __lcd_panel_t lt070me05000_panel;
extern __lcd_panel_t wtq05027d01_panel;
extern __lcd_panel_t t27p06_panel;
extern __lcd_panel_t st7789s_panel;
extern __lcd_panel_t ili9341_panel;
extern __lcd_panel_t vs043con08v0_panel;
extern __lcd_panel_t h043nl007001;
extern __lcd_panel_t gd9161_ips_panel;
extern __lcd_panel_t cog_t350mhhh_05p_panel;

__lcd_panel_t* panel_array[] = {
	&default_panel,
	&lt070me05000_panel,
	&wtq05027d01_panel,
	&t27p06_panel,
	&st7789s_panel,
	&ili9341_panel,
	&vs043con08v0_panel,
	&h043nl007001,
	&gd9161_ips_panel,
	&cog_t350mhhh_05p_panel,
	/* add new panel below */

	NULL,
};

static void lcd_set_panel_funs(void)
{
	int i;

	for(i=0; panel_array[i] != NULL; i++) {
		sunxi_lcd_set_panel_funs(panel_array[i]->name, &panel_array[i]->func);
	}

	return ;
}

int lcd_init(void)
{
	sunxi_disp_get_source_ops(&g_lcd_drv.src_ops);
	lcd_set_panel_funs();

	return 0;
}
