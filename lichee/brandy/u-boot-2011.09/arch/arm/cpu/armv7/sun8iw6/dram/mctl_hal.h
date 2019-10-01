#ifndef   _MCTL_HAL_H
#define   _MCTL_HAL_H

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dram.h>

#ifdef DRAM_PRINK_ENABLE
#  define dram_dbg(fmt,args...)	printf(fmt ,##args)
#else
#  define dram_dbg(fmt,args...) //printk(fmt ,##args)
#endif


extern void dram_udelay (unsigned int n);
extern unsigned int mctl_init(void);
extern unsigned int mctl_init_dram(void);
extern unsigned int mctl_sys_init(__dram_para_t *para);
extern void auto_set_timing_para(__dram_para_t *para);
extern void auto_set_dram_para(__dram_para_t *para);
extern signed int init_DRAM(int type, __dram_para_t *para);
extern void mctl_com_init(__dram_para_t *para);
extern unsigned int mctl_soft_training(void);
extern unsigned int mdfs_dfs(unsigned int freq_jump,__dram_para_t *para);
extern unsigned int mdfs_cfs(unsigned int freq_jump,__dram_para_t *para);
#endif  //_MCTL_HAL_H
