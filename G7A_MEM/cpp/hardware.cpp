#include "types.h"
#include "core.h"
#include "time.h"
#include "COM_DEF.h"

#include "hardware.h"
#include "CRC16_8005.h"
//#include "CRC16.h"

//#pragma O3
//#pragma Otime

#ifdef CPU_SAME53	
#elif defined(CPU_XMC48)
#endif

i16 temp = 0;
i16 cpu_temp = 0;
i16 tempClock = 0;

#ifdef CPU_SAME53	

	// Test Pins
	// 3	- PC00	- EN_VCORE
	// 4	- PC01	- FCS3
	// 5	- PC02	- FCS4
	// 6	- PC03
	// 14	- PB07	- Clock_IRQ
	// 15	- PB08
	// 16	- PB09
	// 28	- PA10	- ManTrmIRQ2
	// 29	- PA11	- ManTrmIRQ2
	// 32	- PB10
	// 33	- PB11
	// 40	- PC10
	// 41	- PC11
	// 42	- PC12
	// 43	- PC13
	// 44	- PC14
	// 52	- PA16
	// 56	- PC16	- PHY_INT
	// 57	- PC17
	// 59	- PC19
	// 60	- PC20	- GRXDV
	// 61	- PC21
	// 64	- PB16
	// 65	- PB17	- GEN_1M
	// 66	- PB18	- ManRcvIRQ sync true
	// 68	- PB20	- I2C_Handler	
	// 69	- PB21	- ManTrmIRQ
	// 74	- PA24	- ManRcvIRQ
	// 75	- PA25	- main loop
	// 80	- PB24
	// 81	- PB25
	// 82	- PC24
	// 83	- PC25
	// 84	- PC26	- CRC_CCITT_DMA
	// 85	- PC27	- MEM USART READ
	// 86	- PC28	- MEM USART RTS	
	// 87	- PA27	- MEM USART CRC ERROR
	// 97	- PB00	- nc



	#define GEN_MCK		0
	#define GEN_32K		1
	#define GEN_25M		2
	#define GEN_1M		3
	//#define GEN_500K	4

	#define	NAND_DMACH				0
	#define	COM1_DMACH				1
	#define	COM2_DMACH				2
	#define	COM3_DMACH				3
	#define	NAND_MEMCOPY_DMACH		30
	#define	CRC_DMACH				31

	#define I2C			HW::I2C3
	#define PIO_I2C		HW::PIOA 
	#define PIN_SDA		22 
	#define PIN_SCL		23 
	#define SDA			(1<<PIN_SDA) 
	#define SCL			(1<<PIN_SCL) 

	__align(16) T_HW::DMADESC DmaTable[32];
	__align(16) T_HW::DMADESC DmaWRB[32];

	#define EVENT_NAND_1	0
	#define EVENT_NAND_2	1
	#define EVENT_NAND_3	2
	#define EVENT_MANR_1	3
	#define EVENT_MANR_2	4
	#define EVENT_MANR_3	5

	#define nandTC			HW::TC0
	//#define				HW::TC1
	//#define 				HW::TC2
	//#define 				HW::TC3
	//#define 				HW::TC4
	//#define 				HW::TC5
	//#define 				HW::TC6
	//#define 				HW::TC7

	#define nandTCC			HW::TCC0
	#define	ManTCC			HW::TCC4
	#define ManRT			HW::TCC2
	#define ManIT			HW::TCC3
	#define ManTT			HW::TCC4
	#define MltTmr			HW::TCC4
	#define MT(v)			(v)
	#define BOUD2CLK(x)		((u32)(1000000/x+0.5))
	#define US2MT(v)		(v)//((v)*(u64)MCK/2/1000000)

	#define MANT_IRQ		TCC4_0_IRQ
	#define MANR_IRQ		TCC2_1_IRQ
	#define MANR_EXTINT		3
	#define MANT_IRQ_2		TCC4_1_IRQ

	#define PIO_MANCH		HW::PIOB
	#define PIN_MEMTX1		31 
	#define PIN_MEMTX2		30 
	#define PIN_MANCHRX		19 
	#define PIN_TX1			10 
	#define PIN_TX2			11 

	#define MEMTX1			(1UL<<PIN_MEMTX1)
	#define MEMTX2			(1UL<<PIN_MEMTX2)
	#define MANCHRX			(1UL<<PIN_MANCHRX) 
	#define TX1				(1UL<<PIN_TX1)
	#define TX2				(1UL<<PIN_TX2)

	//#define ManRxd()		((PIO_MANCH->IN >> PIN_MANCHRX) & 1)

	#define Pin_ManRcvIRQ_Set()	HW::PIOA->BSET(24)
	#define Pin_ManRcvIRQ_Clr()	HW::PIOA->BCLR(24)

	#define Pin_ManTrmIRQ_Set()	HW::PIOB->BSET(21)		
	#define Pin_ManTrmIRQ_Clr()	HW::PIOB->BCLR(21)		

	#define Pin_ManRcvSync_Set()	HW::PIOB->BSET(18)		
	#define Pin_ManRcvSync_Clr()	HW::PIOB->BCLR(18)		

	#define PIO_NAND_DATA	HW::PIOA
	#define PIO_WP			HW::PIOB 
	#define PIO_ALE			HW::PIOB 
	#define PIO_CLE			HW::PIOB 
	#define PIO_WE_RE		HW::PIOA 
	#define PIO_RB			HW::PIOC
	#define PIO_FCS			HW::PIOC

	#define PIN_WE			8 
	#define PIN_RE			9 

	#define WE				(1<<PIN_WE) 
	#define RE				(1<<PIN_RE) 
	#define ALE				(1<<4) 
	#define CLE				(1<<5) 
	#define WP				(1<<6) 
	#define FCS1			(1<<5) 
	#define FCS2			(1<<6) 
	#define RB				(1<<7) 

	#define PIO_MEM_USART	HW::PIOB 
	#define PIN_MEM_TXD		1 
	#define PIN_MEM_RXD		2 
	#define PIN_MEM_SCK		3 
	#define MEM_TXD			(1<<PIN_MEM_TXD) 
	#define MEM_RXD			(1<<PIN_MEM_RXD) 
	#define MEM_SCK			(1<<PIN_MEM_SCK) 

	#define CLOCK_EXTINT	7
	#define CLOCK_IRQ		(EIC_0_IRQ+CLOCK_EXTINT)
	#define PIO_RTC			HW::PIOB
	#define PIN_RTCINT		23 
	#define RTCINT			(1UL<<PIN_RTCINT)

	#define EnableVcore()	HW::PIOC->BSET(0)


#elif defined(CPU_XMC48)

	// Test Pins
	// 1	- P0.1	- main loop
	// 2	- P0.0
	// 3	- P0.10
	// 4	- P0.9
	// 5	- P3.2
	// 40	- P2.15
	// 41	- P2.14
	// 55	- P5.7	- I2C_Handler
	// 57	- P5.1	- ManRcvIRQ
	// 70	- P1.13
	// 71	- P1.12
	// 72	- P1.11
	// 76	- P1.3	- ManTrmIRQ
	// 77	- P1.2	- MEM USART RTS	
	// 79	- P1.0	- MEM USART READ	
	// 80	- P1.9	- CRC_CCITT_DMA
	// 92	- P3.4
	// 94	- P0.11
	// 95	- P0.12
	// 96	- P0.6	- nc



	#define	NAND_DMA		HW::GPDMA0
	#define	NAND_DMACH		HW::GPDMA0_CH7
	#define	NAND_DMA_CHEN	(0x101<<7)
	#define	NAND_DMA_CHST	(1<<7)

	#define	CRC_DMA			HW::GPDMA1
	#define	CRC_DMACH		HW::GPDMA1_CH2
	#define	CRC_DMA_CHEN	(0x101<<2)
	#define	CRC_FCE			HW::FCE_KE3

	#define I2C				HW::USIC2_CH0
	#define PIO_I2C			HW::P5
	#define PIN_SDA			0 
	#define PIN_SCL			2 
	#define SDA				(1<<PIN_SDA) 
	#define SCL				(1<<PIN_SCL) 
	#define I2C_IRQ			USIC2_0_IRQn
	#define I2C_PID			PID_USIC2

	#define ManRT			HW::CCU40_CC43
	#define ManTT			HW::CCU40_CC40
	#define ManCCU			HW::CCU40
	#define ManCCU_PID		PID_CCU40
	#define ManTmr			HW::CCU40_CC41
	#define MT(v)			((u16)((MCK_MHz*(v)+64)/128))
	#define BOUD2CLK(x)		((u32)((MCK/8.0)/x+0.5))

	#define MANT_IRQ		CCU40_0_IRQn
	#define MANR_IRQ		CCU40_2_IRQn

	#define PIO_MANCH		HW::P1
	#define PIN_MEMTX1		7 
	#define PIN_MEMTX2		8 
//	#define PIN_MANCHRX		19 

	#define MEMTX1			(1UL<<PIN_MEMTX1)
	#define MEMTX2			(1UL<<PIN_MEMTX2)
