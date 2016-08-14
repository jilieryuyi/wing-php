
#include "php_wing.h"

//-------wing_select_server-----------------------------------------------------------------------

#define MSGSIZE    10240                       //接收消息缓存长度
CRITICAL_SECTION select_lock;                  //新客户端连接进来加入全局变量、计数关键段锁
CRITICAL_SECTION close_socket_lock;            //客户端掉线移除全局变量、计数关键段锁
unsigned long wing_select_clients_num = 0;     //已连接的socket数量
unsigned long recv_and_send_timeout   = 0;     //recv send 超时时间
unsigned long close_socket_count      = 0;     //待关闭的socket数量
unsigned long max_connection          = 0;     //最大连接数
unsigned long  *close_socket_arr      = NULL;  //待关闭的socket
SOCKET *wing_server_clients_arr       = NULL;  //已连接的socket 65534是最大动态端口数
extern zend_class_entry *wing_sclient_ce;

//将要移除的socket节点
struct CLOSE_ITEM{
	SOCKET socket; //将要移除的socket
	int time;      //将要移除的socket加入的时间，5秒之后清理
};



/**
 *@初始化一些全局变量
 */
void wing_select_server_init( int _max_connection = 65534 ){
	InitializeCriticalSection( &select_lock );
	InitializeCriticalSection( &close_socket_lock );
	wing_server_clients_arr = new SOCKET[ _max_connection ];
	close_socket_arr        = new unsigned long[_max_connection];
	max_connection          = _max_connection;
}
/**
 *@释放全局资源
 */
void wing_select_server_clear(){
	delete[] wing_server_clients_arr;
	delete[] close_socket_arr;
	DeleteCriticalSection( &select_lock );
	DeleteCriticalSection( &close_socket_lock );
}

/**
 *@添加socket客户端到全局变量，有新客户端连接进来调用
 */
void wing_select_server_clients_append( SOCKET client ){
	if( wing_select_clients_num >= max_connection ) {
		//已达到最大连接数
		return;
	}
	EnterCriticalSection(&select_lock);
	wing_server_clients_arr[wing_select_clients_num++] = client;
	LeaveCriticalSection(&select_lock);
}
/**
 *@从全局变量中移除socket，掉线时候执行
 */
void wing_select_server_clients_remove( int i ){
	EnterCriticalSection(&select_lock);
	wing_server_clients_arr[i] = INVALID_SOCKET;
					
	for( unsigned long f=i; f < wing_select_clients_num-1; f++ )
		wing_server_clients_arr[f] = wing_server_clients_arr[f+1];

	wing_server_clients_arr[wing_select_clients_num-1] = INVALID_SOCKET;
	wing_select_clients_num -- ;
	LeaveCriticalSection(&select_lock);
}


/**
 *@创建服务端连接进来的客户端对象
 */
void select_create_wing_sclient(zval *&client , SELECT_ITEM *&item TSRMLS_DC){
	
	MAKE_STD_ZVAL(  client );
	object_init_ex( client,wing_sclient_ce);
				
	//初始化
	if( item ) {
		zend_update_property_string( wing_sclient_ce, client,"sin_addr",    strlen("sin_addr"),   inet_ntoa(item->addr.sin_addr) TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"sin_port",    strlen("sin_port"),   ntohs(item->addr.sin_port)     TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"sin_family",  strlen("sin_family"), item->addr.sin_family          TSRMLS_CC );
		zend_update_property_string( wing_sclient_ce, client,"sin_zero",    strlen("sin_zero"),   item->addr.sin_zero            TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"last_active", strlen("last_active"),item->active                   TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"socket",      strlen("socket"),     item->socket                   TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"online",      strlen("online"),     item->online                   TSRMLS_CC );
		zend_update_property_long(   wing_sclient_ce, client,"client_type", strlen("client_type"),CLIENT_SELECT                  TSRMLS_CC );
	}

}

/**
 *@select 模型新客户端连接事件
 */
