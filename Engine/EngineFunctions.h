#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "ClientList.h"
#include "../Common/Structures.h"
#include "../Common/CommonFunctions.h"
#include "../Common/Dictionary.h"


void RecievePublisherMessage(Client_information **client_info, Topic_node **head, SOCKET acceptedSocket, char* recieveBuffer, int clientId){
	int iResult = -1;




	while (true)
	{
		SelectFunction(acceptedSocket, READ);
		iResult = recv(acceptedSocket, recieveBuffer, sizeof(int) * 2, 0); // pokupim informaciju o topicu i duzini poruke


		PublisherNode *node = (PublisherNode*)malloc(sizeof(PublisherNode));

		if (iResult > 0)
		{
			//printf("Message received from client: %s.\n", recieveBuffer);


			DeserializePublisherNode(node, recieveBuffer);


			int bytesRecieved = 0;
			int tempMessageLength = node->messageLength;

			do
			{
				iResult = recv(acceptedSocket, node->message + bytesRecieved, tempMessageLength - bytesRecieved, 0);
				if (iResult == SOCKET_ERROR || iResult == 0)
				{

					break;
				}
				bytesRecieved += iResult;

			} while (bytesRecieved < tempMessageLength);

			node->message[node->messageLength] = '\0';

			printf("\n\nPublisher sent: %s", node->message);
			printf("Forwarding message to subscribed clients..");


			char* topicName = FindTopicByIdTopicList(head, node->topicId);

			Node* clientsSubscribed = GetDictionaryValue(topicName);   // return clients subsrcibed to specific topic

			
			Node* current = clientsSubscribed;


			while (current != NULL)
			{

				//printf("\n\nID Subsribera kojem saljem: %d", current->clientID);
				Client_information *clientSubsrbiedToTopic = FindElementClientInformation(client_info, current->clientID);


				if (clientSubsrbiedToTopic != NULL)
				{
					Enqueue(&(clientSubsrbiedToTopic->subscriberMessages), node->topicId, node->message);
					ReleaseSemaphore(clientSubsrbiedToTopic->queueSemaphore, 1, NULL);
				}

				current = current->next;
			}

		
			Sleep(1000);


			free(node);
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			iResult = shutdown(acceptedSocket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
				WSACleanup();

			}

			printf("\n\nConnection with publisher closed.\n");
			closesocket(acceptedSocket);


			DeleteElementClientInformation(client_info, clientId);
		

			free(node);


			break;
		}
		else
		{
			int ierr = WSAGetLastError();
			if (ierr == WSAEWOULDBLOCK) {  // currently no data available
				Sleep(100);  // wait and try again
				//printf("OCEKIVANO PONASANJE!");
				continue;
			}
			else
			{
				// there was an error during recv
				//printf("recv failed with error: %d\n", WSAGetLastError());
				printf("\n\nConnection with publisher closed.\n");
				closesocket(acceptedSocket);


				DeleteElementClientInformation(client_info, clientId);
				
				free(node);
				break;
			}

		}
	}

	Sleep(100);




}



