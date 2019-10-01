#ifndef   _MCTL_HAL_H
#define   _MCTL_HAL_H


#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dram.h>

#define  DRAM_PRINK_ENABLE

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	printf(fmt ,##args)
#else
#  define dram_dbg(fmt,args...)
#endif

#define DRAM_MAX_SIZE_MB		(MCTL_PAGE_SIZE*MCTL_BANK_SIZE*(0x1<<(MCTL_ROW_WIDTH-10))*MCTL_CHANNEL_NUM)  //in MB

extern unsigned int mctl_init(void);
extern unsigned int mctl_set_emrs(unsigned int emrs_id, unsigned int emrs_val);
extern unsigned int mctl_scan_readpipe(unsigned int clk);

//extern void mctl_self_refresh_entry(unsigned int channel_num);
extern void mctl_self_refresh_entry(unsigned int ch_index);
//extern void mctl_self_refresh_exit(unsigned int channel_num);
extern void mctl_self_refresh_exit(unsigned int ch_index);
extern void mctl_pad_hold(void);
extern void mctl_pad_release(void);
extern unsigned int mctl_sys_init_setfreq(unsigned int dram_freq);
extern unsigned int mctl_init_setreq(unsigned int dram_freq);
extern void mctl_power_down_entry(void);
extern void mctl_power_down_exit(void);
extern void mctl_precharge_all(void);
extern void mctl_deep_sleep_entry(void);
extern void mctl_deep_sleep_exit(void);

extern void mctl_setup_ar_interval(unsigned int clk);
extern void mctl_DLL_reset(void);
extern void mctl_DLL_enable(void);
extern void mctl_DLL_disable(void);
extern void mctl_hostport_control(unsigned int enable);
extern unsigned int mctl_init_dram(void);


extern void mctl_host_port_cfg(unsigned int port_no, unsigned int cfg);
extern void mctl_power_save_process(void);
extern unsigned int mctl_power_up_process(void);
extern unsigned int mctl_ahb_reset(void);

extern unsigned int mctl_sys_init(__dram_para_t *para);
extern void auto_set_timing_para(__dram_para_t *para);
extern signed int init_DRAM(int type, __dram_para_t *para);

extern unsigned int mctl_reset_release(void);

extern int dram_test_simple(void);

#endif  //_MCTL_HAL_H
