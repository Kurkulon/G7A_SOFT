#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
//#include "core.h"
#include "time.h"


#define NAND_MAX_CHIP		2
#define NAND_CHIP_MASK		1
#define NAND_CHIP_BITS		1
#define NAND_COL_BITS		11
#define NAND_BLOCK_BITS		13
#define NAND_PAGE_BITS		6
#define NAND_RAWPAGE_MASK	((1 << (NAND_PAGE_BITS + NAND_CHIP_BITS + NAND_BLOCK_BITS)) - 1)
#define NAND_RAWBLOCK_MASK	((1 << (NAND_CHIP_BITS + NAND_BLOCK_BITS)) - 1)
#define NAND_RAWADR_MASK	(((u64)1 << (NAND_COL_BITS + NAND_PAGE_BITS + NAND_CHIP_BITS + NAND_BLOCK_BITS)) - 1)


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct NandMemSize
{
 	u64 ch;	// chip
 	u64 fl;	// full
 	u32 bl;	// block
//	u32 row;
	u16 pg;	// page
	u16 mask;
	byte shPg; //(1 << x)
	byte shBl; //(1 << x)
	byte shCh;
	//byte shRow;

	byte bitCol;
	byte bitPage; // 
	byte bitBlock;

	u16	pagesInBlock;


	u16		maskPage;
	u32		maskBlock;

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool NAND_BUSY(); 
extern bool NAND_CmdBusy();
extern void NAND_WriteDataDMA(volatile void *src, u16 len);
extern void NAND_WriteDataPIO(volatile void *src, u16 len);
extern void NAND_ReadDataDMA(volatile void *dst, u16 len);
extern void NAND_ReadDataDMA2(volatile void *dst, u16 len);
extern void NAND_ReadDataPIO(volatile void *dst, u16 len);
extern bool NAND_CheckDataComplete();
extern void NAND_Chip_Select(byte chip);
extern void NAND_Chip_Disable();
extern void NAND_WRITE(byte data);
extern void NAND_CopyDataDMA(volatile void *src, volatile void *dst, u16 len);

extern void NAND_CmdEraseBlock(u32 bl);
extern void NAND_CmdRandomRead(u16 col);
extern void NAND_CmdReadPage(u16 col, u32 bl, u16 pg);
extern void NAND_CmdWritePage(u16 col, u32 bl, u16 pg);
extern void NAND_CmdWritePage2();
extern byte NAND_CmdReadStatus();

inline const NandMemSize* NAND_GetMemSize() { extern NandMemSize nandSize; return &nandSize; } 


extern void Hardware_Init();

extern void UpdateHardware();

extern u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init);
extern u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct DSCI2C
{
	DSCI2C*			next;
	void*			wdata;
	void*			rdata;
	void*			wdata2;
	u16				wlen;
	u16				wlen2;
	u16				rlen;
	u16				readedLen;
	byte			adr;
	volatile bool	ready;
	volatile bool	ack;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//extern bool Write_I2C(DSCI2C *d);
//inline bool Read_I2C(DSCI2C *d) { return Write_I2C(d); }
//extern bool Check_I2C_ready();
extern bool I2C_AddRequest(DSCI2C *d);
extern void I2C_Update();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MRB
{
	bool	ready;
	bool	OK;
	u16		len;
	u16		maxLen;
	u16		*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MTB
{
	MTB*		next;
	const u16	*data;
	u16			len;
	bool		ready;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool RcvManData(MRB *mrb);
extern bool SendManData(MTB *mtb);
extern void SetTrmBoudRate(byte i);
extern void ManRcvUpdate();
//extern void ManRcvStop();

extern bool SendMLT3(MTB *mtb);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void InitHardware();
extern void UpdateHardware();
extern void SetClock(const RTC &t);

#endif // HARDWARE_H__23_12_2013__11_37
