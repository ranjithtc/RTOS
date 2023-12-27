// Kernel functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "kernel.h"
#include "mm.h"
#include "shell.h"
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include <stdint.h>


extern setASP(void);
extern setPSP(uint32_t);
extern setPrivilege(void);
extern setunPrivilege(void);
extern saveregs(void);
extern popregs(void);
extern uint32_t getPSP(void);
extern uint32_t getMSP(void);
extern xpsrtolr(uint32_t, uint32_t, uint32_t);
extern pushremainingone(uint32_t, uint32_t, uint32_t);
extern pushremainingtwo(uint32_t, uint32_t);
extern uint32_t getsvc(void);
static int roundone = 1; // global for pritoiryt sched one pass
uint8_t type = 0;
static int counter = 0;
uint8_t change = 0;

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------
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

// Mutex
typedef struct _mutex {
  bool lock;
  uint8_t queueSize;
  uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
  uint8_t lockedBy;
} mutex;
mutex mutexes[MAX_MUTEXES];

// Semaphore
typedef struct _semaphore {
  uint8_t count;
  uint8_t queueSize;
  uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

// Task states
#define STATE_INVALID 0           // no task
#define STATE_STOPPED 1           // stopped, can be resumed
#define STATE_UNRUN 2             // task has never been run
#define STATE_READY 3             // has run, can resume at any time
#define STATE_DELAYED 4           // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX 5     // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 6 // has run, but now blocked by semaphore

// Task
uint8_t taskCurrent = 0; // index of last dispatched task
uint8_t taskCount = 0;   // total number of valid tasks

// Control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = false;          // preemption (true) or cooperative (false)

// TCB
#define NUM_PRIORITIES 8
struct _tcb {
  uint8_t state;           // see STATE_ values above
  void *pid;               // used to uniquely identify thread (add of task fn)
  void *spInit;            // original top of stack
  void *sp;                // current stack pointer
  uint8_t priority;        // 0=highest
  uint8_t currentPriority; // 0=highest (needed for pi)
  uint32_t ticks;          // ticks until sleep complete
  uint8_t srd[NUM_SRAM_REGIONS]; // MPU subregion disable bits
  char name[16];                 // name of task used in ps command
  uint8_t mutex;     // index of the mutex in use or blocking the thread
  uint8_t semaphore; // index of the semaphore that is blocking the thread
  uint64_t clocks[2];
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex) {
  bool ok = (mutex < MAX_MUTEXES);
  if (ok) {
    mutexes[mutex].lock = false;
    mutexes[mutex].lockedBy = 0;
  }
  return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count) {
  bool ok = (semaphore < MAX_SEMAPHORES);
  { semaphores[semaphore].count = count; }
  return ok;
}

// REQUIRED: initialize systick for 1ms system timer
void s(void) {
  uint8_t i;
  // no tasks running
  taskCount = 0;
  // clear out tcb records
  for (i = 0; i < MAX_TASKS; i++) {
    tcb[i].state = STATE_INVALID;
    tcb[i].pid = 0;
  }
}

uint8_t k, reg[8], track;

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void) {

  if (priorityScheduler == 0) 
  {
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok) {
      task++;
      if (task >= MAX_TASKS)
        task = 0;
      ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
    }
    return task;
  }

  if (priorityScheduler == 1) 
  {
    int i, j, start, end;
    int present = 0;

    for (i = 0; i < 8; i++) 
    {
      if (present + 1 == MAX_TASKS)
        start = 0;
      else
        start = present + 1;

      if (present == MAX_TASKS)
        end = 0;
      else
        end = present;

      if (start < end) {
        while (start < end) {
          if (tcb[start].priority == i)
            if (tcb[start].state == STATE_READY ||
                tcb[start].state == STATE_UNRUN)
              return start;

          if (start == MAX_TASKS)
            start = 0;
          else
            start++;
        }

      } 
      else if ((start > end)) 
      {
        while (start > end) 
        {
          if (tcb[start].priority == i)
            if (tcb[start].state == STATE_READY ||
                tcb[start].state == STATE_UNRUN)
              return start;
          if (start == MAX_TASKS)
            start = 0;
          else
            start++;
        }
        while (start < end) 
        {
          if (tcb[start].priority == i)
            if (tcb[start].state == STATE_READY ||
                tcb[start].state == STATE_UNRUN)
              return start;
          if (start == MAX_TASKS)
            start = 0;
          else
            start++;
        }
      }
    }
  }
}

// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, TMPL bit, and PC
void startRtos(void) 
{
  // since msp if we reference it earlier it will be pointing to the msp
  // for multiple tasks you need a timer
  // if(type==0)
  taskCurrent = rtosScheduler();
  // else if (type==1)
  // taskCurrent=priorityscheduler();
  // dont dereference since we want pc to point to the address and not the value
  // pc sets
  applySramSrdMask((tcb[taskCurrent].srd[reg[taskCurrent]]), reg[taskCurrent],taskCurrent);
  setPSP((uint32_t)tcb[taskCurrent].sp);
  setASP();
  _fn funptr;
  funptr = (_fn)tcb[taskCurrent].pid; // point to the current top

  tcb[taskCurrent].state = STATE_READY;

  setunPrivilege();
  funptr();
}

// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation

bool createThread(_fn fn, const char name[], uint8_t priority,uint32_t stackBytes) 
{
  bool ok = false;
  uint8_t i = 0;
  static uint32_t currbase, nextbase;
  uint8_t j;
  bool found = false;
  if (taskCount < MAX_TASKS) {
    // make sure fn not already in list (prevent reentrancy)
    while (!found && (i < MAX_TASKS)) {
      found = (tcb[i++].pid == fn);
    }
    if (!found) {
      // find first available tcb record
      i = 0;
      while (tcb[i].state != STATE_INVALID) {
        i++;
      }
      tcb[i].state = STATE_UNRUN;
      tcb[i].pid = fn;

      for (j = 0; j < NUM_SRAM_REGIONS; j++)
        tcb[i].srd[j] = 0;

      // we need to calulate  where our stack pointer will be pointing to for
      // that wee make use of the calculations available
      if (i == 0) {
        currbase = 0x20001000;
        nextbase = (uint32_t)mallocFromHeap(stackBytes);
      } else {
        nextbase = (uint32_t)mallocFromHeap(stackBytes);
      }

      /// generate the srd mask ofc
      uint32_t base, temp, curr = 0, part = 0, region, a = 0;

      temp = (uint32_t)(nextbase - stackBytes);
      // nextbase-stackBytes

      if (stackBytes) {
        while (a < stackBytes) {
          if ((temp >= 0x20001000) && (temp < 0x20002000)) {

            base = 0x20001000;
            curr = 4;
            a += 512;
            region = REGIONPRIORITY3;
            k = 0;
          }

          if ((temp >= 0x20002000) && (temp < 0x20004000)) {
            base = 0x20002000;
            curr = 8;
            a += 1024;
            region = REGIONPRIORITY4;
            k = 1;
          }

          if ((temp >= 0x20004000) && (temp < 0x20005000)) {
            base = 0x20004000;
            curr = 4;
            a += 512;
            region = REGIONPRIORITY5;
            k = 2;
          }

          if ((temp >= 0x20005000) && (temp < 0x20006000)) {
            base = 0x20005000;
            curr = 4;
            a += 512;
            region = REGIONPRIORITY6;
            k = 3;
          }

          if ((temp >= 0x20006000) && (temp <= 0x20007FFF)) {
            base = 0x20006000;
            curr = 8;
            a += 1024;
            region = REGIONPRIORITY7;
            k = 4;
          }

          if (temp > 0x20007FFF)
            return 0;

          if (curr == 4) {
            uint32_t div = 0x200;
            part = temp - base;
            part = part / div;
            tcb[i].srd[k] |=
            1 << part; // adding so that we can  utilize this later on
            //  subregionenable( region,1<< part );
            temp += 0x200;
          }

          if (curr == 8) {
            uint32_t div = 0x400;
            part = temp - base;
            part = part / div;
            tcb[i].srd[k] |= 1 << part;
            // subregionenable(region ,1<< part);
            temp += 0x400;
          }
        }
      }

      reg[track++] = k;

      // end of the generate srd mask
      currbase = nextbase;

      tcb[i].sp = (uint8_t *)(nextbase - 1);
      tcb[i].spInit = (uint8_t *)(nextbase - 1);

      tcb[i].ticks = 0;

      uint8_t t = 0;
      while (name[t] != '\0') {
        tcb[i].name[t] = name[t];
        t++;
      }

      tcb[i].name[t] = '\0';

      tcb[i].priority = priority;

      tcb[i].semaphore = 0;
      tcb[i].mutex = 0;
      // increment task count
      taskCount++;
      ok = true;
    }
  }
  return ok;
}

