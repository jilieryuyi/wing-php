#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "zend_constants.h"
#include "ext/standard/info.h"
#include "php_wing.h"
#include "lib/wing_lib.h"



static int wing_thread_count = 0;
static int wing_timer_count  = 0;

/****************************************************************************
 * @ 创建进程
 * @ param command 要在进程中执行的指令 
 * @ param params_ex 额外参数 用于传输给生成的子进程 默认为 NULL，即不传输参数
 * @ param params_ex_len 额外参数长度 默认为0，即参数为 params_ex = NULL
 ***************************************************************************/
unsigned long create_process( char *command, char *params_ex, int params_ex_len ){
	    
	HANDLE m_hRead         = NULL;
	HANDLE m_hWrite        = NULL;
	STARTUPINFO sui;    
	PROCESS_INFORMATION pi;                        // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;                        // 父进程传递给子进程的一些信息
		
	char *params     = NULL;
	int   params_len = 0;
    
	sa.bInheritHandle       = TRUE;                // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
	sa.lpSecurityDescriptor = NULL;
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	
	if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
	{
		return WING_ERROR_FAILED;
	}

	ZeroMemory(&sui, sizeof(STARTUPINFO));         // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
	
	sui.cb         = sizeof(STARTUPINFO);
	sui.dwFlags	   = STARTF_USESTDHANDLES;  
	sui.hStdInput  = m_hRead;
	sui.hStdOutput = m_hWrite;
	sui.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
		
	if( params_ex_len >0 && params_ex != NULL ) {
		DWORD d;
		if( ::WriteFile(m_hWrite,params_ex,params_ex_len,&d,NULL) == FALSE ) {
			//告警
			zend_error(E_USER_WARNING,"write params to process error");
		}
	}

	if ( !CreateProcess(NULL,command, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi ) ) {
		CloseHandle(m_hRead);
		CloseHandle(m_hWrite);
		return WING_ERROR_FAILED;
	}
		
	CloseHandle(m_hRead);
	CloseHandle(m_hWrite);
	CloseHandle(pi.hProcess);  // 子进程的进程句柄
	CloseHandle(pi.hThread);   // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行
		
	return pi.dwProcessId;	
}

/********************************************************************************************************
 * @ 进程参数校验，该怎么说这个函数的作用呢，这么说吧，这个函数用来确认是否让阻塞的函数异步执行的
 ********************************************************************************************************/
void command_params_check(char* &command_params,int *run_process,int *last_value TSRMLS_DC){
	
	zval **argv         = NULL;
	int argc            = 0;
	HashTable *arr_hash = NULL;
	command_params      = (char*)emalloc(1024);

	memset( command_params , 0 ,1024 );
	
	//char *target = NULL;
	//获取命令行参数
	if ( zend_hash_find( &EG(symbol_table), "argv", sizeof("argv"), (void**)&argv) == SUCCESS ){

		zval  **data         = NULL;
		HashPosition pointer = NULL;
		arr_hash	         = Z_ARRVAL_PP(argv);
		argc		         = zend_hash_num_elements(arr_hash);

		for( zend_hash_internal_pointer_reset_ex(arr_hash, &pointer); 
			 zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS; 
			 zend_hash_move_forward_ex(arr_hash, &pointer)
		) {
			
			if( strcmp((char*)Z_LVAL_PP(data),"wing-process") == 0 ){
				*run_process = 1;
			}

			char *key   = NULL;
			int key_len = 0 ;
			int index   = 0;

			zend_hash_get_current_key_ex(arr_hash, &key, (uint*)&key_len, (ulong*)&index, 0, &pointer);

			if( index > 0 ) {
				char *p = (char*)Z_LVAL_PP(data);
				if(p[0]!='\"')
					sprintf(command_params,"%s \"%s\" ",command_params,p);
					//spprintf(&command_params,0,"%s \"%s\" ",command_params,p);
				else 
					sprintf(command_params,"%s %s ",command_params,p);
					//spprintf(&command_params,0,"%s %s ",command_params,p);
			}

			if(index == argc-1&&last_value != NULL){
				 *last_value= atoi((char*)Z_LVAL_PP(data));
			}
		} 
	}
}



