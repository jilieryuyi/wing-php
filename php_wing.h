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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_WING_H
#define PHP_WING_H

extern zend_module_entry wing_module_entry;
#define phpext_wing_ptr &wing_module_entry

#define PHP_WING_VERSION "1.0.3" /* Replace with version number for your extension */


#define WING_SUCCESS				       1
#define WING_CALLBACK_SUCCESS		       0
#define WING_ERROR_PARAMETER_ERROR	      -1
#define WING_ERROR_FAILED			      -2
#define WING_NOTICE_IGNORE			      -3
#define WING_ERROR_CALLBACK_FAILED        -4
#define WING_ERROR_PROCESS_NOT_EXISTS     -5
#define WING_ERROR_WINDOW_NOT_FOUND       -6
#define WING_PROCESS_IS_RUNNING		       1

typedef DWORD wing_ulong;


#ifdef PHP_WIN32
#	define PHP_WING_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WING_API __attribute__ ((visibility("default")))
#else
#	define PHP_WING_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(wing)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(wing)
*/

/* In every utility function you add that needs to use variables 
   in php_wing_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as WING_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define WING_G(v) TSRMG(wing_globals_id, zend_wing_globals *, v)
#else
#define WING_G(v) (wing_globals.v)
#endif



//timer 进程计数器 用于控制多个timer的创建和运行
//static int wing_timer_count = 0;
//线程计数器 用于多线程控制


//unsigned long create_process(char *command,char *params_ex=NULL,int params_ex_len=0);
//void command_params_check(char* &command_params,int *run_process,int *last_value TSRMLS_DC);

PHP_FUNCTION(wing_process_wait);
PHP_FUNCTION( wing_create_thread );
ZEND_FUNCTION(wing_get_process_params);
PHP_FUNCTION(wing_create_process);
PHP_FUNCTION( wing_create_process_ex );
ZEND_FUNCTION(wing_process_kill);
ZEND_FUNCTION(wing_get_current_process_id);
ZEND_FUNCTION(wing_create_mutex);
ZEND_FUNCTION(wing_close_mutex);
ZEND_FUNCTION(wing_process_isalive);
ZEND_FUNCTION(wing_get_env);
ZEND_FUNCTION(wing_set_env);
ZEND_FUNCTION( wing_get_command_path );
ZEND_FUNCTION(wing_timer);

ZEND_FUNCTION(wing_send_msg);
ZEND_FUNCTION( wing_create_window );
ZEND_FUNCTION(wing_destory_window);
ZEND_FUNCTION(wing_message_loop);
ZEND_FUNCTION(wing_message_box);

#endif	/* PHP_WING_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
