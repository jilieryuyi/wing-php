
#include "php_wing.h"
#include "wing_wmic.h" 
#include "hardware_info.h"
#include "wing_string.h"
#include <Iphlpapi.h>  
#include <string.h>  
#include <ctype.h>  
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"

#include "wing_malloc.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Iphlpapi")  
#pragma comment(lib, "ws2_32")  

/**
 * @获取网卡适配器信息
 */
ZEND_FUNCTION( wing_adapters_info ) 
{  
	array_init( return_value );
    PIP_ADAPTER_INFO pIpAdaptTab = NULL;  
    ULONG ulLen = 0;  
  
    GetAdaptersInfo(pIpAdaptTab, &ulLen);  
    if ( ulLen == 0 )  
    {  
        return ;  
    }  
  
    pIpAdaptTab = (PIP_ADAPTER_INFO)malloc(ulLen);  
    if ( pIpAdaptTab == NULL )  
    {  
        return;  
    }  
  
    GetAdaptersInfo(pIpAdaptTab, &ulLen);  
    PIP_ADAPTER_INFO pTmp = pIpAdaptTab;  
    while ( pTmp != NULL )  
    {  
       
		zval *item;
		MAKE_STD_ZVAL( item );
		array_init(item);

		char *mac_address;
		spprintf( &mac_address , 0 , "%02x-%02x-%02x-%02x-%02x-%02x", pTmp->Address[0], pTmp->Address[1], pTmp->Address[2], pTmp->Address[3], pTmp->Address[4], pTmp->Address[5]);

		add_assoc_string( item, "adapter_name",        pTmp->AdapterName,  1 );
		add_assoc_string( item, "adapter_description", pTmp->Description,  1 );
		add_assoc_string( item, "ip_address",          pTmp->IpAddressList.IpAddress.String,  1 );

		add_assoc_string( item, "mac_address",         mac_address,  0 );
		add_next_index_zval( return_value, item );
  
        pTmp = pTmp->Next;  
    }  
  
    free(pIpAdaptTab);  
  
    return;  
}  

ZEND_FUNCTION( wing_get_cpu_id ){

	array_init( return_value );

	char *sql = "SELECT * FROM Win32_Processor";

	WingWmic mic;
	
	mic.query(sql);
	char *processor_id_item = NULL;

	while( mic.next() ) {
		processor_id_item = mic.get("ProcessorId");
		if( processor_id_item != NULL )
		{	
			zval *item;
			MAKE_STD_ZVAL( item );
			array_init( item );
			add_assoc_string( item, "processor_id", processor_id_item, 1 );
			wing_free( processor_id_item );
			add_next_index_zval( return_value, item );
		}
	}
 
}


ZEND_FUNCTION( wing_get_serial_number ){
	
	array_init( return_value );

	char *sql = "SELECT * FROM Win32_PhysicalMedia";

	WingWmic mic;
	
	mic.query(sql);
	char *serial_number_item = NULL;

	while( mic.next() ) {
		serial_number_item = mic.get("SerialNumber");
		if( serial_number_item != NULL )
		{	
			zval *item;
			MAKE_STD_ZVAL( item );
			array_init( item );
			add_assoc_string( item, "serial_number", serial_number_item, 1 );
			wing_free( serial_number_item );
			add_next_index_zval( return_value, item );
		}
	}
 
	
}