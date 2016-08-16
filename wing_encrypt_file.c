
#include "php_wing.h"
#include "encrypt.h"


ZEND_FUNCTION( wing_test ){
	RETURN_BOOL(1);
}

ZEND_FUNCTION( wing_encrypt_file )
{
	
	char *input_file = NULL;
	int input_file_len = 0;

	char *output_file = NULL;
	int output_file_len = 0;

	char *encrypt_password = NULL;
	int password_len = 0;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sss",&input_file,&input_file_len,&output_file,&output_file_len,&encrypt_password,&password_len) != SUCCESS) {
		RETURN_BOOL( 0 );
		return;
	}

	
	RETURN_BOOL( WingEncryptFile( input_file , output_file , encrypt_password ) );
}


ZEND_FUNCTION( wing_run_file )
{

	char *input_file;
	int input_file_len = 0;

	char *encrypt_password;
	int password_len = 0;

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&input_file,&input_file_len,&encrypt_password,&password_len) != SUCCESS) {
		RETURN_LONG( 0 );
		return;
	}

	char *php_code = NULL;
	WingDecryptFile( input_file , php_code , encrypt_password );

	if( NULL == php_code )
	{	
		RETURN_BOOL(0);
		return;
	}

	int code_len   = strlen( php_code );
	char *run_code = php_code;

	//跳过<?php头
	if( php_code[0] == '<')
		run_code+=5;

	//去掉?>结尾
	if( php_code[code_len-1] == '>')
	{	
		php_code[code_len-1] = '\0';
		php_code[code_len-2] = '\0';
	}



	char *eval   = zend_make_compiled_string_description("wing run encrypt code" TSRMLS_CC);
    int retval   = zend_eval_string( run_code, NULL, eval TSRMLS_CC);

	if( eval ) efree( eval );
    delete[] php_code;

	RETURN_BOOL( 1 );
	return;
}