/***************************************************************
 *@wait process进程等待
 *@param process id 进程id
 *@param timeout 等待超时时间 单位毫秒
 *@return exit code 进程退出码
 ***************************************************************/
PHP_FUNCTION(wing_process_wait){
	
	int thread_id,timeout = INFINITE;
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &thread_id, &timeout ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( thread_id<=0 ) {
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	HANDLE handle     = OpenProcess(PROCESS_ALL_ACCESS, FALSE, thread_id);
	DWORD wait_result = 0;
	DWORD wait_status = WaitForSingleObject(handle,timeout);
	 
	if( wait_status != WAIT_OBJECT_0 ) {
		CloseHandle( handle );
		RETURN_LONG(wait_status);
		return;
	}
	if( GetExitCodeProcess(handle,&wait_result) == 0 ) {
		CloseHandle( handle );
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	CloseHandle( handle );
	RETURN_LONG(wait_result);
	return;
}


/******************************************************************
 *@创建多线程，使用进程模拟
 *****************************************************************/
PHP_FUNCTION( wing_create_thread ){
	
	wing_thread_count++;

	zval *callback   = NULL;
	zval *retval_ptr = NULL;
	
	MAKE_STD_ZVAL( callback );

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback ) != SUCCESS ) {
		zval_ptr_dtor(&callback);
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	char *command_params = NULL;
	int run_process      = 0;
	int command_index    = 0;
	int last_value       = 0;
	char *command        = NULL;
	unsigned long pid    = 0; 

	command_params_check( command_params, &run_process, &last_value TSRMLS_CC );

	spprintf( &command, 0, "%s %s %s wing-process %ld", PHP_PATH , zend_get_executed_filename( TSRMLS_C ), command_params, wing_thread_count );

	if( !run_process ) {
		pid = create_process( command, NULL, 0 );
		
		efree(command);
		efree(command_params);
		zval_ptr_dtor(&callback);

		RETURN_LONG(pid);	
		return;
	}

	if( wing_thread_count != last_value ) {
		
		efree(command);
		efree(command_params);
		zval_ptr_dtor(&callback);

		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}
	
	efree(command);
	efree(command_params);
			
	MAKE_STD_ZVAL(retval_ptr);

	if( SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC ) ){
		
		zval_ptr_dtor(&callback);
		zval_ptr_dtor(&retval_ptr);

		RETURN_LONG(WING_ERROR_CALLBACK_FAILED);
		return;
	}

	zval_ptr_dtor(&callback);
	zval_ptr_dtor(&retval_ptr);
			
	RETURN_LONG(WING_CALLBACK_SUCCESS);
}



/*************************************************************************
 *@get data from create process
 *@从父进程获取数据 这里使用了一个小伎俩，性能，待考量
 *return string or null
 *************************************************************************/
ZEND_FUNCTION(wing_get_process_params){
	
	HANDLE m_hRead	 = GetStdHandle(STD_INPUT_HANDLE);
	DWORD data_len	 = 1024;
	int step		 = 1024;
	char *buf		 = new char[data_len];
	DWORD dwRead     = 0;
	DWORD lBytesRead = 0;

	ZeroMemory( buf, data_len );
	
	if( !PeekNamedPipe( m_hRead,buf, data_len, &lBytesRead, 0, 0 ) ) {
		
		delete[] buf;
		buf = NULL;
		RETURN_NULL();
		return;
	}

	if( lBytesRead <= 0 ){

		delete[] buf;
		buf = NULL;
		RETURN_NULL();
		return;
	}

	while( lBytesRead >= data_len ){

		delete[] buf;
		buf = NULL;
		data_len += step;
				
		buf = new char[data_len];
				
		ZeroMemory( buf, data_len );
		if( !PeekNamedPipe( m_hRead,buf, data_len, &lBytesRead, 0, 0 ) ){

			delete[] buf;
			buf = NULL;
			RETURN_NULL();
			return;
		}
	}
				
	if ( ReadFile(m_hRead, buf, lBytesRead+1, &dwRead, NULL ) ) {
		// 从管道中读取数据 
		ZVAL_STRINGL( return_value, buf, dwRead, 1 );
		delete[] buf;
		buf = NULL;	
		return;
	}
	RETURN_NULL();
	return;
}


