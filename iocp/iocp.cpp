/************************************************************************/
/* @ iocp project                                                       */
/************************************************************************/

#include "stdafx.h"
#include <Winsock2.h>
#include "Windows.h"
#include "Winbase.h"
#include "tlhelp32.h"
#include <tchar.h>
#include "strsafe.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"
#include "process.h"
#include <mswsock.h>
#include <ws2tcpip.h>

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#define DATA_BUFSIZE   10485760

struct SELECT_ITEM{
	SOCKET      socket;             //socket
	SOCKADDR_IN addr;               //地址端口信息
	int         active;             //最后的活动时间
	int         online;             //是否在线
	char       *recv;               //收到的消息
	int         recv_bytes;         //收到的消息长度
};


//iocp消息结构体
struct iocp_overlapped{
	OVERLAPPED	m_ol;                          //异步依赖
	int			m_iOpType;                     //操作类型
	SOCKET		m_skServer;                    //服务端socket
	SOCKET		m_skClient;                    //客户端
	DWORD		m_recvBytes;                   //接收的消息长度
	char		m_pBuf[DATA_BUFSIZE];          //接收消息的缓冲区
	WSABUF		m_DataBuf;                     //消息缓冲
	int			m_recv_timeout;                //接收超时
	int         m_send_timeout;
	SOCKADDR_IN m_addrClient;                  //客户端实际的地址
	SOCKADDR_IN m_addrServer;                  //服务端实际的地址
	int			m_isUsed;                      //标志是否已被激活使用 1已被激活 0待激活
	unsigned    m_active;                      //最后的活动时间
	//LPVOID      m_client;                    //wing_client 对象
	int         m_isCrashed;                   //是否发生错误需要回收 0非 1是
	int         m_online;                      //是否在线 1在线 0不在线
	int         m_usenum;                      //引用计数器

	//void (*handler)(int,struct tag_socket_data*);   data->handler(res, data);  请以一个接口 还可以这么用
};

//socket异步发送消息载体
struct iocp_send_node{
	SOCKET socket;
	char   *msg;
	int len;
};
typedef iocp_send_node socket_send_node;



class WingIOCP{
private:
	char* m_listen_ip;
	int   m_port;
	int   m_max_connect;
	int   m_recv_timeout;
	int   m_send_timeout;

	SOCKET  m_sock_listen;
	iocp_overlapped* m_povs;

	const static int OP_ACCEPT = 1;
	const static int OP_RECV   = 2;
	const static int OP_SEND   = 3;

	static VOID CALLBACK WingIOCP::worker( DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped );
	BOOL accept(
			SOCKET sAcceptSocket,
			PVOID lpOutputBuffer,
			DWORD dwReceiveDataLength,
			DWORD dwLocalAddressLength,
			DWORD dwRemoteAddressLength,
			LPDWORD lpdwBytesReceived,
			LPOVERLAPPED lpOverlapped
		);
	BOOL disconnect( SOCKET client_socket , LPOVERLAPPED lpOverlapped , DWORD dwFlags = TF_REUSE_SOCKET , DWORD reserved = 0);
	
	//event callbacks
	static void onconnect( iocp_overlapped *&povl );
	static void ondisconnect( iocp_overlapped *&povl );
	static void onclose( iocp_overlapped *&povl );
	static void onrecv( iocp_overlapped *&povl );
	static void onsend( iocp_overlapped *&povl );
	static void onrun( iocp_overlapped *&povl, DWORD errorcode, int last_error );

public:
	WingIOCP(
		const char* listen       = "0.0.0.0",
		const int   port         = 6998,  
		const int   max_connect  = 1000,
		const int   recv_timeout = 0,
		const int   send_timeout = 0
		);
	~WingIOCP();
	BOOL start();
	void wait();
};

/**
 * @ construct 
 */
