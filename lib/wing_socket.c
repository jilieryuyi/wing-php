#include "wing_socket.h"
#include "wing_msg_queue.h"

//HANDLE m_hIOCompletionPort;//完成端口
SOCKET m_sockListen;//服务socket
//post 队列消息
void wing_post_queue_msg(int message_id,unsigned long wparam,unsigned long lparam,unsigned long eparam){
		
	wing_msg_queue_element *msg	= new wing_msg_queue_element();  

	if( NULL == msg ) 
		wing_socket_throw_error(WING_BAD_ERROR);

	msg->message_id = message_id;
	msg->wparam		= wparam;
	msg->lparam		= lparam;	
	msg->size		= 0;
	msg->eparam		= eparam;

	wing_msg_queue_lpush(msg);	
}

//加载wsa ex系列函数 这里并没有用到这个函数 这个函数每次都会创建一个临时的socket 好浪费资源的 有木有
BOOL WingLoadWSAFunc(GUID &funGuid,void* &pFun)
{
	//本函数利用参数返回函数指针
	DWORD dwBytes = 0;
	pFun = NULL;

	//随便创建一个SOCKET供WSAIoctl使用 并不一定要像下面这样创建
	SOCKET skTemp = ::WSASocket(AF_INET,SOCK_STREAM, IPPROTO_TCP, NULL,0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == skTemp)
	{
		//通常表示没有正常的初始化WinSock环境
		return FALSE;
	}
    ::WSAIoctl(skTemp, SIO_GET_EXTENSION_FUNCTION_POINTER, &funGuid,sizeof(funGuid),&pFun,sizeof(pFun), &dwBytes, NULL,NULL);
    ::closesocket(skTemp);

    return NULL != pFun;
}

//断开socket连接 支持socke复用
BOOL WingDisconnectEx(SOCKET hSocket,LPOVERLAPPED lpOverlapped,DWORD dwFlags,DWORD reserved)
{
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD dwBytes = 0;
	LPFN_DISCONNECTEX lpfnDisconnectEx;
	
	if( 0 != WSAIoctl(hSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,sizeof(GuidDisconnectEx),&lpfnDisconnectEx,sizeof(lpfnDisconnectEx),&dwBytes,NULL,NULL)){
			return 0;
	}
	
	return lpfnDisconnectEx(hSocket,lpOverlapped,/*TF_REUSE_SOCKET*/dwFlags,reserved);
}

//接受新的socket连接
BOOL WingAcceptEx(SOCKET sListenSocket,SOCKET sAcceptSocket,PVOID lpOutputBuffer,DWORD dwReceiveDataLength,DWORD dwLocalAddressLength,DWORD dwRemoteAddressLength,LPDWORD lpdwBytesReceived,LPOVERLAPPED lpOverlapped)
{
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;     //AcceptEx函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(sListenSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&guidAcceptEx,sizeof(guidAcceptEx),&lpfnAcceptEx,sizeof(lpfnAcceptEx),&dwBytes,NULL,NULL)){
		return 0;
	}
	return lpfnAcceptEx( sListenSocket,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped);        
}

//获取acceptex连接进来的socket ip地址和端口 不过在实际应用中 不给力
void WingGetAcceptExSockaddrs(SOCKET sListenSocket,PVOID lpOutputBuffer,DWORD dwReceiveDataLength,DWORD dwLocalAddressLength,DWORD dwRemoteAddressLength,LPSOCKADDR *LocalSockaddr,LPINT LocalSockaddrLength,LPSOCKADDR *RemoteSockaddr,LPINT RemoteSockaddrLength)
{
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(sListenSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&guidGetAcceptExSockaddrs,sizeof(guidGetAcceptExSockaddrs),&lpfnGetAcceptExSockaddrs,sizeof(lpfnGetAcceptExSockaddrs),&dwBytes,NULL,NULL)){
		return;
	}
	return lpfnGetAcceptExSockaddrs( lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength, dwRemoteAddressLength, LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength);   
}
//严重错误 直接exit
void wing_socket_throw_error( int error_code ){
	exit(WING_BAD_ERROR);
}

/****************************************************
 * @ 投递一次 recv 在acceptex 之后 只需要调用一次
 ****************************************************/
