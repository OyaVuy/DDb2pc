#pragma once

//Don't #include "stdafx.h" in your library's public header file.
//Simply list all your big huge headers for your APIs in your stdafx.h file, in the appropriate order, and then start each of your CPP files at the very top with an #include "stdafx.h", before any meaningful content (just about the only thing allowed before is comments).*/

#include <map>

#define MAX_NODES_COUNT 30 
#define NODE_PROBE_TIME 3000

// max time for receiving indication of finished transaction
// if indication is not received - master on its own removes TransactionOnLine flag
#define TRANSACTION_PENDING_TIME_OUT 5 // in seconds

#define USER_CANCELLATION_TIME_MS 3000

#pragma pack(1)

// Data for node registration in cluster
typedef struct
{
	int nodeId;
	int intIpAddress; // in little endian format
	USHORT portForNodes;
	USHORT portForClients;
}NodeRegData;

typedef struct
{
	int ipAddress;
	USHORT port;
}EndpointElement;

typedef struct
{
	int ipAddress;
	USHORT port;
	int endpointId;
}ExtendedEndpointElement;

void StoreEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key, EndpointElement* element);
void RemoveEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key);

void StoreEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key, ExtendedEndpointElement* element);
void RemoveEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key);

void StoreSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key, SOCKET* socket);
void RemoveSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key);
SOCKET GetSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key);

void StoreHandle(std::map<int, HANDLE>* storage, CRITICAL_SECTION *cs, int key, HANDLE* handle);
void RemoveHandle(std::map<int, HANDLE>* storage, CRITICAL_SECTION* cs, int key);


