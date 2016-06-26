#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "zend_constants.h"
#include "ext/standard/info.h"
#include "php_wing.h"
#include "lib/wing_lib.h"

/***************************************************************
 * @ 获取 wing php的版本号 或者使用常量 WING_VERSION                
 **************************************************************/
PHP_FUNCTION(wing_version){
	char *string = NULL;
    int len = spprintf(&string, 0, "%s", PHP_WING_VERSION);
    RETURN_STRING(string,0);
}

/****************************************************************
 *@获取最后发生的错误
 ****************************************************************/
ZEND_FUNCTION(wing_get_last_error){
	RETURN_LONG(GetLastError());
}

ZEND_FUNCTION(wing_wsa_get_last_error){
	RETURN_LONG(WSAGetLastError());
}

