#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include "TopicList.h"

typedef enum SelectOption { READ, WRITE };

int SelectFunctionRW(SOCKET socket)
{
	while (1)
	{
		// Initialize select parameters
		FD_SET set;
		FD_SET write;
		timeval timeVal;

		FD_ZERO(&set);
		FD_ZERO(&write);

		// Add socket we will wait to read from
		FD_SET(socket, &set);
		FD_SET(socket, &write);
		// Set timeouts to zero since we want select to return
		// instantaneously

		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		int iResult;


		iResult = select(0 /* ignored */, &set, &write, NULL, &timeVal);


		if (iResult == 0) {
			// vreme za cekanje je isteklo
			//printf("Isteklooo");
			Sleep(100);
			continue;
		}
		else if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
			printf("\nError occured in select function.. %d\n", WSAGetLastError());
		}
		else {
			// rezultat je jednak broju soketa koji su zadovoljili uslov
			if (FD_ISSET(socket, &set))
			{
				return 0;
			}
			else if (FD_ISSET(socket, &write))
			{
				return 1;
			}
		}
	}

}


void SelectFunction(SOCKET socket, SelectOption option)
{
	while (1)
	{
		// Initialize select parameters
		FD_SET set;
		timeval timeVal;

		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(socket, &set);
		// Set timeouts to zero since we want select to return
		// instantaneously

		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		int iResult;

		if (option == READ)
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		else if (option == WRITE)
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		else
		{
			printf("\nUnknown SelectOption!\n");
			return;
		}


		if (iResult == 0) {
			// vreme za cekanje je isteklo
			//printf("Isteklooo");
			Sleep(100);
			continue;
		}
		else if (iResult == SOCKET_ERROR) {
			//desila se greska prilikom poziva funkcije
			//printf("\nError occured in select function..\n");
			printf("\nError occured in select function.. %d\n", WSAGetLastError());
		}
		else {
			// rezultat je jednak broju soketa koji su zadovoljili uslov
			return;
		}
	}



}



FILE *SafeOpen(char filename[], char mode[]) {

	FILE *fp = fopen(filename, mode);

	if (fp == NULL) {

		printf("Can't open '%s'!", filename);
		char c = getchar();
		exit(-1);
	}
	return fp;
}

void ReadAllTopics(FILE *in, Topic_node** head, int* elementCounter)
{
	*elementCounter = 0;

	char newTopic[MAX_TOPIC_LENGTH];

	while (fscanf(in, "%s", newTopic) != EOF) {



		InsertEndTopicList(head, newTopic);

		(*elementCounter)++;
	}
}

void RecieveServerMessage(Topic_node **head, SOCKET socket, char* recieveBuffer)
{
	int iResult = -1;


	int topicId = -1;
	int messageLength = -1;


	while (true)
	{
		SelectFunction(socket, READ);
		iResult = recv(socket, recieveBuffer, 2 * sizeof(int), 0); // get information about topicId and messageLength


		topicId = *(int*)recieveBuffer;
		messageLength = *(int*)(recieveBuffer + sizeof(int));


		if (iResult > 0)
		{


			int bytesRecieved = 0;


			char *messageFromPublisher = (char*)malloc(sizeof(char)*messageLength);

			do
			{
				iResult = recv(socket, messageFromPublisher + bytesRecieved, messageLength - bytesRecieved, 0);
				if (iResult == SOCKET_ERROR || iResult == 0)
				{

					break;
				}
				bytesRecieved += iResult;

			} while (bytesRecieved < messageLength);

			messageFromPublisher[messageLength - 1] = '\0';


			

				// dobavim niz id jeva topica

			char *topic = FindTopicByIdTopicList(head, topicId);

			printf("Publisher %s released new information: %s\n", topic, messageFromPublisher);

			free(messageFromPublisher);





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


			printf("\nConnection with server closed.\n");
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




}
