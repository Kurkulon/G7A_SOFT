#include "hardware.h"
#include "options.h"
#include "emac.h"
#include "xtrap.h"
#include "flash.h"
#include "CRC16.h"
#include "ComPort.h"
#include "CRC16_CCIT.h"

#ifdef CPU_SAME53	
#elif defined(CPU_XMC48)
#endif

#define VERSION			0x0301

//#pragma O3
//#pragma Otime

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const char build_date[] __attribute__((used)) = "\n" __DATE__ "\n" __TIME__ "\n";

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
u32 fps;
i16 temp = 0;
i16 cpu_temp = 0;

//inline u16 ReverseWord(u16 v) { __asm	{ rev16 v, v };	return v; }

static void* eepromData = 0;
static u16 eepromWriteLen = 0;
static u16 eepromReadLen = 0;
static u16 eepromStartAdr = 0;

static MTB mtb;

static u16 manBuf[16];

u16 manRcvData[10];
u16 manTrmData[50];

u16 txbuf[128 + 512 + 16];


static u16 manReqWord = 0x3A00;
static u16 manReqMask = 0xFF00;

static u16 numDevice = 0;
static u16 verDevice = VERSION;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Response_00(u16 rw, MTB &mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 numDevice; u16 version; u16 temp; byte status; byte flags; RTC rtc; };

	Rsp &rsp = *((Rsp*)&txbuf);

	rsp.rw = rw;
	rsp.device = temp*5/2;  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = cpu_temp;//temp*5/2;
	rsp.status = FLASH_Status();

	GetTime(&rsp.rtc);

	mtb.data = txbuf;
	mtb.len = sizeof(rsp)/2;
	mtb.next = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_00(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_00(data[0], mtb);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWord & manReqMask) | 0x10;

	mtb.data = txbuf;
	mtb.len = 1;
	mtb.next = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(const u16 *data, u16 len, MTB &mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 numDevice; u16 version; u16 temp; u16 status; u16 flags; RTC rtc; };

	if (len != 1) return false;

	Rsp &rsp = *((Rsp*)&txbuf);

	rsp.rw = (manReqWord & manReqMask) | 0x20;
	rsp.device = 0x3C00;  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = temp;
	rsp.status = FLASH_Status();

	GetTime(&rsp.rtc);

	mtb.data = txbuf;
	mtb.len = sizeof(rsp)/2;
	mtb.next = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_04(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 5) return false;

	SetClock(*(RTC*)(&data[1]));

	Response_00(data[0], mtb);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_01(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_00(data[0], mtb);

	FLASH_WriteEnable();

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_02(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_00(data[0], mtb);

	FLASH_WriteDisable();

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_03(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_00(data[0], mtb);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(const u16 *data, u16 len, MTB &mtb)
{
//	PORTB.OUTSET = PIN2_bm; PORTB.OUTCLR = PIN2_bm; 


	if (len != 3) return false;

//	PORTB.OUTSET = PIN2_bm; PORTB.OUTCLR = PIN2_bm; 

	switch (data[1])
	{
		case 1:

			//Options_Set_Serial(data[2]);

			break;

		case 2:

			//for (u16 j = 0; j < (data[2]+1); j++) { PORTB.OUTSET = PIN2_bm; PORTB.OUTCLR = PIN2_bm; };

			Options_Set_Telemetry_Baud_Rate(data[2]);

			SetTrmBoudRate(Options_Get_Telemetry_Baud_Rate()-1);

			break;

		default:

			return false;
	};

	txbuf[0] = (manReqWord & manReqMask) | 0x80;

	mtb.data = txbuf;
	mtb.len = 1;
	mtb.next = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWord & manReqMask) | 0x90;

	mtb.data = txbuf;
	mtb.len = 1;
	mtb.next = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWord & manReqMask) | 0xF0;

	mtb.data = txbuf;
	mtb.len = 1;
	mtb.next = 0;

	Options_Save();

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(const u16 *buf, u16 len, MTB &mtb)
{

	if (buf == 0 || len == 0) return false;

//	PORTB.OUTSET = PIN2_bm;

	bool r = false;

	byte i = buf[0] & 0xFF;

//	for (byte j = 0; j < (i+1); j++) { PORTB.OUTSET = PIN2_bm; PORTB.OUTCLR = PIN2_bm; };

	switch (i)
	{
		case 0x00: 	r = RequestMan_00(buf, len, mtb); break;
		case 0x01: 	r = RequestMan_01(buf, len, mtb); break;
		case 0x02: 	r = RequestMan_02(buf, len, mtb); break;
		case 0x03: 	r = RequestMan_03(buf, len, mtb); break;
		case 0x04: 	r = RequestMan_04(buf, len, mtb); break;
		
//		default:	bfURC++; 
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	static byte i = 0;

	static DSCI2C dsc, dsc2;

	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];

	static TM32 tm;

	switch (i)
	{
		case 0:

			if (tm.Check(100))
			{
				buf[0] = 0x11;

				dsc.adr = 0x68;
				dsc.wdata = buf;
				dsc.wlen = 1;
				dsc.rdata = &rbuf;
				dsc.rlen = 2;
				dsc.wdata2 = 0;
				dsc.wlen2 = 0;

				if (I2C_AddRequest(&dsc))
				{
					i++;
				};
			};

			break;

		case 1:

			if (dsc.ready)
			{
				i16 t = ((i16)ReverseWord(rbuf) + 32) / 64;

				if (t < (-60*4))
				{
					t += 256*4;
				};

				temp = t;

				i++;
			};

			break;

		case 2:

			buf[0] = 0x0E;
			buf[1] = 0x20;
			buf[2] = 0xC8;

			dsc2.adr = 0x68;
			dsc2.wdata = buf;
			dsc2.wlen = 3;
			dsc2.rdata = 0;
			dsc2.rlen = 0;
			dsc2.wdata2 = 0;
			dsc2.wlen2 = 0;

			if (I2C_AddRequest(&dsc2))
			{
				i++;
			};

			break;

		case 3:

			if (dsc2.ready)
			{
				//HW::SCU_GENERAL->DTSCON = SCU_GENERAL_DTSCON_START_Msk;

				i = 0; //i++;
			};

			break;

		//case 4:

		//	if (HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RDY_Msk)
		//	{
		//		cpu_temp = ((i32)(HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RESULT_Msk) - 605) * 1000 / 205;

		//		i = 0;
		//	};

		//	break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	static MTB mtb;
	static MRB mrb;

	static byte i = 0;

	static RTM tm;
//	static TM32 tm2;

	switch (i)
	{
		case 0:

			mrb.data = manRcvData;
			mrb.maxLen = 10;
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			ManRcvUpdate();

			if (mrb.ready)
			{
				tm.Reset();

				if (mrb.OK && mrb.len > 0 && (manRcvData[0] & manReqMask) == manReqWord && RequestMan(manRcvData, mrb.len, mtb))
				{
					i++;
				}
				else
				{
					i = 0;
				};
			}
			else if (mrb.len > 0)
			{

			};

			break;

		case 2:

			if (tm.Check(US2RT(100)))
			{
//				SetTrmBoudRate(3); /*mtb.data = tableCRC;*/ mtb.len = 5; SendMLT3(&mtb);
				SendManData(&mtb);

				i++;
			};

			break;

		case 3:

			if (mtb.ready)
			{
				i = 0;
			};

			break;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateParams()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateTemp()			);
		CALL( UpdateMan()			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateEMAC();		);
		CALL( FLASH_Update();	);
		CALL( UpdateParams();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( NAND_Idle();		);
		CALL( UpdateTraps();	);
		CALL( UpdateMisc();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53

	#define FPS_PIN_SET()	HW::PIOA->BSET(25)
	#define FPS_PIN_CLR()	HW::PIOA->BCLR(25)

#elif defined(CPU_XMC48)

	#define FPS_PIN_SET()	HW::P0->BSET(1)
	#define FPS_PIN_CLR()	HW::P0->BCLR(1)

#endif

//static ComPort com1;

int main()
{
	static bool c = true;

	static byte buf[100];

	volatile byte * const FLD = (byte*)0x60000000;	

//	RTM32 tm;
//	Dbt db(100);

//	__breakpoint(0);

	InitHardware();

	InitEMAC();

	InitTraps();

	FLASH_Init();

	u32 fc = 0;

	TM32 tm;
	RTM rtm;

	buf[0] = 0x01;
	buf[1] = 0x01;
	buf[2] = 0x01;
	buf[3] = 0x01;
	buf[4] = 0x01;
	buf[5] = 0x01;

	txbuf[0] = 0x0101;
	txbuf[1] = 0x0101;
	txbuf[2] = 0x0101;
	txbuf[3] = 0x0101;

//	ComPort::WriteBuffer wb;

//	com1.ConnectSync(0, 2000000, 2, 2);

	while (1)
	{
		FPS_PIN_SET();

		Update();

		//txbuf[2] = CRC_CCITT_DMA(txbuf, 2, 0xFFFF);
		//txbuf[3] = GetCRC16_CCIT(txbuf, 2, 0xFFFF);

		FPS_PIN_CLR();

		//CRC_CCITT_DMA(txbuf, 6, 0xFFFF);
		//GetCRC16_CCIT(txbuf, 6, 0xFFFF);


		//com1.TransmitByte(0x52);
		//com1.Update();



//		if (NAND_CheckDataComplete())
//		{
//			NAND_Chip_Select(0);
//			NAND_CmdWritePage(0, 0, 0);
//			NAND_WriteDataDMA(txbuf, 43);
////			NAND_CopyDataDMA(txbuf, txbuf+1, 1000);
//		};

		//*FLD = 0;
		//*FLD = 0;
		//*FLD = 0;
		//*FLD = 0;

		fc++;

		if (tm.Check(1000))
		{ 
			fps = fc; fc = 0; 

			mtb.next = 0;
			mtb.data = txbuf;
			mtb.len = 1;
			//SetTrmBoudRate(0);
//			SendManData(&mtb);
		};

		if (rtm.Check(US2RT(500)))
		{
//			HW::P1->BTGL(3);
		};

	}; // while (1)
}
