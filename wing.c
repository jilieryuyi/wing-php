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
#include <Winsock2.h>
#include "Winbase.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"
#include "process.h"
#include  <mswsock.h>
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")


//#include "lib/include/vld.h"
//#pragma comment(lib,"lib/lib/Win32/vld.lib")


//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
/*inline void EnableMemLeakCheck()
{
   _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

#define n ew   n ew(_NORMAL_BLOCK, __FILE__, __LINE__)*/



//-------------socket相关参数-----------------------
#define DATA_BUFSIZE 1024
#define OPE_RECV 1
#define OPE_SEND 2
#define OPE_ACCEPT 3
SOCKET m_sockListen;
HANDLE m_hIOCompletionPort;

struct MYOVERLAPPED{
	OVERLAPPED m_ol;
	int m_iOpType;
	SOCKET m_skServer;
	SOCKET m_skClient;
	DWORD recvBytes;
	char m_pBuf[DATA_BUFSIZE];
	WSABUF DATABuf; 
	int timeout;
	LPSOCKADDR addrClient;
	LPSOCKADDR addrServer;
};


typedef struct  
{ 

  OVERLAPPED OVerlapped; 
  WSABUF DATABuf; 
  CHAR Buffer[DATA_BUFSIZE]; 
  int type;
  DWORD recvBytes;
  SOCKET client;

}PER_IO_OPERATION_DATA;

 
typedef struct{ 
  SOCKET Socket;
  SOCKADDR_STORAGE ClientAddr;
} PER_HANDLE_DATA; 

typedef struct{ 
  SOCKET Socket;
  HANDLE IOCompletionPort;
  DWORD threadid;
} COMPARAMS; 

typedef struct{
	long len;
	char *msg;//[DATA_BUFSIZE+1]; 
} RECV_MSG;
//-------------socket相关参数------end-----------------


//-------------内存计数器-----------------------
unsigned long memory_add_times = 0;
unsigned long memory_sub_times = 0;


//CRITICAL_SECTION bytes_lock;
void memory_add(){
    InterlockedIncrement(&memory_add_times);
}
void memory_sub(){
    InterlockedIncrement(&memory_sub_times);
}
//-------------内存计数器-----------------------




/**************************************
 * @获取命令的绝对路径
 **************************************/
