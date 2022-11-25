/*
 * scheduler.h
 *
 *  Created on: Nov 6, 2022
 *      Author: fef0
 */

#define MAXTASKS 10
#define STACKSIZE 256

// A stack will contain the register starting from the 128-16 = 112th position in this order:
// {r12-r0}, LR, PC, xPSR
typedef uint32_t stack[STACKSIZE];

// Task Control Block will point to its relative stack pointer
// And will be linked to the next thread control block
// And the previous one (will speed up hops)
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
void clockInit(uint32_t quanta);
void task0();
void task1();
void startScheduler();

