#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <mach/sys_config.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include "module_pm.h"
// ============= add PMU:GPIO --> wl_reg_on by sochi-hu 20150729  start===========================
//#defined PMU_GPIO_USBED;
//#ifdef PMU_GPIO_USBED
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>
#include <linux/kobject.h> 
#include <linux/miscdevice.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/delay.h>
#include <linux/ktime.h>

#define AXP20_GPIO0_CTL		0x90
#define AXP20_GPIO1_CTL		0x92
#define AXP20_GPIO2_CTL		0x93
#define AXP20_GPIO3_CTL		0x95
#define GPIO012_INPUT_STA		0x94

static struct i2c_client *global_client;
extern s32 i2c_smbus_write_byte_data(const struct i2c_client *client, u8 command, u8 value);

static int gpio1_used = 0;
static int gpio2_used = 0;
static int gpio3_used = 0;
struct regulator {
	struct device *dev;
	struct list_head list;
	int uA_load;
	int min_uV;
	int max_uV;
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
	struct dentry *debugfs;
};

static inline struct device *to_axp_dev(struct regulator_dev *rdev)
{
	return rdev_get_dev(rdev)->parent->parent;
}

void get_client(void){
    struct regulator *dcdc3;		
	struct device *dev;	
	struct axp_dev *chip;    
	dcdc3 = regulator_get(NULL, "vcc-wifi");
	dev = to_axp_dev(dcdc3->rdev);
	chip = dev_get_drvdata(dev);
	global_client = chip->client;
	//ret = i2c_smbus_write_byte_data(chip->client, POWER20_GPIO2_CTL, 0xc0);
	//ret = i2c_smbus_write_byte_data(chip->client, POWER20_GPIO3_CTL, 0xc0);
	/*printk("huangle mark i2c c5\n");
	if (ret < 0) {		
		printk("failed writings\n");
		return ret;	
	}
	printk("huangle mark i2c ===========\n");*/

}
//#endif
// ============= add PMU:GPIO --> wl_reg_on by sochi-hu 20150729  end===========================

static char *wifi_para = "wifi_para";
struct wl_func_info  wl_info;

extern int sunxi_gpio_req(struct gpio_config *gpio);
extern int get_rf_mod_type(void);
extern char * get_rf_mod_name(void);
extern int rf_module_power(int onoff);
extern int rf_pm_gpio_ctrl(char *name, int level);

#if 0
#define wifi_pm_msg(...)    do {printk("[wifi_pm]: "__VA_ARGS__);} while(0)
#else
#define wifi_pm_msg(...)    do {} while(0)
#endif

void wifi_pm_power(int on)
{
	int mod_num = get_rf_mod_type();
	int on_off = 0;

	if (on > 0){
		on_off = 1;
	} else {
		on_off = 0;
	}

	switch(mod_num){
		case 1:   /* rtl8188eu */
			rf_module_power(on_off);
			break;

		case 2:   /* rtl8723bs */
		case 3:   /* ap6181 */
		case 4:   /* ap6210 */
		case 5:   /* ap6330 */
		case 6:   /* ap6335 */
		case 7:   /* rtl8189etv */
			if (wl_info.wl_reg_on != -1) {
				wifi_pm_msg("set wl_reg_on %d !\n", on_off);
				gpio_set_value(wl_info.wl_reg_on, on_off);
			}else if(gpio1_used){
				printk("into i2c_smbus_write_byte_data\n");
				if(on_off == 1)
				{
//					if(i2c_smbus_write_byte_data(global_client, AXP20_GPIO1_CTL, 0xc1) < 0)	//0xc1:output high3.3v bu_qi_zuo_yong	
					if(i2c_smbus_write_byte_data(global_client, AXP20_GPIO1_CTL, 0xc7) < 0)	//0xc7:floating qi_zuo_yong
						printk("failed writing AXP20_GPIO1_CTL\n");
				}else{
					if(i2c_smbus_write_byte_data(global_client, AXP20_GPIO1_CTL, 0xc0) < 0)		
						printk("failed writing AXP20_GPIO1_CTL\n");
				}
			}	
			
			break;
		case 8:   /* xr819 */
			if (wl_info.wl_reg_on != -1) {
				wifi_pm_msg("set wl_reg_on %d !\n", on_off);
				gpio_set_value(wl_info.wl_reg_on, on_off);
				mdelay(30);
			}
			break;
		default:
			wifi_pm_msg("wrong module select %d !\n", mod_num);
	}

	wl_info.wl_power_state = on_off;
}
EXPORT_SYMBOL(wifi_pm_power);

