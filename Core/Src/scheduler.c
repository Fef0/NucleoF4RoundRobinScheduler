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


#define BUS_FREQ 84000000
#define MILLIS_PRESCALER BUS_FREQ/1000

// It's a linked queue of Task Control Blocks
tcbQueueType* tasksQueue;

// Points to the current task
tcbNode* currentTask;

// Boolean flag that signals the scheduler if idleTask is running or not
uint8_t isIdleTaskRunning = 0;

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


// Add a new task to the tasks queue
void addTask(void (*taskFunc)()) {
	// If the idleTask is running then we must remove it to add the first real task
	if (isIdleTaskRunning) {
		// Deallocate idleTask memory
		free(tasksQueue->head);
		// Reset queue tasks num to "empty" the queue
		tasksQueue->tasksNum = 0;
		// Reset isIsleTaskRunning
		isIdleTaskRunning = 0;
	}

	// Allocate space for a new Thread Control Block
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

/*
// TODO: complete this
uint8_t removeTask(uint8_t taskID) {
	tcbNode* iter = tasksQueue->head;
	tcbNode* prev = NULL;
	while(1) {
		if (iter == NULL) {
			return 0;
		}

		if (iter->taskID == taskID) {
			// Case 1: the element to remove is the head
			if (iter == tasksQueue->head) {

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
uint32_t freq = 0;
uint32_t time = 50;

// This task is loaded automatically at startup and is used
// in case no other task exists in the system
void idleTask() {
	while(1);
}

void sysTickInit(uint32_t quanta_ms) {
	// Config systick
	SysTick->CTRL =0;   //Disable the SysTick timer; Offset: 0x000 (R/W)  SysTick Control and Status Register
	SysTick->VAL=0;     //Clear current value to 0; Offset: 0x008 (R/W)  SysTick Current Value Register
	SysTick->LOAD = (quanta_ms * MILLIS_PRESCALER)-1;   //Offset: 0x004 (R/W)  SysTick Reload Value Register
	// Enable systick
	SysTick->CTRL =0x00000007;
}

void startScheduler(uint32_t quanta_ms) {
	// Initialize the circular linked queue
	initQueue();

	// Always add the idle Task as the first task
	addTask(&idleTask);
	isIdleTaskRunning = 1;

	// TODO delete-me
	GPIO_InitTypeDef BoardLEDsB;
	BoardLEDsB.Mode = GPIO_MODE_OUTPUT_PP;
	BoardLEDsB.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &BoardLEDsB);

	GPIO_InitTypeDef BoardLEDsC;
	BoardLEDsC.Mode = GPIO_MODE_OUTPUT_PP;
	BoardLEDsC.Pin = GPIO_PIN_2|GPIO_PIN_3;
	HAL_GPIO_Init(GPIOC, &BoardLEDsC);

	sysTickInit(quanta_ms);
	freq = HAL_RCC_GetSysClockFreq();

	TIM_init(TIM2);
	// Configure the timebase
	TIM_config_timebase(TIM2, 1, 1000);
	TIM_on(TIM2); // starts the timer
	TIM2->CNT = 0; // resets the counter

	// Call SVC_Handler and load the first task
	__asm("svc 0");
}

