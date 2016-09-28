#include "php_wing.h"

extern char* PHP_PATH;
/**
 *@wait process进程等待
 *@param process id 进程id
 *@param timeout 等待超时时间 单位毫秒
 *@return exit code 进程退出码
 */
PHP_FUNCTION( wing_process_wait ){
	
	int process_id,timeout = INFINITE;

	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &process_id, &timeout ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( process_id <= 0 ) {
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	HANDLE handle     = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id );
	DWORD wait_result = 0;
	DWORD wait_status = WaitForSingleObject( handle,timeout );
	 
	if( wait_status != WAIT_OBJECT_0 ) {
		CloseHandle( handle );
		RETURN_LONG( wait_status );
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

/**
 *@get data from create process
 *@从父进程获取数据 这里使用了一个小伎俩，性能，待考量
 *return string or null
 */
ZEND_FUNCTION( wing_get_process_params ){
	
	HANDLE m_hRead	 = GetStdHandle(STD_INPUT_HANDLE);
	DWORD data_len	 = 1024;
	int step		 = 1024;
	char *buf		 = (char*)emalloc(data_len);
	DWORD dwRead     = 0;
	DWORD lBytesRead = 0;

	ZeroMemory( buf, data_len );
	
	if( !PeekNamedPipe( m_hRead,buf, data_len, &lBytesRead, 0, 0 ) ) {
		
		efree( buf );
		buf = NULL;
		RETURN_NULL();
		return;
	}

	if( lBytesRead <= 0 ){

		efree( buf );
		buf = NULL;
		RETURN_NULL();
		return;
	}

	while( lBytesRead >= data_len ){

		efree( buf );
		buf = NULL;
		data_len += step;
				
		buf = (char*)emalloc(data_len);
				
		ZeroMemory( buf, data_len );
		if( !PeekNamedPipe( m_hRead,buf, data_len, &lBytesRead, 0, 0 ) ){

			efree( buf );
			buf = NULL;
			RETURN_NULL();
			return;
		}
	}
				
	if ( ReadFile( m_hRead, buf, lBytesRead+1, &dwRead, NULL ) ) {
		// 从管道中读取数据 
		ZVAL_STRINGL( return_value, buf, dwRead, 1 );
		efree( buf );
		buf = NULL;	
		return;
	}
	RETURN_NULL();
	return;
}



/**
 *@create process 创建进程
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION( wing_create_process ){

	char *exe           = NULL;   //创建进程必须的可执行文件 一般为 exe bat等可以直接运行的文件
	int	  exe_len       = 0;
	char *output_file     = NULL;   //命令行输出到文件
	int	  output_file_len = 0;

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &exe, &exe_len, &output_file, &output_file_len ) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	

	//HANDLE m_hRead         = NULL;
	//HANDLE m_hWrite        = NULL;
	STARTUPINFO sui;    
	PROCESS_INFORMATION pi;                        // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;                        // 父进程传递给子进程的一些信息
		
	
    
	sa.bInheritHandle       = TRUE;                // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
	sa.lpSecurityDescriptor = NULL;
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	
	/*if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
	{
		RETURN_LONG( WING_ERROR_FAILED );
		return;
	}*/

	
    SECURITY_ATTRIBUTES *psa=NULL;  
    DWORD dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE;  
    OSVERSIONINFO osVersion={0};  
    osVersion.dwOSVersionInfoSize =sizeof ( osVersion );  
    if ( GetVersionEx ( &osVersion ) )  
    {  
        if ( osVersion.dwPlatformId ==VER_PLATFORM_WIN32_NT )  
        {  
            psa=&sa;  
            dwShareMode|=FILE_SHARE_DELETE;  
        }  
    }  


	HANDLE hConsoleRedirect=CreateFile (  
                                output_file,  
                                GENERIC_WRITE,  
                                dwShareMode,  
                                psa,  
                                OPEN_ALWAYS,  
                                FILE_ATTRIBUTE_NORMAL,  
                                NULL );  

	ZeroMemory(&sui, sizeof(STARTUPINFO));         // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
	
	sui.cb         = sizeof(STARTUPINFO);
	sui.dwFlags	   = STARTF_USESTDHANDLES;  
	sui.hStdInput  = NULL;//m_hRead;
	sui.hStdOutput = hConsoleRedirect;//m_hWrite;
	sui.hStdError  = hConsoleRedirect;//GetStdHandle(STD_ERROR_HANDLE);
	sui.wShowWindow= SW_HIDE;

	/*if( params_len >0 ) {	
		DWORD byteWrite  = 0;
		if( ::WriteFile( m_hWrite, params, params_len, &byteWrite, NULL ) == FALSE ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "write data to process error");
		}
	}*/

	if ( !CreateProcess( NULL,exe, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi ) ) {
		  CloseHandle(hConsoleRedirect);
		 // CloseHandle(m_hWrite);
		  RETURN_LONG( WING_ERROR_FAILED );
		  return;
	}
		
	//CloseHandle( m_hRead );
	CloseHandle( hConsoleRedirect );
	CloseHandle( pi.hProcess );  // 子进程的进程句柄
	CloseHandle( pi.hThread );   // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行

	RETURN_LONG(  pi.dwProcessId );	
	return;
}



/**
 *@创建进程，把一个php文件直接放到一个进程里面执行
 *@param command path 程序路径
 *@param command params 命令行参数
 */
PHP_FUNCTION( wing_create_process_ex ){
	
	char *php_file       = NULL;
	int	  php_file_len   = 0;
	char *output_file	     = NULL;
	int   output_file_len     = 0;
	char *command        = NULL;

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &php_file, &php_file_len, &output_file, &output_file_len ) != SUCCESS ) {
		 RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		 return;
	}
	
	
	spprintf( &command, 0, "%s %s\0", PHP_PATH, php_file );

	//HANDLE m_hRead         = NULL;
	//HANDLE m_hWrite        = NULL;
	STARTUPINFO sui;    
	PROCESS_INFORMATION pi;                        // 保存了所创建子进程的信息
	SECURITY_ATTRIBUTES sa;                        // 父进程传递给子进程的一些信息
		
	
    
	sa.bInheritHandle       = TRUE;                // 还记得我上面的提醒吧，这个来允许子进程继承父进程的管道句柄
	sa.lpSecurityDescriptor = NULL;
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	
	/*if (!CreatePipe(&m_hRead, &m_hWrite, &sa, 0))
	{
		efree(command);
		RETURN_LONG( WING_ERROR_FAILED );
		return;
	}*/

	SECURITY_ATTRIBUTES *psa=NULL;  
    DWORD dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE;  
    OSVERSIONINFO osVersion={0};  
    osVersion.dwOSVersionInfoSize =sizeof ( osVersion );  
    if ( GetVersionEx ( &osVersion ) )  
    {  
        if ( osVersion.dwPlatformId ==VER_PLATFORM_WIN32_NT )  
        {  
            psa=&sa;  
            dwShareMode|=FILE_SHARE_DELETE;  
        }  
    }  


	HANDLE hConsoleRedirect=CreateFile (  
                                output_file,  
                                GENERIC_WRITE,  
                                dwShareMode,  
                                psa,  
                                OPEN_ALWAYS,  
                                FILE_ATTRIBUTE_NORMAL,  
                                NULL );  

	ZeroMemory(&sui, sizeof(STARTUPINFO));         // 对一个内存区清零，最好用ZeroMemory, 它的速度要快于memset
	
	sui.cb         = sizeof(STARTUPINFO);
	sui.dwFlags	   = STARTF_USESTDHANDLES;  
	sui.hStdInput  = NULL;//m_hRead;
	sui.hStdOutput = hConsoleRedirect;//m_hWrite;
	sui.hStdError  = hConsoleRedirect;//GetStdHandle(STD_ERROR_HANDLE);
		
	/*if( params_len >0 ) {	
		DWORD byteWrite  = 0;
		if( ::WriteFile( m_hWrite, params, params_len, &byteWrite, NULL ) == FALSE ) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "write data to process error");
		}
	}*/

	if ( !CreateProcess( NULL, command , NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi ) ) {
		  CloseHandle(hConsoleRedirect);
		 // CloseHandle(m_hWrite);
		  efree(command);
		  RETURN_LONG( WING_ERROR_FAILED );
		  return;
	}
		
	CloseHandle( hConsoleRedirect );
	//CloseHandle( m_hWrite );
	CloseHandle( pi.hProcess );  // 子进程的进程句柄
	CloseHandle( pi.hThread );   // 子进程的线程句柄，windows中进程就是一个线程的容器，每个进程至少有一个线程在执行

	efree(command);
	RETURN_LONG(  pi.dwProcessId );	
	
	return;
}


