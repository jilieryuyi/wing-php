
/**
 *@完成端口模型
 */

#include "stdafx.h"
#include <WINSOCK2.H>
#include <stdio.h>

#define PORT    5150
#define MSGSIZE 1024
#pragma comment(lib, "ws2_32.lib")

typedef enum
{
	RECV_POSTED
}OPERATION_TYPE;

typedef struct
{

	WSAOVERLAPPED  overlap;
	WSABUF         Buffer;
	char           szMessage[MSGSIZE];
	DWORD          NumberOfBytesRecvd;
	DWORD          Flags;
	OPERATION_TYPE OperationType;

}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

 
DWORD WINAPI WorkerThread(LPVOID);

int main()
{

	WSADATA                 wsaData;
	SOCKET                  sListen, sClient;
	SOCKADDR_IN             local, client;
	DWORD                   i, dwThreadId;
	int                     iaddrSize = sizeof(SOCKADDR_IN);
	HANDLE                  CompletionPort = INVALID_HANDLE_VALUE;
	SYSTEM_INFO             systeminfo;
	LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

	// Initialize Windows Socket library
	WSAStartup(0x0202, &wsaData);

	// Create completion port
	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Create worker thread
	GetSystemInfo(&systeminfo);

	for (i = 0; i < systeminfo.dwNumberOfProcessors; i++)
	{
		CreateThread(NULL, 0, WorkerThread, CompletionPort, 0, &dwThreadId);
	}

	// Create listening socket
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Bind
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT);
	bind(sListen, (struct sockaddr *)&local, sizeof(SOCKADDR_IN));

	// Listen
	listen(sListen, 3);

	while (TRUE)
	{

		// Accept a connection
		sClient = accept(sListen, (struct sockaddr *)&client, &iaddrSize);
		printf("Accepted client:%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		// Associate the newly arrived client socket with completion port
		CreateIoCompletionPort((HANDLE)sClient, CompletionPort, (DWORD)sClient, 0);

		// Launch an asynchronous operation for new arrived connection
		lpPerIOData = (LPPER_IO_OPERATION_DATA)HeapAlloc(
													GetProcessHeap(),
													HEAP_ZERO_MEMORY,
													sizeof(PER_IO_OPERATION_DATA)
												);
												lpPerIOData->Buffer.len = MSGSIZE;
												lpPerIOData->Buffer.buf = lpPerIOData->szMessage;
												lpPerIOData->OperationType = RECV_POSTED;
		WSARecv(
			sClient,
			&lpPerIOData->Buffer,
			1,
			&lpPerIOData->NumberOfBytesRecvd,
			&lpPerIOData->Flags,
			&lpPerIOData->overlap,
			NULL
		);

	}


	PostQueuedCompletionStatus(CompletionPort, 0xFFFFFFFF, 0, NULL);
	CloseHandle(CompletionPort);
	closesocket(sListen);
	WSACleanup();

	return 0;

}

 

DWORD WINAPI WorkerThread(LPVOID CompletionPortID)
{

	HANDLE                  CompletionPort=(HANDLE)CompletionPortID;
	DWORD                   dwBytesTransferred;
	SOCKET                  sClient;
	LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

	while (TRUE)
	{

		GetQueuedCompletionStatus(
			CompletionPort,
			&dwBytesTransferred,
			 (ULONG_PTR *)&sClient,
			(LPOVERLAPPED *)&lpPerIOData,
			INFINITE
		);

		if (dwBytesTransferred == 0xFFFFFFFF)
		{
			return 0;
		}

		if (lpPerIOData->OperationType == RECV_POSTED)
		{
			if (dwBytesTransferred == 0)
			{
				// Connection was closed by client
				printf("client close\r\n");
				closesocket(sClient);
				HeapFree(GetProcessHeap(), 0, lpPerIOData);
			}
			else
			{
				printf("%s\r\n",lpPerIOData->szMessage);

			lpPerIOData->szMessage[dwBytesTransferred] = '\0';
			send(sClient, lpPerIOData->szMessage, dwBytesTransferred, 0);

			// Launch another asynchronous operation for sClient
			memset(lpPerIOData, 0, sizeof(PER_IO_OPERATION_DATA));

			lpPerIOData->Buffer.len = MSGSIZE;
			lpPerIOData->Buffer.buf = lpPerIOData->szMessage;
			lpPerIOData->OperationType = RECV_POSTED;

			WSARecv(
				sClient,
				&lpPerIOData->Buffer,
				1,
				&lpPerIOData->NumberOfBytesRecvd,
				&lpPerIOData->Flags,
				&lpPerIOData->overlap,
				NULL
			);

			}

		}

	}

	return 0;

}

 

 