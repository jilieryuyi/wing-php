/*
  +----------------------------------------------------------------------+
  | WING PHP Version 1.0.0                                                        |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: xiaoan Huang                                                             |
  +----------------------------------------------------------------------+
  | Email: 297341015@qq.com
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "zend_constants.h"
#include "ext/standard/info.h"
#include "php_wing.h"



#include "lib/wing_msg_queue.h"
#include "lib/wing_socket.h"
#include "lib/wing_lib.h"


#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || (PHP_MAJOR_VERSION >= 6)
#undef ZVAL_REFCOUNT
#undef ZVAL_ADDREF
#undef ZVAL_DELREF
#define ZVAL_REFCOUNT Z_REFCOUNT_P
#define ZVAL_ADDREF Z_ADDREF_P
#define ZVAL_DELREF Z_DELREF_P
#endif

/* If you declare any globals in php_wing.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(wing)
*/

/* True global resources - no need for thread safety here */
static int le_wing = 0;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("wing.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_wing_globals, wing_globals)
    STD_PHP_INI_ENTRY("wing.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_wing_globals, wing_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */



/*****************************************************************************************
 * @ socket
 *****************************************************************************************/

//iocp 服务主线程
ZEND_FUNCTION(wing_service){

	zval *onreceive = NULL;
	zval *onconnect = NULL;
	zval *onclose = NULL;
	zval *onerror = NULL;
	zval *service_params = NULL;
	int port = 0;
	char *listen_ip = NULL;
	int timeout = 0;
	int max_connect = 1000;

	MAKE_STD_ZVAL(onreceive);
	MAKE_STD_ZVAL(onconnect);
	MAKE_STD_ZVAL(onclose);
	MAKE_STD_ZVAL(onerror);


	//参数获取
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&service_params) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	//如果参数不是数组
	if(Z_TYPE_P(service_params) != IS_ARRAY){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	
	HashTable *arr_hash = Z_ARRVAL_P(service_params);
	int argc = zend_hash_num_elements(arr_hash);
	zval  **data = NULL;
	HashPosition pointer = NULL;
			
	//数组参数解析
	for( zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
		 zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
		 zend_hash_move_forward_ex(arr_hash, &pointer)) 
	{
        char *key = NULL;
        int key_len = 0;
        long index = 0;
        
		if ( zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer) == HASH_KEY_IS_STRING) 
		{
			if( strcmp(key,"port") == 0 )
			{
				port = Z_LVAL_PP(data);
			}
			else if(strcmp(key,"listen")==0)
			{
				listen_ip = Z_STRVAL_PP(data);
			}
			else if(strcmp(key,"onreceive")==0)
			{
				onreceive = *data;
			}
			else if(strcmp(key,"onconnect")==0)
			{
				onconnect = *data;
			}
			else if(strcmp(key,"onclose")==0)
			{
				onclose = *data;
			}
			else if(strcmp(key,"onerror")==0)
			{
				onerror = *data;
			}
			else if(strcmp(key,"timeout")==0)
			{
				timeout = Z_LVAL_PP(data);
			}
			else if(strcmp(key,"max_connect")==0)
			{
				max_connect = Z_LVAL_PP(data);
			}
        } 
    } 

	//初始化消息队列
	wing_msg_queue_init();  

	//初始化服务端socket 如果失败返回INVALID_SOCKET
	SOCKET m_sockListen = wing_socket_init((const char *)listen_ip,(const int)port,(const int)max_connect,(const int) timeout);
	if( INVALID_SOCKET == m_sockListen ) 
	{
		wing_socket_clear();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	
	//消息队列载体
	wing_msg_queue_element *msg = NULL;//消息
	zend_printf("sockets pool %ld\r\n",wing_get_sockets_map_size());

	while( true )
	{ 
		//获取消息 没有的时候会阻塞
		wing_msg_queue_get(&msg);
		zend_printf("sockets pool %ld\r\n",wing_get_sockets_map_size());

		//根据消息ID进行不同的处理
		switch(msg->message_id){
			
			case WM_ONCONNECT:
			{
				//新的连接
				//zend_printf("===================================new connect===================================\r\n");
				
				zval *params[5] = {0};
				zval *retval_ptr = NULL;

				unsigned long socket_connect = (unsigned long)msg->wparam;
				MYOVERLAPPED *lpol = (MYOVERLAPPED *)wing_get_from_sockets_map(socket_connect);

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(params[2]);
				MAKE_STD_ZVAL(params[3]);
				MAKE_STD_ZVAL(params[4]);

				ZVAL_LONG(params[0],(long)msg->wparam);//socket资源
				ZVAL_STRING(params[1],inet_ntoa(lpol->m_addrClient.sin_addr),1);//ip
				ZVAL_LONG(params[2],ntohs(lpol->m_addrClient.sin_port));//port
				ZVAL_LONG(params[3],lpol->m_addrClient.sin_family);//协议类型
				ZVAL_STRING(params[4],lpol->m_addrClient.sin_zero,1);//这个zero不知道干嘛的 这里也直接支持返回

				MAKE_STD_ZVAL(retval_ptr);
				
				zend_try{
					//通过回调 把相关信息传回给php
					if( SUCCESS != call_user_function(EG(function_table),NULL,onconnect,retval_ptr,5,params TSRMLS_CC ) ){
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "onconnect callback fail");
					}
				}
				zend_catch{
					zend_printf("php syntax error\r\n");
				}
				zend_end_try();

				//释放资源
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				zval_ptr_dtor(&params[2]);
				zval_ptr_dtor(&params[3]);
				zval_ptr_dtor(&params[4]);
						  
			}
			break;
			case WM_ONSEND:{
				//zend_printf("onsend\r\n");
					//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
					//zend_printf("size-onsend:%d\r\n",pmc.WorkingSetSize-msg->size);	   
			}break;
			//目前暂时没有用到 先留着
			case WM_ACCEPT_ERROR:
			{
				//zend_printf("onaccepterror\r\n");
				//zend_printf("accept error\r\n");
				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-accepterror:%d\r\n",pmc.WorkingSetSize-msg->size);	
			}
			break;
			//收到消息
			case WM_ONRECV:
			{
				
				//zend_printf("===================================onrecv===================================\r\n");	
				///
				RECV_MSG *temp = (RECV_MSG*)msg->lparam;
				SOCKET client  = (SOCKET)msg->wparam;

				zval *params[2] = {0};
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(retval_ptr);

				ZVAL_LONG(params[0],client);
				ZVAL_STRINGL(params[1],temp->msg,temp->len,1);

				zend_try{
					if( SUCCESS != call_user_function(EG(function_table),NULL,onreceive,retval_ptr,2,params TSRMLS_CC) ){
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "onreceive callback fail");
					}
				}
				zend_catch{
					zend_printf("php syntax error\r\n");
				}
				zend_end_try();

				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);

				delete[] temp->msg;
				temp->msg = NULL;

				delete temp;
				temp = NULL;
			}
			break;
			//调用 _close_socket 服务端主动关闭socket
			case WM_ONCLOSE_EX:{
				//zend_printf("===================================onclose ex===================================\r\n");	

				unsigned long socket_to_close = (unsigned long)msg->wparam;
				MYOVERLAPPED *lpol = (MYOVERLAPPED *)wing_get_from_sockets_map(socket_to_close);
				wing_socket_on_close(lpol);
			}break;
			
			//客户端掉线了
			case WM_ONCLOSE:
			{
				//zend_printf("===================================onclose===================================\r\n");	
				
				//那个客户端掉线了
				SOCKET client =(SOCKET)msg->wparam;

				zval *params = NULL;
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params);
				ZVAL_LONG(params,(long)client);
				MAKE_STD_ZVAL(retval_ptr);
	 
				zend_try{
					if( SUCCESS != call_user_function(EG(function_table),NULL,onclose,retval_ptr,1,&params TSRMLS_CC ) ){
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "onclose callback fail");
					}
				}
				zend_catch{
					zend_printf("php syntax error\r\n");
				}
				zend_end_try();
							 
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);

			}
			break; 
			//发生错误 目前暂时也还没有用到
			case WM_ONERROR:{
				
				//zend_printf("===============onerror===============r\n");	
				zval *params[3] = {0};
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(params[2]);

				ZVAL_LONG(params[0],(long)msg->eparam);//发生错误的socket
				ZVAL_LONG(params[1],(long)msg->wparam);//自定义错误编码
				ZVAL_LONG(params[2],(long)msg->lparam);//WSAGetLasterror 错误码

				MAKE_STD_ZVAL(retval_ptr);
				
				zend_try{
					if( SUCCESS != call_user_function(EG(function_table),NULL,onerror,retval_ptr,3,params TSRMLS_CC ) ){
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "onerror callback fail");
					}	
				}
				zend_catch{
					zend_printf("php syntax error\r\n");
				}
				zend_end_try();

				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				zval_ptr_dtor(&params[2]);	
			}
			break;
			//退出服务 暂时没有测试
			case WM_ONQUIT:
			{
				//析构函数
				wing_msg_queue_clear();
				
				delete msg;
				msg = NULL;

				RETURN_LONG(WING_SUCCESS);
				return;
			}break;

		}

		delete msg;
		msg = NULL;  
    } 
	wing_socket_clear();
	RETURN_LONG( WING_SUCCESS );
	return;
}
/***********************************
 * @停止服务
 ***********************************/
