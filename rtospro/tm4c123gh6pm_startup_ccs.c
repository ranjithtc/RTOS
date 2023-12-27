//*****************************************************************************
//
// Startup code for use with TI's Code Composer Studio.
//
// Copyright (c) 2011-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "clock.h"
#include "gpio.h"
#include "nvic.h"
#include "wait.h"


extern  void svCallIsr(void);
extern  void pendSvIsr(void);
extern  void systickIsr(void);
//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
void ResetISR(void);
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);
void busfaulthandler(void);
void usagefaulthandler(void);
//void pendsvfaulthandler(void);
void mpufaulthandler(void);
void portBhandler(void);

//*****************************************************************************
//
// External declaration for the reset handler that is to be called when the
// processor is started
//
//*****************************************************************************
extern void _c_int00(void);

//*****************************************************************************
//
// Linker variable that marks the top of the stack.
//
//*****************************************************************************
extern uint32_t __STACK_TOP;

//*****************************************************************************
//
// External declarations for the interrupt handlers used by the application.
//
//*****************************************************************************
// To be added by user

extern  uint32_t getPSP(void);
extern  uint32_t getMSP(void);




//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000 or at the start of
// the program if located at a start address other than 0.
//
//*****************************************************************************
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
                                            // The initial stack pointer
    ResetISR,                               // The reset handler
    NmiSR,                                  // The NMI handler
    FaultISR,                               // The hard fault handler
    mpufaulthandler,                      // The MPU fault handler
    busfaulthandler,                      // The bus fault handler
    usagefaulthandler,                      // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    svCallIsr,                              // SVCall handler
    IntDefaultHandler,                      // Debug monitor handler
    0,                                      // Reserved
    pendSvIsr,                              // The PendSV handler
    systickIsr,                      // The SysTick handler
    IntDefaultHandler,                      // GPIO Port A
    portBhandler,                      // GPIO Port B
    IntDefaultHandler,                      // GPIO Port C
    portBhandler,                      // GPIO Port D
    portBhandler,                      // GPIO Port E
    IntDefaultHandler,                      // UART0 Rx and Tx
    IntDefaultHandler,                      // UART1 Rx and Tx
    IntDefaultHandler,                      // SSI0 Rx and Tx
    IntDefaultHandler,                      // I2C0 Master and Slave
    IntDefaultHandler,                      // PWM Fault
    IntDefaultHandler,                      // PWM Generator 0
    IntDefaultHandler,                      // PWM Generator 1
    IntDefaultHandler,                      // PWM Generator 2
    IntDefaultHandler,                      // Quadrature Encoder 0
    IntDefaultHandler,                      // ADC Sequence 0
    IntDefaultHandler,                      // ADC Sequence 1
    IntDefaultHandler,                      // ADC Sequence 2
    IntDefaultHandler,                      // ADC Sequence 3
    IntDefaultHandler,                      // Watchdog timer
    IntDefaultHandler,                      // Timer 0 subtimer A
    IntDefaultHandler,                      // Timer 0 subtimer B
    IntDefaultHandler,                      // Timer 1 subtimer A
    IntDefaultHandler,                      // Timer 1 subtimer B
    IntDefaultHandler,                      // Timer 2 subtimer A
    IntDefaultHandler,                      // Timer 2 subtimer B
    IntDefaultHandler,                      // Analog Comparator 0
    IntDefaultHandler,                      // Analog Comparator 1
    IntDefaultHandler,                      // Analog Comparator 2
    IntDefaultHandler,                      // System Control (PLL, OSC, BO)
    IntDefaultHandler,                      // FLASH Control
    IntDefaultHandler,                      // GPIO Port F
    IntDefaultHandler,                      // GPIO Port G
    IntDefaultHandler,                      // GPIO Port H
    IntDefaultHandler,                      // UART2 Rx and Tx
    IntDefaultHandler,                      // SSI1 Rx and Tx
    IntDefaultHandler,                      // Timer 3 subtimer A
    IntDefaultHandler,                      // Timer 3 subtimer B
    IntDefaultHandler,                      // I2C1 Master and Slave
    IntDefaultHandler,                      // Quadrature Encoder 1
    IntDefaultHandler,                      // CAN0
    IntDefaultHandler,                      // CAN1
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // Hibernate
    IntDefaultHandler,                      // USB0
    IntDefaultHandler,                      // PWM Generator 3
    IntDefaultHandler,                      // uDMA Software Transfer
    IntDefaultHandler,                      // uDMA Error
    IntDefaultHandler,                      // ADC1 Sequence 0
    IntDefaultHandler,                      // ADC1 Sequence 1
    IntDefaultHandler,                      // ADC1 Sequence 2
    IntDefaultHandler,                      // ADC1 Sequence 3
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port J
    IntDefaultHandler,                      // GPIO Port K
    IntDefaultHandler,                      // GPIO Port L
    IntDefaultHandler,                      // SSI2 Rx and Tx
    IntDefaultHandler,                      // SSI3 Rx and Tx
    IntDefaultHandler,                      // UART3 Rx and Tx
    IntDefaultHandler,                      // UART4 Rx and Tx
    IntDefaultHandler,                      // UART5 Rx and Tx
    IntDefaultHandler,                      // UART6 Rx and Tx
    IntDefaultHandler,                      // UART7 Rx and Tx
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // I2C2 Master and Slave
    IntDefaultHandler,                      // I2C3 Master and Slave
    IntDefaultHandler,                      // Timer 4 subtimer A
    IntDefaultHandler,                      // Timer 4 subtimer B
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // Timer 5 subtimer A
    IntDefaultHandler,                      // Timer 5 subtimer B
    IntDefaultHandler,                      // Wide Timer 0 subtimer A
    IntDefaultHandler,                      // Wide Timer 0 subtimer B
    IntDefaultHandler,                      // Wide Timer 1 subtimer A
    IntDefaultHandler,                      // Wide Timer 1 subtimer B
    IntDefaultHandler,                      // Wide Timer 2 subtimer A
    IntDefaultHandler,                      // Wide Timer 2 subtimer B
    IntDefaultHandler,                      // Wide Timer 3 subtimer A
    IntDefaultHandler,                      // Wide Timer 3 subtimer B
    IntDefaultHandler,                      // Wide Timer 4 subtimer A
    IntDefaultHandler,                      // Wide Timer 4 subtimer B
    IntDefaultHandler,                      // Wide Timer 5 subtimer A
    IntDefaultHandler,                      // Wide Timer 5 subtimer B
    IntDefaultHandler,                      // FPU
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // I2C4 Master and Slave
    IntDefaultHandler,                      // I2C5 Master and Slave
    IntDefaultHandler,                      // GPIO Port M
    IntDefaultHandler,                      // GPIO Port N
    IntDefaultHandler,                      // Quadrature Encoder 2
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port P (Summary or P0)
    IntDefaultHandler,                      // GPIO Port P1
    IntDefaultHandler,                      // GPIO Port P2
    IntDefaultHandler,                      // GPIO Port P3
    IntDefaultHandler,                      // GPIO Port P4
    IntDefaultHandler,                      // GPIO Port P5
    IntDefaultHandler,                      // GPIO Port P6
    IntDefaultHandler,                      // GPIO Port P7
    IntDefaultHandler,                      // GPIO Port Q (Summary or Q0)
    IntDefaultHandler,                      // GPIO Port Q1
    IntDefaultHandler,                      // GPIO Port Q2
    IntDefaultHandler,                      // GPIO Port Q3
    IntDefaultHandler,                      // GPIO Port Q4
    IntDefaultHandler,                      // GPIO Port Q5
    IntDefaultHandler,                      // GPIO Port Q6
    IntDefaultHandler,                      // GPIO Port Q7
    IntDefaultHandler,                      // GPIO Port R
    IntDefaultHandler,                      // GPIO Port S
    IntDefaultHandler,                      // PWM 1 Generator 0
    IntDefaultHandler,                      // PWM 1 Generator 1
    IntDefaultHandler,                      // PWM 1 Generator 2
    IntDefaultHandler,                      // PWM 1 Generator 3
    IntDefaultHandler                       // PWM 1 Fault
};


 void bintohextostr(uint32_t num)
 {
     char str[33];
     int i = 0;
     while (num)
     {
         uint8_t extract = 0;
         extract = num & 0xF;
         if (extract < 9)
             str[i++] = extract + '0';
         else
         {
             if (extract == 10)
                 str[i++] = 'A';
             if (extract == 11)
                 str[i++] = 'B';
             if (extract == 12)
                 str[i++] = 'C';
             if (extract == 13)
                 str[i++] = 'D';
             if (extract == 14)
                 str[i++] = 'E';
             if (extract == 15)
                 str[i++] = 'F';
         }
         num = num >> 4;
     }
     str[i] = '\0';
     int  start = 0;
     char temp;
     int end = i - 1;
     while (start < end)
     {
         temp = str[start];
         str[start] = str[end];
         str[end] = temp;
         start++;
         end--;
     }
     putsUart0(str);
 }




