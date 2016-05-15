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


#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Winbase.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")

//此部分函数来源于c++ cpp定义 不能直接include头文件 通过extern访问
extern void get_command_path(const char *name,char *output);
extern char* qrdecode(char* filename);
extern char* qrencode(char* str,int imageWidth,char* save_path);

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
    RETURN_STRINGL(string,len,0);
}

/**
 *@检查线程是否还在运行
 */
PHP_FUNCTION(wing_thread_isalive){
	//zval *handle;
	int thread_id;
	HANDLE thread_handle;
	int exit_code=0;
	zval *exitcode;
	MAKE_STD_ZVAL(exitcode);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|z",&thread_id,&exitcode)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	thread_handle=OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id);
	GetExitCodeThread(thread_handle,(LPDWORD)&exit_code);
	ZVAL_LONG(exitcode,exit_code);
	if(exit_code==STILL_ACTIVE){
		RETURN_BOOL(1);
	}
	RETURN_BOOL(0);

}

ZEND_FUNCTION(wing_wait_multi_objects){

	zval *arr, **data;
    HashTable *arr_hash;
    HashPosition pointer;
    int array_count;
	long timeout=INFINITE ;
	BOOL bWAitForAll=0;
	//ZVAL_BOOL(bWAitForAll,false);
	MAKE_STD_ZVAL(arr);
	//MAKE_STD_ZVAL(bWAitForAll);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|bl", &arr,&bWAitForAll,&timeout) ==FAILURE) {
		RETURN_BOOL(0);
		return;
	}
	//if(Z_TYPE_P(bWAitForAll)==IS_NULL){
	//	ZVAL_BOOL(bWAitForAll,false);
	//}
	//convert_to_boolean(bWAitForAll);
	arr_hash = Z_ARRVAL_P(arr);
    array_count = zend_hash_num_elements(arr_hash);
	
	HANDLE *handle=new HANDLE[array_count];  

	int _index=0;	
	for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; zend_hash_move_forward_ex(arr_hash, &pointer)) {
		//zval temp;
		char *key;
		int key_len;
		long index;

		if (zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer) == HASH_KEY_IS_STRING) {
			//PHPWRITE(key, key_len);
		} else {
			//php_printf("%ld", index);
		}

		handle[_index]=(HANDLE)Z_LVAL_PP(data);//OpenThread(THREAD_ALL_ACCESS, FALSE,  Z_LVAL_PP(data));//(HANDLE)_h;

		php_printf("%ld\n",(long)handle[_index]);
	}
	//php_printf("count:%ld--%ld--%ld\n",array_count,handle,timeout);
	//BOOL wait_all = Z_BVAL_P(bWAitForAll);
	int b=WaitForMultipleObjects((DWORD)array_count, (const HANDLE*)handle, bWAitForAll, (DWORD)timeout); 

	//php_printf("b:%ld\n%ld\n\n",b,WAIT_FAILED);//

	free(handle);
	RETURN_LONG(b);
}
/**
 *@wait process进程等待
 *@param process id 进程id
 *@return exit code 进程退出码
 */

PHP_FUNCTION(wing_process_wait){
	
	int thread_id,timeout=INFINITE;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|l",&thread_id,&timeout)==FAILURE){
		RETURN_LONG(0);
		return;
	}
	HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS, FALSE, thread_id);

	 DWORD wait_result;

	 DWORD wait_status= WaitForSingleObject(handle,timeout);
	 
	 if(wait_status==WAIT_FAILED){
		RETURN_BOOL(0);
		return;
	 }

	 if(WAIT_TIMEOUT == wait_status){
		RETURN_LONG(-1);
		return;
	 }
	 
	 GetExitCodeProcess(handle,&wait_result);
	 RETURN_LONG(wait_result);
}


