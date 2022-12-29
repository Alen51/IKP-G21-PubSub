#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>

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
