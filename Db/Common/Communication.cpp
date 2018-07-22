#include "stdafx.h"
#include "Communication.h"
#include <strsafe.h>
#include <tchar.h>

/*
	Initialize windows sockets library for calling process.
	Returns true if initialazing WS2_32.dll succeed, otherwise false.
*/
bool InitializeWindowsSockets()
{
	WSADATA wsaData;

	// Initialize windows sockets library (WS2_32.dll) for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

int SetSocketToNonBlocking(SOCKET *socket)
{
	int iResult;
	unsigned long int nonBlockingMode = 1;

	iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		ErrorHandlerTxt(TEXT("ioctlsocket"));
	}

	return iResult;
}

///<param name='listenSocket'>A descriptor identifying an unbound socket.</param>
///<param name='port'>Service name or port number represented as a string.</param>
/*
	Trying to associate a local address with a socket. Parameters are a descriptor identifying an unbound socket,
	service name or port number represented as a string. Returns zero if bind successfull,
	otherwise returns number representing error code.
*/
int bindSocket(SOCKET* listenSocket, char* port)
{
	int iResult;
	addrinfo  *resultingAddress = NULL;

	// hints parameter specifies the preferred socket type, or protocol.
	// null hints specifies that any network address or protocol is acceptable
	addrinfo  hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;

	// protocol-independent translation from an ANSI host name to an address
	iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		ErrorHandlerTxt(TEXT("bindSocket.gettadrinfo"));
		return iResult;
	}

	*listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*listenSocket == INVALID_SOCKET)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("bindSocket.socket"));
		freeaddrinfo(resultingAddress);
		return iResult;
	}

	// Setup the TCP listening socket
	iResult = bind(*listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("bindSocket.bind"));
		freeaddrinfo(resultingAddress);

		if (closesocket(*listenSocket) == SOCKET_ERROR)
		{
			ErrorHandlerTxt(TEXT("bindSocket.bind.closesocket"));
		}
		return iResult;
	}

	freeaddrinfo(resultingAddress);
	return 0;
}

int connectToTarget(SOCKET* connectSocket, const char* addr, USHORT port)
{
	int iResult;
	*connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("connectToTarget"));
		return iResult;;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &(serverAddress.sin_addr));
	serverAddress.sin_port = htons(port);

	if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		int iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("Unable to connect to target"));
		if (closesocket(*connectSocket) == SOCKET_ERROR)
		{
			ErrorHandlerTxt(TEXT("connectToTarget.connect.closesocket(connectSocket)"));
		}
		return iResult;
	}

	if (SetSocketToNonBlocking(connectSocket) == SOCKET_ERROR)
	{		
		if (closesocket(*connectSocket) == SOCKET_ERROR)
		{
			ErrorHandlerTxt(TEXT("main.connect.closesocket(connectSocket)"));
		}
		return WSAGetLastError();
	}

	return 0;
}


