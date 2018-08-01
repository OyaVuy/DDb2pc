#pragma once
#include "Communication.h"
#include <atomic>
#include <map>

#include <Windows.h>

#pragma pack(1)
typedef struct
{
	int clientId;
	int originId;
	int originCounter;
}HeaderData;

typedef struct ListNodeT
{
	int key1;
	int key2;
	int key3;
	ListNodeT* pNext;
	ListNodeT* pPrevious;
	char * pData;
}ListNode;

typedef struct
{
	std::atomic <bool> isInit;
	std::atomic <int> nodesCount;
	CRITICAL_SECTION cs_Data;
	ListNode* pHead;
	ListNode* pTail;
}LinkedList;


void InitList(LinkedList* list);
void AddNodeToList(LinkedList* list, ListNode* node);
void StoreMessage(LinkedList* storage, Message* msgToStore, bool isDataSequential);
ListNode* FindNodeInList(LinkedList* list, int key1, int key2, int key3);
void ClearList(LinkedList * list);
void RemoveFromList(LinkedList *list, int key1, int key2, int key3);