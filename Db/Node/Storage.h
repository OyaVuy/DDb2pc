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

typedef struct StorageListNodeT
{
	int key;
	StorageListNodeT* next;
	void * nextLevelData;
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
	std::map<int, std::map<int, LinkedList>> level1_level2_Data;
}StorageManager;


void InitStorage(StorageManager* storageMng);

std::map<int, LinkedList> AddAndGetLevel2Data(StorageManager* level1storageMng, int level2key);
LinkedList* AddAndGetLevel3Data(std::map<int, LinkedList>*  level2storage, CRITICAL_SECTION* cs, int level3key);

void InitLinkedList(LinkedList* list);
bool AddNodeToList(LinkedList* list, ListNode* node);
ListNode* FindNodeInList(LinkedList* list, int nodeKey);
//int StoreData(StorageManager* storageManager, Message* msgToStore);
