#include "php_wing.h"
#include "wing_wmic.h"

zend_class_entry *wing_com_ce;

/**
 * @ 构造函数
 */
ZEND_METHOD( wing_com, __construct ){
	WingWmic *com = new WingWmic();
	zend_update_property_long( wing_com_ce, getThis(), "com", strlen("com"), (long)(com) TSRMLS_CC );
}

/**
 * @ 析构函数
 */
ZEND_METHOD( wing_com, __destruct ){

	zval *_com  = zend_read_property( wing_com_ce, getThis(), "com", strlen("com"),	0 TSRMLS_CC);
	long com    = Z_LVAL_P(_com);
	if( com > 0 )
	{
		WingWmic *wcom = (WingWmic *)com;
		delete wcom;
	}

}

/**
 * @ 获取某个key
 */
ZEND_METHOD( wing_com, get ){
	char *key = NULL;
	int key_len = 0;
	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s",&key, &key_len ) != SUCCESS ) 
	{
		RETURN_EMPTY_STRING();
		return;
	}
	zval *_com  = zend_read_property( wing_com_ce, getThis(),"com",		     strlen("com"),	        0 TSRMLS_CC);
	long com    = Z_LVAL_P(_com);
	char *res = NULL;
	char *temp = NULL;
	if( com > 0 )
	{
		WingWmic *wcom = (WingWmic *)com;
		temp = wcom->get((const char*)key);
		if( temp ) { 
			spprintf(&res,0,"%s",temp);
			free(temp);
			RETURN_STRING(res,0);
		}
		else
			RETURN_EMPTY_STRING();
	}

	RETURN_EMPTY_STRING();

}

/**
 * @ 查询某个sql
 */
ZEND_METHOD( wing_com, query ){
	char *sql = NULL;
	int sql_len = 0;
	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s",&sql, &sql_len ) != SUCCESS ) 
	{
		RETURN_EMPTY_STRING();
		return;
	}
	zval *_com  = zend_read_property( wing_com_ce, getThis(),"com",		     strlen("com"),	        0 TSRMLS_CC);
	long com    = Z_LVAL_P(_com);

	if( com > 0 )
	{
		WingWmic *wcom = (WingWmic *)com;
		wcom->query( (const char*)sql );
		RETURN_TRUE;
	}

	RETURN_FALSE;

}

/** 
 *@判断是否还有吓一条数据
 */
ZEND_METHOD( wing_com, next ){
	
	zval *_com  = zend_read_property( wing_com_ce, getThis(),"com",		     strlen("com"),	        0 TSRMLS_CC);
	long com    = Z_LVAL_P(_com);
	if( com > 0 )
	{
		WingWmic *wcom = (WingWmic *)com;
		RETURN_BOOL(wcom->next());
	}
	RETURN_FALSE;
				
}
