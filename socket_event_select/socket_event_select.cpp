
/**
 *@事件选择模型
 */
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>

 

#define PORT    5150
#define MSGSIZE 1024
#pragma comment(lib, "ws2_32.lib")

int      g_iTotalConn = 0;
SOCKET   g_CliSocketArr[MAXIMUM_WAIT_OBJECTS];
WSAEVENT g_CliEventArr[MAXIMUM_WAIT_OBJECTS];
DWORD WINAPI WorkerThread(LPVOID);
void Cleanup(int index);

 
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

		// Associate socket with network event
		g_CliSocketArr[g_iTotalConn] = sClient;//接受连接的套接口
		g_CliEventArr[g_iTotalConn] = WSACreateEvent();//返回事件对象句柄

		//在套接口上将一个或多个网络事件与 事件对象关联在一起
		WSAEventSelect( g_CliSocketArr[g_iTotalConn],//套接口
						g_CliEventArr[g_iTotalConn],//事件对象
						FD_READ | FD_CLOSE);//网络事件

		g_iTotalConn++;
	}
}

 

DWORD WINAPI WorkerThread(LPVOID lpParam)
{

	int              ret, index;
	WSANETWORKEVENTS NetworkEvents;
	char             szMessage[MSGSIZE];

	while (TRUE)
	{ 
	
		//返回导致返回的事件对象
		ret = WSAWaitForMultipleEvents( g_iTotalConn,    //数组中的句柄数目
										g_CliEventArr,   //指向一个事件对象句柄数组的指针
										FALSE,           //T，都进才回；F，一进就回
										1000,            //超时间隔
										FALSE);          //是否执行完成例程

		if (ret == WSA_WAIT_FAILED || ret == WSA_WAIT_TIMEOUT)
		{
			continue;
		}

 

		index = ret - WSA_WAIT_EVENT_0;

		//在套接口上查询与事件对象关联的网络事件
		WSAEnumNetworkEvents(g_CliSocketArr[index], g_CliEventArr[index], &NetworkEvents);

		//处理FD-READ网络事件
		if (NetworkEvents.lNetworkEvents & FD_READ)
		{

			// Receive message from client
			ret = recv(g_CliSocketArr[index], szMessage, MSGSIZE, 0);

			if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET))
			{
				Cleanup(index);
			}
			else
			{
				szMessage[ret] = '\0';
				send(g_CliSocketArr[index], szMessage, strlen(szMessage), 0);
			}

		}

		//处理FD-CLOSE网络事件
		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			Cleanup(index);
		}

	}

	return 0;

}

 

void Cleanup(int index)
{

	closesocket(g_CliSocketArr[index]);
	WSACloseEvent(g_CliEventArr[index]);

	if (index < g_iTotalConn - 1)
	{

		g_CliSocketArr[index] = g_CliSocketArr[g_iTotalConn-1];
		g_CliEventArr[index] = g_CliEventArr[g_iTotalConn- 1];

	}

	g_iTotalConn--;

}

 