/**
 *@杀死进程
 */
ZEND_FUNCTION( wing_process_kill )
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
    RETURN_LONG(WING_ERROR_SUCCESS);
	return;
}


/**
 *@获取当前进程id
 */
ZEND_FUNCTION( wing_get_current_process_id ){
	ZVAL_LONG( return_value, GetCurrentProcessId() );
}


/**
 *@获取windows内核对象的引用次数
 *@param int  内核对象句柄，在php内部表示即为int类型
 *@return int
 */
ZEND_FUNCTION( wing_query_object ){

	int handle = 0;

	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "l", &handle ) != SUCCESS ) 
	{
		RETURN_LONG(0);
		return;
	}

	RETURN_LONG( WingQueryObject( (HANDLE)handle ) );
	return;
}

/**
 *@返回 0程序正在运行 -1 获取参数错误 -2 参数不能为空 
 *@-3创建互斥锁失败 long handle创建互斥锁成功  
 */
ZEND_FUNCTION( wing_create_mutex ) {

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

	//---跨边界共享内核对象需要的参数
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE m_hMutex =  CreateMutex( &sa ,TRUE, mutex_name );//CreateMutex(&sa,TRUE,mutex_name);

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

/**
 *@关闭互斥量
 */
ZEND_FUNCTION( wing_close_mutex )
{
	long mutex_handle = 0;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&mutex_handle) != SUCCESS ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	if( mutex_handle <= 0 ) {
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}

	int status_code = CloseHandle( (HANDLE)mutex_handle ) ? WING_ERROR_SUCCESS : WING_ERROR_FAILED ;
	RETURN_LONG( status_code );
	return;
}