/*****************************************************************
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 ****************************************************************/
PHP_FUNCTION(wing_create_process){

	char *exe           = NULL;   //创建进程必须的可执行文件 一般为 exe bat等可以直接运行的文件
	int	  exe_len       = 0;
	char *params        = NULL;   //可选命令行参数
	int	  params_len    = 0;
	char *params_ex     = NULL;   //需要传递到子进程能的参数 通过api wing_get_process_params 获取
	int   params_ex_len = 0;
	char *command       = NULL;   //创建进程的参数
	unsigned long pid   = 0;      //进程id

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &exe, &exe_len, &params, &params_len, &params_ex, &params_ex_len ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	
	spprintf( &command, 0, "%s %s\0", exe, params );

	pid = create_process( command, params_ex, params_ex_len );

	efree(command);
	RETURN_LONG(pid);	
	return;
}



/*******************************************************************
 *@创建进程，把一个php文件直接放到一个进程里面执行
 *@param command path 程序路径
 *@param command params 命令行参数
 ********************************************************************/
PHP_FUNCTION( wing_create_process_ex ){
	
	char *params         = NULL;
	int	  params_len     = 0;
	char *params_ex	     = NULL;
	int   params_ex_len  = 0;
	char *command        = NULL;
	int   pid            = 0;

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &params, &params_len, &params_ex, &params_ex_len ) != SUCCESS ) {
		 RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		 return;
	}
	
	
	spprintf( &command, 0, "%s %s\0", PHP_PATH, params );
	pid = create_process( command, params_ex, params_ex_len );

	efree(command);
	RETURN_LONG(pid);	
	return;
}


/*******************************************************************
 *@杀死进程
 ******************************************************************/
ZEND_FUNCTION(wing_process_kill)
{
	long process_id = 0;
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,process_id);
    if( hProcess == NULL ){
		RETURN_LONG(WING_ERROR_PROCESS_NOT_EXISTS);
        return;
	}
    if( !TerminateProcess( hProcess , 0 ) ) {
		CloseHandle(hProcess);
		RETURN_LONG(WING_ERROR_FAILED);
        return;
	}
	CloseHandle(hProcess);
    RETURN_LONG(WING_SUCCESS);
	return;
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

	char *mutex_name     = NULL;
	int   mutex_name_len = 0;
	
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s", &mutex_name, &mutex_name_len ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( mutex_name_len <= 0 ){
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	/*---跨边界共享内核对象需要的参数 这里不用
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;*/

	HANDLE m_hMutex =  CreateMutex(NULL,TRUE,mutex_name);//CreateMutex(&sa,TRUE,mutex_name);

    if ( !m_hMutex ) {
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

    if ( ERROR_ALREADY_EXISTS == GetLastError() ) {
         CloseHandle(m_hMutex);
		 RETURN_LONG( ERROR_ALREADY_EXISTS );
         return;
	}

	RETURN_LONG( (long)m_hMutex );
	return;
}


/****************************************************************
 *@关闭互斥量
 ****************************************************************/
ZEND_FUNCTION(wing_close_mutex){
	long mutex_handle = 0;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&mutex_handle) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( mutex_handle <= 0 ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	int status_code = CloseHandle((HANDLE)mutex_handle) ? WING_SUCCESS : WING_ERROR_FAILED ;
	RETURN_LONG( status_code );
	return;
}


