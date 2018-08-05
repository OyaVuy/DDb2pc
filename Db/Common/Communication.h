#pragma once
#ifndef _COMMUNICATION_DDB_
#define _COMMUNICATION_DDB_

#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> 
#include <winsock2.h>
#include <ws2tcpip.h>

#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);} 

#define DEFAULT_BUFLEN 8192 
#define INFINITE_ATTEMPT_NO -1

#define CLOSED_GRACEFULLY -2
#define TIMED_OUT -3

#pragma pack(1)
enum MsgType
{
	Error = 0,
	Registration,
	ShutDown,
	Ping,

	// for master
	TransactionCoordinatorRequest,
	TransactionCoordinatorRequestApproved,
	TransactionCoordinatorRequestRejected,
	TransactionFinishedRequest,

	// for node
	PREPARE_REQUEST,
	COMMIT_REQUEST,
	ROLLBACK_REQUEST,
	PREPARE_REPLY,
	COMMIT_REPLY,
	ROLLBACK_REPLY,

	Data
};

enum Errors
{
	NON_UNIQUE_ID = 0,
	TRANSACTION_ONLINE,
	NON_SUPPORTED_OPERATION,
	UNKNOWN
};

enum TransactionRequestType
{
	READ,
	WRITE,
	REMOVE_ONE,
	REMOVE_ALL
};

/*
	Generic message header, no matter if client or node initiates connection
	first int should be size of message; then, according to expected size, whole message is read
	and content pasted to other buffer
*/
typedef struct
{
	int size; // size of content without this field
	MsgType msgType;
	char *payload;
}Message;


typedef struct
{
	// combination of first 3
	// fields uniquely identifies message
	int clientId;
	int originId; // these two fields are setted up by node
	int originCounter;
	TransactionRequestType reqType;
}ClientMessageHeader;

bool InitializeWindowsSockets();

int SetSocketToNonBlocking(SOCKET *socket);

int bindSocket(SOCKET* listenSocket, char* port);

int connectToTarget(SOCKET* connectSocket, const char* addr, USHORT port);

int tryToSelect(SOCKET acceptedSocket, bool isSend, int sleepTime, int selectTimeSec, int noAttempt);

int receiveMessage(SOCKET communicationSocket, Message *outputMsg, int sleepTime, int selectTimeSec, int noAttempt, bool isRegMsg);

int sendMessage(SOCKET communicationSocket, Message *msgToSend, int sleepTime, int selectTimeSec, int noAttempt, bool isRegMsg);

void ErrorHandlerTxt(LPCTSTR lpszFunction);

void ErrorHandlerMsgBox(LPCTSTR lpszFunction);

#endif