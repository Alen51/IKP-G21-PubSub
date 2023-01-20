#pragma once

typedef struct Node {

	int clientID;
	struct  Node* next;
}Node;


#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>

CRITICAL_SECTION clients_cs;

void InsertFirstClientList(Node** head, int clientID) {

	Node* newElement = (Node*)malloc(sizeof(Node));

	EnterCriticalSection(&clients_cs);

	if (*head == NULL)
	{
		newElement->clientID = clientID;
		newElement->next = NULL;
		*head = newElement;
	}
	else
	{
		newElement->clientID = clientID;
		newElement->next = *head;
		*head = newElement;
	}

	LeaveCriticalSection(&clients_cs);
}

void InsertEndClientList(Node** head, int clientID)
{

	EnterCriticalSection(&clients_cs);

	Node* current = *head;
	Node* newElement = (Node*)malloc(sizeof(Node));

	while (current->next != NULL)
	{
		current = current->next;
	}

	newElement->clientID = clientID;
	newElement->next = NULL;
	current->next = newElement;

	LeaveCriticalSection(&clients_cs);

}


bool IsEmptyClientList(Node** head)
{
	return *head == NULL ? true : false;
}


struct Node* FindElementClientList(struct Node** head, int clientID)
{

	EnterCriticalSection(&clients_cs);

	Node* current = *head;
	//Node* newElement = (Node*)malloc(sizeof(Node));

	if (current == NULL)
	{

		LeaveCriticalSection(&clients_cs);

		return NULL;
	}

	while (current != NULL)
	{
		if (current->clientID == clientID)
		{
			LeaveCriticalSection(&clients_cs);
			return current;
		}


		current = current->next;
	}

	LeaveCriticalSection(&clients_cs);

	return NULL;


}


void PrintClientList(Node** head) {


	EnterCriticalSection(&clients_cs);

	Node* current = *head;

	if (current == NULL)
	{
		printf("List is empty");

		LeaveCriticalSection(&clients_cs);
		return;
	}

	while (current != NULL)
	{

		printf("ClientID: %d\n", current->clientID);

		current = current->next;
	}

	LeaveCriticalSection(&clients_cs);

}

void DeleteElementClientList(Node **head, int clientId)
{

	EnterCriticalSection(&clients_cs);


	struct Node* temp = *head, *previous = NULL;


	if (temp != NULL && temp->clientID == clientId)
	{
		*head = temp->next;
		free(temp);

		LeaveCriticalSection(&clients_cs);

		return;
	}


	while (temp != NULL && temp->clientID != clientId)
	{
		previous = temp;
		temp = temp->next;
	}


	if (temp == NULL)
	{

		LeaveCriticalSection(&clients_cs);

		return;
	}


	previous->next = temp->next;

	free(temp);

	LeaveCriticalSection(&clients_cs);
}

void FreeClientList(Node **head)
{
	EnterCriticalSection(&clients_cs);

	Node* current = *head;

	while (current != NULL)
	{
		Node* temp = current;
		current = current->next;

		free(temp);

	}

	*head = NULL;

	LeaveCriticalSection(&clients_cs);

}


void InitializeClientsCriticalSection()
{
	InitializeCriticalSection(&clients_cs);
}

void DeleteClientsCriticalSection()
{
	DeleteCriticalSection(&clients_cs);
}