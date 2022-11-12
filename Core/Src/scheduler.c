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

#define BUS_FREQ   16000000
#define MILLIS_PRESCALER BUS_FREQ/1000

// It's a linked list of Task Control Blocks
tcbListType* tasksList;

// Points to the current task
tcbNode* currentTask;

void taskReturn() {
	while(1);
}

void initList() {
	// Dynamically allocate space for tasksList and init it
	tasksList = (tcbListType*) malloc(sizeof(tcbListType));
	tasksList->head = NULL;
	tasksList->tail = NULL;
	tasksList->tasksNum = 0;
}

uint8_t isListEmpty() {
	return tasksList->tasksNum == 0;
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


// The stack will contain {R0-R12}, LR, PC, xPSR and eventually the Floating Point registers {s0-s31}
// (not in this order)
uint32_t* createStack(void (*taskFunc)()) {
	// Dynamically create the new stack and update the pointer
	uint32_t* stackPt = (uint32_t*) malloc(sizeof(stack));

	// Fill the stack with default values
	initStack(stackPt, taskFunc);

	// The stack will grow from the bottom up so
	// the stack pointer must point to the latest element (top of stack)
	return &(stackPt[STACKSIZE-17]);
}

tcbNode* t1;
tcbNode* t2;

// Add a new task to the tasks list
int addTask(void (*taskFunc)(), uint8_t taskID) {
	// Allocate space for a new Thread Control Block
	tcbNode* t = (tcbNode*) malloc(sizeof(tcbNode));
	// Create the stack
	t->stackPt = createStack(taskFunc);
	// Set taskID
	t->taskID++;

	// If the list is empty then initialize head and tail at the same position
	// Otherwise update the last element to point to the new one and update the tail
	if (isListEmpty()) {
		tasksList->head = tasksList->tail = t;
		// Update currentTask pointer to point to the first
		// task added
		currentTask = t;
		t1 = t;
	} else {
		tasksList->tail->next = t;
		tasksList->tail = t;
		t2 = t;
	}

	// The last element will now point to the head of the circular list
	tasksList->tail->next = tasksList->head;
	// Update tasks number
	tasksList->tasksNum++;

	// Return taskID
	return t->taskID;
}

/*
// TODO: complete this
uint8_t removeTask(uint8_t taskID) {
	tcbNode* iter = tasksList->head;
	tcbNode* prev = NULL;
	while(1) {
		if (iter == NULL) {
			return 0;
		}

		if (iter->taskID == taskID) {
			// Case 1: the element to remove is the head
			if (iter == tasksList->head) {

			}
			// TODO don't forget to free the pointer
			return 1;
		}

		iter = iter->next;
	}
}
*/

int c1 = 0;
int c2 = 0;

void task0() {
	while(1) {
		c1++;
	}
}

void task1() {
	while(1) {
		c2++;
	}
}

void startScheduler() {
	// Initialize the circular linked list
	initList();

	// Add the first task
	addTask(&task0, 1);

	// Add the first second
	addTask(&task1, 2);

	// Config systick
	SysTick->CTRL =0;   //Disable the SysTick timer; Offset: 0x000 (R/W)  SysTick Control and Status Register
	SysTick->VAL=0;     //Clear current value to 0; Offset: 0x008 (R/W)  SysTick Current Value Register
	SysTick->LOAD = (0.01 * MILLIS_PRESCALER)-1;   //Offset: 0x004 (R/W)  SysTick Reload Value Register
	// Enable systick
	SysTick->CTRL =0x00000007;

	// Call SVC_Handler and load the first task
	__asm("svc 0");

	// It shouldn't ever get here
	while(1);
}

