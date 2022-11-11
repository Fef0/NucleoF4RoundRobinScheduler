/*
 * context_switch.s
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
.type SVC_Handler, %function
.type SysTick_Handler, %function

// Function export
.global SVC_Handler
.global SysTick_Handler

// Variable import
.global currentTask

// Used to start the first task
SVC_Handler:
	// Disable interrupts
    cpsid I

    // Load pointer to currentTask, which is a pointer (R0 = &currentTask)
    ldr R0, =currentTask
    // Dereference to get currentTask (R0 = *(&currentTask) = currentTask) which is a pointer to the tcb
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


// TODO fix hardfault from branch
SysTick_Handler:
	// Disable interrupt
	cpsid I

	// CPU already saved the stack frame ({r0-r3}, r12, lr, pc, xpsr)
	// and eventually {s0-s15} if we're using Floating Points
	// so we have to push {r4-r11} to task stack (and {s16-s31}) and then switch to the next stack
	// (Remember that currently SP is pointing to the currentTask stack)
	push {R4-R11}

	// Check if we're inside a Floating Point Context
	// by checking LR content (after an interrupt it gets updated with useful info)
	// In particular if the 5th bit is 1 then FPC is active so we must push the floating
	// registers too
	//tst LR, 0x10
	//it eq
	//push {s16-s31}

	// Get currentTask address (R0 = &currentTask)
	ldr R0, =currentTask

	// Dereference, so R0 = *(&currentTask) = currentTask
	ldr R0, [R0]

	// Dereference again, but this time in a different register so we can save
	// currentStack address
	// We are going to add 4 in order to dereference and get tcb->next at once
	// (r1 = currentTask->next)
	ldr R1, [R0, #4]
	ldr R2, [R0]

	// Now R0 contains currentTask, which is a pointer
	// And R1 contains currentTask>next, which is a pointer too

	// Override currentTask with currentTask->next
	// (currentTask = currentTask->next)
	str R1, [R0]

	// Change stack pointer to point to the currentTask
	ldr SP, [R1]

	// We are assuming that we pushed {R4-R11} and maybe {S16-S31} in the latest iteration, so we have to pop them
	// from the stack
	//tst LR, 0x10
	//it eq
	//pop {S16-S31}

	pop {R4-R11}

	// Now SP points to the PC, so we must pop it to lr
	pop {LR}

	// Enable interrupt
	cpsie I

	// And now we can branch to the next task
	bx LR;