void get_command_path(const char *name,char *output){

	int env_len		= GetEnvironmentVariable("PATH",NULL,0)+1;
	char *env_var	= new char[env_len];
	ZeroMemory(env_var,sizeof(char)*(env_len));
	memory_add();

	GetEnvironmentVariable("PATH",env_var,env_len);

	char *temp = NULL;
	char *start = NULL,*var_begin = NULL;

	start		= env_var;
	var_begin	= env_var;

	char _temp_path[MAX_PATH] = {0};

	while( temp = strchr(var_begin,';') ){
		
		long len_temp	= temp-start;
		long _len_temp	= len_temp+sizeof("\\")+sizeof(".exe")+1;
		
		ZeroMemory(output,sizeof(char)*MAX_PATH);

		strncpy_s(_temp_path,_len_temp,var_begin,len_temp);
		sprintf_s(output,MAX_PATH,"%s\\%s.exe\0",_temp_path,name);

		if(PathFileExists(output)){
			delete[] env_var;
			env_var = NULL;
			memory_sub();
			return;
		}

		ZeroMemory(output,sizeof(char)*MAX_PATH);
		sprintf_s(output,MAX_PATH,"%s\\%s.bat\0",_temp_path,name);

		if(PathFileExists(output)){
			delete[] env_var;
			env_var = NULL;
			memory_sub();
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}
	delete[] env_var;
	env_var = NULL;
	memory_sub();
}

/*********************************************
 * @生成随机字符串
 *********************************************/
const char* create_guid()  
{  
	 CoInitialize(NULL);  
	 static char buf[64] = {0};  
	 GUID guid;  
	 if (S_OK == ::CoCreateGuid(&guid))  
	 {  
	  _snprintf(buf, sizeof(buf), "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"  , 
		guid.Data1  , 
		guid.Data2  , 
		guid.Data3   , 
		guid.Data4[0], 
		guid.Data4[1]  , 
		guid.Data4[2], 
		guid.Data4[3],
		guid.Data4[4], 
		guid.Data4[5]  , 
		guid.Data4[6], 
		guid.Data4[7]  
	   );  
	 }  
	 CoUninitialize();  
	 return (const char*)buf;  
}  




//----消息队列----------------------------
CRITICAL_SECTION queue_lock;
typedef struct _thread_msg{
	int message_id;
	unsigned long lparam;
	unsigned long wparam;
	unsigned int size;

} THREAD_MSG,elemType;

typedef struct nodet   
{  
    elemType *data;  
    struct nodet * next;  
} node_t;            // 节点的结构  
  
typedef struct queuet  
{  
    node_t * head;  
    node_t * tail;  
} queue_t;          // 队列的结构  

queue_t *message_queue = NULL;

void initQueue(queue_t * queue_eg)  
{  
	if( NULL == queue_eg) return;
	InitializeCriticalSection(&queue_lock);
    queue_eg->head = NULL; //队头标志位  
    queue_eg->tail = NULL; //队尾标志位  
}  
   
int enQueue(queue_t *hq, elemType *x)  
{  
	if( hq == NULL || NULL == x)
		return 0;

	EnterCriticalSection(&queue_lock);

    node_t * nnode = new node_t();
	
    if (nnode == NULL )  
    {  
		LeaveCriticalSection(&queue_lock);
        return 0;
    } 

    nnode->data = x;  
    nnode->next = NULL;  
    if (hq->head == NULL)  
    {  
        hq->head = nnode;  
        hq->tail = nnode;  
    } else {  
        hq->tail->next = nnode;  
        hq->tail = nnode;  
    }  
	
	memory_add();

	LeaveCriticalSection(&queue_lock);
    return 1;  
}  
  
void outQueue(queue_t * hq,elemType **temp)  
{  
	if( NULL == hq) return;

	EnterCriticalSection(&queue_lock);
    node_t * p = NULL;  

    if (hq->head == NULL)  
    {  
		*temp = NULL;
		LeaveCriticalSection(&queue_lock);
		return;
    }  

    *temp = hq->head->data;  
    p = hq->head;  
    hq->head = hq->head->next;  
    if(hq->head == NULL)  
    {  
        hq->tail = NULL;  
    }  

    delete p;  
	p  = NULL;
	memory_sub();
	LeaveCriticalSection(&queue_lock);
}  
  
elemType *peekQueue(queue_t * hq)  
{  
	if( NULL == hq ) return NULL;

    if (hq->head == NULL)  
    {  
        return NULL; 
    }   
    return hq->head->data;  
}  
  
int is_emptyQueue(queue_t * hq)  
{  
	if( NULL == hq ) return 1;

    if (hq->head == NULL)  
    {  
        return 1;  
    } else {  
        return 0;  
    }  
}  
    
void clearQueue(queue_t * hq)  
{  
	if( NULL == hq ) return;

    node_t * p = hq->head;  
    while(p != NULL)  
    {  
        hq->head = hq->head->next;  

        delete p; 
		p = NULL;
		memory_sub();
        p = hq->head;  
    }  
    hq->tail = NULL;  
    return;  
}  
 
//----消息队列----------end------------------





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


#define WING_SUCCESS				 1
#define WING_CALLBACK_SUCCESS		 0
#define WING_ERROR_PARAMETER_ERROR	-1
#define WING_ERROR_FAILED			-2
#define WING_NOTICE_IGNORE			-3
#define WING_ERROR_CALLBACK_FAILED  -4
#define WING_ERROR_PROCESS_NOT_EXISTS -5
#define WING_ERROR_WINDOW_NOT_FOUND -6
#define WING_PROCESS_IS_RUNNING		1




/********************************************************************************
 * @ 内存计数器 用于记录申请和释放的次数
 * @ 申请和释放的次数相等 则可以认为没有内存泄漏
 * @ 事实好像没这么简单？why？
 *******************************************************************************/
void memory_times_show(){
	zend_printf("new times : %ld , free times : %ld , need more free : %ld \r\n",memory_add_times,memory_sub_times,(memory_add_times-memory_sub_times));
}

/****************************************************************************
 * @ 创建进程
 * @ param command 要在进程中执行的指令 
 * @ param params_ex 额外参数 用于传输给生成的子进程 默认为 NULL，即不传输参数
 * @ param params_ex_len 额外参数长度 默认为0，即参数为 params_ex = NULL
 ***************************************************************************/
unsigned long create_process(char *command,char *params_ex=NULL,int params_ex_len=0){
	    HANDLE				m_hRead = NULL;
		HANDLE				m_hWrite = NULL;
		STARTUPINFO			sui;    
		PROCESS_INFORMATION pi; // 保存了所创建子进程的信息
		SECURITY_ATTRIBUTES sa;   // 父进程传递给子进程的一些信息
		
		char				*params    = NULL;
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

/********************************************************************************************************
 * @ 进程参数校验，该怎么说这个函数的作用呢，这么说吧，这个函数用来确认是否让阻塞的函数异步执行的
 ********************************************************************************************************/
void command_params_check(char *command_params,int *run_process,int *last_value){
	TSRMLS_FETCH();
	zval **argv = NULL;
	int argc = 0;
	HashTable *arr_hash = NULL;
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
    RETURN_STRING(string,1);
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
	char	*command = NULL;

	command_params_check(command_params,&run_process,&last_value);

	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_thread_count);

	if(!run_process){
		RETURN_LONG(create_process(command,NULL,0));	
		return;
	}

	if(wing_thread_count != last_value){
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}
	
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
			memory_add();

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

				memory_sub();

				data_len+=step;
				
				buf = new char[data_len];
				memory_add();
				
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
				memory_sub();
				
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
	
	char *params	= NULL;
	int	params_len	= 0;
	char *params_ex	= NULL;
	int params_ex_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params,&params_len,&params_ex,&params_ex_len) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	
	char				*command = NULL;
	spprintf(&command, 0, "%s %s\0",PHP_PATH,params);

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
	char				*command = NULL;
	spprintf(&command, 0, "%s %s\0",exe,params);

	RETURN_LONG(create_process(command,params_ex,params_ex_len));	
}
/*******************************************************************
 *@杀死进程
 ******************************************************************/
