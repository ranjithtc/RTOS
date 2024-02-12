// Memory manager functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "mm.h"
#include "clock.h"
#include "gpio.h"
#include "nvic.h"
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "wait.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


static uint32_t *memptr4 = (uint32_t *)0x20001000; // heap pointer
static uint32_t *memptr8 = (uint32_t *)0x20002400; // heap pointer
static int session[40]; // assigned to zero by default
static int present;
static uint32_t *blockptr = (uint32_t *)0x20001E00;
static int number[40]; // for the number allocated at each session for the free
                       // in malloc
static int n;
uint32_t a;
uint32_t b;
uint32_t res;

#define REGIONPART1 0x20001000
#define REGIONPART2 0x20002000
#define REGIONPART3 0x20004000
#define REGIONPART4 0x20005000
#define REGIONPART5 0x20006000

#define REGIONPRIORITY0 0x0
#define REGIONPRIORITY1 0x1
#define REGIONPRIORITY2 0x2
#define REGIONPRIORITY3 0x3
#define REGIONPRIORITY4 0x4
#define REGIONPRIORITY5 0x5
#define REGIONPRIORITY6 0x6
#define REGIONPRIORITY7 0x7

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add your malloc code here and update the SRD bits for the current
// thread
void *mallocFromHeap(uint32_t size_in_bytes) 
{
  uint32_t memptrcpy4 = (uint32_t)memptr4;
  uint32_t *start4 = memptr4;
  uint32_t memptrcpy8 = (uint32_t)memptr8;
  uint32_t *start8 = memptr8;
  uint32_t blockptrcpy = (uint32_t)blockptr;
  uint32_t *start1536 = blockptr;

  uint32_t aligned_size, curr = 0, i = 0, flag8 = 0, flag4 = 0, flag1536 = 0,count = 0;

  if (size_in_bytes <= 512) 
  {
    aligned_size = 512;
  } 
  else if (size_in_bytes <= 1024) 
  {
    aligned_size = 1024;
  } 
  else if (size_in_bytes <= 1536) 
  {
    aligned_size = 1536;
  } 
  else if (size_in_bytes <= 2048) 
  {
    aligned_size = 2048;
  } 
  else if (size_in_bytes <= 3072) 
  {
    aligned_size = 3072;
  } 
  else if (size_in_bytes <= 4096) 
  {
    aligned_size = 4096;
  } 
  else 
  {
    return NULL; // Requested size is too large
  }

  if ((aligned_size % 1024 == 0) && (memptrcpy8 < 0x20008000))
    curr = 8;
  else if ((aligned_size % 1024 == 0) && (memptrcpy8 >= 0x20007FFF) && (aligned_size != 1536))
    curr = 4;
  else if ((aligned_size % 512 == 0) && (memptrcpy4 < 0x20005E00) && (aligned_size != 1536))
    curr = 4;
  else if ((aligned_size % 1536 == 0) && (memptrcpy4 < 0x20006400))
    curr = 1536;
  else
    return NULL;

  // 0x20005E00 just before 20006000
  // 0x200 / 4 = 0x80 (you can t do 0x200 for pointer)
  while ((i < aligned_size) && ((memptrcpy4 < 0x20005E00)) && (aligned_size != 1536) && curr == 4) 
  {
    flag4 = 1;
    if ((memptrcpy4 >= 0x20001000) && (memptrcpy4 < 0x20001E00))    //&&(memptrcpy4 < 20001E00) )//0x20002000 �
                                                                    //0x200 = 0x20001E00 fault line
    {
      curr = 4;
      i += 512;
      session[present++] = 512;
      memptr4 += 0x80;
      memptrcpy4 += 0x200;
      count++;
      continue;
    } 
    else if ((memptrcpy4 >= 0x20004200) && (memptrcpy4 < 0x20005E00) && (aligned_size != 1536)) 
    {
      curr = 4;
      i += 512;
      session[present++] = 512;
      memptr4 += 0x80;
      memptrcpy4 += 0x200;
      count++;
      continue;
    }
    // Continuation of 4k block ie is the second block next jump
    // Hex value: 0x20004200 - 0x20001E00 = 0x2400
    else if ((memptrcpy4 == 20001E00) && (aligned_size != 1536)) 
    {
      // 0x2400 / 4 = 0x900 Steps.
      memptrcpy4 += 0x2400;
      memptr4 = (uint32_t *)memptrcpy4;
    }
    // else if((memptrcpy4 == 0x20004E00) && (aligned_size != 1536))
    // {
    //   memptrcpy4 += 0x2400;
    //   memptr4 = (uint32_t *)memptrcpy4;
    // }
  }

  while ((i < aligned_size) && (memptrcpy8 < 0x20008000) && (aligned_size != 1536) && curr == 8) 
  {
    flag8 = 1;
    if ((memptrcpy8 >= 0x20002400) && (memptrcpy8 < 0x20003C00) && (aligned_size != 1536)) 
    {
      curr = 8;
      i += 1024;
      session[present++] = 1024;
      memptrcpy8 += 0x400;
      memptr8 = (uint32_t *)memptrcpy8;
      count++;
      continue;
    } 
    else if ((memptrcpy8 >= 0x20006400) && (memptrcpy8 < 0x20008000) && (aligned_size != 1536)) 
    {
      curr = 8;
      i += 1024;
      session[present++] = 1024;
      memptrcpy8 += 0x400;
      memptr8 = (uint32_t *)memptrcpy8;
      count++;
      continue;
    }
    // Hex value: 0x20006400 - 0x20003C00 = 0x2800  8k Block  Next Jump
    else if ((memptrcpy8 == 0x20003C00) && (aligned_size != 1536)) 
    {
      memptrcpy8 += 0x2800;
      memptr8 = (uint32_t *)memptrcpy8;
    }
  }

  // Need to consume the 1536 blocks if we have the both the 512 and 1024 blocks occupied
  if ((memptrcpy4 >= 0x20005E00) && (memptrcpy8 < 0x20008000) && (aligned_size != 1536)) 
  {
    flag1536 = 1;
    while ((blockptrcpy >= 0x20001E00) && (blockptrcpy < 0x20006400) && (i < aligned_size)) 
    {
      if (blockptrcpy == 0x20001E00) 
      {
        blockptrcpy += 0x200;
        blockptr = (uint32_t *)blockptrcpy;
        i += 512;
        session[present++] = 512;
        count++;
        continue;
      } 
      else if (blockptrcpy == 0x20002000) 
      {
        blockptrcpy += 0x400;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1024;
        session[present++] = 1024;
        count++;
        continue;
      } 
      else if (blockptrcpy == 0x20002400) 
      {
        //Hex value:20003C00 - 20002400 = 1800
        blockptrcpy += 0x1800;
        blockptr = (uint32_t *)blockptrcpy;
      } 
      else if (blockptrcpy == 0x20003C00) 
      {
        blockptrcpy += 0x400;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1024;
        session[present++] = 1024;
        count++;
        continue;
      } 
      else if (blockptrcpy == 0x20004000) 
      {
        blockptrcpy += 0x200;
        blockptr = (uint32_t *)blockptrcpy;
        i += 512;
        session[present++] = 512;
        count++;
        continue;

      } 
      else if (blockptrcpy == 0x20004200) 
      {
        //Hex value:20005E00 - 20004200 = 1C00
        blockptrcpy += 0x1C00;
        blockptr = (uint32_t *)blockptrcpy;
      } 
      else if (blockptrcpy == 0x20005E00) 
      {
        blockptrcpy += 0x200;
        blockptr = (uint32_t *)blockptrcpy;
        i += 512;
        session[present++] = 512;
        count++;
        continue;

      } 
      else if (blockptrcpy == 0x20006000) 
      {
        blockptrcpy += 0x400;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1024;
        session[present++] = 1024;
        count++;
        continue;
      }
    }
  }

  if ((aligned_size == 1536) && (blockptrcpy >= 0x20001E00) && (blockptrcpy < 0x20006400)) 
  {
    flag1536 = 1;
    while (i < aligned_size)
      if (blockptrcpy == 0x20001E00) 
      {
        // Increment with 1536
        blockptrcpy += 0x600;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1536;
        session[present++] = 1536;
        count++;
        continue;

      } 
      else if (blockptrcpy == 0x20002400) 
      {
        //Hex value:20003C00 � 20002400 = 1800
        blockptrcpy += 0x1800;
        blockptr = (uint32_t *)blockptrcpy;
        start1536 = blockptr;
      } 
      else if (blockptrcpy == 0x20003C00) 
      {
        blockptrcpy += 0x600;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1536;
        session[present++] = 1536;
        count++;
        continue;
      } 
      else if (blockptrcpy == 0x20004200) 
      {
        // Hex value:20005E00 � 20004200 = 1C00
        blockptrcpy += 0x1C00;
        blockptr = (uint32_t *)blockptrcpy;
        start1536 = blockptr;
      } 
      else if (blockptrcpy == 0x20005E00) 
      {
        blockptrcpy += 0x600;
        blockptr = (uint32_t *)blockptrcpy;
        i += 1536;
        session[present++] = 1536;
        count++;
        continue;
      }
  }

  uint32_t a = (uint32_t)memptr4;
  uint32_t b = (uint32_t)memptr8;
  uint32_t c = (uint32_t)blockptr;

  if ((blockptrcpy >= 0x20006400) && (memptrcpy4 >= 0x20005E00) && (memptrcpy8 >= 0x20008000)) 
  {
    return NULL;
  }
  if ((flag4 == 1) && (flag8 == 0) && (flag1536 == 0))
    return (uint32_t *)(a);
  if ((flag4 == 0) && (flag8 == 1) && (flag1536 == 0))
    return (uint32_t *)(b);
  if ((flag4 == 0) && (flag8 == 0) && (flag1536 == 1))
    return (uint32_t *)(c);

  return 0;
}

