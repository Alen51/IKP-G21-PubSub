#pragma once
#include "../Common/TopicList.h"
#include "../Common/CommonFunctions.h"

typedef struct Important_data {

	Topic_node **head;
	SOCKET connectSocket;

}Important_data;

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


bool ValidateSubscriberInput(int selectedOption, int topicCounter)
{
	bool condition = (selectedOption < 1 || selectedOption > topicCounter + 1) ? false : true;

	if (!condition)
	{
		if (topicCounter == 1)
			printf("\nMozete odabrati %d., probajte sa opcijom 1.\n", selectedOption);
		else
			printf("\nMozete odabrati %d., probajte sa opcijom izmedju 1. i %d.\n", selectedOption, topicCounter + 1);
	}

	return condition;

}

int SubscribeToSpecificTopic(int topicCounter, int* subscribedTopics)
{
	int option = -1;

	if (topicCounter == 0)
	{
		printf("\nNema nijedna tema!\n");
		return -1;
	}

	do {

		printf("Izabrite temu da se pretplatite: ");
		scanf("%d", &option);

		if (subscribedTopics[option - 1] == 1)
			printf("\nVec ste pretplaceni na datu temu!\n");

	} while (!ValidateSubscriberInput(option, topicCounter));

	return option;
}

void PrintSubsribedTopics(Topic_node** head, int *subscribedTopics, int topicCounter)
{



	printf("\n*****************************************************************\n");
	printf("Pretplaceni ste na ove teme: ");

	for (int i = 0; i < topicCounter; i++)
	{
		if (subscribedTopics[i] == 1)
		{
			//subsribedTo = FindElementByIdTopicList(head, i);
			//printf("%s, ", subsribedTo->topic);

			printf("%s, ", (FindTopicByIdTopicList(head, i)));

		}
	}

	printf("\n*****************************************************************\n\n");


}

bool SubscribedToAllTopics(int *subscribedTopics, int topicCounter)
{
	int i = 0;

	for (i = 0; i < topicCounter; i++)
	{
		if (subscribedTopics[i] == 0)
			return true;

	}

	return false;
}

DWORD WINAPI ThreadRecieveFunction(LPVOID lpvThreadParam)
{

	Important_data *importantData = (Important_data*)lpvThreadParam;

	SOCKET connectSocket = importantData->connectSocket;
	Topic_node **head = importantData->head;

	free(lpvThreadParam);

	char recvbuf[DEFAULT_BUFLEN];



	printf("\n\n----------------------------------------------------------------\n");
	printf("Waiting for news from publisher..\n\n");

	SelectFunction(connectSocket, READ);
	RecieveServerMessage(head, connectSocket, recvbuf);


	return 0;
}