PHP_FUNCTION(wing_create_thread){
	zval *callback;
	MAKE_STD_ZVAL(callback);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) ==FAILURE) {
		RETURN_LONG(-1);	
		return;
	}
	zval **argv;
	int argc;
	char *command_params="";
	HashTable *arr_hash;
	int run_process = 0;
	 //获取命令行参数
	if (zend_hash_find(&EG(symbol_table),"argv",sizeof("argv"),(void**)&argv) == SUCCESS){
		zval  **data;
		HashPosition pointer;
		arr_hash	= Z_ARRVAL_PP(argv);
		argc		= zend_hash_num_elements(arr_hash);
		for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; zend_hash_move_forward_ex(arr_hash, &pointer)) {
			if(strcmp((char*)Z_LVAL_PP(data),"wing-process")==0){
				run_process=1;
			}

			char *key;
			int key_len,index;
			zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer);
			if(index>0)
				spprintf(&command_params,0,"%s \"%s\" ",command_params,(char*)Z_LVAL_PP(data));
		} 
	}




	//===========================================================================

	/*Bucket *p;
	HashTable *ht = EG(active_symbol_table);
	p = ht->pListHead;
	while (p != NULL) {
		int result =1;// apply_func(p->pData TSRMLS_CC);
		if (result & ZEND_HASH_APPLY_REMOVE) {
			p = zend_hash_apply_deleter(ht, p);
		} else {
			p = p->pListNext;
		}

		if (result & ZEND_HASH_APPLY_STOP) {
			break;
		}
	}*/
	//===========================================================================





	//zend_printf("%s\n",command_params);

	if(!run_process){

		TCHAR   _php[1000];   
	    GetModuleFileName(NULL,_php,MAX_PATH);
		HANDLE				m_hRead;
		HANDLE				m_hWrite;
		STARTUPINFO			sui;    
		PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
		SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		char				*command;
		char				*params="";
		int					debug_len;
		int					params_len= sizeof("")-1;
    
		sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
		sa.lpSecurityDescriptor = NULL;
		sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
		if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
		{
			RETURN_LONG(-1);
		   return;
		}

   
		ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
		sui.cb				= sizeof(STARTUPINFO);
		sui.dwFlags			= STARTF_USESTDHANDLES;  
		sui.hStdInput		= m_hRead;
		sui.hStdOutput		= m_hWrite;
		/* 以上两行也许大家要有些疑问，为什么把管道读句柄（m_hRead）赋值给了hStdInput, 因为管道是双向的，
		对于父进程写的一端正好是子进程读的一端，而m_hRead就是父进程中对管道读的一端， 自然要把这个句柄给子进程让它来写数据了
		(sui是父进程传给子进程的数据结构，里面包含了一些父进程要告诉子进程的一些信息)，反之一样*/
		sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);
		debug_len = spprintf(&command, 0, "%s %s %s wing-process",_php, zend_get_executed_filename(TSRMLS_C),command_params);
		if (!CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
		{
			CloseHandle(m_hRead);
			CloseHandle(m_hWrite);
			RETURN_LONG(-1);
		}
		else
		{
			CloseHandle(pi.hProcess); // 子进程的进程句柄
			CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		}
		RETURN_LONG(pi.dwProcessId);	
		return;
	}

	zval *retval_ptr;
	MAKE_STD_ZVAL(retval_ptr);
	if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
		RETURN_LONG(-1);	
	}
	zval_ptr_dtor(&retval_ptr);
	RETURN_LONG(0);	
}
/**
 *@get data from create process
 *@从父进程获取数据
 *return string or null
 */
ZEND_FUNCTION(wing_get_process_params){

			HANDLE m_hRead = GetStdHandle(STD_INPUT_HANDLE);
			char buf[1024];
			memset(buf,0,sizeof(buf));
			DWORD dwRead;
			//char t[1];
			unsigned long lBytesRead;
			if(PeekNamedPipe(m_hRead, buf, 1024, &dwRead, NULL, 0)){
				
				if(!PeekNamedPipe(m_hRead,buf,1024,&lBytesRead,0,0)){
					RETURN_NULL();
					return;
				}

				if(lBytesRead<=0){
					RETURN_NULL();
					return;
				}
				
				if (ReadFile(m_hRead, buf, 1024, &dwRead, NULL))// 从管道中读取数据 
				{
					//这种读取管道的方式非常不好，最好在实际项目中不要使用，因为它是阻塞式的，
					//如果这个时候管道中没有数据他就会一直阻塞在那里， 程序就会被挂起，而对管道来说一端正在读的时候，
					//另一端是无法写的，也就是说父进程阻塞在这里后，子进程是无法把数据写入到管道中的， 
					//在调用ReadFile之前最好调用PeekNamePipe来检查管道中是否有数据，它会立即返回, 
					//或者使用重叠式读取方式,那么ReadFile的最后一个参数不能为NULL
					RETURN_STRING(buf,1);						
				}
			}
			RETURN_NULL();
}

