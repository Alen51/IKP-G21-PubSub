#pragma once
#define WIN32_LEAN_AND_MEAN   

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ws2tcpip.h>
#define MAX_MESSAGE_LENGTH 512
#define DEFAULT_BUFLEN 512
#define MAX_TOPIC_LENGTH 256


typedef struct Topic_node {

	char topic[MAX_TOPIC_LENGTH];
	struct Topic_node* next;
}Topic_node;


typedef struct PublisherNode {

	int topicId; // id topica za koji se salje nova poruka
	int messageLength; // duzina poruke, koristice se za proveru celosti poruke
	char message[MAX_MESSAGE_LENGTH]; // tekst poruka

}PublisherNode;