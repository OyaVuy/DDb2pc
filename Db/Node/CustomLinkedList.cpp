#include "CustomLinkedList.h"

// napisati u potencijalna oboljsanja sta si mogla od struktura da koristis-napravis...bla bla
// da si imala vise listi nivoa, moglo je biti manje zakljucavanja, na nivou nodea - klijenta..msm svakako se jedan klijenat opsluzuje


// todo srediti da nijedna od ovih metoda ne prolazi ako je cs_Storage zauzet
void InitList(LinkedList* list)
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

ListNode* FindNodeInList(LinkedList* list, int key1, int key2, int key3)
{
	ListNode* retVal = nullptr;
	if (list->count == 0 ||
		key1 < 0 || key2 < 0 || key3 < 0)
		return nullptr;

	ListNode *temp = list->pHead;

	do {
		if (temp->key1 == key1 && temp->key2 == key2 && temp->key3 == key3)
		{
			retVal = temp;
			break;
		}
	} while ((temp = temp->pNext) != nullptr);

	return retVal;
}

void StoreMessage(LinkedList* storage, Message* msgToStore)
{
	// check when to use cs
	if (!storage->isInit)
		InitList(storage);

	ClientMessageHeader request = *((ClientMessageHeader*)msgToStore->payload);
	int clientId = request.clientId;
	int nodeId = request.originNodeId;
	storage->count++;
	int msgCounter = request.originNodeCounter = storage->count;

	// whole Message has to be stored, in order for client to retrieve it with keys
	//size_t concreteDataSize = msgToStore->size - sizeof(MsgType) - sizeof(ClientMessageHeader);
	//int concreteDataIdx = sizeof(ClientMessageHeader);
	//char* pConcreteData = msgToStore->payload + concreteDataIdx;

	int wholeMessageSize = msgToStore->size + 4;
	char* newConcreteData = (char*)calloc(wholeMessageSize, sizeof(char));

	memcpy(newConcreteData, msgToStore, wholeMessageSize);
	//memcpy(newConcreteData, pConcreteData, wholeMessageSize);

	ListNode* newConcreteDataNode = (ListNode*)malloc(sizeof(ListNode));
	newConcreteDataNode->key1 = clientId;
	newConcreteDataNode->key2 = nodeId;
	newConcreteDataNode->key3 = msgCounter;
	newConcreteDataNode->pNext = nullptr;
	newConcreteDataNode->pData = newConcreteData;

	AddNodeToList(storage, newConcreteDataNode);
}


void ClearList(LinkedList * list)
{

}

void RemoveFromList(LinkedList *list, int key1, int key2, int key3)
{

}