void wing_query_process_item( zval *&return_value, PROCESSINFO process )
{

		zval *item;
		MAKE_STD_ZVAL( item );
		array_init( item );

		add_assoc_string(    item, "process_name",   process.process_name,  1 );
		delete[] process.process_name;
				
		add_assoc_string(    item, "command_line",   process.command_line,  1 );
		delete[] process.command_line;
			
		add_assoc_string(    item, "file_name",      process.file_name,     1 );
		delete[] process.file_name;
	
		add_assoc_string(    item, "file_path",      process.file_path,     1 );
		delete[] process.file_path;

		add_assoc_long(      item,"process_id",        process.process_id          );
		add_assoc_long(      item,"parent_process_id", process.parent_process_id   );
		add_assoc_long(      item,"working_set_size",  process.working_set_size    );
		add_assoc_long(      item,"base_priority",     process.base_priority       );
		add_assoc_long(      item,"thread_count",      process.thread_count        );
		add_assoc_long(      item,"handle_count",      process.handle_count        );
		add_assoc_long(      item,"cpu_time",          process.cpu_time            );

		add_next_index_zval( return_value, item );

}

ZEND_FUNCTION( wing_query_process ){

	zval *keyword     = NULL;
	int search_by     = 0;

	MAKE_STD_ZVAL( keyword );
	ZVAL_STRING( keyword ,"",1 );

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zl", &keyword, &search_by ) != SUCCESS ) 
	{
		if( keyword ) 
			zval_ptr_dtor( &keyword );

		RETURN_EMPTY_STRING();
		return;
	}

	PROCESSINFO *all_process = NULL;
	
	//第一次传null返回实际的进程数量
	int count   = WingQueryProcess( all_process , 0 );
	
	all_process = new PROCESSINFO[count];
	count       = WingQueryProcess( all_process , count );

	array_init( return_value );


	for( int i = 0; i < count ; i++ ) 
	{
		if( Z_TYPE_P(keyword) == IS_NULL || Z_STRLEN_P(keyword) == 0  ) 
		{
			wing_query_process_item( return_value, all_process[i] );
		}
		else
		{
			//如果是数字
			if( Z_TYPE_P( keyword ) == IS_LONG || is_numeric_string(Z_STRVAL_P(keyword),Z_STRLEN_P(keyword),NULL,NULL,0) ) 
			{
				int _keyword = 0;
			
				if( Z_TYPE_P( keyword ) == IS_LONG )
					_keyword = Z_LVAL_P( keyword );

				else if( is_numeric_string(Z_STRVAL_P(keyword),Z_STRLEN_P(keyword),NULL,NULL,0) )
					_keyword = zend_atoi( Z_STRVAL_P(keyword) , Z_STRLEN_P(keyword) );
			
				if( search_by <= 0 )
				{
					if( _keyword ==  all_process[i].process_id ||  _keyword == all_process[i].parent_process_id ){
						wing_query_process_item( return_value, all_process[i] );
					}

				}
				else if( search_by == WING_SEARCH_BY_PROCESS_ID ) 
				{

					if( _keyword ==  all_process[i].process_id ){
						wing_query_process_item( return_value, all_process[i] );
					}

				}
				else if( search_by == WING_SEARCH_BY_PARENT_PROCESS_ID ) 
				{
				
					if( _keyword ==  all_process[i].parent_process_id ){
						wing_query_process_item( return_value, all_process[i] );
					}
				
				}
			}


			//如果是字符串
			if( Z_TYPE_P(keyword) == IS_STRING && Z_STRLEN_P(keyword) > 0  ) 
			{
				//不指定查询字段
				if( search_by <= 0 ){

					if( ( all_process[i].process_name != NULL && strlen(all_process[i].process_name) > 0 && strstr( all_process[i].process_name , (const char*)Z_STRVAL_P(keyword) ) != NULL ) ||
						( all_process[i].command_line != NULL && strlen(all_process[i].command_line) > 0 && strstr( all_process[i].command_line , (const char*)Z_STRVAL_P(keyword) ) != NULL ) ||
						( all_process[i].file_path    != NULL && strlen(all_process[i].file_path)    > 0 && strstr( all_process[i].file_path ,    (const char*)Z_STRVAL_P(keyword) ) != NULL )
					)
					{
						wing_query_process_item( return_value, all_process[i] );
					}

				}
				else if( search_by == WING_SEARCH_BY_PROCESS_NAME )
				{
			
					if( all_process[i].process_name != NULL && strlen(all_process[i].process_name) > 0 && strstr( all_process[i].process_name , (const char*)Z_STRVAL_P(keyword) ) != NULL )
					{
						wing_query_process_item( return_value, all_process[i] );
					}
			
				}
				else if( search_by == WING_SEARCH_BY_COMMAND_LINE ) 
				{
			
					if( all_process[i].command_line != NULL && strlen(all_process[i].command_line) > 0 && strstr( all_process[i].command_line , (const char*)Z_STRVAL_P(keyword) ) != NULL )
					{
						wing_query_process_item( return_value, all_process[i] );
					}
			
				}
				else if( search_by == WING_SEARCH_BY_PROCESS_FILE_PATH ) 
				{
				
					if( all_process[i].file_path != NULL && strlen(all_process[i].file_path)    > 0 && strstr( all_process[i].file_path , (const char*)Z_STRVAL_P(keyword) )    != NULL )
					{
						wing_query_process_item( return_value, all_process[i] );
					}

				}
			}
		
		}
	}
	delete[] all_process;

}

/**
 * @获取使用的内存信息 
 * @进程实际占用的内存大小
 */
ZEND_FUNCTION( wing_get_memory_used ) {

	int process_id = 0;
	HANDLE handle  = INVALID_HANDLE_VALUE;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&process_id) != SUCCESS) {
		RETURN_LONG( 0 );
		return;
	}

	if( process_id <= 0)
		handle = GetCurrentProcess();

	else
		handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, process_id );

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo( handle, &pmc, sizeof(pmc) );
	CloseHandle( handle );
	RETURN_LONG( pmc.WorkingSetSize );
	return;
}