///<param name='acceptedSocket'>Socket to be checked for readability/writeability</param>
///<param name='isSend'>bool flag that indicates operation to be checked on socket. <value>True</value> if socket is checking on for writeability <value>true</value>; otherwise <value>false</value></param>
///<param name='sleepTime'>Time in ms allowed to pass between current and next select attempt if there is no success or error indication in current attempt.</param>
///<param name='noAttempt'>Max number of attempts for selecting before function returns; -1(INFINITE_ATTEMPT_NO) for infinite number of attempts </param>
/*
Trying to select socket ready for read/write in a non blocking manner. Parameters are Socket to be checked, bool flag that indicates operation (read/write) to be checked on socket,
Time in ms allowed to pass between current and next select attempt if there is no success or error indication in current attempt,
Max number of attempts for selecting before function returns; -1(INFINITE_ATTEMPT_NO) for infinite number of attempts.
Returns SCOKET_ERROR if an error occured, 0 if time limit expired or number of sockets meeting a condition if selecting succeed.
*/
int tryToSelect(SOCKET acceptedSocket, bool isSend, int sleepTime, int noAttempt)
{
	do
	{
		int iResult;

		FD_SET set;
		FD_ZERO(&set);
		FD_SET(acceptedSocket, &set);

		timeval timeVal;

		// both fields zero -> select returns immediately, it <does not block>
		// null value -> select block indefinitely
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		if (!isSend)
			iResult = select(0, &set, NULL, NULL, &timeVal); // set readfds parameter to set value		
		else
			iResult = select(0, NULL, &set, NULL, &timeVal); // set writefds to set value

		// if number of attempts is different than INFINITE_ATTEMPT_NO loop should end; 
		// otherwise it should last till the end of time :) 
		if (noAttempt != INFINITE_ATTEMPT_NO)
			noAttempt--;

		/********************* SELECT *************************************
		==========read======================================================
		readfds identifies the sockets that are to be checked for readability
		if the socket is in the <listen> state it will be marked as readable if
		an <incoming connection> request has been received such that an accept is
		guaranteed to complete without blocking.

		for <other sockets state> readability means that queued <data is available>
		for reading such that a call to recv, recvfrom is guaranteed not to block

		*for connection oriented sockets, readability can also indicate that a request to
		CLOSE the socket (closesocket != shutdown, but closesocket implicitly causes a shutdown sequence to occur)
		has been received from the peer.

			-If the virtual circuit was closed gracefully, and all the data was received,
			 then a <recv> will return immediately with <zero> bytes read.

			-It the virtual circuit was reset, then a <recv> will
			 complete immediately with an error code such as <wsaeconnreset>.

		==========write=====================================================
		writefds identiefies the sockets that are to be checked for writeablity.
		-If a socket is processing a <connect> call (nonblocking), a socket is writeable
		if the connection establishment succeffsully completes.
		-If the socket is not processing a connect call, writeablity means a <send>,
		<sendto> are guaranteed to succeed. However, they <can block> on a blocking socket
		if the len parameter exceeds the amount of outgoing system buffer space available.

		*/

		if (iResult == SOCKET_ERROR)
		{
			return iResult;
		}

		// if there is stil unused attempts
		if (noAttempt != 0)
		{
			// if there is stil no success neither error -> continue to attempt
			if (iResult == 0)
			{
				Sleep(sleepTime);
				continue;
			}
			// success
		}

		// returning number of sockets or zero as timeout indication
		return iResult;

	} while (true);
}


///<param name='communicationSocket'>Socket on which receiveing of data should be performed</param>
///<param name='outputMsg'>Received data</param>
///<param name='sleepTime'>Time in ms allowed to pass between current and next select attempt to determine if socket is ready for receiving.</param>
///<param name='noAttempt'>Max number of attempts for selecting for reading before function returns; -1(INFINITE_ATTEMPT_NO) for infinite number of attempts</param>
///<param name='isRegMsg'>Value that indicated that sleepTime and noAttempt parameters should be configured in runtime differently if it is registration message.</param>
/*
Trying to receive whole message. If succeed returns 0; otherwise special error code.
*/
int receiveMessage(SOCKET communicationSocket, Message *outputMsg, int sleepTime, int noAttempt, bool isRegMsg)
{
	int iResult;
	int currLength = 0;
	int expectedPayloadSize = -1;

	//printf("\n------receiveMessage, before receiving first 8 header bytes");
	iResult = tryToSelect(communicationSocket, false, sleepTime, noAttempt);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("receiveMessage.tryToSelect (first 8 bytes)"));
		return iResult;
	}
	if (iResult == 0)
	{
		printf("\nTime limit expired for select (recv) operation (first 8 bytes)");
		ErrorHandlerTxt(TEXT("receiveMessage.tryToSelect (first 8 bytes)"));
		return TIMED_OUT;
	}

	// determining expected msg size (how many bytes left to read) and expected MsgType
	iResult = recv(communicationSocket, (char*)outputMsg, 8, 0);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("receiveMessage.recv (first 8 bytes)"));
		return iResult;
	}
	else if (iResult == 0)
	{
		// If the virtual circuit was closed gracefully, and all the data was received, then a <recv> will return immediately with zero bytes read.
		printf("\nTarget closed connection gracefully.");
		return CLOSED_GRACEFULLY;
	}
	else if (iResult > 0)
	{
		expectedPayloadSize = outputMsg->size - 4;
		outputMsg->payload = (char*)calloc(expectedPayloadSize, sizeof(char));
		//printf("\n----------received first %d header bytes, expected %d payload bytes", iResult, expectedPayloadSize);
	}

	// node skoro istom frekvencijoom salje (samo malo sporije)
	if (isRegMsg && noAttempt != INFINITE_ATTEMPT_NO) {
		noAttempt /= 10;
		sleepTime *= (10 * 6);
	}

	while (currLength < expectedPayloadSize)
	{
		//printf("\n-------------Looping for receiving payload");
		iResult = tryToSelect(communicationSocket, false, sleepTime, noAttempt);
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			ErrorHandlerTxt(TEXT("receiveMessage.tryToSelect (payload)"));
			return iResult;
		}
		if (iResult == 0)
		{
			printf("\nTime limit expired for select operation (payload)");
			ErrorHandlerTxt(TEXT("receiveMessage.tryToSelect (payload)"));
			return TIMED_OUT;
		}

		iResult = recv(communicationSocket, outputMsg->payload + currLength, expectedPayloadSize - currLength, 0);

		if (iResult > 0)
			currLength += iResult;

		// else cases -> there is no whole message received for sure
		else if (iResult == 0)
		{
			printf("\nTarget closed connection gracefully.");
			return CLOSED_GRACEFULLY;
		}
		else
		{
			iResult = WSAGetLastError();
			ErrorHandlerTxt(TEXT("receiveMessage.recv (payload)"));
			return iResult;
		}
		//printf("\n-------------currently received = %d payload bytes", currLength);
	}

	//printf("\n----------before receiveMessage returning, received totally %d bytes\n", currLength + 8);
	return 0;
}