WingIOCP::WingIOCP( 
	const char* listen,    //listen ip
	const int port,        //listen port
	const int max_connect, //max connect
	const int recv_timeout,//recv timeout in milliseconds 
	const int send_timeout //send timeout in milliseconds
)
{ 

	this->m_listen_ip      = _strdup(listen);            //listen ip
	this->m_port           = port;                      //listen port
	this->m_max_connect    = max_connect;               //max connect
	this->m_recv_timeout   = recv_timeout;              //recv timeout
	this->m_send_timeout   = send_timeout;              //send timeout
	this->m_povs           = new iocp_overlapped[max_connect];   //clients 

}
/**
 * @ destruct
 */
WingIOCP::~WingIOCP(){
	if( this->m_listen_ip ) 
		free(this->m_listen_ip );
	delete[] this->m_povs;
	closesocket( this->m_sock_listen );
	WSACleanup();
}
void WingIOCP::wait(){
	while( true ){
		Sleep(10);
	}
}
 void WingIOCP::onconnect( iocp_overlapped *&povl ){}
 void WingIOCP::ondisconnect( iocp_overlapped *&povl ){}
 void WingIOCP::onclose( iocp_overlapped *&povl ){}
 void WingIOCP::onrecv( iocp_overlapped *&povl ){}
 void WingIOCP::onsend( iocp_overlapped *&povl ){}
 void WingIOCP::onrun( iocp_overlapped *&povl, DWORD errorcode, int last_error ){}
/**
 * @ acceptex
 */
