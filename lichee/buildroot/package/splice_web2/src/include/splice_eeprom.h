typedef struct EEPROM{
	char EEP_BELT_tip_direction;
	char EEP_BELT_wid_check;
	char 
}EEPROM;

typedef struct R_REGISTER{
	unsigned int MWidth;
	unsigned int SWidthIn;
	unsigned int OffsetIn;
	unsigned int GetSWidth;
	unsigned int TolPos;
	unsigned int TolNeg;
	unsigned int SWidthOut;
	unsigned int OffsetOut;
	unsigned int P_Offset;
	unsigned int M_Offset;
	unsigned int Leading_Offset_Enable;
	unsigned int Trailing_Offset_Enable;
	unsigned int Leading_Offset[7];
	unsigned int Trailing_Offset[7]
	unsigned int 
}R_REGISTER;