ZEND_FUNCTION(wing_process_kill)
{
	long process_id = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id) != SUCCESS){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
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
    if (m_hMutex)
    {
        if (ERROR_ALREADY_EXISTS == dwRet)
        {
            CloseHandle(m_hMutex);
			RETURN_LONG(ERROR_ALREADY_EXISTS);
            return;
		}else{
			RETURN_LONG((long)m_hMutex);
		}
    }
   
    CloseHandle(m_hMutex);
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
	memory_add();
	
	ZeroMemory(var,sizeof(var));

	GetEnvironmentVariable(name,var,len);
	ZVAL_STRINGL(return_value,var,len-1,1);

	delete[] var;
	var = NULL;
	memory_sub();
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
				zval_ptr_dtor(&retval_ptr);
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
/*****************************************************************************************
 * @ 关闭socket
 *****************************************************************************************/
#define WM_ONCONNECT		WM_USER+60
#define WM_ACCEPT_ERROR		WM_USER+61
#define WM_ONERROR			WM_USER+62
#define WM_ONCLOSE			WM_USER+63
#define WM_ONRECV			WM_USER+64
#define WM_ONQUIT           WM_USER+65
#define WM_ONCLOSE_EX		WM_USER+66
#define WM_ONSEND			WM_USER+67

#define WM_TEST				WM_USER+68
#define WM_TEST2			WM_USER+69
#define WM_TEST3			WM_USER+70

#define WING_ERROR_CLOSE_SOCKET 4001
#define WING_ERROR_ACCEPT		4002
#define WING_ERROR_MALLOC		4003
#define WING_BAD_ERROR			-4

void _throw_error( int error_code );
VOID CALLBACK MyIOCPThread(DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped);
void _post_msg(int message_id,unsigned long wparam=0,unsigned long lparam=0){
		elemType *msg	= new elemType();  
		if( NULL == msg ) _throw_error(WING_BAD_ERROR);

		memory_add();

		//HANDLE handle = GetCurrentProcess();
		//PROCESS_MEMORY_COUNTERS pmc;
		//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

		msg->message_id = message_id;
		msg->wparam		= wparam;
		msg->lparam		= lparam;	
		msg->size		= 0;//pmc.WorkingSetSize;

		enQueue(message_queue,msg);	
}
BOOL LoadWSAFun(GUID&funGuid,void*& pFun)
{
	//本函数利用参数返回函数指针
	DWORD dwBytes = 0;

	pFun = NULL;

	//随便创建一个SOCKET供WSAIoctl使用 并不一定要像下面这样创建

	SOCKET skTemp = ::WSASocket(AF_INET,SOCK_STREAM, IPPROTO_TCP, NULL,0, WSA_FLAG_OVERLAPPED);

	if(INVALID_SOCKET == skTemp)
	{//通常表示没有正常的初始化WinSock环境

		return FALSE;

	}

       ::WSAIoctl(skTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,

                     &funGuid,sizeof(funGuid),&pFun,

                     sizeof(pFun), &dwBytes, NULL,NULL);

       ::closesocket(skTemp);

       return NULL != pFun;

}

LPFN_DISCONNECTEX lpfnDisconnectEx = NULL;
BOOL DisconnectEx(
	SOCKET hSocket,
	LPOVERLAPPED lpOverlapped,
	DWORD dwFlags,
	DWORD reserved
)
{
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD dwBytes = 0;
	//LPFN_DISCONNECTEX lpfnDisconnectEx;
	if( NULL == lpfnDisconnectEx ){
		if( 0 != WSAIoctl(hSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,sizeof(GuidDisconnectEx),&lpfnDisconnectEx,sizeof(lpfnDisconnectEx),&dwBytes,NULL,NULL)){
			return false;
		}
	}
	return lpfnDisconnectEx(hSocket,lpOverlapped,/*TF_REUSE_SOCKET*/dwFlags,reserved);
}

BOOL WingAcceptEx(
	SOCKET sListenSocket,
	SOCKET sAcceptSocket,
	PVOID lpOutputBuffer,
	DWORD dwReceiveDataLength,
	DWORD dwLocalAddressLength,
	DWORD dwRemoteAddressLength,
	LPDWORD lpdwBytesReceived,
	LPOVERLAPPED lpOverlapped
){
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;     //AcceptEx函数指针
	//Accept function GUID
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	//get acceptex function pointer
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(sListenSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&guidAcceptEx,sizeof(guidAcceptEx),&lpfnAcceptEx,sizeof(lpfnAcceptEx),&dwBytes,NULL,NULL)){
		return false;
	}
	return lpfnAcceptEx( sListenSocket,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped);        
}