int wifi_pm_get_mod_type(void)
{
	return get_rf_mod_type();
}
EXPORT_SYMBOL(wifi_pm_get_mod_type);

int wifi_pm_gpio_ctrl(char *name, int level)
{
	int i = 0;
	int gpio = 0;
	char * gpio_name[1] = {"wl_reg_on"};

	for (i = 0; i < 1; i++) {
		if (strcmp(name, gpio_name[i]) == 0) {
			switch (i)
			{
				case 0: /*wl_reg_on*/
					gpio = wl_info.wl_reg_on;
					break;
				default:
					wifi_pm_msg("no matched gpio.\n");
			}
			break;
		}
	}

	gpio_set_value(gpio, level);
	printk("gpio %s set val %d, act val %d\n", name, level, gpio_get_value(gpio));

	return 0;
}
EXPORT_SYMBOL(wifi_pm_gpio_ctrl);

#ifdef CONFIG_PROC_FS
static int wifi_pm_power_stat(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;

	p += sprintf(p, "%s : wl power state %s\n", wl_info.module_name, wl_info.wl_power_state ? "on" : "off");
	return p - page;
}

static int wifi_pm_power_ctrl(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	int power = simple_strtoul(buffer, NULL, 10);

	power = power ? 1 : 0;
	wifi_pm_power(power);
	return sizeof(power);
}

static inline void awwifi_procfs_attach(void)
{
	char proc_rootname[] = "driver/wifi-pm";
	struct wl_func_info *wl_info_p = &wl_info;

	wl_info_p->proc_root = proc_mkdir(proc_rootname, NULL);
	if (IS_ERR(wl_info_p->proc_root))
	{
		wifi_pm_msg("failed to create procfs \"driver/wifi-pm\".\n");
	}

	wl_info_p->proc_power = create_proc_entry("power", 0644, wl_info_p->proc_root);
	if (IS_ERR(wl_info_p->proc_power))
	{
		wifi_pm_msg("failed to create procfs \"power\".\n");
	}

	wl_info_p->proc_power->data = wl_info_p;
	wl_info_p->proc_power->read_proc = wifi_pm_power_stat;
	wl_info_p->proc_power->write_proc = wifi_pm_power_ctrl;
}

static inline void awwifi_procfs_remove(void)
{
	struct wl_func_info *wl_info_p = &wl_info;
	char proc_rootname[] = "driver/wifi-pm";

	remove_proc_entry("power", wl_info_p->proc_root);
	remove_proc_entry(proc_rootname, NULL);
}
#else
static inline void awwifi_procfs_attach(void) {}
static inline void awwifi_procfs_remove(void) {}
#endif

static int wifi_pm_get_res(void)
{
	script_item_value_type_e type;
	script_item_u val;
	struct gpio_config  *gpio_p = NULL;

	type = script_get_item(wifi_para, "wifi_used", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		wifi_pm_msg("failed to fetch wifi configuration!\n");
		return -1;
	}
	if (!val.val) {
		wifi_pm_msg("no wifi used in configuration\n");
		return -1;
	}
	wl_info.wifi_used = val.val;

	wl_info.wl_reg_on = -1;
	
	type = script_get_item(wifi_para, "wl_reg_on", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){
		wifi_pm_msg("get wl_reg_on gpio failed\n");
		type = script_get_item(wifi_para, "pmu_gpio1_used", &val);
		gpio1_used = val.val;
		mdelay(1);
		printk("[%s:%d]gpio1_used = %d\n",__func__,__LINE__,gpio1_used);

			if(gpio1_used == 1){
				get_client();
				printk("wl_reg_on used at PMU_GPIO1\n");
			}else
				printk("ERR:wl_reg_on bad!!!...\n");
	}
	else {
		gpio_p = &val.gpio;
		wl_info.wl_reg_on = gpio_p->gpio;
		sunxi_gpio_req(gpio_p);
	}

	return 0;
}

