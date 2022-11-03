/*
 * scheduler.c
 *
 *  Created on: Nov 3, 2022
 *      Author: fef0
 */

#define MAXTASKS 10

// Task Control Block will point to its relative stack pointer
// And will be linked to the next thread control block
// And the previous one (will speed up hops)
struct tcb {
	int32_t* stackPt;
	struct tcb* next;
	int taskID;
};

typedef struct tcb tcbNode;

// It's a circular linked list
typedef struct tcbList {
	tcbNode* head;
	tcbNode* tail;
	int tasksNum;
};

// It's a linked list of Task Control Blocks
tcbList* tasksList;

// Points to the current task
tcbNode* currentTask;

int init() {
	tasksList->head = NULL;
	tasksList->tail = NULL;
	tasksList->tasksNum = 0;
}

int isListEmpty() {
	return tasksList->head == NULL;
}


// Add a new task to the tasks list
int addTask(void* taskFunc) {
	// Allocate space for a new Thread Control Block
	tcb* t = (tcbNode*) malloc(sizeof(tcb));

	// If the list is empty then initialize head and tail at the same position
	// Otherwise update the last element to point to the new one and update the tail
	if (isListEmpty()) {
		tasksList->head = tasksList->tail = t;
	} else {
		tasksList->tail->next = t;
		tasksList->tail = t;
	}

	// The last element will now point to the head of the circular list
	tasksList->tail->next = tasksList->head;
}
