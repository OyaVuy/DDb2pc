#include "stdafx.h"
#include "ClusterLifecycle.h"

void StoreEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key, EndpointElement* element) {

	EnterCriticalSection(cs);	
	storage->insert(std::make_pair(key, *element));
	LeaveCriticalSection(cs);
}
void RemoveEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key)
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}

void  StoreEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key, ExtendedEndpointElement* element)
{
	EnterCriticalSection(cs);
	storage->insert(std::make_pair(key, *element));
	LeaveCriticalSection(cs);
}
void RemoveEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key)
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}

void StoreSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key, SOCKET* socket)
{
	EnterCriticalSection(cs);
	storage->insert(std::make_pair(key, *socket));
	LeaveCriticalSection(cs);
}
void RemoveSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key)
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}
SOCKET GetSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key)
{
	int retVal;

	EnterCriticalSection(cs);	
	std::map<int, SOCKET>::iterator targetIt=storage->find(key);
	if (targetIt == storage->end()) // if does not exist
		retVal = INVALID_SOCKET;
	else
		retVal = targetIt->second;
	LeaveCriticalSection(cs);
	return retVal;
}

void StoreHandle(std::map<int, HANDLE>* storage, CRITICAL_SECTION *cs, int key, HANDLE* handle)
{
	EnterCriticalSection(cs);
	storage->insert(std::make_pair(key, *handle));
	LeaveCriticalSection(cs);
}
void RemoveHandle(std::map<int, HANDLE>* storage, CRITICAL_SECTION* cs, int key)
{
	EnterCriticalSection(cs);
	storage->erase(key);
	LeaveCriticalSection(cs);
}