#ifndef __HT_RFKILL_H__
#define __HT_RFKILL_H__

#include <linux/rfkill.h>

struct ht_rfkill_platform_data {
	char				*name;
	enum rfkill_type	type;
	void				*data;
	int     (*set_power)(void *data, bool blocked);
	void	(*cleanup)(struct platform_device *pdev);
	int 	(*setup)(struct platform_device *pdev);
};

#endif  /* __HT_RFKILL_H__ */
