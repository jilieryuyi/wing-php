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
  | Author: yuyi                                                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_wing_encrypt.h"
#include "Shlwapi.h"

extern BOOL WingEncryptFile( PCHAR szSource, PCHAR szDestination,  PCHAR szPassword);
extern BOOL WingDecryptFile( PCHAR szSource, PCHAR &szDestination, PCHAR szPassword);
extern void get_cpu_id( char *&processor_id );
extern void get_serial_number( char *&serial_number );
/**
 *@php源码加密
 */
ZEND_FUNCTION( wing_dz_encrypt_file )
{
	
	char *input_file   = NULL;
	int input_file_len = 0;

	char *output_file   = NULL;
	int output_file_len = 0;

	char *encrypt_password = NULL;
	int password_len = 0;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&input_file,&input_file_len,&output_file,&output_file_len) != SUCCESS) {
		RETURN_BOOL( 0 );
		return;
	}
	//char _input_file[MAX_PATH];
	//realpath( input_file,_input_file);

	//zend_printf("");
	//return;

	int needfree = 0;
	//得到硬件加密密码

	char *processor_id = NULL;
	char *serial_number= NULL;

	//get_cpu_id( processor_id );
	//get_serial_number( serial_number );

	TCHAR strPath[MAX_PATH]={0};  
	DWORD plen = GetTempPathA(MAX_PATH, strPath);  
	if(plen<=0){
		printf("error : cache error");
		RETURN_FALSE;
		return;
	}
	//return; 1
	//printf("==>%s<==\r\n",strPath);

	char* temp_processor_id = NULL;
	spprintf(&temp_processor_id,0,"%sprocessor_id.data",strPath);
	//printf("==>%s<==\r\n",temp_processor_id);

	char* temp_serial_number = NULL;
	spprintf(&temp_serial_number,0,"%sserial_number.data",strPath);
	//printf("==>%s<==\r\n",temp_serial_number);

	if( PathFileExists( temp_processor_id ) ){
		php_stream *read_pid_stream = php_stream_open_wrapper( temp_processor_id, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		processor_id = (char*)malloc(1024);
		memset(processor_id,0,1024);
		php_stream_read(read_pid_stream,processor_id,1024);
		//printf("get from buf processor_id=>%s<==\r\n",processor_id);
		php_stream_close(read_pid_stream);	

	}else{ 

		get_cpu_id( processor_id );
		if(processor_id == NULL)
		printf("error : get cpu id error");

		php_stream *read_pid_stream = php_stream_open_wrapper( temp_processor_id, "w+", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		php_stream_write(read_pid_stream,processor_id, strlen(processor_id));
		php_stream_close(read_pid_stream);	
	}

	//return; 2

	if( PathFileExists( temp_serial_number ) ){
		php_stream *read_serial_number_stream = php_stream_open_wrapper( temp_serial_number, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		serial_number = (char*)malloc(1024);
		memset(serial_number,0,1024);
		php_stream_read(read_serial_number_stream,serial_number,1024);
		//printf("get from buf serial_number=>%s<==\r\n",serial_number);
		php_stream_close(read_serial_number_stream);	
	}
	else{
		get_serial_number( serial_number );
		if(serial_number == NULL)
			printf("error : get serial number error");

		//printf("serial_number=%s\r\n",serial_number);
		php_stream *read_serial_number_stream = php_stream_open_wrapper( temp_serial_number, "w+", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		php_stream_write(read_serial_number_stream,serial_number, strlen(serial_number));
		php_stream_close(read_serial_number_stream);
	}

	//return; 3


	if( processor_id != NULL && serial_number != NULL )
	{	
		spprintf( &encrypt_password, 0, "%s-%s%s-%s", WING_ENCRYPT_PRE_KEY , processor_id, serial_number , WING_ENCRYPT_END_KEY );
		needfree = 1;
		delete[] processor_id;
		delete[] serial_number;

	}else{
		if(temp_processor_id) {
			efree(temp_processor_id);
			temp_processor_id = NULL;
		}
		if(temp_serial_number) {
			efree(temp_serial_number);
			temp_serial_number = NULL;
		}
		RETURN_BOOL(0);
		return;
	}
	//return;4

	//执行文件加密
	int res = WingEncryptFile( input_file , output_file , encrypt_password );
	if( needfree ) 
		efree( encrypt_password );

	if(temp_processor_id) {
		efree(temp_processor_id);
		temp_processor_id = NULL;
	}
	if(temp_serial_number) {
		efree(temp_serial_number);
		temp_serial_number = NULL;
	}
	RETURN_BOOL( res );
	return;
}

/**
 *@执行加密过的php源码文件
 */
ZEND_FUNCTION( wing_dz_run_file )
{

	char *input_file       = NULL;
	int input_file_len     = 0;
	char *encrypt_password = NULL;
	int password_len       = 0;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&input_file,&input_file_len ) != SUCCESS) {
		RETURN_LONG( 0 );
		return;
	}

	int needfree = 0;
	//获取硬件加密密码
	
	char *processor_id = NULL;
	char *serial_number= NULL;

	TCHAR strPath[MAX_PATH]={0};  
	DWORD plen = GetTempPath(MAX_PATH, strPath);  
	if(plen<=0){
		printf("error : cache error");
		RETURN_FALSE;
		return;
	}
	//printf("==>%s<==\r\n",strPath);

	char* temp_processor_id = NULL;
	spprintf(&temp_processor_id,0,"%sprocessor_id.data",strPath);
	//printf("==>%s<==\r\n",temp_processor_id);

	char* temp_serial_number = NULL;
	spprintf(&temp_serial_number,0,"%sserial_number.data",strPath);
	//printf("==>%s<==\r\n",temp_serial_number);

	if( PathFileExists( temp_processor_id ) ){
		php_stream *read_pid_stream = php_stream_open_wrapper( temp_processor_id, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		processor_id = (char*)malloc(1024);
		memset(processor_id,0,1024);
		php_stream_read(read_pid_stream,processor_id,1024);
		//printf("get from buf processor_id=>%s<==\r\n",processor_id);
		php_stream_close(read_pid_stream);	

	}else{ 

		get_cpu_id( processor_id );
		//printf("processor_id=%s\r\n",processor_id);
		
		php_stream *read_pid_stream = php_stream_open_wrapper( temp_processor_id, "w+", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		php_stream_write(read_pid_stream,processor_id, strlen(processor_id));
		php_stream_close(read_pid_stream);	
	}
	if( PathFileExists( temp_serial_number ) ){
		php_stream *read_serial_number_stream = php_stream_open_wrapper( temp_serial_number, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		serial_number = (char*)malloc(1024);
		memset(serial_number,0,1024);
		php_stream_read(read_serial_number_stream,serial_number,1024);
		//printf("get from buf serial_number=>%s<==\r\n",serial_number);
		php_stream_close(read_serial_number_stream);	
	}
	else{
		get_serial_number( serial_number );
		//printf("serial_number=%s\r\n",serial_number);
		php_stream *read_serial_number_stream = php_stream_open_wrapper( temp_serial_number, "w+", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		php_stream_write(read_serial_number_stream,serial_number, strlen(serial_number));
		php_stream_close(read_serial_number_stream);
	}

	char *check_password = NULL;

	if( processor_id != NULL && serial_number != NULL )
	{	
		spprintf( &encrypt_password, 0, "%s-%s%s-%s", WING_ENCRYPT_PRE_KEY , processor_id, serial_number , WING_ENCRYPT_END_KEY );
		spprintf( &check_password,   0, "%s-%s-%s",   WING_ENCRYPT_PRE_KEY , WING_ENCRYPT_KEY , WING_ENCRYPT_END_KEY );

		needfree = 1;
		delete[] processor_id;
		delete[] serial_number;
		//printf("encrypt_password=>%s<=\r\n",encrypt_password);
		//printf("check_password=>%s<=\r\n",check_password);
		

		if( strcmp( encrypt_password , check_password) != 0 ){
			printf("warning : encrypt key error, can not run file %s\r\n",input_file);
			efree(encrypt_password);
			efree(check_password);
			if(temp_processor_id) {
				efree(temp_processor_id);
				temp_processor_id = NULL;
			}
			if(temp_serial_number) {
				efree(temp_serial_number);
				temp_serial_number = NULL;
			}
			RETURN_BOOL(0);
		}else{
			//printf("password check ok\r\n");
		}

	}else
	{
		if(temp_processor_id) {
			efree(temp_processor_id);
			temp_processor_id = NULL;
		}
		if(temp_serial_number) {
			efree(temp_serial_number);
			temp_serial_number = NULL;
		}
		RETURN_BOOL(0);
		return;
	}

	char *php_code = NULL;
	//解密得到源码
	WingDecryptFile( input_file , php_code , encrypt_password );

	if( NULL == php_code )
	{	
		if( needfree ) 
		{	
			efree( encrypt_password );
			efree(check_password);
		}

		if(temp_processor_id) {
			efree(temp_processor_id);
			temp_processor_id = NULL;
		}
		if(temp_serial_number) {
			efree(temp_serial_number);
			temp_serial_number = NULL;
		}
		RETURN_BOOL(0);
		return;
	}

	size_t code_len   = strlen( php_code );
	char *run_code = php_code;

	//跳过<?php头
	if( php_code[0] == '<')
		run_code += 5;

	//执行源码
	char *eval   = zend_make_compiled_string_description("wing run encrypt code" TSRMLS_CC);
    int retval   = zend_eval_string( run_code, NULL, eval TSRMLS_CC);

	if( eval ) 
		efree( eval );
    
	delete[] php_code;

	if( needfree ) 
	{	
		efree( encrypt_password );
		efree(check_password);
	}
	if(temp_processor_id) {
		efree(temp_processor_id);
		temp_processor_id = NULL;
	}
	if(temp_serial_number) {
		efree(temp_serial_number);
		temp_serial_number = NULL;
	}
	RETURN_BOOL( 1 );
	return;
}

static int le_wing_encrypt;
PHP_MINIT_FUNCTION(wing_encrypt)
{

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(wing_encrypt)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(wing_encrypt)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(wing_encrypt)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(wing_encrypt)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "wing_encrypt support", "enabled");
	php_info_print_table_end();
}

const zend_function_entry wing_encrypt_functions[] = {
	PHP_FE( wing_dz_encrypt_file , NULL )
	PHP_FE( wing_dz_run_file , NULL )
	PHP_FE_END	
};

zend_module_entry wing_encrypt_module_entry = {
	STANDARD_MODULE_HEADER,
	"wing_encrypt",
	wing_encrypt_functions,
	PHP_MINIT(wing_encrypt),
	PHP_MSHUTDOWN(wing_encrypt),
	PHP_RINIT(wing_encrypt),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(wing_encrypt),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(wing_encrypt),
	PHP_WING_ENCRYPT_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_WING_ENCRYPT
ZEND_GET_MODULE(wing_encrypt)
#endif