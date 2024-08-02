#include "hardware.h"
//#include "options.h"
#include "EMAC\emac.h"
#include "EMAC\xtrap.h"
#include "EMAC\trap.h"
#include "FLASH\NandFlash.h"
#include "CRC\CRC16.h"
#include "hw_com.h"
//#include "CRC16_CCIT.h"
#include "MANCH\manch.h"
#include "SEGGER_RTT\SEGGER_RTT.h"
#include "PointerCRC.h"
#include "hw_com.h"
#include "MEM\mem.h"


#ifdef CPU_SAME53	
#elif defined(CPU_XMC48)
#endif

#define VERSION			0x0204

//#pragma O3
//#pragma Otime

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//const char build_date[] __attribute__((used)) = "\n" __DATE__ "\n" __TIME__ "\n";

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
u32 fps;

//inline u16 ReverseWord(u16 v) { __asm	{ rev16 v, v };	return v; }

//static void* eepromData = 0;
//static u16 eepromWriteLen = 0;
//static u16 eepromReadLen = 0;
//static u16 eepromStartAdr = 0;

//static MTB mtb;

//static u16 manBuf[16];

u16 manRcvData[10];
//u16 manTrmData[50];
static u16 manTrmBaud = 0;

u16 txbuf[128 + 512 + 16];


static u16 manReqWord = 0x3A00;
static u16 manReqMask = 0xFF00;
static u16 manReqWordNew = 0x3C00;