void select_onconnect( SELECT_ITEM *&item){

	wing_select_server_clients_append( item->socket );

	item->active = time(NULL);
	item->online = 1;

	SOCKET socket = item->socket;

	// 设置发送、接收超时时间 单位为毫秒
	if( recv_and_send_timeout > 0 )
	{
		if( setsockopt( socket, SOL_SOCKET,SO_SNDTIMEO, (const char*)&recv_and_send_timeout,sizeof(recv_and_send_timeout)) !=0 )
		{
			//setsockopt失败
			iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
		}
		if( setsockopt( socket, SOL_SOCKET,SO_RCVTIMEO, (const char*)&recv_and_send_timeout,sizeof(recv_and_send_timeout)) != 0 )
		{
			//setsockopt失败
			iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
		}
	}
	
	linger so_linger;
	so_linger.l_onoff	= TRUE;
	so_linger.l_linger	= 0; //拒绝close wait状态
	setsockopt( socket,SOL_SOCKET,SO_LINGER,(const char*)&so_linger,sizeof(so_linger) );

	int dt		= 1;
	DWORD dw	= 0;
	tcp_keepalive live ;     
	live.keepaliveinterval	= 5000;     //连接之后 多长时间发现无活动 开始发送心跳吧 单位为毫秒 
	live.keepalivetime		= 1000;     //多长时间发送一次心跳包 1分钟是 60000 以此类推     
	live.onoff				= TRUE;     //是否开启 keepalive

	if( setsockopt( socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&dt, sizeof(dt) ) != 0 )
	{
		//setsockopt失败
		iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
	}           
	
	if( WSAIoctl(   socket, SIO_KEEPALIVE_VALS, &live, sizeof(live), NULL, 0, &dw, NULL , NULL ) != 0 )
	{
		//WSAIoctl 错误
		iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
	}

	iocp_post_queue_msg( WM_ONCONNECT, (unsigned long)item );
}

/**
 *@掉线回调
 */
void select_onclose( SELECT_ITEM *&item )
{
	if( shutdown( item->socket, SD_BOTH ) != 0 ) {
		iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
	}
		
	CLOSE_ITEM *close_item = new CLOSE_ITEM();
	close_item->socket     = item->socket;
	close_item->time       = time(NULL);

	EnterCriticalSection(&close_socket_lock);
	close_socket_arr[close_socket_count++] = (unsigned long)close_item;			
	LeaveCriticalSection(&close_socket_lock);

	iocp_post_queue_msg( WM_ONCLOSE ,(unsigned long)item );
}

/**
 *@收到消息回调
 */
void select_onrecv( SELECT_ITEM *&item )
{

	//获取客户端ip地址、端口信息
	int client_size = sizeof(item->addr);  
	ZeroMemory( &item->addr , sizeof(item->addr) );
	
	if( getpeername( item->socket , (SOCKADDR *)&item->addr , &client_size ) != 0 ) 
    {
		//getpeername失败
		iocp_post_queue_msg( WM_ONERROR, (unsigned long)item, WSAGetLastError() );
	}

	iocp_post_queue_msg( WM_ONRECV , (unsigned long)item);
}

/**
 *@accept工作线程
 */
unsigned int __stdcall  wing_select_server_accept( PVOID params ) {

	SOCKET sListen = *(SOCKET*)(params);
	int iaddrSize  = sizeof(SOCKADDR_IN);

	while (TRUE)
	{
		if( wing_select_clients_num >= max_connection ) 
		{
			//已达到最大连接数
			Sleep(10);
			continue;
		}

		SELECT_ITEM *item = new SELECT_ITEM();
		item->online = 1;
		item->active = time(NULL);

		item->socket = accept( sListen, (struct sockaddr *)&item->addr, &iaddrSize );
		if( INVALID_SOCKET == item->socket ) 
		{
			delete item;
			continue;
		}

		select_onconnect( item );
		
	}
	return 0;
}


/**
 *@select工作线程
 */
