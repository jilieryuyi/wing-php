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

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <ctrdbg.h>

#include "php.h"
#include "php_ini.h"
#include "zend_constants.h"
#include "ext/standard/info.h"
#include "php_wing.h"


#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include <Winsock2.h>
#include "Winbase.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")

#include "lib/library.h"
#include "lib/socket.h"
#include "lib/queue.h"
#include "lib/memory.h"

//此部分函数来源于c++ cpp定义 不能直接include头文件 通过extern访问
//extern void get_command_path(const char *name,char *output);
//extern char* qrdecode(char* filename);
//extern char* qrencode(char* str,int imageWidth,char* save_path);

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
static int le_wing;
TCHAR  *PHP_PATH;
//timer 进程计数器 用于控制多个timer的创建和运行
static int wing_timer_count = 0;
//线程计数器 用于多线程控制
static int wing_thread_count =0;


#define WING_SUCCESS				 1
#define WING_CALLBACK_SUCCESS		 0
#define WING_ERROR_PARAMETER_ERROR	-1
#define WING_ERROR_FAILED			-2
#define WING_NOTICE_IGNORE			-3
#define WING_ERROR_CALLBACK_FAILED  -4
#define WING_ERROR_PROCESS_NOT_EXISTS -5
#define WING_ERROR_WINDOW_NOT_FOUND -6
#define WING_PROCESS_IS_RUNNING		1


#define WM_ONCONNECT		WM_USER+60
#define WM_ACCEPT_ERROR		WM_USER+61
#define WM_ONERROR			WM_USER+62
#define WM_ONCLOSE			WM_USER+63
#define WM_ONRECV			WM_USER+64
#define WM_ONQUIT           WM_USER+65
#define WM_ONCLOSE_EX		WM_USER+66

queue_t *message_queue;

void memory_times_show(){
	//if((memory_add_times-memory_sub_times)!=0)
	zend_printf("memory add times:%ld , memory sub times:%ld ,need more free:%ld \r\n",memory_add_times,memory_sub_times,(memory_add_times-memory_sub_times));

//unsigned long accept_add_times = 0;
//unsigned long accept_sub_times = 0;
	//if((accept_add_times-accept_sub_times)!=0)
	{
	zend_printf("accept times need more free:%ld\r\n",(accept_add_times-accept_sub_times));
	}

	//unsigned long queue_add_times = 0;
//unsigned long queue_sub_times = 0;
//	zend_printf("queue add times need more free:%ld\r\n",(queue_add_times-queue_sub_times));

	//unsigned long node_add_times = 0;
//unsigned long node_sub_times = 0;

	zend_printf("node add times need more free:%ld\r\n",(node_add_times-node_sub_times));

	zend_printf("accept and close:%ld\r\n",(accept_add_times_ex-accept_sub_times_ex));

	zend_printf("recv times:%ld\r\n",(recv_add_times-recv_sub_times));

	zend_printf("send msg times:%ld\r\n",(send_msg_add_times-send_msg_sub_times));

}

/**
 *@创建进程
 */
DWORD create_process(char *command,char *params_ex,int params_ex_len){
	    HANDLE				m_hRead;
		HANDLE				m_hWrite;
		STARTUPINFO			sui;    
		PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
		SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		
		char				*params    = "";
		int					params_len = 0;
    
		sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
		sa.lpSecurityDescriptor = NULL;
		sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
		if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
		{
			return WING_ERROR_FAILED;
		}

   
		ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
		sui.cb				= sizeof(STARTUPINFO);
		sui.dwFlags			= STARTF_USESTDHANDLES;  
		sui.hStdInput		= m_hRead;
		sui.hStdOutput		= m_hWrite;
		
		sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);
		
		if( params_ex_len >0 && params_ex != NULL ){
			DWORD d;
			if(::WriteFile(m_hWrite,params_ex,params_ex_len,&d,NULL)==FALSE){
				//告警
				zend_error(E_USER_WARNING,"write params to process error");
			}
		}

		if (!CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
		{
			CloseHandle(m_hRead);
			CloseHandle(m_hWrite);
			return WING_ERROR_FAILED;
		}
		
		CloseHandle(pi.hProcess); // 子进程的进程句柄
		CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		
		return pi.dwProcessId;	
}

void command_params_check(char *command_params,int *run_process,int *last_value){
	TSRMLS_FETCH();
	zval **argv;
	int argc;
	//char *command_params="";
	HashTable *arr_hash;
	//int run_process = 0;
	 //获取命令行参数
	if (zend_hash_find(&EG(symbol_table),"argv",sizeof("argv"),(void**)&argv) == SUCCESS){
		zval  **data;
		HashPosition pointer;
		arr_hash	= Z_ARRVAL_PP(argv);
		argc		= zend_hash_num_elements(arr_hash);
		for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; zend_hash_move_forward_ex(arr_hash, &pointer)) {
			if(strcmp((char*)Z_LVAL_PP(data),"wing-process")==0){
				*run_process=1;
			}

			char *key;
			int key_len,index;
			zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer);
			if(index>0){
				char *p = (char*)Z_LVAL_PP(data);
				if(p[0]!='\"')
					spprintf(&command_params,0,"%s \"%s\" ",command_params,p);
				else 
					spprintf(&command_params,0,"%s %s ",command_params,p);
			}

			if(index == argc-1&&last_value != NULL){
				 *last_value= atoi((char*)Z_LVAL_PP(data));
			}
		} 
	}
}

/*
typedef struct _timer_thread_params{
	DWORD thread_id;
	DWORD dwMilliseconds;
} timer_thread_params;
//子线程函数   
unsigned int __stdcall wing_timer(PVOID pM)  
{  
	timer_thread_params *param=(timer_thread_params *)pM;
	while(1){
		Sleep(param->dwMilliseconds);
		PostThreadMessageA(param->thread_id,WM_USER+99,NULL,NULL);
	}
	return 0;
}  */






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


PHP_FUNCTION(wing_version){
	char *string;
    int len;
	len = spprintf(&string, 0, "%s", PHP_WING_VERSION);
    RETURN_STRING(string,1);
}


/**
 *@wait process进程等待
 *@param process id 进程id
 *@param timeout 等待超时时间 单位毫秒
 *@return exit code 进程退出码
 */

