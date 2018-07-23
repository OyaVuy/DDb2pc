#pragma once
#pragma once

#define CQUEUE_INITIAL_CAPACITY 300
#include<Windows.h>

typedef struct {
	int messageSize;
	int priority;
	int clientId;
	char *data;
}Message;

typedef struct {
	int bucketSize;
	int priority;
	int count; //broj popunjenih bajtova u baferu
	int capacity; //menjacemo kad budemo radili resize
	int pushIdx;
	int popIdx;
	CRITICAL_SECTION cs;
	CRITICAL_SECTION htbCs;
	char *data; //pocetak niza
}CQueue;

void cqueue_init(CQueue *cqueue);
int push(CQueue *cqueue, char *content);
char* pop(CQueue *cqueue, int* success);
int nextMessageSize(CQueue *cqueue);
void cqueue_free(CQueue *cqueue);