int priorityscheduler() {}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn) 
{
  int i = 0;
  for (i = 0; i < MAX_TASKS; i++) {
    if (tcb[i].pid == fn) {
      tcb[i].sp = tcb[i].spInit;
      tcb[i].state = STATE_UNRUN;
      break;
    }
  }
}

// REQUIRED: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn) 
{
  int i = 0, index, j, k = 0;
  for (i = 0; i < MAX_TASKS; i++) {
    if ((tcb[i].pid) == fn) {
      tcb[i].state = STATE_STOPPED;
      index = i;
      for (j = 0; j < MAX_SEMAPHORES; j++) {
        for (k = 0; k < MAX_SEMAPHORE_QUEUE_SIZE; k++) {
          // which task is that
          if (semaphores[j].processQueue[k] == index) {
            while (k < semaphores[j].queueSize) {
              if (k <= semaphores[j].queueSize - 2) {
                semaphores[j].processQueue[k] =
                semaphores[j].processQueue[k + 1];
                semaphores[j].processQueue[k + 1] = 0;
                semaphores[j].queueSize--;
                /// not incrementing coutn since it is being transferred from
                /// the next process (already incremented )
              } else if (k == semaphores[j].queueSize - 1) {
                semaphores[j].processQueue[k] = 0;
                semaphores[j].queueSize--;
              }
              k++;
            }
          }
        }
      }
    }
  }
  // mutex
  int flag, pass;
  for (i = 0; i < MAX_TASKS; i++) {
    if ((tcb[i].pid) == fn) {
      tcb[i].state = STATE_STOPPED;
      index = i;
      for (j = 0; j < MAX_MUTEXES; j++) {
        flag = 0, pass = 0;
        if ((mutexes[j].lockedBy == index) && pass == 0) {
          mutexes[j].lock = 0;
          flag = 1;
          pass = 1;
        }
        for (k = 0; k < MAX_MUTEX_QUEUE_SIZE; k++) {
          if ((flag == 1) && pass == 1) {
            tcb[mutexes[j].processQueue[0]].state = STATE_READY;
            mutexes[j].lockedBy = mutexes[j].processQueue[0];

            mutexes[j].processQueue[0] = mutexes[j].processQueue[1];
            mutexes[j].processQueue[1] = 0;
            mutexes[j].queueSize--;
          }
          // which task is that
          else if ((mutexes[j].processQueue[k]) == index) {
            // mark the next task as ready is ????
            //  tcb[mutexes[j].processQueue[k+1]].state=STATE_READY;

            while (k < mutexes[j].queueSize) {
              if (k <= mutexes[j].queueSize - 2) {
                mutexes[j].processQueue[k] = mutexes[j].processQueue[k + 1];
                mutexes[j].processQueue[k + 1] = 0;
                mutexes[j].queueSize--;
                /// not incrementing coutn since it is being transferred from
                /// the next process (already incremented )
              } else if (k == mutexes[j].queueSize - 1) {
                mutexes[j].processQueue[k] = 0;
                mutexes[j].queueSize--;
              }
              k++;
            }
          }

          /// wake up next waiter how to do it ?
          // pkilll
          // show run
          flag = 0;
        }
      }
    }
  }
}

//

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority) {}

// REQUIRED: modify this function to yield execution back to scheduler using
// pendsv
void yield(void) { __asm("  SVC #1"); }

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick) { __asm("  SVC #2"); }

// REQUIRED: modify this function to lock a mutex using pendsv
void lock(int8_t mutex) { __asm("  SVC #3"); }

// REQUIRED: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex) { __asm("  SVC #4"); }

// REQUIRED: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore) { __asm("  SVC #5"); }

// REQUIRED: modify this function to signal a semaphore is available using
// pendsv
void post(int8_t semaphore) { __asm("  SVC #6"); }

void schedsv(char *p) { __asm("  SVC #7"); }

void pidofsv(char *p) { __asm("  SVC #8"); }

void ipcsv(void) { __asm("  SVC #9"); }

void killsv(uint32_t num) { __asm("  SVC #10"); }

void pkillsv(char *temp) { __asm("  SVC #11"); }

void runsv(char *temp) { __asm("  SVC #12"); }

void pssv() { __asm("  SVC #13"); }

void rebootsv() { __asm("  SVC #14"); }