PHP_FUNCTION(wing_process_wait){
	
	int thread_id,timeout=INFINITE;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|l",&thread_id,&timeout)!=SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS, FALSE, thread_id);

	DWORD wait_result;

	DWORD wait_status= WaitForSingleObject(handle,timeout);
	 
	 if(wait_status != WAIT_OBJECT_0){
		RETURN_LONG(wait_status);
		return;
	 }

	 //WING_WAIT_OBJECT_0
	 
	 if(GetExitCodeProcess(handle,&wait_result) == 0) 
		 RETURN_LONG(WING_ERROR_FAILED);

	 RETURN_LONG(wait_result);
}

/**
 *@创建多线程，使用进程模拟
 */
PHP_FUNCTION(wing_create_thread){
	
	wing_thread_count++;
	zval *callback;
	
	MAKE_STD_ZVAL(callback);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	char *command_params = "";
	int run_process = 0;
	int command_index = 0;
	int last_value = 0;

	command_params_check(command_params,&run_process,&last_value);

	char	*command;

	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_thread_count);

	if(!run_process){
		RETURN_LONG(create_process(command,NULL,0));	
		return;
	}

	if(wing_thread_count != last_value){
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}
	
	zval *retval_ptr;
	
			
	MAKE_STD_ZVAL(retval_ptr);
	if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
		RETURN_LONG(WING_ERROR_CALLBACK_FAILED);
		return;
	}
			
	
	
	RETURN_LONG(WING_CALLBACK_SUCCESS);
}
/**
 *@get data from create process
 *@从父进程获取数据
 *return string or null
 */
ZEND_FUNCTION(wing_get_process_params){
			HANDLE m_hRead = GetStdHandle(STD_INPUT_HANDLE);

			DWORD data_len=1024;
			int step = 1024;
			char *buf=new char[data_len];memory_add("add memory wing_get_process_params 331\r\n");

			memset(buf,0,sizeof(buf));
			DWORD dwRead;
			DWORD lBytesRead;
	
			if(!PeekNamedPipe(m_hRead,buf,data_len,&lBytesRead,0,0)){
				RETURN_NULL();
				return;
			}

			if(lBytesRead<=0){
				RETURN_NULL();
				return;
			}

			while(lBytesRead>=data_len){
				free(buf);memory_sub("sub memory wing_get_process_params 349\r\n");

				data_len+=step;
				buf = new char[data_len];memory_add("add memory wing_get_process_params 353\r\n");
				memset(buf,0,sizeof(buf));
				if(!PeekNamedPipe(m_hRead,buf,data_len,&lBytesRead,0,0)){
					RETURN_NULL();
					return;
				}
			}
				
			if (ReadFile(m_hRead, buf, lBytesRead+1, &dwRead, NULL))// 从管道中读取数据 
			{
				ZVAL_STRINGL(return_value,buf,dwRead,1);
				free(buf);memory_sub("sub memory wing_get_process_params 363\r\n");
				return;
			}
			RETURN_NULL();
}

/**
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION(wing_create_process_ex){
	char *params	= "";
	int	params_len	= 0;
	char *params_ex	= "";
	int params_ex_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params,&params_len,&params_ex,&params_ex_len) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	
	char				*command;
	spprintf(&command, 0, "%s %s\0",PHP_PATH,params);

	RETURN_LONG(create_process(command,params_ex,params_ex_len));	
}


/**
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION(wing_create_process){
	char				*exe;
	int					exe_len;
	char				*params="";
	int					params_len=0;
	char *params_ex="";
	int params_ex_len=0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &exe,&exe_len,&params,&params_len,&params_ex,&params_ex_len) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	char				*command;
	spprintf(&command, 0, "%s %s\0",exe,params);

	RETURN_LONG(create_process(command,params_ex,params_ex_len));	
}
/**
 *@杀死进程
 */
ZEND_FUNCTION(wing_process_kill)
{
	long process_id;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if(hProcess==NULL){
		RETURN_LONG(WING_ERROR_PROCESS_NOT_EXISTS);
        return;
	}
    if(!TerminateProcess(hProcess,0)){
		RETURN_LONG(WING_ERROR_FAILED);
        return;
	}
    RETURN_LONG(WING_SUCCESS);
}

/**
 *@获取当前进程id
 */
ZEND_FUNCTION(wing_get_current_process_id){
	ZVAL_LONG(return_value,GetCurrentProcessId());
}

/**
 *@return 0程序正在运行 -1 获取参数错误 -2 参数不能为空 -3创建互斥锁失败 long handle创建互斥锁成功  
 */
ZEND_FUNCTION(wing_create_mutex){
	char *mutex_name;
	int mutex_name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&mutex_name,&mutex_name_len) != SUCCESS){
		//zend_error(E_COMPILE_WARNING,"get params error");
		//RETURN_BOOL(0);
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	if(mutex_name_len<=0){
		//zend_error(E_COMPILE_WARNING,"mutex name must not empty string");
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	/*SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;*/

	HANDLE m_hMutex =  CreateMutex(NULL,TRUE,mutex_name);//CreateMutex(&sa,TRUE,mutex_name);
    DWORD dwRet = GetLastError();
    if (m_hMutex)
    {
        if (ERROR_ALREADY_EXISTS == dwRet)
        {
            //printf("程序已经在运行中了,程序退出!\n");
            CloseHandle(m_hMutex);
			RETURN_LONG(ERROR_ALREADY_EXISTS);
            return;
		}else{
			RETURN_LONG((long)m_hMutex);
		}
    }
   
    // printf("创建互斥量错误,程序退出!\n");
    CloseHandle(m_hMutex);
	//zend_error(E_COMPILE_ERROR,"mutex create error");
	RETURN_LONG(WING_ERROR_FAILED);
}
/**
 *@关闭互斥量
 */
ZEND_FUNCTION(wing_close_mutex){
	long mutex_handle;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&mutex_handle) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	if(mutex_handle<=0){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	RETURN_LONG(CloseHandle((HANDLE)mutex_handle)?WING_SUCCESS:WING_ERROR_FAILED);
}
/**
 *@检测进程是否存活--实际意义不大，因为进程id重用的特性 进程退出后 同样的进程id可能立刻被重用
 */
ZEND_FUNCTION(wing_process_isalive)
{
	long process_id;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if(hProcess==NULL){
		RETURN_LONG(WING_ERROR_PROCESS_NOT_EXISTS);
        return;
	}
	RETURN_LONG(WING_PROCESS_IS_RUNNING);
}

/**
 *@获取环境变量
 *@--test ok
 */
ZEND_FUNCTION(wing_get_env){
	zend_printf("wing_get_env\r\n");
	char *name;
	//zval *temp;
	int name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len) != SUCCESS){
		RETURN_NULL();
		return;
	}
	int len = GetEnvironmentVariable(name,NULL,0);
	if(len<=0){
		RETURN_NULL();
		return;
	}
	char *var=new char[len];memory_add("add memory wing_get_env 536\r\n");
	memset(var,0,sizeof(var));
	GetEnvironmentVariable(name,var,len);
	ZVAL_STRINGL(return_value,var,len-1,1);
	free(var);memory_sub("sub memory wing_get_env 541\r\n");
}

