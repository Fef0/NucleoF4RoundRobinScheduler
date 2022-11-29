/*
 * scheduler.h
 *
 *  Created on: Nov 6, 2022
 *      Author: fef0
 */
#define BUS_FREQ 84000000
#define MICRO_PRESCALER BUS_FREQ/1000000
#define STACKSIZE 256

// A stack will contain the task registers
typedef uint32_t stack[STACKSIZE];

// Task Control Block will point to its relative stack pointer
// And will be linked to the next task control block
struct tcb {
	uint32_t* stackPt;
	struct tcb* next;
	uint8_t taskID;
};
typedef struct tcb tcbNode;


// It's a circular queue
struct tcbQueue {
	tcbNode* head;
	tcbNode* tail;
	uint8_t tasksNum;
};
typedef struct tcbQueue tcbQueueType;

void initList();
uint8_t isListEmpty();

uint32_t* createStack();
void addTask(void (*taskFunc)());
void idleTask();
void sysTickInit(uint32_t quanta_us);
void task0();
void task1();
void initScheduler(uint32_t quanta_us);
void startScheduler();

