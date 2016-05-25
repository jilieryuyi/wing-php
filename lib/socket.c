/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-20
 ******************************/
#include <Winsock2.h>
#include "Windows.h"
#include "socket.h"


DWORD WINAPI socket_worker(PVOID pM)  
{  
	
	return 0;
}  

bool socket_service(){

	//1、创建完成端口
	HANDLE m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 ); 
	if(m_hIOCompletionPort==NULL){
		return false;
	}

	//2、根据cpu数量创建工作线程
	//获取cpu数量 用于后面创建worker数量
	SYSTEM_INFO si; 
	GetSystemInfo(&si); 
	int m_nProcessors = si.dwNumberOfProcessors; 
	// 根据CPU数量，建立*2的线程
	int m_nThreads = 2 * m_nProcessors; 
	HANDLE* m_phWorkerThreads = new HANDLE[m_nThreads]; 
	for (int i = 0; i < m_nThreads; i++) 
	{ 
		 m_phWorkerThreads[i] = CreateThread(NULL,NULL,socket_worker,0,0,0);//_beginthreadex(NULL, 0, socket_worker, NULL/*参数*/, 0, NULL); //::CreateThread(0, 0, _WorkerThread, …); 
	} 


	// 初始化Socket库
	WSADATA wsaData; 
	WSAStartup(MAKEWORD(2,2), &wsaData); 
	//初始化Socket
	struct sockaddr_in ServerAddress; 
	// 这里需要特别注意，如果要使用重叠I/O的话，这里必须要使用WSASocket来初始化Socket
	// 注意里面有个WSA_FLAG_OVERLAPPED参数
	SOCKET m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	// 填充地址结构信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress)); 
	ServerAddress.sin_family = AF_INET; 
	// 这里可以选择绑定任何一个可用的地址，或者是自己指定的一个IP地址
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                     
	ServerAddress.sin_addr.s_addr = inet_addr("0.0.0.0");          
	ServerAddress.sin_port = htons(6998);                           
	// 绑定端口
	if (SOCKET_ERROR == bind(m_sockListen, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))  
	// 开始监听
	listen(m_sockListen,SOMAXCONN);

	CreateIoCompletionPort ((HANDLE )m_sockListen ,m_hIOCompletionPort , (ULONG_PTR )m_sockListen ,0);
	return true;
}
