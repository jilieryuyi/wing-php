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
char  *PHP_PATH = NULL;
//timer 进程计数器 用于控制多个timer的创建和运行
static int wing_timer_count = 0;
//线程计数器 用于多线程控制
static int wing_thread_count =0;

/****************************************************************************
 * @ 创建进程
 * @ param command 要在进程中执行的指令 
 * @ param params_ex 额外参数 用于传输给生成的子进程 默认为 NULL，即不传输参数
 * @ param params_ex_len 额外参数长度 默认为0，即参数为 params_ex = NULL
 ***************************************************************************/
unsigned long create_process(char *command,char *params_ex=NULL,int params_ex_len=0){
	    
	HANDLE m_hRead = NULL;
	HANDLE m_hWrite = NULL;
	STARTUPINFO sui;    
	PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		
	char *params = NULL;
	int params_len = 0;
    
	sa.bInheritHandle = TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	
	if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
	{
		return WING_ERROR_FAILED;
	}

	ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
	
	sui.cb = sizeof(STARTUPINFO);
	sui.dwFlags	= STARTF_USESTDHANDLES;  
	sui.hStdInput = m_hRead;
	sui.hStdOutput = m_hWrite;
	sui.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		
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
		
	CloseHandle(m_hRead);
	CloseHandle(m_hWrite);
	CloseHandle(pi.hProcess); // 子进程的进程句柄
	CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		
	return pi.dwProcessId;	
}

/********************************************************************************************************
 * @ 进程参数校验，该怎么说这个函数的作用呢，这么说吧，这个函数用来确认是否让阻塞的函数异步执行的
 ********************************************************************************************************/