//	#define MANCHRX			(1UL<<PIN_MANCHRX) 

	//#define ManRxd()		((PIO_MANCH->IN >> PIN_MANCHRX) & 1)

	#define Pin_ManRcvIRQ_Set()		HW::P5->BSET(1);
	#define Pin_ManRcvIRQ_Clr()		HW::P5->BCLR(1);

	#define Pin_ManTrmIRQ_Set()		HW::P1->SET(1<<3);
	#define Pin_ManTrmIRQ_Clr()		HW::P1->CLR(1<<3);

	#define Pin_ManRcvSync_Set()			
	#define Pin_ManRcvSync_Clr()			

	#define PIO_WP			HW::P3 
	#define PIO_RB			HW::P1
	#define PIO_FCS			HW::P4

	#define PIN_WP			3 
	#define PIN_FCS1		1 
	#define PIN_FCS2		0 
	#define PIN_RB			6 

	#define WP				(1<<PIN_WP) 
	#define FCS1			(1<<PIN_FCS1) 
	#define FCS2			(1<<PIN_FCS2) 
	#define RB				(1<<PIN_RB) 

	#define EnableVcore()	

	// SDA_0_0 P1.5
	// SCL_0_0 P0.8 P1.1
	 
	// SDA_0_1 P2.5 P3.13
	// SCL_0_1 P2.4 P3.0 P6.2

	// SDA_1_0 P0.5 P2.14
	// SCL_1_0 P0.11 P5.8

	// SDA_1_1 P3.15 P4.2
	// SCL_1_1 P0.10 P0.13

	// SDA_2_0 P5.0
	// SCL_2_0 P5.2 

	// SDA_2_1 P3.5 
	// SCL_2_1 P4.2 P3.6

	//ARM_IHP VectorTableInt[16] __attribute__((at(0x1FFE8000)));;
	//ARM_IHP VectorTableExt[112] __attribute__((at(0x1FFE8040)));;

	//ARM_IHP * const VectorTableInt = (ARM_IHP*)0x1FFE8000;
	//ARM_IHP * const VectorTableExt = (ARM_IHP*)0x1FFE8040;

	/*******************************************************************************
	 * MACROS
	 *******************************************************************************/
	#define	OFI_FREQUENCY        (24000000UL)  /**< 24MHz Backup Clock (fOFI) frequency. */
	#define OSI_FREQUENCY        (32768UL)    /**< 32KHz Internal Slow Clock source (fOSI) frequency. */  

	#define XMC4800_F144x2048

	#define CHIPID_LOC ((uint8_t *)0x20000000UL)

	#define PMU_FLASH_WS          (NS2CLK(30))	//(0x3U)

	#define OSCHP_FREQUENCY			(25000000U)
	#define FOSCREF					(2500000U)
	#define VCO_NOM					(400000000UL)
	#define VCO_IN_MAX				(5000000UL)

	#define DELAY_CNT_50US_50MHZ  (2500UL)
	#define DELAY_CNT_150US_50MHZ (7500UL)
	#define DELAY_CNT_50US_48MHZ  (2400UL)
	#define DELAY_CNT_50US_72MHZ  (3600UL)
	#define DELAY_CNT_50US_96MHZ  (4800UL)
	#define DELAY_CNT_50US_120MHZ (6000UL)
	#define DELAY_CNT_50US_144MHZ (7200UL)

	#define SCU_PLL_PLLSTAT_OSC_USABLE  (SCU_PLL_PLLSTAT_PLLHV_Msk | \
										 SCU_PLL_PLLSTAT_PLLLV_Msk | \
										 SCU_PLL_PLLSTAT_PLLSP_Msk)


	#define USB_PDIV (4U)
	#define USB_NDIV (79U)


	/*
	//    <o> Backup clock calibration mode
	//       <0=> Factory calibration
	//       <1=> Automatic calibration
	//    <i> Default: Automatic calibration
	*/
	#define FOFI_CALIBRATION_MODE 1
	#define FOFI_CALIBRATION_MODE_FACTORY 0
	#define FOFI_CALIBRATION_MODE_AUTOMATIC 1

	/*
	//    <o> Standby clock (fSTDBY) source selection
	//       <0=> Internal slow oscillator (32768Hz)
	//       <1=> External crystal (32768Hz)
	//    <i> Default: Internal slow oscillator (32768Hz)
	*/
	#define STDBY_CLOCK_SRC 0
	#define STDBY_CLOCK_SRC_OSI 0
	#define STDBY_CLOCK_SRC_OSCULP 1

	/*
	//    <o> PLL clock source selection
	//       <0=> External crystal
	//       <1=> Internal fast oscillator
	//    <i> Default: External crystal
	*/
	#define PLL_CLOCK_SRC 0
	#define PLL_CLOCK_SRC_EXT_XTAL 0
	#define PLL_CLOCK_SRC_OFI 1

	#define PLL_CON1(ndiv, k2div, pdiv) (((ndiv) << SCU_PLL_PLLCON1_NDIV_Pos) | ((k2div) << SCU_PLL_PLLCON1_K2DIV_Pos) | ((pdiv) << SCU_PLL_PLLCON1_PDIV_Pos))

	/* PLL settings, fPLL = 288MHz */
	#if PLL_CLOCK_SRC == PLL_CLOCK_SRC_EXT_XTAL

		#define PLL_K2DIV	((VCO_NOM/MCK)-1)
		#define PLL_PDIV	(((OSCHP_FREQUENCY-VCO_IN_MAX)*2/VCO_IN_MAX+1)/2)
		#define PLL_NDIV	((MCK*(PLL_K2DIV+1)*2/(OSCHP_FREQUENCY/(PLL_PDIV+1))+1)/2-1) // (7U) 

		#define VCO ((OSCHP_FREQUENCY / (PLL_PDIV + 1UL)) * (PLL_NDIV + 1UL))

	#else /* PLL_CLOCK_SRC == PLL_CLOCK_SRC_EXT_XTAL */

		#define PLL_PDIV (1U)
		#define PLL_NDIV (23U)
		#define PLL_K2DIV (0U)

		#define VCO ((OFI_FREQUENCY / (PLL_PDIV + 1UL)) * (PLL_NDIV + 1UL))

	#endif /* PLL_CLOCK_SRC == PLL_CLOCK_SRC_OFI */

	#define PLL_K2DIV_24MHZ  ((VCO / OFI_FREQUENCY) - 1UL)
	#define PLL_K2DIV_48MHZ  ((VCO / 48000000U) - 1UL)
	#define PLL_K2DIV_72MHZ  ((VCO / 72000000U) - 1UL)
	#define PLL_K2DIV_96MHZ  ((VCO / 96000000U) - 1UL)
	#define PLL_K2DIV_120MHZ ((VCO / 120000000U) - 1UL)

	#define SCU_CLK_CLKCLR_ENABLE_USBCLK SCU_CLK_CLKCLR_USBCDI_Msk
	#define SCU_CLK_CLKCLR_ENABLE_MMCCLK SCU_CLK_CLKCLR_MMCCDI_Msk
	#define SCU_CLK_CLKCLR_ENABLE_ETHCLK SCU_CLK_CLKCLR_ETH0CDI_Msk
	#define SCU_CLK_CLKCLR_ENABLE_EBUCLK SCU_CLK_CLKCLR_EBUCDI_Msk
	#define SCU_CLK_CLKCLR_ENABLE_CCUCLK SCU_CLK_CLKCLR_CCUCDI_Msk

	#define SCU_CLK_SYSCLKCR_SYSSEL_OFI      (0U << SCU_CLK_SYSCLKCR_SYSSEL_Pos)
	#define SCU_CLK_SYSCLKCR_SYSSEL_PLL      (1U << SCU_CLK_SYSCLKCR_SYSSEL_Pos)

	#define SCU_CLK_USBCLKCR_USBSEL_USBPLL   (0U << SCU_CLK_USBCLKCR_USBSEL_Pos)
	#define SCU_CLK_USBCLKCR_USBSEL_PLL      (1U << SCU_CLK_USBCLKCR_USBSEL_Pos)

	#define SCU_CLK_ECATCLKCR_ECATSEL_USBPLL (0U << SCU_CLK_ECATCLKCR_ECATSEL_Pos)
	#define SCU_CLK_ECATCLKCR_ECATSEL_PLL    (1U << SCU_CLK_ECATCLKCR_ECATSEL_Pos)

	#define SCU_CLK_WDTCLKCR_WDTSEL_OFI      (0U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)
	#define SCU_CLK_WDTCLKCR_WDTSEL_STANDBY  (1U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)
	#define SCU_CLK_WDTCLKCR_WDTSEL_PLL      (2U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)

	#define SCU_CLK_EXTCLKCR_ECKSEL_SYS      (0U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)
	#define SCU_CLK_EXTCLKCR_ECKSEL_USBPLL   (2U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)
	#define SCU_CLK_EXTCLKCR_ECKSEL_PLL      (3U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)

	#define EXTCLK_PIN_P0_8  (1)
	#define EXTCLK_PIN_P1_15 (2)

	#define __CLKSET    (0x00000000UL)
	#define __SYSCLKCR  (0x00010000UL)
	#define __CPUCLKCR  (0x00000000UL)
	#define __PBCLKCR   (0x00000000UL)
	#define __CCUCLKCR  (0x00000000UL)
	#define __WDTCLKCR  (0x00000000UL)
	#define __EBUCLKCR  (0x00000003UL)
	#define __USBCLKCR  (0x00010005UL)
	#define __ECATCLKCR (0x00000001UL)

	#define __EXTCLKCR (0x01200003UL)
	#define __EXTCLKPIN (0U)

	#define ENABLE_PLL \
		(((__SYSCLKCR & SCU_CLK_SYSCLKCR_SYSSEL_Msk) == SCU_CLK_SYSCLKCR_SYSSEL_PLL) || \
		 ((__ECATCLKCR & SCU_CLK_ECATCLKCR_ECATSEL_Msk) == SCU_CLK_ECATCLKCR_ECATSEL_PLL) || \
		 ((__CLKSET & SCU_CLK_CLKSET_EBUCEN_Msk) != 0) || \
		 (((__CLKSET & SCU_CLK_CLKSET_USBCEN_Msk) != 0) && ((__USBCLKCR & SCU_CLK_USBCLKCR_USBSEL_Msk) == SCU_CLK_USBCLKCR_USBSEL_PLL)) || \
		 (((__CLKSET & SCU_CLK_CLKSET_WDTCEN_Msk) != 0) && ((__WDTCLKCR & SCU_CLK_WDTCLKCR_WDTSEL_Msk) == SCU_CLK_WDTCLKCR_WDTSEL_PLL)))

	#define ENABLE_USBPLL \
		(((__ECATCLKCR & SCU_CLK_ECATCLKCR_ECATSEL_Msk) == SCU_CLK_ECATCLKCR_ECATSEL_USBPLL) || \
		 (((__CLKSET & SCU_CLK_CLKSET_USBCEN_Msk) != 0) && ((__USBCLKCR & SCU_CLK_USBCLKCR_USBSEL_Msk) == SCU_CLK_USBCLKCR_USBSEL_USBPLL)) || \
		 (((__CLKSET & SCU_CLK_CLKSET_MMCCEN_Msk) != 0) && ((__USBCLKCR & SCU_CLK_USBCLKCR_USBSEL_Msk) == SCU_CLK_USBCLKCR_USBSEL_USBPLL)))

	#define SLAD(v)		((v)&0xffff)                /*!< USIC_CH PCR_IICMode: SLAD (Bitfield-Mask: 0xffff)           */
	#define ACK00     	(1<<16UL)                    /*!< USIC_CH PCR_IICMode: ACK00 (Bit 16)                         */
	#define STIM      	(1<<17UL)                    /*!< USIC_CH PCR_IICMode: STIM (Bit 17)                          */
	#define SCRIEN    	(1<<18UL)                    /*!< USIC_CH PCR_IICMode: SCRIEN (Bit 18)                        */
	#define RSCRIEN   	(1<<19UL)                    /*!< USIC_CH PCR_IICMode: RSCRIEN (Bit 19)                       */
	#define PCRIEN    	(1<<20UL)                    /*!< USIC_CH PCR_IICMode: PCRIEN (Bit 20)                        */
	#define NACKIEN   	(1<<21UL)                    /*!< USIC_CH PCR_IICMode: NACKIEN (Bit 21)                       */
	#define ARLIEN    	(1<<22UL)                    /*!< USIC_CH PCR_IICMode: ARLIEN (Bit 22)                        */
	#define SRRIEN    	(1<<23UL)                    /*!< USIC_CH PCR_IICMode: SRRIEN (Bit 23)                        */
	#define ERRIEN    	(1<<24UL)                    /*!< USIC_CH PCR_IICMode: ERRIEN (Bit 24)                        */
	#define SACKDIS   	(1<<25UL)                    /*!< USIC_CH PCR_IICMode: SACKDIS (Bit 25)                       */
	#define HDEL(v)		(((v)&0xF)<<26UL)                    /*!< USIC_CH PCR_IICMode: HDEL (Bit 26)                          */
	#define ACKIEN    	(1<<30UL)                    /*!< USIC_CH PCR_IICMode: ACKIEN (Bit 30)                        */
	//#define MCLK      	(1<<31UL)                    /*!< USIC_CH PCR_IICMode: MCLK (Bit 31)                          */

	#define SLSEL         (0x1UL)                   /*!< USIC_CH PSR_IICMode: SLSEL (Bitfield-Mask: 0x01)            */
	#define WTDF          (0x2UL)                   /*!< USIC_CH PSR_IICMode: WTDF (Bitfield-Mask: 0x01)             */
	#define SCR           (0x4UL)                   /*!< USIC_CH PSR_IICMode: SCR (Bitfield-Mask: 0x01)              */
	#define RSCR          (0x8UL)                   /*!< USIC_CH PSR_IICMode: RSCR (Bitfield-Mask: 0x01)             */
	#define PCR           (0x10UL)                  /*!< USIC_CH PSR_IICMode: PCR (Bitfield-Mask: 0x01)              */
	#define NACK          (0x20UL)                  /*!< USIC_CH PSR_IICMode: NACK (Bitfield-Mask: 0x01)             */
	#define ARL           (0x40UL)                  /*!< USIC_CH PSR_IICMode: ARL (Bitfield-Mask: 0x01)              */
	#define SRR           (0x80UL)                  /*!< USIC_CH PSR_IICMode: SRR (Bitfield-Mask: 0x01)              */
	#define ERR           (0x100UL)                 /*!< USIC_CH PSR_IICMode: ERR (Bitfield-Mask: 0x01)              */
	#define ACK           (0x200UL)                 /*!< USIC_CH PSR_IICMode: ACK (Bitfield-Mask: 0x01)              */
	#define RSIF          (0x400UL)                 /*!< USIC_CH PSR_IICMode: RSIF (Bitfield-Mask: 0x01)             */
	#define DLIF          (0x800UL)                 /*!< USIC_CH PSR_IICMode: DLIF (Bitfield-Mask: 0x01)             */
	#define TSIF          (0x1000UL)                /*!< USIC_CH PSR_IICMode: TSIF (Bitfield-Mask: 0x01)             */
	#define TBIF          (0x2000UL)                /*!< USIC_CH PSR_IICMode: TBIF (Bitfield-Mask: 0x01)             */
	#define RIF           (0x4000UL)                /*!< USIC_CH PSR_IICMode: RIF (Bitfield-Mask: 0x01)              */
	#define AIF           (0x8000UL)                /*!< USIC_CH PSR_IICMode: AIF (Bitfield-Mask: 0x01)              */
	#define BRGIF         (0x10000UL)               /*!< USIC_CH PSR_IICMode: BRGIF (Bitfield-Mask: 0x01)            */

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define TDF_MASTER_SEND				(0U << 8U)
	#define TDF_SLAVE_SEND				(1U << 8U)
	#define TDF_MASTER_RECEIVE_ACK   	(2U << 8U)
	#define TDF_MASTER_RECEIVE_NACK  	(3U << 8U)
	#define TDF_MASTER_START         	(4U << 8U)
	#define TDF_MASTER_RESTART      	(5U << 8U)
	#define TDF_MASTER_STOP         	(6U << 8U)

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define __SCTR (SDIR(1) | TRM(3) | FLE(0x3F) | WLE(7))

	#define __CCR (MODE(4))

	#define __BRG (DCTQ(24))

	#define __DX0CR (DSEL(1) | INSW(0) | DFEN(1) | DSEN(1) | DPOL(0) | SFSEL(1) | CM(0) | DXS(0))
	#define __DX1CR (DSEL(0) | INSW(0) | DFEN(1) | DSEN(1) | DPOL(0) | SFSEL(1) | CM(0) | DXS(0))
	#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
	#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

	#define __PCR (STIM)

	#define __FDR ((1024 - (((MCK + 400000/2) / 400000 + 8) / 16)) | DM(1))

	#define __TCSR (TDEN(1)|TDSSM(1))


	static void delay(u32 cycles)
	{
	  for(volatile u32 i = 0UL; i < cycles ;++i)
	  {
		__nop();
	  }
	}

#endif


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void delay(u32 cycles) { for(volatile u32 i = 0UL; i < cycles ;++i) { __nop(); }}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/

extern "C" void SystemInit()
{
	//u32 i;
	using namespace CM4;
	using namespace HW;

//	__breakpoint(0);

	#ifdef CPU_SAME53	

		HW::PIOA->DIRSET = (1<<25)|(1<<24)|(1<<21)|(1<<27);
		HW::PIOA->CLR((1<<25)|(1<<24)|(1<<21)|(1<<27));

		PIO_MEM_USART->SetWRCONFIG(MEM_TXD|MEM_RXD|MEM_SCK, PORT_PMUX(3)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);

		HW::PIOB->DIRSET = (1<<18)|(1<<20)|(1<<21);
		//HW::PIOB->SetWRCONFIG((1<<17), PORT_PMUX(11)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX;

		HW::PIOC->DIRSET = (1<<15)|(1<<28)|(1<<27)|(1<<26)|(1<<25)|(1<<0);
		HW::PIOC->SET((1<<15)/*|(1<<0)*/);

		HW::PIOA->BCLR(25);
		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);

		OSCCTRL->XOSC[1] = XOSC_ENABLE|XOSC_ONDEMAND; // RUNSTDBY|ENABLE

		HW::PIOA->BSET(25);

		OSCCTRL->DPLL[0].CTRLA = 0; while ((OSCCTRL->DPLL[0].SYNCBUSY & DPLLSYNCBUSY_ENABLE) != 0);

		OSCCTRL->DPLL[0].CTRLB = DPLL_REFCLK_XOSC1|DPLL_DIV(24);	// 0x70010; // XOSC clock source division factor 50 = 2*(DIV+1), XOSC clock reference
		OSCCTRL->DPLL[0].RATIO = DPLL_LDR((MCK*2+500000)/1000000-1)|DPLL_LDRFRAC(0);	// 47; // Loop Divider Ratio = 200, Loop Divider Ratio Fractional Part = 0

		OSCCTRL->DPLL[0].CTRLA = DPLL_ONDEMAND|DPLL_ENABLE; 

		HW::PIOA->BCLR(25);

		HW::GCLK->GENCTRL[GEN_MCK] = GCLK_DIV(0)|GCLK_SRC_DPLL0|GCLK_GENEN;

		HW::PIOA->BSET(25);

		HW::MCLK->AHBMASK |= AHB_DMAC;
		HW::DMAC->CTRL = 0;
		HW::DMAC->CTRL = DMAC_SWRST;
		HW::DMAC->DBGCTRL = DMAC_DBGRUN;
		HW::DMAC->BASEADDR	= DmaTable;
		HW::DMAC->WRBADDR	= DmaWRB;
		HW::DMAC->CTRL = DMAC_DMAENABLE|DMAC_LVLEN0|DMAC_LVLEN1|DMAC_LVLEN2|DMAC_LVLEN3;

		HW::PIOA->BCLR(25);

		if ((CMCC->SR & CMCC_CSTS) == 0)
		{
			CMCC->CTRL = CMCC_CEN;
		};

		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);
		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);

	#elif defined(CPU_XMC48)

		__disable_irq();