//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called.  Any fancy
// actions (such as making decisions based on the reset cause register, and
// resetting the bits in that register) are left solely in the hands of the
// application.
//
//*****************************************************************************
void
ResetISR(void)
{
    //
    // Jump to the CCS C initialization routine.  This will enable the
    // floating-point unit as well, so that does not need to be done here.
    //
    __asm("    .global _c_int00\n"
          "    b.w     _c_int00");
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
NmiSR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************


static void
FaultISR(void)
{
    putsUart0("\n Hard fault in process N\n");

    volatile uint32_t  psp = getPSP();
        volatile uint32_t  msp = getMSP(); // Get the Main Stack Pointer
        volatile uint32_t hfault = NVIC_HFAULT_STAT_R ;//first derefernce it



        putsUart0("\n PSP= 0x");
        bintohextostr(psp);
        putcUart0('\n');

        putsUart0("\n MSP=0x");
       bintohextostr(msp);
        putcUart0('\n');

        putsUart0("\n Hard fault flag=0x");
        bintohextostr(hfault);
        putcUart0('\n');
        while(1)
        {

        }



}


//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
    //
    // Go into an infinite loop.
    //
    while(1)
    {
    }
}






void busfaulthandler()
{
    putsUart0("\nBus fault in process N\n");
//    int a = 34;
 //       int b = 0;
 //       int res;
 //       res=a/b;
    while(1)
    {

    }


}