void command_params_check(char* &command_params,int *run_process,int *last_value){
	
	TSRMLS_FETCH();
	zval **argv = NULL;
	int argc = 0;
	HashTable *arr_hash = NULL;
	
	//char *target = NULL;
	//获取命令行参数
	if ( zend_hash_find(&EG(symbol_table),"argv",sizeof("argv"),(void**)&argv) == SUCCESS ){
		zval  **data = NULL;
		HashPosition pointer = NULL;
		arr_hash	= Z_ARRVAL_PP(argv);
		argc		= zend_hash_num_elements(arr_hash);
		for( zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
			 zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
			 zend_hash_move_forward_ex(arr_hash, &pointer)
		) {
			
			if( strcmp((char*)Z_LVAL_PP(data),"wing-process") == 0 ){
				*run_process = 1;
			}

			char *key = NULL;
			int key_len = 0 ,index = 0;

			zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer);

			if( index > 0 ){
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
//-----------------------function--end---------------------------------------------------



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

/***************************************************************
 * @ 获取 wing php的版本号 或者使用常量 WING_VERSION                
 **************************************************************/
PHP_FUNCTION(wing_version){
	char *string = NULL;
    int len = spprintf(&string, 0, "%s", PHP_WING_VERSION);
    RETURN_STRING(string,0);
}


/***************************************************************
 *@wait process进程等待
 *@param process id 进程id
 *@param timeout 等待超时时间 单位毫秒
 *@return exit code 进程退出码
 ***************************************************************/
PHP_FUNCTION(wing_process_wait){
	
	int thread_id,timeout = INFINITE;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|l",&thread_id,&timeout)!=SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, thread_id);

	DWORD wait_result = 0;

	DWORD wait_status = WaitForSingleObject(handle,timeout);
	 
	 if( wait_status != WAIT_OBJECT_0 ){
		RETURN_LONG(wait_status);
		return;
	 }

	 //WING_WAIT_OBJECT_0
	 
	 if(GetExitCodeProcess(handle,&wait_result) == 0) 
		 RETURN_LONG(WING_ERROR_FAILED);

	 RETURN_LONG(wait_result);
}

/******************************************************************
 *@创建多线程，使用进程模拟
 *****************************************************************/
PHP_FUNCTION(wing_create_thread){
	
	wing_thread_count++;
	zval *callback = NULL;
	
	MAKE_STD_ZVAL(callback);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	char *command_params = NULL;
	int run_process = 0;
	int command_index = 0;
	int last_value = 0;
	char *command = NULL;

	command_params_check(command_params,&run_process,&last_value);

	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_thread_count);

	if(!run_process){
		unsigned long pid = create_process(command,NULL,0);
		efree(command);
		efree(command_params);
		RETURN_LONG(pid);	
		return;
	}

	if(wing_thread_count != last_value){
		efree(command);
		efree(command_params);
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}
	
	efree(command);
	efree(command_params);

	zval *retval_ptr = NULL;
			
	MAKE_STD_ZVAL(retval_ptr);
	if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
		
		RETURN_LONG(WING_ERROR_CALLBACK_FAILED);
		return;

	}
			
	RETURN_LONG(WING_CALLBACK_SUCCESS);
}
/*************************************************************************
 *@get data from create process
 *@从父进程获取数据 这里使用了一个小伎俩，性能，待考量
 *return string or null
 *************************************************************************/
ZEND_FUNCTION(wing_get_process_params){
			HANDLE m_hRead = GetStdHandle(STD_INPUT_HANDLE);

			DWORD data_len = 1024;
			int step = 1024;

			char *buf = new char[data_len];

			ZeroMemory(buf,sizeof(buf));
			DWORD dwRead;
			DWORD lBytesRead;
	
			if(!PeekNamedPipe(m_hRead,buf,data_len,&lBytesRead,0,0)){

				delete[] buf;
				buf = NULL;
				RETURN_NULL();
				return;
			}

			if(lBytesRead<=0){

				delete[] buf;
				buf = NULL;
				RETURN_NULL();
				return;
			}

			while(lBytesRead>=data_len){

				delete[] buf;
				buf = NULL;
				data_len+=step;
				
				buf = new char[data_len];
				
				ZeroMemory(buf,sizeof(buf));
				if(!PeekNamedPipe(m_hRead,buf,data_len,&lBytesRead,0,0)){

					delete[] buf;
					buf = NULL;
					RETURN_NULL();
					return;
				}
			}
				
			if (ReadFile(m_hRead, buf, lBytesRead+1, &dwRead, NULL))// 从管道中读取数据 
			{
				ZVAL_STRINGL(return_value,buf,dwRead,1);

				delete[] buf;
				buf = NULL;
				
				return;
			}
			RETURN_NULL();
}

/*******************************************************************
 *@创建进程，把一个php文件直接放到一个进程里面执行
 *@param command path 程序路径
 *@param command params 命令行参数
 ********************************************************************/
PHP_FUNCTION(wing_create_process_ex){
	
	char *params = NULL;
	int	params_len = 0;
	char *params_ex	= NULL;
	int params_ex_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params,&params_len,&params_ex,&params_ex_len) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	
	char *command = NULL;
	spprintf(&command, 0, "%s %s\0",PHP_PATH,params);
	efree(command);
	RETURN_LONG(create_process(command,params_ex,params_ex_len));	
}


/*****************************************************************
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 ****************************************************************/
PHP_FUNCTION(wing_create_process){
	char *exe=NULL;
	int	exe_len=0;
	char *params=NULL;
	int	params_len=0;
	char *params_ex=NULL;
	int params_ex_len=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &exe,&exe_len,&params,&params_len,&params_ex,&params_ex_len) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	char *command = NULL;
	spprintf(&command, 0, "%s %s\0",exe,params);
	efree(command);
	RETURN_LONG(create_process(command,params_ex,params_ex_len));	
}
/*******************************************************************
 *@杀死进程
 ******************************************************************/
ZEND_FUNCTION(wing_process_kill)
{
	long process_id = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id) != SUCCESS ){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if( hProcess == NULL ){
		RETURN_LONG(WING_ERROR_PROCESS_NOT_EXISTS);
        return;
	}
    if( !TerminateProcess( hProcess , 0 ) ){
		RETURN_LONG(WING_ERROR_FAILED);
        return;
	}
    RETURN_LONG(WING_SUCCESS);
}


/***************************************************************
 *@获取当前进程id
 **************************************************************/
ZEND_FUNCTION(wing_get_current_process_id){
	ZVAL_LONG(return_value,GetCurrentProcessId());
}


