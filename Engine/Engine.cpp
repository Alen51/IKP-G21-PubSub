// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Queue.h"
#include "ThreadFunctions.h"
#include "../Publisher/PublisherFunctions.h"
#include "ClientList.h";

#define DEFAULT_PORT "27000"

HANDLE handleFinishSignal;
DWORD WINAPI ThreadExitFunction(LPVOID lpvThreadParam);

int main()
{
	Client_information *clientInformation = NULL;

	InitializeClientInformationCriticalSection();
	InitializeDictionaryCriticalSection();
	InitializeTopicListCriticalSection();

	handleFinishSignal = CreateSemaphore(0, 0, 1, NULL);

	FILE *topicsFile = SafeOpen((char*)"Topics.txt", (char*)"r");
	Topic_node* head = NULL;
	int counter;
	ReadAllTopics(topicsFile, &head, &counter);
	fclose(topicsFile);

	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket = INVALID_SOCKET;

	int iResult;

	char *recvbuf = (char*)malloc(sizeof(char) * DEFAULT_BUFLEN);

	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}


	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}


	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 0;
	}

	if (!PutSocketInNonblockingMode(&listenSocket))
		return 1;

	printf("Server initialized, waiting for clients.\n");

	DWORD lpThreadIdExit = -1;
	HANDLE handleExitThread = NULL;


	handleExitThread = CreateThread(NULL, 0, &ThreadExitFunction, NULL, 0, &lpThreadIdExit);

	DWORD lpThreadId = -1;
	HANDLE handleClientThread = NULL;


	int clientId = -1;
	int clientType = -1;

	ClientThradData *clientThreadData = (ClientThradData*)malloc(sizeof(ClientThradData));


	clientThreadData->head_clientInfo = &clientInformation;
	clientThreadData->topic_head = &head;

	char endServer;


	while (WaitForSingleObject(handleFinishSignal, 10) == WAIT_TIMEOUT)
	{

		if (SelectFunctionServer(listenSocket) == 0)
			continue;

		acceptedSocket = AcceptNewSocket(listenSocket);
		int clientType = RecieveInitialMessage(acceptedSocket, recvbuf); // return information about clientType ( Publisher or Subsriber)

		clientId++;

		clientThreadData->clientId = clientId;
		clientThreadData->acceptedSocket = acceptedSocket;

		if (clientType == 0) // publisher connected
		{
			// create thread for publisher with publisher function
			handleClientThread = CreateThread(NULL, 0, &ThreadPublisherFunction, clientThreadData, 0, &lpThreadId);

			if (handleClientThread == NULL)
			{
				printf("\nCreating publisher thread failed!\n");

				return -1;
			}

			InsertEndClientInformation(&clientInformation, clientId, acceptedSocket, lpThreadId, handleClientThread);
		}
		else if (clientType == 1) // subsriber connected
		{
			// create thread for subscriber with subscriber function

			handleClientThread = CreateThread(NULL, 0, &ThreadSubscriberFunction, clientThreadData, 0, &lpThreadId);


			if (handleClientThread == NULL)
			{
				printf("\nCreating subscriber thread failed!\n");

				return -1;
			}

			InsertEndClientInformation(&clientInformation, clientId, acceptedSocket, lpThreadId, handleClientThread);
		}

	}



	printf("\n\nPress any key to close server..\n\n");




	char c = getchar();
	c = getchar();




	CloseAllHandles(&clientInformation);




	FreeClientInformationList(&clientInformation);
	FreeDictionary(&head);
	FreeTopicList(&head);
	free(recvbuf);
	free(clientThreadData);


	CloseHandle(handleFinishSignal);



	closesocket(listenSocket);
	WSACleanup();

	printf("\n\nServer successfully closed..\n\n");

	char end = getchar();

	return 0;
}

DWORD WINAPI ThreadExitFunction(LPVOID lpvThreadParam)
{



	char endServer;

	printf("\n\nPress ESC to close server..\n\n");

	do {


		endServer = getchar();


	} while (endServer != 27);

	printf("\nServer is closing down..\n");



	ReleaseSemaphore(handleFinishSignal, 1, NULL);


	return 0;
}

