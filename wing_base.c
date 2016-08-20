#include "php_wing.h"

zend_bool wing_func_is_callable(zval **var TSRMLS_DC){

	char *error = NULL;
	zend_bool is_call_able = zend_is_callable_ex(*var, NULL, 0, NULL, NULL, NULL, &error TSRMLS_CC);
	if( error ) 
		efree( error );
	return is_call_able ? 1 : 0;

}

void wing_call_func( zval **func TSRMLS_DC ,int params_count  ,zval **params ) {
	
	if( !wing_func_is_callable( func TSRMLS_CC) ) {
		return;
	}

	zval *retval = NULL;
	MAKE_STD_ZVAL(retval);

	if( SUCCESS != call_user_function( EG(function_table), NULL, *func, retval, params_count, params TSRMLS_CC ) ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "call user func fail");
	}

	if( retval )
	zval_ptr_dtor(&retval);
}

void get_error_msg( char *&error_msg, int last_error ){
			
	//获取错误码对应的错误描述
	HLOCAL hlocal     = NULL;
	DWORD systemlocal = MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL);
	BOOL fok          = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER , NULL , last_error, systemlocal , (LPSTR)&hlocal , 0 , NULL );
	if( !fok ) 
	{
		if( hlocal ) 
		{
			LocalFree( hlocal );
			hlocal = NULL;
		}

		HMODULE hDll  = LoadLibraryEx("netmsg.dll",NULL,DONT_RESOLVE_DLL_REFERENCES);
		if( NULL != hDll ) 
		{
				fok  = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER , hDll , last_error, systemlocal , (LPSTR)&hlocal , 0 , NULL );
				FreeLibrary( hDll );
		}
	}

	if( fok && hlocal != NULL ) 
	{
		char *gbk_error_msg   = (char*)LocalLock( hlocal );
		char *utf8_error_msg  =  wing_str_char_to_utf8( (const char*)gbk_error_msg );
					
		if( NULL == utf8_error_msg )
		{
			spprintf( &error_msg,0,"%s",gbk_error_msg );
		}else
		{
			spprintf( &error_msg,0,"%s",utf8_error_msg );
			free(utf8_error_msg);
		}
			
		LocalFree( hlocal );
        			
	}else{		
		spprintf( &error_msg, 0, "unknow error" );				
	}
}

/**
 * @生成随机字符串
 * @buf会自动初始化 用完需要释放 
 */
void wing_guid( _Out_ char *&buf TSRMLS_DC)  
{  
	buf = (char*)emalloc(64);
	CoInitialize(NULL);   
	GUID guid;  
	if (S_OK == ::CoCreateGuid(&guid))  
	{  
		  _snprintf( buf, 64*sizeof(char), "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"  , 
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
}  

void wing_get_command_path( const char *name ,char *&path ){

	path            = (char*)emalloc( MAX_PATH );
	int   size      = GetEnvironmentVariableA( "PATH", NULL, 0 );
	char *env_var	= (char*)emalloc( size );
	char *temp      = NULL;
	char *start     = NULL;
	char *var_begin = NULL;
	
	ZeroMemory( env_var, size );

	GetEnvironmentVariableA( "PATH", env_var, size );

	start		= env_var;
	var_begin	= env_var;

	char _temp_path[MAX_PATH] = {0};

	while( temp = strchr( var_begin, ';' ) ) {
		
		long len_temp	= temp - start;
		long _len_temp	= len_temp + sizeof("\\.exe") +1;
		
		ZeroMemory( path, MAX_PATH );

		strncpy_s( _temp_path, _len_temp, var_begin, len_temp );
		sprintf_s( path, MAX_PATH, "%s\\%s.exe\0", _temp_path, name );

		if( PathFileExists( path ) ) {
			efree( env_var );
			env_var = NULL;
			return;
		}

		ZeroMemory( path, MAX_PATH );
		sprintf_s(  path, MAX_PATH , "%s\\%s.bat\0", _temp_path, name );

		if( PathFileExists( path ) ) {
			efree( env_var );
			env_var = NULL;
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}

	efree( env_var );
	env_var = NULL;

	sprintf_s( path, MAX_PATH, "" );
	return;
}



/**
 * @ 获取 wing php的版本号 或者使用常量 WING_VERSION                
 */
PHP_FUNCTION( wing_version ){

	char *string = NULL;
    int len      = spprintf(&string, 0, "%s", PHP_WING_VERSION);

    RETURN_STRING(string,0);
}

/**
 *@获取最后发生的错误
 */
ZEND_FUNCTION( wing_get_last_error ){
	RETURN_LONG(GetLastError());
}

/**
 *@获取WSA系列函数的最后错误
 */
ZEND_FUNCTION( wing_wsa_get_last_error ){
	RETURN_LONG(WSAGetLastError());
}
/**
 *@获取错误信息
 */
ZEND_FUNCTION( wing_get_error_msg ){
			
	int last_error  = 0;
	char *error_msg = NULL;

	zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "|l", &last_error );
	
	if( last_error <= 0 ) {
		last_error = WSAGetLastError();
	}

	if( last_error <= 0 ) {
		last_error = GetLastError();
	}

	get_error_msg( error_msg, last_error ) ;
	RETURN_STRING( error_msg, 0);			
}


/**
 *@获取环境变量
 */
ZEND_FUNCTION( wing_get_env ){
	
	char *name     = NULL;
	int   name_len = 0;
	int   size     = 0;
	char *var      = NULL;
	
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,"s", &name, &name_len ) != SUCCESS ) 
	{
		RETURN_EMPTY_STRING();
		return;
	}

	size = GetEnvironmentVariable( name, NULL, 0 );

	if( size<=0 || GetLastError() == ERROR_ENVVAR_NOT_FOUND ) 
	{
		RETURN_EMPTY_STRING();
		return;
	}
	
	var = (char*)emalloc(size);  
	
	ZeroMemory( var , size );

	size = GetEnvironmentVariable(name,var,size);

	if( size == 0 ) 
	{
		efree(var);
		var = NULL;
		RETURN_EMPTY_STRING();
		return;
	}

	ZVAL_STRINGL( return_value, var, size, 1 );

	efree(var);
	var = NULL;
	return;
}

