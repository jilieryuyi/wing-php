#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php_wing.h"

#include <Winsock2.h>
#include "Windows.h"
#include "Winbase.h"

#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"
#include "process.h"
#include <mswsock.h>

#include <ws2tcpip.h>


#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#include "wing_message_queue.h"
#include "wing_iocp_message_queue.h"
#include "wing_utf8.h"
#include "wing_socket_api.h"
#include "wing_ntdll.h"
#include "wing_base.h"

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

	if( !iocp_socket_send( (SOCKET)isocket , msg, msg_len )) 
	{
		RETURN_LONG(WING_ERROR_FAILED);
		return;
	}
	RETURN_LONG(WING_ERROR_SUCCESS);
	return;
				
}
