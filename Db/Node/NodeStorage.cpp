#include "NodeStorage.h"

// mapraviti mapu samo jednog nivoa, za svakog klijenta imas po jednu listu
// zgodno je da bude dvostruko spregnuta
// i onda iteriras kroz tu listu redom i trazis sta ti treba...


void InitStorage(StorageManager* storageMng)
{
	storageMng->isInit = true;
	InitializeCriticalSection(&storageMng->cs_DataLvl_2);
	InitializeCriticalSection(&storageMng->cs_DataLvl_3);
}


void InitLinkedList(LinkedList* list)
{
	InitializeCriticalSection(&list->cs_Data);
	list->isInit = true;
	list->count = 0;
}

// adding at end
void AddNodeToList(LinkedList* list, ListNode* node)
{
	node->pNext = nullptr;
	if (list->head == nullptr)
	{
		list->head = list->tail = node;
	}
	else
	{
		list->tail->pNext = node;
		node->pNext = nullptr;
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
		if (temp->key1 == nodeKey)
		{
			retVal = temp;
			break;
		}
	} while ((temp = temp->pNext) != nullptr);

	return retVal;
}


// go backwards at storing. all whole Message has to be stored, in order for client to retrieve it with keys