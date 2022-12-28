// Publisher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

//#include "../Common/TopicRepository.h"
//#include "../Common/Common.h"
#include "PublisherFunctions.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016



bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}



int main()
{
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;


	char *messageToSend = (char*)malloc(sizeof(char) * DEFAULT_BUFLEN);





	// create a socket
	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(connectSocket == INVALID_SOCKET)
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

	unsigned long mode = 1;
	if (ioctlsocket(connectSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("ioctl failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	int publisher = 0;  // when we send 0 server know that publisher connected
	iResult = send(connectSocket, (char*)&publisher, sizeof(int), 0);



	closesocket(connectSocket);
	WSACleanup();

	return 0;
}