/*****************************************************************************************************
 *@检测进程是否存活--实际意义不大，因为进程id重用的特性 进程退出后 同样的进程id可能立刻被重用
 *****************************************************************************************************/
ZEND_FUNCTION(wing_process_isalive)
{
	long process_id = 0;
	HANDLE hProcess = NULL;
	
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,"l",&process_id) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( process_id <= 0 ) {
		RETURN_LONG( WING_ERROR_PARAMETER_ERROR );
		return;
	}

    hProcess = OpenProcess( PROCESS_TERMINATE , FALSE , process_id );
    
	if( hProcess == NULL ){
		RETURN_LONG(WING_ERROR_PROCESS_NOT_EXISTS);
        return;
	}

	CloseHandle( hProcess );
	RETURN_LONG(WING_PROCESS_IS_RUNNING);
	return;
}


/***************************************************************************************************
 *@获取环境变量
 **************************************************************************************************/
ZEND_FUNCTION(wing_get_env){
	
	char *name     = NULL;
	int   name_len = 0;
	int   size     = 0;
	char *var      = NULL;
	
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,"s", &name, &name_len ) != SUCCESS ) {
		RETURN_EMPTY_STRING();
		return;
	}

	size = GetEnvironmentVariable(name,NULL,0);

	if( size<=0 || GetLastError() == ERROR_ENVVAR_NOT_FOUND ) {
		RETURN_EMPTY_STRING();
		return;
	}
	
	var = new char[size];  
	
	ZeroMemory( var , size );

	size = GetEnvironmentVariable(name,var,size);

	if (size == 0) {
		delete[] var;
		var = NULL;
		RETURN_EMPTY_STRING();
		return;
	}

	ZVAL_STRINGL( return_value, var, size, 1 );

	delete[] var;
	var = NULL;
	return;
}

/****************************************************************************************************
 * @ 设置环境变量，子进程和父进程可以共享，可以简单用作进程间的通信方式
 ***************************************************************************************************/
ZEND_FUNCTION(wing_set_env){

	char *name     = NULL;
	zval *value    = NULL;
	int   name_len = 0;
	int   res      = 0;

	MAKE_STD_ZVAL(value);
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz", &name, &name_len, &value) != SUCCESS ) {
		zval_ptr_dtor( &value );
		RETURN_LONG( WING_ERROR_PARAMETER_ERROR );
		return;
	}
	convert_to_string(value);
	
	res = SetEnvironmentVariableA( name,(LPCTSTR)Z_STRVAL_P(value) ) ? WING_SUCCESS : WING_ERROR_FAILED;
	zval_ptr_dtor(&value);

	RETURN_LONG( res );
}


/**************************************************************************************************
 * @ 获取一个命令所在的绝对文件路径
 * @ 比如说获取php的安装目录，不过wing php里面可以直接使用常量 WING_PHP 代表的书php的安装路径
 *************************************************************************************************/
ZEND_FUNCTION( wing_get_command_path ){ 

	char *name     = 0;
	int   name_len = 0;
	char *path     = NULL;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len ) != SUCCESS ) {
		RETURN_EMPTY_STRING();
		return;
	}
	
	path = (char*)emalloc(MAX_PATH);
	
	get_command_path( (const char*)name, path );
	
	RETURN_STRING(path,0);
	return;
}




/********************************************************************************
 *@毫秒级别定时器
 *@author yuyi
 *@created 2016-05-15
 ********************************************************************************/
