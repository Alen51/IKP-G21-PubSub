#pragma once


#include <stdio.h>
#include <stdlib.h>

#include "Queue.h"
#include <ws2tcpip.h>

typedef struct Client_information {

	int clientID;
	SOCKET clientSocket;
	DWORD lpThreadId;
	HANDLE handleClientThread;
	HANDLE queueSemaphore;
	Queue* subscriberMessages;

	struct Client_information* next;

}Client_information;

CRITICAL_SECTION clientInformation_cs;


void InsertFirstClientInformation(Client_information** head, int clientID, SOCKET clientSocket, DWORD lpThreadId, HANDLE handleClientThread) {

	Client_information* newElement = (Client_information*)malloc(sizeof(Client_information));

	EnterCriticalSection(&clientInformation_cs);

	if (*head == NULL)
	{
		newElement->clientID = clientID;
		newElement->clientSocket = clientSocket;
		newElement->lpThreadId = lpThreadId;
		newElement->handleClientThread = handleClientThread;
		newElement->queueSemaphore = CreateSemaphore(0, 0, QUEUE_MAX_SIZE, NULL);
		InitializeQueue(&(newElement->subscriberMessages));

		newElement->next = NULL;

		*head = newElement;
	}
	else
	{
		newElement->clientID = clientID;
		newElement->clientSocket = clientSocket;
		newElement->lpThreadId = lpThreadId;
		newElement->handleClientThread = handleClientThread;
		newElement->queueSemaphore = CreateSemaphore(0, 0, QUEUE_MAX_SIZE, NULL);
		InitializeQueue(&(newElement->subscriberMessages));

		newElement->next = *head;

		*head = newElement;
	}

	LeaveCriticalSection(&clientInformation_cs);

}

void InsertEndClientInformation(Client_information** head, int clientID, SOCKET clientSocket, DWORD lpThreadId, HANDLE handleClientThread)
{

	EnterCriticalSection(&clientInformation_cs);

	Client_information* current = *head;
	Client_information* newElement = (Client_information*)malloc(sizeof(Client_information));

	if (*head == NULL)
	{
		newElement->clientID = clientID;
		newElement->clientSocket = clientSocket;
		newElement->lpThreadId = lpThreadId;
		newElement->handleClientThread = handleClientThread;
		newElement->queueSemaphore = CreateSemaphore(0, 0, QUEUE_MAX_SIZE, NULL);
		InitializeQueue(&(newElement->subscriberMessages));

		newElement->next = NULL;

		*head = newElement;

		LeaveCriticalSection(&clientInformation_cs);

		return;
	}

	while (current->next != NULL)
	{
		current = current->next;
	}

	newElement->clientID = clientID;
	newElement->clientSocket = clientSocket;
	newElement->lpThreadId = lpThreadId;
	newElement->handleClientThread = handleClientThread;
	newElement->queueSemaphore = CreateSemaphore(0, 0, QUEUE_MAX_SIZE, NULL);
	InitializeQueue(&(newElement->subscriberMessages));

	newElement->next = NULL;

	current->next = newElement;

	LeaveCriticalSection(&clientInformation_cs);

}


bool IsEmptyClientInformation(Client_information** head)
{
	return *head == NULL ? true : false;
}


struct Client_information* FindElementClientInformation(Client_information** head, int clientID)
{

	EnterCriticalSection(&clientInformation_cs);

	Client_information* current = *head;
	//Client_information* newElement = (Client_information*)malloc(sizeof(Client_information));

	if (current == NULL)
	{
		LeaveCriticalSection(&clientInformation_cs);

		return NULL;

	}

	while (current != NULL)
	{
		if (current->clientID == clientID)
		{
			LeaveCriticalSection(&clientInformation_cs);
			return current;
		}

		current = current->next;
	}


	LeaveCriticalSection(&clientInformation_cs);

	return NULL;



}


void PrintClientInformationList(Client_information** head) {

	EnterCriticalSection(&clientInformation_cs);

	Client_information* current = *head;

	if (current == NULL)
	{
		printf("List is empty");
		LeaveCriticalSection(&clientInformation_cs);
		return;
	}

	while (current != NULL)
	{

		printf("\n\nClientInfomrationElement\n");
		printf("ClientID: %d\n", current->clientID);
		printf("lpThreadId: %d\n", current->lpThreadId);

		if (current->handleClientThread != NULL)
			printf("handleClientThread have assigned value. Thread succesfully created!");
		else
			printf("handleClientThread does not have assigned value. Thread unsuccesfully created!");

		if (current->queueSemaphore != NULL)
			printf("queueSemaphore have assigned value. Semaphore succesfully created!");
		else
			printf("queueSemaphore does not have assigned value. Semaphore unsuccesfully created!");


		current = current->next;
	}

	LeaveCriticalSection(&clientInformation_cs);

}

void DeleteElementClientInformation(Client_information **head, int clientId)
{
	EnterCriticalSection(&clientInformation_cs);

	struct Client_information* temp = *head, *previous = NULL;

	if (temp != NULL && temp->clientID == clientId)
	{
		*head = temp->next;



		CloseHandle(temp->handleClientThread);
		CloseHandle(temp->queueSemaphore);
		FreeQueue(&(temp->subscriberMessages));

		free(temp);


		LeaveCriticalSection(&clientInformation_cs);

		return;
	}


	while (temp != NULL && temp->clientID != clientId)
	{
		previous = temp;
		temp = temp->next;
	}


	if (temp == NULL)
	{
		LeaveCriticalSection(&clientInformation_cs);
		return;
	}


	previous->next = temp->next;


	CloseHandle(temp->queueSemaphore);
	CloseHandle(temp->handleClientThread);
	FreeQueue(&(temp->subscriberMessages));

	free(temp);

	LeaveCriticalSection(&clientInformation_cs);
}

void FreeClientInformationList(Client_information **head)
{

	EnterCriticalSection(&clientInformation_cs);

	Client_information* current = *head;

	while (current != NULL)
	{
		Client_information* temp = current;
		current = current->next;





		int iResult = shutdown(temp->clientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(temp->clientSocket);
			WSACleanup();
			return;
		}

		//CloseHandle(temp->handleClientThread);
		CloseHandle(temp->queueSemaphore);
		closesocket(temp->clientSocket);
		FreeQueue(&(temp->subscriberMessages));


		free(temp);

	}

	*head = NULL;

	LeaveCriticalSection(&clientInformation_cs);
	DeleteCriticalSection(&clientInformation_cs);
}

void InitializeClientInformationCriticalSection()
{
	InitializeCriticalSection(&clientInformation_cs);
}

void CloseAllHandles(Client_information **head)
{
	EnterCriticalSection(&clientInformation_cs);

	Client_information* current = *head;

	while (current != NULL)
	{
		Client_information* temp = current;
		current = current->next;

		TerminateThread(temp->handleClientThread, temp->lpThreadId);
		CloseHandle(temp->handleClientThread);

	}

	LeaveCriticalSection(&clientInformation_cs);
}