unsigned int __stdcall  wing_select_server_worder( PVOID params )
{
	unsigned long  i   = 0;
	unsigned long  ret = 0;
	struct timeval tv  = {1, 0};
	char *szMessage    = new char[MSGSIZE];
	fd_set fdread;
	
	while (TRUE)
	{
		if( wing_select_clients_num <= 0 ) 
		{
			Sleep(10);
			continue;
		}

		FD_ZERO( &fdread );                              //将fdread初始化空集

		for (i = 0; i < wing_select_clients_num; i++)
		{
			FD_SET(wing_server_clients_arr[i], &fdread); //将要检查的套接口加入到集合中
		}

		ret = select( 0, &fdread, NULL, NULL, &tv );     //每隔一段时间，检查可读性的套接口
		if( ret < 0 ) {                                  //发生严重错误
			//iocp_post_queue_msg( WM_ONERROR, (unsigned long)0, WSAGetLastError() );
		}
		if (ret == 0 )                                   //超时返回
		{
			continue;
		}

		for ( i = 0; i < wing_select_clients_num; i++ )
		{
			if ( FD_ISSET( wing_server_clients_arr[i], &fdread ) )                           //如果可读
			{
				memset( szMessage, 0, MSGSIZE );
				ret = recv( wing_server_clients_arr[i], szMessage, MSGSIZE, 0 );             //接收消息

				if (ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)) //客户端掉线
				{
					SELECT_ITEM *item = new SELECT_ITEM();

					item->online = 0;
					item->active = 0;
					item->socket = wing_server_clients_arr[i];

					select_onclose( item );
					wing_select_server_clients_remove( i );
				}
				else
				{
					if( ret <= 0 ) {
						continue;
					}

					SELECT_ITEM *item = new SELECT_ITEM();

					item->online     = 1;
					item->active     = time(NULL);
					item->socket     = wing_server_clients_arr[i];
					item->recv       = new char[ret+1];
					item->recv_bytes = ret;

					memset( item->recv, 0, ret+1 );
					memcpy( item->recv, szMessage ,ret );

					select_onrecv( item );
				}
			}
		}
	}

	delete[] szMessage;
	return 0;
}


/**
 *@释放socket资源线程，shutdown之后延迟5秒释放，经测试，效果稳定，哇哈哈哈
 */
unsigned int __stdcall  wing_close_socket_thread( PVOID params ) {
	
	CLOSE_ITEM *close_item = NULL;
	unsigned long i         = 0;
	unsigned long now_time  = 0;
	
	while( 1 ) {

		if( close_socket_count<=0 ) 
		{
			Sleep(10);
			continue;
		}

		EnterCriticalSection(&close_socket_lock);
		
		for( i=0; i < close_socket_count; i++ ) 
		{
			close_item = (CLOSE_ITEM *)close_socket_arr[i];
			now_time   = time(NULL);

			if( ( now_time-close_item->time ) >= 5 ) 
			{
				closesocket( close_item->socket );                  //清理socket，回收socket资源			
				for( unsigned long f = i; f < close_socket_count-1; f++ )  
				{
					close_socket_arr[f] = close_socket_arr[f+1];     //数组清理
				}
				close_socket_arr[close_socket_count-1] = 0;          //最后一条清零
				close_socket_count--;                                //元素计数器减少
				delete close_item;                                   //删除 清理内存
			}
		}
				
		LeaveCriticalSection(&close_socket_lock);
		Sleep(100);
	}
	return 0;
}

zend_class_entry *wing_select_server_ce;

/***
 *@构造方法
 */
ZEND_METHOD( wing_select_server, __construct )
{

	char *listen         = "0.0.0.0";   //监听ip
	int   listen_len     = 0;           //ip参数长度
	int   port           = 6998;        //监听端口
	int   max_connect    = 1000;        //最大连接数 也就是并发上限
	int   timeout        = 0;           //收发超时时间
	int   active_timeout = 0;           //多长时间不活动超时

	if( SUCCESS != zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "|slllll", &listen, &listen_len, &port, &max_connect, &timeout, &active_timeout ) ) {
		return;
	}
	zend_update_property_string(  wing_select_server_ce, getThis(), "listen",         strlen("listen"),         listen               TSRMLS_CC );
	zend_update_property_long(    wing_select_server_ce, getThis(), "port",           strlen("port"),           port                 TSRMLS_CC );
	zend_update_property_long(    wing_select_server_ce, getThis(), "max_connect",    strlen("max_connect"),    max_connect          TSRMLS_CC );
	zend_update_property_long(    wing_select_server_ce, getThis(), "timeout",        strlen("timeout"),        timeout              TSRMLS_CC );
	zend_update_property_long(    wing_select_server_ce, getThis(), "active_timeout", strlen("active_timeout"), active_timeout       TSRMLS_CC );

}
/***
 *@绑定事件回调
 */