ZEND_FUNCTION(wing_timer){
	
	wing_timer_count++;
	
	zval *callback         = NULL;
	zval *dwMilliseconds   = NULL;
	int   max_run_times    = 0;
	long  accuracy         = 10000; //设置相对时间为1秒 1000 0000。
	char *command_params   = NULL;
	int   run_process      = 0;
	int   command_index	   = 0;
	int   last_value	   = 0;
	char *command          = NULL;
	unsigned long pid      = 0;

	MAKE_STD_ZVAL( callback );
	MAKE_STD_ZVAL( dwMilliseconds );

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|lll",&dwMilliseconds, &callback,&max_run_times,&accuracy) != SUCCESS ) {
		 zval_ptr_dtor( &callback );
		 zval_ptr_dtor( &dwMilliseconds ); 
		 RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		 return;
	}

	convert_to_long_ex(&dwMilliseconds);
	command_params_check(command_params,&run_process,&last_value TSRMLS_CC);
	spprintf(&command, 0, "%s %s %s wing-process %ld",PHP_PATH, zend_get_executed_filename(TSRMLS_C),command_params,wing_timer_count);

	if(!run_process){
		pid = create_process(command,NULL,0);
		efree( command );
		efree( command_params );
		zval_ptr_dtor( &callback );
		zval_ptr_dtor( &dwMilliseconds );
		RETURN_LONG(pid);	
		return;
	}

	if(wing_timer_count!=last_value){
		efree( command );
		efree( command_params );
		zval_ptr_dtor( &callback );
		zval_ptr_dtor( &dwMilliseconds );
		RETURN_LONG(WING_NOTICE_IGNORE);
		return;
	}

	HANDLE hTimer             = NULL;
	LARGE_INTEGER liDueTime;
	zval *retval_ptr          = NULL;
	int times                 = 0;
	LONGLONG time             = (-1)*accuracy*Z_LVAL_P(dwMilliseconds);
    char *timername           = NULL;
	liDueTime.QuadPart        = time; //设置相对时间为1秒 10000000。
	
	
	spprintf(&timername,0,"wing_waitable_timer-%s",create_guid());
    hTimer = CreateWaitableTimer(NULL, TRUE, timername);  //创建定时器。
	
	efree(timername);
	efree(command);
	efree( command_params );

    if( !hTimer ) {       
		 zval_ptr_dtor( &callback );
		 zval_ptr_dtor( &dwMilliseconds );
		 RETURN_LONG(WING_ERROR_FAILED);	
		 return;
    }
 
    if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0)) {         
         CloseHandle(hTimer);
		 zval_ptr_dtor( &callback );
		 zval_ptr_dtor( &dwMilliseconds );
		 RETURN_LONG(WING_ERROR_FAILED);	
         return;
    }
 
    //等定时器有信号。
	while(true)
	{
		if ( WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0) {
			 CloseHandle(hTimer);
			 zval_ptr_dtor( &callback );
		     zval_ptr_dtor( &dwMilliseconds );
			 RETURN_LONG(WING_ERROR_FAILED);	
			 return;
		}
		else 
		{
			//时钟到达。
            //SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);//将hTimer信息重置为无信号，如果不然就会不断的输出

			MAKE_STD_ZVAL(retval_ptr);
			
			if( SUCCESS != call_user_function(EG(function_table),NULL,callback,retval_ptr,0,NULL TSRMLS_CC ) ) {
				zval_ptr_dtor( &retval_ptr );
				zval_ptr_dtor( &callback );
				zval_ptr_dtor( &dwMilliseconds );
				RETURN_LONG(WING_ERROR_FAILED);	
				return;
			}
			zval_ptr_dtor( &retval_ptr );

			if( max_run_times>0 ) {
				times++;
				if( times >= max_run_times ) break;
			}


			 if ( !SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0)) {         
                   CloseHandle(hTimer);
			       zval_ptr_dtor( &callback );
			       zval_ptr_dtor( &dwMilliseconds );
			       RETURN_LONG(WING_ERROR_FAILED);	
                   return;
			 }

		}  
	}
	SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
    CloseHandle(hTimer);
	zval_ptr_dtor( &callback );
	zval_ptr_dtor( &dwMilliseconds );
	RETURN_LONG(WING_SUCCESS);	
	return;
}

