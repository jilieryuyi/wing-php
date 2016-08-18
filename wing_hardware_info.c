#include "php_wing.h"
#include "hardware_info.h"
#include "wing_string.h"
#include <Iphlpapi.h>  
#include <string.h>  
#include <ctype.h>  
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"

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

	HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
		return;
    }

    hres =  CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );

	if( FAILED(hres) )
    {
        CoUninitialize();
		return;
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if( FAILED(hres) )
    {
        CoUninitialize();
		return;
    }

    IWbemServices *pSvc = NULL;
    hres                = pLoc->ConnectServer( _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0,  NULL, 0, 0, &pSvc );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
		return;
    }


    hres = CoSetProxyBlanket(  pSvc,  RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE  );

    if (FAILED(hres))
    {
       
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;           
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery( bstr_t("WQL"),  bstr_t("SELECT * FROM Win32_Processor "),  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL, &pEnumerator );
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize(); 
        return; 
    }
 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn             = 0;
	
	
	
	
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if(0 == uReturn)
        {
            break;
        }

		zval *item;
		MAKE_STD_ZVAL( item );
		array_init( item );

        VARIANT vtProp;
        hr = pclsObj->Get(L"ProcessorId", 0, &vtProp, 0, 0);
		if( vtProp.bstrVal )
		{
			WingString _temp_processor_id( (const wchar_t *)vtProp.bstrVal );
			int size = _temp_processor_id.length();
			char *temp_processor_id = new char[size+1];
			memset(temp_processor_id,0,size+1);
			memcpy(temp_processor_id,_temp_processor_id.c_str(),size);
			if( temp_processor_id )
			{
				add_assoc_string( item, "processor_id", temp_processor_id, 1 );
				delete[] temp_processor_id;
			}
		}
		
        VariantClear(&vtProp);
		add_next_index_zval( return_value, item );

		if(pclsObj!=NULL)
		{
			pclsObj->Release();
			pclsObj=NULL;
		}
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
	if(pclsObj!=NULL)
	{
		pclsObj->Release();
		pclsObj=NULL;
	}
    CoUninitialize();
}


ZEND_FUNCTION( wing_get_serial_number ){
	
	array_init(return_value);

	HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
        return; 
    }

    hres =  CoInitializeSecurity( NULL,  -1, NULL,   NULL,  RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,  NULL,  EOAC_NONE,  NULL );

	if (FAILED(hres))
    {
        CoUninitialize();
        return;                    
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance( CLSID_WbemLocator, 0,  CLSCTX_INPROC_SERVER,  IID_IWbemLocator, (LPVOID *) &pLoc );
 
    if (FAILED(hres))
    {
        CoUninitialize();
        return;                
    }


    IWbemServices *pSvc = NULL;
 
    hres = pLoc->ConnectServer(  _bstr_t(L"ROOT\\CIMV2"),  NULL, NULL,   0,  NULL,  0, 0,  &pSvc  );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
        return;                
    }


    hres = CoSetProxyBlanket( pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;              
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery( bstr_t("WQL"),  bstr_t("SELECT * FROM Win32_PhysicalMedia"),  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,  NULL, &pEnumerator );
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;               
    }


    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

		zval *item;
		MAKE_STD_ZVAL( item );
		array_init( item );
		//add_assoc_long(      item,"process_id",        process.process_id          );

        VARIANT SerialNumber;

        hr = pclsObj->Get(L"SerialNumber", 0, &SerialNumber, 0, 0);

		if( SerialNumber.bstrVal )
		{	
			WingString _temp_serial_number(  (const wchar_t *)SerialNumber.bstrVal );
			int size = _temp_serial_number.length();
			char *temp_serial_number = new char[size+1];
			memset( temp_serial_number,0,size+1);
			memcpy( temp_serial_number,_temp_serial_number.c_str(),size );
			if( temp_serial_number ) 
			{
				add_assoc_string( item, "serial_number", temp_serial_number, 1 );
				delete[] temp_serial_number;
			}
		}
		VariantClear(&SerialNumber);

		
		add_next_index_zval( return_value, item );

       
		if(pclsObj!=NULL)
		{
			pclsObj->Release();
			pclsObj=NULL;
		}
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
	if(pclsObj!=NULL)
	{
		pclsObj->Release();
		pclsObj=NULL;
	}
   CoUninitialize();
}