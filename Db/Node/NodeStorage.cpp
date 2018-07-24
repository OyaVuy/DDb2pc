#include "NodeStorage.h"

// napisati u potencijalna oboljsanja sta si mogla od struktura da koristis-napravis...bla bla
// da si imala vise listi nivoa, moglo je biti manje zakljucavanja, na nivou nodea - klijenta..msm svakako se jedan klijenat opsluzuje

void InitStorage(StorageManager* storageMng)
{
	storageMng->isInit = true;
	std::atomic <int> dataCount = 0;
	std::atomic <int> dataCount_Lvl1 = 0;
}

void InitLinkedList(LinkedList* list)
{
	InitializeCriticalSection(&list->cs_Data);
	list->isInit = true;
	list->count = 0;
	list->pHead = list->pTail = nullptr;
}

// adding at end
void AddNodeToList(LinkedList* list, ListNode* node)
{
	// todo cs data?
	node->pNext = nullptr;
	if (list->pHead == nullptr)
	{
		list->pHead = list->pTail = node;
	}
	else
	{
		list->pTail->pNext = node;
		node->pNext = nullptr;
		list->pTail = node;
	}
	list->count++;
}

ListNode* FindNodeInList(LinkedList* list, int nodeKey)
{
	ListNode* retVal = nullptr;
	// if list->count ==0 then head==nullptr
	if (list->count == 0 || nodeKey<0 || nodeKey> list->count)
		return nullptr;

	ListNode *temp = list->pHead;

	do {
		if (temp->key1 == nodeKey)
		{
			retVal = temp;
			break;
		}
	} while ((temp = temp->pNext) != nullptr);

	return retVal;
}

void StoreMessage(StorageManager* storage, Message* msgToStore)
{
	// check when to use cs
	if (!storage->isInit)
		InitStorage(storage);

	ClientMessageHeader request = *((ClientMessageHeader*)msgToStore->payload);
	int clientId = request.clientId;
	int nodeId = request.originNodeId;
	storage->dataCount++;
	int msgCounter = request.originNodeCounter = storage->dataCount;

	// STORE WHOLE MESSAGE
	// now we have prepared keys....
	// go backwards at storing. all whole Message has to be stored, in order for client to retrieve it with keys
	// first prepare data for last (second) node
	size_t concreteDataSize = msgToStore->size - sizeof(MsgType) - sizeof(ClientMessageHeader);
	int concreteDataIdx = sizeof(ClientMessageHeader);
	char* pConcreteData = msgToStore->payload + concreteDataIdx;

	int wholeMessageSize = msgToStore->size + 4;
	char* newConcreteData = (char*)calloc(wholeMessageSize, sizeof(char));
	memcpy(newConcreteData, pConcreteData, wholeMessageSize);

	ListNode* newConcreteDataNode = (ListNode*)malloc(sizeof(ListNode));
	newConcreteDataNode->key1 = nodeId;
	newConcreteDataNode->key2 = msgCounter;
	newConcreteDataNode->pNext = nullptr;
	newConcreteDataNode->pData = newConcreteData;
	// todo maybe do not remember pointer to previous

	AddNodeToList(storage->data_Lvl1, newConcreteDataNode);
}

