;/*****************************************************************************
; * @file     startup_same53.s
; * @brief    CMSIS Cortex-M4 Core Device Startup File for
; *           Atmel SAME53 Device Series
; * @version  V1.0.0
; * @date     16. January 2017
; *
; * @note
; * Copyright (C) 2017 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


VecTableIntSize	EQU		16*4	
VecTableExtSize	EQU		137*4	
SCB_VTOR		EQU		0xE000ED08

				AREA	||.ARM.__AT_0x20000000||, DATA, NOINIT, ALIGN=7
                EXPORT  VectorTableInt
                EXPORT  VectorTableExt
VectorTableInt	SPACE	VecTableIntSize				
VectorTableExt	SPACE	VecTableExtSize				

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, CODE, READONLY, ALIGN=4

				DCD     __initial_sp              ;    Top of Stack
                DCD     Reset_Handler             ;    Reset Handler


                ALIGN	16
                DCB		"\n", __PROGNAME__, "\n"
                DCB		__DATE__, "\n"
                DCB		__TIME__, "\n"

                ALIGN	16

_MainAppStart	PROC
				EXPORT	_MainAppStart
				
				LDR		R1, [R0]
				MOV		SP, R1
				LDR		R0, [R0, #4]
				BX		R0

                ENDP


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                
                
                MOVS	R0, #(VecTableIntSize+VecTableExtSize-4)
                LDR		R1, =VectorTableInt
                LDR		R2, =Dummy_Handler
|L1.16|
				STR		R2, [R1,R0]
                SUBS	R0, #4
                BNE		|L1.16|
                
                LDR		R0, =SCB_VTOR 
                STR		R1, [R0]		;CM0::SCB->VTOR = VectorTableInt

                
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; Dummy Exception Handlers (infinite loops which can be modified)

;[SP+0x00] = R[0];
;[SP+0x04] = R[1];
;[SP+0x08] = R[2];
;[SP+0x0C] = R[3];
;[SP+0x10] = R[12];
;[SP+0x14] = LR;
;[SP+0x18] = ReturnAddress();
;[SP+0x1C] = xPSR;


Dummy_Handler   PROC

				LDR		R0,[SP,#0x18]	; R0 = return Address
				MRS		R1, IPSR		; R1 = Exception number
				BKPT	#0
				BX		lr
                ENDP

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
