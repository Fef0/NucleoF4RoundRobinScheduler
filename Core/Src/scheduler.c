/*
 * scheduler.c
 *
 *  Created on: Nov 3, 2022
 *      Author: fef0
 */

#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"

// It's a linked queue of Task Control Blocks
tcbQueueType* tasksQueue;

// Points to the current task
tcbNode* currentTask;

void taskReturn() {
	while(1);
}

void initQueue() {
	// Dynamically allocate space for tasksQueue and init it
	tasksQueue = (tcbQueueType*) malloc(sizeof(tcbQueueType));
	tasksQueue->head = NULL;
	tasksQueue->tail = NULL;
	tasksQueue->tasksNum = 0;
}

uint8_t isQueueEmpty() {
	return tasksQueue->tasksNum == 0;
}

void initStack(uint32_t* stackPt, void (*taskFunc)()) {
	// STACK FRAME {R0-R3, R12, LR, PC, xPSR}
	// xPSR default (from Cortex m4 manual)
	stackPt[STACKSIZE-1] = 0x01000000;
	// PC will point to the function itself
	stackPt[STACKSIZE-2] = (uint32_t) taskFunc;
	// stack frame LR will contain a default return address
	stackPt[STACKSIZE-3] = (uint32_t) taskReturn;
	// Init R12, R3, R2, R1, R0
	// R12: stackPt[STACKSIZE-4] = 0
	// R3: stackPt[STACKSIZE-5] = 0
	// ...
	// R0: stackPt[STACKSIZE-8] = 0
	for (int i=4; i<9; i++) {
		stackPt[STACKSIZE-i] = 0;
	}

	// Software LR will contain an EXC_RETURN that tells
	// the processor to switch to thread mode and to use PSP
	// (from cortex manual)
	stackPt[STACKSIZE-9] = 0xFFFFFFFD;

	// Init the rest of the software stack {R4-R11}
	// R11: stackPt[STACKSIZE-10] = 0
	// ...
	// R4: stackPt[STACKSIZE-17] = 0
	for (int i=10; i<=17; i++) {
		stackPt[STACKSIZE-i] = 0;
	}
}

uint32_t* createStack(void (*taskFunc)()) {
	// Dynamically create the new stack and update the pointer
	uint32_t* stackPt = (uint32_t*) malloc(sizeof(stack));

	// Fill the stack with default values
	initStack(stackPt, taskFunc);

	// The stack will grow from the bottom up so
	// the stack pointer must point to the latest element (top of stack)
	return &(stackPt[STACKSIZE-17]);
}


// Add a new task to the tasks queue
void addTask(void (*taskFunc)()) {
	// Allocate space for a new Task Control Block
	tcbNode* t = (tcbNode*) malloc(sizeof(tcbNode));
	// Create the stack
	t->stackPt = createStack(taskFunc);
	t->taskID = tasksQueue->tasksNum;
	// If the queue is empty then initialize head and tail at the same position
	// Otherwise update the last element to point to the new one and update the tail
	if (isQueueEmpty()) {
		tasksQueue->head = tasksQueue->tail = t;
		// Update currentTask pointer to point to the first
		// task added
		currentTask = t;
	} else {
		tasksQueue->tail->next = t;
		tasksQueue->tail = t;
	}

	// The last element will now point to the head of the circular queue
	tasksQueue->tail->next = tasksQueue->head;
	// Update tasks number
	tasksQueue->tasksNum++;
}


void forceTaskSwitch() {
	// Force a context switch through PendSV interrupt
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void taskYield() {
	forceTaskSwitch();
}

// Config SysTick
void sysTickInit(uint32_t quanta_us) {
	// Disable the SysTick timer
	SysTick->CTRL = 0;
	//Clear current value to 0
	SysTick->VAL = 0;
	// Load SysTick to generate an interrupt after a quanta
	SysTick->LOAD = (quanta_us * MICRO_PRESCALER)-1;
	// Enable SysTick
	SysTick->CTRL =0x00000007;
}

void SysTick_Handler() {
	forceTaskSwitch();
}


void initScheduler(uint32_t quanta_us) {
	// Initialize the circular linked queue
	initQueue();

	// Initialize the system clock
	sysTickInit(quanta_us);
}

void startScheduler() {
	// Call SVC_Handler and load the first task
	__asm("svc 0");
}

