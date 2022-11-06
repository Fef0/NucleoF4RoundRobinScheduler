/*
 * scheduler.c
 *
 *  Created on: Nov 5, 2022
 *      Author: fef0
 */

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section  .text

// Function def
.type  SVC_Handler, %function

// Function export
.global SVC_Handler

// Variable import
.global currentTask

// Used to start the first task
SVC_Handler:
	// Disable interrupts
    cpsid I

    // Load pointer to currentTask (R0 = *currentTask)
    ldr R0, =currentTask
    // Dereference to get currentTask (R0 = &(*currentTask) = currentTask) which is a pointer too
    ldr R0, [R0]
	// Dereference one last time to get the tcb that currentTask points to
	// The first element of tcb struct is the stackPt (SP = currentTask->stackPt)
	// Now SP points to the first full cell of the task stack, the PC register
    ldr SP, [R0]

	// Pop the PC register from the task stack into the CPU LR register in order to return from the SVC_Handler
	// directly inside the first task code
	pop {LR}

    cpsie I // Enable interrupts

    bx LR // Return from the function and start the first task

