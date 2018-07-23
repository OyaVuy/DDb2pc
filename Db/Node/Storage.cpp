#include "Storage.h"


void InitStorage(StorageManager* storageMng)
{
	// storage gets initialized on first client request or on
	// first integrity update
	// removing of data can be done on client basis
	storageMng->isInit = true;
	InitializeCriticalSection(&storageMng->cs_DataLvl_2);
	InitializeCriticalSection(&storageMng->cs_DataLvl_3);
}

// add and/or getmap associated with particular client
std::map<int, LinkedList> AddAndGetLevel2Data(StorageManager* level1storageMng, int level2key)
{
	std::map<int, LinkedList> retVal;

	EnterCriticalSection(&level1storageMng->cs_DataLvl_2);
	retVal = level1storageMng->level1_level2_Data[level2key];
	LeaveCriticalSection(&level1storageMng->cs_DataLvl_2);

	return retVal;
}

// add and/or get list associated with particular node
LinkedList* AddAndGetLevel3Data(std::map<int, LinkedList>*  level2storage, CRITICAL_SECTION* cs, int level3key)
{
	std::map<int, LinkedList> map = *level2storage;
	LinkedList* retVal;

	EnterCriticalSection(cs);
	retVal = &map[level3key];
	LeaveCriticalSection(cs);

	if (!retVal->isInit)
		InitLinkedList(retVal);

	return retVal;
}

void InitLinkedList(LinkedList* list)
{
	InitializeCriticalSection(&list->cs_Data);
	list->isInit = true;
	list->count = 0;
}

// adding at end
bool AddNodeToList(LinkedList* list, ListNode* node)
{
	node->next = nullptr;
	if (list->head == nullptr)
	{
		list->head = list->tail = node;
	}
	else
	{
		list->tail->next = node;
		node->next = nullptr;
		list->tail = node;
	}
	list->count++;
}

ListNode* FindNodeInList(LinkedList* list, int nodeKey)
{
	ListNode* retVal = nullptr;
	// if list->count ==0 then head==nullptr
	if (list->count == 0 || nodeKey<0 || nodeKey> list->count)
		return nullptr;

	ListNode *temp = list->head;

	do {
		if (temp->key == nodeKey);
		{
			retVal = temp;
			break;
		}
	} while ((temp = temp->next) != nullptr);

	return retVal;
}
//
//int StoreData(StorageManager* storageManager, Message* msgToStore)
//{
//	ClientMessageHeader request = *((ClientMessageHeader*)msgToStore->payload);
//	storageManager->dataCount++;
//
//	int level1key = request.clientId;
//	int level2key = request.originNodeId;
//	int level3key = request.originNodeCounter = storageManager->dataCount;
//
//	int wholeMessageSize = msgToStore->size + 4; // for first four size bytes
//
//	ListNode* level3Node = (ListNode*)malloc(sizeof(ListNode));
//	level3Node->key = level3key;
//	level3Node->next = nullptr;
//	level3Node->nextLevelData = (char*)calloc(wholeMessageSize, sizeof(char));
//
//	memcpy(level3Node->nextLevelData, msgToStore, wholeMessageSize);
//
//	std::map<int, LinkedList> level2dataMap = AddAndGetLevel2Data(storageManager, level1key);
//	LinkedList* level3dataList = AddAndGetLevel3Data(&level2dataMap, &storageManager->cs_DataLvl_3, level3key);
//	AddNodeToList(level3dataList, level3Node);
//}

// go backwards at storing. all whole Message has to be stored, in order for client to retrieve it with keys