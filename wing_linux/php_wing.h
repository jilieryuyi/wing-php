
/**
 * @ wing php - linux 版本
 * @ author yuyi
 * @ email 297341015@qq.com
 * @ create 2016-08-23 
 */

#ifndef WING_H
#define WING_H

#define MAX_PATH 260
#define WM_USER                         0x0400

#define WING_EMPTY_STRING '\0'
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


//加载config.h，如果配置了的话
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define PHP_WING_VERSION "1.0.8"
//加载php头文件
#include "php.h"
#define phpext_wing_ptr &walu_module_entry
extern zend_module_entry wing_module_entry;

#ifdef ZTS
#include "TSRM.h"
#endif

char* PHP_PATH = NULL;
extern void wing_get_cpu_id (char *cpuid);
extern int wing_get_module_file_name( char* module_name, char* file_name, int size );

#endif