void WingGetAcceptExSockaddrs(
		SOCKET sListenSocket,
		PVOID lpOutputBuffer,
		DWORD dwReceiveDataLength,
		DWORD dwLocalAddressLength,
		DWORD dwRemoteAddressLength,
		LPSOCKADDR *LocalSockaddr,
		LPINT LocalSockaddrLength,
		LPSOCKADDR *RemoteSockaddr,
		LPINT RemoteSockaddrLength
	){
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	//get acceptex function pointer
	DWORD dwBytes = 0;
	if( 0 != WSAIoctl(sListenSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,&guidGetAcceptExSockaddrs,sizeof(guidGetAcceptExSockaddrs),&lpfnGetAcceptExSockaddrs,sizeof(lpfnGetAcceptExSockaddrs),&dwBytes,NULL,NULL)){
		return;
	}
	return lpfnGetAcceptExSockaddrs( lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength, dwRemoteAddressLength, LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength);   
}



void _close_socket( SOCKET socket, LPOVERLAPPED lpOverlapped=NULL){
	
	//CancelIo((HANDLE)socket);
	//CancelIoEx((HANDLE)socket,lpOverlapped);
	//shutdown(socket,SD_BOTH);
	if( !DisconnectEx(socket,NULL,TF_REUSE_SOCKET,0) ){
		printf("---------------------------%ld disconnect error------------------------\r\n",socket);
		closesocket(socket);
		//_post_msg(WM_ONERROR,0,WING_ERROR_CLOSE_SOCKET);		
	}
	//closesocket(socket);
	printf("%ld disconnect\r\n",socket);

	

}

void _throw_error( int error_code ){
	exit(WING_BAD_ERROR);
}

/****************************************************
 * @ 投递一次 recv
 ****************************************************/
bool _post_recv(MYOVERLAPPED* &pMyOL){
	
	pMyOL->DATABuf.buf	= pMyOL->m_pBuf;  
	pMyOL->DATABuf.len	= DATA_BUFSIZE;  
	pMyOL->m_iOpType	= OPE_RECV;

	DWORD RecvBytes=0;
	DWORD Flags=0;

	int code = WSARecv(pMyOL->m_skClient,&(pMyOL->DATABuf),1,&RecvBytes,&Flags,&(pMyOL->m_ol),NULL);
	int error_code =  WSAGetLastError();
	//调用一次 WSARecv 触发iocp事件
	if(0 != code && WSA_IO_PENDING != error_code){
		return false;
	}
	return true;
}
/****************************************************
 * @ accept回调 
 * @ 回调之后需要投递一次新的 accept 以待下次新客户端连接
 * @ 还需要头第一次recv 用来接收数据
 ****************************************************/
void _wing_on_accept(MYOVERLAPPED* &pMyOL/* PER_IO_OPERATION_DATA* &perIoData,PER_HANDLE_DATA* &perHandleData*/){

	//LPSOCKADDR addrHost = NULL;      //服务端地址
	//LPSOCKADDR addrClient = NULL;     //客户端地址
	int lenHost = 0;
	int lenClient = 0;

	WingGetAcceptExSockaddrs( pMyOL->m_skClient , pMyOL->m_pBuf , 0 , sizeof(sockaddr_in) + 16 , sizeof(sockaddr_in) + 16, (LPSOCKADDR*) &pMyOL->addrServer , &lenHost , (LPSOCKADDR*) &pMyOL->addrClient , &lenClient );

	
	// 设置超时时间 某些短连接协议 就很有用 貌似没效果
	if( pMyOL->timeout > 0 ){
		::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_SNDTIMEO, (const char*)&pMyOL->timeout,sizeof(pMyOL->timeout));
		::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_RCVTIMEO, (const char*)&pMyOL->timeout,sizeof(pMyOL->timeout));
	}
	

	linger so_linger;
	so_linger.l_onoff = TRUE;
	so_linger.l_linger = 3; //强制closesocket后 设置允许3秒逗留时间 防止数据丢失
	setsockopt(pMyOL->m_skClient,SOL_SOCKET,SO_LINGER,(const char*)&so_linger,sizeof(so_linger));

	int nRet = ::setsockopt(pMyOL->m_skClient, SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(const char *)&m_sockListen,sizeof(m_sockListen));
	
	_post_msg(WM_ONCONNECT, pMyOL->m_skClient,0);

	
	printf("%ld onconnect\r\n", pMyOL->m_skClient);

	_post_recv(pMyOL);
}
/****************************************************
 * @ 发送消息
 ****************************************************/
