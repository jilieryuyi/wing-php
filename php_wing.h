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

#define PHP_WING_VERSION "1.0.8" /* Replace with version number for your extension */

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







//自定义消息
#define WM_ONCONNECT		WM_USER + 60
#define WM_ACCEPT_ERROR		WM_USER + 61
#define WM_ONERROR			WM_USER + 62
#define WM_ONCLOSE			WM_USER + 63
#define WM_ONRECV			WM_USER + 64
#define WM_ONQUIT           WM_USER + 65
#define WM_ONCLOSE_EX		WM_USER + 66
#define WM_ONSEND			WM_USER + 67
#define WM_THREAD_RUN		WM_USER + 71
#define WM_TIMEOUT          WM_USER + 72
#define WM_ADD_CLIENT       WM_USER + 73
#define WM_ONCLOSE2			WM_USER + 75
#define WM_ONBEAT			WM_USER + 76
#define WM_POST_RECV_ERR    WM_USER + 77


//错误码
#define WING_ERROR_SUCCESS				   1
#define WING_ERROR_CALLBACK_SUCCESS		   0
#define WING_ERROR_PARAMETER_ERROR	      -1
#define WING_ERROR_FAILED			      -2
#define WING_NOTICE_IGNORE			      -3
#define WING_ERROR_CALLBACK_FAILED        -4
#define WING_ERROR_PROCESS_NOT_EXISTS     -5
#define WING_ERROR_WINDOW_NOT_FOUND       -6
#define WING_ERROR_PROCESS_IS_RUNNING      1




#define DATA_BUFSIZE 1024

//完成端口操作码
#define OP_ACCEPT 1
#define OP_RECV   2
#define OP_SEND   3


#define CLIENT_IOCP   1
#define CLIENT_SELECT 2

#define WING_SEARCH_BY_PROCESS_EXE_FILE  1
#define WING_SEARCH_BY_PROCESS_NAME      1

#define WING_SEARCH_BY_PROCESS_ID        2
#define WING_SEARCH_BY_PARENT_PROCESS_ID 3
#define WING_SEARCH_BY_COMMAND_LINE      4

#define WING_SEARCH_BY_PROCESS_EXE_PATH  5
#define WING_SEARCH_BY_PROCESS_FILE_PATH 5





#endif	/* PHP_WING_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
