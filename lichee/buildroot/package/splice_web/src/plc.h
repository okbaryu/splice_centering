
#define PLC_PORT 6000

#define EEP_Data_Start_R_Register 8100
#define EEP_Width_R_Register 8170
#define EEP_Err_Bit_R_Register 8172
#define EEP_R_Register_Size 54
#define EEP_Scale_Num_for_PLC 10

#define MAX_PLC_IO_NUM 4
#define PLC_WR_SYSTEM_READY 0
#define PLC_WR_TIP_DETECT 1
#define PLC_WR_CENTERING 2
#define PLC_WR_COMM_ERROR 3
#define PLC_WR_RESET 4

#define PLC_RD_AUTO_MANUAL 0
#define PLC_RD_CPC_TO_EPC 1
#define PLC_RD_SAVE_WIDTH 2
#define PLC_RD_CHECK_CUTTING 3

typedef struct{
	long MWidth;
	long SWidthIn;
	long OffsetIn;
	long GetSWidth;
	long TolPos;
	long TolNeg;
	long SWidthOut;
	long OffsetOut;
	long POffset;
	long MOffset;
	long LeadingOffsetEnable;
	long TrailingOffsetEnable;
	long LeadingOffset[7];
	long TrailingOffset[7];
	long ActReset;
	float CPCStart;
}RRegister;

enum {
	PLC_ERR_DUST_DETECT = 0x1,
	PLC_ERR_WIDTH_ERROR = 0x2,
	PLC_ERR_COMM_ERROR  = 0x4,
	PLC_ERR_OFFSET_ERROR = 0x8,
	PLC_ERR_ACT_LIMIT   = 0x100,
	PLC_ERR_ACT_ORG_ERROR = 0x800,
	PLC_ERR_CUTTING_ERROR = 0x1000
};

enum {
	PLC_COMM_ACK = 0x1,
	PLC_COMM_NAK = 0x2,
	PLC_COMM_ERR = 0x3
};

enum {
	FROM_PLC_CENTERING_MANUAL = 0,
	FROM_PLC_CENTERING_AUTO   = 1
};

enum {
	FROM_PLC_SWITCH_TO_TRAILING_EPC_NOT_YET = 0,
	FROM_PLC_SWITCH_TO_TRAILING_EPC_START   = 1
};

enum {
	FROM_PLC_SAVE_WIDTH_AVG_STOP  = 0,
	FROM_PLC_SAVE_WIDTH_AVG_START = 1
};

enum {
	FROM_PLC_CHECK_CUTTING_ERROR_STOP = 0,
	FROM_PLC_CHECK_CUTTING_ERROR_START = 1
};

enum {
	TO_PLC_SYSTEM_NOT_READY = 0,
	TO_PLC_SYSTEM_READY     = 1
};

enum { /* Detect web and request conveyor stop */
	TO_PLC_WEB_DETECT_NOT_YET = 0,
	TO_PLC_WEB_DETECT = 1
};

enum { /* After leading tip centering finished, request conveyor start */
	TO_PLC_TIP_CENTERING = 0,
	TO_PLC_TIP_CENTERING_FINISHED = 1
};

enum {
	TO_PLC_COMM_NORMAL = 0,
	TO_PLC_COMM_ERROR  = 1
};

int plc_init(void);
int sendPlc(unsigned int r_head_address, char *head_data_address, char size, char readCmd);
void sendPlcIO(int data);

