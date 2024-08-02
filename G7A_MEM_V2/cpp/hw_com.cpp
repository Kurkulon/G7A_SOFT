//#pragma O3
//#pragma Otime

//#include <stdio.h>
//#include <conio.h>

#include <ComPort\ComPort.h>
#include "hw_conf.h"
#include "hw_com.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CPU_SAME53

ComPort com0	(UART0_MEM, PIO_USCK0, PIO_UTXD0, PIO_URXD0, PIO_RTS0, PIN_USCK0, PIN_UTXD0, PIN_URXD0, PIN_RTS0, PMUX_USCK0, PMUX_UTXD0, PMUX_URXD0, UART0_TXPO, UART0_RXPO, UART0_GEN_SRC, UART0_GEN_CLK, &UART0_DMA);

#elif defined(CPU_XMC48)

ComPort com0	(UART0_USIC_NUM, PIO_USCK0, PIO_UTXD0, PIO_URXD0, PIO_RTS0, PIN_USCK0, PIN_UTXD0, PIN_URXD0, PIN_RTS0, MUX_USCK0, MUX_UTXD0, UART0_DX0CR, UART0_DX1CR, &UART0_DMA, UART0_DRL);	

#elif defined(WIN32)

ComPort commoto;	
ComPort comdsp;		

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <ComPort\ComPort_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
