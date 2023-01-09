
#pragma once
#define MAX_MESSAGE_LENGTH 512


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Common/TopicList.h"
#include "../Common/CommonFunctions.h"


bool ValidatePublisherInput(int selectedOption, int topicCounter)
{
	bool condition = (selectedOption < 0 || selectedOption > topicCounter) ? false : true;

	if (!condition)
	{
		if (topicCounter == 1)
			printf("\nYou can not select %d., try with option 0. or 1.\n", selectedOption);
		else
			printf("\nYou can not select %d., try with option beetween 0. and %d.\n", selectedOption, topicCounter);
	}

	return condition;
}

int SelectSpecificTopic(int topicCounter)
{
	int option = -1;

	if (topicCounter == 0)
	{
		printf("\nThere are no topics!\n");
		return -1;
	}

	do {

		printf("Select topic to release news: ");
		scanf_s("%d", &option);

		fflush(stdin);

	} while (!ValidatePublisherInput(option, topicCounter));

	return option;
}





void EnterMessage(char* newMessage)
{
	memset(newMessage, 0, MAX_MESSAGE_LENGTH);

	int a;

	printf("Enter new message for selected topic: ");
	while (newMessage[0] == 0 || strlen(newMessage) == 0 || newMessage[0] == 10) {

		fgets(newMessage, MAX_MESSAGE_LENGTH, stdin);
		fflush(stdin);
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

	while (fscanf_s(in, "%s", newTopic) != EOF) {



		InsertEndTopicList(head, newTopic);

		(*elementCounter)++;
	}
}

bool SendPublisherMessageToServer(SOCKET socket, char* messageToSend, int size)
{
	int bytesSent = 0;
	int iResult = -1;


	char *buffer = (char*)malloc(sizeof(int));





	do {


		int rw = SelectFunctionRW(socket);
		if (rw == 0)
		{
			//printf("\n\nServer poslao nesto!\n\n");
			iResult = recv(socket, buffer, sizeof(int), 0);

			free(buffer);


		}
		else
		{
			//printf("\n\nRedovno slanje sa punlishera\n\n");
			iResult = send(socket, messageToSend + bytesSent, size - bytesSent, 0);
		}



		if (iResult == 0)
		{
			// connection was closed gracefully
			iResult = shutdown(socket, SD_SEND);
			if (iResult == SOCKET_ERROR)
			{
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				WSACleanup();

			}


			printf("\n\nConnection with server closed.\n\n");
			closesocket(socket);
			WSACleanup();
			//break;

			return true;
		}
		else if (iResult < 0)
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
				// break;

				return true;
			}

		}

		bytesSent += iResult;

	} while (bytesSent < size);

    printf("\n\nMessage sucessfully sent!\n\Bytes Sent: %ld\n", iResult);

	return false;
}

char* SerializePublisherData(PublisherNode* publisherNode)
{
	int messageLength = strlen(publisherNode->message);

	int size = 2 * sizeof(int) + messageLength;
	char *buffer = (char*)malloc(sizeof(char) * (size + 1));


	*(int*)buffer = publisherNode->topicId;
	*(int*)(buffer + sizeof(int)) = messageLength;
	memcpy(buffer + sizeof(int) * 2, publisherNode->message, messageLength);
	buffer[size] = '\0';

	return buffer;
}