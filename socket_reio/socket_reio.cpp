
/**
 *@ÖØµþioÄ£ÐÍ
 */
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>

#define PORT    5150
#define MSGSIZE 1024
#pragma comment(lib, "ws2_32.lib")

typedef struct
{

	WSAOVERLAPPED overlap;
	WSABUF        Buffer;
	char          szMessage[MSGSIZE];
	DWORD         NumberOfBytesRecvd;
	DWORD         Flags;

}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

 

int                     g_iTotalConn = 0;
SOCKET                  g_CliSocketArr[MAXIMUM_WAIT_OBJECTS];
WSAEVENT                g_CliEventArr[MAXIMUM_WAIT_OBJECTS];
LPPER_IO_OPERATION_DATA g_pPerIODataArr[MAXIMUM_WAIT_OBJECTS];

DWORD WINAPI WorkerThread(LPVOID);
void Cleanup(int);

int main()
{

	WSADATA     wsaData;
	SOCKET      sListen, sClient;
	SOCKADDR_IN local, client;
	DWORD       dwThreadId;
	int         iaddrSize = sizeof(SOCKADDR_IN);

	// Initialize Windows Socket library
	WSAStartup(0x0202, &wsaData);

	// Create listening socket
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Bind
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT);
	bind(sListen, (struct sockaddr *)&local, sizeof(SOCKADDR_IN));

	// Listen
	listen(sListen, 3);

	// Create worker thread
	CreateThread(NULL, 0, WorkerThread, NULL, 0, &dwThreadId);

	while (TRUE)
	{

		// Accept a connection
		sClient = accept(sListen, (struct sockaddr *)&client, &iaddrSize);
		printf("Accepted client:%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		g_CliSocketArr[g_iTotalConn] = sClient;

		// Allocate a PER_IO_OPERATION_DATA structure
		g_pPerIODataArr[g_iTotalConn] = (LPPER_IO_OPERATION_DATA)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PER_IO_OPERATION_DATA) );
		g_pPerIODataArr[g_iTotalConn]->Buffer.len = MSGSIZE;
		g_pPerIODataArr[g_iTotalConn]->Buffer.buf = g_pPerIODataArr[g_iTotalConn]->szMessage;
		g_CliEventArr[g_iTotalConn] = g_pPerIODataArr[g_iTotalConn]->overlap.hEvent = WSACreateEvent();

		// Launch an asynchronous operation
		WSARecv( 
				g_CliSocketArr[g_iTotalConn], 
				&g_pPerIODataArr[g_iTotalConn]->Buffer,
				1,
				&g_pPerIODataArr[g_iTotalConn]->NumberOfBytesRecvd,
				&g_pPerIODataArr[g_iTotalConn]->Flags,
				&g_pPerIODataArr[g_iTotalConn]->overlap,
				NULL
			);

		g_iTotalConn++;

	}

 

	closesocket(sListen);
	WSACleanup();

	return 0;

}

 

DWORD WINAPI WorkerThread(LPVOID lpParam)
{

	int   ret, index;
	DWORD cbTransferred;

	while (TRUE)
	{

		ret = WSAWaitForMultipleEvents(g_iTotalConn, g_CliEventArr, FALSE, 1000, FALSE);
		if (ret == WSA_WAIT_FAILED || ret == WSA_WAIT_TIMEOUT)
		{
			continue;
		}

 

		index = ret - WSA_WAIT_EVENT_0;
		WSAResetEvent(g_CliEventArr[index]);

		WSAGetOverlappedResult(
				g_CliSocketArr[index],
				&g_pPerIODataArr[index]->overlap,
				&cbTransferred,TRUE,
				&g_pPerIODataArr[g_iTotalConn]->Flags
			);

		if (cbTransferred == 0)
		{
			// The connection was closed by client
			printf("client close\r\n");
			Cleanup(index);
		}
		else
		{

			// g_pPerIODataArr[index]->szMessage contains the received data
			g_pPerIODataArr[index]->szMessage[cbTransferred] = '\0';
			send(g_CliSocketArr[index], g_pPerIODataArr[index]->szMessage,\
			cbTransferred, 0);

			// Launch another asynchronous operation
			WSARecv(
					g_CliSocketArr[index],
					&g_pPerIODataArr[index]->Buffer,1,
					&g_pPerIODataArr[index]->NumberOfBytesRecvd,
					&g_pPerIODataArr[index]->Flags,
					&g_pPerIODataArr[index]->overlap,
					NULL
				);

		}

	}

	return 0;

}

 

void Cleanup(int index)
{

	closesocket(g_CliSocketArr[index]);
	WSACloseEvent(g_CliEventArr[index]);
	HeapFree(GetProcessHeap(), 0, g_pPerIODataArr[index]);

	if (index < g_iTotalConn - 1)
	{

		g_CliSocketArr[index] = g_CliSocketArr[g_iTotalConn - 1];
		g_CliEventArr[index] = g_CliEventArr[g_iTotalConn - 1];
		g_pPerIODataArr[index] = g_pPerIODataArr[g_iTotalConn - 1];

	}

	g_pPerIODataArr[--g_iTotalConn] = NULL;

}

 