// REQUIRED: add your custom MPU functions here (eg to return the srd bits)
// REQUIRED: initialize MPU here

void initMpu(void) 
{
  // REQUIRED: call your MPU functions here

  NVIC_SYS_HND_CTRL_R |= (NVIC_SYS_HND_CTRL_BUS | NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_MEM);

  // NVIC_INT_CTRL_R|= NVIC_INT_CTRL_PEND_SV;

  // NVIC_CFG_CTRL_R |= NVIC_CFG_CTRL_DIV0;
  overallaccess();
  allowFlashAccess();
  allowPeripheralAccess();
  setupSramAccess();
  NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;
}

void overallaccess() 
{
  NVIC_MPU_BASE_R = NVIC_MPU_BASE_VALID | REGIONPRIORITY0;

  NVIC_MPU_ATTR_R = NVIC_MPU_ATTR_XN | 3 << 24 | (0x1F << 1) | NVIC_MPU_ATTR_ENABLE;
                                                                                    // 3<<24
                                                                                    // if implementing all access (no backgnd what
}

void allowFlashAccess() 
{
  NVIC_MPU_NUMBER_R |= REGIONPRIORITY1;
  NVIC_MPU_BASE_R =(0x00000000 | REGIONPRIORITY1);  // not including the base address as i
                                                    // base is 0x00000000
  NVIC_MPU_ATTR_R = ((3 << 24) | (17 << 1) | NVIC_MPU_ATTR_ENABLE); // ap bit is set for full access
                                                                    // 256 when calculated gave 17
}