/**
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION(wing_create_process_ex){
	HANDLE				m_hRead;
	HANDLE				m_hWrite;
	STARTUPINFO			sui;    
    PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
	
	char				*command;
	char				*params="";
	int					debug_len;
	int					params_len= sizeof("")-1;
	char *params_ex="";
	int params_ex_len=0;
    
	sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
    sa.lpSecurityDescriptor = NULL;
    sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
    if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
    {
		RETURN_BOOL(0);
       return;
    }

   
    ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
    sui.cb				= sizeof(STARTUPINFO);
    sui.dwFlags			= STARTF_USESTDHANDLES;
        
    sui.hStdInput		= m_hRead;
	sui.hStdOutput		= m_hWrite;
    /* 以上两行也许大家要有些疑问，为什么把管道读句柄（m_hRead）赋值给了hStdInput, 因为管道是双向的，
	对于父进程写的一端正好是子进程读的一端，而m_hRead就是父进程中对管道读的一端， 自然要把这个句柄给子进程让它来写数据了
	(sui是父进程传给子进程的数据结构，里面包含了一些父进程要告诉子进程的一些信息)，反之一样*/
    sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params,&params_len,&params_ex,&params_ex_len) ==FAILURE) {
		RETURN_BOOL(0);
		return;
	}


	TCHAR   _php[1000];   
	GetModuleFileName(NULL,_php,MAX_PATH);
	debug_len = spprintf(&command, 0, "%s %s\0",_php,params);
	if(params_ex_len>0){
		DWORD d;
		if(::WriteFile(m_hWrite,params_ex,params_ex_len,&d,NULL)==FALSE){
			//告警
			zend_error(E_USER_WARNING,"write params to parcess error");
		}
	}


	if (!CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
    {
        CloseHandle(m_hRead);
        CloseHandle(m_hWrite);
		RETURN_BOOL(0);
    }
    else
    {
        CloseHandle(pi.hProcess); // 子进程的进程句柄
        CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
    }

	//zval_ptr_dtor(&exe);
	RETURN_LONG(pi.dwProcessId);	
}


/**
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION(wing_create_process){
	HANDLE				m_hRead;
	HANDLE				m_hWrite;
	STARTUPINFO			sui;    
    PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
	char				*exe;
	char				*command;
	char				*params="";
	int					exe_len,debug_len;
	int					params_len= sizeof("")-1;
    char *params_ex="";
	int params_ex_len=0;

	sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
    sa.lpSecurityDescriptor = NULL;
    sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
    if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
    {
		RETURN_BOOL(0);
       return;
    }

   
    ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
    sui.cb				= sizeof(STARTUPINFO);
    sui.dwFlags			= STARTF_USESTDHANDLES;
        
    sui.hStdInput		= m_hRead;
	sui.hStdOutput		= m_hWrite;
    /* 以上两行也许大家要有些疑问，为什么把管道读句柄（m_hRead）赋值给了hStdInput, 因为管道是双向的，
	对于父进程写的一端正好是子进程读的一端，而m_hRead就是父进程中对管道读的一端， 自然要把这个句柄给子进程让它来写数据了
	(sui是父进程传给子进程的数据结构，里面包含了一些父进程要告诉子进程的一些信息)，反之一样*/
    sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &exe,&exe_len,&params,&params_len,&params_ex,&params_ex_len) ==FAILURE) {
		RETURN_BOOL(0);
		return;
	}

	debug_len = spprintf(&command, 0, "%s %s\0",exe,params);


	if(params_ex_len>0){
		DWORD d;
		if(::WriteFile(m_hWrite,params_ex,params_ex_len,&d,NULL)==FALSE){
			//告警
			zend_error(E_USER_WARNING,"write params to parcess error");
		}
	}
	if (!CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
    {
        CloseHandle(m_hRead);
        CloseHandle(m_hWrite);
		RETURN_BOOL(0);
    }
    else
    {
        CloseHandle(pi.hProcess); // 子进程的进程句柄
        CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
    }

	//zval_ptr_dtor(&exe);
	RETURN_LONG(pi.dwProcessId);	
}

ZEND_FUNCTION(wing_process_kill)
{
	long process_id;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if(hProcess==NULL){
		RETURN_BOOL(0);
        return;
	}
    if(!TerminateProcess(hProcess,0)){
		RETURN_BOOL(0);
        return;
	}
    RETURN_BOOL(1);
}


ZEND_FUNCTION(wing_get_current_process_id){
	ZVAL_LONG(return_value,GetCurrentProcessId());
}

/**
 *@return 0程序正在运行 -1 获取参数错误 -2 参数不能为空 -3创建互斥锁失败 long handle创建互斥锁成功  
 */
ZEND_FUNCTION(wing_create_mutex){
	char *mutex_name;
	int mutex_name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&mutex_name,&mutex_name_len)==FAILURE){
		//zend_error(E_COMPILE_WARNING,"get params error");
		//RETURN_BOOL(0);
		RETURN_LONG(-1);
		return;
	}
	if(mutex_name_len<=0){
		//zend_error(E_COMPILE_WARNING,"mutex name must not empty string");
		RETURN_LONG(-2);
		return;
	}

	HANDLE m_hMutex = CreateMutex(NULL,TRUE,mutex_name);
    DWORD dwRet = GetLastError();
    if (m_hMutex)
    {
        if (ERROR_ALREADY_EXISTS == dwRet)
        {
            //printf("程序已经在运行中了,程序退出!\n");
            CloseHandle(m_hMutex);
			RETURN_LONG(0);
            return;
		}else{
			RETURN_LONG((long)m_hMutex);
		}
    }
   
    // printf("创建互斥量错误,程序退出!\n");
    CloseHandle(m_hMutex);
	//zend_error(E_COMPILE_ERROR,"mutex create error");
	RETURN_LONG(-3);
}