void _wing_on_send( MYOVERLAPPED*  pOL){
	/*ZeroMemory(perIoData,sizeof(PER_IO_OPERATION_DATA));

			//CloseHandle(PerIOData->OVerlapped.hEvent);
			GlobalFree( perIoData ); 

			perIoData = NULL;
		

			memory_sub();

			_post_msg(WM_ONSEND,perHandleData->Socket);*/
}
void _wing_on_close(MYOVERLAPPED*  &pMyOL);
void _wing_on_recv(MYOVERLAPPED*  &pOL){
	

	printf("recv: %s\r\n", pOL->m_pBuf);
	send(pOL->m_skClient,"hello",strlen("hello"),0);
	//_close_socket(pOL->m_skClient);
	printf("%ld onconnect\r\n", pOL->m_skClient);
	_wing_on_close(pOL);
	//构建消息
	//RECV_MSG *recv_msg	= new RECV_MSG();   
	//ZeroMemory(recv_msg,sizeof(RECV_MSG));

	//DWORD len = pOL->recvBytes+1;
	//recv_msg->msg		= new char[len];
	//ZeroMemory(recv_msg->msg,len);

	//strcpy_s(recv_msg->msg,len,pOL->m_pBuf);
//	recv_msg->len		=  len;

	//内存计数器 此处new了两次
	//memory_add();
	//memory_add();
	
	//发送消息
	//_post_msg(WM_ONRECV,pOL->m_skClient,(unsigned long)recv_msg);
	//_post_recv(pOL);
}

void _wing_on_close(MYOVERLAPPED*  &pMyOL){
	
	//此处应该修改为 ON_DISCONNECT
	_post_msg(WM_ONCLOSE,pMyOL->m_skClient);

	DisconnectEx(pMyOL->m_skClient,NULL,TF_REUSE_SOCKET,0);

	//创建一个自定义的OVERLAPPED扩展结构，使用IOCP方式调用

	//MYOVERLAPPED *pMyOL	= new MYOVERLAPPED();
	pMyOL->m_iOpType	= OPE_ACCEPT;        //AcceptEx操作
	//pMyOL->m_skServer	= pMyOL->m_skServer;
	//pMyOL->m_skClient	= pMyOL->m_skClient;
	//ZeroMemory(&(pMyOL->m_ol),sizeof(OVERLAPPED)); 
	ZeroMemory(pMyOL->m_pBuf,sizeof(char)*DATA_BUFSIZE);

	int error_code = WingAcceptEx(m_sockListen,pMyOL->m_skClient,pMyOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pMyOL);
	int last_error = WSAGetLastError() ;
	if( !error_code && WSAECONNRESET != last_error && ERROR_IO_PENDING != last_error ){
		if( INVALID_SOCKET != pMyOL->m_skClient ) 
		{
			printf("------------onerror----------close socket-------------------");
			closesocket(pMyOL->m_skClient);
		}	
		if( NULL != pMyOL) 
		{
			printf("------------onerror----------delete pMyOL-------------------");
			delete pMyOL;
			pMyOL = NULL; 
		}
		//delete pOL;
		return;
	}

	//注意在这个SOCKET被重新利用后，后面的再次捆绑到完成端口的操作会返回一个已设置//的错误，这个错误直接被忽略即可
	::BindIoCompletionCallback((HANDLE)pMyOL->m_skClient,MyIOCPThread, 0);
	printf("%ld onclose\r\n", pMyOL->m_skClient);
	//delete pOL;

}


VOID CALLBACK MyIOCPThread(DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped)
{//IOCP回调函数
	if( NULL == lpOverlapped )
	{//没有真正的完成
		SleepEx(20,TRUE);//故意置成可警告状态
		return;
	}

	int error_code = WSAGetLastError();
	//找回“火车头”以及后面的所有东西
	MYOVERLAPPED*  pOL = CONTAINING_RECORD(lpOverlapped, MYOVERLAPPED, m_ol);

	if( 0 != dwErrorCode ){
		if( 0 == dwBytesTrans || 10054 == error_code || 64 == error_code){
					_wing_on_close(pOL);
				}
	}

	switch(pOL->m_iOpType)
	{
		case OPE_ACCEPT: //AcceptEx结束
		{//有链接进来了 SOCKET句柄就是 pMyOL->m_skClient
			_wing_on_accept(pOL);
		}
		break;
		case OPE_RECV:
		{
			if( 0 == dwBytesTrans || 10054 == error_code || 64 == error_code){
					_wing_on_close(pOL);
				}else{
				pOL->recvBytes = dwBytesTrans;
				_wing_on_recv(pOL);
			}
			
		}
		break;
		case OPE_SEND:
		{
			_wing_on_send(pOL);
		}
		break;

	}

}//end fun

