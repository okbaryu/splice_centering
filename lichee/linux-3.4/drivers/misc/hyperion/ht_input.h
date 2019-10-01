#ifndef __HT_INPUT_H__
#define __HT_INPUT_H__

struct ht_touchpanel {
	unsigned int	up_threshold;
	unsigned long	report_period_ms;

	bool (*is_down)(void *context);
	int  (*read_sample)(void *context);
	void (*report_down)(void *context);
	void (*report_sample)(void *context);
	void (*report_up)(void *context);
};

int ht_touchpanel_loop_run(const struct ht_touchpanel *tp, void *context);

#endif /* __HT_INPUT_H__ */