void allowPeripheralAccess() 
{
  NVIC_MPU_BASE_R = (0x20000000 | NVIC_MPU_BASE_VALID | 0x2);
  // 0x0000000C size after calculating for 2^(n+1)
  NVIC_MPU_ATTR_R =(NVIC_MPU_ATTR_XN | (1 << 24) | (0xB << 1) | NVIC_MPU_ATTR_ENABLE);
}

void setupSramAccess() 
{
  block4rule(REGIONPART1, REGIONPRIORITY3);
  block8rule(REGIONPART2, REGIONPRIORITY4);
  block4rule(REGIONPART3, REGIONPRIORITY5);
  block4rule(REGIONPART4, REGIONPRIORITY6);
  block8rule(REGIONPART5, REGIONPRIORITY7);
}

//  SRAM region   is   fast enough for most applications, and adding
//  bufferability might not provide significant performance improvements.

void block8rule(uint32_t baseaddr, uint32_t regionpriority) 
{
  //  NVIC_MPU_NUMBER_R | = regionpriority;

  // Base address register
  NVIC_MPU_BASE_R = (baseaddr | NVIC_MPU_BASE_VALID | regionpriority);
  // 0x0000000C size after calculating for 2^(n+1)
  NVIC_MPU_ATTR_R = (NVIC_MPU_ATTR_XN | (1 << 24) | (0xC << 1) | NVIC_MPU_ATTR_ENABLE);
  // 0x01000000 =1<<24  ap bit----  privileged(rw) and  unprivileged(no access)
  // accordign to the document
}

void block4rule(uint32_t baseaddr, uint32_t regionpriority) 
{
  // Base address register
  NVIC_MPU_BASE_R = (baseaddr | NVIC_MPU_BASE_VALID | regionpriority);
  // 0x0000000B size after calculating for 2^(n+1)
  NVIC_MPU_ATTR_R = (NVIC_MPU_ATTR_XN | (1 << 24) | (0xB << 1) | NVIC_MPU_ATTR_ENABLE);
  // 0x01000000 =1<<24 which is the rw acess priv .ap bit----  privileged(rw)
  // and  unprivileged(no access) accordign to the document
}

void applySramSrdMask(uint8_t srdMask, uint8_t k, uint8_t curr) 
{
  uint8_t i = k, mask;
  uint32_t region;
  mask = srdMask;

  if (i == 0)
    region = REGIONPRIORITY3;
  else if (i == 1)
    region = REGIONPRIORITY4;
  else if (i == 2)
    region = REGIONPRIORITY5;
  else if (i == 3)
    region = REGIONPRIORITY6;
  else if (i == 4)
    region = REGIONPRIORITY7;

  NVIC_MPU_NUMBER_R = region;
  NVIC_MPU_ATTR_R |= (srdMask << 8);
  // NVIC_MPU_ATTR_R |= (1<<8);
}