bool wing_socket_post_recv(MYOVERLAPPED* &pMyOL){
	
	pMyOL->m_DataBuf.buf	= pMyOL->m_pBuf;  
	pMyOL->m_DataBuf.len	= DATA_BUFSIZE;  
	pMyOL->m_iOpType		= OPE_RECV;

	DWORD RecvBytes=0;
	DWORD Flags=0;

	int code = WSARecv(pMyOL->m_skClient,&(pMyOL->m_DataBuf),1,&RecvBytes,&Flags,&(pMyOL->m_ol),NULL);
	int error_code =  WSAGetLastError();
	//调用一次 WSARecv 触发iocp事件
	if(0 != code && WSA_IO_PENDING != error_code){
		return false;
	}
	return true;
}
/****************************************************
 * @ accept回调 新的客户端链接进来了
 * @ 回调之后需要投递一次新的 accept 以待下次新客户端连接
 * @ 还需要头第一次recv 用来接收数据
 ****************************************************/
void wing_socket_on_accept(MYOVERLAPPED* &pMyOL){

	int lenHost		= sizeof(sockaddr_in) + 16;
	int lenClient	= sizeof(sockaddr_in) + 16;

	pMyOL->m_isUsed = WING_SOCKET_IS_ALIVE;
	
	int nRet = ::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(const char *)&pMyOL->m_skServer,sizeof(pMyOL->m_skServer));

	
	// 设置超时时间 某些短连接协议 就很有用 貌似没效果
	if( pMyOL->m_timeout > 0 ){
		::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_SNDTIMEO, (const char*)&pMyOL->m_timeout,sizeof(pMyOL->m_timeout));
		::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_RCVTIMEO, (const char*)&pMyOL->m_timeout,sizeof(pMyOL->m_timeout));
	}

	linger so_linger;
	so_linger.l_onoff = TRUE;
	so_linger.l_linger = 0; //强制closesocket后 设置允许3秒逗留时间 防止数据丢失
	setsockopt(pMyOL->m_skClient,SOL_SOCKET,SO_LINGER,(const char*)&so_linger,sizeof(so_linger));
	
	int client_size = sizeof(pMyOL->m_addrClient);  
	ZeroMemory(&pMyOL->m_addrClient,sizeof(pMyOL->m_addrClient));
	getpeername(pMyOL->m_skClient,(SOCKADDR *)&pMyOL->m_addrClient,&client_size);  

	wing_post_queue_msg(WM_ONCONNECT, pMyOL->m_skClient,0);
	wing_socket_post_recv(pMyOL);
}
/****************************************************
 * @ 发送消息
 ****************************************************/
void wing_socket_on_send( MYOVERLAPPED*  pOL){
	/*

			//CloseHandle(PerIOData->OVerlapped.hEvent);
			GlobalFree( perIoData ); 

			perIoData = NULL;
		

			memory_sub();

			wing_post_queue_msg(WM_ONSEND,perHandleData->Socket);*/
}

void wing_socket_on_recv(MYOVERLAPPED*  &pOL){
	//构建消息
	RECV_MSG *recv_msg	= new RECV_MSG();   
	ZeroMemory(recv_msg,sizeof(RECV_MSG));

	DWORD len = pOL->m_recvBytes+1;
	recv_msg->msg = new char[len];
	ZeroMemory(recv_msg->msg,len);

	strcpy_s(recv_msg->msg,len,pOL->m_pBuf);
	recv_msg->len		=  len;
	
	//发送消息
	wing_post_queue_msg(WM_ONRECV,pOL->m_skClient,(unsigned long)recv_msg);
}

