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
  | Author: yuyi 2016-08-17                                              |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_WING_ENCRYPT_H
#define PHP_WING_ENCRYPT_H

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

extern zend_module_entry wing_encrypt_module_entry;
#define phpext_wing_encrypt_ptr &wing_encrypt_module_entry

#define PHP_WING_ENCRYPT_VERSION "0.1.0"     //这个是版本号 按需调整升级
#define WING_EMPTY_STRING        '\0'

#define WING_ENCRYPT_KEY        "BFEBFBFF000306C3WD-WCC3F2FT0LTF"//"BFEBFBFF000306C3NA7R6PNDS1X1NYAG304186"  //(注：这里必须填写wing_get_key.exe生成的秘钥)这个就是加密的秘钥了，从别的电脑拿到后写在这 重新编译就好啦
#define WING_ENCRYPT_PRE_KEY    "wing"							  //随机秘钥前缀 这个可以随便写 每次写的都不一样 防止秘钥被人猜测
#define WING_ENCRYPT_END_KEY    "20160817"                        //随机秘钥后缀


#ifdef PHP_WIN32
#	define PHP_WING_ENCRYPT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WING_ENCRYPT_API __attribute__ ((visibility("default")))
#else
#	define PHP_WING_ENCRYPT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define WING_ENCRYPT_G(v) TSRMG(wing_encrypt_globals_id, zend_wing_encrypt_globals *, v)
#else
#define WING_ENCRYPT_G(v) (wing_encrypt_globals.v)
#endif

//两个api声明
ZEND_FUNCTION( wing_dz_encrypt_file );
ZEND_FUNCTION( wing_dz_run_file );


#endif	