BOOL WingIOCP::accept(
	SOCKET  sAcceptSocket,
	PVOID   lpOutputBuffer,
	DWORD   dwReceiveDataLength,
	DWORD   dwLocalAddressLength,
	DWORD   dwRemoteAddressLength,
	LPDWORD lpdwBytesReceived,
	LPOVERLAPPED lpOverlapped
)
{
	if( this->m_sock_listen == INVALID_SOCKET || !lpOverlapped ) 
	{	
		return 0;
	}
	GUID guidAcceptEx	= WSAID_ACCEPTEX;
	DWORD dwBytes		= 0;
	LPFN_ACCEPTEX lpfnAcceptEx;

	if( 0 != WSAIoctl( 
		this->m_sock_listen ,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&lpfnAcceptEx,
		sizeof(lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL
		)
	)
	{
		return 0;
	}

	return lpfnAcceptEx( 
		this->m_sock_listen, 
		sAcceptSocket,
		lpOutputBuffer,
		dwReceiveDataLength,
		dwLocalAddressLength,
		dwRemoteAddressLength,
		lpdwBytesReceived,
		lpOverlapped
		);        
}

/**
 * @ disconnect socket and reuse the socket
 */
BOOL WingIOCP::disconnect( SOCKET client_socket , LPOVERLAPPED lpOverlapped , DWORD dwFlags  , DWORD reserved  )
{
	if( client_socket == INVALID_SOCKET || !lpOverlapped ) 
	{	
		return 0;
	}
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD dwBytes = 0;
	LPFN_DISCONNECTEX lpfnDisconnectEx; 

	if( 0 != WSAIoctl( client_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,
		sizeof(GuidDisconnectEx),&lpfnDisconnectEx,sizeof(lpfnDisconnectEx),&dwBytes,NULL,NULL))
	{
		return 0;
	}

	return lpfnDisconnectEx(client_socket,lpOverlapped,/*TF_REUSE_SOCKET*/dwFlags,reserved);
}


	
/**
 * @ iocp worker thread
 */
VOID CALLBACK WingIOCP::worker( DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped )
{

	if( NULL == lpOverlapped  )
	{
		//not real complete
		SleepEx(20,TRUE);//set warn status
		return;
	}

	//get overlapped data
	iocp_overlapped*  pOL = CONTAINING_RECORD(lpOverlapped, iocp_overlapped, m_ol);
	
	//just a test
	onrun( pOL, dwErrorCode, WSAGetLastError() );
	
	switch( pOL->m_iOpType )
	{
		case OP_ACCEPT: 
		{
			//new client connect
			onconnect( pOL );
		}
		break;
		case OP_RECV:
		{
			pOL->m_recvBytes = dwBytesTrans;
			//check client offline
			if( 0 == dwBytesTrans || WSAECONNRESET ==  WSAGetLastError() || ERROR_NETNAME_DELETED ==  WSAGetLastError()){
				onclose( pOL );
			} 
			else
			{   //recv msg from client
				pOL->m_recvBytes = dwBytesTrans;
				onrecv( pOL );
			}	
		}
		break;
		case OP_SEND:
			{

			}
			break;
		

	}

}
BOOL WingIOCP::start(){	

	WSADATA wsaData; 
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{
		return 0; 
	}
 
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
        WSACleanup();  
        return 0;  
    }  

	this->m_sock_listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if( INVALID_SOCKET == this->m_sock_listen )
	{
		WSACleanup();
		return 0;
	}

	//bind the worker thread
	BOOL bReuse      = TRUE;
	BOOL bind_status = ::BindIoCompletionCallback((HANDLE)(this->m_sock_listen), worker, 0 );
	if( !bind_status )
	{
		closesocket(this->m_sock_listen);
		WSACleanup();
		return 0;
	}

	//set option SO_REUSEADDR 
	if( 0 != ::setsockopt(this->m_sock_listen,SOL_SOCKET,SO_REUSEADDR,(LPCSTR)&bReuse,sizeof(BOOL)) )
	{
		//some error happened
	}


	struct sockaddr_in ServerAddress; 
	ZeroMemory(&ServerAddress, sizeof(ServerAddress)); 

	ServerAddress.sin_family		= AF_INET;                    
	ServerAddress.sin_addr.s_addr	= inet_addr( this->m_listen_ip );          
	ServerAddress.sin_port			= htons( this->m_port );   

	if ( SOCKET_ERROR == bind( this->m_sock_listen, (struct sockaddr *) &ServerAddress, sizeof( ServerAddress ) ) )
	{
		closesocket( this->m_sock_listen );
		WSACleanup();
		return 0;
	}  

	if( 0 != listen( this->m_sock_listen , SOMAXCONN ) )
	{
		closesocket( this->m_sock_listen );
		WSACleanup();
		return 0;
	}

	//socket pool
	for( int i = 0 ; i < this->m_max_connect ; i++ ) 
	{
	
		SOCKET client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
		if( INVALID_SOCKET == client ) 
		{	
			continue;
		}

		if( !BindIoCompletionCallback( (HANDLE)client ,worker,0) )
		{
			closesocket(client);
			continue;
		}

		iocp_overlapped *povl = new iocp_overlapped();
		if( NULL == povl )
		{
			closesocket(client);
			continue;
		}

		DWORD dwBytes = 0;
		ZeroMemory(povl,sizeof(iocp_overlapped));
		
		povl->m_iOpType			= OP_ACCEPT;
		povl->m_skServer		= this->m_sock_listen;
		povl->m_skClient		= client;
		povl->m_recv_timeout	= this->m_recv_timeout;
		povl->m_isUsed			= 0;
		povl->m_active			= 0; 
		povl->m_isCrashed		= 0;
		povl->m_online			= 0;
		povl->m_usenum			= 1;

		int server_size = sizeof(povl->m_addrServer);  
		ZeroMemory(&povl->m_addrServer,server_size);
		getpeername(povl->m_skServer,(SOCKADDR *)&povl->m_addrServer,&server_size);  

		int error_code = this->accept( povl->m_skClient,povl->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)povl );
		int last_error = WSAGetLastError() ;
		if( !error_code && ERROR_IO_PENDING != last_error ) 
		{
			
			closesocket( client );
			client = povl->m_skClient = INVALID_SOCKET;
			delete povl;
			povl = NULL; 

			continue;
		}else{
			this->m_povs[i] = *povl;
		}
	}
	return 1;
}




int _tmain(int argc, _TCHAR* argv[])
{
	WingIOCP *iocp=new WingIOCP();
	iocp->start();
	iocp->wait();
	return 0;
}

