#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
//#include "core.h"
#include "time.h"
#include "i2c.h"
#include "spi.h"
#include "hw_nand.h"
#include "hw_rtm.h"
#include "MANCH\manch.h"
#include "clock.h"



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//extern u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init);
//extern u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init);
//extern bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init);
//extern bool CRC_CCITT_DMA_CheckComplete(u16* crc);

extern void InitHardware();
extern void UpdateHardware();
extern void SetClock(const RTC &t);
extern i16 GetDeviceTemp();

#endif // HARDWARE_H__23_12_2013__11_37
