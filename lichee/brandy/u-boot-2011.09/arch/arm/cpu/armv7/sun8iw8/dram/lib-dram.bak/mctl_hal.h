#ifndef   _MCTL_HAL_H   
#define   _MCTL_HAL_H
#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dram.h>
#define pattern_goto(...)
#define DRAM_PRINK_ENABLE
#ifdef DRAM_PRINK_ENABLE
#define dram_dbg(fmt,args...)	printf(fmt ,##args)
#else
#define dram_dbg(fmt,args...)
#endif
#define DRAM_MEM_BASE_ADDR	0x40000000
#define DRAM_RET_OK		0
#define DRAM_RET_FAIL	1


extern void dramc_bit_align(__dram_para_t *para);
extern void dram_udelay (unsigned int n);
extern unsigned int mctl_init(void);
extern unsigned int mctl_init_dram(void);
extern unsigned int mctl_sys_init(__dram_para_t *para);
extern void auto_set_timing_para(__dram_para_t *para);
extern void auto_set_dram_para(__dram_para_t *para);
extern signed int init_DRAM(int type, __dram_para_t *para);
extern void mctl_com_init(__dram_para_t *para);
#endif  //_MCTL_HAL_H