//		__DSB();
		__enable_irq();

		HW::FLASH0->FCON = FLASH_FCON_IDLE_Msk | PMU_FLASH_WS;

		/* Automatic calibration uses the fSTDBY */

		/* Enable HIB domain */
		/* Power up HIB domain if and only if it is currently powered down */
		//if((SCU_POWER->PWRSTAT & SCU_POWER_PWRSTAT_HIBEN_Msk) == 0)
		//{
		//	SCU_POWER->PWRSET |= SCU_POWER_PWRSET_HIB_Msk;

		//	while((SCU_POWER->PWRSTAT & SCU_POWER_PWRSTAT_HIBEN_Msk) == 0) ;
		//};

		/* Remove the reset only if HIB domain were in a state of reset */
		//if((SCU_RESET->RSTSTAT) & SCU_RESET_RSTSTAT_HIBRS_Msk)
		//{
		//	SCU_RESET->RSTCLR |= SCU_RESET_RSTCLR_HIBRS_Msk;
		//	delay(DELAY_CNT_150US_50MHZ);
		//}

		/* Enable automatic calibration of internal fast oscillator */
		//SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_AOTREN_Msk;

		//delay(DELAY_CNT_50US_50MHZ);

		/* enable PLL */
		SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk);

		/* enable OSC_HP */
		//if ((SCU_OSC->OSCHPCTRL & SCU_OSC_OSCHPCTRL_MODE_Msk) != 0U)
		//{
		//	SCU_OSC->OSCHPCTRL &= ~(SCU_OSC_OSCHPCTRL_MODE_Msk | SCU_OSC_OSCHPCTRL_OSCVAL_Msk);
		SCU_OSC->OSCHPCTRL = OSC_MODE(2) | OSC_OSCVAL(OSCHP_FREQUENCY / FOSCREF - 1UL);

			/* select OSC_HP clock as PLL input */
			SCU_PLL->PLLCON2 = 0;

			/* restart OSC Watchdog */
			SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;

			while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_OSC_USABLE) != SCU_PLL_PLLSTAT_OSC_USABLE);
		//};

		/* Go to bypass the Main PLL */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;

		/* disconnect Oscillator from PLL */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;

		/* Setup divider settings for main PLL */
		SCU_PLL->PLLCON1 =  PLL_CON1(PLL_NDIV, PLL_K2DIV_24MHZ, PLL_PDIV);

		/* Set OSCDISCDIS */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

		/* connect Oscillator to PLL */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;

		/* restart PLL Lock detection */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;	while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk) == 0U);

		/* Disable bypass- put PLL clock back */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_VCOBYP_Msk;	while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOBYST_Msk) != 0U);

		/* Before scaling to final frequency we need to setup the clock dividers */
		SCU_CLK->SYSCLKCR = __SYSCLKCR;
		SCU_CLK->PBCLKCR = __PBCLKCR;
		SCU_CLK->CPUCLKCR = __CPUCLKCR;
		SCU_CLK->CCUCLKCR = __CCUCLKCR;
		SCU_CLK->WDTCLKCR = __WDTCLKCR;
		SCU_CLK->EBUCLKCR = __EBUCLKCR;
		SCU_CLK->USBCLKCR = __USBCLKCR;
		SCU_CLK->ECATCLKCR = __ECATCLKCR;
		SCU_CLK->EXTCLKCR = __EXTCLKCR;

		/* PLL frequency stepping...*/
		/* Reset OSCDISCDIS */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_48MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_48MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_72MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_72MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_96MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_96MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_120MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_120MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV, PLL_PDIV);			delay(DELAY_CNT_50US_144MHZ);

		/* Enable selected clocks */
		SCU_CLK->CLKSET = __CLKSET;

		//P2->ModePin10(	G_PP	);
		//P2->BSET(10);

		P0->ModePin0(	G_PP	);
		P0->ModePin1(	G_PP	);
		P0->ModePin2(	HWIO1	);
		P0->ModePin3(	HWIO1	);
		P0->ModePin4(	HWIO1	);
		P0->ModePin5(	HWIO1	);
		P0->ModePin6(	G_PP	);
		P0->ModePin7(	HWIO1	);
		P0->ModePin8(	HWIO1	);
		P0->ModePin9(	G_PP	);
		P0->ModePin10(	G_PP	);
		P0->ModePin11(	G_PP	);
		P0->ModePin12(	G_PP	);

		P0->PPS = 0;

		P1->ModePin0(	G_PP	);
		P1->ModePin1(	I2DPU	);
		P1->ModePin2(	G_PP	);
		P1->ModePin3(	G_PP	);
		P1->ModePin4(	I2DPU	);
		P1->ModePin5(	A2PP	);
		P1->ModePin6(	I2DPU	);
		P1->ModePin7(	G_PP	);
		P1->ModePin8(	G_PP	);
		P1->ModePin9(	G_PP	);
		P1->ModePin10(	I2DPU	);
		P1->ModePin11(	G_PP	);
		P1->ModePin12(	G_PP	);
		P1->ModePin13(	G_PP	);
		P1->ModePin14(	HWIO1	);
		P1->ModePin15(	HWIO1	);

		P1->PPS = 0;

		P2->ModePin0(	HWIO0	);
		P2->ModePin1(	I1DPD	);
		P2->ModePin2(	I2DPU	);
		P2->ModePin3(	I1DPD	);
		P2->ModePin4(	I1DPD	);
		P2->ModePin5(	A1PP	);
		P2->ModePin6(	I2DPU	);
		P2->ModePin7(	A1PP	);
		P2->ModePin8(	A1PP	);
		P2->ModePin9(	A1PP	);
		P2->ModePin10(	G_PP	);
		P2->ModePin14(	G_PP	);
		P2->ModePin15(	G_PP	);

		P2->PPS = 0;

		P3->ModePin0(	HWIO1	);
		P3->ModePin1(	HWIO1	);
		P3->ModePin2(	G_PP	);
		P3->ModePin3(	G_PP	);
		P3->ModePin4(	G_PP	);
		P3->ModePin5(	HWIO1	);
		P3->ModePin6(	HWIO1	);

		P3->PPS = 0;

		P4->ModePin0(	G_PP	);
		P4->ModePin1(	G_PP	);

		P4->PPS = 0;

		P5->ModePin0(	HWIO0	);
		P5->ModePin1(	G_PP	);
		P5->ModePin2(	A1OD	);
		P5->ModePin7(	G_PP	);

		P5->PPS = 0;

		P14->ModePin0(	I0DNP	);
		P14->ModePin1(	I2DPU	);
		P14->ModePin2(	I2DPU	);
		P14->ModePin3(	I2DPU	);
		P14->ModePin4(	I2DPU	);
		P14->ModePin5(	I2DPU	);
		P14->ModePin6(	I2DPU	);
		P14->ModePin7(	I2DPU	);
		P14->ModePin8(	I2DPU	);
		P14->ModePin9(	I2DPU	);
		P14->ModePin12(	I2DPU	);
		P14->ModePin13(	I2DPU	);
		P14->ModePin14(	I2DPU	);
		P14->ModePin15(	I2DPU	);

		P14->PPS = 0;
		P14->PDISC = (1<<0);

		P15->ModePin2(	I2DPU	);
		P15->ModePin3(	I2DPU	);
		P15->ModePin8(	I2DPU	);
		P15->ModePin9(	I1DPD	);

		P15->PPS = 0;
		P15->PDISC = 0;

		HW::Peripheral_Enable(PID_DMA0);
		HW::Peripheral_Enable(PID_DMA1);

		//HW::DLR->SRSEL0 = SRSEL0(10,11,0,0,0,0,0,0);
		//HW::DLR->SRSEL1 = SRSEL1(0,0,0,0);

		HW::DLR->DRL0 = DRL0_USIC0_SR0;
		HW::DLR->DRL1 = DRL1_USIC1_SR0;

		HW::DLR->LNEN |= 3;

	#endif

	#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
		CM4::SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
							(3UL << 11*2)  );               /* set CP11 Full Access */
	#else
		CM4::SCB->CPACR = 0;
	#endif

  /* Enable unaligned memory access - SCB_CCR.UNALIGN_TRP = 0 */
	CM4::SCB->CCR &= ~(SCB_CCR_UNALIGN_TRP_Msk);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define NAND_BASE_PIO_DATA 		AT91C_BASE_PIOA
//#define NAND_PIO_DATA_OFFSET	10
//#define NAND_PIO_DATA_MASK		0xFF
//#define NAND_READ_PACK_BYTES	512
//#define NAND_WRITE_PACK_BYTES	256

