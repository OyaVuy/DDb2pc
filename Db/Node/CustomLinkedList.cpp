#include "CustomLinkedList.h"

// napisati u potencijalna oboljsanja sta si mogla od struktura da koristis-napravis...bla bla
// da si imala vise listi nivoa, moglo je biti manje zakljucavanja, na nivou nodea - klijenta..msm svakako se jedan klijenat opsluzuje


void InitList(LinkedList* list)
{
	InitializeCriticalSection(&list->cs_Data);
	list->isInit = true;
	list->nodesCount = 0;
	list->pHead = list->pTail = nullptr;
}

// adding at end
void AddNodeToList(LinkedList* list, ListNode* node)
{
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
}

ListNode* FindNodeInList(LinkedList* list, int key1, int key2, int key3)
{
	ListNode* retVal = nullptr;
	if (list->nodesCount == 0 ||
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

// todo use critical section
void StoreMessage(LinkedList* storage, Message* msgToStore, bool isDataSequential)
{
	if (!storage->isInit)
		InitList(storage);

	ClientMessageHeader request;
	if (!isDataSequential)
	{
		request = *((ClientMessageHeader*)msgToStore->payload);
	}
	else
	{
		request = *(ClientMessageHeader*)(msgToStore->payload);
	}


	// todo fix key-s associated with origin node...
	int clientId = request.clientId;
	int nodeId = request.originId;
	storage->nodesCount++;
	int msgCounter = request.originCounter = storage->nodesCount;

	// whole Message has to be stored, in order for client to retrieve it with keys
	size_t concreteDataSize = msgToStore->size - sizeof(MsgType) - sizeof(ClientMessageHeader);
	int concreteDataPayload_Offset = sizeof(ClientMessageHeader); // offset from Message.payload
	char* pConcreteData = msgToStore->payload + concreteDataPayload_Offset;

	// imprtant: 
	// later free first pData in node, 
	// than remove node from list while saving reference to node, 
	// then free node 
	int wholeMessageSize = msgToStore->size + 4;
	char* newConcreteData = (char*)calloc(wholeMessageSize, sizeof(char));

	if (!isDataSequential)
	{
		memcpy(newConcreteData, msgToStore, 8);
		memcpy(newConcreteData + 8, msgToStore->payload, msgToStore->size - 4);
	}
	else
	{
		memcpy(newConcreteData, msgToStore, wholeMessageSize);
	}

	ListNode* newConcreteDataNode = (ListNode*)malloc(sizeof(ListNode));
	newConcreteDataNode->key1 = clientId;
	newConcreteDataNode->key2 = nodeId;
	newConcreteDataNode->key3 = msgCounter;
	newConcreteDataNode->pNext = nullptr;
	newConcreteDataNode->pData = newConcreteData;

	AddNodeToList(storage, newConcreteDataNode);
}

void StoreOneMessage(LinkedList* storage, Message* msgToStore)
{
	if (!storage->isInit)
		InitList(storage);

	ClientMessageHeader request;
	request = *((ClientMessageHeader*)msgToStore->payload);

	int clientId = request.clientId;
	int nodeId = request.originId;
	storage->nodesCount++;
	int msgCounter = request.originCounter = storage->nodesCount;

	// whole Message has to be stored, in order for client to retrieve it with keys
	size_t concreteDataSize = msgToStore->size - sizeof(MsgType) - sizeof(ClientMessageHeader);
	int concreteDataPayload_Offset = sizeof(ClientMessageHeader); // offset from Message.payload
	char* pConcreteData = msgToStore->payload + concreteDataPayload_Offset;

	// imprtant: 
	// later free first pData in node, 
	// than remove node from list while saving reference to node, 
	// then free node 
	int wholeMessageSize = msgToStore->size + 4;
	char* newConcreteData = (char*)calloc(wholeMessageSize, sizeof(char));

	memcpy(newConcreteData, msgToStore, 8);
	memcpy(newConcreteData + 8, msgToStore->payload, msgToStore->size - 4);

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
	if (list->nodesCount == 0)
		return;

	ListNode* temp = list->pHead;
	ListNode* previous = list->pHead;
	do {

		if (temp->pData != nullptr)
			free(temp->pData);

		previous = temp;
		temp = temp->pNext;

		free(previous);

	} while (temp != nullptr);
}

bool RemoveFromList(LinkedList *list, int key1, int key2, int key3)
{
	int retVal = false;
	if (list->nodesCount == 0 || key1 < 0 || key2 < 0 || key3 < 0)
		return retVal;

	ListNode* temp = list->pHead;
	ListNode* previous = list->pHead;
	do {
		if (temp->key1 == key1 && temp->key2 == key2 && temp->key3 == key3)
		{
			// node is found...
			// first free pData, than rearrange links in list, remamber this node, and then free the node
			if (temp->pData != nullptr)
				free(temp->pData);

			previous->pNext = temp->pNext;
			free(temp);
			list->nodesCount--;
			break;
		}
		previous = temp;
	} while ((temp = temp->pNext) != nullptr);

	return retVal;
}