void wing_socket_on_close(MYOVERLAPPED*  &pMyOL){
	
	wing_post_queue_msg(WM_ONCLOSE,pMyOL->m_skClient);

	WingDisconnectEx(pMyOL->m_skClient,NULL,TF_REUSE_SOCKET,0);

	pMyOL->m_iOpType	= OPE_ACCEPT;        //AcceptEx操作
	pMyOL->m_isUsed     = WING_SOCKET_IS_SLEEP;
	ZeroMemory(pMyOL->m_pBuf,sizeof(char)*DATA_BUFSIZE);

	
	int error_code = WingAcceptEx(pMyOL->m_skServer,pMyOL->m_skClient,pMyOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pMyOL);
	int last_error = WSAGetLastError() ;
	unsigned long w_client = pMyOL->m_skClient;
	
	//这里经常会返回 10022错误 得找找解决方案
	if( !error_code && WSAECONNRESET != last_error && ERROR_IO_PENDING != last_error ){
		
		//如果socket发生了错误 并且是可用的socket
		if( INVALID_SOCKET != pMyOL->m_skClient ) 
		{
			//关掉这个错误的socket
			closesocket(pMyOL->m_skClient);
			
			//创建一个新的socket
			pMyOL->m_iOpType	= OPE_ACCEPT;        //AcceptEx操作
			ZeroMemory(pMyOL->m_pBuf,sizeof(char)*DATA_BUFSIZE);
			SOCKET new_client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
				
			//如果创建的新的socket可用
			if( INVALID_SOCKET != new_client ) 
			{	
				//如果绑定完成端口错误
				if( !BindIoCompletionCallback((HANDLE)new_client ,wing_icop_thread,0) )
				{
					//将老的socket 从关系映射中移除
					wing_remove_from_sockets_map((unsigned long)w_client);
					//关闭刚才创建的新的socket
					closesocket(new_client);
					new_client = INVALID_SOCKET;
					//清理资源
					delete pMyOL;
					pMyOL = NULL; 
					return;
				}
				else
				{
					//如果绑定iocp成功		
					pMyOL->m_skClient = new_client;

					int error_code = WingAcceptEx(pMyOL->m_skServer,pMyOL->m_skClient,pMyOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pMyOL);
					int last_error = WSAGetLastError() ;
					
					//如果acceptex错误 啊！好复杂有木有 感谢掉进了死循环
					if( !error_code && WSAECONNRESET != last_error && ERROR_IO_PENDING != last_error ){
						
						//将老的socket从关系映射移除
						wing_remove_from_sockets_map((unsigned long)w_client);
						//关闭掉新的socket
						closesocket(new_client);
						new_client = pMyOL->m_skClient = INVALID_SOCKET;
						//清理资源
						delete pMyOL;
						pMyOL = NULL; 
						return;

					}else{
						//如果没错误 添加到map映射 后面需要使用
						wing_add_to_sockets_map((unsigned long)new_client,(unsigned long)pMyOL);
						wing_remove_from_sockets_map((unsigned long)w_client);
					}
				}
			}
			else
			{
				wing_remove_from_sockets_map((unsigned long)w_client);
				delete pMyOL;
				pMyOL = NULL; 
				return;		
			}
		}	

		////如果发生错误应该要有增补方案 否则到最后可用socket为0了就不好了
		//触发onerror 会新建一个 socket 然后重用 pMyOL 所以 pMyOL不要删除
		wing_post_queue_msg(WM_ONERROR,WING_ERROR_ACCEPT,last_error,w_client);
		return;
	}

	//注意在这个SOCKET被重新利用后，后面的再次捆绑到完成端口的操作会返回一个已设置//的错误，这个错误直接被忽略即可
	::BindIoCompletionCallback((HANDLE)pMyOL->m_skClient,wing_icop_thread, 0);

}

//iocp工作线程
VOID CALLBACK wing_icop_thread(DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped)
{
	//IOCP回调函数
	if( NULL == lpOverlapped )
	{
		//没有真正的完成
		SleepEx(20,TRUE);//故意置成可警告状态
		return;
	}

	int error_code = WSAGetLastError();
	//这里还原ovl
	MYOVERLAPPED*  pOL = CONTAINING_RECORD(lpOverlapped, MYOVERLAPPED, m_ol);

	if( 0 != dwErrorCode ) {
		//这里用来判断客户端掉线的
		if( 0 == dwBytesTrans || 10054 == error_code || 64 == error_code){
			wing_socket_on_close(pOL);
		}
	}
	pOL->m_recvBytes = dwBytesTrans;
	switch(pOL->m_iOpType)
	{
		case OPE_ACCEPT: //AcceptEx结束
		{
			//有链接进来了 SOCKET句柄就是 pMyOL->m_skClient
			wing_socket_on_accept(pOL);
		}
		break;
		case OPE_RECV:
		{
			//收到信的消息 需要判断一下是否掉线
			if( 0 == dwBytesTrans || 10054 == error_code || 64 == error_code){
				wing_socket_on_close(pOL);
			} else {
				wing_socket_on_recv(pOL);
			}	
		}
		break;
		case OPE_SEND:
		{
			//异步发送 这里还没实现
			wing_socket_on_send(pOL);
		}
		break;

	}

}