///<param name='communicationSocket'>Socket on which sending of data should be performed</param>
///<param name='msgToSend'>Data for sending</param>
///<param name='sleepTime'>Time in ms allowed to pass between current and next select attempt to determine if socket is ready for sending.</param>
///<param name='noAttempt'>Max number of attempts for selecting for sending before function returns; -1(INFINITE_ATTEMPT_NO) for infinite number of attempts</param>
///<param name='isRegMsg'>Value that indicated that sleepTime and noAttempt parameters should be configured in runtime differently if it is registration message.</param>
/*
Trying to send whole message. If succeed returns expected number of bytes; otherwise special error code.
*/
int sendMessage(SOCKET communicationSocket, Message *msgToSend, int sleepTime, int noAttempt, bool isRegMsg)
{
	int iResult = SOCKET_ERROR;

	int firstPartOfMessageBytesSent = 0;
	int payloadBytesSent = 0;
	int payloadLength = msgToSend->size - sizeof(MsgType);  // msgToSend->size equals payload size + sizeof(MsgType)
	//printf("\n\n------sendMessage, exactly %d bytes for sending", msgToSend->size + 4);

	iResult = tryToSelect(communicationSocket, true, sleepTime, noAttempt);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("sendMessage.tryToSelect (first 8 bytes)"));
		return iResult;
	}
	if (iResult == 0)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("sendMessage.tryToSelect (Time limit - first 8 bytes)"));
		return TIMED_OUT;
	}

	iResult = send(communicationSocket, (char*)msgToSend, 8, 0); // 8 = sizeof int + sizeeof MessageType
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		ErrorHandlerTxt(TEXT("sendMessage.send (first 8 bytes)"));
		return iResult;
	}

	firstPartOfMessageBytesSent = iResult;
	if (firstPartOfMessageBytesSent != 8)
	{
		printf("\n---------not good-------nije poslato koliko treba inicijalno. mozda je pun odlazni bafer...");
	}
	//printf("\n----------sent first %d header bytes; expected %d payload bytes", firstPartOfMessageBytesSent, payloadLength);

	if (isRegMsg && noAttempt != INFINITE_ATTEMPT_NO) {
		noAttempt /= 10;
		sleepTime *= (10 * 6);
	}

	while (payloadBytesSent < payloadLength)
	{
		iResult = tryToSelect(communicationSocket, true, sleepTime, noAttempt);
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			ErrorHandlerTxt(TEXT("sendMessage.tryToSelect (payload)"));
			return iResult;
		}
		if (iResult == 0)
		{
			printf("\nTime limit expired for select operation (payload)");
			ErrorHandlerTxt(TEXT("sendMessage.tryToSelect (payload)"));
			return TIMED_OUT;
		}

		iResult = send(communicationSocket, (char*)(msgToSend->payload) + payloadBytesSent, payloadLength - payloadBytesSent, 0);
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			ErrorHandlerTxt(TEXT("sendMessage.send (payload)"));
			return iResult;
		}

		payloadBytesSent += iResult;
	}

	//printf("\n--------before sendMessage returning, sent totally %d bytes\n", payloadBytesSent + firstPartOfMessageBytesSent);
	return payloadBytesSent + firstPartOfMessageBytesSent;
}

void ErrorHandlerTxt(LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	// Display the error message.
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("\n< %s > failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);


	_tprintf((LPTSTR)lpDisplayBuf);

	// Free error-handling buffer allocations.
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

void ErrorHandlerMsgBox(LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);

	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
