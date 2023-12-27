// Shell functions

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "faults.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: If these were written in assembly
//           omit this file and add a faults.s file

// REQUIRED: code this function
void mpuFaultIsr(void)
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

// REQUIRED: code this function
void hardFaultIsr(void)
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

// REQUIRED: code this function
void busFaultIsr(void)
{
    putsUart0("\nBus fault in process N\n");

    while(1)
    {

    }
}

// REQUIRED: code this function
void usageFaultIsr(void)
{
    putsUart0("\nUsage fault in process");
    NVIC_SYS_HND_CTRL_R &=~NVIC_SYS_HND_CTRL_USAGEP;

    while(1)
    {

    }
}


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