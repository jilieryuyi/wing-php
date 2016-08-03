/**
 *@vc socket select 模型示例
 */
#include "stdafx.h"
#include <winsock.h>
#include <stdio.h>


#define PORT       5150
#define MSGSIZE    1024
#pragma comment(lib, "ws2_32.lib")


int    g_iTotalConn = 0;
SOCKET g_CliSocketArr[FD_SETSIZE];
DWORD  WINAPI WorkerThread(LPVOID lpParameter);

 
int main()

{

    WSADATA wsaData;
    SOCKET  sListen, sClient;
    SOCKADDR_IN local, client;
    int         iaddrSize = sizeof(SOCKADDR_IN);
    DWORD       dwThreadId;

    // Initialize Windows socket library
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

		// Add socket to g_CliSocketArr
		g_CliSocketArr[g_iTotalConn++] = sClient;

	}
	return 0;
}

 

DWORD WINAPI WorkerThread(LPVOID lpParam)
{

	int            i;
	fd_set         fdread;
	int            ret;
	struct timeval tv = {1, 0};
	char           *szMessage=new char[MSGSIZE];
	

	while (TRUE)
	{

		FD_ZERO(&fdread);//将fdread初始化空集

		for (i = 0; i < g_iTotalConn; i++)
		{
			FD_SET(g_CliSocketArr[i], &fdread);//将要检查的套接口加入到集合中
		}

 
		// We only care read event
		ret = select(0, &fdread, NULL, NULL, &tv);//每隔一段时间，检查可读性的套接口
		if (ret == 0)
		{
			// Time expired
			continue;
		}

		for (i = 0; i < g_iTotalConn; i++)
		{
			if ( FD_ISSET( g_CliSocketArr[i], &fdread ) )//如果可读
			{
				// A read event happened on g_CliSocketArr
				memset(szMessage,0,MSGSIZE);
				ret = recv(g_CliSocketArr[i], szMessage, MSGSIZE, 0);

				if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET))
				{

					// Client socket closed
					printf("Client socket %d closed.\n", g_CliSocketArr);

					closesocket(g_CliSocketArr[i]);

					if (i < g_iTotalConn-1)
					{
						g_CliSocketArr[i--] = g_CliSocketArr[--g_iTotalConn];
					}
				}
				else
				{
					//We received a message from client
					szMessage[ret] = '\0';
					send(g_CliSocketArr[i], szMessage, strlen(szMessage), 0);
				}

			}

		}

	}
	delete[] szMessage;

	return 0;
}

 