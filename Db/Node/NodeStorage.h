#pragma once
#include "Communication.h"
#include <atomic>
#include <map>

#include <Windows.h>

// mapraviti mapu samo jednog nivoa, za svakog klijenta imas po jednu listu
// zgodno je da bude dvostruko spregnuta
// i onda iteriras kroz tu listu redom i trazis sta ti treba...
// dodajes liste cim se pojavi poruka od tog klijenta
// prvo proveris jel postoji da lista

#pragma pack(1)
typedef struct
{
	int clientId;
	int originId;
	int originCounter;
}HeaderData;

typedef struct StorageListNodeT
{
	int key1;
	int key2;
	int key3;
	StorageListNodeT* pNext;
	StorageListNodeT* pPrevious;
	char * pData;
}ListNode;

typedef struct
{
	std::atomic <bool> isInit;
	std::atomic <int> count;
	CRITICAL_SECTION cs_Data;
	ListNode* pHead;
	ListNode* pTail;
}LinkedList;

// level1 is accessing by clientId 
// then we are on level2, where we have on access based on level2key
typedef struct
{
	CRITICAL_SECTION cs_Storage;
	std::atomic <int> elementsCount; // cumulative
	std::atomic <int> dataCount_Lvl1;
	std::atomic <bool> isInit;
	LinkedList* data_Lvl1;
}StorageManager;

void InitStorage(StorageManager* storageMng);
void InitLinkedList(LinkedList* list);
void AddNodeToList(LinkedList* list, ListNode* node);
void StoreMessage(StorageManager* storage, Message* msgToStore);
ListNode* FindNodeInList(LinkedList* list, int nodeKey);
void FreeStorage(StorageManager* storageMng);