void RecieveSubsriberMessage(Client_information **client_info, Topic_node **head, SOCKET acceptedSocket, char* recieveBuffer, int clientId){
	int iResult = -1;
	int topicCounter = -1;
	int subsribedTopicCounter = -1;



	while (true)
	{
		SelectFunction(acceptedSocket, READ);
		iResult = recv(acceptedSocket, recieveBuffer, sizeof(int), 0); // pokupim informaciju o broj topica


		topicCounter = *(int*)recieveBuffer;

		if (iResult > 0)
		{
			

			int bytesRecieved = 0;
			int topicsLength = topicCounter * sizeof(int);

			char* topics = (char*)malloc(topicsLength);

			do
			{
				iResult = recv(acceptedSocket, topics + bytesRecieved, topicsLength - bytesRecieved, 0);
				if (iResult == SOCKET_ERROR || iResult == 0)
				{

					free(topics);

				}
				bytesRecieved += iResult;

			} while (bytesRecieved < topicsLength);


			
			int *topicsReal = (int*)topics;

			
			for (int i = 0; i < topicCounter; i++)
			{
				if (topicsReal[i] == 1) {
					char *topicName = FindTopicByIdTopicList(head, i);

					AddDictionaryElement(topicName, clientId);
				}

			}



			free(topics);

			break;





		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			iResult = shutdown(acceptedSocket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{

				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
				WSACleanup();

			}


			closesocket(acceptedSocket);


			DeleteElementClientInformation(client_info, clientId);
			
			printf("\nConnection with subscriber closed.\n");

			break;
		}
		else
		{
			int ierr = WSAGetLastError();
			if (ierr == WSAEWOULDBLOCK) {  // currently no data available
				Sleep(100);  // wait and try again
				
				continue;
			}
			else
			{
				// there was an error during recv
				
				closesocket(acceptedSocket);
				DeleteElementClientInformation(client_info, clientId);
				
				printf("\nConnection with subscriber closed.\n");

				break;
			}

		}

		Sleep(100);
	}

}
void SendMessageToSubscriber(Client_information** client_info, Topic_node** topic_head, int clientId){
	Client_information *client = NULL;


	int iResult = -1;
	bool end = false;



	while (true)
	{

		char *buffer = (char*)malloc(sizeof(int));
		client = FindElementClientInformation(client_info, clientId);

		if (client == NULL)
		{
			free(buffer);
			
			break;
		}

		WaitForSingleObject(client->queueSemaphore, INFINITE);





		int topicId = -1;

		int rw = SelectFunctionRW(client->clientSocket);

		if (rw == 0)
		{
			iResult = recv(client->clientSocket, buffer, sizeof(int), 0);
			free(buffer);

			iResult = shutdown(client->clientSocket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(client->clientSocket);
				

			}

			printf("\n\nConnection with subsriber closed.\n\n");
			closesocket(client->clientSocket);
			
			DeleteElementClientInformation(client_info, client->clientID);
			
			end = true;

			break;
		}


		if (client->subscriberMessages != NULL && !IsEmptyQueue(client->subscriberMessages))
		{

			char *messageFromQueue = (char*)malloc(sizeof(char) * MAX_MESSAGE_LEN);
			char *messageToSend = (char*)malloc(sizeof(char) * MAX_MESSAGE_LENGTH);

			Dequeue(&(client->subscriberMessages), messageFromQueue, &topicId);
			
			int bytesSent = 0;
			int size = strlen(messageFromQueue) + 2 * sizeof(int);

			*(int*)messageToSend = topicId;
			*(int*)(messageToSend + 4) = strlen(messageFromQueue);
			memcpy((messageToSend + 2 * sizeof(int)), messageFromQueue, size);

			

			do {


				SelectFunction(client->clientSocket, WRITE);
				iResult = send(client->clientSocket, messageToSend + bytesSent, size - bytesSent, 0);


				if (iResult == SOCKET_ERROR || iResult == 0)
				{
					iResult = shutdown(client->clientSocket, SD_SEND);
					if (iResult == SOCKET_ERROR)
					{
						printf("shutdown failed with error: %d\n", WSAGetLastError());
						closesocket(client->clientSocket);
						
					}


					printf("\n\nConnection with subsriber closed.\n\n");
					closesocket(client->clientSocket);
					DeleteElementClientInformation(client_info, client->clientID);
					
					end = true;
					break;
				}

				bytesSent += iResult;

			} while (bytesSent < size);

			printf("\nMessage sent to subsriber with id = %d", client->clientID);

			free(messageToSend);
			free(messageFromQueue);



			if (end)
				break;

		}


		free(buffer);
		Sleep(100);
	}

}


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

int SelectFunctionServer(SOCKET socket)
{


	while (1)
	{
		// Initialize select parameters
		FD_SET set;
		//FD_SET write;
		timeval timeVal;

		FD_ZERO(&set);
		//FD_ZERO(&write);

		// Add socket we will wait to read from
		FD_SET(socket, &set);


		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		int iResult;



		iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);


		if (iResult == 0) {
			// vreme za cekanje je isteklo
			//printf("Isteklooo");
			Sleep(100);
			//continue;

			return 0;
		}
		else if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
			printf("\nError occured in select function.. %d\n", WSAGetLastError());
		}
		else {
			// rezultat je jednak broju soketa koji su zadovoljili uslov

			return 1;
		}
	}


}


int RecieveInitialMessage(SOCKET socket, char* recieveBuffer)
{
	int iResult = -1;


	int clientType = -1;


	while (true)
	{
		SelectFunction(socket, READ);
		iResult = recv(socket, recieveBuffer, sizeof(int), 0); // pokupim informaciju o topicu 


		if (iResult > 0)
		{


			clientType = *(int*)recieveBuffer;


			if (clientType == 0)
				printf("\nNew Publisher connected!\n");
			else if (clientType == 1)
				printf("\nNew Subscriber connected!\n");
			else
				printf("Unknown client connected!\n");


			break;

		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			iResult = shutdown(socket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				WSACleanup();

			}


			printf("\nConnection with client closed.\n");
			closesocket(socket);
			break;
		}
		else
		{
			int ierr = WSAGetLastError();
			if (ierr == WSAEWOULDBLOCK) {  // currently no data available
				Sleep(100);  // wait and try again
				//printf("OCEKIVANO PONASANJE!");
				continue;
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				break;
			}

		}



		Sleep(100);
	}

	return clientType;
}