ZEND_FUNCTION(wing_close_mutex){
	long mutex_handle;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&mutex_handle)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	if(mutex_handle<=0){
		RETURN_LONG(0);
		return;
	}
	RETURN_LONG(CloseHandle((HANDLE)mutex_handle));
}



ZEND_FUNCTION(wing_process_isalive)
{
	long process_id;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if(hProcess==NULL){
		RETURN_BOOL(0);
        return;
	}
    RETURN_BOOL(1);
}

ZEND_FUNCTION(wing_get_env){
	char *name;
	//zval *temp;
	int name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	int len = GetEnvironmentVariable(name,NULL,0)+1;
	char *var=new char[len];
	GetEnvironmentVariable(name,var,len);
	ZVAL_STRINGL(return_value,var,len,1);
	free(var);
}

ZEND_FUNCTION(wing_set_env){
	char *name;
	zval *value;
	int name_len;
	MAKE_STD_ZVAL(value);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&name,&name_len,&value)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	convert_to_cstring(value);
	RETURN_BOOL(SetEnvironmentVariable(name,Z_STRVAL_P(value)));
}

ZEND_FUNCTION(wing_get_command_path){ 
	char *name;
	int name_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&name_len)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	char path[MAX_PATH];
	get_command_path((const char*)name,path);
	RETURN_STRING(path,1);
}

ZEND_FUNCTION(wing_set_console_title){
	char *title;
	int title_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&title,&title_len)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	RETURN_BOOL(SetConsoleTitleA(title));
}

ZEND_FUNCTION(wing_get_console_title){
	char title[255];
	memset(title,0,sizeof(title));
	GetConsoleTitle(title,255);
	RETURN_STRING(title,1);
}

ZEND_FUNCTION(wing_send_msg){
	zval *console_title;
	zval *message_id;
	zval *message;
	//zval *message_type;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzz",&console_title,&message_id,&message)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	convert_to_cstring(console_title);
	convert_to_long(message_id);
	convert_to_cstring(message);
	//convert_to_long(message_type);

	HWND hwnd=FindWindow (Z_STRVAL_P(console_title),NULL);
	if(hwnd==NULL){
		RETURN_BOOL(0);
		return;
	}
	//WM_USER
	COPYDATASTRUCT CopyData; 
	CopyData.dwData	= Z_LVAL_P(message_id);  
	CopyData.cbData	= Z_STRLEN_P(message);  
	CopyData.lpData = Z_STRVAL_P(message);  //WM_COPYDATA
	SendMessageA(hwnd,WM_COPYDATA,NULL,(LPARAM)&CopyData);
	RETURN_BOOL(GetLastError()==0);
}


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
ZEND_FUNCTION(wing_create_window){
	zval *console_title;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&console_title)==FAILURE){
		RETURN_BOOL(0);
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

ZEND_FUNCTION(wing_destory_window){
	long hwnd;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&hwnd)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	RETURN_BOOL(DestroyWindow((HWND)hwnd));
}
ZEND_FUNCTION(wing_message_box){
	char *content;
	int c_len,t_len;
	char *title;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&content,&c_len,&title,&t_len)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	MessageBox(0,content,title,0);
}
ZEND_FUNCTION(wing_get_last_error){
	RETURN_LONG(GetLastError());
}

ZEND_FUNCTION(wing_message_loop){
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


ZEND_FUNCTION(wing_create_name_pipe){
	////./pipe/PipeName
	char *pipe_name;
	int name_len;
	long open_mode = PIPE_ACCESS_DUPLEX;
	long pipe_mode = PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE;
	long max_instance = PIPE_UNLIMITED_INSTANCES;
	long output_buffer_size = 0;
	long input_buffer_size = 0; 
	long time_out = 5000;
	long security = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|lllllll",&pipe_name,&name_len,&open_mode,&pipe_mode,&max_instance,
		&output_buffer_size,&input_buffer_size,&time_out,&security)==FAILURE){
		RETURN_BOOL(0);
		return;
	}

	//zend_printf("123=>%u\n",FILE_FLAG_OVERLAPPED);
	
	HANDLE pipe= CreateNamedPipeA( 
         pipe_name,            // pipe name 
        // PIPE_ACCESS_DUPLEX |     // read/write access 
        // FILE_FLAG_OVERLAPPED,    // overlapped mode 

		open_mode,

        // PIPE_TYPE_MESSAGE |      // message-type pipe 
        // PIPE_READMODE_MESSAGE |  // message-read mode 
        // PIPE_WAIT,               // blocking mode 

		pipe_mode,

        // PIPE_UNLIMITED_INSTANCES,               // number of instances 

		max_instance,

        output_buffer_size,   // output buffer size 
        input_buffer_size,   // input buffer size 
         time_out,            // client time-out 
         (LPSECURITY_ATTRIBUTES)security);                   // default security attributes 
		
		//CreateNamedPipeA(pipe_name,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED ,PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_NOWAIT,PIPE_UNLIMITED_INSTANCES,0,0,NMPWAIT_USE_DEFAULT_WAIT,0);
	RETURN_LONG((long)pipe);
}

