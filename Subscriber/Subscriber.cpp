#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "SubscriberFunctions.h"


#define DEFAULT_PORT 27016

bool InitializeWindowsSockets();

int __cdecl main()
{

	InitializeTopicListCriticalSection();

	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;




	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	PutSocketInNonblockingMode(&connectSocket);

	int subscriber = 1; //  // when we send 1 server know that subscriber connected
	iResult = send(connectSocket, (char*)&subscriber, sizeof(int), 0);


	FILE *topicsFile = SafeOpen("Topics.txt", "r");
	Topic_node* head = NULL;
	int counter;
	ReadAllTopics(topicsFile, &head, &counter);
	fclose(topicsFile);


	int choice = -1;
	int *subscribedTopics = (int*)malloc(counter * sizeof(int));

	memset(subscribedTopics, 0, counter * sizeof(int));

	bool isSubsribed = false;
	//int subscribedTopicsCounter = 0;

	do {


		do {

			PrintSubsriberMenu(&head, counter);
			choice = SubscribeToSpecificTopic(counter, subscribedTopics);

			if (choice != counter + 1)
			{
				subscribedTopics[choice - 1] = 1;
				//subscribedTopicsCounter++;
			}



			if ((choice - 1) != counter)
				isSubsribed = true;

			if (isSubsribed)
				PrintSubsribedTopics(&head, subscribedTopics, counter);

		} while (SubscribedToAllTopics(subscribedTopics, counter) && (choice != counter + 1));

		if (!isSubsribed)
			printf("\nYou have to subscribe to at least one topic!\n");

	} while (!isSubsribed);




	// send information about subscribed topics to server

	int size = (counter + 1) * sizeof(int);      //send to server topicId, topicCounter, arrayOfIntegers which represent subscribed topics

	char *initialMessageToServer = (char*)malloc(size);
	*(int*)(initialMessageToServer) = counter;
	memcpy(initialMessageToServer + sizeof(int), subscribedTopics, counter * sizeof(int));

	int bytesSent = 0;

	do {


		iResult = send(connectSocket, initialMessageToServer + bytesSent, size - bytesSent, 0);


		if (iResult == SOCKET_ERROR || iResult == 0)
		{
			break;
		}

		bytesSent += iResult;

	} while (bytesSent < size);



	printf("Bytes Sent: %ld\n", iResult);







	printf("\n\nPress ESC to close client subscriber...\n\n");

	DWORD lpThreadId;
	HANDLE hThreadRecieve;

	Important_data *importantData = (Important_data*)malloc(sizeof(Important_data));
	importantData->connectSocket = connectSocket;
	importantData->head = &head;

	hThreadRecieve = CreateThread(NULL, 0, &ThreadRecieveFunction, importantData, 0, &lpThreadId);

	if (hThreadRecieve == NULL)
	{
		printf("\nCreating thread for receiving messages failed!\n");

		return -1;
	}




	char c;

	do {

		c = getch();


	} while (c != 27); // if ESC break


	printf("\n\nSubscriber closed successfully!\n");

	char end = getchar();
	end = getchar();

	CloseHandle(hThreadRecieve);

	free(initialMessageToServer);
	free(subscribedTopics);
	FreeTopicList(&head);




	// cleanup

	shutdown(connectSocket, SD_BOTH);
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
