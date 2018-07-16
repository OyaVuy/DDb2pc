#include "stdafx.h"
#include "ClusterLifecycle.h"

void StoreEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key, EndpointElement* element) {

	EnterCriticalSection(cs);	
	storage->insert(std::make_pair(key, *element));
	LeaveCriticalSection(cs);
}

void  StoreEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key, ExtendedEndpointElement* element)
{
	EnterCriticalSection(cs);
	storage->insert(std::make_pair(key, *element));
	LeaveCriticalSection(cs);
}

void StoreSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key, SOCKET* socket)
{
	EnterCriticalSection(cs);
	storage->insert(std::make_pair(key, *socket));
	LeaveCriticalSection(cs);
}

void RemoveElement(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key) 
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}

void RemoveElement(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key)
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}