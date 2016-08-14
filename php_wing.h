/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */


#ifndef PHP_WING_H
#define PHP_WING_H

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

/* If you declare any globals in php_wing.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(wing)
*/

/* True global resources - no need for thread safety here */
static int le_wing;
char *PHP_PATH = NULL;



#include <Winsock2.h>
#include "Windows.h"
#include "Winbase.h"

#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"
#include "process.h"
#include <mswsock.h>

#include <ws2tcpip.h>//ipv4 ipv6
//int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result );
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms742203(v=vs.85).aspx

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#include "wing_message_queue.h"
#include "wing_iocp_message_queue.h"
#include "wing_utf8.h"
#include "wing_socket_api.h"
#include "wing_ntdll.h"
#include "wing_base.h"


extern zend_module_entry wing_module_entry;
#define phpext_wing_ptr &wing_module_entry

#define PHP_WING_VERSION "1.0.8" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_WING_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WING_API __attribute__ ((visibility("default")))
#else
#	define PHP_WING_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(wing)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(wing)
*/

/* In every utility function you add that needs to use variables 
   in php_wing_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as WING_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define WING_G(v) TSRMG(wing_globals_id, zend_wing_globals *, v)
#else
#define WING_G(v) (wing_globals.v)
#endif

#define DATA_BUFSIZE 1024

//自定义消息
#define WM_ONCONNECT		WM_USER + 60
#define WM_ACCEPT_ERROR		WM_USER + 61
#define WM_ONERROR			WM_USER + 62
#define WM_ONCLOSE			WM_USER + 63
#define WM_ONRECV			WM_USER + 64
#define WM_ONQUIT           WM_USER + 65
#define WM_ONCLOSE_EX		WM_USER + 66
#define WM_ONSEND			WM_USER + 67
#define WM_THREAD_RUN		WM_USER + 71
#define WM_TIMEOUT          WM_USER + 72
#define WM_ADD_CLIENT       WM_USER + 73
#define WM_ONCLOSE2			WM_USER + 75
#define WM_ONBEAT			WM_USER + 76
#define WM_POST_RECV_ERR    WM_USER + 77


//错误码
#define WING_ERROR_SUCCESS				   1
#define WING_ERROR_CALLBACK_SUCCESS		   0
#define WING_ERROR_PARAMETER_ERROR	      -1
#define WING_ERROR_FAILED			      -2
#define WING_NOTICE_IGNORE			      -3
#define WING_ERROR_CALLBACK_FAILED        -4
#define WING_ERROR_PROCESS_NOT_EXISTS     -5
#define WING_ERROR_WINDOW_NOT_FOUND       -6
#define WING_ERROR_PROCESS_IS_RUNNING      1

//完成端口操作码
#define OP_ACCEPT 1
#define OP_RECV   2
#define OP_SEND   3


#define CLIENT_IOCP   1
#define CLIENT_SELECT 2

#define WING_SEARCH_BY_PROCESS_EXE_FILE  1
#define WING_SEARCH_BY_PROCESS_NAME      1

#define WING_SEARCH_BY_PROCESS_ID        2
#define WING_SEARCH_BY_PARENT_PROCESS_ID 3
#define WING_SEARCH_BY_COMMAND_LINE      4

#define WING_SEARCH_BY_PROCESS_EXE_PATH  5
#define WING_SEARCH_BY_PROCESS_FILE_PATH 5

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
	int			m_timeout;                     //设置超时
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




void iocp_call_func( zval **func TSRMLS_DC ,int params_count  ,zval **params );
BOOL iocp_socket_send( SOCKET socket,char *&msg , int len );
void wing_guid( _Out_ char *&buf TSRMLS_DC) ;

//---------------------------------------------------------
//                  wing select server
//---------------------------------------------------------
ZEND_METHOD( wing_select_server, __construct );
ZEND_METHOD( wing_select_server, on );
ZEND_METHOD( wing_select_server, start );

//---------------------------------------------------------
//                  wing sclient
//---------------------------------------------------------
ZEND_METHOD( wing_sclient, send );

//---------------------------------------------------------
//                  wing iocp server
//---------------------------------------------------------
ZEND_METHOD(wing_server,__construct);
ZEND_METHOD(wing_server,on);
ZEND_METHOD(wing_server,start);

PHP_FUNCTION( wing_version );
ZEND_FUNCTION( wing_get_last_error );
ZEND_FUNCTION( wing_wsa_get_last_error );
ZEND_FUNCTION( wing_get_error_msg );
ZEND_FUNCTION( wing_get_memory_used );

PHP_FUNCTION( wing_process_wait );
ZEND_FUNCTION( wing_get_process_params );
PHP_FUNCTION( wing_create_process );
PHP_FUNCTION( wing_create_process_ex );
ZEND_FUNCTION( wing_process_kill );
ZEND_FUNCTION( wing_get_current_process_id );
ZEND_FUNCTION( wing_query_object );
ZEND_FUNCTION( wing_create_mutex ) ;
ZEND_FUNCTION( wing_close_mutex );
ZEND_FUNCTION( wing_query_process );

ZEND_FUNCTION( wing_get_env );
ZEND_FUNCTION( wing_set_env );
ZEND_FUNCTION( wing_get_command_path );
ZEND_FUNCTION( wing_get_command_line );
ZEND_FUNCTION( wing_override_function );

ZEND_FUNCTION( wing_windows_send_msg );
ZEND_FUNCTION( wing_windows_version );

#endif	/* PHP_WING_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
