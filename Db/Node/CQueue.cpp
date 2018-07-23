#include "CQueue.h"
#include<stdio.h>

void cqueue_init(CQueue *cqueue) {
	InitializeCriticalSection(&(cqueue->cs));
	InitializeCriticalSection(&(cqueue->htbCs));
	cqueue->bucketSize = 0;
	cqueue->count = 0;
	cqueue->capacity = CQUEUE_INITIAL_CAPACITY;
	cqueue->pushIdx = 0;
	cqueue->popIdx = 0;

	cqueue->data = (char *)calloc(cqueue->capacity, sizeof(char));
}
int push(CQueue *cqueue, char *content) {
	Message *msg = (Message *)content;
	int messageAddress = *(int *)(content + 12);
	int length = msg->messageSize;
	int i = 0;

	EnterCriticalSection(&(cqueue->cs));

	if (msg->messageSize > (cqueue->capacity - cqueue->count)) {
		LeaveCriticalSection(&cqueue->cs);
		return 0; //nema mesta		
	}


	//strpati ova dva for-a u jedan for
	for (i; i < 12; i++) {
		cqueue->data[cqueue->pushIdx] = content[i];
		cqueue->pushIdx = (++(cqueue->pushIdx)) % (cqueue->capacity);
	}

	for (i = 0; i < length - 12; i++) {
		cqueue->data[cqueue->pushIdx] = content[i + 12];
		cqueue->pushIdx = (++(cqueue->pushIdx)) % (cqueue->capacity);
	}

	cqueue->count += length;

	LeaveCriticalSection(&(cqueue->cs));
	return 1; //bilo mesta, dodato
}
char* pop(CQueue *cqueue, int* success) {
	int i = 0;
	int size = 0;
	char *retVal;

	EnterCriticalSection(&(cqueue->cs));
	if (cqueue->count == 0) {
		*success = 0; //neuspesno popovanje
		retVal = NULL;
	}
	else {
		for (i; i < sizeof(int); i++) {
			size |= (((cqueue->data)[cqueue->popIdx]) << (8 * i));
			cqueue->popIdx = (++(cqueue->popIdx)) % (cqueue->capacity);
		}

		retVal = (char *)calloc(size, sizeof(char));
		((Message *)retVal)->messageSize = size;

		for (i = 0; i < (size - 4); i++) {
			*(retVal + 4 + i) = (cqueue->data)[cqueue->popIdx];
			cqueue->popIdx = (++(cqueue->popIdx)) % (cqueue->capacity);
		}
		*success = 1;

		cqueue->count -= size;
		printf("\nUspeli smo da popujemo poruku sa reda %d", cqueue->priority);
	}

	LeaveCriticalSection(&(cqueue->cs));
	return retVal;
}

int nextMessageSize(CQueue *cqueue) {
	int size = 0;
	int i = 0;
	int oldPopIdx = cqueue->popIdx;
	if (cqueue->count == 0)
		return 0;

	for (i; i < sizeof(int); i++) {
		size |= (((cqueue->data)[cqueue->popIdx]) << (8 * i));
		cqueue->popIdx = (++(cqueue->popIdx)) % (cqueue->capacity);
	}

	cqueue->popIdx = oldPopIdx;
	return size;
}

void cqueue_free(CQueue *cqueue) {
	DeleteCriticalSection(&(cqueue->cs));
	DeleteCriticalSection(&(cqueue->htbCs));
	free(cqueue->data);
}