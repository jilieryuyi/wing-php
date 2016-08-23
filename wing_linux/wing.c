#include "php_wing.h"
#include <stdio.h>
#include "inttypes.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



/**
 * @ 获取 wing php的版本号 或者使用常量 WING_VERSION                
 */
PHP_FUNCTION( wing_version ){

	char *version_str = NULL;
    int len      = spprintf(&version_str, 0, "%s", PHP_WING_VERSION);

    RETURN_STRING(version_str,0);
}




ZEND_FUNCTION(wing_get_cpu_id){
	char cpuid[100];
    wing_get_cpu_id( cpuid );
	RETURN_STRING( cpuid,1 );
}


PHP_MINIT_FUNCTION( wing )
{
	//常量定义
	PHP_PATH = (char*)malloc(MAX_PATH);
	memset( PHP_PATH, 0, MAX_PATH );

	wing_get_module_file_name( "php",  PHP_PATH, MAX_PATH);

	zend_register_string_constant( "WING_PHP",                       sizeof("WING_PHP"),                     PHP_PATH,                      CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	zend_register_string_constant( "WING_VERSION",                   sizeof("WING_VERSION"),                 PHP_WING_VERSION,              CONST_CS | CONST_PERSISTENT, module_number TSRMLS_CC);
	
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(wing)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(wing)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(wing)
{
	if( PHP_PATH ) 
		free( PHP_PATH );
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
	
	PHP_FE(wing_version,NULL)
	PHP_FE(wing_get_cpu_id,NULL)
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