ZEND_FUNCTION(wing_service_stop){
	wing_post_queue_msg(WM_ONQUIT);
	RETURN_LONG(WING_SUCCESS);
}

/********************************************
 * @ 关闭socket
 * @ param socket
 ********************************************/
ZEND_FUNCTION(wing_close_socket){

	zval *socket = NULL;
	
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	convert_to_long(socket);
	wing_post_queue_msg(WM_ONCLOSE_EX,Z_LVAL_P(socket));

	RETURN_LONG(WING_SUCCESS);
}

/*****************************************
 * @获取socket信息，ip 协议 端口 等
 * @return array //GetAcceptExSockaddrs
 ****************************************/
ZEND_FUNCTION(wing_socket_info){

	zval *socket = NULL;
	MAKE_STD_ZVAL(socket);
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	convert_to_long(socket);
	SOCKADDR_IN addr_conn;  
	int nSize = sizeof(addr_conn);  
	
	//memset((void *)&addr_conn,0,sizeof(addr_conn)); 
	ZeroMemory(&addr_conn,sizeof(addr_conn));
	getpeername((SOCKET)Z_LVAL_P(socket),(SOCKADDR *)&addr_conn,&nSize);  
  
	array_init(return_value);
	add_assoc_string(return_value,"sin_addr",inet_ntoa(addr_conn.sin_addr),1);
    add_assoc_long(return_value,"sin_family",addr_conn.sin_family);
	add_assoc_long(return_value,"sin_port",ntohs(addr_conn.sin_port));
	add_assoc_string(return_value,"sin_zero",addr_conn.sin_zero,1);

	return;
}
/*****************************************
 * @ 发送socket数据
 * @ 同步发送接口 没有使用iocp
 ****************************************/