#define NAND_CMD_RESET				0xFF
#define NAND_CMD_READ_ID			0x90
#define NAND_CMD_READ_1				0x00
#define NAND_CMD_READ_2				0x30
#define NAND_CMD_RANDREAD_1			0x05
#define NAND_CMD_RANDREAD_2			0xE0
#define NAND_CMD_PAGE_PROGRAM_1		0x80
#define NAND_CMD_PAGE_PROGRAM_2		0x10
#define NAND_CMD_READ_STATUS		0x70
#define NAND_CMD_BLOCK_ERASE_1		0x60
#define NAND_CMD_BLOCK_ERASE_2		0xD0
#define NAND_CMD_READ_PARAM			0xEC
#define NAND_CMD_CHANGE_WRCOL		0x85
#define NAND_CMD_COPYBACK_PROGRAM	0x85
#define NAND_CMD_SET_FEATURES		0xEF
#define NAND_CMD_GET_FEATURES		0xEE

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NandMemSize nandSize;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NandID
{
 	byte marker;
 	byte device;
 	byte data[3];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NandParamPage
{
 	char	signature[4];
 	u16		revisionNumber;
 	u16		featuresSupported;
	u16		optionalCommandsSupported;
	byte	_rezerved1[22];
	char	deviceManufacturer[12];
	char	deviceModel[20];
	byte	JEDEC_manufacturer_ID;
	u16		dateCode;
	byte	_rezerved2[13];
	u32		numberDataBytesPerPage;
	u16		numberSpareBytesPerPage;
	byte	_rezerved3[6];
	u32		numberPagesPerBlock;
	u32		numberBlocksPerLUN;
	byte	numberLUNsPerChip;
	byte	numberAddressCycles;
	byte	numberBitsPerCell;
	u16		badBlocksMaximumPerLUN;
	u16		blockEndurans;
	byte	guaranteedValidBlocks;
	u16		blockEnduranceForGuaranteedValidBlocks;
	byte	numberProgramsPerPage;
	byte	_rezerved4[143];
	u16		integrityCRC;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NandParamPage nandParamPage[NAND_MAX_CHIP];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 chipSelect[2] = { FCS1, FCS2 };
#define maskChipSelect (FCS1|FCS2)

#ifdef CPU_SAME53	

	#define NAND_DIR_IN() { PIO_NAND_DATA->DIRCLR = 0xFF; }
	#define NAND_DIR_OUT() { PIO_NAND_DATA->DIRSET = 0xFF; }

#elif defined(CPU_XMC48)

	volatile byte * const FLC = (byte*)0x60000008;	
	volatile byte * const FLA = (byte*)0x60000010;	
	volatile byte * const FLD = (byte*)0x60000000;	

	#define NAND_DIR_IN() {}
	#define NAND_DIR_OUT() {}

#endif



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte NAND_READ()
{
	#ifdef CPU_SAME53	
		PIO_WE_RE->CLR(RE); 
		__nop(); __nop();
		byte v = PIO_NAND_DATA->IN; 
		PIO_WE_RE->SET(RE); 
		return v; 
	#elif defined(CPU_XMC48)
		return *FLD;
	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#pragma push
//#pragma O0

void NAND_WRITE(byte data)
{ 
	#ifdef CPU_SAME53	

		PIO_WE_RE->CLR(WE); 
		PIO_NAND_DATA->OUT8(data); 
		__nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop();
		//__nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop();
		PIO_WE_RE->SET(WE); 
		__nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop();
		//__nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop(); __nop();

	#elif defined(CPU_XMC48)

		*FLD = data;

	#endif
}       

//#pragma pop

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CMD_LATCH(byte cmd)
{ 
	#ifdef CPU_SAME53	

		PIO_CLE->SET(CLE); 
		PIO_ALE->CLR(ALE); 
		NAND_WRITE(cmd); 
		PIO_CLE->CLR(CLE|ALE); 

	#elif defined(CPU_XMC48)

		*FLC = cmd;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ADR_LATCH(byte cmd)
{
	#ifdef CPU_SAME53	

		PIO_ALE->SET(ALE); 
		PIO_CLE->CLR(CLE); 
		NAND_WRITE(cmd); 
		PIO_CLE->CLR(CLE|ALE); 

	#elif defined(CPU_XMC48)

		*FLA = cmd;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ADR_LATCH_COL(u16 col) 
{ 
	#ifdef CPU_SAME53	

		PIO_CLE->CLR(CLE); 
		PIO_ALE->SET(ALE); 
		NAND_WRITE(col); 
		NAND_WRITE(col>>8); 
		PIO_CLE->CLR(CLE|ALE); 

	#elif defined(CPU_XMC48)

		*FLA = col; *FLA = col >> 8;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ADR_LATCH_ROW(u32 row) 
{ 
	#ifdef CPU_SAME53	

		PIO_CLE->CLR(CLE); 
		PIO_ALE->SET(ALE); 
		NAND_WRITE(row); 
		NAND_WRITE(row>>8); 
		NAND_WRITE(row>>16); 
		PIO_CLE->CLR(CLE|ALE); 

	#elif defined(CPU_XMC48)

		*FLA = row; *FLA = row >> 8; *FLA = row >> 16;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ADR_LATCH_COL_ROW(u16 col, u32 row)
{ 
	#ifdef CPU_SAME53	

		PIO_CLE->CLR(CLE); 
		PIO_ALE->SET(ALE); 
		NAND_WRITE(col); 
		NAND_WRITE(col>>8); 
		NAND_WRITE(row); 
		NAND_WRITE(row>>8); 
		NAND_WRITE(row>>16); 
		PIO_CLE->CLR(CLE|ALE);

	#elif defined(CPU_XMC48)

		*FLA = col; *FLA = col >> 8;
		*FLA = row; *FLA = row >> 8; *FLA = row >> 16;

	#endif
} 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashReady()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) != 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashBusy()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) == 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_BUSY() 
{
	#ifdef CPU_SAME53	
		return ((PIO_RB->IN & RB) == 0);
	#elif defined(CPU_XMC48)
		return PIO_RB->TBCLR(PIN_RB);
	#endif
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void EnableWriteProtect()
{
	PIO_WP->CLR(WP);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void DisableWriteProtect()
{
	PIO_WP->SET(WP);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte NAND_CmdReadStatus()
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_READ_STATUS);
	NAND_DIR_IN();
	return NAND_READ();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_CmdBusy()
{
	return NAND_BUSY() || ((NAND_CmdReadStatus() & (1<<6)) == 0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Chip_Select(byte chip) 
{    
	if(chip < 2)                   
	{ 				
	#ifdef CPU_SAME53	
		PIO_WE_RE->SET(RE|WE); 
	#endif
		PIO_FCS->SET(maskChipSelect ^ chipSelect[chip]);
		PIO_FCS->CLR(chipSelect[chip]);
	};
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Chip_Disable() 
{    
	PIO_FCS->SET(maskChipSelect);

	#ifdef CPU_SAME53	
		NAND_DIR_IN();
		PIO_WE_RE->SET(RE|WE); 
	#endif
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ResetNand()
{
	while(NAND_BUSY());
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_RESET);
	NAND_CmdReadStatus();
	u32 count = 1000; while (!NAND_BUSY() && (count-- > 0));
	while(NAND_BUSY());
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_CheckDataComplete_old()
{
	#ifdef CPU_SAME53	
		return (HW::DMAC->CH[0].CTRLA & DMCH_ENABLE) == 0;
	#elif defined(CPU_XMC48)
		return (HW::GPDMA1->CHENREG & (1<<3)) == 0;
	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_CheckDataComplete()
{
	#ifdef CPU_SAME53

		if ((HW::DMAC->CH[NAND_DMACH].CTRLA & DMCH_ENABLE) == 0 || (HW::DMAC->CH[NAND_DMACH].INTFLAG & DMCH_TCMPL))
		{
			PIO_WE_RE->SET(WE|RE);
			PIO_WE_RE->DIRSET = WE|RE;
			PIO_WE_RE->PINCFG[PIN_RE] = PINGFG_DRVSTR;
			PIO_WE_RE->PINCFG[PIN_WE] = PINGFG_DRVSTR;

			return true;
		}
		else
		{
			return false;
		};
	
	#elif defined(CPU_XMC48)

		return (NAND_DMA->CHENREG & NAND_DMA_CHST) == 0;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_CheckCopyComplete()
{
	#ifdef CPU_SAME53

		return (HW::DMAC->CH[NAND_MEMCOPY_DMACH].CTRLA & DMCH_ENABLE) == 0 || (HW::DMAC->CH[NAND_MEMCOPY_DMACH].INTFLAG & DMCH_TCMPL);
	
	#elif defined(CPU_XMC48)

		return (NAND_DMA->CHENREG & NAND_DMA_CHST) == 0;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Set_Features(byte adr, byte p1, byte p2, byte p3, byte p4)
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_SET_FEATURES);
	ADR_LATCH(adr);
	NAND_WRITE(p1); 
	NAND_WRITE(p2); 
	NAND_WRITE(p3); 
	NAND_WRITE(p4); 
	while(!NAND_BUSY());
	while(NAND_BUSY());

	NAND_DIR_IN();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Get_Features(byte adr, byte* p)
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_GET_FEATURES);
	ADR_LATCH(adr);

	while(!NAND_BUSY());

	NAND_DIR_IN();

	while(NAND_BUSY());

	p[0] = NAND_READ(); 
	p[1] = NAND_READ(); 
	p[2] = NAND_READ(); 
	p[3] = NAND_READ(); 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool NAND_Read_ID(NandID *id)
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_READ_ID);
	ADR_LATCH(0);
	NAND_DIR_IN();

	NAND_ReadDataDMA(id, sizeof(NandID));

	while (!NAND_CheckDataComplete());

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Read_PARAM(NandParamPage *pp)
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_READ_PARAM);
	ADR_LATCH(0);
	NAND_DIR_IN();

	while(!NAND_BUSY());
	while(NAND_BUSY());
	
	NAND_ReadDataDMA(pp, sizeof(NandParamPage)); while (!NAND_CheckDataComplete());

	PIO_WE_RE->SET(RE); 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u32 NAND_ROW(u32 block, u16 page)
{
	return (block << NAND_PAGE_BITS) + page;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CmdEraseBlock(u32 bl)
{
	bl = NAND_ROW(bl, 0);
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_1);
	ADR_LATCH_ROW(bl);
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CmdRandomRead(u16 col)
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_RANDREAD_1);
	ADR_LATCH_COL(col);
	CMD_LATCH(NAND_CMD_RANDREAD_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CmdReadPage(u16 col, u32 bl, u16 pg)
{
	bl = NAND_ROW(bl, pg);
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_READ_1);
	ADR_LATCH_COL_ROW(col, bl);
	CMD_LATCH(NAND_CMD_READ_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CmdWritePage(u16 col, u32 bl, u16 pg)
{
	bl = NAND_ROW(bl, pg);
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_1);
	ADR_LATCH_COL_ROW(col, bl);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CmdWritePage2()
{
	NAND_DIR_OUT();
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Init()
{
	using namespace HW;

	byte p[4];

#ifdef CPU_SAME53

	HW::GCLK->PCHCTRL[GCLK_TCC0_TCC1] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	
	HW::MCLK->APBBMASK |= APBB_TCC0;

	HW::GCLK->PCHCTRL[EVENT_NAND_1+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[EVENT_NAND_2+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[EVENT_NAND_3+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	EVSYS->CH[EVENT_NAND_1].CHANNEL = EVGEN_DMAC_CH_0|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;
//	EVSYS->USER[EVSYS_USER_PORT_EV_1] = EVENT_NAND_1+1;
	EVSYS->USER[EVSYS_USER_TCC0_EV_1] = EVENT_NAND_1+1;

//	EVSYS->CH[EVENT_NAND_2].CHANNEL = EVGEN_TC0_MC_0|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;
//	EVSYS->USER[EVSYS_USER_PORT_EV_0] = EVENT_NAND_2+1;

//	EVSYS->CH[EVENT_NAND_3].CHANNEL = EVGEN_TC0_OVF|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;
//	EVSYS->USER[EVSYS_USER_PORT_EV_2] = EVENT_NAND_3+1;

	PIO_NAND_DATA->DIRSET = 0xFF;
	PIO_NAND_DATA->WRCONFIG = 0xFF|PORT_INEN|PORT_DRVSTR|PORT_WRPINCFG;
	PIO_NAND_DATA->CTRL |= 0xFF;
	PIO_FCS->DIRSET = FCS1|FCS2; PIO_FCS->SET(FCS1|FCS2);
	PIO_CLE->DIRSET = CLE; PIO_CLE->CLR(CLE);
	PIO_ALE->DIRSET = ALE; PIO_ALE->CLR(ALE);
	PIO_WE_RE->DIRSET = WE; PIO_WE_RE->SET(WE); PIO_WE_RE->WRCONFIG = WE|PORT_DRVSTR|PORT_WRPINCFG|PORT_PMUX(5)|PORT_WRPMUX/*|PORT_PMUXEN*/;
	PIO_WE_RE->DIRSET = RE; PIO_WE_RE->SET(RE); PIO_WE_RE->WRCONFIG = RE|PORT_DRVSTR|PORT_WRPINCFG|PORT_PMUX(5)|PORT_WRPMUX/*|PORT_PMUXEN*/;
	PIO_RB->DIRCLR = RB; PIO_RB->WRCONFIG = RB|PORT_INEN|PORT_PULLEN|PORT_WRPINCFG; PIO_RB->CTRL |= RB; PIO_RB->SET(RB);
	PIO_WP->DIRSET = WP; PIO_WP->SET(WP);

	nandTCC->CTRLA = 0;
	nandTCC->CTRLA = TCC_SWRST;
	while(nandTCC->SYNCBUSY & TCC_SWRST);

	nandTCC->CTRLA = 0;
	nandTCC->WAVE = TCC_WAVEGEN_NPWM|TCC_POL0;
	nandTCC->DRVCTRL = TCC_NRE0|TCC_NRE1|TCC_NRV0|TCC_NRV1;
	nandTCC->PER = 250;
	nandTCC->CC[0] = 2; 
	nandTCC->CC[1] = 2; 

	nandTCC->EVCTRL = TCC_OVFEO|TCC_MCEO0|TCC_TCEI0|TCC_EVACT0_RETRIGGER;

	nandTCC->CTRLBSET = TCC_ONESHOT;
	nandTCC->CTRLA = TCC_ENABLE;

	//PIO_WE_RE->EVCTRL.EV[0] = PIN_WE|PORT_PORTEI|PORT_EVACT_SET;
	//PIO_WE_RE->EVCTRL.EV[1] = PIN_WE|PORT_PORTEI|PORT_EVACT_CLR;

	NAND_DIR_OUT();

#elif defined(CPU_XMC48)

	HW::EBU_Enable(0);

	HW::Peripheral_Enable(PID_DMA0);

	NAND_DMA->DMACFGREG = 1;

	EBU->CLC = 0;
	EBU->MODCON = /*EBU_ARBSYNC|*/EBU_ARBMODE(3);
	EBU->USERCON = 0x3FF<<16;

	EBU->ADDRSEL0 = EBU_REGENAB/*|EBU_ALTENAB*/;

	EBU->BUSRCON0 = EBU_AGEN(4)|EBU_WAIT(0)|EBU_PORTW(1);
	EBU->BUSRAP0 = EBU_ADDRC(0)|EBU_CMDDELAY(0)|EBU_WAITRDC(NS2CLK(30))|EBU_DATAC(0)|EBU_RDRECOVC(NS2CLK(0))|EBU_RDDTACS(0);

	EBU->BUSWCON0 = EBU_LOCKCS|EBU_AGEN(4)|EBU_WAIT(0)|EBU_PORTW(1);

//				 = |			|				 |		tWP		 |			   |			   |				;
	EBU->BUSWAP0 = EBU_ADDRC(0)|EBU_CMDDELAY(0)|EBU_WAITWRC(NS2CLK(20))|EBU_DATAC(0)|EBU_WRRECOVC(NS2CLK(0))|EBU_WRDTACS(0);

#endif

	for(byte chip = 0; chip < NAND_MAX_CHIP; chip ++)
	{
		NAND_Chip_Select(chip);
		
		ResetNand();
		NandID id;
		NAND_Read_ID(&id);
		
		if((id.marker == 0xEC) && (id.device == 0xD3))
		{
			if (nandSize.shCh == 0)
			{
				nandSize.pg = 1 << (nandSize.bitCol = nandSize.shPg = ((id.data[1] >> 0) & 0x03) + 10);
				nandSize.bl = 1 << (nandSize.shBl = ((id.data[1] >> 4) & 0x03) + 16);
				nandSize.ch = 1 << (nandSize.shCh = (((id.data[2] >> 4) & 0x07) + 23) + (((id.data[2] >> 2) & 0x03) + 0));
//				nandSize.row = 1 << (nandSize.shRow = nandSize.shCh - nandSize.shPg);
				
				nandSize.pagesInBlock = 1 << (nandSize.bitPage = nandSize.shBl - nandSize.shPg);

				nandSize.maskPage = nandSize.pagesInBlock - 1;
				nandSize.maskBlock = (1 << (nandSize.bitBlock = nandSize.shCh - nandSize.shBl)) - 1;
			};
			
			nandSize.fl += nandSize.ch;
			
			nandSize.mask |= (1 << chip);
		}
		else if((id.marker == 0x2C) && (id.device == 0x68))
		{
			NAND_Set_Features(1, 5, 0, 0, 0);
			
			ResetNand();

			NAND_Get_Features(1, p);

			NandParamPage &np = nandParamPage[chip];

			NAND_Read_PARAM(&np);

			u16 crc = GetCRC16_8005_refl(&np, sizeof(np)-2, 0x4F4E);

			if (np.integrityCRC == crc/* || np.integrityCRC == 0xA61F*/)
			{
				if (nandSize.mask == 0)
				{
					nandSize.pg = np.numberDataBytesPerPage;
					nandSize.pagesInBlock = np.numberPagesPerBlock;
					nandSize.bl = np.numberBlocksPerLUN * np.numberLUNsPerChip;
					nandSize.ch = nandSize.pg * nandSize.pagesInBlock;
					nandSize.ch *= nandSize.bl;

					//nandSize.maskPage = nandSize.pagesInBlock - 1;
					//nandSize.maskBlock = 0;//(1 << (nandSize.bitBlock = nandSize.shCh - nandSize.shBl)) - 1;
				};
				
				nandSize.fl += nandSize.ch;
				
				nandSize.mask |= (1 << chip);
			};
		};

	};

	NAND_Chip_Disable();

	DisableWriteProtect();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53	

void NAND_WriteDataDMA_old(volatile void *src, u16 len)
{
	using namespace HW;

	nandTC->CTRLA = 0;
	nandTC->CTRLA = TC_SWRST;

	DmaTable[NAND_DMACH].SRCADDR = (byte*)src+len;
	DmaTable[NAND_DMACH].DSTADDR = &HW::PIOA->OUT;
	DmaTable[NAND_DMACH].DESCADDR = 0;
	DmaTable[NAND_DMACH].BTCNT = len;
	DmaTable[NAND_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC|DMDSC_EVOSEL_BEAT;

	DMAC->CH[NAND_DMACH].EVCTRL = DMCH_EVOE;
	DMAC->CH[NAND_DMACH].PRILVL = DMCH_PRILVL_LVL3;
	DMAC->CH[NAND_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|DMCH_TRIGSRC_TC0_MC0;

	PIO_WE_RE->EVCTRL.EV[0] = PIN_WE|PORT_PORTEI|PORT_EVACT_SET;
	PIO_WE_RE->EVCTRL.EV[1] = PIN_WE|PORT_PORTEI|PORT_EVACT_CLR;
	PIO_WE_RE->EVCTRL.EV[2] = PIN_WE|PORT_PORTEI|PORT_EVACT_SET;

	while(nandTC->SYNCBUSY & TC_SWRST);

	nandTC->CTRLA = TC_MODE_COUNT8;
	nandTC->WAVE = TC_WAVEGEN_NPWM;
	nandTC->PER8 = 250;
	nandTC->CC8[0] = 1; 

	NAND_DIR_OUT();

	nandTC->EVCTRL = TC_OVFEO|TC_MCEO0|TC_TCEI|TC_EVACT_RETRIGGER;

	nandTC->CTRLBSET = TC_ONESHOT;
	nandTC->CTRLA = TC_MODE_COUNT8|TC_ENABLE;

	DMAC->SWTRIGCTRL = 1;
}

#elif defined(CPU_XMC48)
#endif


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_WriteDataDMA(volatile void *src, u16 len)
{
	using namespace HW;

	#ifdef CPU_SAME53	

		nandTCC->CTRLA = 0;
		nandTCC->CTRLA = TC_SWRST;

		DmaTable[NAND_DMACH].SRCADDR = (byte*)src+len;
		DmaTable[NAND_DMACH].DSTADDR = &HW::PIOA->OUT;
		DmaTable[NAND_DMACH].DESCADDR = 0;
		DmaTable[NAND_DMACH].BTCNT = len;
		DmaTable[NAND_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC|DMDSC_EVOSEL_BEAT;

		DMAC->CH[NAND_DMACH].INTENCLR = ~0;
		DMAC->CH[NAND_DMACH].INTFLAG = ~0;
		DMAC->CH[NAND_DMACH].EVCTRL = DMCH_EVOE;
		DMAC->CH[NAND_DMACH].PRILVL = DMCH_PRILVL_LVL3;
		DMAC->CH[NAND_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|DMCH_TRIGSRC_TCC0_MC0;

		nandTCC->WAVE = TCC_WAVEGEN_NPWM|TCC_POL0;
		nandTCC->DRVCTRL = TCC_NRE0|TCC_NRE1|TCC_NRV0|TCC_NRV1;
		nandTCC->PER = 3;
		nandTCC->CC[0] = 2; 
		nandTCC->CC[1] = 2; 

		nandTCC->EVCTRL = TCC_OVFEO|TCC_MCEO0|TCC_TCEI1|TCC_EVACT1_RETRIGGER;

		NAND_DIR_OUT();
		PIO_WE_RE->SET(WE|RE); 
		PIO_WE_RE->DIRSET = WE|RE;
		PIO_WE_RE->PINCFG[PIN_WE] = PINGFG_PMUXEN|PINGFG_DRVSTR;

		nandTCC->CTRLBSET = TC_ONESHOT;
		nandTCC->CTRLA = TC_ENABLE;

		DMAC->SWTRIGCTRL = 1<<NAND_DMACH;

	#elif defined(CPU_XMC48)

		NAND_DMA->DMACFGREG = 1;

		NAND_DMACH->CTLL = DST_NOCHANGE|SRC_INC|TT_FC_M2M_GPDMA|DEST_MSIZE_8|SRC_MSIZE_8;
		NAND_DMACH->CTLH = BLOCK_TS(len);

		NAND_DMACH->SAR = (u32)src;
		NAND_DMACH->DAR = (u32)FLD;	//0x1FFE8000;//;
		NAND_DMACH->CFGL = 0;
		NAND_DMACH->CFGH = PROTCTL(1);

		NAND_DMA->CHENREG = NAND_DMA_CHEN;

//		NAND_WriteDataPIO(src, len);

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_WriteDataPIO(volatile void *src, u16 len)
{
	using namespace HW;

	byte* p = (byte*)src;

	#ifdef CPU_SAME53	

		PIO_WE_RE->PINCFG[PIN_WE] &= ~PINGFG_PMUXEN;
		PIO_WE_RE->PINCFG[PIN_RE] &= ~PINGFG_PMUXEN;

		NAND_DIR_OUT();

		while(len != 0) { NAND_WRITE(*(p++)); len--; };

	#elif defined(CPU_XMC48)

		while(len != 0) { *FLD = *(p++); len--; };

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CPU_SAME53	

void NAND_ReadDataDMA_old(volatile void *dst, u16 len)
{
	using namespace HW;

	nandTC->CTRLA = 0;
	nandTC->CTRLA = TC_SWRST;

	DmaTable[NAND_DMACH].SRCADDR = &PIO_NAND_DATA->IN;
	DmaTable[NAND_DMACH].DSTADDR = (byte*)dst+len;
	DmaTable[NAND_DMACH].DESCADDR = 0;
	DmaTable[NAND_DMACH].BTCNT = len;
	DmaTable[NAND_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC|DMDSC_EVOSEL_BEAT;

	DMAC->CH[NAND_DMACH].EVCTRL = DMCH_EVOE;
	DMAC->CH[NAND_DMACH].PRILVL = DMCH_PRILVL_LVL3;
	DMAC->CH[NAND_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|DMCH_TRIGSRC_TC0_MC0;

	PIO_WE_RE->EVCTRL.EV[0] = PIN_RE|PORT_PORTEI|PORT_EVACT_CLR;
	PIO_WE_RE->EVCTRL.EV[1] = PIN_RE|PORT_PORTEI|PORT_EVACT_SET;
	PIO_WE_RE->EVCTRL.EV[2] = PIN_RE|PORT_PORTEI|PORT_EVACT_SET;

	while(nandTC->SYNCBUSY & TC_SWRST);

	nandTC->CTRLA = TC_MODE_COUNT8;
	nandTC->WAVE = TC_WAVEGEN_NPWM;
	nandTC->PER8 = 250;
	nandTC->CC8[0] = 1; 

	NAND_DIR_IN();
	PIO_WE_RE->CLR(RE); 

	nandTC->EVCTRL = TC_OVFEO|TC_MCEO0|TC_TCEI|TC_EVACT_RETRIGGER;

	nandTC->CTRLBSET = TC_ONESHOT;
	nandTC->CTRLA = TC_MODE_COUNT8|TC_ENABLE;

	DMAC->SWTRIGCTRL = 1;
}

#elif defined(CPU_XMC48)
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_ReadDataDMA(volatile void *dst, u16 len)
{
	using namespace HW;

	#ifdef CPU_SAME53	

		nandTCC->CTRLA = 0;
		nandTCC->CTRLA = TC_SWRST;

		DmaTable[NAND_DMACH].SRCADDR = &PIO_NAND_DATA->IN;
		DmaTable[NAND_DMACH].DSTADDR = (byte*)dst+len;
		DmaTable[NAND_DMACH].DESCADDR = 0;
		DmaTable[NAND_DMACH].BTCNT = len;
		DmaTable[NAND_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC|DMDSC_EVOSEL_BEAT;

		DMAC->CH[NAND_DMACH].INTENCLR = ~0;
		DMAC->CH[NAND_DMACH].INTFLAG = ~0;
		DMAC->CH[NAND_DMACH].EVCTRL = DMCH_EVOE;
		DMAC->CH[NAND_DMACH].PRILVL = DMCH_PRILVL_LVL3;
		DMAC->CH[NAND_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|DMCH_TRIGSRC_TCC0_MC0;

		//PIO_WE_RE->EVCTRL.EV[0] = PIN_RE|PORT_PORTEI|PORT_EVACT_CLR;
		//PIO_WE_RE->EVCTRL.EV[1] = PIN_RE|PORT_PORTEI|PORT_EVACT_SET;
		//PIO_WE_RE->EVCTRL.EV[2] = PIN_RE|PORT_PORTEI|PORT_EVACT_SET;

		nandTCC->WAVE = TCC_WAVEGEN_NPWM|TCC_POL0;
		nandTCC->DRVCTRL = TCC_NRE0|TCC_NRE1|TCC_NRV0|TCC_NRV1;
		nandTCC->PER = 250;
		nandTCC->CC[0] = 2; 
		nandTCC->CC[1] = 2; 

		nandTCC->EVCTRL = TCC_OVFEO|TCC_MCEO0|TCC_TCEI1|TCC_EVACT1_RETRIGGER;


		NAND_DIR_IN();
		PIO_WE_RE->SET(WE|RE); 
		PIO_WE_RE->DIRSET = WE|RE;
		PIO_WE_RE->PINCFG[PIN_RE] = PINGFG_PMUXEN|PINGFG_DRVSTR;

		nandTCC->CTRLA = TCC_ENABLE;
		nandTCC->CTRLBSET = TCC_ONESHOT|TCC_CMD_RETRIGGER;

	//	DMAC->SWTRIGCTRL = 1;

	#elif defined(CPU_XMC48)

		NAND_DMA->DMACFGREG = 1;

		NAND_DMACH->CTLL = DST_INC|SRC_NOCHANGE|TT_FC(0);
		NAND_DMACH->CTLH = BLOCK_TS(len);

		NAND_DMACH->SAR = (u32)FLD;
		NAND_DMACH->DAR = (u32)dst;
		NAND_DMACH->CFGL = 0;
		NAND_DMACH->CFGH = PROTCTL(1);

		NAND_DMA->CHENREG = NAND_DMA_CHEN;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_ReadDataPIO(volatile void *dst, u16 len)
{
	using namespace HW;

	byte* p = (byte*)dst;

	#ifdef CPU_SAME53

		PIO_WE_RE->PINCFG[PIN_WE] &= ~PINGFG_PMUXEN;
		PIO_WE_RE->PINCFG[PIN_RE] &= ~PINGFG_PMUXEN;

		NAND_DIR_IN();


	#elif defined(CPU_XMC48)
		
		while(len != 0) { *(p++) = *FLD; len--; };

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
	using namespace HW;

	#ifdef CPU_SAME53	

		DmaTable[NAND_MEMCOPY_DMACH].SRCADDR = (byte*)src+len;
		DmaTable[NAND_MEMCOPY_DMACH].DSTADDR = (byte*)dst+len;
		DmaTable[NAND_MEMCOPY_DMACH].DESCADDR = 0;
		DmaTable[NAND_MEMCOPY_DMACH].BTCNT = len;
		DmaTable[NAND_MEMCOPY_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC|DMDSC_SRCINC;

		DMAC->CH[NAND_MEMCOPY_DMACH].INTENCLR = ~0;
		DMAC->CH[NAND_MEMCOPY_DMACH].INTFLAG = ~0;
		DMAC->CH[NAND_MEMCOPY_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_TRANSACTION;
		DMAC->SWTRIGCTRL = 1<<NAND_MEMCOPY_DMACH;

	#elif defined(CPU_XMC48)

//		register u32 t __asm("r0");

		NAND_DMA->DMACFGREG = 1;

		NAND_DMACH->CTLL = DST_INC|SRC_INC|TT_FC(0)|DEST_MSIZE(0)|SRC_MSIZE(0);
		NAND_DMACH->CTLH = BLOCK_TS(len);

		NAND_DMACH->SAR = (u32)src;
		NAND_DMACH->DAR = (u32)dst;
		NAND_DMACH->CFGL = 0;
		NAND_DMACH->CFGH = PROTCTL(1);

		NAND_DMA->CHENREG = NAND_DMA_CHEN;

	#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 NAND_GetMaxBlockLenDMA()
{
	#ifdef CPU_SAME53	

		return 0xFFFF;

	#elif defined(CPU_XMC48)

		return BLOCK_TS(~0);

	#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ PIO_MANCH->CLR(MEMTX1|MEMTX2);}
inline void ManOne()		{ PIO_MANCH->CLR(MEMTX1); PIO_MANCH->SET(MEMTX2); }
inline void ManZero()		{ PIO_MANCH->CLR(MEMTX2); PIO_MANCH->SET(MEMTX1); } 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const u16 manboud[4] = { BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz

u16 trmHalfPeriod = BOUD2CLK(20833)/2;
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&3]/2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;

const u16 rcvPeriod = BOUD2CLK(20833);

static u16* rcvManPtr = 0;
static u16 rcvManCount = 0;

static u16 rcvManLen = 0;
static u16 rcvManLen72 = 0;
static u16 rcvManLen96 = 0;
static u16 rcvManLen24 = 0;
static u16 rcvManLen48 = 0;

static u32 rcvManSum72 = 0;
static u32 rcvManSum96 = 0;
static u32 rcvManSum24 = 0;
static u32 rcvManSum48 = 0;

static u16 rcvManCount72 = 0;
static u16 rcvManCount96 = 0;
static u16 rcvManCount24 = 0;
static u16 rcvManCount48 = 0;

static MRB *manRB = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// 11110, 01001, 10100, 10101, 01010, 01011, 01110, 01111, 10010, 10011, 10110, 10111, 11010, 11011, 11100, 11101

byte tbl_4B5B[16] = { 0x1E, 0x09, 0x14, 0x15, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13, 0x16, 0x17, 0x1A, 0x1B, 0x1C, 0x1D };

inline u32 Encode_4B5B(u16 v) { return tbl_4B5B[v&15]|(tbl_4B5B[(v>>4)&15]<<5)|(tbl_4B5B[(v>>8)&15]<<10)|(tbl_4B5B[(v>>12)&15]<<15); }


// 0 - L1
// 1 - H1
// 2 - H2
// 3 - L2

// L2 H2 H1 L1
// Z - 1111, P - 1100, Z - 1111, N - 0011

u32 mltArr[4] = { MEMTX1, 0, MEMTX2, 0 };
byte mltSeq = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void MltOff()	{ PIO_MANCH->CLR(MEMTX1|MEMTX2);} // 0110  
inline void MltZ()		{ PIO_MANCH->CLR(MEMTX1|MEMTX2); mltSeq = 0; } // 1111
inline void MltNext()	{ PIO_MANCH->CLR(MEMTX1|MEMTX2); PIO_MANCH->SET(mltArr[(mltSeq++)&3]); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte stateMLT3 = 0;
static MTB *mltTB = 0;
static bool mltBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void MLT3_TrmIRQ()
{
	static u32 tw = 0;
//	static u32 pw = 0;
	static byte nrz = 0;
//	static byte pnrz = 0;
	static u16 count = 0;
//	static byte i = 0;
	static const u16 *data = 0;
	static u16 len = 0;


	switch (stateMLT3)
	{
		case 0:	// Idle; 

			MltZ();
		
			data = mltTB->data;
			len = mltTB->len;

//			pw = 0;
			tw = Encode_4B5B(*data);

			data++;
			len--;

			nrz = 0;

			count = 20;

			stateMLT3 = 1;

			break;

		case 1: // Start data

			if (tw & 0x80000)
			{
				nrz ^= 1;
//				MltNext();
			};

			if (nrz != 0)
			{
				MltNext();
			};

			count--;
//			pw = tw;
			tw <<= 1;

			if (count == 0)
			{
				if (len > 0)
				{
					tw = Encode_4B5B(*data);

					data++;
					len--;
					count = 20;
				}
				else
				{
					stateMLT3++;
				};
			};

			break;

		case 2:	

			MltOff();
			stateMLT3 = 0;

			#ifdef CPU_SAME53	
				ManTT->CTRLA = 0;
				ManTT->INTENCLR = ~0;
			#elif defined(CPU_XMC48)
				ManTT->TCCLR = CC4_TRBC;
			#endif


			mltTB->ready = true;
			mltBusy = false;

			break;


	}; // 	switch (stateManTrans)

	#ifdef CPU_SAME53	
		ManTT->INTFLAG = ~0;//TCC_OVF;
	#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendMLT3(MTB *mtb)
{
	if (mltBusy || mtb == 0 || mtb->data == 0 || mtb->len == 0)
	{
		return false;
	};

	mtb->ready = false;

	mltTB = mtb;

	stateMLT3 = 0;

	VectorTableExt[MANT_IRQ] = MLT3_TrmIRQ;
	CM4::NVIC->CLR_PR(MANT_IRQ);
	CM4::NVIC->SET_ER(MANT_IRQ);

	#ifdef CPU_SAME53	

		ManTT->CTRLA = 0;
		ManTT->PER = trmHalfPeriod-1;
		//ManTT->EVCTRL = 1<<6;

		ManTT->INTENCLR = ~TCC_OVF;
		ManTT->INTENSET = TCC_OVF;

		ManTT->INTFLAG = ~0;

		ManTT->CTRLA = TCC_ENABLE;

	#elif defined(CPU_XMC48)

		ManTT->PRS = trmHalfPeriod-1;
		ManTT->PSC = 3; //0.08us

		ManCCU->GCSS = CCU4_S0SE;  

		VectorTableExt[CCU40_0_IRQn] = MLT3_TrmIRQ;
		CM4::NVIC->CLR_PR(CCU40_0_IRQn);
		CM4::NVIC->SET_ER(CCU40_0_IRQn);	

		ManTT->SRS = 0;

		ManTT->SWR = ~0;
		ManTT->INTE = CC4_PME;

		ManTT->TCSET = CC4_TRBS;

	#endif

	return mltBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u16 CheckParity(u16 x)
{
	u16 y = x ^ (x >> 1);

	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	
	return (y ^ (y >> 8))^1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static const u16 *data = 0;
	static u16 len = 0;
	static bool cmd = false;

	Pin_ManTrmIRQ_Set();

	switch (stateManTrans)
	{
		case 0:	// Idle; 

			ManDisable();
		
			data = manTB->data;
			len = manTB->len;
			stateManTrans = 1;
			cmd = false;

			break;

		case 1: // Start data

			i = 3;
			tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

			data++;
			len--;

			if (cmd) { ManZero(); }	else { ManOne(); };

			//ManOne();

			stateManTrans++;

			break;

		case 2:	// Wait data 1-st sync imp

			i--;

			if (i == 0)
			{
				stateManTrans++;

				if (cmd) { ManOne(); }	else { ManZero(); };

				//ManZero();

				cmd = false;

				i = 3;
			};

			break;

		case 3: // Wait 2-nd sync imp

			i--;

			if (i == 0)
			{
				stateManTrans++;
				count = 17;

				if (tw & 0x10000) ManZero(); else ManOne();
			};

			break;

		case 4: // 1-st half bit wait

			if (tw & 0x10000) ManOne(); else ManZero();

			count--;

			if (count == 0)
			{
				if (len > 0)
				{
					stateManTrans = 1;
				}
				else if (manTB->next != 0)
				{
					manTB->ready = true;

					manTB = manTB->next;

					len = manTB->len;
					data = manTB->data;

					stateManTrans = (data != 0 && len != 0) ? 1 : 6;
				}
				else
				{
					stateManTrans = 6;
				};
			}
			else
			{
				stateManTrans++;
			};

			break;

		case 5: // 2-nd half bit wait

			tw <<= 1;
			stateManTrans = 4;
			if (tw & 0x10000) ManZero(); else ManOne();

			break;

		case 6:

			stateManTrans++;

			break;

		case 7:

			ManDisable();
			stateManTrans = 0;

			#ifdef CPU_SAME53	
				ManTT->CTRLA = 0;
				ManTT->INTENCLR = ~0;
			#elif defined(CPU_XMC48)
				ManTT->TCCLR = CC4_TRBC;
			#endif

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)


	#ifdef CPU_SAME53	
		ManTT->INTFLAG = ~0;//TCC_OVF;
	#endif

	Pin_ManTrmIRQ_Clr();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData(MTB *mtb)
{
	if (trmBusy || /*rcvBusy ||*/ mtb == 0 || mtb->data == 0 || mtb->len == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	stateManTrans = 0;

	#ifdef CPU_SAME53	

		ManTT->CTRLA = 0;

		ManTT->PER = trmHalfPeriod-1;

		ManTT->INTENCLR = ~TCC_OVF;
		ManTT->INTENSET = TCC_OVF;

		ManTT->INTFLAG = ~0;

		ManTT->CTRLA = TCC_ENABLE;

	#elif defined(CPU_XMC48)

		ManTT->PRS = trmHalfPeriod-1;
		ManTT->PSC = 3; //0.08us

		ManCCU->GCSS = CCU4_S0SE;  

		ManTT->SWR = ~0;
		ManTT->INTE = CC4_PME;

		ManTT->TCSET = CC4_TRBS;

	#endif

	return trmBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit()
{
	using namespace HW;

	VectorTableExt[MANT_IRQ] = ManTrmIRQ;
	CM4::NVIC->CLR_PR(MANT_IRQ);
	CM4::NVIC->SET_ER(MANT_IRQ);

#ifdef CPU_SAME53	
	HW::GCLK->PCHCTRL[GCLK_TCC4] = GCLK_GEN(GEN_1M)|GCLK_CHEN;

	HW::MCLK->APBDMASK |= APBD_TCC4;

	PIO_MANCH->DIRSET = MEMTX1|MEMTX2;

	ManTT->CTRLA = TCC_SWRST;

	while(ManTT->SYNCBUSY);


	SetTrmBoudRate(0);

	ManTT->PER = trmHalfPeriod-1;

	ManTT->INTENCLR = ~TCC_OVF;
	ManTT->INTENSET = TCC_OVF;

	ManTT->INTFLAG = ~0;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManCCU_PID);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_S0I|CCU4_PRB;

	ManTT->PRS = trmHalfPeriod-1;
	ManTT->PSC = 3; //0.08us

	ManCCU->GCSS = CCU4_S0SE;  

	ManTT->SRS = 0;

	ManTT->SWR = ~0;
	ManTT->INTE = CC4_PME;

#endif

	ManDisable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte stateMT = 0;
static MTB *manTB2 = 0;
static bool trmBusy2 = false;
static bool trmTurbo = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ_2()
{
	static u32 tw = 0;
	static u16 count = 0;
	static const u16 *data = 0;
	static u16 len = 0;

	HW::PIOA->BSET(10);

	switch (stateMT)
	{
		case 0:	// 1-st sync imp 

			HW::PIOA->BSET(11); 

			data = manTB2->data;
			len = manTB2->len;

			ManTCC->CC[0] += trmHalfPeriod*3; //US2MT(72);
			stateMT++;

			break;

		case 1:	// 2-nd sync imp

			tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

			data++;
			len--;

			count = 17;

			if (tw & 0x10000)
			{
				ManTCC->CC[0] += trmHalfPeriod*4; //US2MT(96);
				ManTCC->CC[1] += trmHalfPeriod*4; //US2MT(96);
				stateMT += 2;
			}
			else
			{
				ManTCC->CC[0] += trmHalfPeriod*3; //US2MT(72);
				ManTCC->CC[1] += trmHalfPeriod*3; //US2MT(72);
				stateMT++;
			};
 
			break;

		case 2: // 1-st half bit

			ManTCC->CC[0] += trmHalfPeriod; //US2MT(24);
			ManTCC->CC[1] += trmHalfPeriod; //US2MT(24);
			stateMT++;

			break;

		case 3:	// 2-nd half bit

			count--;

			if (count != 0)
			{
				u32 t = tw;
				tw <<= 1;

				t = (t ^ tw) & 0x10000;

				if (t)
				{
					ManTCC->CC[0] += trmHalfPeriod*2; //US2MT(48);
					ManTCC->CC[1] += trmHalfPeriod*2; //US2MT(48);
					stateMT = 3;
				}
				else
				{
					ManTCC->CC[0] += trmHalfPeriod; //US2MT(24);
					ManTCC->CC[1] += trmHalfPeriod; //US2MT(24);
					stateMT = 2;
				};
			}
			else
			{
				if (len == 0)
				{
					if (manTB2->next != 0)
					{
						manTB2->ready = true;

						manTB2 = manTB2->next;

						len = manTB2->len;
						data = manTB2->data;
					};
				};

				if (len > 0)
				{
					if (!trmTurbo)
					{
						if (tw & 0x10000)
						{
							ManTCC->CC[0] += trmHalfPeriod*4;
							ManTCC->CC[1] += trmHalfPeriod*4;
							stateMT = 1;
						}
						else
						{
							ManTCC->CC[0] += trmHalfPeriod; //US2MT(24);
							ManTCC->CC[1] += trmHalfPeriod; //US2MT(24);
							stateMT++;
						};
					}
					else
					{
						tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

						data++;
						len--;

						count = 17;

						if (tw & 0x10000)
						{
							ManTCC->CC[0] += trmHalfPeriod*4; //US2MT(96);
							ManTCC->CC[1] += trmHalfPeriod*4; //US2MT(96);
							stateMT = 3;
						}
						else
						{
							ManTCC->CC[0] += trmHalfPeriod*3; //US2MT(72);
							ManTCC->CC[1] += trmHalfPeriod*3; //US2MT(72);
							stateMT = 2;
						};
					};
				}
				else
				{
					ManTCC->CC[0] += trmHalfPeriod; //US2MT(24);
					ManTCC->CC[1] += trmHalfPeriod; //US2MT(24);

					stateMT += 2;
				}
			};

			break;

		case 4:

			ManTCC->CC[0] += trmHalfPeriod*3;
			ManTCC->CC[1] += trmHalfPeriod*3;
			stateMT = 1;

			break;

		case 5:

//			ManDisable();
			stateMT = 0;

			ManTCC->CTRLA = 0;
			ManTCC->INTENCLR = ~0;

			manTB2->ready = true;
			trmBusy2 = false;

			break;


	}; // 	switch (stateManTrans)


	ManTCC->CTRLBSET = TCC_CMD_UPDATE;

	ManTCC->INTFLAG = TCC_MC0;

	HW::PIOA->BCLR(10);
	HW::PIOA->BCLR(11);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData_2(MTB *mtb)
{
	if (trmBusy2 || mtb == 0 || mtb->data == 0 || mtb->len == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB2 = mtb;

	stateMT = 0;

	ManTCC->CTRLA = TCC_PRESCALER_DIV1;
	ManTCC->WAVE = TCC_WAVEGEN_NFRQ;//|TCC_POL0;
	ManTCC->DRVCTRL = 0;//TCC_INVEN1;
	ManTCC->PER = 0xFFFFFF;
	ManTCC->CC[0] = US2MT(100); 
	ManTCC->CC[1] = US2MT(100)+trmHalfPeriod*3; 

	ManTCC->EVCTRL = 0;

	ManTCC->INTENCLR = ~0;
	ManTCC->INTENSET = TCC_MC0;
	ManTCC->INTFLAG = ~0;

	ManTCC->CTRLBCLR = TCC_LUPD;

	ManTCC->CTRLA = TCC_ENABLE|TCC_PRESCALER_DIV1;
	ManTCC->CTRLBSET = TCC_CMD_RETRIGGER;

	return trmBusy2 = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit_2()
{
	using namespace HW;

	HW::PIOA->DIRSET = (1<<10)|(1<<11);

	VectorTableExt[MANT_IRQ_2] = ManTrmIRQ_2;
	CM4::NVIC->CLR_PR(MANT_IRQ_2);
	CM4::NVIC->SET_ER(MANT_IRQ_2);

	//HW::MCLK->APBBMASK |= APBB_TCC1;

	HW::GCLK->PCHCTRL[GCLK_TCC4] = GCLK_GEN(GEN_1M)|GCLK_CHEN;
	HW::MCLK->APBDMASK |= APBD_TCC4;

	PIO_MANCH->DIRCLR = MEMTX1|MEMTX2;
	PIO_MANCH->SetWRCONFIG(MEMTX1|MEMTX2, PORT_PMUX_F|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);

	ManTCC->CTRLA = TCC_SWRST;

	while(ManTCC->SYNCBUSY);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
#ifdef CPU_SAME53	
	ManRT->INTENCLR = ~0;
#elif defined(CPU_XMC48)
	ManRT->INTE = 0;
#endif

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = rcvManLen;

	rcvManLen96 = (rcvManCount96 != 0) ? (rcvManSum96 / rcvManCount96) : 0;
	rcvManLen72 = (rcvManCount72 != 0) ? (rcvManSum72 / rcvManCount72) : 0;
	rcvManLen48 = (rcvManCount48 != 0) ? (rcvManSum48 / rcvManCount48) : 0;
	rcvManLen24 = (rcvManCount24 != 0) ? (rcvManSum24 / rcvManCount24) : 0;
	
	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static RTM manRcvTime;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManRcvIRQ2()
{
	using namespace HW;

	static u32 _number = 0;
	static u32 _length = 0;
	static u32 _data = 0;
	static bool _command = false;
	static bool _parity_temp = false;
	const bool _parity = true;
	static bool _sync = false;
	static bool _state = false;

	Pin_ManRcvIRQ_Set();

	#ifdef CPU_SAME53	

		u32 len = ManRT->CC[0];

		//ManRT->CTRLBSET = TCC_CMD_RETRIGGER;

	#elif defined(CPU_XMC48)

		u16 len = ManTmr->TIMER-1;

		ManTmr->TCCLR = CC4_TCC;
		ManTmr->TCSET = CC4_TRB;

	#endif

	_state = !_state;

	if (len <= MT(60))
	{
		if (len <= MT(36))
		{
			_length += 1; rcvManSum24 += len; rcvManCount24++;
		}
		else
		{
			_length += 2; rcvManSum48 += len; rcvManCount48++;
		};

		if(_length >= 3) { _sync = false; };
	}
	else
	{
		if(len > MT(108))
		{
			_sync = false;
		}
		else
		{
			manRcvTime.Reset();

			_sync = true;
			_data = 0;
			_parity_temp = _parity;
			_number = 0;
			_command = !_state; 

			if (len <= MT(84))
			{
				_length = 1; rcvManSum72 += len; rcvManCount72++;
			}
			else
			{
				_length = 2; rcvManSum96 += len; rcvManCount96++;
			};

		};
	};

	if(_sync && _length == 2)
	{
		manRcvTime.Reset();

		if(_number < 16)
		{
			_data <<= 1;
			_data |= _state;
			_parity_temp ^= _state;
			_number++;
			_length = 0;
		}
	 	else
		{
			Pin_ManRcvSync_Set();

			_sync = false;

			if(_state != _parity_temp)
			{
				_state = !_state;
				_data = (~_data);
				_command = !_command;
			};

			if (rcvManLen == 0)
			{
				if(_command)
				{
					*rcvManPtr++ = _data;
					rcvManLen = 1;
				};
			}
			else 
			{
				if(rcvManLen < rcvManCount)
				{
					*rcvManPtr++ = _data;
				};

				rcvManLen += 1;	
			};

			Pin_ManRcvSync_Clr();
		};
	};

	#ifdef CPU_SAME53	
		ManRT->INTFLAG = ~0;
	#elif defined(CPU_XMC48)
	#endif

	Pin_ManRcvIRQ_Clr();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		__disable_irq();

		if (rcvManLen > 0 && manRcvTime.Timeout(US2RT(200)))
		{
			ManRcvEnd(true);
		}
		else
		{
			manRB->len = rcvManLen;
		};

		__enable_irq();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ManRcvStop()
//{
//	ManRcvEnd(true);
//}
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	using namespace HW;

	VectorTableExt[MANR_IRQ] = ManRcvIRQ2;
	CM4::NVIC->CLR_PR(MANR_IRQ);
	CM4::NVIC->SET_ER(MANR_IRQ);	


#ifdef CPU_SAME53	
	HW::GCLK->PCHCTRL[GCLK_TCC2_TCC3] = GCLK_GEN(GEN_1M)|GCLK_CHEN;
	HW::MCLK->APBCMASK |= APBC_TCC2|APBC_TCC3;

	HW::GCLK->PCHCTRL[EVENT_MANR_1+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[EVENT_MANR_2+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	if ((EIC->CTRLA & EIC_ENABLE) == 0)
	{
		EIC->CTRLA = EIC_SWRST;
		while(EIC->SYNCBUSY);
	}
	else
	{
		EIC->CTRLA = 0;
		while(EIC->SYNCBUSY);
	};

	EIC->EVCTRL |= EIC_EXTINT0<<MANR_EXTINT;
	EIC->SetConfig(MANR_EXTINT, 1, EIC_SENSE_BOTH);
	EIC->INTENCLR = EIC_EXTINT0<<MANR_EXTINT;
	EIC->CTRLA = EIC_ENABLE;

	EVSYS->CH[EVENT_MANR_1].CHANNEL = (EVGEN_EIC_EXTINT_0+MANR_EXTINT)|EVSYS_PATH_ASYNCHRONOUS;
	EVSYS->USER[EVSYS_USER_TCC3_EV_0] = EVENT_MANR_1+1;

	EVSYS->CH[EVENT_MANR_2].CHANNEL = EVGEN_TCC3_OVF|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;
	EVSYS->USER[EVSYS_USER_TCC2_MC_0] = EVENT_MANR_2+1;
	EVSYS->USER[EVSYS_USER_TCC2_EV_0] = EVENT_MANR_2+1;

	PIO_MANCH->DIRCLR = MANCHRX;
	PIO_MANCH->CTRL |= MANCHRX;

	PIO_MANCH->SetWRCONFIG(MANCHRX, PORT_PMUX(0)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);
	PIO_MANCH->PINCFG[PIN_MANCHRX] = PINGFG_INEN|PINGFG_PMUXEN;

	ManRT->CTRLA = TCC_SWRST;
	while(ManRT->SYNCBUSY);

	ManRT->CTRLA = TCC_CPTEN0;
	ManRT->EVCTRL = TCC_MCEI0|TCC_TCEI0|TCC_EVACT0_RETRIGGER;

	ManRT->PER = ~0;
	ManRT->CC[1] = 250;

	ManRT->INTENCLR = ~0;
	//ManRT->INTENSET = TCC_MC0;

	ManRT->CTRLA = TCC_CPTEN0|TCC_ENABLE;


	ManIT->CTRLA = TCC_SWRST;

	while(ManIT->SYNCBUSY);

	ManIT->CTRLA = 0;
	ManIT->EVCTRL = TCC_TCEI0|TCC_EVACT0_RETRIGGER|TCC_OVFEO;

	ManIT->PER = 11;
	ManIT->CC[0] = ~0;
	ManIT->CC[1] = ~0;

	ManIT->INTENCLR = ~0;
	//ManIT->INTENSET = TCC_OVF;

	ManIT->INTFLAG = ~0;

	ManIT->CTRLA = TCC_ENABLE;

	ManIT->CTRLBSET = TCC_ONESHOT;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManCCU_PID);

	P2->ModePin6(I2DPU);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_CS1I|CCU4_CS3I|CCU4_SPRB;

	ManRT->PRS = MT(12)-1;
	ManRT->PSC = 7; //1.28us

	ManTmr->PRS = MT(250);
	ManTmr->PSC = 7; //1.28us

	ManCCU->GCSS = CCU4_S1SE|CCU4_S3SE;  

	ManRT->INS = CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->INTE = 0;//CC4_PME;
	ManRT->SRS = CC4_POSR(2);

	ManTmr->INS = 0;
	ManTmr->CMC = 0;
	ManTmr->TC = CC4_TSSM;
	ManTmr->TCSET = CC4_TRB;

	ManTmr->INTE = 0;//CC4_PME;

#endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
	if (rcvBusy /*|| trmBusy*/ || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	//ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	rcvManLen = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;

	rcvManSum96 = 0; rcvManCount96 = 0;
	rcvManSum72 = 0; rcvManCount72 = 0;
	rcvManSum48 = 0; rcvManCount48 = 0;
	rcvManSum24 = 0; rcvManCount24 = 0;

	#ifdef CPU_SAME53	

		ManRT->INTFLAG = ~0;
		ManRT->INTENSET = TCC_MC0;//|TCC_TRG;

	#elif defined(CPU_XMC48)

		ManRT->SWR = CC4_RPM;
		ManRT->INTE = CC4_PME;

	#endif

	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte *twi_wrPtr = 0;
static byte *twi_rdPtr = 0;
static u16 twi_wrCount = 0;
static u16 twi_rdCount = 0;
static byte *twi_wrPtr2 = 0;
static u16 twi_wrCount2 = 0;
static byte twi_adr = 0;
static DSCI2C* twi_dsc = 0;
static DSCI2C* twi_lastDsc = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53	

static __irq void I2C_Handler()
{
	using namespace HW;

	HW::PIOB->BSET(20);

	byte state = I2C->INTFLAG;
	bool nextdsc = false;

	if(state & I2C_ERROR) // Received data is available
	{
		I2C->INTFLAG = I2C_ERROR;
		I2C->STATUS = ~0;
		nextdsc = true;
	}
	else if(state & I2C_SB) // Received data is available
	{
		*twi_rdPtr++ = I2C->DATA; // receive data

		twi_rdCount--;

		if (twi_rdCount > 0)
		{
			I2C->CTRLB = I2C_CMD_2;
		}
		else
		{
			I2C->CTRLB = I2C_ACKACT;

			nextdsc = true; 
		};
	}
	else if(state & I2C_MB) // Data can be transmitted 
	{
		if (I2C->STATUS.RXNACK)
		{
			nextdsc = true;
		}
		else if (twi_wrCount > 0)
		{
			I2C->DATA = *twi_wrPtr++;

			twi_wrCount--;

			twi_dsc->ack = true;

			if(twi_wrCount == 0 && twi_wrCount2 != 0)
			{
				twi_wrPtr = twi_wrPtr2;
				twi_wrCount = twi_wrCount2;
				twi_wrCount2 = 0;
			};
		}
		else if (twi_rdCount > 0)
		{
			I2C->ADDR = (twi_adr << 1) | 1;
		}
		else
		{
			nextdsc = true; //I2C->CTRLB |= I2C_CMD_STOP;
		};
	}
	else
	{
		twi_rdCount = 0;
		twi_wrCount = 0;

		nextdsc = true; //I2C->CTRLB |= I2C_CMD_STOP;
	};

	if (nextdsc)
	{
		twi_dsc->ready = true;
		twi_dsc->readedLen = twi_dsc->rlen - twi_rdCount;

		DSCI2C *ndsc = twi_dsc->next;

		if (ndsc != 0)
		{
			twi_dsc->next = 0;
			twi_dsc = ndsc;

			twi_dsc->ready = false;
			twi_dsc->ack = false;
			twi_dsc->readedLen = 0;

			twi_wrPtr = (byte*)twi_dsc->wdata;	
			twi_rdPtr = (byte*)twi_dsc->rdata;	
			twi_wrPtr2 = (byte*)twi_dsc->wdata2;	
			twi_wrCount = twi_dsc->wlen;
			twi_wrCount2 = twi_dsc->wlen2;
			twi_rdCount = twi_dsc->rlen;
			twi_adr = twi_dsc->adr;

			if (twi_wrPtr2 == 0) twi_wrCount2 = 0;

			//I2C->STATUS.BUSSTATE = BUSSTATE_IDLE;

			I2C->INTFLAG = ~0;
			I2C->INTENSET = I2C_MB|I2C_SB|I2C_ERROR;

			I2C->ADDR = (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);
		}
		else
		{
			I2C->CTRLB = I2C_CMD_STOP|I2C_ACKACT;

			twi_lastDsc = twi_dsc = 0;
		};
	};

	HW::PIOB->BCLR(20);
}

#elif defined(CPU_XMC48)

static __irq void I2C_Handler()
{
	using namespace HW;

//	HW::P5->BSET(7);

	u32 a = I2C->PSR_IICMode;

	if(a & ACK)
	{
		if (twi_wrCount > 0)
		{
			I2C->TBUF[0] = TDF_MASTER_SEND | *twi_wrPtr++;

			twi_wrCount--;

			twi_dsc->ack = true;

			if(twi_wrCount == 0 && twi_wrCount2 != 0)
			{
				twi_wrPtr = twi_wrPtr2;
				twi_wrCount = twi_wrCount2;
				twi_wrCount2 = 0;
			};
		}
		else if (twi_rdCount > 0)
		{
			if(a & (SCR|RSCR))
			{
				I2C->TBUF[0] = TDF_MASTER_RECEIVE_ACK; 
			}
			else
			{
				I2C->TBUF[0] = TDF_MASTER_RESTART | (twi_adr << 1) | 1;
			};
		}
		else
		{
			I2C->TBUF[0] = TDF_MASTER_STOP;
		};
	}
	else if (a & (RIF|AIF))
	{
		byte t = I2C->RBUF;

		if (twi_rdCount > 0)
		{
			*twi_rdPtr++ = t; // receive data
			twi_rdCount--;
		};
			
		I2C->TBUF[0] = (twi_rdCount > 0) ? TDF_MASTER_RECEIVE_ACK : TDF_MASTER_RECEIVE_NACK; 
	}
	//else if (wrCount != 0 || rdCount != 0)
	//{
	//	I2C->PSCR = a;

	//	I2C->TBUF[0] = TDF_MASTER_STOP; 

	//	wrCount = 0;
	//	rdCount = 0;
	//}
	else
	{
		twi_dsc->ready = true;
		twi_dsc->readedLen = twi_dsc->rlen - twi_rdCount;

//		state = 0;
		
		DSCI2C *ndsc = twi_dsc->next;

		if (ndsc != 0)
		{
			twi_dsc->next = 0;
			twi_dsc = ndsc;

			twi_dsc->ready = false;
			twi_dsc->ack = false;
			twi_dsc->readedLen = 0;

			twi_wrPtr = (byte*)twi_dsc->wdata;	
			twi_rdPtr = (byte*)twi_dsc->rdata;	
			twi_wrPtr2 = (byte*)twi_dsc->wdata2;	
			twi_wrCount = twi_dsc->wlen;
			twi_wrCount2 = twi_dsc->wlen2;
			twi_rdCount = twi_dsc->rlen;
			twi_adr = twi_dsc->adr;

			if (twi_wrPtr2 == 0) twi_wrCount2 = 0;

			//I2C->CCR |= RIEN|AIEN;
			//I2C->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

			I2C->TBUF[0] = TDF_MASTER_START | (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);
		}
		else
		{
			I2C->CCR = __CCR;
			I2C->PCR_IICMode = __PCR;

			twi_lastDsc = twi_dsc = 0;
		};

//		I2C->PSCR = PCR|NACK;
	};

	I2C->PSCR = a;

//	HW::P5->BCLR(7);
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_Write(DSCI2C *d)
{
	using namespace HW;

	if (twi_dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	twi_lastDsc = twi_dsc = d;

	twi_dsc->ready = false;
	twi_dsc->ack = false;
	twi_dsc->readedLen = 0;

	twi_wrPtr = (byte*)twi_dsc->wdata;	
	twi_rdPtr = (byte*)twi_dsc->rdata;	
	twi_wrPtr2 = (byte*)twi_dsc->wdata2;	
	twi_wrCount = twi_dsc->wlen;
	twi_wrCount2 = twi_dsc->wlen2;
	twi_rdCount = twi_dsc->rlen;
	twi_adr = twi_dsc->adr;

	if (twi_wrPtr2 == 0) twi_wrCount2 = 0;

	__disable_irq();

	#ifdef CPU_SAME53

		I2C->STATUS.BUSSTATE = BUSSTATE_IDLE;

		I2C->INTFLAG = ~0;
		I2C->INTENSET = I2C_MB|I2C_SB|I2C_ERROR;

		I2C->ADDR = (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);

	#elif defined(CPU_XMC48)

		I2C->PSCR = ~0;//RIF|AIF|TBIF|ACK|NACK|PCR;

//		state = (wrCount == 0) ? 1 : 0;

		I2C->TBUF[0] = TDF_MASTER_START | (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);

		I2C->CCR |= RIEN|AIEN;
		I2C->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

	#endif
		
	__enable_irq();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_AddRequest(DSCI2C *d)
{
	if (twi_dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	d->next = 0;
	d->ready = false;

	if (d->wdata2 == 0) d->wlen2 = 0;

	__disable_irq();

	if (twi_lastDsc == 0)
	{
		twi_lastDsc = d;

		__enable_irq();

		return I2C_Write(d);
	}
	else
	{
		twi_lastDsc->next = d;
		twi_lastDsc = d;

		__enable_irq();
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void I2C_Update()
{
#ifdef CPU_SAME53

#elif defined(CPU_XMC48)

	using namespace HW;

	static TM32 tm;

	__disable_irq();

	if (twi_dsc != 0)
	{
		if (I2C->PSR_IICMode & (PCR|NACK|ACK|RIF|AIF))
		{
			tm.Reset();
		}
		else if (tm.Check(10))
		{
			HW::Peripheral_Disable(PID_USIC2);

 			P5->ModePin0(A1OD);
			P5->ModePin2(A1PP);

			HW::Peripheral_Enable(I2C_PID);

			I2C->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

			I2C->SCTR = __SCTR;

			I2C->FDR = __FDR;
			I2C->BRG = __BRG;
		    
			I2C->TCSR = __TCSR;

			I2C->PSCR = ~0;

			I2C->CCR = 0;

			I2C->DX0CR = __DX0CR;
			I2C->DX1CR = __DX1CR;

			I2C->CCR = __CCR;

			I2C->PCR_IICMode = __PCR;

			VectorTableExt[I2C_IRQ] = I2C_Handler;
			CM4::NVIC->CLR_PR(I2C_IRQ);
			CM4::NVIC->SET_ER(I2C_IRQ);

			twi_dsc->ready = true;
			twi_dsc->readedLen = twi_dsc->rlen - twi_rdCount;

			DSCI2C *ndsc = twi_dsc->next;

			if (ndsc != 0)
			{
				twi_dsc->next = 0;
				twi_dsc = ndsc;

				twi_dsc->ready = false;
				twi_dsc->ack = false;
				twi_dsc->readedLen = 0;

				twi_wrPtr = (byte*)twi_dsc->wdata;	
				twi_rdPtr = (byte*)twi_dsc->rdata;	
				twi_wrPtr2 = (byte*)twi_dsc->wdata2;	
				twi_wrCount = twi_dsc->wlen;
				twi_wrCount2 = twi_dsc->wlen2;
				twi_rdCount = twi_dsc->rlen;
				twi_adr = twi_dsc->adr;

				if (twi_wrPtr2 == 0) twi_wrCount2 = 0;

				I2C->PSCR = ~0;//RIF|AIF|TBIF|ACK|NACK|PCR;

				I2C->CCR |= RIEN|AIEN;
				I2C->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

				I2C->TBUF[0] = TDF_MASTER_START | (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);
			}
			else
			{
				I2C->CCR = __CCR;
				I2C->PCR_IICMode = __PCR;

				twi_lastDsc = twi_dsc = 0;
			};
		};
	}
	else
	{
		tm.Reset();
	};
	
	__enable_irq();

#endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_Init()
{
	using namespace HW;

#ifdef CPU_SAME53	

	HW::GCLK->PCHCTRL[GCLK_SERCOM3_CORE]	= GCLK_GEN(GEN_25M)|GCLK_CHEN;	// 25 MHz
	HW::GCLK->PCHCTRL[GCLK_SERCOM_SLOW]		= GCLK_GEN(GEN_32K)|GCLK_CHEN;	// 32 kHz

	MCLK->APBBMASK |= APBB_SERCOM3;

	PIO_I2C->WRCONFIG = ((SDA|SCL)>>16)	|PORT_HWSEL_HI|PORT_PMUX(2)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX;

	I2C->CTRLA = I2C_SWRST;

	while(I2C->SYNCBUSY);

	I2C->CTRLA = SERCOM_MODE_I2C_MASTER;

	I2C->CTRLA = SERCOM_MODE_I2C_MASTER|I2C_INACTOUT_205US|I2C_SPEED_SM;
	I2C->CTRLB = 0;
	I2C->BAUD = 0x0018;

	I2C->CTRLA |= I2C_ENABLE;

	while(I2C->SYNCBUSY);

	I2C->STATUS = 0;
	I2C->STATUS.BUSSTATE = BUSSTATE_IDLE;

	VectorTableExt[SERCOM3_0_IRQ] = I2C_Handler;
	VectorTableExt[SERCOM3_1_IRQ] = I2C_Handler;
	VectorTableExt[SERCOM3_3_IRQ] = I2C_Handler;
	CM4::NVIC->CLR_PR(SERCOM3_0_IRQ);
	CM4::NVIC->CLR_PR(SERCOM3_1_IRQ);
	CM4::NVIC->CLR_PR(SERCOM3_3_IRQ);
	CM4::NVIC->SET_ER(SERCOM3_0_IRQ);
	CM4::NVIC->SET_ER(SERCOM3_1_IRQ);
	CM4::NVIC->SET_ER(SERCOM3_3_IRQ);

#elif defined(CPU_XMC48)

	HW::Peripheral_Enable(I2C_PID);

 	P5->ModePin0(A1OD);
	P5->ModePin2(A1PP);

	I2C->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

	I2C->SCTR = __SCTR;

	I2C->FDR = __FDR;
	I2C->BRG = __BRG;
    
	I2C->TCSR = __TCSR;

	I2C->PSCR = ~0;

	I2C->CCR = 0;

	I2C->DX0CR = __DX0CR;
	I2C->DX1CR = __DX1CR;

	I2C->CCR = __CCR;


	I2C->PCR_IICMode = __PCR;

	VectorTableExt[I2C_IRQ] = I2C_Handler;
	CM4::NVIC->CLR_PR(I2C_IRQ);
	CM4::NVIC->SET_ER(I2C_IRQ);

#endif

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_DS3232	0x68
#define ADR_RV3129	0x56

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SetClock_DS3232(const RTC &t)
{
	static DSCI2C dsc;

	//static byte reg = 0;
	//static u16 rbuf = 0;
	static byte buf[8];

	buf[0] = 0;
	buf[1] = ((t.sec/10) << 4)|(t.sec%10);
	buf[2] = ((t.min/10) << 4)|(t.min%10);
	buf[3] = ((t.hour/10) << 4)|(t.hour%10);
	buf[4] = 1;
	buf[5] = ((t.day/10) << 4)|(t.day%10);
	buf[6] = ((t.mon/10) << 4)|(t.mon%10);

	byte y = t.year % 100;

	buf[7] = ((y/10) << 4)|(y%10);

	dsc.adr = ADR_DS3232;
	dsc.wdata = buf;
	dsc.wlen = 8;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	if (SetTime(t))
	{
		I2C_AddRequest(&dsc);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SetClock_RV3129(const RTC &t)
{
	static DSCI2C dsc;

	//static byte reg = 0;
	//static u16 rbuf = 0;
	static byte buf[8];

	buf[0] = 8;
	buf[1] = ((t.sec/10) << 4)|(t.sec%10);
	buf[2] = ((t.min/10) << 4)|(t.min%10);
	buf[3] = ((t.hour/10) << 4)|(t.hour%10);
	buf[4] = 1;
	buf[5] = ((t.day/10) << 4)|(t.day%10);
	buf[6] = ((t.mon/10) << 4)|(t.mon%10);

	byte y = t.year % 100;

	buf[7] = ((y/10) << 4)|(y%10);

	dsc.adr = ADR_RV3129;
	dsc.wdata = buf;
	dsc.wlen = 8;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	if (SetTime(t))
	{
		I2C_AddRequest(&dsc);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqClockTemp_DS3232(DSCI2C &dsc)
{
	static byte buf[3];

	buf[0] = 0x11;

	dsc.adr = ADR_DS3232;
	dsc.wdata = buf;
	dsc.wlen = 1;
	dsc.rdata = buf+1;
	dsc.rlen = 2;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	return I2C_AddRequest(&dsc);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqClockTemp_RV3129(DSCI2C &dsc)
{
	static byte buf[2];

	buf[0] = 0x20;

	dsc.adr = ADR_RV3129;
	dsc.wdata = buf;
	dsc.wlen = 1;
	dsc.rdata = buf+1;
	dsc.rlen = 1;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	return I2C_AddRequest(&dsc);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static i16 GetClockTemp_DS3232(DSCI2C &dsc)
{
	byte *p = (byte*)dsc.rdata;

	i16 t = ((i16)((p[0]<<8) + p[1]) + 128) / 256;

	if (t < (-60))
	{
		t += 256;
	};

	return t;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static i16 GetClockTemp_RV3129(DSCI2C &dsc)
{
	byte *p = (byte*)dsc.rdata;

	return (i16)(p[0]) - 60;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool StartConvClockTemp_DS3232(DSCI2C &dsc)
{
	static byte buf[3];

	buf[0] = 0x0E;
	buf[1] = 0x20;
	buf[2] = 0xC8;

	dsc.adr = ADR_DS3232;
	dsc.wdata = buf;
	dsc.wlen = 3;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	return I2C_AddRequest(&dsc);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool StartConvClockTemp_RV3129(DSCI2C &dsc)
{
	static byte buf[2];

	buf[0] = 0x02; // Reset Control_INT Flag 
	buf[1] = 0x00;

	dsc.adr = ADR_RV3129;
	dsc.wdata = buf;
	dsc.wlen = 2;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	return I2C_AddRequest(&dsc);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void (*pSetClock)(const RTC &t) = SetClock_DS3232;
static bool (*pReqClockTemp)(DSCI2C &dsc) = ReqClockTemp_DS3232;
static i16 (*pGetClockTemp)(DSCI2C &dsc) = GetClockTemp_DS3232;
static bool (*pStartConvClockTemp)(DSCI2C &dsc) = StartConvClockTemp_DS3232;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetClock(const RTC &t)
{
	if (pSetClock != 0)	pSetClock(t);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void Clock_IRQ()
{
	HW::PIOB->BSET(7);

	timeBDC.msec = (timeBDC.msec < 500) ? 0 : 999;

	HW::EIC->INTFLAG = 1 << CLOCK_EXTINT;
	HW::PIOB->BCLR(7);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool InitClock_DS3232()
{
	DSCI2C dsc;

	byte reg = 0;
	byte buf[10];
	
	::RTC t;

	buf[0] = 0x0F;
	buf[1] = 0x88;
	dsc.adr = ADR_DS3232;
	dsc.wdata = buf;
	dsc.wlen = 2;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	if (!dsc.ack) return false;

	dsc.adr = ADR_DS3232;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();	// { HW::WDT->Update(); };

	if (!dsc.ack) return false;

	t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
	t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
	t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
	t.day	= (buf[4]&0xF) + ((buf[4]>>4)*10);
	t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
	t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

	SetTime(t);

	pSetClock = SetClock_DS3232;
	pReqClockTemp = ReqClockTemp_DS3232;
	pGetClockTemp = GetClockTemp_DS3232;
	pStartConvClockTemp = StartConvClockTemp_DS3232;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool InitClock_RV3129()
{
	DSCI2C dsc;

	byte reg = 8;
	byte buf[10];
	
	::RTC t;

	buf[0] = 0x18;
	buf[1] = 31;	// Timer Low
	buf[2] = 0;		// Timer High

	dsc.adr = ADR_RV3129;
	dsc.wdata = buf;
	dsc.wlen = 3;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	if (!dsc.ack) return false;

	buf[0] = 0x00;
	buf[1] = 0x9F;	// Control_1
	buf[2] = 0x02;	// Control_INT: TIE

	dsc.adr = ADR_RV3129;
	dsc.wdata = buf;
	dsc.wlen = 3;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	if (!dsc.ack) return false;

	dsc.adr = ADR_RV3129;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();	// { HW::WDT->Update(); };

	if (!dsc.ack) return false;

	t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
	t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
	t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
	t.day	= (buf[3]&0xF) + ((buf[3]>>4)*10);
	t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
	t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

	SetTime(t);

	pSetClock = SetClock_RV3129;
	pReqClockTemp = ReqClockTemp_RV3129;
	pGetClockTemp = GetClockTemp_RV3129;
	pStartConvClockTemp = StartConvClockTemp_RV3129;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitClock()
{
	using namespace HW;

	VectorTableExt[CLOCK_IRQ] = Clock_IRQ;
	CM4::NVIC->CLR_PR(CLOCK_IRQ);
	CM4::NVIC->SET_ER(CLOCK_IRQ);	

	PIOB->DIRSET = 1 << 7;

	PIO_RTC->SetWRCONFIG(RTCINT, PORT_PMUX_A|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);

	if ((EIC->CTRLA & EIC_ENABLE) == 0)
	{
		EIC->CTRLA = EIC_SWRST;
		while(EIC->SYNCBUSY);
	}
	else
	{
		EIC->CTRLA = 0;
		while(EIC->SYNCBUSY);
	};

	EIC->EVCTRL &= ~(1 << CLOCK_EXTINT);
	EIC->SetConfig(CLOCK_EXTINT, 1, EIC_SENSE_FALL);
	EIC->ASYNCH |= 1<<CLOCK_EXTINT;
	EIC->INTENSET = 1 << CLOCK_EXTINT;
	EIC->CTRLA = EIC_ENABLE;

	if (InitClock_DS3232())
	{

	}
	else if (InitClock_RV3129())
	{

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_XMC48

u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init)
{
	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	__packed const byte *s = (__packed const byte*)data;

	for ( ; len > 0; len--)
	{
		CRC_FCE->IR = *(s++);
	};

	//if (len > 0)
	//{
	//	CRC_FCE->IR = *(s++)&0xFF;
	//}

	return CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	HW::P1->BSET(9);

	byte *s = (byte*)data;

	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

//	if ((u32)s & 1) { CRC_FCE->IR = *s++; len--; };

	if (len > 0)
	{
		CRC_DMACH->CTLH = BLOCK_TS(len);

		CRC_DMACH->SAR = (u32)s;

		CRC_DMA->CHENREG = CRC_DMA_CHEN;

		while(CRC_DMA->CHENREG & (1<<2));
	};

	//if (len & 1) { CRC_FCE->IR = s[len-1]; };

	HW::P1->BCLR(9);

	return (byte)CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	HW::Peripheral_Enable(PID_FCE);

	HW::FCE->CLC = 0;
	CRC_FCE->CFG = 0;

	CRC_DMA->DMACFGREG = 1;

	CRC_DMACH->CTLL = DST_NOCHANGE|SRC_INC|DST_TR_WIDTH_8|SRC_TR_WIDTH_8|TT_FC_M2M_GPDMA|DEST_MSIZE_1|SRC_MSIZE_1;
	CRC_DMACH->DAR = (u32)&CRC_FCE->IR;
	CRC_DMACH->CFGL = 0;
	CRC_DMACH->CFGH = PROTCTL(1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#elif defined(CPU_SAME53)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	HW::PIOC->BSET(26);

	T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	dmadsc.DESCADDR = 0;
	dmadsc.DSTADDR = (void*)init;
	dmadsc.SRCADDR = (byte*)data+len;
	dmadsc.BTCNT = len;
	dmadsc.BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC;

	HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x20+CRC_DMACH);

	dmach.INTENCLR = ~0;
	dmach.INTFLAG = ~0;
	dmach.CTRLA = DMCH_ENABLE/*|DMCH_TRIGACT_TRANSACTION*/;

	HW::DMAC->SWTRIGCTRL = 1UL << CRC_DMACH;

	while (((dmach.CTRLA & DMCH_ENABLE) != 0) && (dmach.INTFLAG & DMCH_TCMPL) == 0);

	HW::PIOC->BCLR(26);

	return ReverseWord(HW::DMAC->CRCCHKSUM);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CRC_CCITT_DMA_Async(const void *data, u32 len, u16 init)
{
	HW::PIOC->BSET(26);

	T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	dmadsc.DESCADDR = 0;
	dmadsc.DSTADDR = (void*)init;
	dmadsc.SRCADDR = (byte*)data+len;
	dmadsc.BTCNT = len;
	dmadsc.BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC;

	HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x20+CRC_DMACH);

	dmach.INTENCLR = ~0;
	dmach.INTFLAG = ~0;
	dmach.CTRLA = DMCH_ENABLE/*|DMCH_TRIGACT_TRANSACTION*/;

	HW::DMAC->SWTRIGCTRL = 1UL << CRC_DMACH;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16 *crc)
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	if ((dmach.CTRLA & DMCH_ENABLE) == 0 || (dmach.INTFLAG & DMCH_TCMPL))
	{
		*crc = ReverseWord(HW::DMAC->CRCCHKSUM);

		HW::PIOC->BCLR(26);

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	//T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	//HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x3F);
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WDT_Init()
{
	#ifdef CPU_SAME53	

		HW::MCLK->APBAMASK |= APBA_WDT;

		HW::WDT->CONFIG = WDT_WINDOW_CYC512|WDT_PER_CYC1024;
	
		#ifndef _DEBUG
		HW::WDT->CTRLA = WDT_ENABLE|WDT_WEN;
		#else
		HW::WDT->CTRLA = WDT_ENABLE|WDT_WEN;
		#endif

		while(HW::WDT->SYNCBUSY);

	#elif defined(CPU_XMC48)

		HW::WDT_Enable();

		HW::WDT->WLB = OFI_FREQUENCY/2;
		HW::WDT->WUB = (3 * OFI_FREQUENCY)/2;
		HW::SCU_CLK->WDTCLKCR = 0|SCU_CLK_WDTCLKCR_WDTSEL_OFI;

		#ifndef _DEBUG
		HW::WDT->CTR = WDT_CTR_ENB_Msk|WDT_CTR_DSP_Msk;
		#else
		HW::WDT->CTR = WDT_CTR_ENB_Msk;
		#endif

	#endif
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i16 GetDeviceTemp()
{
	return temp;
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

			if (tm.Check(101))
			{
				if (pReqClockTemp(dsc))
				{
					i++;
				};
			};

			break;

		case 1:

			if (dsc.ready)
			{
				tempClock = (dsc.ack && dsc.readedLen == dsc.rlen) ? pGetClockTemp(dsc) : -273;

				i++;
			};

			break;

		case 2:

			if (pStartConvClockTemp(dsc2))
			{
				i++;
			};

			break;

		case 3:

			if (dsc2.ready)
			{
				buf[0] = 0;

				dsc.adr = 0x49;
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

		case 4:

			if (dsc.ready)
			{
				if (dsc.ack && dsc.readedLen == dsc.rlen)
				{
					i32 t = (i16)ReverseWord(rbuf);

					temp = (t * 10 + 64) / 128;
				};
				//else
				//{
				//	temp = -2730;
				//};

#ifdef CPU_SAME53	

				i = 0;
			};

			break;

#elif defined(CPU_XMC48)

				HW::SCU_GENERAL->DTSCON = SCU_GENERAL_DTSCON_START_Msk;
				
				i++;
			};

			break;

		case 5:

			if (HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RDY_Msk)
			{
				cpu_temp = ((i32)(HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RESULT_Msk) - 605) * 1000 / 205;

				i = 0;
			};

			break;
#endif
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	using namespace HW;

#ifdef CPU_SAME53	

//	HW::PIOA->BSET(13);

	HW::GCLK->GENCTRL[GEN_32K]	= GCLK_DIV(1)	|GCLK_SRC_OSCULP32K	|GCLK_GENEN;

	HW::GCLK->GENCTRL[GEN_1M]	= GCLK_DIV(25)	|GCLK_SRC_XOSC1		|GCLK_GENEN		|GCLK_OE;

	HW::GCLK->GENCTRL[GEN_25M]	= GCLK_DIV(1)	|GCLK_SRC_XOSC1		|GCLK_GENEN;

//	HW::GCLK->GENCTRL[GEN_500K] = GCLK_DIV(50)	|GCLK_SRC_XOSC1		|GCLK_GENEN;


	HW::MCLK->APBAMASK |= APBA_EIC;
	HW::GCLK->PCHCTRL[GCLK_EIC] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;


	HW::MCLK->APBBMASK |= APBB_EVSYS;


	HW::GCLK->PCHCTRL[GCLK_SERCOM_SLOW]		= GCLK_GEN(GEN_32K)|GCLK_CHEN;	// 32 kHz
	HW::GCLK->PCHCTRL[GCLK_SERCOM5_CORE]	= GCLK_GEN(GEN_MCK)|GCLK_CHEN;	

	HW::MCLK->APBDMASK |= APBD_SERCOM5;


#endif

	Init_time();
	NAND_Init();
	I2C_Init();
	InitClock();

	//InitManTransmit();
	InitManTransmit_2();
	InitManRecieve();
	Init_CRC_CCITT_DMA();
	
	WDT_Init();

	EnableVcore();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateTemp();

//	static byte i = 0;

//	static Deb db(false, 20);

//	#define CALL(p) case (__LINE__-S): p; break;

//	enum C { S = (__LINE__+3) };
//	switch(i++)
//	{
//		CALL( Update_AD5312();		);
//	};

//	i = (i > (__LINE__-S-3)) ? 0 : i;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
