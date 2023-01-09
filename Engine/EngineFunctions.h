#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Informations.h"
#include "../Common/Structures.h"

void RecievePublisherMessage(Client_information **client_info, Topic_node **head, SOCKET acceptedSocket, char* recieveBuffer, int clintId){/*TODO*/ }
void RecieveSubsriberMessage(Client_information **client_info, Topic_node **head, SOCKET acceptedSocket, char* recieveBuffer, int clintId){/*TODO*/ }
void SendMessageToSubscriber(Client_information** client_info, Topic_node** topic_head, int clientId){/*TODO*/}


SOCKET AcceptNewSocket(SOCKET listenSocket)
{

	SOCKET acceptedSocket = accept(listenSocket, NULL, NULL);

	if (acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	return acceptedSocket;
}

bool PutSocketInNonblockingMode(SOCKET* listenSocket) {

	unsigned long mode = 1;

	if (ioctlsocket(*listenSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("ioctl failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return false;
	}

	printf("Socket is now in nonblocking mode.\n");
	return true;
}
