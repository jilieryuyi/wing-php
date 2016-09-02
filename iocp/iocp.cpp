/************************************************************************/
/* @ iocp project                                                       */
/************************************************************************/

#include "stdafx.h"
#include "Winsock2.h"
#include "Windows.h"
#include "Winbase.h"
#include "tlhelp32.h"
#include "tchar.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Shlwapi.h"
#include "mstcpip.h"
#include <mswsock.h>
#include "ws2tcpip.h"
#include "time.h"

#pragma comment( lib, "Kernel32.lib" )
#pragma comment( lib, "Shlwapi.lib" )
#pragma comment( lib, "Psapi.lib" )
#pragma comment( lib, "Winmm.lib" )
#pragma comment( lib, "Ws2_32.lib" )

#define DATA_BUFSIZE   10240
#define OP_ACCEPT   1
#define OP_RECV     2
#define OP_SEND     3
#define OP_DIS      4
#define OP_ONACCEPT 5

//iocp struct
struct iocp_overlapped{
	OVERLAPPED	m_ol;                          // 
	int			m_iOpType;                     //do type
	SOCKET		m_skServer;                    //server socket
	SOCKET		m_skClient;                    //client
	DWORD		m_recvBytes;                   //recv msg bytes
	char		m_pBuf[DATA_BUFSIZE];          //recv buf
	WSABUF		m_DataBuf;                     //recv data buf
	int			m_recv_timeout;                //recv timeout
	int         m_send_timeout;
	SOCKADDR_IN m_addrClient;                  //client address
	SOCKADDR_IN m_addrServer;                  //server address
	int			m_isUsed;                      //client is active 1 yes 0 not
	time_t      m_active;                      //the last active time
	int         m_isCrashed;                   //is crashed? 0 not 1 yes
	int         m_online;                      //is online 1 yes 0 not
	int         m_usenum;                      //

	//void (*handler)(int,struct tag_socket_data*);   data->handler(res, data);  
};




static SOCKET  m_sock_listen = INVALID_SOCKET;    //the server listen socket

class WingIOCP{
private:
	char* m_listen_ip;        //listen ip
	int   m_port;             //listen port
	int   m_max_connect;      //max connection
	int   m_recv_timeout;     //recv timeout
	int   m_send_timeout;     //send timeout
	
	unsigned long* m_povs;  //clients 

	//iocp worker
	static VOID CALLBACK worker( 
		DWORD dwErrorCode,
		DWORD dwBytesTrans,
		LPOVERLAPPED lpOverlapped 
		);
	//accept ex
	static BOOL accept(
			SOCKET sAcceptSocket,
			PVOID lpOutputBuffer,
			DWORD dwReceiveDataLength,
			DWORD dwLocalAddressLength,
			DWORD dwRemoteAddressLength,
			LPDWORD lpdwBytesReceived,
			LPOVERLAPPED lpOverlapped
		);
	//disconnect a client socket and reuse it
	static BOOL disconnect( SOCKET client_socket , LPOVERLAPPED lpOverlapped , DWORD dwFlags = TF_REUSE_SOCKET , DWORD reserved = 0);
	
	//event callbacks
	static void onconnect( iocp_overlapped *&povl );
	static void ondisconnect( iocp_overlapped *&povl );
	static void onclose( iocp_overlapped *&povl );
	static void onrecv( iocp_overlapped *&povl );
	static void onsend( iocp_overlapped *&povl );
	static void onrun( iocp_overlapped *&povl, DWORD errorcode, int last_error );

	static void onaccept(iocp_overlapped *&pOL);

public:
	