void preemptsv(char *temp) { __asm("  SVC #15"); }

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void) 
{
  int j = 0;
  // for 1 sec and 1ms is then it should tick 1000 times in order for it to go
  // off

  if (counter == 1000) {

    if ((change % 2) == 0)
      change = 1;
    else if ((change % 2) != 0)
      change = 0;

    counter = 0;

    for (j = 0; j < MAX_TASKS; j++) {
      tcb[j].clocks[change] = 0;
    }
  }
  counter++;

  int i = 0;
  while (i < 12) // total tasks
  {
    if (tcb[i].state == STATE_DELAYED) {
      if (tcb[i].ticks > 0)
        tcb[i].ticks--;

      if (tcb[i].ticks == 0) {
        tcb[i].state = STATE_READY;
      }
    }
    i++;
  }

  if (preemption == 1)
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
}

void pidint(uint32_t pid) 
{
  static char str[12];
  int i = 0;

  if (pid == 0) {
    str[i++] = 0;
  }

  while (pid != 0) {
    int rem = pid % 10;
    str[i++] = rem + '0';
    pid = pid / 10;
  }
  str[i] = '\0';

  // Reverse the string
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }

  putsUart0("  pid no is ");
  putsUart0(str);
  putsUart0("   ");
}

// REQUIRED: in coop and preemptive, modify this function to add support for
// task switching REQUIRED: process UNRUN and READY tasks differently
void pendSvIsr(void) 
{
  __asm("   MRS R1, PSP");
  __asm("   SUB R1,#4");
  __asm("   STR LR,[R1]");
  __asm("   MSR PSP,R1");

  __asm("   MRS R1, PSP ");
  __asm("   SUB R1,#4");
  __asm("   STR R4,[R1]");
  __asm("   SUB R1,#4");
  __asm("   STR R5,[R1]");

  __asm("   SUB R1,#4");
  __asm("    STR R6,[R1]");

  __asm("       SUB R1,#4");
  __asm("    STR R7,[R1]");

  __asm("     SUB R1,#4");
  __asm("     STR R8,[R1]");

  __asm("      SUB R1,#4");
  __asm("       STR R9,[R1]");

  __asm("       SUB R1,#4");
  __asm("     STR R10,[R1]");

  __asm("      SUB R1,#4");
  __asm("      STR R11,[R1]");

  __asm("      MSR PSP,R1");

  tcb[taskCurrent].sp = (void *)getPSP();
  TIMER1_CTL_R &= ~TIMER_CTL_TAEN; // stop and then get the value
  tcb[taskCurrent].clocks[change] += TIMER1_TAV_R;

  taskCurrent = rtosScheduler();

  // applySramSrdMask((tcb[taskCurrent].srd[k]),k,taskCurrent);

  applySramSrdMask((tcb[taskCurrent].srd[reg[taskCurrent]]), reg[taskCurrent],taskCurrent);

  if (tcb[taskCurrent].state == STATE_READY) {
    // start of next task enable timer
    TIMER1_TAV_R = 0;
    TIMER1_CTL_R |= TIMER_CTL_TAEN;
    // you need to point to the current task using the psp so set it to the
    // current task
    setPSP((uint32_t)tcb[taskCurrent].sp);
    //  popregs();

    __asm("  MRS R0, PSP");

    __asm("  LDR R11,[R0] ");
    __asm("  ADD R0,#4");

    __asm("  LDR R10,[R0] ");
    __asm("   ADD R0,#4");

    __asm("   LDR R9,[R0] ");
    __asm("    ADD R0,#4");

    __asm("   LDR R8,[R0] ");
    __asm("   ADD R0,#4");

    __asm("   LDR R7,[R0] ");
    __asm("   ADD R0,#4");

    __asm("  LDR R6,[R0] ");
    __asm("   ADD R0,#4");

    __asm("  LDR R5,[R0] ");
    __asm("  ADD R0,#4");

    __asm("  LDR R4,[R0] ");
    __asm("  ADD R0,#4");

    __asm("  MSR PSP,R0");

    __asm("   MRS R0, PSP");
    __asm("   LDR LR, [R0]");
    __asm("   ADD R0,#4");
    __asm("   MSR PSP,R0");

  } 
  else 
  {
    // start of next task enable timer
    TIMER1_TAV_R = 0;
    TIMER1_CTL_R |= TIMER_CTL_TAEN;
    // current psp to work with
    setPSP((uint32_t)tcb[taskCurrent].sp);
    // Setting the Thumb bit in the xPSR register indicates that the processor
    // is in Thumb state and should execute Thumb instructions . If the Thumb
    //bit is not set, the processor may attempt to execute ARM instructions
    xpsrtolr(0x01000000, (uint32_t)tcb[taskCurrent].pid,
             0xFFFFFFFD); // entering garbage
    pushremainingone(0x12, 0x12, 0x12);
    pushremainingtwo(0x12, 0x12);
    tcb[taskCurrent].state = STATE_READY;
  }
}

// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void) {
  /*  uint32_t* psp = getPSP();
    uint32_t* y = psp + 6; // Move the pointer to the stacked PC value
    uint16_t* currsvc = (uint16_t*)(*y - 2); // Subtract 2 from the return
    address to get the SVC instruction address as normally this will point to
    the next instruction uint8_t svcno = (uint8_t)(*currsvc);
    */
  // arm documentation :how to write an svc function
  // points to R0 (psp)
  uint8_t svcno = getsvc();

  uint32_t *val;

  static char str[12];
  char ch, st[2];
  int i = 0, j = 0;
  char *name;
  _fn fptr;
  uint8_t flagsem = 0, flagmut = 0;

  int stable;
  uint64_t total = 0, taskusage;
  uint32_t a, b;

  switch (svcno) {
  case 1:
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
    break;
  case 2:
    // ro stores the sleep time
    val = getPSP();
    tcb[taskCurrent].state = STATE_DELAYED;
    tcb[taskCurrent].ticks = *val;

    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
    break;

  case 3: // lock
    val = getPSP();
    if (mutexes[*val].lock == 0) {
      mutexes[*val].lock = 1;
      mutexes[*val].lockedBy = taskCurrent;
      tcb[taskCurrent].mutex = *val;

      return;
    } else {
      mutexes[*val].processQueue[mutexes[*val].queueSize] = taskCurrent;
      // sizxe check
      mutexes[*val].queueSize++;
      tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
    }
    break;

  case 4: // unlock
    val = getPSP();
    if ((mutexes[*val].lock == 1) && (mutexes[*val].lockedBy == taskCurrent)) {
      mutexes[*val].lock = 0;
      if (mutexes[*val].queueSize > 0) // get the index that is zero
      {
        uint8_t currmutex = mutexes[*val].processQueue[0];
        tcb[mutexes[*val].processQueue[0]].state =
            STATE_READY; // mark the first task as ready

        mutexes[*val].lock = 1;
        //   mutexes[*val].lockedBy = currmutex;
        mutexes[*val].processQueue[0] = mutexes[*val].processQueue[1]; // shift
        mutexes[*val].queueSize--;
        mutexes[*val].lockedBy = currmutex;
      }
    }

    break;
  case 5: // wait
    val = getPSP();
    if (semaphores[*val].count > 0) // indicates that some resource is available
    {
      semaphores[*val].count--;
      tcb[taskCurrent].semaphore = *val;
      return;
    }

    else {
      semaphores[*val].processQueue[semaphores[*val].queueSize] = taskCurrent;
      semaphores[*val].queueSize++;
      tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;
      NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
      break;
    }

  case 6: // post
    val = getPSP();
    semaphores[*val].count++;
    if (semaphores[*val].queueSize > 0) {
      uint8_t currsem = semaphores[*val].processQueue[0];
      tcb[currsem].state = STATE_READY;
      semaphores[*val].count--;
      int i;
      for (i = 1; i < semaphores[*val].queueSize; i++) {
        semaphores[*val].processQueue[i - 1] = semaphores[*val].processQueue[i];
      }
      semaphores[*val].queueSize--;
    }
    break;

  case 7: // sched
    val = getPSP();
    if (strcompare(*val, "prio")) {
      priorityScheduler = 1;

    } else if (strcompare(*val, "rr")) {
      priorityScheduler = 0;
    }
    break;

  case 8: // pid

    val = getPSP();
    name = (char *)*val;
    for (i = 0; i < taskCount; i++) {
      if (strcompare(tcb[i].name, name) == 1) {
        *val = (uint32_t)tcb[i].pid; // replace it with the current
        // imported from kill function
        pidint(*val);

        break;
      }
    }
    break;

  case 9: // ipcs

    for (i = 0; i < MAX_SEMAPHORES; i++) {
      // putsUart0("\n QUEUE info\n");
      if (semaphores[i].queueSize > 0) {

        flagsem = 1;
        //  putsUart0("\n name\n");
        //  putsUart0(tcb[semaphores[i]].name);

        putsUart0("\n queuesize");
        putcUart0(' ');
        ch = semaphores[i].queueSize + '0';
        str[0] = ch;
        str[1] = '\0';
        putsUart0(str);
        //   putcUart0('\r');
        //     putcUart0('  ');

        putsUart0(" count");
        putcUart0(' ');
        ch = semaphores[i].count + '0';
        str[0] = ch;
        str[1] = '\0';
        putsUart0(str);
        // putsUart0(semaphores[i].count + '0');
        //   putcUart0('\r');
        putcUart0(' ');

        // names of those waiting in the queue
        for (j = 0; j < semaphores[i].queueSize; j++) {
          putsUart0("  Waiting semaphore");
          putcUart0(' ');
          putsUart0(tcb[semaphores[i].processQueue[j]].name);
          putcUart0(' ');
          putcUart0(' ');
          putcUart0(' ');
        }
      }
    }
    if (flagsem == 0) {
      putsUart0("\n no semaphores ");
    }

    for (i = 0; i < MAX_MUTEXES; i++) {
      // putsUart0("\n QUEUE info\n");
      if (mutexes[i].queueSize > 0) {
        flagmut = 1;

        //   putsUart0(tcb[mutexes[i]].name);
        putcUart0('\n');
        putcUart0(' ');
        putcUart0(' ');
        putcUart0(' ');

        putsUart0(" queuesize ");
        putcUart0(' ');
        ch = mutexes[i].queueSize + '0';
        str[0] = ch;
        str[1] = '\0';
        putsUart0(str);
        //  putsUart0(mutexes[i].queueSize + '0');
        putcUart0(' ');
        putcUart0(' ');
        putcUart0(' ');

        putsUart0("  lockedBY");
        putcUart0(' ');
        putcUart0(' ');
        putcUart0(' ');

        ch = mutexes[i].lockedBy + '0';
        str[0] = ch;
        str[1] = '\0';
        putsUart0(tcb[mutexes[i].lockedBy].name);
        putsUart0(str);
        //  putsUart0(mutexes[i].lockedBy + '0');
        putcUart0(' ');
        putcUart0(' ');
        putcUart0(' ');

        //   putcUart0('\n');
        for (j = 0; j < mutexes[i].queueSize; j++) {
          putsUart0("    waiting mutex    ");
          putcUart0(' ');
          putsUart0(tcb[mutexes[i].processQueue[j]].name);
        }
      }
    }
    if (flagmut == 0) {
      putsUart0("\nno mutex");
    }
    break;

  case 10: // kill
    val = getPSP();

    stopThread((_fn)val);
    break;

  case 11: // pkill  // check the scheduler hwo this is beign done
    val = getPSP();
    char *name = (char *)*val;

    for (i = 0; i < taskCount; i++) {
      if (strcompare(tcb[i].name, name) == 1) {
        stopThread((_fn)tcb[i].pid);
        break;
      }
    }

    break;

  case 12: // run/restart
    val = getPSP();
    name = (char *)*val;

    for (i = 0; i < taskCount; i++) {
      if (strcompare(tcb[i].name, name) == 1) {
        restartThread((_fn)tcb[i].pid);
        break;
      }
    }

    break;

  case 13: // PS

    putsUart0("\n Name |            PID  |          Cpu time   ");

    if (change == 0)
      stable = 1;
    else if (change == 1)
      stable = 0;
    putsUart0("\n");
    for (i = 0; i < taskCount; i++) {
      total += tcb[i].clocks[stable];
    }

    for (i = 0; i < taskCount; i++) {
      // usage per task
      taskusage = (tcb[i].clocks[stable] * 25 * 100);
      taskusage /= total;

      putsUart0(" ");

      putsUart0(tcb[i].name);
      // putsUart0((char *)tcb[i].pid);
      putsUart0("    ");

      *val = (uint32_t)tcb[i].pid; // replace it with the current
      // imported from kill function
      pidint(*val);

      putsUart0("    ");

      convert(taskusage);

      putsUart0("    ");
      putsUart0("\n");
    }

    break;

  case 14: // reboot
    NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;

  case 15: // preemption
    val = getPSP();
    if (strcompare(*val, "on")) {
      preemption = 1;

    } else if (strcompare(*val, "off")) {
      preemption = 0;
    }
    break;
  }
}

void convert(uint32_t pid) {
  static char str[20];
  int i = 0;

  if (pid == 0) {
    str[i++] = '0';
  }

  while (pid != 0) {
    int rem = pid % 10;
    str[i++] = rem + '0';
    pid = pid / 10;
  }
  str[i] = '\0';

  // Reverse the string
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }
  putsUart0(str);
}