/**
 *@设置环境变量
 */
ZEND_FUNCTION(wing_set_env){
	char *name;
	zval *value;
	int name_len;
	MAKE_STD_ZVAL(value);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&name,&name_len,&value) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_string(value);
	//char *str;
	//spprintf(&str,Z_STRLEN_P(value)-2,"%s",Z_STRVAL_P(value));
	RETURN_LONG(SetEnvironmentVariableA(name,(LPCTSTR)Z_STRVAL_P(value)));
}
/**
 *@获取一个命令所在的绝对文件路径
 */
ZEND_FUNCTION(wing_get_command_path){ 
	char *name;
	int name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len) != SUCCESS){
		RETURN_NULL();
		return;
	}
	char path[MAX_PATH];
	get_command_path((const char*)name,path);
	RETURN_STRING(path,1);
}
/**
 *@通过WM_COPYDATA发送进程间消息 只能发给窗口程序
 */
ZEND_FUNCTION(wing_send_msg){
	zval *console_title;
	zval *message_id;
	zval *message;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzz",&console_title,&message_id,&message) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_cstring(console_title);
	convert_to_long(message_id);
	convert_to_cstring(message);

	HWND hwnd=FindWindow (Z_STRVAL_P(console_title),NULL);
	if(hwnd==NULL){
		RETURN_LONG(WING_ERROR_WINDOW_NOT_FOUND);
		return;
	}
	//WM_USER
	COPYDATASTRUCT CopyData; 
	CopyData.dwData	= Z_LVAL_P(message_id);  
	CopyData.cbData	= Z_STRLEN_P(message);  
	CopyData.lpData = Z_STRVAL_P(message);  //WM_COPYDATA
	SendMessageA(hwnd,WM_COPYDATA,NULL,(LPARAM)&CopyData);
	long status = GetLastError()==0 ? WING_SUCCESS:WING_ERROR_FAILED;
	RETURN_LONG(status);
}


/****
 *@窗口过程
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		ExitProcess(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
/**
 *@创建一个窗口 纯属测试
 */
ZEND_FUNCTION(wing_create_window){
	zval *console_title;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&console_title) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_cstring(console_title);
	
	HINSTANCE hInstance;
	WNDCLASSEX wcex;

	
	hInstance=GetModuleHandle(NULL);
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_WIN32PROJECT1);
	wcex.lpszClassName	= Z_STRVAL_P(console_title);
	wcex.hIconSm		= NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	HWND hWnd=CreateWindowA(Z_STRVAL_P(console_title),Z_STRVAL_P(console_title), WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, 1);
    UpdateWindow(hWnd);
	RETURN_LONG((long)hWnd);
}
/**
 *@销毁一个窗口
 */
ZEND_FUNCTION(wing_destory_window){
	long hwnd;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&hwnd) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	long  status = DestroyWindow((HWND)hwnd)?WING_SUCCESS:WING_ERROR_FAILED;
	RETURN_BOOL(status);
}
/**
 *@启用消息循环 创建窗口必用 阻塞
 */
ZEND_FUNCTION(wing_message_loop){
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
/**
 *@简单的消息弹窗
 */
ZEND_FUNCTION(wing_message_box){
	char *content;
	int c_len,t_len;
	char *title;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&content,&c_len,&title,&t_len) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	MessageBox(0,content,title,0);
	RETURN_NULL();
}
/**
 *@获取最后发生的错误
 */
ZEND_FUNCTION(wing_get_last_error){
	RETURN_LONG(GetLastError());
}



/****
 *@毫秒级别定时器
 *@author yuyi
 *@created 2016-05-15
 */