	WingIOCP(
		const char* listen       = "0.0.0.0",
		const int   port         = 6998,  
		const int   max_connect  = 10,
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

	this->m_listen_ip      = _strdup(listen);               //listen ip
	this->m_port           = port;                          //listen port
	this->m_max_connect    = max_connect;                   //max connect
	this->m_recv_timeout   = recv_timeout;                  //recv timeout
	this->m_send_timeout   = send_timeout;                  //send timeout                               
	this->m_povs           = new unsigned long[max_connect];//clients 

}

/**
 * @ destruct
 */
WingIOCP::~WingIOCP(){
	
	if( this->m_listen_ip ) 
	{	
		free(this->m_listen_ip );
		this->m_listen_ip = NULL;
	}

	if( this->m_povs )
	{
		delete[] this->m_povs;
		this->m_povs = NULL;
	}

	if( m_sock_listen != INVALID_SOCKET )
	{
		closesocket( m_sock_listen );
		m_sock_listen = INVALID_SOCKET;
	}

	WSACleanup();
}

/**
 *@wait
 */
void WingIOCP::wait(){
	while( true ){
		Sleep(10);
	}
}

//event callbacks
 void WingIOCP::onconnect( iocp_overlapped *&pOL ){
	 printf("%ld onconnect\r\n",pOL->m_skClient);
	 pOL->m_online = 1;
	 pOL->m_active = time(NULL);

	 if( setsockopt( pOL->m_skClient, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(const char *)&pOL->m_skServer,sizeof(pOL->m_skServer) ) != 0 )
	 {
		 //setsockopt fail
		 //printf("1=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
		 WSASetLastError(0);
		 return;
	 }

	 // set send timeout
	 if( pOL->m_send_timeout > 0 )
	 {
		 if( setsockopt( pOL->m_skClient, SOL_SOCKET,SO_SNDTIMEO, (const char*)&pOL->m_send_timeout,sizeof(pOL->m_send_timeout)) !=0 )
		 {
			 //setsockopt fail
			// printf("2=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
		 }
	 }
	 if( pOL->m_recv_timeout > 0 )
	 {
		 if( setsockopt( pOL->m_skClient, SOL_SOCKET,SO_RCVTIMEO, (const char*)&pOL->m_recv_timeout,sizeof(pOL->m_recv_timeout)) != 0 )
		 {
			 //setsockopt fail
			// printf("3=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
		 }
	 }

	 linger so_linger;
	 so_linger.l_onoff	= TRUE;
	 so_linger.l_linger	= 0; // without close wait status
	 if( setsockopt( pOL->m_skClient,SOL_SOCKET,SO_LINGER,(const char*)&so_linger,sizeof(so_linger) ) != 0 ){
		// printf("31=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
	 } 

	 //get client ip and port
	 int client_size = sizeof(pOL->m_addrClient);  
	 ZeroMemory( &pOL->m_addrClient , sizeof(pOL->m_addrClient) );

	 if( getpeername( pOL->m_skClient , (SOCKADDR *)&pOL->m_addrClient , &client_size ) != 0 ) 
	 {
		 //getpeername fail
		// printf("4=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
	 }

	// printf("%s %d connect\r\n",inet_ntoa(pOL->m_addrClient.sin_addr), pOL->m_addrClient.sin_port);

	 //keepalive open
	 int dt		= 1;
	 DWORD dw	= 0;
	 tcp_keepalive live ;     
	 live.keepaliveinterval	= 5000;     //连接之后 多长时间发现无活动 开始发送心跳吧 单位为毫秒 
	 live.keepalivetime		= 1000;     //多长时间发送一次心跳包 1分钟是 60000 以此类推     
	 live.onoff				= TRUE;     //是否开启 keepalive

	 if( setsockopt( pOL->m_skClient, SOL_SOCKET, SO_KEEPALIVE, (char *)&dt, sizeof(dt) ) != 0 )
	 {
		 //setsockopt fail
		// printf("5=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
	 }           

	 if( WSAIoctl(   pOL->m_skClient, SIO_KEEPALIVE_VALS, &live, sizeof(live), NULL, 0, &dw, &pOL->m_ol , NULL ) != 0 )
	 {
		 //WSAIoctl error
		// printf("6=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
	 }

	 memset(pOL->m_pBuf,0,DATA_BUFSIZE);
	 //post recv
	 pOL->m_DataBuf.buf	= pOL->m_pBuf;  
	 pOL->m_DataBuf.len	= DATA_BUFSIZE;  
	 pOL->m_iOpType		= OP_RECV;

	 DWORD RecvBytes		= 0;
	 DWORD Flags			= 0;

	 int code		    = WSARecv(pOL->m_skClient,&(pOL->m_DataBuf),1,&RecvBytes,&Flags,&(pOL->m_ol),NULL);
	 int error_code	    = WSAGetLastError();

	 if( 0 != code )
	 {
		 if( WSA_IO_PENDING != error_code ) 
		 {
			// printf("7=>onconnect some error happened , error code %d \r\n", WSAGetLastError());
			 return;
		 }
	 }
	 else
	 {
		 //recv complete
		 onrecv( pOL );
	 }
 }
 void WingIOCP::ondisconnect( iocp_overlapped *&pOL ){
	// printf("ondisconnect error %d\r\n",WSAGetLastError());
	 WSASetLastError(0);
	 pOL->m_online   = 0;                                //set offline
	 pOL->m_active   = time(NULL);                       //the last active time
	 pOL->m_iOpType	 = OP_ONACCEPT;                      //reset status
	 pOL->m_isUsed   = 0;                                //
	 ZeroMemory(pOL->m_pBuf,sizeof(char)*DATA_BUFSIZE);  //clear buf

	
	
	 if( !BindIoCompletionCallback( (HANDLE)pOL->m_skClient ,worker,0) ){
		// printf("BindIoCompletionCallback error %ld\r\n",WSAGetLastError());
	 }
	 //post acceptex
	 int error_code     = accept( pOL->m_skClient,pOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pOL );
	 //printf("accept error %d\r\n",WSAGetLastError());
	 int last_error     = WSAGetLastError() ;

	 if( !error_code && ERROR_IO_PENDING != last_error ){

	 }
	 //printf("2=>ondisconnect some error happened , error code %d \r\n================================================\r\n\r\n", WSAGetLastError());
	  
	  //printf("21=>ondisconnect some error happened , error code %d \r\n================================================\r\n\r\n", WSAGetLastError());

	 WSASetLastError(0);
 }

