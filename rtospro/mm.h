// Memory manager functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

#define NUM_SRAM_REGIONS 5

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocFromHeap(uint32_t size_in_bytes);
void initMpu(void);
void generateSramSrdMasks(uint8_t srdMask[NUM_SRAM_REGIONS],void *p,uint32_t size_in_bytes);
void applySramSrdMask(uint8_t srdMask,uint8_t k,uint8_t curr);

void overallaccess(void);
void allowFlashAccess(void);
void allowPeripheralAccess(void);
void block8rule(uint32_t ,uint32_t);
void block4rule(uint32_t ,uint32_t);
void setupSramAccess(void);



#endif