//iocp 服务主线程
ZEND_FUNCTION(wing_service){
	//InitializeCriticalSection(&bytes_lock);
	
	zval *onreceive = NULL;
	zval *onconnect = NULL;
	zval *onclose = NULL;
	zval *onerror = NULL;
	zval *call_cycle=NULL;
	zval *_params = NULL;
	int port = 0;
	//zval *_port;
	//zval *_listen_ip;
	char *listen_ip = NULL;
	int timeout = 0;

	MAKE_STD_ZVAL(onreceive);
	MAKE_STD_ZVAL(onconnect);
	MAKE_STD_ZVAL(onclose);
	MAKE_STD_ZVAL(onerror);


	//参数获取
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&_params) != SUCCESS) {
		
			RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
			return;
	}
	//如果参数不是数组
	if(Z_TYPE_P(_params) != IS_ARRAY){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	
	HashTable *arr_hash = Z_ARRVAL_P(_params);
	int argc = zend_hash_num_elements(arr_hash);
	zval  **data = NULL;
	HashPosition pointer = NULL;
			
	//数组参数解析
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
			//call_cycle
			else if(strcmp(key,"call_cycle")==0){
				call_cycle = *data;
			}
			else if(strcmp(key,"timeout")==0){
				timeout = Z_LVAL_PP(data);
			}

        } 
    } 

	//初始化消息队列
	message_queue= new queue_t();
	
	if( NULL == message_queue )
		exit(WING_BAD_ERROR);

    initQueue(message_queue);  

	//初始化Socket
	WSADATA wsaData; 
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{
		RETURN_LONG(WING_ERROR_FAILED);
		return; 
	}


	 if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){// 检查是否申请了所需版本的套接字库   
        WSACleanup();  
        return;  
    }  

	

	//WSACleanup( );
	//初始化Socket
	// 这里需要特别注意，如果要使用重叠I/O的话，这里必须要使用WSASocket来初始化Socket
	// 注意里面有个WSA_FLAG_OVERLAPPED参数
	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if(m_sockListen == INVALID_SOCKET){
		WSACleanup();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	BOOL   bReuse=TRUE;
	::BindIoCompletionCallback((HANDLE)m_sockListen,MyIOCPThread, 0);
	::setsockopt(m_sockListen,SOL_SOCKET,SO_REUSEADDR,(LPCSTR)&bReuse,sizeof(BOOL));

	struct sockaddr_in ServerAddress; 
	// 填充地址结构信息
	ZeroMemory(&ServerAddress, sizeof(ServerAddress)); 
	ServerAddress.sin_family = AF_INET; 
	// 这里可以选择绑定任何一个可用的地址，或者是自己指定的一个IP地址
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                     
	ServerAddress.sin_addr.s_addr = inet_addr(listen_ip);          
	ServerAddress.sin_port = htons(port);   


	// 绑定端口
	if (SOCKET_ERROR == bind(m_sockListen, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))){
		_close_socket(m_sockListen);
		WSACleanup();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}  

	// 开始监听
	if( 0 != listen(m_sockListen,SOMAXCONN)){
		_close_socket(m_sockListen);
		WSACleanup();
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	zval *sockets;
	MAKE_STD_ZVAL(sockets);
	array_init(sockets);
	/*add_assoc_string(return_value,"sin_addr",inet_ntoa(addr_conn.sin_addr),1);
    add_assoc_long(return_value,"sin_family",addr_conn.sin_family);
	add_assoc_long(return_value,"sin_port",addr_conn.sin_port);
	add_assoc_string(return_value,"sin_zero",addr_conn.sin_zero,1);*/



	//socket 池
	for( int i=0;i<100;i++){
	
		SOCKET client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
		
		if( INVALID_SOCKET == client ) continue;

		BindIoCompletionCallback((HANDLE)client ,MyIOCPThread,0);

		MYOVERLAPPED *pMyOL = new MYOVERLAPPED();
		DWORD dwBytes=0;
		ZeroMemory(pMyOL,sizeof(MYOVERLAPPED));
		
		pMyOL->m_iOpType	= OPE_ACCEPT;
		pMyOL->m_skServer	= m_sockListen;
		pMyOL->m_skClient	= client;
		pMyOL->timeout		= timeout;
		pMyOL->addrClient   = NULL;
		pMyOL->addrServer   = NULL;

		int error_code = WingAcceptEx(m_sockListen,pMyOL->m_skClient,pMyOL->m_pBuf,0,sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,NULL, (LPOVERLAPPED)pMyOL);
		int last_error = WSAGetLastError() ;
		if( !error_code && WSAECONNRESET != last_error && ERROR_IO_PENDING != last_error ){
			if( INVALID_SOCKET != pMyOL->m_skClient ) 
			{
				closesocket(pMyOL->m_skClient);
				client = pMyOL->m_skClient = INVALID_SOCKET;
			}
				
			if( NULL != pMyOL) 
			{
				delete pMyOL;
				pMyOL = NULL; 
			}
			continue;
		}

		add_index_long(sockets,(unsigned long)client,(unsigned long)pMyOL);
	}

	int times = 0;
	int nSize = 0;
	elemType *msg = NULL;//消息
	

	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
    
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	unsigned int begin_size = pmc.WorkingSetSize;
    zend_printf("size:%d\r\n",pmc.WorkingSetSize);

	unsigned int _begin_size = pmc.WorkingSetSize;

	while( true )
	{ 
		
		//判断是否有消息
		if(is_emptyQueue(message_queue)){
			Sleep(10);
			continue;
		}
		//_CrtMemCheckpoint( &s1 );
		 
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
		begin_size = pmc.WorkingSetSize;
		zend_printf("start memory:%d\r\n",pmc.WorkingSetSize);


		outQueue(message_queue,&msg);

		if( NULL == msg ){

			Sleep(10);
			continue;

		}

		//根据消息ID进行不同的处理
		switch(msg->message_id){
			//新的连接
			case WM_ONCONNECT:
			{
				zend_printf("===================================onconnect===================================\r\n");
				
				zval *params = NULL;
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params);
				ZVAL_LONG(params,(long)msg->wparam);//socket资源
				MAKE_STD_ZVAL(retval_ptr);
							 
				if( SUCCESS != call_user_function(EG(function_table),NULL,onconnect,retval_ptr,1,&params TSRMLS_CC) ){
					zend_error(E_USER_WARNING,"onconnect call_user_function fail");
				}
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);
						  
			}
			break;
			case WM_ONSEND:{
				zend_printf("onsend\r\n");
					//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
					//zend_printf("size-onsend:%d\r\n",pmc.WorkingSetSize-msg->size);	   
			}break;
			//目前暂时没有用到 先留着
			case WM_ACCEPT_ERROR:
			{
				zend_printf("onaccepterror\r\n");
				//zend_printf("accept error\r\n");
				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-accepterror:%d\r\n",pmc.WorkingSetSize-msg->size);	
			}
			break;
			//收到消息
			case WM_ONRECV:
			{
				
				zend_printf("onrecv\r\n");
				
				RECV_MSG *temp = (RECV_MSG*)msg->lparam;
				SOCKET client = (SOCKET)msg->wparam;

				zval *params[2] = {0};
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				MAKE_STD_ZVAL(retval_ptr);

				ZVAL_LONG(params[0],(long)client);
				ZVAL_STRINGL(params[1],temp->msg,temp->len,1);

				//zend_printf("2-onrecv\r\n");

				if( SUCCESS != call_user_function(EG(function_table),NULL,onreceive,retval_ptr,2,params TSRMLS_CC) ){
					zend_error(E_USER_WARNING,"onreceive call_user_function fail");
				}
				//zend_printf("30-onrecv\r\n");

				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
			//	zend_printf("3-onrecv\r\n");

				delete[] temp->msg;
				temp->msg = NULL;

				delete temp;
				temp = NULL;

				memory_sub();
				memory_sub();

				//zend_printf("onrecv end\r\n");

				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-onrecv:%d\r\n",pmc.WorkingSetSize-msg->size);	

				//new_add_size+=pmc.WorkingSetSize-msg->size;
			}
			break;
			//调用 _close_socket 服务端主动关闭socket
			case WM_ONCLOSE_EX:{
				zend_printf("oncloseex\r\n");
				//zend_printf("close ex\r\n");
				_close_socket((SOCKET)msg->wparam);
				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-onclose ex:%d\r\n",pmc.WorkingSetSize-msg->size);	
			}break;
			case WM_TEST:
			{
				zend_printf("ontest\r\n");
					//zend_printf("test---------------------------------------:%d\r\n",msg->size);	
			}
			case WM_TEST2:
			{
				zend_printf("ontest2\r\n");
					//zend_printf("test-2222---------------------------------------:%d\r\n",msg->size);	
			}
			case WM_TEST3:
			{
				zend_printf("ontest3\r\n");
					//zend_printf("test-3333---------------------------------------:%d\r\n",msg->size);	
			}
			break;
			//客户端掉线了
			case WM_ONCLOSE:
			{
				zend_printf("===================================onclose===================================\r\n");	

				
				SOCKET client =(SOCKET)msg->wparam;

				zval *params = NULL;
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params);
				ZVAL_LONG(params,(long)client);
				MAKE_STD_ZVAL(retval_ptr);
	 
				if(SUCCESS != call_user_function(EG(function_table),NULL,onclose,retval_ptr,1,&params TSRMLS_CC)){
					zend_error(E_USER_WARNING,"WM_ONCLOSE call_user_function fail\r\n");
				}
							 
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);

				//zend_printf("onclose end\r\n");

				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-onclose:%d\r\n",pmc.WorkingSetSize-msg->size);	
				//new_add_size = 0;
			}
			break; 
			//发生错误 目前暂时也还没有用到
			case WM_ONERROR:{
				zend_printf("onerror\r\n");		
				//zend_printf("------------------------------------onerror----warning-------------------------------\r\n");
				
				SOCKET client =(SOCKET)msg->wparam;

				zval *params = NULL;
				zval *retval_ptr = NULL;

				MAKE_STD_ZVAL(params);

				//ZVAL_STRING(params[1],(char*)msg.lParam,1);
				ZVAL_LONG(params,(long)msg->lparam);

				MAKE_STD_ZVAL(retval_ptr);
							 
				if(SUCCESS != call_user_function(EG(function_table),NULL,onerror,retval_ptr,1,&params TSRMLS_CC)){
					zend_error(E_USER_WARNING,"onerror call_user_function fail");
				}
							 
				zval_ptr_dtor(&retval_ptr);
				zval_ptr_dtor(&params);
			

				//zend_printf("onerror end\r\n");

				//GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
				//zend_printf("size-onerror:%d\r\n",pmc.WorkingSetSize-msg->size);	
				//
			}
			break;
			//退出服务 暂时没有测试
			case WM_ONQUIT:
			{
				zend_printf("onquit\r\n");
				
				//PostQueuedCompletionStatus(m_hIOCompletionPort, 0xFFFFFFFF, 0, NULL);
				
				CloseHandle(m_hIOCompletionPort);
				_close_socket(m_sockListen);
				
				WSACleanup();
				clearQueue(message_queue);
				DeleteCriticalSection(&queue_lock);

				delete msg;
				delete message_queue;

				msg = NULL;
				message_queue = NULL;

				//zend_printf("quit end\r\n");
				RETURN_LONG(WING_SUCCESS);
				return;
			}break;

		}

		delete msg;
		msg = NULL;

		memory_sub();

		//显示内存申请 释放次数对比
		memory_times_show();
		
		
		//_CrtMemCheckpoint( &s2 );
		//if ( _CrtMemDifference( &s3, &s1, &s2) )
	//	{
			//zend_printf("memory crash\r\n");
		//	_CrtMemDumpStatistics( &s3 );//内存泄露
	//	}
	//	else {
			//zend_printf("memory not crash\r\n");
	//	}

		//call_cycle
		/*zval *retval_ptr = NULL;
		MAKE_STD_ZVAL(retval_ptr);				 
		call_user_function(EG(function_table),NULL,call_cycle,retval_ptr,0,NULL TSRMLS_CC);
		zval_ptr_dtor(&retval_ptr);*/

		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
		zend_printf("free memory:%d\r\n",pmc.WorkingSetSize-begin_size);
		zend_printf("add memory:%d\r\n\r\n",pmc.WorkingSetSize-_begin_size);
		zend_printf("--------------------------------------------------------------------------------\r\n\r\n");
  
    } 
	zend_printf("service quit\r\n");
	RETURN_LONG(WING_SUCCESS);
}
/***********************************
 * @停止服务
 ***********************************/