ZEND_FUNCTION(wing_create_event){
	  HANDLE handle = CreateEventA( 
         NULL,    // default security attribute 
         TRUE,    // manual-reset event 
         TRUE,    // initial state = signaled 
         NULL);   // unnamed event object 
	  if(NULL==handle)RETURN_BOOL(0);
	  RETURN_LONG((long)handle);
}

ZEND_FUNCTION(wing_read_file){
	HANDLE hFile;
	zval *lpBuffer;
	LPOVERLAPPED lpOverlapped;
	long size = 4096*sizeof(TCHAR);
	//LPDWORD lpdword;

	zval *_hFile;
	zval *_lpOverlapped;
	zval *_read_num;
	MAKE_STD_ZVAL(_lpOverlapped);
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzz|z",&_hFile,&lpBuffer,&_read_num,&_lpOverlapped);


	convert_to_long(_hFile);
	convert_to_long(_lpOverlapped);
	//convert_to_long(_lpdword);

	hFile			=(HANDLE)Z_LVAL_P(_hFile);
	lpOverlapped	=(LPOVERLAPPED)Z_LVAL_P(_lpOverlapped);
	//lpdword			=(LPDWORD)Z_LVAL_P(_lpdword);

	char data_read[10240];
	long read_num;

	BOOL status=ReadFile(hFile,data_read,sizeof(data_read),(LPDWORD)&read_num,lpOverlapped);
	ZVAL_STRING(lpBuffer,data_read,1);

	//ZVAL_LONG(_hFile,(long)hFile);
	ZVAL_LONG(_lpOverlapped,(long)lpOverlapped);
	ZVAL_LONG(_read_num,read_num);

	RETURN_BOOL(status);
}


ZEND_FUNCTION(wing_write_file){
	HANDLE hFile;
	zval *buffer;
	zval *write_num;
//	LPDWORD lpdword;
	LPOVERLAPPED lpOverlapped;
	zval *_hFile;
	zval *_lpOverlapped;
	//zval *_lpdword;

	MAKE_STD_ZVAL(_lpOverlapped);
	//MAKE_STD_ZVAL(_lpdword);
	MAKE_STD_ZVAL(write_num);

	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz|zz",&_hFile,&buffer,&write_num,&_lpOverlapped);


	convert_to_long(_hFile);
	convert_to_long(_lpOverlapped);
	//convert_to_long(_lpdword);

	hFile			=(HANDLE)Z_LVAL_P(_hFile);
	lpOverlapped	=(LPOVERLAPPED)Z_LVAL_P(_lpOverlapped);
	//lpdword			=(LPDWORD)Z_LVAL_P(_lpdword);
	long _write_num =0;
	BOOL status = WriteFile(hFile,Z_STRVAL_P(buffer),Z_STRLEN_P(buffer),(LPDWORD)&_write_num,lpOverlapped);

	ZVAL_LONG(_lpOverlapped,(long)lpOverlapped);
	//ZVAL_LONG(_lpdword,(long)lpdword);
	ZVAL_LONG(write_num,_write_num);

	RETURN_BOOL(status);

}

ZEND_FUNCTION(wing_get_overlapped_result){
	HANDLE hFile;
	LPOVERLAPPED lpOverlapped;
	LPDWORD lpdword;

	zval *_hFile;
	zval *_lpOverlapped;
	zval *_lpdword;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzz",&_hFile,&_lpOverlapped,&_lpdword);


	convert_to_long(_hFile);
	convert_to_long(_lpOverlapped);
	convert_to_long(_lpdword);

	hFile			=(HANDLE)Z_LVAL_P(_hFile);
	lpOverlapped	=(LPOVERLAPPED)Z_LVAL_P(_lpOverlapped);
	lpdword			=(LPDWORD)Z_LVAL_P(_lpdword);

	BOOL status = GetOverlappedResult(
                  hFile, // handle to pipe
            lpOverlapped, // OVERLAPPED structure
            lpdword,            // bytes transferred
            FALSE); // do not wait

	ZVAL_LONG(_hFile,(long)hFile);
	ZVAL_LONG(_lpOverlapped,(long)lpOverlapped);
	ZVAL_LONG(_lpdword,(long)lpdword);

	RETURN_BOOL(status);           
}

