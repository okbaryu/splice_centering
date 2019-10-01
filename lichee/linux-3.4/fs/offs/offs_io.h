#ifndef _OFFS_FS
#define _OFFS_FS

#include <linux/types.h>

struct offs_data {
	__u32 sector;
	__u32 count;
	__u8 *buffer;
};

#define OFFS_IO_MAGIC          'o'
#define OFFS_IO_WRITE           _IOW(OFFS_IO_MAGIC, 3, struct offs_data)
#define OFFS_IO_READ            _IOR(OFFS_IO_MAGIC, 4, struct offs_data)
#define OFFS_IO_GET_SECTOR_COUNT _IOR(OFFS_IO_MAGIC, 5, int)


#endif /* _OFFS_FS */