 void WingIOCP::onaccept(iocp_overlapped *&pOL){
	 pOL->m_active   = time(NULL);                       //the last active time
	 pOL->m_iOpType	 = OP_ACCEPT;                        //reset status
	 printf("%ld reuse socket real complete , error code %d \r\n", pOL->m_skClient,WSAGetLastError());

	 WSASetLastError(0);
 }
 void WingIOCP::onclose( iocp_overlapped *&pOL ){
	// printf("%ld close\r\n", pOL->m_skClient);

	 SOCKET m_sockListen = pOL->m_skServer;
	 SOCKET m_client     = pOL->m_skClient;
	 int send_timeout    = pOL->m_send_timeout;
	 int recv_timeout    = pOL->m_recv_timeout;
	 pOL->m_iOpType = OP_DIS;

	 //socket reuse
	 if( !disconnect( pOL->m_skClient , &pOL->m_ol ) && WSA_IO_PENDING != WSAGetLastError()) {
		// printf("1=>onclose some error happened , error code %d \r\n", WSAGetLastError());
	 }
	 //printf("onclose complete %d \r\n", WSAGetLastError());
 }
 void WingIOCP::onrecv( iocp_overlapped *&pOL ){
	 pOL->m_active = time(NULL);
	// printf("recv:\r\n%s\r\n\r\n",pOL->m_pBuf);
	 ZeroMemory(pOL->m_pBuf,DATA_BUFSIZE);      
 }
 void WingIOCP::onsend( iocp_overlapped *&povl ){

 }
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
	WSASetLastError(0);
	if( m_sock_listen == INVALID_SOCKET || !lpOverlapped ) 
	{	
		return 0;
	}
	GUID guidAcceptEx	= WSAID_ACCEPTEX;
	DWORD dwBytes		= 0;
	LPFN_ACCEPTEX lpfnAcceptEx;

	int res= WSAIoctl( m_sock_listen, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, 
		sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL );

	if( 0 != res )
	{
		return 0;
	}

	return lpfnAcceptEx( m_sock_listen, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength,
		dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped );        
}

/**
 * @ disconnect socket and reuse the socket
 */