ZEND_FUNCTION(wing_disconnect_name_pipe){
	HANDLE handle;
	long _handle;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&_handle);
	handle=(HANDLE)_handle;
	RETURN_BOOL(DisconnectNamedPipe(handle));
}
ZEND_FUNCTION(wing_set_event){

	LPOVERLAPPED lpo;

	//long _hPipe;
	zval *_lpo;
	MAKE_STD_ZVAL(_lpo);
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&_lpo);
	


	lpo = (LPOVERLAPPED)Z_LVAL_P(_lpo);
	RETURN_BOOL(SetEvent(lpo->hEvent));
}
ZEND_FUNCTION(wing_connect_name_pipe){

	HANDLE hPipe;
	

	long _hPipe;
	zval *_lpo;
	zval *_event;
	MAKE_STD_ZVAL(_lpo);
	MAKE_STD_ZVAL(_event);
	
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|zz",&_hPipe,&_event,&_lpo);
	convert_to_long(_event);
	
	OVERLAPPED  lpo;// = new OVERLAPPED();
	LPOVERLAPPED __lpo;
	if(Z_LVAL_P(_event)>0){
		lpo.hEvent = (HANDLE)Z_LVAL_P(_event);
		__lpo = &lpo;
	}else{
		__lpo=NULL;
	}

	hPipe = (HANDLE)(_hPipe);
	//lpo = (LPOVERLAPPED)Z_LVAL_P(_lpo);

	BOOL fConnected = FALSE; 
 
// Start an overlapped connection for this pipe instance. 
   fConnected = ConnectNamedPipe(hPipe, __lpo); 
   zend_printf("fConnected:%ld\n",fConnected);

   //https://msdn.microsoft.com/en-us/library/aa365146(VS.85).aspx
   //如果异步 即__lpo不为null 返回0正确
   if(__lpo != NULL){
	if(fConnected==0)fConnected=1;
   }

   //ERROR_IO_INCOMPLETE;

   //zend_printf("lpo:%ld\n",(long)&lpo);

   if(Z_TYPE_P(_lpo) != IS_NULL){
		ZVAL_LONG(_lpo,(long)__lpo);
   }

   //zend_printf("lpo:%ld\n",Z_LVAL_P(_lpo));

   RETURN_BOOL(fConnected);

}

ZEND_FUNCTION(wing_create_file){



	zval *_file_name;
	long access =  GENERIC_READ |  // read and write access
           GENERIC_WRITE;
	long share =0;
	long security =0;
	long create = OPEN_EXISTING;
	long attributes= FILE_ATTRIBUTE_NORMAL;
	long template_file = 0;

	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|lllll",&_file_name,&access,&share,&security,&create,&attributes,&template_file);


	convert_to_cstring(_file_name);
	//CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE,
     //   0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//zend_printf("%u\n",FILE_ATTRIBUTE_NORMAL);

	HANDLE handle = CreateFileA(
           Z_STRVAL_P(_file_name),   // pipe name
           access,
           share,              // no sharing
           (LPSECURITY_ATTRIBUTES)security,           // default security attributes
           create,  // opens existing pipe
           attributes,              // default attributes
           (HANDLE)template_file);          // no template file

	RETURN_LONG((long)handle);
}

ZEND_FUNCTION(wing_close_handle){
	long handle;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&handle);
	RETURN_BOOL(CloseHandle((HANDLE)handle));
}


ZEND_FUNCTION(wing_wait_name_pipe){
	char *name;
	int name_len;
	long timeout=20000;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&name,&name_len,&timeout);

	RETURN_BOOL(WaitNamedPipeA(name,timeout));
}

ZEND_FUNCTION(wing_set_name_pipe_handle_state){

	/**
	BOOL WINAPI SetNamedPipeHandleState(
  __in      HANDLE  hNamedPipe,   // 要更改的管道句柄
  __in_opt  LPDWORD lpMode,       // 更改管道的运行模式
  __in_opt  LPDWORD lpMaxCollectionCount, // 更改管道的收集的最大字节数
  __in_opt  LPDWORD lpCollectDataTimeout  // 更改管道的等待时间
);
	*/

	zval *handle;
	zval *mode;
	zval *lpMaxCollectionCount;
	zval *lpCollectDataTimeout;

	MAKE_STD_ZVAL(mode);
	ZVAL_LONG(mode,PIPE_READMODE_MESSAGE);
	MAKE_STD_ZVAL(lpMaxCollectionCount);
	MAKE_STD_ZVAL(lpCollectDataTimeout);

	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|zzz",&handle,&mode,&lpMaxCollectionCount,&lpCollectDataTimeout );
	

	convert_to_long(handle);
	convert_to_long(mode);
	convert_to_long(lpMaxCollectionCount);
	convert_to_long(lpCollectDataTimeout);

	int dwMode = Z_LVAL_P(mode);
	
	BOOL status = SetNamedPipeHandleState((HANDLE)Z_LVAL_P(handle),(LPDWORD)&dwMode,(LPDWORD)lpMaxCollectionCount,(LPDWORD)lpCollectDataTimeout);

	//ZVAL_LONG(mode,dwMode);

	RETURN_BOOL(status);

}



