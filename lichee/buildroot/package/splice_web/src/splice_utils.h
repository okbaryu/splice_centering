#ifndef __SPLICE_UTILS_H__
#define __SPLICE_UTILS_H__

#define SPLICE_NUM_MEMC 1
#define SPLICE_MAX_SUPPORTED_CLIENTS 10
#define SATA_USB_OUTPUT_FILE           "satausb.txt"
#define MAX_LINE_LENGTH                512
#define TEMP_FILE_FULL_PATH_LEN        64
#define SPLICE_SATA_USB_MAX          12 /* maximum number of devices like sda1, sda2, sda3, sda4, sdb1, sdc1 */
#define TRUE_OR_FALSE(value)           ((value)?"True":"False")
#define MAX_LENGTH_CPU_STATUS 128

#define IFCONFIG_UTILITY "/sbin/ifconfig"

#define UNUSED(expr) do { (void)(expr); } while (0)

/* command */
typedef enum
{
    SPLICE_CMD_GET_OVERALL_STATS = 0x001, /* over all stats for all the memcs and memory bw consumers, sorted list */
    SPLICE_CMD_START_SATA_USB = 0x002,
    SPLICE_CMD_STOP_SATA_USB = 0x004,
    SPLICE_CMD_QUIT = 0x008,
    SPLICE_CMD_GET_CPU_IRQ_INFO = 0x010,
	SPLICE_CMD_GET_ACTUATOR_STATUS = 0x020,
	SPLICE_CMD_SET_ACTUATOR_STATUS = 0x040,
	SPLICE_CMD_GET_ACTUATOR_POSITION = 0x080,
	SPLICE_CMD_SET_ACTUATOR_POSITION = 0x100,
	SPLICE_CMD_GET_RREGISTER_PLC = 0x200,
    SPLICE_CMD_MAX = 0xFFFF
} splice_cmd;

typedef enum
{
    SPLICE_COLUMN_ID = 1,
    SPLICE_COLUMN_NAME,
    SPLICE_COLUMN_BW,
    SPLICE_COLUMN_MAX
} splice_columnTypes;

typedef struct  splice_cmd_overall_stats_data
{
    unsigned int dummy;
} splice_cmd_overall_stats_data;

typedef struct splice_cmd_get_client_stats_data
{
    unsigned int         client_list[SPLICE_NUM_MEMC][SPLICE_MAX_SUPPORTED_CLIENTS]; /* SPLICE_CGI_INVALID indicates no client id is desired */
    splice_columnTypes sortColumn[SPLICE_NUM_MEMC];
} splice_cmd_get_client_stats_data;

typedef struct splice_cmd_set_client_rts
{
    unsigned int client_id;
    unsigned int memc_index;
    unsigned int rr;        /** If not set then this should be SPLICE_CGI_INVALID **/
    unsigned int block_out; /** If not set then this should be SPLICE_CGI_INVALID **/
} splice_cmd_set_client_rts;

typedef struct splice_request
{
    splice_cmd             cmd;
    splice_cmd             cmdSecondary;
    int        cmdSecondaryOption;
    int                      boxmode;
    //splice_boxmode_sources source;
    union {
        splice_cmd_overall_stats_data    overall_stats_data;
        splice_cmd_get_client_stats_data client_stats_data;
        splice_cmd_set_client_rts        client_rts_setting;
        char                               strCmdLine[256];
    } request;
} splice_request;

/* response */
typedef struct splice_client_data
{
    unsigned int  client_id;
    unsigned int  bw;
    unsigned int  rr;
    unsigned int  block_out;
    bool          is_detailed; /* true if this client id has details being accumulated for it */
    unsigned char err_cnt;
} splice_client_data;

typedef struct splice_system_stats
{
    /** Normal mode overall service data **/
    unsigned int dataBW;        /** In Mbps **/
    unsigned int transactionBW; /** In Mbps **/
    unsigned int idleBW;        /** In Mbps **/
    float        dataUtil;
    unsigned int ddrFreqMhz;
    unsigned int scbFreqMhz;
} splice_system_stats;

typedef struct
{
    float readMbps;
    float writeMbps;
    char  deviceName[32];
} splice_sata_usb_data;

typedef struct
{
    splice_cpu_percent   cpuData;
    splice_irq_data      irqData;
    splice_sata_usb_data sataUsbData[SPLICE_SATA_USB_MAX]; /* expect no more than 6 SATA and USB devices */
} splice_cpu_irq;

typedef struct
{
    bool isActive[SPLICE_MAX_NUM_CPUS];
} splice_cpu_status;

#define wl_bss_info_t_max_num 8
#define wl_bss_info_t_size    132

typedef struct splice_overall_stats
{
    splice_system_stats systemStats[SPLICE_NUM_MEMC];
    struct {
        splice_client_data clientData[SPLICE_MAX_NUM_CLIENT];
    } clientOverallStats[SPLICE_NUM_MEMC];
    splice_columnTypes sortColumn[SPLICE_NUM_MEMC];
    splice_cpu_percent cpuData;
    splice_irq_data    irqData;
    unsigned long int    contextSwitches;
    unsigned long int    fileSize;
    unsigned long int    pidCount;
    unsigned long int    ulWifiScanApCount;
    unsigned char        bssInfo[wl_bss_info_t_size*wl_bss_info_t_max_num]; /* CAD replace with real wl_bss_info_t structure */
} splice_overall_stats;

typedef struct splice_per_client_stats
{
    unsigned int clientId;
    unsigned int clientDataBW;
    unsigned int clientRdTransInPerc;
    unsigned int clientWrTransInPerc;
    unsigned int avgClientDataSizeInBits;
    float        clientDataUtil;
    unsigned int clientTransBW; /** In Mbps **/
} splice_per_client_stats;

typedef struct splice_client_stats
{
    struct {
        splice_per_client_stats perClientStats[SPLICE_MAX_SUPPORTED_CLIENTS];
    } clientStats[SPLICE_NUM_MEMC];
} splice_client_stats;

typedef enum
{
    PROFILE_ENV,
    PROFILE_RTS,
    PROFILE_BROWSER
} splice_profile_sources;

typedef struct
{
    int profile;
    splice_profile_sources source;
} splice_profile_source;

typedef struct splice_response
{
    char                     data[110]; /* used to make size of struct an even multiple of 256 */
    splice_cmd             cmd;
    splice_profile_sources source;
    unsigned long int        timestamp;
    union {
        splice_overall_stats overallStats;
        splice_client_stats  clientDetailStats;
        splice_cpu_irq       cpuIrqData; /* splice uses this structure to get cpu and irq information */
    } response;
} splice_response;

typedef struct
{
    unsigned char webMjVersion;
    unsigned char webMnVersion;
    unsigned char spliceMjVersion;
    unsigned char spliceMnVersion;
    unsigned char cameraMjVersion;
    unsigned char cameraMnVersion;
    unsigned char kernelVersion;
    unsigned int  sizeOfResponse;
    char          platform[8];
    char          platVersion[4];
} splice_version_info;

#endif
