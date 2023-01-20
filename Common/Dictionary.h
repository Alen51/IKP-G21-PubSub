#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <ws2tcpip.h>
#include "List.h"
#include "TopicList.h"

typedef struct Key_value_pair {

	char *topic;
	Node *listAllClients;
	struct Key_value_pair *next;

}Key_value_pair;




#define HASHSIZE 101
static Key_value_pair* hashTable[HASHSIZE];

CRITICAL_SECTION dictionary_cs;

unsigned HashFunction(char *key)
{
	unsigned hashValue;

	for (hashValue = 0; *key != '\0'; key++)
		hashValue = *key + 31 * hashValue;

	return hashValue % HASHSIZE;
}


// return clients subsrcibed to specific topic
Node* GetDictionaryValue(char *key)
{
	Key_value_pair* keyValuePair;
	keyValuePair = NULL;

	EnterCriticalSection(&dictionary_cs);

	for (keyValuePair = hashTable[HashFunction(key)]; keyValuePair != NULL; keyValuePair = keyValuePair->next)
		if (strcmp(key, keyValuePair->topic) == 0)
		{
			LeaveCriticalSection(&dictionary_cs);

			return keyValuePair->listAllClients;
		}


	LeaveCriticalSection(&dictionary_cs);

	return NULL;
}



// return key value pair from dictionary
Key_value_pair* FindDictionaryElement(char *key)
{
	Key_value_pair* keyValuePair;

	EnterCriticalSection(&dictionary_cs);

	for (keyValuePair = hashTable[HashFunction(key)]; keyValuePair != NULL; keyValuePair = keyValuePair->next)
		if (strcmp(key, keyValuePair->topic) == 0)
		{
			LeaveCriticalSection(&dictionary_cs);

			return keyValuePair;
		}

	LeaveCriticalSection(&dictionary_cs);

	return NULL;
}



// add element to dictionary 
Key_value_pair* AddDictionaryElement(char *topic, int clientId)
{
	Key_value_pair* keyValuePair;
	unsigned hashValue;

	EnterCriticalSection(&dictionary_cs);

	if ((keyValuePair = FindDictionaryElement(topic)) == NULL)
	{
		keyValuePair = (Key_value_pair*)malloc(sizeof(Key_value_pair));


		if (keyValuePair == NULL || (keyValuePair->topic = topic) == NULL)
		{
			LeaveCriticalSection(&dictionary_cs);
			return NULL;
		}

		hashValue = HashFunction(topic);
		keyValuePair->next = hashTable[hashValue];
		keyValuePair->listAllClients = NULL;
		InitializeClientsCriticalSection();

		hashTable[hashValue] = keyValuePair;
	}

	if ((keyValuePair->listAllClients) == NULL)
		InsertFirstClientList(&(keyValuePair->listAllClients), clientId);
	else
		InsertEndClientList(&(keyValuePair->listAllClients), clientId);

	LeaveCriticalSection(&dictionary_cs);

	return keyValuePair;
}

void InitializeDictionaryCriticalSection()
{
	InitializeCriticalSection(&dictionary_cs);
}


void DeleteDictionaryCriticalSection()
{
	DeleteCriticalSection(&dictionary_cs);
}


void FreeDictionary(Topic_node** head)
{
	Topic_node* current = *head;

	if (current == NULL)
	{
		printf("List is empty");


		return;
	}



	while (current != NULL)
	{

		Key_value_pair* key_value_pair = FindDictionaryElement(current->topic);

		//FreeClientList(&(key_value_pair->listAllClients));

		if (key_value_pair != NULL)
		{
			if ((key_value_pair->listAllClients) != NULL)
				FreeClientList(&(key_value_pair->listAllClients));

			free(key_value_pair);
		}



		current = current->next;
	}

	DeleteClientsCriticalSection();

	for (int i = 0; i < HASHSIZE; i++)
	{
		hashTable[i] = NULL;
	}




}