/***************************************************************
 *@返回 0程序正在运行 -1 获取参数错误 -2 参数不能为空 
 *@-3创建互斥锁失败 long handle创建互斥锁成功  
 **************************************************************/
ZEND_FUNCTION(wing_create_mutex){
	char *mutex_name = NULL;
	int mutex_name_len = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&mutex_name,&mutex_name_len) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	if(mutex_name_len<=0){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	/*---跨边界共享内核对象需要的参数 这里不用
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;*/

	HANDLE m_hMutex =  CreateMutex(NULL,TRUE,mutex_name);//CreateMutex(&sa,TRUE,mutex_name);
    DWORD dwRet = GetLastError();
    if ( m_hMutex )
    {
        if ( ERROR_ALREADY_EXISTS == dwRet )
        {
            CloseHandle(m_hMutex);
			RETURN_LONG(ERROR_ALREADY_EXISTS);
            return;
		}else{
			RETURN_LONG((long)m_hMutex);
			return;
		}
    }
   
	RETURN_LONG(WING_ERROR_FAILED);
}


/****************************************************************
 *@关闭互斥量
 ****************************************************************/
ZEND_FUNCTION(wing_close_mutex){
	long mutex_handle = 0;
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


/*****************************************************************************************************
 *@检测进程是否存活--实际意义不大，因为进程id重用的特性 进程退出后 同样的进程id可能立刻被重用
 *****************************************************************************************************/
ZEND_FUNCTION(wing_process_isalive)
{
	long process_id = 0;
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

/***************************************************************************************************
 *@获取环境变量
 **************************************************************************************************/
ZEND_FUNCTION(wing_get_env){
	
	char *name = NULL;
	int name_len = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len) != SUCCESS){
		RETURN_NULL();
		return;
	}
	int len = GetEnvironmentVariable(name,NULL,0);
	if(len<=0){
		RETURN_NULL();
		return;
	}
	char *var= new char[len];  
	
	ZeroMemory(var,sizeof(var));

	GetEnvironmentVariable(name,var,len);
	ZVAL_STRINGL(return_value,var,len-1,1);

	delete[] var;
	var = NULL;
}

/****************************************************************************************************
 * @ 设置环境变量，子进程和父进程可以共享，可以简单用作进程间的通信方式
 ***************************************************************************************************/
ZEND_FUNCTION(wing_set_env){
	char *name = NULL;
	zval *value = NULL;
	int name_len = 0;
	MAKE_STD_ZVAL(value);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&name,&name_len,&value) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_string(value);
	RETURN_LONG(SetEnvironmentVariableA(name,(LPCTSTR)Z_STRVAL_P(value)));
}


/**************************************************************************************************
 * @ 获取一个命令所在的绝对文件路径
 * @ 比如说获取php的安装目录，不过wing php里面可以直接使用常量 WING_PHP 代表的书php的安装路径
 *************************************************************************************************/
ZEND_FUNCTION(wing_get_command_path){ 
	char *name = 0;
	int name_len = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len) != SUCCESS){
		RETURN_NULL();
		return;
	}
	char path[MAX_PATH] = {0};
	get_command_path((const char*)name,path);
	RETURN_STRING(path,1);
}


/*************************************************************************************************
 *@通过WM_COPYDATA发送进程间消息 只能发给窗口程序
 *@注：只能给窗口程序发消息
 ************************************************************************************************/
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


/**********************************************************************
 *@窗口过程 仅供测试
 ********************************************************************/
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
/*********************************************************************
 *@创建一个窗口 纯属测试
 *********************************************************************/
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
/********************************************************************************
 *@销毁一个窗口
 ********************************************************************************/
ZEND_FUNCTION(wing_destory_window){
	long hwnd;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&hwnd) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	long  status = DestroyWindow((HWND)hwnd)?WING_SUCCESS:WING_ERROR_FAILED;
	RETURN_BOOL(status);
}
/*******************************************************************************
 *@启用消息循环 创建窗口必用 阻塞
 ******************************************************************************/
