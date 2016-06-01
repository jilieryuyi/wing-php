/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-20
 ******************************/
#include <Winsock2.h>
#include "Windows.h"
#include "socket.h"
#define PORT 5150 

/*
unsigned int __stdcall  socket_worker(LPVOID ComlpetionPortID)  
{  
	HANDLE ComplectionPort = (HANDLE) ComlpetionPortID;  
	DWORD BytesTransferred;  
	LPOVERLAPPED Overlapped;  
	LPPER_HANDLE_DATA PerHandleData;  
	LPPER_IO_OPERATION_DATA PerIOData;  
	DWORD SendBytes,RecvBytes;  
	DWORD Flags;  

	while (TRUE)  
	{  

		if (GetQueuedCompletionStatus(ComplectionPort,&BytesTransferred,(LPDWORD)&PerHandleData,(LPOVERLAPPED*)&PerIOData,INFINITE) == 0)  
		{  
			//printf("GetQueuedCompletionStatus failed with error%d ",GetLastError());  
			return 0;
		 }  

		//首先检查套接字上是否发生错误，如果发生了则关闭套接字并且清除同套节字相关的SOCKET_INFORATION 结构体  
		if (BytesTransferred == 0) 
		{  

			  //printf("Closing Socket %d ",PerHandleData->Socket);  
			 if (closesocket(PerHandleData->Socket) == SOCKET_ERROR)  
			 {  
				//printf("closesocket failed with error %d ",WSAGetLastError());  
				return 0;  
			 }  

			GlobalFree(PerHandleData);  
			GlobalFree(PerIOData);  
			continue;  
		}  

   
		//检查BytesRecv域是否等于0，如果是，说明WSARecv调用刚刚完成，可以用从己完成的WSARecv调用返回的BytesTransferred值更新BytesRecv域  
		if (PerIOData->BytesRecv == 0)  
		{  
			PerIOData->BytesRecv = BytesTransferred;  
			PerIOData->BytesSend = 0; 
		}
		else
		{  
		  PerIOData->BytesRecv +=BytesTransferred;  
		}  


		if (PerIOData->BytesRecv > PerIOData->BytesSend)  
		{  

			   //发布另一个WSASend()请求，因为WSASendi 不能确保发送了请的所有字节，继续WSASend调用直至发送完所有收到的字节  
			  ZeroMemory(&(PerIOData->OVerlapped),sizeof(OVERLAPPED));  
			  PerIOData->DATABuf.buf = PerIOData->Buffer + PerIOData->BytesSend;  
			  PerIOData->DATABuf.len = PerIOData->BytesRecv - PerIOData->BytesSend;  
			  if (WSASend(PerHandleData->Socket,&(PerIOData->DATABuf),1,&SendBytes,0,&(PerIOData->OVerlapped),NULL) ==SOCKET_ERROR )  
			  {  
				if (WSAGetLastError() != ERROR_IO_PENDING)  
				{  
				  printf("WSASend() fialed with error %d ",WSAGetLastError());  
				  return 0;  
				}  
			  }  
		}  
		else  
		{  
			  PerIOData->BytesRecv = 0;  
			  //Now that is no more bytes to send post another WSARecv() request  
			  //现在己经发送完成  
			  Flags = 0;  
			  ZeroMemory(&(PerIOData->OVerlapped),sizeof(OVERLAPPED));  
			  PerIOData->DATABuf.buf = PerIOData->Buffer;  
			  PerIOData->DATABuf.len = DATA_BUFSIZE;  
			  if (WSARecv(PerHandleData->Socket,&(PerIOData->DATABuf),1,&RecvBytes,&Flags,&(PerIOData->OVerlapped),NULL) == SOCKET_ERROR)  
			  {  
				if (WSAGetLastError() != ERROR_IO_PENDING)  
				{  
				 // printf("WSARecv() failed with error %d ",WSAGetLastError());  
				  return 0;  
				}  
			  }  
		}  
	}  
	return 0;
}  


bool socket_service(){

	//1、创建完成端口
	HANDLE m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 ); 
	if(m_hIOCompletionPort == NULL){
		return false;
	}

	//2、根据cpu数量创建工作线程
	//获取cpu数量 用于后面创建worker数量
	SYSTEM_INFO si; 
	GetSystemInfo(&si); 
	int m_nProcessors = si.dwNumberOfProcessors; 
	// 根据CPU数量，建立*2的线程
	int m_nThreads = 2 * m_nProcessors; 
	//HANDLE* m_phWorkerThreads = new HANDLE[m_nThreads]; 
	for (int i = 0; i < m_nThreads; i++) 
	{ 
		// m_phWorkerThreads[i] = CreateThread(NULL,NULL,socket_worker,0,0,0);
		 _beginthreadex(NULL, 0, socket_worker, NULL, 0, NULL); //::CreateThread(0, 0, _WorkerThread, …); 
	} 


	// 初始化Socket库
	WSADATA wsaData; 
	if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0)
		return false; 
	
	//WSACleanup( );

	//初始化Socket
	
	// 这里需要特别注意，如果要使用重叠I/O的话，这里必须要使用WSASocket来初始化Socket
	// 注意里面有个WSA_FLAG_OVERLAPPED参数
	SOCKET m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if(m_sockListen == INVALID_SOCKET){
		WSACleanup();
		return false;
	}


	struct sockaddr_in ServerAddress; 
	// 填充地址结构信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress)); 
	ServerAddress.sin_family = AF_INET; 
	// 这里可以选择绑定任何一个可用的地址，或者是自己指定的一个IP地址
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                     
	ServerAddress.sin_addr.s_addr = inet_addr("0.0.0.0");          
	ServerAddress.sin_port = htons(6998);   


	// 绑定端口
	if (SOCKET_ERROR == bind(m_sockListen, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))){
		closesocket(m_sockListen);
		WSACleanup();
		return false;
	}  
	// 开始监听
	if( 0 != listen(m_sockListen,SOMAXCONN)){
		closesocket(m_sockListen);
		WSACleanup();
		return false;
	}


	SOCKET accept ;
	PER_HANDLE_DATA *PerHandleData;
	LPPER_IO_OPERATION_DATA PerIOData;
	DWORD SendBytes,RecvBytes;  
    DWORD Flags;  

	while(true){

		 accept = WSAAccept(m_sockListen,NULL,NULL,NULL,0);
		 if( SOCKET_ERROR == accept){
			closesocket(m_sockListen);
			WSACleanup();
			return false;
		 }
		
		 PerHandleData = (PER_HANDLE_DATA*)GlobalAlloc(GPTR,sizeof(PER_HANDLE_DATA));
		 PerHandleData->Socket = accept;

		 if( NULL == CreateIoCompletionPort ((HANDLE )accept ,m_hIOCompletionPort , (ULONG_PTR )PerHandleData ,0)){
			closesocket(accept);
			closesocket(m_sockListen);
			WSACleanup();
			return false;
		 }

		PerIOData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR,sizeof(PER_IO_OPERATION_DATA));
		if ( PerIOData == NULL )
		{
			closesocket(accept);
			closesocket(m_sockListen);
			WSACleanup();
			return false;
		} 

		ZeroMemory(&(PerIOData->OVerlapped),sizeof(OVERLAPPED)); 
		PerIOData->BytesRecv = 0;
		PerIOData->BytesSend = 0; 
		PerIOData->DATABuf.len = DATA_BUFSIZE; 
		PerIOData->DATABuf.buf = PerIOData->Buffer; 
		Flags = 0; 

		if (WSARecv(accept,&(PerIOData->DATABuf),1,&RecvBytes,&Flags,&(PerIOData->OVerlapped),NULL) == SOCKET_ERROR) 
		{ 
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{
				closesocket(accept);
				closesocket(m_sockListen);
				WSACleanup();
				return false; 
			} 
		}
	}
	return true;
}
*/