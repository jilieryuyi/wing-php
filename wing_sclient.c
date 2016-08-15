
#include "php_wing.h"


/**
 *@发送线程
 */
DWORD WINAPI send_thread(PVOID *_node) {

	socket_send_node *node  = (socket_send_node*)_node;
	unsigned long _socket = (unsigned long)node->socket;

	int send_status = 1;
	int sendBytes   = send( node->socket ,node->msg ,node->len,0);

	if( SOCKET_ERROR == sendBytes ){
		send_status = 0;
	}

	if( node->msg ) delete[] node->msg;
	if( node )      delete node;

	iocp_post_queue_msg( WM_ONSEND,_socket, send_status );
	return 1;
}

/**
 *@发送消息
 */
BOOL socket_send( SOCKET socket,char *&msg , int len ) {

	socket_send_node *node = new socket_send_node();

	if( NULL == node ) {
		iocp_post_queue_msg( WM_ONSEND,(unsigned long)socket, 0 );
		return 0;
	}

	if( INVALID_SOCKET == socket ) {
		iocp_post_queue_msg( WM_ONSEND,(unsigned long)socket, 0 );
		delete node;
		return 0;
	}


	node->socket         = socket;
	node->msg            = new char[len];
	node->len            = len;
	
	memset( node->msg, 0, len );
	memcpy( node->msg , msg , len );


	BOOL ret = QueueUserWorkItem( (LPTHREAD_START_ROUTINE)send_thread, node, WT_EXECUTEINIOTHREAD);
	if( !ret ) {
		delete[] node->msg;
		delete node;

		iocp_post_queue_msg( WM_ONSEND,(unsigned long)socket, 0 );

		return 0;
	}
	return 1;
}
//socket -sclient------------------------------------------------------------
zend_class_entry *wing_sclient_ce;

/** 
 *@发送消息 支持同步、异步
 */
ZEND_METHOD( wing_sclient,send ){
	
	char *msg    = NULL;
	int  msg_len = 0;
	

	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s",&msg, &msg_len ) != SUCCESS ) 
	{
		RETURN_LONG(WING_ERROR_PARAMETER_ERROR);
		return;
	}
	zval *socket = zend_read_property( wing_sclient_ce ,getThis(),"socket",strlen("socket"),0 TSRMLS_CC);
	
	int isocket = Z_LVAL_P(socket);

	if( isocket <= 0 ) 
	{
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}

	if( !socket_send( (SOCKET)isocket , msg, msg_len )) 
	{
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	RETURN_LONG(WING_ERROR_SUCCESS);
	return;
				
}
