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
// Since this function is an interrupt, {R0-R3, LR, PC, xPSR} have already been pushed to stack
SVC_Handler:

	// Disable interrupts
    cpsid i
    // Load currentTask pointer (R0 = &currentTask)
    ldr R0, =currentTask

    // Dereference &currentTask (R0 = *(&currentTask) = currentTask) which is
    // a pointer to the task tcb
    ldr R0, [R0]

    // Dereference &tcb to get tcb->stackPt
    ldr R0, [R0]

    // Pop the software stack registers and update tcb->stackPt (which is the task stack pointer)
    // in this case LR will contain 0xFFFFFFD which tells the CPU to return to
    // the hardware stack fram LR (which will be pop later) with thread mode
    // and using psp for the task stack
    ldmfd R0!, {R4-R11, LR}

	// Update PSP to point to R0 (which is the task stack pointer)
    msr PSP, R0

    // Enable interrupts
    cpsie i

    // Branch and exchange will load software LR value, tells the CPU
    // to use thread mode and psp and then pop the hardware stack frame
    // from the psp in order to branch to the first task
    bx LR


// Used to handle the context switch from one task to another
// Since this function is an interrupt, {R0-R3, LR, PC, xPSR} have already been pushed to stack
SysTick_Handler:
	// REMEMBER: every edit done before switching task
	// is done on currentTask stack

	// Disable interrupt
	cpsid I

	// Save the PSP in R0 in order to manipulate it
	mrs R0, PSP

	// Load current task pointer (R1 = &currentTask)
	ldr R1, =currentTask
	// Dereference (R1 = *(&currentTask) = currentTask = &tcb) which is
	// a pointer to the tcb
	ldr R2, [R1]

	// Dereference again to get tcb (R2 = *(&tcb) = tcb) but since it is a struct
	// and its first element is stackPt, then R2 = tcb = tcb->stackPt
	ldr R2, [R2]

	// Check if the current context is a Floating Point Context
	// (see cortex m4 manual)
	//tst LR, #0x00000010
	// if equal then
	//it eq
	// If equal, then push FPU registers into currentTask stack
	//vstmdbeq  R0!, {S16-S31}

	// Push normal context registers into currentTask stack
	stmfd R0!, {R4-R11, LR}

	// Save the new stack pointer inside currentTask stack
	// in order to be used for the next task run
	str R0, [R2]

	ldr r2, [r1]

	// ---- Load next task ----
	// Load next task pointer starting from currentTask
	// (R3 = (&tcb->next)) which is a pointer to another tcb
	// (since it's a uint32_t then the "next" element is 4 bytes after "stackPt")
	ldr R3, [R2, #4]

	// Dereference to get tcb->next (R3 = *(&(tcb->next)) = tcb->next) which is equal to
	// tcb->next->stackPt and load it into R0 (R0 = tcb->next->stackPt)
	ldr R0, [R3]

	// Make currentTask point to currentTask->next = tcb->next
	// currentTask = tcb->next
	str R3, [R1]

	// Pop R4-R11 and LR from the next task stack
	ldmfd R0!, {R4-R11, LR}

	// Check if the current context is a Floating Point Context
	// (see cortex m4 manual)
	//tst LR, #0x00000010
	// if equal then
	//it eq
	// If equal, then push FPU registers into currentTask stack
	//vldmiaeq  R0!, {S16-S31}

	// PSP will now point to the next task stack
	msr PSP, R0

	cpsie I