BOOL WingIOCP::disconnect( SOCKET client_socket , LPOVERLAPPED lpOverlapped , DWORD dwFlags  , DWORD reserved  )
{
	WSASetLastError(0);
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
	//why here get the error code 87 ?
	//printf("worker error %d\r\n",WSAGetLastError());
	if( NULL == lpOverlapped  )
	{
		//not real complete
		SleepEx(20,TRUE);//set warn status
		WSASetLastError(0);
		return;
	}

	//get overlapped data
	iocp_overlapped*  pOL = CONTAINING_RECORD(lpOverlapped, iocp_overlapped, m_ol);
	
	//just a test
	onrun( pOL, dwErrorCode, WSAGetLastError() );
	
	switch( pOL->m_iOpType )
	{
	case OP_DIS:
		ondisconnect(pOL);
		break;
	case OP_ONACCEPT:
		onaccept(pOL);
		break;
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

	WSASetLastError(0);

}
BOOL WingIOCP::start(){	

	do{ 

		WSADATA wsaData; 
		if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
		{
			return FALSE;
		}
 
		if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			break;
		}  

		m_sock_listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
		if( INVALID_SOCKET == m_sock_listen )
		{
			break;
		}

		//bind the worker thread
		BOOL bReuse      = TRUE;
		BOOL bind_status = ::BindIoCompletionCallback((HANDLE)( m_sock_listen ), worker, 0 );
		if( !bind_status )
		{
			break;
		}

		//set option SO_REUSEADDR 
		if( 0 != ::setsockopt( m_sock_listen, SOL_SOCKET, SO_REUSEADDR,(LPCSTR)&bReuse, sizeof(BOOL) ) )
		{
			//some error happened
			break;
		}


		struct sockaddr_in ServerAddress; 
		ZeroMemory(&ServerAddress, sizeof(ServerAddress)); 

		ServerAddress.sin_family		= AF_INET;                    
		ServerAddress.sin_addr.s_addr	= inet_addr( this->m_listen_ip );          
		ServerAddress.sin_port			= htons( this->m_port );   

		if ( SOCKET_ERROR == bind( m_sock_listen, (struct sockaddr *) &ServerAddress, sizeof( ServerAddress ) ) )
		{
			break;
		}  

		if( 0 != listen( m_sock_listen , SOMAXCONN ) )
		{
			break;
		}
		//printf("1=>start get error %d\r\n",WSAGetLastError());
		WSASetLastError(0);
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
			povl->m_skServer			= m_sock_listen;
			povl->m_skClient			= client;
			povl->m_recv_timeout		= m_recv_timeout;
			povl->m_isUsed			= 0;
			povl->m_active			= 0; 
			povl->m_isCrashed		= 0;
			povl->m_online			= 0;
			povl->m_usenum			= 1;

			int server_size = sizeof(povl->m_addrServer);  
			ZeroMemory(&povl->m_addrServer,server_size);
			getpeername(povl->m_skServer,(SOCKADDR *)&povl->m_addrServer,&server_size);  

			int error_code = accept( povl->m_skClient, povl->m_pBuf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, NULL, (LPOVERLAPPED)povl );
			int last_error = WSAGetLastError() ;
			if( !error_code && ERROR_IO_PENDING != last_error ) 
			{
			
				closesocket( client );
				client = povl->m_skClient = INVALID_SOCKET;
				delete povl;
				povl = NULL; 
				//printf("client=>crate error %d\r\n",WSAGetLastError());
			}else{
				this->m_povs[i] = (unsigned long)povl;
			}
			//here all the last error is 997 , means nothing error happened
			//printf("client=>start get error %d\r\n",WSAGetLastError());
			WSASetLastError(0);
		}
		//printf("last start get error %d\r\n",WSAGetLastError());
		WSASetLastError(0);
		return TRUE;

	} while( 0 );

	if( m_sock_listen != INVALID_SOCKET )
	{
		closesocket( m_sock_listen );
		m_sock_listen = INVALID_SOCKET;
	}
	WSACleanup();

	return FALSE;
}




int _tmain(int argc, _TCHAR* argv[])
{
	WingIOCP *iocp  = new WingIOCP();
	iocp->start();
	iocp->wait();
	delete iocp;
	return 0;
}