ZEND_FUNCTION(wing_timer){
	wing_timer_count++;
	zval *callback;
	zval *dwMilliseconds;
	MAKE_STD_ZVAL(callback);
	MAKE_STD_ZVAL(dwMilliseconds);

	int max_run_times = 0;
	//设置相对时间为1秒 1000 0000。
	long accuracy = 10000;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|lll",&dwMilliseconds, &callback,&max_run_times,&accuracy) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long_ex(&dwMilliseconds);

	char *command_params	= "";
	int run_process			= 0;
	int command_index		= 0;
	int last_value			= 0;
	
	command_params_check(command_params,&run_process,&last_value);

	char	*command;
	 
	
	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_timer_count);

	if(!run_process){
		RETURN_LONG(create_process(command,NULL,0));	
		return;
	}

	if(wing_timer_count!=last_value){
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}

	/*
	timer_thread_params *param	=  timer_thread_params();
	param->dwMilliseconds		= (DWORD)Z_DVAL_P(dwMilliseconds);
	param->thread_id			= GetCurrentThreadId();
	_beginthreadex(NULL, 0, wing_timer, param, 0, NULL); 

	
	MSG msg;
	BOOL bRet;
	zval *retval_ptr;
	int times=0;//param->times;
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			free(param);memory_sub();
			RETURN_LONG(times);	
			return;
		}

		if(msg.message == (WM_USER+99)){
			
			MAKE_STD_ZVAL(retval_ptr);
			if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
				free(param);memory_sub();
				RETURN_LONG(times);	
				return;
			}
			times++;
			
			zval_ptr_dtor(&retval_ptr);

			if(times>=max_run_times&&max_run_times>0)break;

		}

    } 

	
	
	free(param);memory_sub();
	RETURN_LONG(times);	
	return;*/


	HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;
	zval *retval_ptr;
	int times=0;

	LONGLONG time = (-1)*accuracy*Z_LVAL_P(dwMilliseconds);

    //设置相对时间为1秒 10000000。
	liDueTime.QuadPart = time;
	char *timername;
	spprintf(&timername,0,"wing_waitable_timer-%s",create_guid());
    //创建定时器。
    hTimer = CreateWaitableTimer(NULL, TRUE, timername);
    if(!hTimer)
    {       
		RETURN_LONG(WING_ERROR_FAILED);	
		return;
    }
 
    if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
    {         
               CloseHandle(hTimer);
			   RETURN_LONG(WING_ERROR_FAILED);	
               return;
    }
 
    //等定时器有信号。
	while(true)
	{
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			CloseHandle(hTimer);
			RETURN_LONG(WING_ERROR_FAILED);	
			return;
		}
		else
		{
			//时钟到达。
            //SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);//将hTimer信息重置为无信号，如果不然就会不断的输出

			MAKE_STD_ZVAL(retval_ptr);
			if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
				//zval_ptr_dtor(&retval_ptr);
				RETURN_LONG(WING_ERROR_FAILED);	
				return;
			}
			//times++;
			
			zval_ptr_dtor(&retval_ptr);

			if(max_run_times>0){
				times++;
				if(times>=max_run_times)break;
			}


			 if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
			 {         
               CloseHandle(hTimer);
			   RETURN_LONG(WING_ERROR_FAILED);	
               return;
			 }

		}  
	}
	SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
    CloseHandle(hTimer);
	RETURN_LONG(WING_SUCCESS);	
}

///////////////////////////--socket-start--


unsigned int __stdcall  socket_worker(LPVOID lpParams)  
{  
	COMPARAMS *params = (COMPARAMS *)lpParams;
	HANDLE ComplectionPort = params->IOCompletionPort;  
	DWORD thread_id = params->threadid;
	DWORD BytesTransferred;  
	//LPOVERLAPPED Overlapped;  
	LPPER_HANDLE_DATA PerHandleData;  
	LPPER_IO_OPERATION_DATA PerIOData;  
	DWORD RecvBytes;  
	DWORD Flags;

//	queue_t *message_queue = params->message_queue;

	while (TRUE)  
	{  
		BOOL wstatus = GetQueuedCompletionStatus(ComplectionPort,&BytesTransferred,(LPDWORD)&PerHandleData,(LPOVERLAPPED*)&PerIOData,INFINITE);
		
		if( BytesTransferred==-1 && PerIOData==NULL )   
        { 
		
			elemType *msg= new elemType();
			memory_add();

			msg->message_id = WM_ONQUIT;
			msg->wparam = 0;
			msg->lparam = 0;
			enQueue(message_queue,msg);
			
			closesocket(PerHandleData->Socket);
			
			free(PerHandleData);
			free(PerIOData);
			
			memory_sub();
			memory_sub();
			
			return 0;  
        }  

		//首先检查套接字上是否发生错误，如果发生了则关闭套接字并且清除同套节字相关的SOCKET_INFORATION 结构体  

		if (BytesTransferred == 0 || 10054 == WSAGetLastError()) 
		{   
			elemType *msg=new elemType();
			memory_add();

			msg->message_id = WM_ONCLOSE;
			msg->wparam = (unsigned long)PerHandleData->Socket;
			msg->lparam = 0;
			enQueue(message_queue,msg);

			free(PerIOData);
			free(PerHandleData);

			memory_sub();
			memory_sub();

			memory_accept_sub_times();
			memory_accept_sub_times();
			continue;  
		}

		if(PerIOData->type==OPE_RECV)  {

					
						RECV_MSG *recv_msg	= new RECV_MSG();
						recv_msg->msg		= new char[BytesTransferred+1];

						_recv_add_times();
									
						memset(recv_msg->msg,0,sizeof(recv_msg->msg));
						strcpy(recv_msg->msg,PerIOData->Buffer);
						recv_msg->len		=  BytesTransferred;

						memory_add();
						memory_add();
	
					
						elemType *msg=new elemType();
						memory_add();

						msg->message_id = WM_ONRECV;
						msg->wparam		= (unsigned long)PerHandleData->Socket;
						msg->lparam		= (unsigned long)recv_msg;
						enQueue(message_queue,msg);

						BytesTransferred = 0;
						Flags = 0;  
						ZeroMemory(&(PerIOData->OVerlapped),sizeof(OVERLAPPED)); 
						memset(PerIOData->Buffer,0,DATA_BUFSIZE);  

						PerIOData->DATABuf.buf = PerIOData->Buffer;  
						PerIOData->DATABuf.len = DATA_BUFSIZE;  
						PerIOData->type = 1;

						int code = WSARecv(PerHandleData->Socket,&(PerIOData->DATABuf),1,&RecvBytes,&Flags,&(PerIOData->OVerlapped),NULL);
						if( 0 != code && WSAGetLastError() != WSA_IO_PENDING){

							elemType *msg=new elemType();
							memory_add();

							msg->message_id = WM_ONCLOSE;
							msg->wparam = (unsigned long)PerHandleData->Socket;
							msg->lparam = 0;
							enQueue(message_queue,msg);


							free(PerIOData);
							memory_sub();

							free(PerHandleData);
							memory_sub();
							
							memory_accept_sub_times();
							memory_accept_sub_times();
						}
				
		}

		else if(PerIOData->type == OPE_SEND)  
		{  
			memset(PerIOData,0,sizeof(PER_IO_OPERATION_DATA));  
			free(PerIOData); 

			memory_sub();

			_send_msg_sub_times();

			BytesTransferred = 0;
		}  
			
	}  
	return 0;
}  



