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

DWORD create_process(char *command,char *params_ex,int params_ex_len){
	    HANDLE				m_hRead;
		HANDLE				m_hWrite;
		STARTUPINFO			sui;    
		PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
		SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		
		char				*params    = "";
		int					params_len = sizeof("")-1;
    
		sa.bInheritHandle		= TRUE; // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
		sa.lpSecurityDescriptor = NULL;
		sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
		if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
		{
			return -1;
		}

   
		ZeroMemory(&sui, sizeof(STARTUPINFO)); // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
		sui.cb				= sizeof(STARTUPINFO);
		sui.dwFlags			= STARTF_USESTDHANDLES;  
		sui.hStdInput		= m_hRead;
		sui.hStdOutput		= m_hWrite;
		
		sui.hStdError		= GetStdHandle(STD_ERROR_HANDLE);
		
		if(params_ex_len>0&&params_ex!=NULL){
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
			return -1;
		}
		
		CloseHandle(pi.hProcess); // 子进程的进程句柄
		CloseHandle(pi.hThread); // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		
		return pi.dwProcessId;	
}

void command_params_check(char *command_params,int *run_process){
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
			if(index>0)
				spprintf(&command_params,0,"%s \"%s\" ",command_params,(char*)Z_LVAL_PP(data));
		} 
	}
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


//timer 进程计数器 用于控制多个timer的创建和运行
static int wing_timer_count = 0;




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

/**
 *@创建多线程，使用进程模拟
 */
PHP_FUNCTION(wing_create_thread){
	zval *callback;
	char *command_params	= "";
	int run_process			= 0;
	char	*command;
	char   _php[MAX_PATH];  


	MAKE_STD_ZVAL(callback);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) ==FAILURE) {
		RETURN_LONG(-1);	
		return;
	}
	
	command_params_check(command_params,&run_process);
	
	GetModuleFileName(NULL,_php,MAX_PATH);
	spprintf(&command, 0, "%s %s %s wing-process",_php, zend_get_executed_filename(TSRMLS_C),command_params);

	if(!run_process){
		RETURN_LONG(create_process(command,NULL,0));	
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
	char *params = "";
	int	params_len	= 0;
	char *params_ex="";
	int params_ex_len=0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params,&params_len,&params_ex,&params_ex_len) ==FAILURE) {
		RETURN_BOOL(0);
		return;
	}

	TCHAR   _php[MAX_PATH];   
	char				*command;
	GetModuleFileName(NULL,_php,MAX_PATH);
	spprintf(&command, 0, "%s %s\0",_php,params);

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
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &exe,&exe_len,&params,&params_len,&params_ex,&params_ex_len) ==FAILURE) {
		RETURN_BOOL(0);
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
/**
 *@关闭互斥量
 */
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
/**
 *@检测进程是否存活--实际意义不大，因为进程id重用的特性 进程退出后 同样的进程id可能立刻被重用
 */
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

/**
 *@获取环境变量
 */
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

/**
 *@设置环境变量
 */
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
/**
 *@获取一个命令所在的绝对文件路径
 */
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
/**
 *@通过WM_COPYDATA发送进程间消息 只能发给窗口程序
 */
ZEND_FUNCTION(wing_send_msg){
	zval *console_title;
	zval *message_id;
	zval *message;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzz",&console_title,&message_id,&message)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	convert_to_cstring(console_title);
	convert_to_long(message_id);
	convert_to_cstring(message);

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
/**
 *@销毁一个窗口
 */
ZEND_FUNCTION(wing_destory_window){
	long hwnd;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&hwnd)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	RETURN_BOOL(DestroyWindow((HWND)hwnd));
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
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&content,&c_len,&title,&t_len)==FAILURE){
		RETURN_BOOL(0);
		return;
	}
	MessageBox(0,content,title,0);
}
/**
 *@获取最后发生的错误
 */
ZEND_FUNCTION(wing_get_last_error){
	RETURN_LONG(GetLastError());
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
	convert_to_long(width);
	convert_to_string(save_path);
	
	if(strlen(Z_STRVAL_P(str))==0){
		RETURN_BOOL(0);
		return;
	}
	
	int _width = Z_LVAL_P(width);
	if(_width<=0){
		_width=120;
	}

	char *base64_encode_image_str = qrencode(Z_STRVAL_P(str),_width,Z_STRVAL_P(save_path));
	if(return_value_used)
	RETURN_STRING(base64_encode_image_str,1);
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
	int __index = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|ll",&dwMilliseconds, &callback,&max_run_times,&__index) ==FAILURE) {
		RETURN_LONG(-1);	
		return;
	}
	convert_to_double(dwMilliseconds);
	zval **argv;
	int argc;
	char *command_params="";
	HashTable *arr_hash;
	int run_process = 0;
	int command_index = 0;
	int last_value=0;
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

			if(index>0){
				spprintf(&command_params,0,"%s \"%s\" ",command_params,(char*)Z_LVAL_PP(data));
			}

			if(index == argc-1){
				 last_value= atoi((char*)Z_LVAL_PP(data));
			}
		} 
	}

	char	*command;
	char   _php[MAX_PATH];   
	GetModuleFileName(NULL,_php,MAX_PATH);
	spprintf(&command, 0, "%s %s %s wing-process %ld",_php, zend_get_executed_filename(TSRMLS_C),command_params,wing_timer_count);

	if(!run_process){
		RETURN_LONG(create_process(command,NULL,0));	
		return;
	}

	if(wing_timer_count!=last_value){
		RETURN_LONG(-1);
		return;
	}


	timer_thread_params *param	= new timer_thread_params();
	param->dwMilliseconds		= (DWORD)Z_DVAL_P(dwMilliseconds);
	param->thread_id			= GetCurrentThreadId();
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

		}

    } 

	
	
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

	PHP_FE(wing_process_kill,NULL)
	ZEND_FALIAS(wing_thread_kill,wing_process_kill,NULL)

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