/**
 *@author yuyi
 *@created 2016-05-14
 *@qr image decode 二维码解析
 *@param string qr image local file path 二维码图片本地路径
 *@return string return qr image decode string 返回解码后的字符串
 */
ZEND_FUNCTION(wing_qrdecode){
	zval *file_path;
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&file_path);
	convert_to_string(file_path);
	if(!PathFileExists(Z_STRVAL_P(file_path))){
		RETURN_BOOL(0);
		return;
	}
	RETURN_STRING(qrdecode(Z_STRVAL_P(file_path)),1);
}
/**
 *@author yuyi
 *@created 2016-05-14
 *@create qrcode 创建二维码
 *@param string source string 要转换为二维码的字符串
 *@param int qr image width px 二维码大小（像素）
 *@param string qr image save as file,default null,do nothing 二维码保存图片，默认为null，即不保存图片
 *@return string  return base64 image string if success 如果成功 返回base64编码后的图片字符串
 */
ZEND_FUNCTION(wing_qrencode){
	zval *str;
	zval *width;
	zval *save_path;
	MAKE_STD_ZVAL(save_path);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz|z",&str,&width,&save_path)!=SUCCESS){
		RETURN_BOOL(0);
		return;
	}
	convert_to_string(str);
	convert_to_double(width);
	convert_to_string(save_path);
	
	if(strlen(Z_STRVAL_P(str))==0){
		RETURN_BOOL(0);
		return;
	}
	int _width = Z_LVAL_P(width);
	if(_width<=0){
		_width=120;
	}

	char *base64_encode_image_str = qrencode(Z_STRVAL_P(str),Z_LVAL_P(width),Z_STRVAL_P(save_path));
	if(return_value_used)
	RETURN_STRING(base64_encode_image_str,1);
	RETURN_NULL();
}

ZEND_FUNCTION(wing_peek_message){
	PostThreadMessageA(GetCurrentThreadId(),WM_USER+99,(WPARAM)"hello",(LPARAM)"word");
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_QS_POSTMESSAGE|PM_QS_SENDMESSAGE)) 
    { 
       // if (msg.message == WM_QUIT) 
		//{
		//	RETURN_LONG(-1);
		//	return;
	//	}
		//msg.message 区分消息类型
		//msg.lParam;
		//msg.wParam;
		//msg.time;


		
                  array_init(return_value);
                  add_assoc_string(return_value,"lParam",(char*)msg.lParam,1);
				  add_assoc_string(return_value,"wParam",(char*)msg.wParam,1);
                  add_assoc_long(return_value,"message",msg.message);
				  add_assoc_long(return_value,"time",msg.time);

				 // zend_printf("%ld\n",msg.time);
				//  zend_printf("WM_QUIT=%ld\n",WM_QUIT);
                 return;
                 // zval_copy_ctor(&temp);
                //  add_next_index_zval(return_value,&temp);
                 // zval_dtor(p);

    } 
	
	RETURN_NULL();
}


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
}  


/****
 *@毫秒级别定时器
 *@author yuyi
 *@created 2016-05-15
 */