//socket服务初始化
SOCKET wing_socket_init(const char *listen_ip,const int port,const int max_connect,const int timeout){
	
	//初始化Socket
	WSADATA wsaData; 
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{
		return INVALID_SOCKET; 
	}

	// 检查是否申请了所需版本的套接字库   
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
        WSACleanup();  
        return INVALID_SOCKET;  
    }  

	
	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if( INVALID_SOCKET == m_sockListen )
	{
		WSACleanup();
		return INVALID_SOCKET;
	}

	//设置 SO_REUSEADDR 
	BOOL bReuse = TRUE;
	BOOL bind_status = ::BindIoCompletionCallback((HANDLE)m_sockListen,wing_icop_thread, 0);
	if( !bind_status )
	{
		closesocket(m_sockListen);
		WSACleanup();
		return INVALID_SOCKET;
	}

	if( 0 != ::setsockopt(m_sockListen,SOL_SOCKET,SO_REUSEADDR,(LPCSTR)&bReuse,sizeof(BOOL)) )
	{
		//设置错误 这里先不处理 因为是可选的设置
	}

	struct sockaddr_in ServerAddress; 
	// 填充地址结构信息
	ZeroMemory(&ServerAddress, sizeof(ServerAddress)); 
	ServerAddress.sin_family = AF_INET; 
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                     
	ServerAddress.sin_addr.s_addr = inet_addr(listen_ip);          
	ServerAddress.sin_port = htons(port);   


	// 绑定端口
	if ( SOCKET_ERROR == bind( m_sockListen, (struct sockaddr *) &ServerAddress, sizeof( ServerAddress ) ) )
	{
		closesocket(m_sockListen);
		WSACleanup();
		return INVALID_SOCKET;
	}  

	// 开始监听
	if( 0 != listen( m_sockListen , SOMAXCONN ) )
	{
		closesocket(m_sockListen);
		WSACleanup();
		return INVALID_SOCKET;
	}

	//socket 池
	for( int i = 0 ; i < max_connect ; i++ ) 
	{
	
		SOCKET client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
		
		if( INVALID_SOCKET == client ) 
		{	
			continue;
		}

		if( !BindIoCompletionCallback((HANDLE)client ,wing_icop_thread,0) )
		{
			closesocket(client);
			continue;
		}

		MYOVERLAPPED *pMyOL = new MYOVERLAPPED();
		if( NULL == pMyOL )
		{
			closesocket(client);
			continue;
		}

		DWORD dwBytes = 0;
		ZeroMemory(pMyOL,sizeof(MYOVERLAPPED));
		
		pMyOL->m_iOpType	= OPE_ACCEPT;
		pMyOL->m_skServer	= m_sockListen;
		pMyOL->m_skClient	= client;
		pMyOL->m_timeout	= timeout;
		pMyOL->m_isUsed     = WING_SOCKET_IS_SLEEP;

		int server_size = sizeof(pMyOL->m_addrServer);  
		ZeroMemory(&pMyOL->m_addrServer,server_size);
		getpeername(pMyOL->m_skServer,(SOCKADDR *)&pMyOL->m_addrServer,&server_size);  


		int error_code = WingAcceptEx( m_sockListen,pMyOL->m_skClient,pMyOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pMyOL );
		int last_error = WSAGetLastError() ;
		if( !error_code && WSAECONNRESET != last_error && ERROR_IO_PENDING != last_error )
		{
			
			closesocket(client);
			client = pMyOL->m_skClient = INVALID_SOCKET;
			delete pMyOL;
			pMyOL = NULL; 

			continue;
		}
		//添加到map映射 后面需要使用
		wing_add_to_sockets_map( (unsigned long)client , (unsigned long)pMyOL );
	}

	return m_sockListen;
}

//最后的资源清理
void wing_socket_clear(){
	//CloseHandle(m_hIOCompletionPort);
	//低版本的系统可能需要使用到这个函数 这里先不做兼容了
	//PostQueuedCompletionStatus(m_hIOCompletionPort, 0xFFFFFFFF, 0, NULL);
	if( INVALID_SOCKET != m_sockListen ) 
		closesocket(m_sockListen);
	WSACleanup();
}