void usagefaulthandler(void)
{
    putsUart0("\nUsage fault in process");
   // NVIC_SYS_HND_CTRL_R &=~NVIC_SYS_HND_CTRL_USAGEP;

    while(1)
    {

    }
}




void mpufaulthandler()
{
    putsUart0("\nMPU fault in process N\n");
      uint32_t * psp = (uint32_t *)getPSP();
      uint32_t * msp = (uint32_t *)getMSP(); // Get the Main Stack Pointer

      uint32_t  pspval =getPSP();
      uint32_t  mspval =getMSP();

    putsUart0("\n PSP= 0x");
    bintohextostr(pspval);
    putcUart0('\n');

    putsUart0("\n MSP=0x");
    bintohextostr(mspval);
    putcUart0('\n');

    putsUart0("\n mfaultflags= 0x");
    volatile uint32_t mfaultflags = NVIC_FAULT_STAT_R  & 0xFF;
    bintohextostr(mfaultflags);

  volatile uint32_t memfaultaddress = NVIC_MM_ADDR_R;
  bintohextostr(memfaultaddress);
     putcUart0('\n');

    uint32_t var=0;

    putsUart0("\nStack Dump for R0,R1,R2,R3,R12,LR,PC,xPSR is as follows");
    putsUart0("\n R0= 0x");

    var=(uint32_t)psp;
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nR1= 0x");
    var=(uint32_t)(psp + 1);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nR2= 0x");
    var=(uint32_t)(psp+2);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nR3= 0x");
    var=(uint32_t)(psp+3);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nR12= 0x");
    var=(uint32_t)(psp+4);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nLR= 0x");
    var=(uint32_t)(psp+5);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nPC= 0x");
    var=(uint32_t)(psp+6);
    bintohextostr(var);
    putcUart0('\n');

    putsUart0("\nxPSR= 0x");
    var=(uint32_t)(psp+7);
    bintohextostr(var);
    putcUart0('\n');

    NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_MEMP;
    NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;

}






void portBhandler()
{

    if(!getPinValue(PORTB, 0)) //blue button
    {
        togglePinValue(PORTF, 1); //On-board Red LED

           int a = 34;
              int b = 0;
              int res;
              res=a/b;



    }
    /*
    else if(!getPinValue(PORTD, 2)) //white button
    {
        while(1)
        {
            togglePinValue(PORTF, 1); //On-board Red LED
            waitMicrosecond(1000000); //wait 1s

        }
    }

    else if(!getPinValue(PORTE, 4)) //
    {
        while(1)
        {
            togglePinValue(PORTF, 2); //On-board Blue LED
            waitMicrosecond(1000000); //wait 1s
        }
    }
*/
    clearPinInterrupt(PORTB, 0);
   // clearPinInterrupt(PORTD, 2);
    //clearPinInterrupt(PORTE, 4);

}

/*
void pendsvfaulthandler()
{
    putsUart0("\nPendsv in process N");
    volatile uint32_t x,y;

    x= NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_IERR;
    y= NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DERR;
    if(x)
    {
        NVIC_FAULT_STAT_R &=~NVIC_FAULT_STAT_IERR;
    }
    if(y)
    {
        NVIC_FAULT_STAT_R &= ~NVIC_FAULT_STAT_DERR;
    }
    putsUart0("\ncalled from MPU");
    NVIC_INT_CTRL_R|=NVIC_INT_CTRL_UNPEND_SV;
    while(1)
    {

    }

 }

*/
