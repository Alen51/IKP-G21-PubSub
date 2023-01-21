#define _CRT_SECURE_NO_WARNINGS 
#pragma once
#define MAX_TOPIC_LENGTH 256
#define WIN32_LEAN_AND_MEAN   


#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ws2tcpip.h>
#include "Structures.h" 

CRITICAL_SECTION topicList_cs;




void InsertFirstTopicList(Topic_node** head, char* newTopic) {

	Topic_node* newElement = (Topic_node*)malloc(sizeof(Topic_node));

	EnterCriticalSection(&topicList_cs);

	if (*head == NULL)
	{
		strcpy_s(newElement->topic, newTopic);
		//newElement->topic = newTopic;
		newElement->next = NULL;
		*head = newElement;
	}
	else
	{
		strcpy_s(newElement->topic, newTopic);
		//newElement->topic = newTopic;
		newElement->next = *head;
		*head = newElement;
	}

	LeaveCriticalSection(&topicList_cs);


}

void InsertEndTopicList(Topic_node** head, char* newTopic)
{

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;



	Topic_node* newElement = (Topic_node*)malloc(sizeof(Topic_node));

	if (*head == NULL)
	{
		//char *str = _strdup(newTopic);
		strcpy_s(newElement->topic, newTopic);
		//newElement->topic = newTopic;
		newElement->next = NULL;
		*head = newElement;


		LeaveCriticalSection(&topicList_cs);

		return;
	}

	while (current->next != NULL)
	{
		current = current->next;
	}

	strcpy_s(newElement->topic, newTopic);

	//newElement->topic = newTopic;
	newElement->next = NULL;
	current->next = newElement;

	LeaveCriticalSection(&topicList_cs);

}


bool IsEmptyTopicList(Topic_node** head)
{
	return *head == NULL ? true : false;
}


struct Topic_node* FindElementTopicList(struct Topic_node** head, char* newTopic)
{

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;
	//Topic_node* newElement = (Topic_node*)malloc(sizeof(Topic_node));

	if (current == NULL)
	{
		LeaveCriticalSection(&topicList_cs);
		return NULL;
	}

	while (current != NULL)
	{


		if ((strcmp(current->topic, newTopic)) == 0)
		{
			LeaveCriticalSection(&topicList_cs);
			return current;
		}

		current = current->next;
	}

	LeaveCriticalSection(&topicList_cs);

	return NULL;

}

Topic_node* FindElementByIdTopicList(Topic_node** head, int id)
{

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;
	//Topic_node* newElement = (Topic_node*)malloc(sizeof(Topic_node));

	if (current == NULL)
	{
		LeaveCriticalSection(&topicList_cs);
		return NULL;
	}

	int counter = 0;

	while (current != NULL)
	{


		if (counter == id)
		{
			LeaveCriticalSection(&topicList_cs);
			return current;
		}

		counter++;
		current = current->next;
	}


	//free(newElement);

	LeaveCriticalSection(&topicList_cs);

	return NULL;
}


char* FindTopicByIdTopicList(Topic_node** head, int id)
{

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;
	//Topic_node* newElement = (Topic_node*)malloc(sizeof(Topic_node));

	if (current == NULL)
	{
		LeaveCriticalSection(&topicList_cs);
		return NULL;
	}

	int counter = 0;

	while (current != NULL)
	{


		if (counter == id)
		{
			LeaveCriticalSection(&topicList_cs);
			return current->topic;
		}

		counter++;
		current = current->next;
	}

	//free(newElement);

	LeaveCriticalSection(&topicList_cs);

	return NULL;
}


void PrintTopicList(Topic_node** head) {

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;

	if (current == NULL)
	{
		printf("List is empty");
		LeaveCriticalSection(&topicList_cs);

		return;
	}



	while (current != NULL)
	{

		printf("Topic: %s\n", current->topic);

		current = current->next;
	}

	LeaveCriticalSection(&topicList_cs);

}


void PrintTopicListWithCounter(Topic_node** head) {

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;

	if (current == NULL)
	{
		printf("List is empty");
		LeaveCriticalSection(&topicList_cs);

		return;
	}

	int counter = 0;

	while (current != NULL)
	{

		printf("%d. %s\n", counter + 1, current->topic);
		counter++;

		current = current->next;
	}

	LeaveCriticalSection(&topicList_cs);


}


void DeleteElementTopicList(Topic_node **head, char* newTopic)
{

	EnterCriticalSection(&topicList_cs);

	// Store head node 
	struct Topic_node* temp = *head, *previous = NULL;

	// If head node itself holds the key to be deleted 
	if (temp != NULL && (strcmp(temp->topic, newTopic)) == 0)
	{
		*head = temp->next;   // Changed head 
		free(temp);               // free old head 

		LeaveCriticalSection(&topicList_cs);

		return;
	}

	// Search for the key to be deleted, keep track of the 
	// previous node as we need to change 'prev->next' 
	while (temp != NULL && (strcmp(temp->topic, newTopic) != 0))
	{
		previous = temp;
		temp = temp->next;
	}

	// If key was not present in linked list 
	if (temp == NULL)
	{
		LeaveCriticalSection(&topicList_cs);

		return;
	}

	// Unlink the node from linked list 
	previous->next = temp->next;

	free(temp);  // Free memory 

	LeaveCriticalSection(&topicList_cs);
}

void FreeTopicList(Topic_node **head)
{

	EnterCriticalSection(&topicList_cs);

	Topic_node* current = *head;

	while (current != NULL)
	{
		Topic_node* temp = current;
		current = current->next;

		free(temp);

	}

	*head = NULL;

	LeaveCriticalSection(&topicList_cs);
	DeleteCriticalSection(&topicList_cs);
}

void InitializeTopicListCriticalSection()
{
	InitializeCriticalSection(&topicList_cs);
}



