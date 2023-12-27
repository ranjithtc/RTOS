// Tasks


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "tasks.h"
#include "gpio.h"
#include "kernel.h"
#include "shell.h"
#include "tm4c123gh6pm.h"
#include "wait.h"
#include <stdbool.h>
#include <stdint.h>

extern setunPrivilege(void);

#define BLUE_LED PORTF, 2   // on-board blue LED
#define RED_LED PORTA, 5    // off-board red LED
#define ORANGE_LED PORTA, 7 // off-board orange LED
#define YELLOW_LED PORTB, 4 // off-board yellow LED
#define GREEN_LED PORTA, 6  // off-board green LED
#define RED_LED_MASK 2
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons
void initHw(void) {
  // Setup LEDs and pushbuttons

  // Power-up flash
  /* setPinValue(GREEN_LED, 1);
   waitMicrosecond(250000);
   setPinValue(GREEN_LED, 0);
   waitMicrosecond(250000);
*/

  /////////////
  // Enable clocks
  SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5 | SYSCTL_RCGCGPIO_R0 |
                      SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R3 |
                      SYSCTL_RCGCGPIO_R4;
  _delay_cycles(3);

  selectPinPushPullOutput(PORTA, 5);
  selectPinPushPullOutput(PORTA, 6);
  selectPinPushPullOutput(PORTA, 7);
  selectPinPushPullOutput(PORTB, 4);
  selectPinPushPullOutput(PORTF, 2);

  selectPinDigitalInput(PORTD, 2);
  selectPinDigitalInput(PORTB, 0);
  selectPinDigitalInput(PORTB, 1);
  selectPinDigitalInput(PORTE, 4);
  selectPinDigitalInput(PORTB, 5);
  selectPinDigitalInput(PORTE, 2);

  enablePinPullup(PORTD, 2);
  enablePinPullup(PORTB, 0);
  enablePinPullup(PORTB, 1);
  enablePinPullup(PORTE, 4);
  enablePinPullup(PORTB, 5);
  enablePinPullup(PORTE, 2);

  // Configuring SysTick Timer
  NVIC_ST_RELOAD_R = 39999;
  NVIC_ST_CURRENT_R = 0;
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_ENABLE;

  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1; // enable the timer1 clock

  TIMER1_CTL_R &= ~TIMER_CTL_TAEN;         // disable before setting value
  TIMER1_CFG_R = 0x00000000;               // 32 bit timer
  TIMER1_TAMR_R |= TIMER_TAMR_TAMR_PERIOD; // periodic
  TIMER1_TAMR_R |= TIMER_TAMR_TACDIR;      // up count

  /*   // Configure LED pins
     GPIO_PORTF_DIR_R |=   RED_LED_MASK;  // bits 3 are outputs
     GPIO_PORTF_DR2R_R |=  RED_LED_MASK; // set drive strength to 2mA (not
     needed since default configuration -- for clarity) GPIO_PORTF_DEN_R |=
     RED_LED_MASK;  // enable LEDs
     ////////////////////////////////////
 */
  // setPinValue(BLUE_LED,1);
}

// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are
// pressed
uint8_t readPbs(void) {
  if (!getPinValue(PORTD, 2))
    return 1;

  if (!getPinValue(PORTB, 0))
    return 2;

  if (!getPinValue(PORTB, 1))
    return 4;

  if (!getPinValue(PORTE, 4))
    return 8;

  if (!getPinValue(PORTB, 5))
    return 16;
  if (!getPinValue(PORTE, 2))
    return 32;

  return 0;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void) {
  while (true) {
    // setPinValue(PORTF, 1); //On-board Red LED

    setPinValue(ORANGE_LED, 1);
    waitMicrosecond(1000);
    setPinValue(ORANGE_LED, 0);

    yield();
  }
}

void idle2(void) {
  while (true) {
    togglePinValue(PORTF, 2); // On-board Blue LED
    //setPinValue(BLUE_LED, 1);
    waitMicrosecond(1000);
    setPinValue(RED_LED, 1);
    yield();
  }
}
void idle3(void) {
  while (true) {
    togglePinValue(PORTF, 2); // On-board Blue LED
    //setPinValue(BLUE_LED, 1);
    waitMicrosecond(1000);
    setPinValue(RED_LED, 1);
    yield();
  }
}

void flash4Hz(void) {
  while (true) {
    setPinValue(GREEN_LED, !getPinValue(GREEN_LED));

    sleep(125);
  }
}

void oneshot(void) {
  while (true) {
    wait(flashReq);
    setPinValue(YELLOW_LED, 1);
    sleep(1000);
    setPinValue(YELLOW_LED, 0);
  }
}

void partOfLengthyFn(void) {
  // represent some lengthy operation
  waitMicrosecond(990);
  // give another process a chance to run
  yield();
}

void lengthyFn(void) {

  uint16_t i;
  while (true) {

    lock(resource);
    for (i = 0; i < 5000; i++) {
      partOfLengthyFn();
    }
    setPinValue(RED_LED, !getPinValue(RED_LED));
    unlock(resource);
  }
}

void readKeys(void) {
  uint8_t buttons;
  while (true) {
    wait(keyReleased);
    buttons = 0;
    while (buttons == 0) {
      buttons = readPbs();
      yield();
    }
    post(keyPressed);
    if ((buttons & 1) != 0) {
      setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
      setPinValue(RED_LED, 1);
    }
    if ((buttons & 2) != 0) {
      post(flashReq);
      setPinValue(RED_LED, 0);
    }
    if ((buttons & 4) != 0) {
      // restartThread(flash4Hz);
      char *p = "Flash4Hz";
      runsv(p);
    }
    if ((buttons & 8) != 0) {
      // stopThread(flash4Hz);
      char *p = "Flash4Hz";
      pkillsv(p);
    }
    if ((buttons & 16) != 0) {
      setThreadPriority(lengthyFn, 4);
    }
    yield();
  }
}

void debounce(void) {
  uint8_t count;
  while (true) {
    wait(keyPressed);
    count = 10;
    while (count != 0) {
      sleep(10);
      if (readPbs() == 0)
        count--;
      else
        count = 10;
    }
    post(keyReleased);
  }
}

void uncooperative(void) {
  while (true) {
    while (readPbs() == 8) {
    }
    yield();
  }
}

void errant(void) {
  uint32_t *p = (uint32_t *)0x20000000;

  // uint32_t* p = (uint32_t*)0x20002018;
  // uint32_t* p = (uint32_t*)0x20004000;
  // *p = 0;
  //(*((volatile uint32_t *)0x20006004)) = 11;

  while (true) {
    while (readPbs() == 32) {
      *p = 0;
    }
    yield();
  }
}

void important(void) {
  while (true) {
    lock(resource);
    setPinValue(BLUE_LED, 1);
    sleep(1000);
    setPinValue(BLUE_LED, 0);
    unlock(resource);
  }
}