unsigned int __stdcall  accept_worker(LPVOID _socket) {
	
	COMPARAMS *_data			= (COMPARAMS*)_socket;
	SOCKET m_sockListen			= _data->Socket;
	HANDLE m_hIOCompletionPort	= _data->IOCompletionPort;
	DWORD threadid				= _data->threadid;

	SOCKET accept ;
	PER_HANDLE_DATA *PerHandleData;
	LPPER_IO_OPERATION_DATA PerIOData;
	DWORD RecvBytes;  
    DWORD Flags; 

	//queue_t *message_queue = _data->message_queue;
	
	while(true){

		 accept = WSAAccept(m_sockListen,NULL,NULL,NULL,0);
		 if( INVALID_SOCKET == accept){

			elemType *msg=new elemType();
			memory_add();

			msg->message_id = WM_ACCEPT_ERROR;
			msg->wparam = 0; 
			msg->lparam = 0; 

			enQueue(message_queue,msg);
			continue;
		 }

/* unsigned long dwBytes = 0;
		 DWORD dwError = 0L ;
tcp_keepalive sKA_Settings = {0}, sReturned = {0} ;
sKA_Settings.onoff = 1 ;
sKA_Settings.keepalivetime = 1 ; // Keep Alive in 5.5 sec.
sKA_Settings.keepaliveinterval = 1 ; // Resend if No-Reply 
if (WSAIoctl(accept, SIO_KEEPALIVE_VALS, &sKA_Settings,
sizeof(sKA_Settings), &sReturned, sizeof(sReturned),&dwBytes,
NULL, NULL) != 0)
{
//dwError = WSAGetLastError() ;
}
*/



						

		elemType *msg=new elemType();
		memory_add();
		msg->message_id = WM_ONCONNECT;
		msg->wparam = (unsigned long)accept;
		msg->lparam = 0;
		enQueue(message_queue,msg);


		_accept_add_times_ex();
					
		 PerHandleData =new PER_HANDLE_DATA();
		 memory_add();
		 memory_accept_add_times();
		 
		 PerHandleData->Socket = accept;
	
		 if( NULL == CreateIoCompletionPort ((HANDLE )accept ,m_hIOCompletionPort , (ULONG_PTR)PerHandleData ,0)){
			
			elemType *msg=new elemType();
			memory_add("add memory elemType 1029\r\n");

			msg->message_id = WM_ONCLOSE;
			msg->wparam = (unsigned long)PerHandleData->Socket;
			msg->lparam = 0;
			enQueue(message_queue,msg);

			free(PerHandleData);
			memory_sub();
			memory_accept_sub_times();
				
			continue;
		 }

		PerIOData = new PER_IO_OPERATION_DATA();
		memory_add();

		if ( PerIOData == NULL )
		{
			elemType *msg=new elemType();
			memory_add();

			msg->message_id = WM_ONCLOSE;
			msg->wparam = (unsigned long)PerHandleData->Socket;
			msg->lparam = 0;
			enQueue(message_queue,msg);

			free(PerHandleData);
			memory_sub();
			memory_accept_sub_times();

			continue;
		}else{
			memory_accept_add_times();
		} 

		ZeroMemory(&(PerIOData->OVerlapped),sizeof(OVERLAPPED)); 

		PerIOData->DATABuf.len = DATA_BUFSIZE; 
		PerIOData->DATABuf.buf = PerIOData->Buffer; 
		PerIOData->type = 1;
		Flags = 0; 

		int code = WSARecv(accept,&(PerIOData->DATABuf),1,&RecvBytes,&Flags,&(PerIOData->OVerlapped),NULL);
		if(0 != code && WSAGetLastError() != WSA_IO_PENDING){
			elemType *msg=new elemType();
			memory_add();

			msg->message_id = WM_ONCLOSE;
			msg->wparam = (unsigned long)PerHandleData->Socket;
			msg->lparam = 0;
			enQueue(message_queue,msg);

			free(PerHandleData);
			memory_sub();
			memory_accept_sub_times();

			free(PerIOData);
			memory_sub();
			memory_accept_sub_times();
		}
	}
	return 0;
}

