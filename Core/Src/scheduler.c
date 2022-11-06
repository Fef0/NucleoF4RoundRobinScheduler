/*
 * scheduler.c
 *
 *  Created on: Nov 3, 2022
 *      Author: fef0
 */

#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"

// It's a linked list of Task Control Blocks
tcbListType* tasksList;

// Points to the current task
tcbNode* currentTask;

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

// The stack will contain {R0-R12}, LR, PC, xPSR
// but not in this order
uint32_t* createStack(void (*taskFunc)()) {
	// Dynamically create the new stack and update the pointer
	uint32_t* stackPt = (uint32_t*) malloc(sizeof(stack));

	// Fill the stack with default values
	// xPSR default (from Cortex m4 manual)
	stackPt[STACKSIZE-1] = 0x01000000;
	// PC will point to the function itself
	stackPt[STACKSIZE-2] = (uint32_t) taskFunc;

	// The stack will grow from the bottom up so
	// the stack pointer must point to the PC which is
	// the last full cell of the stack
	return &(stackPt[STACKSIZE-2]);
}

// Add a new task to the tasks list
void addTask(void (*taskFunc)(), uint8_t taskID) {
	// Allocate space for a new Thread Control Block
	tcbNode* t = (tcbNode*) malloc(sizeof(tcbNode));
	// Create the stack
	t->stackPt = createStack(taskFunc);
	// Set taskID
	t->taskID = taskID;

	// If the list is empty then initialize head and tail at the same position
	// Otherwise update the last element to point to the new one and update the tail
	if (isListEmpty()) {
		tasksList->head = tasksList->tail = t;
		// Update currentTask pointer to point to the first
		// task added
		currentTask = t;
	} else {
		tasksList->tail->next = t;
		tasksList->tail = t;
	}

	// The last element will now point to the head of the circular list
	tasksList->tail->next = tasksList->head;
	// Update tasks number
	tasksList->tasksNum++;
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

void task0() {
	int z = 0;

	while(1) {
		z++;
	}
}

void startScheduler() {
	// Initialize the circular linked list
	initList();

	// Add the first task
	addTask(&task0, 1);


	// Call SVC_Handler and load the first task
	__asm("svc 0");

	// It shouldn't ever get here
	while(1);
}

