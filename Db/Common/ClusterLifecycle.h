#pragma once

//Don't #include "stdafx.h" in your library's public header file.
//Simply list all your big huge headers for your APIs in your stdafx.h file, in the appropriate order, and then start each of your CPP files at the very top with an #include "stdafx.h", before any meaningful content (just about the only thing allowed before is comments).*/

#include <map>

#define MAX_NODES_COUNT 30 

#pragma pack(1)

// Data for node registration in cluster
typedef struct {
	int nodeId;
	int intIpAddress; // in little endian format
	USHORT portForNodes;
	USHORT portForClients;
}NodeRegData;

typedef struct {
	int ipAddress;
	USHORT port;
}EndpointElement;

typedef struct {
	int ipAddress;
	USHORT port;
	int endpointId;
}ExtendedEndpointElement;

void  StoreEndpoint(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key, EndpointElement* element);
void  StoreEndpoint(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key, ExtendedEndpointElement* element);

void StoreSocket(std::map<int, SOCKET>* storage, CRITICAL_SECTION* cs, int key, SOCKET* socket);

void RemoveElement(std::map<int, EndpointElement>* storage, CRITICAL_SECTION* cs, int key);
void RemoveElement(std::map<int, ExtendedEndpointElement>* storage, CRITICAL_SECTION* cs, int key);