ZEND_FUNCTION(wing_socket_send_msg)
{  

	zval *socket;
	zval *msg;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",&socket,&msg) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long(socket);

	//此处没有使用完成端口 完成端口发送后 如果直接调用 socketclose 
	//关闭socket 有坑，处理不好会有内存泄漏
	if( SOCKET_ERROR == send((SOCKET)Z_LVAL_P(socket),Z_STRVAL_P(msg),Z_STRLEN_P(msg),0)){
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	
	RETURN_LONG(WING_SUCCESS);
	return;
}  
/***************************************************
 * @ 使用iocp异步发送消息--未测试
 ***************************************************/ 
ZEND_FUNCTION(wing_socket_send_msg_ex){
	zval *socket = NULL;
	zval *msg	 = NULL;
	int close_after_send = 0;//发送完关闭socket 默认为false 否 待定 还没开发

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|l",&socket,&msg,&close_after_send) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long(socket);

	
	SOCKET sClient		= (SOCKET)Z_LVAL_P(socket);
	char *sendbuf		= Z_STRVAL_P(msg);
	ulong bufsize		= Z_STRLEN_P(msg);
	ulong  Flag			= 0;  
	ulong SendByte		= 0;  

	WSABUF DataBuf;
	WSAOVERLAPPED SendOverlapped;

    if ( sClient == INVALID_SOCKET || sendbuf == NULL || bufsize == 0 )
	{
		RETURN_LONG(WING_ERROR_FAILED);
		return;  
	}
   
	
	SecureZeroMemory((PVOID) & SendOverlapped, sizeof (WSAOVERLAPPED));
	SendOverlapped.hEvent = WSACreateEvent();
    if (SendOverlapped.hEvent == NULL) 
	{
        RETURN_LONG(WING_ERROR_FAILED);
		return;  
    }


	DataBuf.buf	= sendbuf; 
	DataBuf.len	= bufsize; 
	
	int bRet  = WSASend(sClient,&(DataBuf),1,&SendByte,Flag,&(SendOverlapped),NULL);  
	if( bRet != 0 &&  WSAGetLastError() != WSA_IO_PENDING ){
		WSAResetEvent(SendOverlapped.hEvent);
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	bRet = WSAWaitForMultipleEvents(1, &SendOverlapped.hEvent, TRUE, INFINITE,
                                      TRUE);
	if ( bRet == WSA_WAIT_FAILED) {
		WSAResetEvent(SendOverlapped.hEvent);
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	bRet = WSAGetOverlappedResult(sClient, &SendOverlapped, &SendByte,FALSE, &Flag);
	if ( bRet == FALSE) 
	{
		WSAResetEvent(SendOverlapped.hEvent);
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	WSAResetEvent(SendOverlapped.hEvent);
	RETURN_LONG(WING_SUCCESS);
}

//////////////////////////--socket-end--

/*********************************
 * @获取使用的内存信息 
 * @进程实际占用的内存大小
 *********************************/
ZEND_FUNCTION(wing_get_memory_used){

	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	CloseHandle(handle);
	RETURN_LONG( pmc.WorkingSetSize );
	return;
}
//----------------------wing_client class--------------------------
zend_class_entry *wing_client_ce;

ZEND_METHOD(wing_client,close){
	zval *socket = zend_read_property(wing_client_ce,getThis(),"socket",strlen("socket"),0 TSRMLS_CC);
	int isocket = Z_LVAL_P(socket);
	if(isocket<=0) {
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	wing_post_queue_msg(WM_ONCLOSE_EX,isocket);
	RETURN_LONG(WING_SUCCESS);
}
ZEND_METHOD(wing_client,send){
	zval *msg;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&msg) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	zval *socket = zend_read_property(wing_client_ce,getThis(),"socket",strlen("socket"),0 TSRMLS_CC);
	
	int isocket = Z_LVAL_P(socket);
	if(isocket<=0) {
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	if( SOCKET_ERROR == send((SOCKET)Z_LVAL_P(socket),Z_STRVAL_P(msg),Z_STRLEN_P(msg),0)){
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	RETURN_LONG(WING_SUCCESS);
	return;
				
}
static zend_function_entry wing_client_method[]={
	ZEND_ME(wing_client,close,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(wing_client,send,NULL,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};



//----------------------wing_server class--------------------------
zend_class_entry *wing_server_ce;
ZEND_METHOD(wing_server,__construct){
	//构造方法 ip 端口 最大连接数
	zval *listen = NULL;
	//MAKE_STD_ZVAL(listen);
	//ZVAL_STRING(listen,"0.0.0.0",1);

	int port = 6998;
	int max_connect = 1000;
	int timeout = 0;
	int active_timeout = 0;
	int tick = 0;

	if( SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zl|llll",&listen,&port,&max_connect,&timeout,&active_timeout,&tick)){
	
	}
	zend_update_property_string(  wing_server_ce,getThis(),"listen",         strlen("listen"),         Z_STRVAL_P(listen)   TSRMLS_CC);
	zend_update_property_long(    wing_server_ce,getThis(),"port",           strlen("port"),           port                 TSRMLS_CC);
	zend_update_property_long(    wing_server_ce,getThis(),"max_connect",    strlen("max_connect"),    max_connect          TSRMLS_CC);
	zend_update_property_long(    wing_server_ce,getThis(),"timeout",        strlen("timeout"),        timeout              TSRMLS_CC);
	zend_update_property_long(    wing_server_ce,getThis(),"active_timeout", strlen("active_timeout"), active_timeout       TSRMLS_CC);
	zend_update_property_long(    wing_server_ce,getThis(),"tick",           strlen("tick"),           tick                 TSRMLS_CC);
}
ZEND_METHOD(wing_server,on){
	zval *pro = NULL;
	zval *callback = NULL;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz",&pro,&callback);

	zend_update_property(wing_server_ce,getThis(),Z_STRVAL_P(pro),Z_STRLEN_P(pro),callback TSRMLS_CC);
}


int wing_is_call_able(zval **var TSRMLS_DC){
	char *error = NULL;
	zend_bool is_call_able = zend_is_callable_ex(*var, NULL, 0, NULL, NULL, NULL, &error TSRMLS_CC);
	if( error ) 
		efree( error );
	return is_call_able ? 1 : 0;
}

void wing_call_func( zval **func TSRMLS_DC ,int params_count = 0 ,zval **params = NULL) {
	if( !wing_is_call_able(func TSRMLS_CC) ) return;
	zval *retval_ptr = NULL;
	MAKE_STD_ZVAL(retval_ptr);
	if( SUCCESS != call_user_function( EG(function_table),NULL,*func,retval_ptr,params_count,params TSRMLS_CC ) ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "call user func fail");
	}
	zval_ptr_dtor(&retval_ptr);
}
void wing_create_wing_client(zval **client , wing_myoverlapped *&lpol TSRMLS_DC){
	
	MAKE_STD_ZVAL( *client );
	object_init_ex( *client,wing_client_ce);
				
	//初始化
	if( lpol ) {
		zend_update_property_string( wing_client_ce,*client,"sin_addr",   strlen("sin_addr"),   inet_ntoa(lpol->m_addrClient.sin_addr) TSRMLS_CC);
		zend_update_property_long(   wing_client_ce,*client,"sin_port",   strlen("sin_port"),   ntohs(lpol->m_addrClient.sin_port)     TSRMLS_CC);
		zend_update_property_long(   wing_client_ce,*client,"sin_family", strlen("sin_family"), lpol->m_addrClient.sin_family          TSRMLS_CC);
		zend_update_property_string( wing_client_ce,*client,"sin_zero",   strlen("sin_zero"),   lpol->m_addrClient.sin_zero            TSRMLS_CC);
		zend_update_property_long(   wing_client_ce,*client,"last_active",strlen("last_active"),lpol->m_active                         TSRMLS_CC);
		zend_update_property_long(   wing_client_ce,*client,"socket",     strlen("socket"),     lpol->m_skClient                       TSRMLS_CC);
	}

}


void wing_event_callback(wing_msg_queue_element *&msg,zval *&callback TSRMLS_DC)
{
	zval *wing_client            = NULL;
	wing_myoverlapped *lpol      = (wing_myoverlapped*) msg->lparam;

	zend_printf("active:%ld\r\n",lpol->m_active);

	wing_create_wing_client( &wing_client , lpol TSRMLS_CC);
				
	zend_try
	{
		wing_call_func( &callback TSRMLS_CC,1,&wing_client );
	}
	zend_catch
	{
		//php脚本语法错误
	}
	zend_end_try();

	//释放资源
	zval_ptr_dtor( &wing_client );
}


ZEND_METHOD(wing_server,start){
	//启动服务
	zval *onreceive      = NULL;
	zval *onconnect      = NULL;
	zval *onclose        = NULL;
	zval *onerror        = NULL;
	zval *ontimeout      = NULL;
	zval *ontick         = NULL;
	zval *service_params = NULL;

	int port           = 0;
	char *listen_ip    = NULL;
	int timeout        = 0;
	int max_connect    = 1000;
	int active_timeout = 0;
	int tick           = 0;

	MAKE_STD_ZVAL( onreceive );
	MAKE_STD_ZVAL( onconnect );
	MAKE_STD_ZVAL( onclose );
	MAKE_STD_ZVAL( onerror );


	onreceive			    = zend_read_property( wing_server_ce,getThis(),"onreceive",	     strlen("onreceive"),	    0 TSRMLS_CC);
	onconnect			    = zend_read_property( wing_server_ce,getThis(),"onconnect",	     strlen("onconnect"),	    0 TSRMLS_CC);
	onclose				    = zend_read_property( wing_server_ce,getThis(),"onclose",	     strlen("onclose"),         0 TSRMLS_CC);
	onerror				    = zend_read_property( wing_server_ce,getThis(),"onerror",	     strlen("onerror"),         0 TSRMLS_CC);
	ontimeout               = zend_read_property( wing_server_ce,getThis(),"ontimeout",	     strlen("ontimeout"),       0 TSRMLS_CC);
	ontick                  = zend_read_property( wing_server_ce,getThis(),"ontick",	     strlen("ontick"),          0 TSRMLS_CC);

	zval *_listen		    = zend_read_property( wing_server_ce,getThis(),"listen",	     strlen("listen"),		    0 TSRMLS_CC);
	zval *_port			    = zend_read_property( wing_server_ce,getThis(),"port",		     strlen("port"),	        0 TSRMLS_CC);
	zval *_max_connect	    = zend_read_property( wing_server_ce,getThis(),"max_connect",    strlen("max_connect"),     0 TSRMLS_CC);
	zval *_timeout		    = zend_read_property( wing_server_ce,getThis(),"timeout",	     strlen("timeout"),         0 TSRMLS_CC);
	zval *_active_timeout   = zend_read_property( wing_server_ce,getThis(),"active_timeout", strlen("active_timeout"),  0 TSRMLS_CC);
	zval *_tick             = zend_read_property( wing_server_ce,getThis(),"tick",           strlen("tick"),            0 TSRMLS_CC);


	timeout				= Z_LVAL_P(_timeout);
	listen_ip			= Z_STRVAL_P(_listen);
	port				= Z_LVAL_P(_port);
	max_connect			= Z_LVAL_P(_max_connect);
	active_timeout		= Z_LVAL_P(_active_timeout);
	tick		        = Z_LVAL_P(_tick);

	//按需启动活动超时检测
	if( active_timeout > 0 ) {
		zend_printf("timeout event thread start ...\r\n");
		_beginthreadex(NULL, 0, wing_socket_check_active_timeout, (void*)&active_timeout, 0, NULL);  
	}

	//初始化消息队列
	wing_msg_queue_init();  


	//初始化服务端socket 如果失败返回INVALID_SOCKET
	SOCKET m_sockListen = wing_socket_init((const char *)listen_ip,(const int)port,(const int)max_connect,(const int) timeout);
	if( INVALID_SOCKET == m_sockListen ) 
	{
		wing_socket_clear();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	
	wing_msg_queue_element *msg = NULL;//消息

	while( true )
	{ 
		//获取消息 没有的时候会阻塞
		wing_msg_queue_get(&msg);

		switch( msg->message_id ){
			//这部分主要用于debug观察
			case WM_THREAD_RUN:
			{
				MYOVERLAPPED *lpol            = (MYOVERLAPPED*) msg->eparam;
				zend_printf("thread run %ld last error %ld socket=%ld\r\n",msg->wparam,msg->lparam,0);
			}break;
			case WM_ADD_CLIENT:
				{
					MYOVERLAPPED *lpol            = (MYOVERLAPPED*) msg->lparam;
					wing_socket_add_client(lpol);
				}break;
			
			case WM_TIMEOUT:
			{
				zend_printf("timeout event happened\r\n");
				wing_event_callback( msg , ontimeout TSRMLS_CC);
			}
			break;
			case WM_ONCONNECT:
			{
				wing_event_callback( msg , onconnect TSRMLS_CC);	  
			}
			break;
			//收到消息
			case WM_ONRECV:
			{
				

				char *recv_msg			= (char*)msg->lparam;
				wing_socket client		= (wing_socket)msg->wparam;
				zval *params[2]			= {0};
				wing_myoverlapped *lpol	= (wing_myoverlapped*)wing_get_from_sockets_map((unsigned long)msg->wparam);

				MAKE_STD_ZVAL( params[1] );
				wing_create_wing_client( &params[0] , lpol TSRMLS_CC);
				ZVAL_STRING( params[1] , recv_msg , 1 );
				
				zend_try
				{
					wing_call_func( &onreceive TSRMLS_CC , 2 , params );
				}
				zend_catch
				{
					//php语法错误
				}
				zend_end_try();

				zval_ptr_dtor( &params[0] );
				zval_ptr_dtor( &params[1] );

				delete[] recv_msg;
				recv_msg = NULL;
			}
			break;
			//服务端主动关闭socket
			case WM_ONCLOSE_EX:{
				//zend_printf("===================================onclose ex===================================\r\n");	

				unsigned long socket_to_close = (unsigned long)msg->wparam;
				MYOVERLAPPED *lpol            = (MYOVERLAPPED*)wing_get_from_sockets_map(socket_to_close);
				wing_socket_on_close(lpol);
			}break;
			
			//客户端掉线了
			case WM_ONCLOSE:
			{
				wing_event_callback( msg , onclose TSRMLS_CC);	
			}
			break; 
			//发生错误 目前暂时也还没有用到
			case WM_ONERROR:{
				
				//zend_printf("===============onerror===============r\n");	
				zval *params[3] = {0};

				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(params[2]);

				MYOVERLAPPED* lpol = (MYOVERLAPPED *)wing_get_from_sockets_map((unsigned long)msg->wparam);
				
				wing_create_wing_client( &params[0] , lpol TSRMLS_CC);

				ZVAL_LONG(params[1],msg->lparam);  //自定义错误编码
				ZVAL_LONG(params[2],msg->eparam);  //WSAGetLasterror 错误码

				zend_try{
					wing_call_func( &onerror TSRMLS_CC , 3 , params );
				}
				zend_catch{
					//php语法错误
				}
				zend_end_try();

				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				zval_ptr_dtor(&params[2]);	
			}
			break;
			//退出服务 暂时没有测试
			case WM_ONQUIT:
			{
				//析构函数
				wing_msg_queue_clear();
				
				delete msg;
				msg = NULL;

				RETURN_LONG(WING_SUCCESS);
				return;
			}break;

		}

		delete msg;
		msg = NULL;  
    } 
	wing_socket_clear();
	RETURN_LONG( WING_SUCCESS );
	return;
}

static zend_function_entry wing_server_methods[]={
	ZEND_ME(wing_server,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(wing_server,on,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(wing_server,start,NULL,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};


//----------------------wing_server class--------------------------
/* }}} */
/* The previous line is mant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_wing_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_wing_init_globals(zend_wing_globals *wing_globals)
{
	wing_globals->global_value = 0;
	wing_globals->global_string = NULL;
}
*/
/* }}} */
/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(wing)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	zend_class_entry  _wing_client_ce;
	INIT_CLASS_ENTRY( _wing_client_ce , "wing_client" , wing_client_method );
	wing_client_ce = zend_register_internal_class( &_wing_client_ce TSRMLS_CC );

	zend_declare_property_string( wing_client_ce,"sin_addr",    strlen("sin_addr"),   "", ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_client_ce,"sin_port",    strlen("sin_port"),   0,  ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_client_ce,"sin_family",  strlen("sin_family"), 0,  ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_string( wing_client_ce,"sin_zero",    strlen("sin_zero"),   "", ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_client_ce,"socket",      strlen("socket"),     0,  ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_client_ce,"last_active", strlen("last_active"),0,  ZEND_ACC_PUBLIC TSRMLS_CC );
	


	//----wing_server-----
	zend_class_entry  _wing_server_ce;
	INIT_CLASS_ENTRY( _wing_server_ce,"wing_server", wing_server_methods );
	wing_server_ce = zend_register_internal_class( &_wing_server_ce TSRMLS_CC );
	
	//事件回调函数 默认为null
	zend_declare_property_null(    wing_server_ce,"onreceive",   strlen("onreceive"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onconnect",   strlen("onconnect"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onclose",     strlen("onclose"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onerror",     strlen("onerror"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"ontimeout",   strlen("ontimeout"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"ontick",      strlen("ontick"),   ZEND_ACC_PRIVATE TSRMLS_CC);

	//端口和监听ip地址 
	zend_declare_property_long(   wing_server_ce,"port",           strlen("port"),           6998,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string( wing_server_ce,"listen",         strlen("listen"),         "0.0.0.0", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"max_connect",    strlen("max_connect"),    1000,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"timeout",        strlen("timeout"),        0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"tick",           strlen("tick"),           0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"active_timeout", strlen("active_timeout"), 0,         ZEND_ACC_PRIVATE TSRMLS_CC);


	//注册常量或者类等初始化操作
	//REGISTER_STRING_CONSTANT("WING_VERSION",PHP_WING_VERSION,CONST_CS | CONST_PERSISTENT);
	
	
	PHP_PATH = new char[MAX_PATH];
	if( 0 != GetModuleFileName(NULL,PHP_PATH,MAX_PATH) )
	{
		zend_register_string_constant("WING_PHP",                    sizeof("WING_PHP"),                     PHP_PATH,                      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	} 
	zend_register_string_constant( "WING_VERSION",                   sizeof("WING_VERSION"),                 PHP_WING_VERSION,              CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_TIMEOUT",              sizeof("WING_WAIT_TIMEOUT"),            WAIT_TIMEOUT,                  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_FAILED",               sizeof("WING_WAIT_FAILED"),             WAIT_FAILED,                   CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_INFINITE",                  sizeof("WING_INFINITE"),                INFINITE,                      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_OBJECT_0",             sizeof("WING_WAIT_OBJECT_0"),           WAIT_OBJECT_0,                 CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_ABANDONED",            sizeof("WING_WAIT_ABANDONED"),          WAIT_ABANDONED,                CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_ALREADY_EXISTS",      sizeof("WING_ERROR_ALREADY_EXISTS"),    ERROR_ALREADY_EXISTS,          CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_PARAMETER_ERROR",     sizeof("WING_ERROR_PARAMETER_ERROR"),   WING_ERROR_PARAMETER_ERROR,    CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_FAILED",              sizeof("WING_ERROR_FAILED"),            WING_ERROR_FAILED,             CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_CALLBACK_FAILED",     sizeof("WING_ERROR_CALLBACK_FAILED"),   WING_ERROR_CALLBACK_FAILED,    CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_CALLBACK_SUCCESS",          sizeof("WING_CALLBACK_SUCCESS"),        WING_CALLBACK_SUCCESS,         CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_PROCESS_NOT_EXISTS",  sizeof("WING_ERROR_PROCESS_NOT_EXISTS"),WING_ERROR_PROCESS_NOT_EXISTS, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SUCCESS",                   sizeof("WING_SUCCESS"),                 WING_SUCCESS,                  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);	
	zend_register_long_constant(   "WING_PROCESS_IS_RUNNING",        sizeof("WING_PROCESS_IS_RUNNING"),      WING_PROCESS_IS_RUNNING,       CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(wing)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	delete[] PHP_PATH;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(wing)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(wing)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(wing)
{
	php_info_print_table_start();
	php_info_print_table_header( 2, "wing support",    "enabled"          );
	php_info_print_table_row(    2, "version",         PHP_WING_VERSION   );
	php_info_print_table_row(    2, "author","         yuyi"              );
	php_info_print_table_row(    2, "email",           "297341015@qq.com" );
	php_info_print_table_row(    2, "qq-group",        "535218312"        );
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}

/* }}} */

/* {{{ wing_functions[]
 *
 * Every user visible function must have an entry in wing_functions[].
 */
const zend_function_entry wing_functions[] = {
	PHP_FE(wing_version,NULL)
	PHP_FE(wing_create_thread,NULL)
	PHP_FE(wing_create_process,NULL) //wing_thread_wait
	PHP_FE(wing_get_process_params,NULL)
	PHP_FE(wing_create_process_ex,NULL)
	//PHP_FE(wing_thread_wait,NULL)

	PHP_FE(wing_process_wait,NULL)
	//wing_thread_wait 是别名
	ZEND_FALIAS(wing_thread_wait,wing_process_wait,NULL)

	PHP_FE(wing_process_kill,NULL)
	ZEND_FALIAS(wing_thread_kill,wing_process_kill,NULL)
	ZEND_FALIAS(wing_kill_thread,wing_process_kill,NULL)
	ZEND_FALIAS(wing_kill_timer,wing_process_kill,NULL)
	ZEND_FALIAS(wing_kill_process,wing_process_kill,NULL)

	PHP_FE(wing_process_isalive,NULL)
	ZEND_FALIAS(wing_thread_isalive,wing_process_isalive,NULL)

	PHP_FE(wing_get_current_process_id,NULL)
	PHP_FE(wing_create_mutex,NULL)
	PHP_FE(wing_close_mutex,NULL)
	PHP_FE(wing_get_env,NULL)
	PHP_FE(wing_get_command_path,NULL)
	PHP_FE(wing_set_env,NULL)
	PHP_FE(wing_send_msg,NULL)

	PHP_FE(wing_get_last_error,NULL)
	PHP_FE(wing_wsa_get_last_error,NULL)

	PHP_FE(wing_create_window,NULL)
	PHP_FE(wing_message_loop,NULL)
	PHP_FE(wing_destory_window,NULL)
	PHP_FE(wing_message_box,NULL)
	PHP_FE(wing_timer,NULL)
	PHP_FE(wing_service,NULL)
	ZEND_FALIAS(wing_socket,wing_service,NULL)
	ZEND_FALIAS(wing_tcp_server,wing_service,NULL)

	PHP_FE(wing_socket_info,NULL)
	PHP_FE(wing_socket_send_msg,NULL)
	PHP_FE(wing_service_stop,NULL)
	PHP_FE(wing_close_socket,NULL)
	PHP_FE(wing_get_memory_used,NULL)
	PHP_FE_END	/* Must be the last line in wing_functions[] */
};
/* }}} */

/* {{{ wing_module_entry
 */
zend_module_entry wing_module_entry = {
	STANDARD_MODULE_HEADER,
	"wing",
	wing_functions,
	PHP_MINIT(wing),
	PHP_MSHUTDOWN(wing),
	PHP_RINIT(wing),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(wing),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(wing),
	PHP_WING_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_WING
ZEND_GET_MODULE(wing)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
