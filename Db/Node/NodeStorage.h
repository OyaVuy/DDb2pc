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
	StorageListNodeT* pNext;
	StorageListNodeT* pPrevious;
	void * data;
}ListNode;

typedef struct
{
	std::atomic <bool> isInit;
	std::atomic <int> count;
	CRITICAL_SECTION cs_Data;
	ListNode* head;
	ListNode* tail;
}LinkedList;

// level1 is accessing by clientId 
// then we are on level2, where we have on access based on level2key
typedef struct
{
	std::atomic <int> dataCount;
	std::atomic <bool> isInit;
	CRITICAL_SECTION cs_DataLvl_2;
	CRITICAL_SECTION cs_DataLvl_3;
	std::map<int, LinkedList> data;
}StorageManager;

void InitStorage(StorageManager* storageMng);
void AddNewLinkedList(int key, LinkedList* list);
void InitLinkedList(LinkedList* list);
void AddNodeToList(LinkedList* list, ListNode* node);
ListNode* FindNodeInList(LinkedList* list, int nodeKey);