ZEND_FUNCTION(wing_timer){
	//PostThreadMessageA(GetCurrentThreadId(),WM_USER+99,(WPARAM)"hello",(LPARAM)"word");

	zval *callback;
	zval *dwMilliseconds;
	MAKE_STD_ZVAL(callback);
	MAKE_STD_ZVAL(dwMilliseconds);

	int max_run_times = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|l",&dwMilliseconds, &callback,&max_run_times) ==FAILURE) {
		RETURN_LONG(-1);	
		return;
	}
	convert_to_double(dwMilliseconds);
	zval **argv;
	int argc;
	char *command_params="";
	HashTable *arr_hash;
	int run_process = 0;
	 //获取命令行参数
	if (zend_hash_find(&EG(symbol_table),"argv",sizeof("argv"),(void**)&argv) == SUCCESS){
		zval  **data;
		HashPosition pointer;
		arr_hash	= Z_ARRVAL_PP(argv);
		argc		= zend_hash_num_elements(arr_hash);
		for(zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; zend_hash_move_forward_ex(arr_hash, &pointer)) {
			if(strcmp((char*)Z_LVAL_PP(data),"wing-process")==0){
				run_process=1;
			}

			char *key;
			int key_len,index;
			zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer);
			if(index>0)
				spprintf(&command_params,0,"%s \"%s\" ",command_params,(char*)Z_LVAL_PP(data));
		} 
	}




	//===========================================================================

	/*Bucket *p;
	HashTable *ht = EG(active_symbol_table);
	p = ht->pListHead;
	while (p != NULL) {
		int result =1;// apply_func(p->pData TSRMLS_CC);
		if (result & ZEND_HASH_APPLY_REMOVE) {
			p = zend_hash_apply_deleter(ht, p);
		} else {
			p = p->pListNext;
		}

		if (result & ZEND_HASH_APPLY_STOP) {
			break;
		}
	}*/
	//===========================================================================





	//zend_printf("%s\n",command_params);

	if(!run_process){

		TCHAR   _php[1000];   
	    GetModuleFileName(NULL,_php,MAX_PATH);
		HANDLE				m_hRead;
		HANDLE				m_hWrite;
		STARTUPINFO			sui;    
		PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
		SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		char				*command;
		char				*params="";
		int					debug_len;
		int					params_len= sizeof("")-1;
    
		sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
		sa.lpSecurityDescriptor = NULL;
		sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
		if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
		{
			RETURN_LONG(-1);
		   return;
		}

   
		ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
		sui.cb				= sizeof(STARTUPINFO);
		sui.dwFlags			= STARTF_USESTDHANDLES;  
		sui.hStdInput		= m_hRead;
		sui.hStdOutput		= m_hWrite;
		
		sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);
		debug_len = spprintf(&command, 0, "%s %s %s wing-process",_php, zend_get_executed_filename(TSRMLS_C),command_params);
		if (!CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi))
		{
			CloseHandle(m_hRead);
			CloseHandle(m_hWrite);
			RETURN_LONG(-1);
		}
		else
		{
			CloseHandle(pi.hProcess); // 子进程的进程句柄
			CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		}
		RETURN_LONG(pi.dwProcessId);	
		return;
	}

	timer_thread_params *param =new timer_thread_params();
	param->dwMilliseconds = Z_DVAL_P(dwMilliseconds);
	param->thread_id=GetCurrentThreadId();
	_beginthreadex(NULL, 0, wing_timer, param, 0, NULL); 

	
	MSG msg;
	BOOL bRet;
	zval *retval_ptr;
	DWORD times=0;//param->times;
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (bRet == -1)
		{
			free(param);
			RETURN_LONG(times);	
			return;
		}

		if(msg.message == (WM_USER+99)){
			
			MAKE_STD_ZVAL(retval_ptr);
			if(SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC)){
				free(param);
				RETURN_LONG(times);	
				return;
			}
			times++;
			
			zval_ptr_dtor(&retval_ptr);

			if(times>=max_run_times&&max_run_times>0)break;
			//RETURN_LONG(0);	
		}
		/*array_init(return_value);
		add_assoc_string(return_value,"lParam",(char*)msg.lParam,1);
		add_assoc_string(return_value,"wParam",(char*)msg.wParam,1);
		add_assoc_long(return_value,"message",msg.message);
		add_assoc_long(return_value,"time",msg.time);*/
		//return;
    } 
	//zend_printf("exit\n");
	
	
				free(param);
				RETURN_LONG(times);	
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
	PHP_FE(wing_qrdecode,NULL)
	PHP_FE(wing_qrencode,NULL)
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

	PHP_FE(wing_wait_multi_objects,NULL)
	PHP_FE(wing_thread_isalive,NULL)

	
	PHP_FE(wing_process_kill,NULL)
	PHP_FE(wing_process_isalive,NULL)
	PHP_FE(wing_get_current_process_id,NULL)
	PHP_FE(wing_create_mutex,NULL)
	PHP_FE(wing_close_mutex,NULL)
	PHP_FE(wing_get_env,NULL)
	PHP_FE(wing_get_command_path,NULL)
	PHP_FE(wing_set_env,NULL)
	PHP_FE(wing_set_console_title,NULL)
	PHP_FE(wing_get_console_title,NULL)
	PHP_FE(wing_send_msg,NULL)
	
	PHP_FE(wing_get_last_error,NULL)
	PHP_FE(wing_create_window,NULL)
	PHP_FE(wing_message_loop,NULL)
	PHP_FE(wing_destory_window,NULL)
	PHP_FE(wing_message_box,NULL)
	PHP_FE(wing_create_name_pipe,NULL)
	PHP_FE(wing_create_event,NULL)

	
	PHP_FE(wing_get_overlapped_result,NULL)
	PHP_FE(wing_disconnect_name_pipe,NULL)
	
	PHP_FE(wing_read_file,NULL)
	PHP_FE(wing_write_file,NULL)
	PHP_FE(wing_create_file,NULL)

	PHP_FE(wing_close_handle,NULL)
	PHP_FE(wing_wait_name_pipe,NULL)
	PHP_FE(wing_set_name_pipe_handle_state,NULL)
	PHP_FE(wing_connect_name_pipe,NULL)
	PHP_FE(wing_set_event,NULL)
	PHP_FE(wing_timer,NULL)
	PHP_FE(wing_peek_message,NULL)
	//PHP_FE(wing_timer,NULL)
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
