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
  | Author:yuyi[email:297341015@qq.com]                                  |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_wing.h"

extern zend_class_entry *wing_sclient_ce;
extern zend_class_entry *wing_select_server_ce;
extern zend_class_entry *wing_server_ce;
extern zend_class_entry *wing_com_ce;
char *PHP_PATH = NULL;

static zend_function_entry wing_select_server_methods[]={
	ZEND_ME( wing_select_server, __construct, NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME( wing_select_server, on,          NULL,ZEND_ACC_PUBLIC)
	ZEND_ME( wing_select_server, start,       NULL,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};
static zend_function_entry wing_sclient_method[]={
	ZEND_ME( wing_sclient,send,  NULL,ZEND_ACC_PUBLIC )
	{NULL,NULL,NULL}
};
static zend_function_entry wing_com_method[]={
	ZEND_ME( wing_com, __construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR )
	ZEND_ME( wing_com, __destruct, NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR )
	ZEND_ME( wing_com, next,  NULL,ZEND_ACC_PUBLIC )
	ZEND_ME( wing_com, query,  NULL,ZEND_ACC_PUBLIC )
	ZEND_ME( wing_com, get,  NULL,ZEND_ACC_PUBLIC )
	{NULL,NULL,NULL}
};
static zend_function_entry wing_server_methods[]={
	ZEND_ME(wing_server,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(wing_server,on,NULL,ZEND_ACC_PUBLIC)
	ZEND_ME(wing_server,start,NULL,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};



PHP_MINIT_FUNCTION( wing )
{
	//-----wing_select_server----------------------------------
	zend_class_entry  _wing_select_server_ce;
	INIT_CLASS_ENTRY( _wing_select_server_ce,"wing_select_server", wing_select_server_methods );
	wing_select_server_ce = zend_register_internal_class( &_wing_select_server_ce TSRMLS_CC );
	
	//事件回调函数 默认为null
	zend_declare_property_null(    wing_select_server_ce,"onreceive",   strlen("onreceive"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_select_server_ce,"onconnect",   strlen("onconnect"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_select_server_ce,"onclose",     strlen("onclose"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_select_server_ce,"onerror",     strlen("onerror"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_select_server_ce,"ontimeout",   strlen("ontimeout"),ZEND_ACC_PRIVATE TSRMLS_CC);
	//zend_declare_property_null(    wing_select_server_ce,"ontick",      strlen("ontick"),   ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_select_server_ce,"onsend",      strlen("onsend"),   ZEND_ACC_PRIVATE TSRMLS_CC);
	

	//端口和监听ip地址 
	zend_declare_property_long(   wing_select_server_ce,"port",           strlen("port"),           6998,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string( wing_select_server_ce,"listen",         strlen("listen"),         "0.0.0.0", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_select_server_ce,"max_connect",    strlen("max_connect"),    1000,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_select_server_ce,"timeout",        strlen("timeout"),        0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	//zend_declare_property_long(   wing_select_server_ce,"tick",           strlen("tick"),           0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_select_server_ce,"active_timeout", strlen("active_timeout"), 0,         ZEND_ACC_PRIVATE TSRMLS_CC);

	//////////////////////////////////////////////////////////////////////////

	zend_class_entry  _wing_server_ce;
	INIT_CLASS_ENTRY( _wing_server_ce,"wing_server", wing_server_methods );
	wing_server_ce = zend_register_internal_class( &_wing_server_ce TSRMLS_CC );
	
	//事件回调函数 默认为null
	zend_declare_property_null(    wing_server_ce,"onreceive",   strlen("onreceive"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onconnect",   strlen("onconnect"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onclose",     strlen("onclose"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onerror",     strlen("onerror"),  ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"ontimeout",   strlen("ontimeout"),ZEND_ACC_PRIVATE TSRMLS_CC);
	//zend_declare_property_null(    wing_server_ce,"ontick",      strlen("ontick"),   ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(    wing_server_ce,"onsend",      strlen("onsend"),   ZEND_ACC_PRIVATE TSRMLS_CC);
	

	//端口和监听ip地址 
	zend_declare_property_long(   wing_server_ce,"port",           strlen("port"),           6998,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string( wing_server_ce,"listen",         strlen("listen"),         "0.0.0.0", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"max_connect",    strlen("max_connect"),    1000,      ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"timeout",        strlen("timeout"),        0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"tick",           strlen("tick"),           0,         ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(   wing_server_ce,"active_timeout", strlen("active_timeout"), 0,         ZEND_ACC_PRIVATE TSRMLS_CC);


	//sclient-------------------------
	zend_class_entry  _wing_sclient_ce;
	INIT_CLASS_ENTRY( _wing_sclient_ce , "wing_sclient" , wing_sclient_method );
	wing_sclient_ce = zend_register_internal_class( &_wing_sclient_ce TSRMLS_CC );

	zend_declare_property_string( wing_sclient_ce,"sin_addr",    strlen("sin_addr"),   "",           ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"sin_port",    strlen("sin_port"),   0,            ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"sin_family",  strlen("sin_family"), 0,            ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_string( wing_sclient_ce,"sin_zero",    strlen("sin_zero"),   "",           ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"socket",      strlen("socket"),     0,            ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"last_active", strlen("last_active"),0,            ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"online",      strlen("online"),     0,            ZEND_ACC_PUBLIC TSRMLS_CC );
	zend_declare_property_long(   wing_sclient_ce,"client_type", strlen("client_type"),CLIENT_IOCP,  ZEND_ACC_PUBLIC TSRMLS_CC );
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	zend_class_entry _wing_com_ce;
	INIT_CLASS_ENTRY( _wing_com_ce , "wing_com" , wing_com_method );
	wing_com_ce = zend_register_internal_class( &_wing_com_ce TSRMLS_CC );
	zend_declare_property_long(   wing_com_ce, "com",           strlen("com"),           0,      ZEND_ACC_PRIVATE TSRMLS_CC);


	//常量定义
	PHP_PATH = (char*)malloc(MAX_PATH);
	memset( PHP_PATH, 0, MAX_PATH );
	//(char*)emalloc(MAX_PATH); 测试发现这里不能用php的内存分配 如果使用php的内存分配 在使用WING_PHP常量的时候会拿不到值
#ifdef WIN32
	GetModuleFileName( NULL, PHP_PATH, MAX_PATH );
#else
	getcwd( PHP_PATH, 260 );
	strcat(PHP_PATH,"/php");
#endif
	
	zend_register_string_constant( "WING_PHP",                       sizeof("WING_PHP"),                     PHP_PATH,                      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_string_constant( "WING_VERSION",                   sizeof("WING_VERSION"),                 PHP_WING_VERSION,              CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	
	zend_register_long_constant(   "WING_WAIT_TIMEOUT",              sizeof("WING_WAIT_TIMEOUT"),            WAIT_TIMEOUT,                  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_FAILED",               sizeof("WING_WAIT_FAILED"),             WAIT_FAILED,                   CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_INFINITE",                  sizeof("WING_INFINITE"),                INFINITE,                      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_OBJECT_0",             sizeof("WING_WAIT_OBJECT_0"),           WAIT_OBJECT_0,                 CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WAIT_ABANDONED",            sizeof("WING_WAIT_ABANDONED"),          WAIT_ABANDONED,                CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	
	zend_register_long_constant(   "WING_ERROR_ALREADY_EXISTS",      sizeof("WING_ERROR_ALREADY_EXISTS"),    ERROR_ALREADY_EXISTS,          CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_PARAMETER_ERROR",     sizeof("WING_ERROR_PARAMETER_ERROR"),   WING_ERROR_PARAMETER_ERROR,    CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_FAILED",              sizeof("WING_ERROR_FAILED"),            WING_ERROR_FAILED,             CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_CALLBACK_FAILED",     sizeof("WING_ERROR_CALLBACK_FAILED"),   WING_ERROR_CALLBACK_FAILED,    CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_CALLBACK_SUCCESS",    sizeof("WING_ERROR_CALLBACK_SUCCESS"),  WING_ERROR_CALLBACK_SUCCESS,   CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_PROCESS_NOT_EXISTS",  sizeof("WING_ERROR_PROCESS_NOT_EXISTS"),WING_ERROR_PROCESS_NOT_EXISTS, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_SUCCESS",             sizeof("WING_ERROR_SUCCESS"),           WING_ERROR_SUCCESS,            CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);	
	zend_register_long_constant(   "WING_ERROR_PROCESS_IS_RUNNING",  sizeof("WING_ERROR_PROCESS_IS_RUNNING"),WING_ERROR_PROCESS_IS_RUNNING, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_ERROR_NT",                  sizeof("WING_ERROR_NT"),                WING_ERROR_NT,                 CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	
	

	zend_register_long_constant(   "WING_SEARCH_BY_COMMAND_LINE",      sizeof("WING_SEARCH_BY_COMMAND_LINE"),      WING_SEARCH_BY_COMMAND_LINE,      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PARENT_PROCESS_ID", sizeof("WING_SEARCH_BY_PARENT_PROCESS_ID"), WING_SEARCH_BY_PARENT_PROCESS_ID, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PROCESS_EXE_FILE",  sizeof("WING_SEARCH_BY_PROCESS_EXE_FILE"),  WING_SEARCH_BY_PROCESS_EXE_FILE,  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PROCESS_ID",        sizeof("WING_SEARCH_BY_PROCESS_ID"),        WING_SEARCH_BY_PROCESS_ID,        CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PROCESS_EXE_PATH",  sizeof("WING_SEARCH_BY_PROCESS_EXE_PATH"),  WING_SEARCH_BY_PROCESS_EXE_PATH,  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PROCESS_NAME",      sizeof("WING_SEARCH_BY_PROCESS_NAME"),      WING_SEARCH_BY_PROCESS_NAME,      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_SEARCH_BY_PROCESS_FILE_PATH", sizeof("WING_SEARCH_BY_PROCESS_FILE_PATH"), WING_SEARCH_BY_PROCESS_FILE_PATH, CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);



	

	zend_register_long_constant(   "WING_WINDOWS_ANCIENT",      sizeof("WING_WINDOWS_ANCIENT"),      WING_WINDOWS_ANCIENT,      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_XP",           sizeof("WING_WINDOWS_XP"),           WING_WINDOWS_XP,           CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_SERVER_2003",  sizeof("WING_WINDOWS_SERVER_2003"),  WING_WINDOWS_SERVER_2003,  CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_VISTA",        sizeof("WING_WINDOWS_VISTA"),        WING_WINDOWS_VISTA,        CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_7",            sizeof("WING_WINDOWS_7"),            WING_WINDOWS_7,            CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_8",            sizeof("WING_WINDOWS_8"),            WING_WINDOWS_8,            CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_8_1",          sizeof("WING_WINDOWS_8_1"),          WING_WINDOWS_8_1,          CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_10",           sizeof("WING_WINDOWS_10"),           WING_WINDOWS_10,           CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_long_constant(   "WING_WINDOWS_NEW",          sizeof("WING_WINDOWS_NEW"),          WING_WINDOWS_NEW,          CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);




	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(wing)
{
	if( PHP_PATH ) 
		free( PHP_PATH );
	return SUCCESS;
}

PHP_RINIT_FUNCTION(wing)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(wing)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(wing)
{
	php_info_print_table_start();
	php_info_print_table_header( 2, "wing support",    "enabled"          );
	php_info_print_table_row(    2, "version",         PHP_WING_VERSION   );
	php_info_print_table_row(    2, "author",          "yuyi"             );
	php_info_print_table_row(    2, "email",           "297341015@qq.com" );
	php_info_print_table_row(    2, "qq-group",        "535218312"        );
	php_info_print_table_end();
}

const zend_function_entry wing_functions[] = {
	
	PHP_FE( wing_version, NULL )
	PHP_FE( wing_create_process, NULL )
	//PHP_FE( wing_get_process_params, NULL )
	PHP_FE( wing_create_process_ex, NULL )
	PHP_FE( wing_process_wait, NULL )
	PHP_FE( wing_process_kill, NULL )
	ZEND_FALIAS( wing_kill_process, wing_process_kill, NULL )
	PHP_FE( wing_get_command_line , NULL )
	PHP_FE( wing_query_object , NULL  )
	PHP_FE( wing_override_function, NULL )
	PHP_FE( wing_get_current_process_id, NULL )
	PHP_FE( wing_create_mutex, NULL )
	PHP_FE( wing_close_mutex, NULL )
	PHP_FE( wing_get_env, NULL )
	PHP_FE( wing_get_command_path, NULL )
	PHP_FE( wing_set_env, NULL )
	PHP_FE( wing_windows_send_msg, NULL )
	PHP_FE( wing_get_last_error, NULL )
	PHP_FE( wing_wsa_get_last_error, NULL )
	PHP_FE( wing_get_error_msg, NULL )
	PHP_FE( wing_get_memory_used , NULL )
	ZEND_FALIAS( wing_get_process_memory_used , wing_get_memory_used , NULL )
	PHP_FE( wing_windows_version, NULL )
	PHP_FE( wing_query_process, NULL )
	ZEND_FALIAS( wing_find_process , wing_query_process , NULL )

	PHP_FE( wing_encrypt_file, NULL )
	PHP_FE( wing_run_file, NULL )

	PHP_FE( wing_adapters_info , NULL )
	PHP_FE( wing_get_cpu_id, NULL )
	PHP_FE( wing_get_serial_number, NULL )
	
	PHP_FE_END	
};

zend_module_entry wing_module_entry = {
	STANDARD_MODULE_HEADER,
	"wing",
	wing_functions,
	PHP_MINIT(wing),
	PHP_MSHUTDOWN(wing),
	PHP_RINIT(wing),		
	PHP_RSHUTDOWN(wing),	
	PHP_MINFO(wing),
	PHP_WING_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_WING
ZEND_GET_MODULE(wing)
#endif