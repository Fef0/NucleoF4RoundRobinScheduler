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
// Since this function is an interrupt handler, {R0-R3, R12, LR, PC, xPSR} have already been pushed to stack
SVC_Handler:
	// Disable interrupts
    cpsid i
    // Load currentTask pointer (R0 = &currentTask)
    ldr R0, =currentTask

    // Dereference &currentTask (R0 = *(&currentTask) = currentTask) which is
    // a pointer to the task tcb (&tcb)
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
// Since this function is an interrupt handler, {R0-R3, R12, LR, PC, xPSR} have already been pushed to stack
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

	// Push normal context registers into currentTask stack
	// (after the hardware stack frame {R0-R3, R12, LR, PC, xPSR})
	stmfd R0!, {R4-R11, LR}

	// Save the new stack pointer inside currentTask pointer
	str R0, [R2]


	// ----------------------- LOAD NEXT TASK -------------------------------
	// Dereference again (*(&currentTask) = currentTask)
	ldr R2, [R1]

	// Load next task pointer starting from currentTask
	// (R3 = (&tcb->next)) which is a pointer to another tcb
	// (since it's a uint32_t then the "next" element is 4 bytes after "stackPt")
	ldr R3, [R2, #4]

	// Make currentTask point to tcb->next
	// (currentTask = tcb->next)
	str R3, [R1]

	// Dereference to get tcb->next (R3 = *(&(tcb->next)) = tcb->next) which is equal to
	// tcb->next->stackPt and load it into R0 (R0 = tcb->next->stackPt)
	ldr R0, [R3]

	// Pop R4-R11 and LR from the next task stack
	ldmfd R0!, {R4-R11, LR}

	// PSP will now point to the next task stack
	msr PSP, R0

	// Enable interrupts
	cpsie I

	bx LR