/**
 * @ 设置环境变量，子进程和父进程可以共享，可以简单用作进程间的通信方式
 */
ZEND_FUNCTION( wing_set_env ){

	char *name     = NULL;
	char *value    = NULL;
	
	int   name_len  = 0;
	int   value_len = 0;
	int   res       = 0;
	
	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "ss",  &name, &name_len, &value,&value_len ) != SUCCESS ) 
	{
		RETURN_LONG( WING_ERROR_PARAMETER_ERROR );
		return;
	}

	res = SetEnvironmentVariableA( name,(LPCTSTR)value ) ? WING_ERROR_SUCCESS : WING_ERROR_FAILED;
	
	RETURN_LONG( res );
}


/**
 * @ 获取一个命令所在的绝对文件路径
 * @ 比如说获取php的安装目录，不过wing php里面可以直接使用常量 WING_PHP 代表的书php的安装路径
 */
ZEND_FUNCTION( wing_get_command_path )
{ 

	char *name      = 0;
	int   name_len  = 0;
	char *path      = NULL;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len ) != SUCCESS ) 
	{
		RETURN_EMPTY_STRING();
		return;
	}
	
	wing_get_command_path( (const char *)name ,path );

	RETURN_STRING( path, 0 );
	return;
}

/**
 *@获取启动命令行
 */
ZEND_FUNCTION( wing_get_command_line )
{
	RETURN_STRING( GetCommandLineA(),1 );
}


static int override_fetch_function(char *fname, long fname_len,zend_function **pfe, int flag TSRMLS_DC)
{
    zend_function *fe = NULL;

	zend_str_tolower(fname, strlen(fname));

    if( zend_hash_find(EG(function_table), fname, fname_len+1,(void **)&fe) == FAILURE ) 
	{
        zend_error(E_WARNING, "%s not found", fname);
        return FAILURE;
    }

    if( fe->type != ZEND_USER_FUNCTION && fe->type != ZEND_INTERNAL_FUNCTION ) 
	{
        zend_error(E_WARNING, "%s is not a user or internal function", fname);
        return FAILURE;
    }

    if( pfe ) 
	{
        *pfe = fe;
    }

    return SUCCESS;
}

/**
 *@重写函数
 */
ZEND_FUNCTION( wing_override_function )
{
    char *fname, *fargs, *fcode, *forigin = NULL;
    long fname_len, fargs_len, fcode_len, forigin_len = 0;
    char *eval_code, *eval;
    zend_function *fe, func;
    int retval, ftype = 0;

    if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC,"sss|s", &fname, &fname_len,&fargs, &fargs_len, &fcode, &fcode_len,&forigin, &forigin_len) == FAILURE ) 
	{
        RETURN_FALSE;
    }

    if( forigin && forigin_len > 0 ) 
	{
        ftype = 2;
    }

    if( override_fetch_function(fname, fname_len, &fe,ftype TSRMLS_CC) != SUCCESS ) 
	{
        RETURN_FALSE;
    }

    func = *fe;
    if( fe->type == ZEND_USER_FUNCTION && ftype == 2 ) {
        function_add_ref(&func);
    }

    if( zend_hash_del(EG(function_table), fname, fname_len+1) == FAILURE ) 
	{
        zend_error(E_WARNING, "Error removing reference to function name %s()",fname);
        zend_function_dtor(&func);
        RETURN_FALSE;
    }

    if (ftype == 2) 
	{
        if( func.type == ZEND_USER_FUNCTION ) 
		{
            efree((char *)func.common.function_name);
            func.common.function_name = estrndup(forigin, forigin_len);
        }
        if( zend_hash_add(EG(function_table), forigin, forigin_len+1, &func, sizeof(zend_function), NULL) == FAILURE ) 
		{
            zend_error(E_WARNING, "Unable to rename function %s()", forigin);
            zend_function_dtor(fe);
            RETURN_FALSE;
        }
    }

    spprintf(&eval_code, 0, "function %s(%s){%s}", fname, fargs, fcode);

    if( !eval_code ) 
	{
        RETURN_FALSE;
    }

    eval   = zend_make_compiled_string_description("runtime created function" TSRMLS_CC);
    retval = zend_eval_string(eval_code, NULL, eval TSRMLS_CC);

    efree(eval_code);
    efree(eval);

    if( retval == SUCCESS ) 
	{
        RETURN_TRUE;
    } 
	else 
	{
        RETURN_FALSE;
    }
}