ZEND_FUNCTION(wing_service_stop){
	//_post_msg(WM_ONQUIT);
	RETURN_LONG(WING_SUCCESS);
}

/********************************************
 * @ 关闭socket
 * @ param socket
 ********************************************/
ZEND_FUNCTION(wing_close_socket){

	zval *socket = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	convert_to_long(socket);
	
	//_post_msg(WM_ONCLOSE_EX,Z_LVAL_P(socket));

	RETURN_LONG(WING_SUCCESS);
}

/*****************************************
 * @获取socket信息，ip 协议 端口 等
 * @return array //GetAcceptExSockaddrs
 ****************************************/
ZEND_FUNCTION(wing_socket_info){

	zval *socket = NULL;
	MAKE_STD_ZVAL(socket);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",&socket) != SUCCESS) {
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
	add_assoc_long(return_value,"sin_port",addr_conn.sin_port);
	add_assoc_string(return_value,"sin_zero",addr_conn.sin_zero,1);
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
}  
/***************************************************
 * @ 使用iocp异步发送消息
 ***************************************************/ 
ZEND_FUNCTION(wing_socket_send_msg_ex){
	zval *socket = NULL;
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
   
	PER_IO_OPERATION_DATA  *PerIoData =  (PER_IO_OPERATION_DATA*)GlobalAlloc(GPTR,sizeof(PER_IO_OPERATION_DATA));//new PER_IO_OPERATION_DATA();
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
	RETURN_LONG(WING_SUCCESS);
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
	ZEND_FALIAS(wing_socket,wing_service,NULL)

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