//static u16 numDevice = 0;
static u16 verDevice = VERSION;
static byte svCount = 0;
static u32	badVec = 0;
static u32	badVecLen = 0;
static u16	lastBadLen = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct MainVars // NonVolatileVars  
{
	u32 timeStamp;

	u16 numDevice;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static MainVars mv;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SaveMainParams()
{
	svCount = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetNumDevice(u16 num)
{
	mv.numDevice = num;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 GetNumDevice()
{
	return mv.numDevice;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 GetVersionDevice()
{
	return verDevice;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Response_0(u16 rw, MTB &mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 numDevice; u16 version; u16 temp; byte status; byte flags; RTC rtc; };

	Rsp &rsp = *((Rsp*)&txbuf);

	rsp.rw = rw;
	rsp.device = NandFlash_GetDeviceID();  
	rsp.session = NandFlash_Session_Get();	  
	rsp.rcvVec =  NandFlash_Vectors_Recieved_Get();
	rsp.rejVec = badVec;
	rsp.wrVec = NandFlash_Vectors_Saved_Get();
	rsp.errVec = NandFlash_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = NandFlash_Current_Adress_Get();
	rsp.temp = (GetDeviceTemp()+2)/4;
	rsp.status = NandFlash_Status();

	GetTime(&rsp.rtc);

	mtb.data1 = txbuf;
	mtb.len1 = sizeof(rsp)/2;
	mtb.data2 = 0;
	mtb.len2 = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_0(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_0(data[0], mtb);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_1(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_0(data[0], mtb);

	NandFlash_WriteEnable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_2(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_0(data[0], mtb);

	NandFlash_WriteDisable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_3(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	Response_0(data[0], mtb);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_4(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 5) return false;

	SetClock(*(RTC*)(&data[1]));

	Response_0(data[0], mtb);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(const u16 *buf, u16 len, MTB &mtb)
{
	if (buf == 0 || len == 0) return false;

	bool r = false;

	byte i = buf[0] & 0xFF;

	switch (i)
	{
		case 0x00: 	r = RequestMan_0(buf, len, mtb); break;
		case 0x01: 	r = RequestMan_1(buf, len, mtb); break;
		case 0x02: 	r = RequestMan_2(buf, len, mtb); break;
		case 0x03: 	r = RequestMan_3(buf, len, mtb); break;
		case 0x04: 	r = RequestMan_4(buf, len, mtb); break;
	};

	mtb.baud = manTrmBaud;

	return r;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_00(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWordNew & manReqMask) | 0x00;
	txbuf[1] = mv.numDevice;
	txbuf[2] = verDevice;

	mtb.data1 = txbuf;
	mtb.len1 = 3;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWordNew & manReqMask) | 0x10;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(const u16 *data, u16 len, MTB &mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 temp; byte status; byte flags; RTC rtc; };

	if (len != 1) return false;

	Rsp &rsp = *((Rsp*)&txbuf);

	rsp.rw = (manReqWordNew & manReqMask) | 0x20;
	rsp.device = NandFlash_GetDeviceID();  
	rsp.session = NandFlash_Session_Get();	  
	rsp.rcvVec =  NandFlash_Vectors_Recieved_Get();
	rsp.rejVec = badVec;
	rsp.wrVec = NandFlash_Vectors_Saved_Get();
	rsp.errVec = NandFlash_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = NandFlash_Current_Adress_Get();
	rsp.temp = (GetDeviceTemp()+5)/10;
	rsp.status = NandFlash_Status();

	GetTime(&rsp.rtc);

	mtb.data1 = txbuf;
	mtb.len1 = sizeof(rsp)/2;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_30(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 5) return false;

	SetClock(*(RTC*)(&data[1]));

	txbuf[0] = (manReqWordNew & manReqMask) | 0x30;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_31(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	NandFlash_WriteEnable();

	txbuf[0] = (manReqWordNew & manReqMask) | 0x31;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_32(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	NandFlash_WriteDisable();

	txbuf[0] = (manReqWordNew & manReqMask) | 0x32;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_33(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	// Erase all

	txbuf[0] = (manReqWordNew & manReqMask) | 0x33;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 3) return false;

	switch (data[1])
	{
		case 1:

			SetNumDevice(data[2]);

			break;

		case 2:

			manTrmBaud = data[2]-1; //SetTrmBoudRate(data[2]-1);

			break;

		default:

			return false;
	};

	txbuf[0] = (manReqWordNew & manReqMask) | 0x80;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWordNew & manReqMask) | 0x90;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(const u16 *data, u16 len, MTB &mtb)
{
	if (len != 1) return false;

	txbuf[0] = (manReqWordNew & manReqMask) | 0xF0;

	mtb.data1 = txbuf;
	mtb.len1 = 1;
	mtb.data2 = 0;
	mtb.len2 = 0;

	SaveMainParams();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestManNew(const u16 *buf, u16 len, MTB &mtb)
{
	if (buf == 0 || len == 0) return false;

	bool r = false;

	byte i = buf[0] & 0xFF;

	switch (i)
	{
		case 0x00: 	r = RequestMan_00(buf, len, mtb); break;
		case 0x10: 	r = RequestMan_10(buf, len, mtb); break;
		case 0x20: 	r = RequestMan_20(buf, len, mtb); break;
		case 0x30: 	r = RequestMan_30(buf, len, mtb); break;
		case 0x31: 	r = RequestMan_31(buf, len, mtb); break;
		case 0x32: 	r = RequestMan_32(buf, len, mtb); break;
		case 0x33: 	r = RequestMan_33(buf, len, mtb); break;
		case 0x80: 	r = RequestMan_80(buf, len, mtb); break;
		case 0x90: 	r = RequestMan_90(buf, len, mtb); break;
		case 0xF0: 	r = RequestMan_F0(buf, len, mtb); break;
	};

	mtb.baud = manTrmBaud;

	return r;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

				if (mrb.OK && mrb.len > 0)
				{
					if ((manRcvData[0] & manReqMask) == manReqWord)
					{
						i = (RequestMan(manRcvData, mrb.len, mtb)) ? (i+1) : 0;
					}
					else if ((manRcvData[0] & manReqMask) == manReqWordNew)
					{
						i = (RequestManNew(manRcvData, mrb.len, mtb)) ? (i+1) : 0;
					}
					else
					{
						i = 0;
					};
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

			if (tm.Check(US2RT(500)))
			{
				//SendManData(&mtb);
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

static void InitMainVars()
{
	mv.numDevice		= 11111;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "Init Main Vars Vars ... OK\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadVars()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "Load Vars ... ");

	static DSCI2C dsc;
	static DSCSPI spi;
	static u16 romAdr = 0;
	
	byte buf[sizeof(mv)*2+4];

	MainVars mv1, mv2;

	bool c1 = false, c2 = false;

	bool loadVarsOk = false;

	spi.adr = (ReverseDword(FRAM_SPI_MAINVARS_ADR) & ~0xFF) | 3;
	spi.alen = 4;
	spi.csnum = 1;
	spi.wdata = 0;
	spi.wlen = 0;
	spi.rdata = buf;
	spi.rlen = sizeof(buf);

	if (SPI_AddRequest(&spi))
	{
		while (!spi.ready) { SPI_Update(); };
	};

	PointerCRC p(buf);

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&mv1, sizeof(mv1));
		p.ReadW();

		if (p.CRC.w == 0) { c1 = true; break; };
	};

	romAdr = ReverseWord(FRAM_I2C_MAINVARS_ADR);

	dsc.wdata = &romAdr;
	dsc.wlen = sizeof(romAdr);
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;
	dsc.rdata = buf;
	dsc.rlen = sizeof(buf);
	dsc.adr = 0x50;


	if (I2C_AddRequest(&dsc))
	{
		while (!dsc.ready) { I2C_Update(); };
	};

	p.b = buf;

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&mv2, sizeof(mv2));
		p.ReadW();

		if (p.CRC.w == 0) { c2 = true; break; };
	};

	SEGGER_RTT_WriteString(0, "FRAM SPI - "); SEGGER_RTT_WriteString(0, (c1) ? (RTT_CTRL_TEXT_BRIGHT_GREEN "OK") : (RTT_CTRL_TEXT_BRIGHT_RED "ERROR"));

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE " ... FRAM I2C - "); SEGGER_RTT_WriteString(0, (c2) ? (RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n") : (RTT_CTRL_TEXT_BRIGHT_RED "ERROR\n"));

	if (c1 && c2)
	{
		if (mv1.timeStamp > mv2.timeStamp)
		{
			c2 = false;
		}
		else if (mv1.timeStamp < mv2.timeStamp)
		{
			c1 = false;
		};
	};

	if (c1)	{ mv = mv1; } else if (c2) { mv = mv2; };

	loadVarsOk = c1 || c2;

	if (!c1 || !c2)
	{
		if (!loadVarsOk) InitMainVars();

		svCount = 2;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	static DSCI2C dsc;
	static DSCSPI spi,spi2;
	static u16 romAdr = 0;
	static byte buf[sizeof(mv) * 2 + 8];

	static byte i = 0;
	static TM32 tm;

	PointerCRC p(buf);

	switch (i)
	{
		case 0:

			if (svCount > 0)
			{
				svCount--;
				i++;
			};

			break;

		case 1:

			mv.timeStamp = GetMilliseconds();

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&mv, sizeof(mv));
				p.WriteW(p.CRC.w);
			};

			spi.adr = (ReverseDword(FRAM_SPI_MAINVARS_ADR) & ~0xFF) | 2;
			spi.alen = 4;
			spi.csnum = 1;
			spi.wdata = buf;
			spi.wlen = p.b-buf;
			spi.rdata = 0;
			spi.rlen = 0;

			romAdr = ReverseWord(FRAM_I2C_MAINVARS_ADR);

			dsc.wdata = &romAdr;
			dsc.wlen = sizeof(romAdr);
			dsc.wdata2 = buf;
			dsc.wlen2 = p.b-buf;
			dsc.rdata = 0;
			dsc.rlen = 0;
			dsc.adr = 0x50;

			spi2.adr = 6;
			spi2.alen = 1;
			spi2.csnum = 1;
			spi2.wdata = 0;
			spi2.wlen = 0;
			spi2.rdata = 0;
			spi2.rlen = 0;

			tm.Reset();

			SPI_AddRequest(&spi2);

			i++;

			break;

		case 2:

			if (spi2.ready || tm.Check(200))
			{
				SPI_AddRequest(&spi);

				i++;
			};

			break;

		case 3:

			if (spi.ready || tm.Check(200))
			{
				I2C_AddRequest(&dsc);
				
				i++;
			};

			break;

		case 4:

			if (dsc.ready || tm.Check(100))
			{
				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool testWriteFlash = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqHdr
{
	byte	dst;
	byte 	src;
	byte 	cmd;
	byte 	crc;
	u16		device;
	u32		counter;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqData 
{ 
	u32 cnt; 
	
	__packed struct Control	{ u16 version; u32 count; /* счётчик 50Гц измерений */ i16 voltage, temperature; byte status, flags; u16 errors; } con;
	__packed struct Gen		{ u16 version; i16 voltage, current, offset, temperature;	byte status, flags;	u16 errors;	} gen;
	__packed struct Incl	{ u16 version, ax, ay,az,fx,fy,fz, az_correction, temperature_sensor;	i16 temperature; byte status,flags;	u16 errors;	} incl;
	__packed struct Zond	{ u16 version; i16 compensation, temperature; byte status, flags; u16 errors; } zond;
	__packed struct Driver	{ u16 version,voltage,current; i16 temperature; byte status, flags; u16 errors;} driver;
	__packed struct Comm	{ u16 version;	i16 radius[4];	i16 temperature;byte status;byte flags;	u16 errors;	} comm[2];
	__packed struct Electrode {	u16 version; i16 current[24]; i16 phase[24]; i16 temperature; byte status; byte flags; u16 errors; } elect[8];
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req
{
	ReqHdr	hdr;
	ReqData rqd;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp
{
	byte	adr;
	byte	func;
	
	__packed union
	{
		__packed struct  { word crc; } f1;  // Старт новой сессии
		__packed struct  { word crc; } f2;  // Запись вектора
		__packed struct  { word crc; } f3;  // 
		__packed struct  { word crc; } fFE;  // Ошибка CRC
		__packed struct  { word crc; } fFF;  // Неправильный запрос
	};
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Rsp rspData;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte GetRequestCRC_old(byte *s, u32 len)
{
	byte crc = 0;

	while (len > 0)	{ crc += *(s++); len--;	};

	return crc;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma push
#pragma O3
#pragma Otime

static byte GetRequestCRC(void *s, u32 len)
{
	byte crc = 0;

	//u32 n = (~((u32)s))&3;

	//while (n > 0) { crc += *(s++); n--;	len--;};

	DataPointer p(s);

	u32 n = len >> 2; len &= 3;

	u32 crc32 = 0;

	while (n > 0) { crc32 = __uadd8(crc32, *(p.d++)); n--; };

	while (len > 0)	{ crc += *(p.b++); len--;	};

	crc += (byte)crc32; crc32 >>= 8;
	crc += (byte)crc32; crc32 >>= 8;
	crc += (byte)crc32; crc32 >>= 8;
	crc += (byte)crc32; 

	return crc;
}

#pragma pop

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static bool CreateRsp01(ComPort::WriteBuffer *wb)
//{
//	static Rsp rsp;
//
//	if (wb == 0)
//	{
//		return false;
//	};
//
//	rsp.adr = 1;
//	rsp.func = 1;
//	rsp.f1.crc = GetCRC16(&rsp, sizeof(rsp)-2);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	return true;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRsp02(ComPort::WriteBuffer *wb)
{
	static trap_memory_control_main_type rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.header.dst = DEVICE_CONTROL;
	rsp.header.src = DEVICE_MEMORY;
	rsp.header.cmd = DEVICE_COMMAND_MAIN;
	rsp.header.crc = 0;

	rsp.memory_data.version = GetVersionDevice();
	rsp.memory_data.serial = GetNumDevice();
	rsp.memory_data.temperature = GetDeviceTemp();
	rsp.memory_data.status = 0;
	rsp.memory_data.errors = 0;

	rsp.header.crc = GetRequestCRC(&rsp, sizeof(rsp));

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static bool CreateRsp03(ComPort::WriteBuffer *wb)
//{
//	static Rsp rsp;
//
//	if (wb == 0)
//	{
//		return false;
//	};
//
//	rsp.adr = 1;
//	rsp.func = 3;
//	rsp.f3.crc = GetCRC16(&rsp, sizeof(rsp)-2);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	return true;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static bool CreateRspErrCRC(ComPort::WriteBuffer *wb)
//{
//	static Rsp rsp;
//
//	if (wb == 0)
//	{
//		return false;
//	};
//
//	rsp.adr = 1;
//	rsp.func = 0xFE;
//	rsp.fFE.crc = GetCRC16(&rsp, sizeof(rsp)-2);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	return true;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static bool CreateRspErrReq(ComPort::WriteBuffer *wb)
//{
//	static Rsp rsp;
//
//	if (wb == 0)
//	{
//		return false;
//	};
//
//	rsp.adr = 1;
//	rsp.func = 0xFF;
//	rsp.fFF.crc = GetCRC16(&rsp, sizeof(rsp)-2);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	return true;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
static bool RequestFunc02(FLWB *fwb, ComPort::WriteBuffer *wb)
{
	VecData &vd = fwb->vd;

//	Req &req = *((Req*)vd.data);

	//byte n = vd.data[0] & 7;

	//vecCount[n] += 1;

	if (!RequestFlashWrite(fwb))
	{
		freeFlWrBuf.Add(fwb);
//		return false;
	};


	return CreateRsp02(wb);
}
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc(Ptr<MB> &mb, ComPort::WriteBuffer *wb)
{
	bool result = false;

	if (mb.Valid())
	{
		Req &req = *((Req*)mb->GetDataPtr());

		req.hdr.crc = -req.hdr.crc;

		if (GetRequestCRC(&req, mb->len) == 0) // (req.rw & 0xFF00) == 0xAA00) // 
		{
			mb->dataOffset += sizeof(req.hdr);
			mb->len -= sizeof(req.hdr);

			if (NandFlash_RequestWrite(mb, req.hdr.device, true))
			{
				result = CreateRsp02(wb);
			}
		}
		else
		{
//			HW::PIOA->BSET(27);

			if (mb->len != 0x3DE) badVecLen++, lastBadLen = mb->len;

			badVec++;

//			HW::PIOA->BCLR(27);
		};
	};

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
static void RequestTestWrite(FLWB *fwb)
{
	static u32 pt = 0;

	if (fwb == 0)
	{
		return;
	};

	u32 t = GetMilliseconds();

	if (t == pt)
	{
		freeFlWrBuf.Add(fwb);

		return;
	};

	pt = t;

	static byte nr = 0;
	static byte nf = 0;
	static u32 count = 0;

	VecData &vd = fwb->vd;

	Req &req = *((Req*)vd.data);

	vd.h.device = 0xEC00;
	req.cnt = count;

	count += 1;

	fwb->dataLen = sizeof(req);

//	req.crc = GetCRC16(fwb->vd.data, fwb->dataLen - 2);
	req.crc = CRC_CCITT_DMA(fwb->vd.data, fwb->dataLen - 2, 0xFFFF);
	
	RequestFlashWrite(fwb);
}
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
//	__packed struct Req { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[512*4]; };

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static byte state = 0;

	static Ptr<MB> mb;
//	static Req *req;
	//static VecData *vd;

	static u32 count = 0;
	static u16 v = 0;
	static byte b = 0;

	static TM32 tm;

	switch (state)
	{
		case 0:

			mb = NandFlash_AllocWB(sizeof(Req));

			if (mb.Valid())
			{
				//if (testWriteFlash)
				//{
				//	RequestTestWrite(fwb);
				//}
				//else
				{
					state++;
				};
			};

			break;

		case 1:

			rb.data = mb->GetDataPtr();
			rb.maxLen = mb->GetDataMaxLen();

			com0.Read(&rb, MS2COM(50), US2COM(200));

			state++;

			break;

		case 2:

			if (!com0.Update())
			{
				if (rb.recieved)
				{
					mb->len = rb.len;

					if (RequestFunc(mb, &wb))
					{
						state++;
					}
					else
					{
						state = 1;
					};

				}
				else
				{
					state = 1;
				};
			};

			break;

		case 3:

			com0.Write(&wb);

			state++;

			break;

		case 4:
			
			if (!com0.Update())
			{
				state = 0;
			};

			break;
	};

//	HW::PIOB->CODR = 1<<13;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateParams()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateEMAC()			);
		CALL( UpdateHardware()		);
		CALL( UpdateMan(); 			);
		CALL( NandFlash_Update();	);
		CALL( I2C_Update();			);
		CALL( SaveVars();			);
		CALL( UpdateCom();			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateParams();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	NAND_Idle();		
	
	if (EmacIsConnected())
	{
		UpdateEMAC();		
		UpdateTraps();	
	}
	else
	{
		UpdateParams();	
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53

	#define FPS_PIN_SET()	HW::PIOA->BSET(25)
	#define FPS_PIN_CLR()	HW::PIOA->BCLR(25)

#elif defined(CPU_XMC48)

	#define FPS_PIN_SET()	HW::P0->BSET(1)
	#define FPS_PIN_CLR()	HW::P0->BCLR(1)

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
//	__breakpoint(0);

	InitHardware();

	LoadVars();

	InitEMAC();

	//InitTraps();

	NandFlash_Init();

	com0.Connect(com0.SYNC_S, 2000000, 2, 2);

	u32 fc = 0;

	TM32 tm;

	tm.pt = 0;

	while (1)
	{
		FPS_PIN_SET();

		Update();

		FPS_PIN_CLR();

		fc++;

		if (tm.Check(1000))
		{ 
			fps = fc; fc = 0; 

			HW::ResetWDT();
		};

	}; // while (1)
}