//CRITICAL_SECTION g_cs;
ZEND_FUNCTION(wing_service){

	zval *onreceive;
	zval *onconnect;
	zval *onclose;
	zval *onerror;
	zval *_params;
	int port;
	char *listen_ip;

	MAKE_STD_ZVAL(onreceive);
	MAKE_STD_ZVAL(onconnect);
	MAKE_STD_ZVAL(onclose);
	MAKE_STD_ZVAL(onerror);


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&_params) != SUCCESS) {
		
			RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
			return;
	}

	/*if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"zzzz",
		&onreceive, 
		&onconnect,
		&onclose,
		&onerror
		) != SUCCESS) {
		
			RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
			return;
	}*/

	//zend_hash_find();
	if(Z_TYPE_P(_params) != IS_ARRAY){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
			return;
	}




	int argc;
	HashTable *arr_hash;

		zval  **data;
		HashPosition pointer;
		arr_hash	= Z_ARRVAL_P(_params);
		argc		= zend_hash_num_elements(arr_hash);
	
		for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; zend_hash_move_forward_ex(arr_hash, &pointer)) {

            char *key;
            int key_len;
            long index;

            if (zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer) == HASH_KEY_IS_STRING) {

				if(strcmp(key,"port")==0){
					port = Z_LVAL_PP(data);
				}else if(strcmp(key,"listen")==0){
					listen_ip = Z_STRVAL_PP(data);
				}else if(strcmp(key,"onreceive")==0){
					onreceive = *data;
				}else if(strcmp(key,"onconnect")==0){
					onconnect = *data;
				}else if(strcmp(key,"onclose")==0){
					onclose = *data;
				}else if(strcmp(key,"onerror")==0){
					onerror = *data;
				}

            } 
        } 


	 message_queue=new queue_t();  

     initQueue(message_queue);  

	//1、创建完成端口
	HANDLE m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 ); 
	if(m_hIOCompletionPort == NULL){
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	DWORD thread_id					= GetCurrentThreadId();
	COMPARAMS *accept_params		= new COMPARAMS();

	accept_params->threadid			= thread_id;
	accept_params->IOCompletionPort = m_hIOCompletionPort;
	//accept_params->message_queue	= message_queue;

	//2、根据cpu数量创建工作线程
	SYSTEM_INFO si; 
	GetSystemInfo(&si); 
	int m_nProcessors = si.dwNumberOfProcessors; 
	int m_nThreads = 2 * m_nProcessors; 
	for (int i = 0; i < m_nThreads; i++) 
	{ 
		 _beginthreadex(NULL, 0, socket_worker, accept_params, 0, NULL); 
	} 


	// 初始化Socket库
	WSADATA wsaData; 
	if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0){
		free(accept_params);memory_sub("sub memory wing_service 1180\r\n");
		RETURN_LONG(WING_ERROR_FAILED);
		return; 
	}
	
	//WSACleanup( );
	//初始化Socket
	// 这里需要特别注意，如果要使用重叠I/O的话，这里必须要使用WSASocket来初始化Socket
	// 注意里面有个WSA_FLAG_OVERLAPPED参数
	SOCKET m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if(m_sockListen == INVALID_SOCKET){
		WSACleanup();
		free(accept_params);memory_sub("sub memory wing_service 1192\r\n");
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}


	struct sockaddr_in ServerAddress; 
	// 填充地址结构信息
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress)); 
	ServerAddress.sin_family = AF_INET; 
	// 这里可以选择绑定任何一个可用的地址，或者是自己指定的一个IP地址
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                     
	ServerAddress.sin_addr.s_addr = inet_addr(listen_ip);          
	ServerAddress.sin_port = htons(port);   


	// 绑定端口
	if (SOCKET_ERROR == bind(m_sockListen, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))){
		closesocket(m_sockListen);
		WSACleanup();
		free(accept_params);memory_sub("sub memory wing_service 1212\r\n");
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}  

	// 开始监听
	if( 0 != listen(m_sockListen,SOMAXCONN)){
		closesocket(m_sockListen);
		WSACleanup();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	
	accept_params->Socket = m_sockListen;
	//客户端连接进来的处理线程
	 _beginthreadex(NULL, 0, accept_worker, accept_params/*参数*/, 0, NULL);

	//MSG msg;
	//BOOL bRet;
	int times=0;
	int nSize =0;
	elemType *msg=NULL;//=NULL;
	


	//InitializeCriticalSection(&g_cs);//DeleteCriticalSection(&g_cs);
	while( true/*( bRet = GetMessage( &msg, NULL, 0, 0 )) != 0*/)
	{ 
		/*if (bRet == -1)
		{
			free(accept_params);memory_sub("sub memory wing_service 1242\r\n");
			//zend_printf("GetMessage error\r\n");
			::MessageBoxA(0,"GetMessage error\r\n","quit",0);
			RETURN_LONG(WING_ERROR_FAILED);	
			return;
		}*/
		if(is_emptyQueue(message_queue)){
			Sleep(100);
			continue;
		}

		//zend_printf("start...\r\n");

		
		//zend_printf("start...\r\n");

		//  outQueue(message_queue,msg);


	
    if (message_queue->head == NULL)  
    {  
		continue;
    }  
	EnterCriticalSection(&queue_lock);
	node_t * _temp_node;  
    msg			= message_queue->head->data;  
    _temp_node	= message_queue->head;  
    message_queue->head = message_queue->head->next;  
    if(message_queue->head == NULL)  
    {  
        message_queue->tail = NULL;  
    }  
    free(_temp_node);memory_sub("sub memory outQueue 59\r\n"); 
	_node_sub_times();

	
	LeaveCriticalSection(&queue_lock);

	//zend_printf("last error:%ld\r\n",WSAGetLastError());

		//  if(msg==NULL){
			//  zend_printf("null...\r\n");
			//  continue;
		//  }

		//  if(msg == NULL) continue;
		 
		 

		//if(NULL == msg) continue;

		//zend_printf("message:%ld\r\n",msg->message_id);

		switch(msg->message_id){
			case WM_ONCONNECT:
			{
			//	zend_printf("onconnect\r\n");
				zval *params;
				zval *retval_ptr;
				MAKE_STD_ZVAL(params);
				//ZVAL_LONG(params,(long)msg.wParam);
				ZVAL_LONG(params,(long)msg->wparam);
				MAKE_STD_ZVAL(retval_ptr);

				//zend_printf("onconnect\r\n");
							 
				if( SUCCESS != call_user_function(EG(function_table),NULL,onconnect,retval_ptr,1,&params TSRMLS_CC) ){
					zval_ptr_dtor(&retval_ptr);
					zval_ptr_dtor(&params);
					zend_error(E_USER_WARNING,"WM_ONCONNECT call_user_function fail\r\n");
					continue;
				}
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);
						  
			}
			break;
			case WM_ACCEPT_ERROR:
			{
			//	zend_printf("accept error\r\n");
				//accept error
			}
			break;
			case WM_ONRECV:
			{
				
			//	zend_printf("onrecv\r\n");
				//EnterCriticalSection(&g_cs);
				
				//RECV_MSG *temp		= (RECV_MSG*)msg.lParam;
				//SOCKET sClient		= (SOCKET)msg.wParam;

				RECV_MSG *temp		= (RECV_MSG*)msg->lparam;
				SOCKET sClient		= (SOCKET)msg->wparam;

				zval *params[2];
				zval *retval_ptr;

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				ZVAL_LONG(params[0],(long)sClient);
				//ZVAL_STRING(params[1],temp->msg,1); 
				ZVAL_STRINGL(params[1],temp->msg,temp->len,1);

				MAKE_STD_ZVAL(retval_ptr);
				

				if(SUCCESS!=call_user_function(EG(function_table),NULL,onreceive,retval_ptr,2,params TSRMLS_CC)){
					zval_ptr_dtor(&retval_ptr);
					zval_ptr_dtor(&params[0]);
					zval_ptr_dtor(&params[1]);
				
					free(temp->msg);_recv_sub_times();memory_sub("sub memory wing_service 1319\r\n");
					free(temp);_recv_sub_times();memory_sub("sub memory wing_service 1321\r\n");
		
					zend_error(E_USER_WARNING,"call_user_function fail\r\n");
					continue;
				}
				//zend_printf("recv=>%s\n\n",temp->msg);
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				
				free(temp->msg);_recv_sub_times();memory_sub("sub memory wing_service 1332\r\n");
				free(temp);_recv_sub_times();memory_sub("sub memory wing_service 1334\r\n");

				//LeaveCriticalSection(&g_cs);
			}
			break;
			case WM_ONCLOSE_EX:
			{
			//	zend_printf("onclose ex\r\n");
				zval *params;
				zval *retval_ptr;
				MAKE_STD_ZVAL(params);
				//ZVAL_LONG(params,(long)msg.wParam);
				ZVAL_LONG(params,(long)msg->wparam);

				MAKE_STD_ZVAL(retval_ptr);
							 
				if(SUCCESS != call_user_function(EG(function_table),NULL,onclose,retval_ptr,1,&params TSRMLS_CC)){
					zval_ptr_dtor(&retval_ptr);
					zval_ptr_dtor(&params);
					//closesocket((SOCKET)msg.wParam);
					closesocket((SOCKET)msg->wparam);
					zend_error(E_USER_WARNING,"WM_ONCLOSE_EX call_user_function fail\r\n");
					continue;
				}
				
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);
				//closesocket((SOCKET)msg.wParam);
				closesocket((SOCKET)msg->wparam);

			}
			break;
			case WM_ONCLOSE:
			{
				zend_printf("1-onclose\r\n");
				_accept_sub_times_ex();

				SOCKET client =(SOCKET)msg->wparam;

				zval *params;
				zval *retval_ptr;
				MAKE_STD_ZVAL(params);
				ZVAL_LONG(params,(long)client);
				MAKE_STD_ZVAL(retval_ptr);
	 
				if(SUCCESS != call_user_function(EG(function_table),NULL,onclose,retval_ptr,1,&params TSRMLS_CC)){
					zval_ptr_dtor(&retval_ptr);
					zval_ptr_dtor(&params);
				
					closesocket(client);
					zend_error(E_USER_WARNING,"WM_ONCLOSE call_user_function fail\r\n");
					continue;
				}
							 
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);

				closesocket(client);
	
				closesocket(client);
				zend_printf("onclose\r\n");

			}
			break;
			case WM_ONERROR:{
				
				SOCKET client =(SOCKET)msg->wparam;

				zval *params[2];
				zval *retval_ptr;
				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);

				ZVAL_LONG(params[0],(long)client);
				//ZVAL_STRING(params[1],(char*)msg.lParam,1);
				ZVAL_STRING(params[1],(char*)msg->lparam,1);

				MAKE_STD_ZVAL(retval_ptr);
							 
				if(SUCCESS != call_user_function(EG(function_table),NULL,onerror,retval_ptr,2,params TSRMLS_CC)){
					zval_ptr_dtor(&retval_ptr);
					zval_ptr_dtor(&params[0]);
					zval_ptr_dtor(&params[1]);
					zend_error(E_USER_WARNING,"WM_ONERROR call_user_function fail\r\n");
					continue;
				}
							 
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);

			}
			break;
			case WM_ONQUIT:
			{
			//	zend_printf("quit\r\n");
				PostQueuedCompletionStatus(m_hIOCompletionPort, 0xFFFFFFFF, 0, NULL);
				CloseHandle(m_hIOCompletionPort);
				closesocket(m_sockListen);
				WSACleanup();
				clearQueue(message_queue);
				free(msg);memory_sub();
				free(message_queue);memory_sub("sub memory wing_service 1439\r\n");
				DeleteCriticalSection(&queue_lock);
				//zend_printf("1=>service quit\r\n");
				RETURN_LONG(WING_SUCCESS);

				////zend_printf("1=>service quit\r\n");
				return;
			}break;

		}
		//if(msg!=NULL){
			free(msg);memory_sub("sub memory elemType 1395\r\n");
			_queue_sub_times();
			
		//}
			//zend_printf("showshow\r\n");
		 memory_times_show();

    } 
	//zend_printf("service quit\r\n");
	//::MessageBoxA(0,"service quit\r\n","quit",0);
	RETURN_LONG(WING_SUCCESS);
}

