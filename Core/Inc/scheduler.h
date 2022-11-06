/*
 * scheduler.h
 *
 *  Created on: Nov 6, 2022
 *      Author: fef0
 */

// Must fit in a uint8_t
#define MAXTASKS 255
#define STACKSIZE 128

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


// It's a circular linked list
struct tcbList {
	tcbNode* head;
	tcbNode* tail;
	uint8_t tasksNum;
};
typedef struct tcbList tcbListType;

void initList();
uint8_t isListEmpty();
uint32_t* createStack();
void addTask(void (*taskFunc)(), uint8_t taskID);
void task0();
void startScheduler();

