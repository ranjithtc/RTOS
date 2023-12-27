// Kernel functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef KERNEL_H_
#define KERNEL_H_

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// mutex
#define MAX_MUTEXES 1
#define MAX_MUTEX_QUEUE_SIZE 2
#define resource 0

// semaphore
#define MAX_SEMAPHORES 3
#define MAX_SEMAPHORE_QUEUE_SIZE 2
#define keyPressed 0
#define keyReleased 1
#define flashReq 2

// tasks
#define MAX_TASKS 12

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex);
bool initSemaphore(uint8_t semaphore, uint8_t count);

void initRtos(void);
void startRtos(void);

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes);
void restartThread(_fn fn);
void stopThread(_fn fn);
void setThreadPriority(_fn fn, uint8_t priority);

void yield(void);
void sleep(uint32_t tick);
void lock(int8_t mutex);
void unlock(int8_t mutex);
void wait(int8_t semaphore);
void post(int8_t semaphore);

void systickIsr(void);
void pendSvIsr(void);
void svCallIsr(void);
void pidint(uint32_t pid);

int priorityscheduler(void);

void pidofsv(char * p);
void ipcsv(void);
void killsv(uint32_t num);
void  pkillsv(char *temp);
void  runsv(char *temp);
void schedsv(char * p);
void  pssv(void);
void  rebootsv(void);
void convert(uint32_t pid);
void  preemptsv(char *temp);


#endif