ZEND_FUNCTION(wing_service_stop){
	/*if(!PostThreadMessageA(GetCurrentThreadId(),WM_ONQUIT,NULL,NULL)){
		::MessageBoxA(0,"PostThreadMessageA error","error",0);
	}*/
	 elemType *msg=new elemType();memory_add("add memory elemType 1458\r\n");
						msg->message_id = WM_ONQUIT;
						msg->wparam = 0;// (unsigned long)PerHandleData;
						msg->lparam = 0;//(unsigned long)recv_msg;
						enQueue(message_queue,msg);
	RETURN_LONG(WING_SUCCESS);
}

/********************************************
 * @ 关闭socket
 * @ param socket
 ********************************************/
ZEND_FUNCTION(wing_close_socket){
	zval *socket;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long(socket);
	/*if(!PostThreadMessageA(GetCurrentThreadId(),WM_ONCLOSE_EX,(WPARAM)Z_LVAL_P(socket),NULL)){
		::MessageBoxA(0,"PostThreadMessageA error","error",0);
	}*/
	 elemType *msg=new elemType();memory_add("add memory elemType 1481 \r\n");
						msg->message_id = WM_ONCLOSE_EX;
						msg->wparam =  (unsigned long)Z_LVAL_P(socket);
						msg->lparam = 0;//(unsigned long)recv_msg;
						enQueue(message_queue,msg);
	RETURN_LONG(WING_SUCCESS);
}

/*****************************************
 *@获取socket信息，ip 协议 端口 等
 ****************************************/