#include <linux/crypto.h>
#include <linux/scatterlist.h>

extern int sunxi_get_soc_chipid(uint8_t *chip_id);
void wifi_hwaddr_from_chipid(u8 *addr)
{
#define MD5_SIZE	16
#define CHIP_SIZE	16

	struct crypto_hash *tfm;
	struct hash_desc desc;
	struct scatterlist sg;
	u8 result[MD5_SIZE];
	u8 chipid[CHIP_SIZE];
	int i = 0;
	int ret = -1;

	memset(chipid, 0, sizeof(chipid));
	memset(result, 0, sizeof(result));

	sunxi_get_soc_chipid((u8 *)chipid);

	tfm = crypto_alloc_hash("md5", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		pr_err("Failed to alloc md5\n");
		return;
	}
	desc.tfm = tfm;
	desc.flags = 0;

	ret = crypto_hash_init(&desc);
	if (ret < 0) {
		pr_err("crypto_hash_init() failed\n");
		goto out;
	}

	sg_init_one(&sg, chipid, sizeof(chipid) - 1);
	ret = crypto_hash_update(&desc, &sg, sizeof(chipid) - 1);
	if (ret < 0) {
		pr_err("crypto_hash_update() failed for id\n");
		goto out;
	}

	crypto_hash_final(&desc, result);
	if (ret < 0) {
		pr_err("crypto_hash_final() failed for result\n");
		goto out;
	}

	/* Choose md5 result's [0][2][4][6][8][10] byte as mac address */
	for (i = 0; i < 6; i++) {
		addr[i] = result[2*i];
	}
	addr[0] &= 0xfe;     /* clear multicast bit */
	addr[0] &= 0xfd;     /* clear local assignment bit (IEEE802) */
	addr[5] ^= 0x01;     /* be different with ethernet mac address */

out:
	crypto_free_hash(tfm);
}
EXPORT_SYMBOL(wifi_hwaddr_from_chipid);


static int __devinit wifi_pm_probe(struct platform_device *pdev)
{
	awwifi_procfs_attach();
	wifi_pm_msg("wifi gpio init is OK !!\n");
	return 0;
}

static int __devexit wifi_pm_remove(struct platform_device *pdev)
{
	awwifi_procfs_remove();
	wifi_pm_msg("wifi gpio is released !!\n");
	return 0;
}

#ifdef CONFIG_PM
static int wifi_pm_suspend(struct device *dev)
{
	return 0;
}

static int wifi_pm_resume(struct device *dev)
{
	return 0;
}

static struct dev_pm_ops wifi_dev_pm_ops = {
	.suspend	= wifi_pm_suspend,
	.resume		= wifi_pm_resume,
};
#endif

static struct platform_device wifi_pm_dev = {
	.name           = "wifi_pm",
};

static struct platform_driver wifi_pm_driver = {
	.driver.name    = "wifi_pm",
	.driver.owner   = THIS_MODULE,
#ifdef CONFIG_PM
	.driver.pm      = &wifi_dev_pm_ops,
#endif
	.probe          = wifi_pm_probe,
	.remove         = __devexit_p(wifi_pm_remove),
};

static int __init wifi_pm_init(void)
{
	memset(&wl_info, 0, sizeof(struct wl_func_info));
	wifi_pm_get_res();
	if (!wl_info.wifi_used)
		return 0;

	wl_info.module_name = get_rf_mod_name();
	wl_info.wl_power_state = 0;

	platform_device_register(&wifi_pm_dev);
	return platform_driver_register(&wifi_pm_driver);
}

static void __exit wifi_pm_exit(void)
{
	if (!wl_info.wifi_used)
		return;

	platform_driver_unregister(&wifi_pm_driver);
	platform_device_unregister(&wifi_pm_dev);
}

late_initcall_sync(wifi_pm_init);
module_exit(wifi_pm_exit);