ZEND_FUNCTION(wing_message_loop){
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
/*******************************************************************************
 *@简单的消息弹窗
 *******************************************************************************/
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
/********************************************************************************
 *@获取最后发生的错误
 ********************************************************************************/
ZEND_FUNCTION(wing_get_last_error){
	RETURN_LONG(GetLastError());
}



/********************************************************************************
 *@毫秒级别定时器
 *@author yuyi
 *@created 2016-05-15
 ********************************************************************************/
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

	char *command = NULL;
	 
	
	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_timer_count);

	if(!run_process){
		unsigned long pid = create_process(command,NULL,0);
		efree(command);
		RETURN_LONG(pid);	
		return;
	}

	if(wing_timer_count!=last_value){
		efree(command);
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}

	efree(command);

	HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;
	zval *retval_ptr;
	int times=0;

	LONGLONG time = (-1)*accuracy*Z_LVAL_P(dwMilliseconds);

    //设置相对时间为1秒 10000000。
	liDueTime.QuadPart = time;
	char *timername = NULL;
	spprintf(&timername,0,"wing_waitable_timer-%s",create_guid());
    //创建定时器。
    hTimer = CreateWaitableTimer(NULL, TRUE, timername);
	efree(timername);

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
				zval_ptr_dtor(&retval_ptr);
				RETURN_LONG(WING_ERROR_FAILED);	
				return;
			}
			
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
						zend_error(E_USER_WARNING,"onconnect call_user_function fail");
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
				SOCKET client = (SOCKET)msg->wparam;

				zval *params[2] = {0};
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(retval_ptr);

				ZVAL_LONG(params[0],(long)client);
				ZVAL_STRINGL(params[1],temp->msg,temp->len,1);

				zend_try{
					if( SUCCESS != call_user_function(EG(function_table),NULL,onreceive,retval_ptr,2,params TSRMLS_CC) ){
						zend_error(E_USER_WARNING,"onreceive call_user_function fail");
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
						zend_error(E_USER_WARNING,"WM_ONCLOSE call_user_function fail\r\n");
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
						zend_error(E_USER_WARNING,"onerror call_user_function fail");
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
	/*zval *socket = NULL;
	zval *msg = NULL;
	int close_after_send = 0;//发送完关闭socket 默认为false 否 待定 还没开发

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|l",&socket,&msg,&close_after_send) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long(socket);

	
	SOCKET sClient		= (SOCKET)Z_LVAL_P(socket);

	char *pData			= Z_STRVAL_P(msg);
	ulong Length		= Z_STRLEN_P(msg);
	unsigned long  Flag	= 0;  
	DWORD SendByte		= 0;  

    if ( sClient == INVALID_SOCKET || pData == NULL || Length == 0 ){
		
		RETURN_LONG(WING_ERROR_FAILED);
		return;  
	}
   
	  *PerIoData =  (*)GlobalAlloc(GPTR,sizeof());//new ();
	memory_add();

	ZeroMemory(&(PerIoData->OVerlapped),sizeof(OVERLAPPED));      
	PerIoData->DATABuf.buf	= pData; 
	PerIoData->DATABuf.len	= Length; 
	PerIoData->type			= OPE_SEND;
	
	int bRet  = WSASend(sClient,&(PerIoData->DATABuf),1,&SendByte,Flag,&(PerIoData->OVerlapped),NULL);  
	if( bRet != 0 &&  WSAGetLastError() != WSA_IO_PENDING ){

		GlobalFree( PerIoData );
		PerIoData = NULL;
		memory_sub();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	RETURN_LONG(WING_SUCCESS);*/
}

//////////////////////////--socket-end--

/*********************************
 * @获取使用的内存信息 
 * @进程实际占用的内存大小
 *********************************/
ZEND_FUNCTION(wing_get_memory_info){

	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	CloseHandle(handle);
	RETURN_LONG( pmc.WorkingSetSize );
	return;
}
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
	
	PHP_PATH = new char[MAX_PATH];
	//memory_add();

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
	ZEND_FALIAS(wing_socket,wing_service,NULL)
	ZEND_FALIAS(wing_tcp_server,wing_service,NULL)

	PHP_FE(wing_socket_info,NULL)
	PHP_FE(wing_socket_send_msg,NULL)
	PHP_FE(wing_service_stop,NULL)
	PHP_FE(wing_close_socket,NULL)
	PHP_FE(wing_get_memory_info,NULL)
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