ZEND_FUNCTION(wing_socket_info){

	zval *socket;
	MAKE_STD_ZVAL(socket);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	convert_to_long(socket);
	SOCKADDR_IN addr_conn;  
	int nSize = sizeof(addr_conn);  
	
	memset((void *)&addr_conn,0,sizeof(addr_conn));  
	getpeername((SOCKET)Z_LVAL_P(socket),(SOCKADDR *)&addr_conn,&nSize);  
  
	array_init(return_value);
	add_assoc_string(return_value,"sin_addr",inet_ntoa(addr_conn.sin_addr),1);
    add_assoc_long(return_value,"sin_family",addr_conn.sin_family);
	add_assoc_long(return_value,"sin_port",addr_conn.sin_port);
	add_assoc_string(return_value,"sin_zero",addr_conn.sin_zero,1);
}
/*****************************************
 * @ 发送socket数据
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

	send((SOCKET)Z_LVAL_P(socket),Z_STRVAL_P(msg),Z_STRLEN_P(msg),0);
	RETURN_LONG(WING_SUCCESS);
	return;
	/*SOCKET sClient		= (SOCKET)Z_LVAL_P(socket);

	//send((SOCKET)Z_LVAL_P(socket),Z_STRVAL_P(msg),Z_STRLEN_P(msg),);

	SOCKET sClient		= (SOCKET)Z_LVAL_P(socket);
	send(sClient, Z_STRVAL_P(msg),Z_STRLEN_P(msg),0);
	RETURN_LONG(WING_SUCCESS);
	return;


	char *pData			= Z_STRVAL_P(msg);
	ulong Length		= Z_STRLEN_P(msg);
	unsigned long  Flag	= 0;  
	DWORD SendByte		= 0;  

    if (sClient==INVALID_SOCKET||pData==NULL||Length==0){
		RETURN_LONG(WING_ERROR_FAILED);
		return;  
	}
   
	LPPER_IO_OPERATION_DATA  PerIoData=new PER_IO_OPERATION_DATA();//(LPPER_IO_OPERATION_DATA) GlobalAlloc(GPTR,sizeof(PER_IO_OPERATION_DATA));//new PER_IO_OPERATION_DATA();
	
	_send_msg_add_times();
	memory_add("add memory wing_socket_send_msg 1544 \r\n");

	ZeroMemory(&(PerIoData->OVerlapped),sizeof(OVERLAPPED));      
	PerIoData->DATABuf.buf	= pData; 
	PerIoData->DATABuf.len	= Length; 
	PerIoData->type			= OPE_SEND;
	
	int bRet=WSASend(sClient,&(PerIoData->DATABuf),1,&SendByte,Flag,&(PerIoData->OVerlapped),NULL);  
	if(bRet == 0){
		///free(PerIoData);_send_msg_sub_times();
		///memory_sub();
		RETURN_LONG(WING_SUCCESS);
		return;
	}
	if( WSAGetLastError()!=WSA_IO_PENDING)  
	{  
		free(PerIoData);_send_msg_sub_times();
		memory_sub();

		RETURN_LONG(WING_ERROR_FAILED);
		return;
	} 
	RETURN_LONG(WING_SUCCESS);*/
}  
  

//////////////////////////--socket-end--

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
	//注册常量或者类等初始化操作
	//REGISTER_STRING_CONSTANT("WING_VERSION",PHP_WING_VERSION,CONST_CS | CONST_PERSISTENT);
	zend_register_string_constant("WING_VERSION", sizeof("WING_VERSION"), PHP_WING_VERSION,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	
	PHP_PATH =new char[MAX_PATH];  

	//memory_add("add memory PHP_MINIT_FUNCTION 1596 \r\n");
	if(GetModuleFileName(NULL,PHP_PATH,MAX_PATH) != 0){
		zend_register_string_constant("WING_PHP", sizeof("WING_PHP"), PHP_PATH,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	}


	
	//WAIT_TIMEOUT
	zend_register_long_constant("WING_WAIT_TIMEOUT", sizeof("WING_WAIT_TIMEOUT"), WAIT_TIMEOUT,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	//WAIT_FAILED
	zend_register_long_constant("WING_WAIT_FAILED", sizeof("WING_WAIT_FAILED"), WAIT_FAILED,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	//INFINITE
	zend_register_long_constant("WING_INFINITE", sizeof("WING_INFINITE"), INFINITE,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	//WAIT_OBJECT_0
	zend_register_long_constant("WING_WAIT_OBJECT_0", sizeof("WING_WAIT_OBJECT_0"), WAIT_OBJECT_0,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	//WAIT_ABANDONED
	zend_register_long_constant("WING_WAIT_ABANDONED", sizeof("WING_WAIT_ABANDONED"), WAIT_ABANDONED,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);


	zend_register_long_constant("WING_ERROR_ALREADY_EXISTS",sizeof("WING_ERROR_ALREADY_EXISTS"),ERROR_ALREADY_EXISTS,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_ERROR_PARAMETER_ERROR",sizeof("WING_ERROR_PARAMETER_ERROR"),WING_ERROR_PARAMETER_ERROR,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_ERROR_FAILED",sizeof("WING_ERROR_FAILED"),WING_ERROR_FAILED,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_ERROR_CALLBACK_FAILED",sizeof("WING_ERROR_CALLBACK_FAILED"),WING_ERROR_CALLBACK_FAILED,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_CALLBACK_SUCCESS",sizeof("WING_CALLBACK_SUCCESS"),WING_CALLBACK_SUCCESS,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_ERROR_PROCESS_NOT_EXISTS",sizeof("WING_ERROR_PROCESS_NOT_EXISTS"),WING_ERROR_PROCESS_NOT_EXISTS,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant("WING_SUCCESS",sizeof("WING_SUCCESS"),WING_SUCCESS,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);	
	zend_register_long_constant("WING_PROCESS_IS_RUNNING",sizeof("WING_PROCESS_IS_RUNNING"),WING_PROCESS_IS_RUNNING,CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);

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
	free(PHP_PATH);//memory_sub("sub memory PHP_MSHUTDOWN_FUNCTION 1637 \r\n");
	//memory_times_show();
	//efree(PHP_PATH);memory_sub();
	//DeleteCriticalSection(&g_cs);
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
	php_info_print_table_header(2, "wing support", "enabled");
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
	//PHP_FE(wing_qrdecode,NULL)
	//PHP_FE(wing_qrencode,NULL)
	//wing_create_thread
	PHP_FE(wing_create_thread,NULL)
	//wing_create_process
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
	PHP_FE(wing_create_window,NULL)
	PHP_FE(wing_message_loop,NULL)
	PHP_FE(wing_destory_window,NULL)
	PHP_FE(wing_message_box,NULL)
	PHP_FE(wing_timer,NULL)
	PHP_FE(wing_service,NULL)
	PHP_FE(wing_socket_info,NULL)
	PHP_FE(wing_socket_send_msg,NULL)
	PHP_FE(wing_service_stop,NULL)
	PHP_FE(wing_close_socket,NULL)
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