ZEND_METHOD( wing_select_server, on )
{

	char *pro      = NULL;
	int   pro_len  = 0;
	zval *callback = NULL;

	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "sz", &pro, &pro_len, &callback ) != SUCCESS ) {
		return;
	}

	zend_update_property( wing_select_server_ce, getThis(), pro, pro_len, callback TSRMLS_CC );
}

/***
 *@开始服务
 */
ZEND_METHOD( wing_select_server, start )
{
	
	//启动服务
	zval *onreceive      = NULL;
	zval *onconnect      = NULL;
	zval *onclose        = NULL;
	zval *onerror        = NULL;
	zval *ontimeout      = NULL;
	zval *onsend         = NULL;

	int port           = 0;
	char *listen_ip    = NULL;
	int active_timeout = 0;

	MAKE_STD_ZVAL( onreceive );
	MAKE_STD_ZVAL( onconnect );
	MAKE_STD_ZVAL( onclose );
	MAKE_STD_ZVAL( onerror );


	onreceive			    = zend_read_property( wing_select_server_ce, getThis(),"onreceive",	     strlen("onreceive"),	    0 TSRMLS_CC);
	onconnect			    = zend_read_property( wing_select_server_ce, getThis(),"onconnect",	     strlen("onconnect"),	    0 TSRMLS_CC);
	onclose				    = zend_read_property( wing_select_server_ce, getThis(),"onclose",	     strlen("onclose"),         0 TSRMLS_CC);
	onerror				    = zend_read_property( wing_select_server_ce, getThis(),"onerror",	     strlen("onerror"),         0 TSRMLS_CC);
	ontimeout               = zend_read_property( wing_select_server_ce, getThis(),"ontimeout",	     strlen("ontimeout"),       0 TSRMLS_CC);
	onsend                  = zend_read_property( wing_select_server_ce, getThis(),"onsend",	     strlen("onsend"),          0 TSRMLS_CC);

	zval *_listen		    = zend_read_property( wing_select_server_ce, getThis(),"listen",	     strlen("listen"),		    0 TSRMLS_CC);
	zval *_port			    = zend_read_property( wing_select_server_ce, getThis(),"port",		     strlen("port"),	        0 TSRMLS_CC);
	zval *_max_connect	    = zend_read_property( wing_select_server_ce, getThis(),"max_connect",    strlen("max_connect"),     0 TSRMLS_CC);
	zval *_timeout		    = zend_read_property( wing_select_server_ce, getThis(),"timeout",	     strlen("timeout"),         0 TSRMLS_CC);
	zval *_active_timeout   = zend_read_property( wing_select_server_ce, getThis(),"active_timeout", strlen("active_timeout"),  0 TSRMLS_CC);


	recv_and_send_timeout = Z_LVAL_P(_timeout);
	listen_ip			  = Z_STRVAL_P(_listen);
	port				  = Z_LVAL_P(_port);
	max_connection		  = Z_LVAL_P(_max_connect);
	active_timeout		  = Z_LVAL_P(_active_timeout);

	zend_printf("==================== wing select server %s ====================\r\n", PHP_WING_VERSION );
	

	//---start---------------------------------------------------------
	//初始化服务端socket 如果失败返回INVALID_SOCKET
	WSADATA wsaData; 
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{
		return; 
	}

	// 检查是否申请了所需版本的套接字库   
	if( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
	{
        WSACleanup(); 
        return;  
    }  

	//创建sokket
	SOCKET m_sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( INVALID_SOCKET == m_sockListen )
	{
		WSACleanup();
		return;
	}

	BOOL bReuse = 1;
	//可选设置 SO_REUSEADDR 
	if( 0 != ::setsockopt(m_sockListen,SOL_SOCKET,SO_REUSEADDR,(LPCSTR)&bReuse,sizeof(BOOL)) )
	{
		//设置错误 这里先不处理 因为是可选的设置
	}


	// 填充地址结构信息
	struct sockaddr_in ServerAddress; 
	ZeroMemory(&ServerAddress, sizeof(ServerAddress)); 

	ServerAddress.sin_family		= AF_INET;                    
	ServerAddress.sin_addr.s_addr	= inet_addr(listen_ip);          
	ServerAddress.sin_port			= htons(port);   


	// 绑定端口
	if ( SOCKET_ERROR == bind( m_sockListen, (struct sockaddr *) &ServerAddress, sizeof( ServerAddress ) ) )
	{
		closesocket(m_sockListen);
		WSACleanup();
		return;
	}  

	// 开始监听
	if( 0 != listen( m_sockListen , SOMAXCONN ) )
	{
		closesocket(m_sockListen);
		WSACleanup();
		return;
	}
	

	iocp_message_queue_init();  //初始化消息队列
	wing_select_server_init();  //初始化一些全局变量

	HANDLE _thread;

	//负责accept新客户端的线程
	_thread=(HANDLE)_beginthreadex(NULL, 0, wing_select_server_accept, (void*)&m_sockListen, 0, NULL); 
	if( !_thread ) 
	{
		closesocket(m_sockListen);
		WSACleanup();
		iocp_message_queue_clear();
		wing_select_server_clear();
		return;
	}
	CloseHandle( _thread );
	
	//负责select的线程
	 _thread = (HANDLE)_beginthreadex( NULL, 0, wing_select_server_worder, NULL, 0, NULL ); 
	if( !_thread ) 
	{
		closesocket(m_sockListen);
		WSACleanup();
		iocp_message_queue_clear();
		wing_select_server_clear();
		return;
	}
	CloseHandle( _thread );

	//负责释放socket资源的线程
	 _thread = (HANDLE)_beginthreadex (NULL, 0, wing_close_socket_thread, NULL, 0, NULL ); 
	if( !_thread ) 
	{
		closesocket(m_sockListen);
		WSACleanup();
		iocp_message_queue_clear();
		wing_select_server_clear();
		return;
	}
	CloseHandle( _thread );

	SELECT_ITEM *item               = NULL;  //消息内容
	zval *wing_sclient              = NULL;  //sclient连接到服务端的客户端
	iocp_message_queue_element *msg = NULL;  //消息节点

	while( true )
	{ 
		//获取消息 没有的时候会阻塞
		iocp_message_queue_get(msg);

		switch( msg->message_id )
		{
		    case WM_ONSEND:
			{
				
				SOCKET send_socket     = (SOCKET)msg->wparam;
				long   send_status     = msg->lparam;

				item = new SELECT_ITEM();

				item->online  = 1;
				item->active  = time(NULL);
				item->socket  = send_socket;
				int iaddrSize = sizeof(SOCKADDR_IN);
				memset(&item->addr,0,iaddrSize);

				if( INVALID_SOCKET != item->socket )
				{
					getpeername( item->socket , (SOCKADDR *)&item->addr , &iaddrSize ); 
				}
				
				zval *send_params[2]	= {0};
				
				MAKE_STD_ZVAL( send_params[1] );

				//实例化一个对象
				select_create_wing_sclient( send_params[0] , item TSRMLS_CC);

				ZVAL_LONG( send_params[1] , send_status );
				
				zend_try
				{
					iocp_call_func( &onsend TSRMLS_CC , 2 , send_params );
				}
				zend_catch
				{
					//php语法错误
				}
				zend_end_try();

				zval_ptr_dtor( &send_params[0] );
				zval_ptr_dtor( &send_params[1] );


				delete item;
				item = NULL;
			}
			break;
			case WM_ONCONNECT:
			{
				item =  (SELECT_ITEM*)msg->wparam;
				
				//wing_sclient 实例化一个对象
				select_create_wing_sclient( wing_sclient , item TSRMLS_CC);
				
				zend_try
				{
					iocp_call_func( &onconnect TSRMLS_CC, 1, &wing_sclient );
				}
				zend_catch
				{
					//php脚本语法错误
				}
				zend_end_try();

				//释放资源
				zval_ptr_dtor( &wing_sclient );

				wing_sclient = NULL;

				delete item;
				item = NULL;

			}
			break;
			case WM_ONCLOSE:
			{
				item =  (SELECT_ITEM*)msg->wparam;
				
				select_create_wing_sclient( wing_sclient , item TSRMLS_CC);
				
				zend_try
				{
					iocp_call_func( &onclose TSRMLS_CC, 1, &wing_sclient );
				}
				zend_catch
				{
					//php脚本语法错误
				}
				zend_end_try();

				//释放资源
				zval_ptr_dtor( &wing_sclient );

				wing_sclient = NULL;

				delete item;
				item = NULL;
			}
			break;
			case WM_ONRECV:
			{
				item =  (SELECT_ITEM*)msg->wparam;
			
				zval *recv_params[2]			= {0};
				
				MAKE_STD_ZVAL( recv_params[1] );

				select_create_wing_sclient( recv_params[0] , item TSRMLS_CC);


				ZVAL_STRINGL( recv_params[1] , item->recv , item->recv_bytes, 1 );
				
				zend_try
				{
					iocp_call_func( &onreceive TSRMLS_CC , 2 , recv_params );
				}
				zend_catch
				{
					//php语法错误
				}
				zend_end_try();
				
				zval_ptr_dtor( &recv_params[0] );
				zval_ptr_dtor( &recv_params[1] );


				delete[] item->recv;
				item->recv = NULL;

				delete item;
				item = NULL;
			}
			break;
			case WM_ONERROR:
			{
				//这里不对item进行删除
				SELECT_ITEM *item     = (SELECT_ITEM*)msg->wparam;
				int last_error        = (DWORD)msg->lparam;
				
			
				//获取错误码对应的错误描述
				HLOCAL hlocal     = NULL;
				DWORD systemlocal = MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL);
				BOOL fok          = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER , NULL , last_error, systemlocal , (LPSTR)&hlocal , 0 , NULL );
				if( !fok ) 
				{
					if( hlocal ) 
					{
						LocalFree( hlocal );
						hlocal = NULL;
					}

					HMODULE hDll  = LoadLibraryEx("netmsg.dll",NULL,DONT_RESOLVE_DLL_REFERENCES);
					if( NULL != hDll ) 
					{
						 fok  = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER , hDll , last_error, systemlocal , (LPSTR)&hlocal , 0 , NULL );
						 FreeLibrary( hDll );
					}
				}

				zval *params[3] = {0};

				MAKE_STD_ZVAL( params[1] );
				MAKE_STD_ZVAL( params[2] );
				
				select_create_wing_sclient( params[0] , item TSRMLS_CC);
				ZVAL_LONG( params[1], msg->lparam );              //自定义错误编码

				if( fok && hlocal != NULL ) 
				{

					char *gbk_error_msg   = (char*)LocalLock( hlocal );
					char *utf8_error_msg  = NULL;

					//把gbk转换为utf8
					iocp_gbk_to_utf8( gbk_error_msg, utf8_error_msg );
					
					if( utf8_error_msg )
					{
						ZVAL_STRING( params[2], utf8_error_msg, 1 );  //WSAGetLasterror 错误
						delete[] utf8_error_msg;
					}
					else
					{
						ZVAL_STRING( params[2], gbk_error_msg, 1 );  //WSAGetLasterror 错误
					}
			
					LocalFree( hlocal );
					
				}else{
					
					ZVAL_STRING( params[2], "unknow error", 1 );     //WSAGetLasterror 错误
					
				}

				zend_try
				{
					iocp_call_func( &onerror TSRMLS_CC , 3 , params );
				}
				zend_catch
				{
					//php语法错误
				}
				zend_end_try();	

				
				zval_ptr_dtor( &params[0] );
				zval_ptr_dtor( &params[1] );
				zval_ptr_dtor( &params[2] );
			}
			break;
		}

		delete msg;
		msg = NULL;  
	}

	closesocket(m_sockListen);
	WSACleanup();
	iocp_message_queue_clear();
	wing_select_server_clear();
	return;
}

