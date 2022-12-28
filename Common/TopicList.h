#pragma once
#define MAX_TOPIC_LENGTH 256

typedef struct Topic_node {

	char topic[MAX_TOPIC_LENGTH];
	struct Topic_node* next;
}Topic_node;