#pragma once
#define MAX_MESSAGE_LENGTH 512
/*void PrintPublisherMenu(Topic_node** head)
{
	printf("\n\n-----------------------Menu---------------------\n");
	printf("Add news to specific topic\n");
	//PrintTopicListWithCounter(head);
	printf("0. Finished with sending messages\n");
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
		scanf("%d", &option);

		fflush(stdin);

	} while (!ValidatePublisherInput(option, topicCounter));

	return option;
}*/

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