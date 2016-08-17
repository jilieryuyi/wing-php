#include "hardware_info.h"
#include "wing_utf8.h"
#include <Iphlpapi.h>  
#include <string.h>  
#include <ctype.h>  
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Iphlpapi")  
#pragma comment(lib, "ws2_32")  

void get_cpu_id( char *&processor_id ){
	
	HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
		processor_id = NULL; 
		return;
    }

    hres =  CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );

	if( FAILED(hres) )
    {
        CoUninitialize();
        processor_id = NULL; 
		return;
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if( FAILED(hres) )
    {
        CoUninitialize();
        processor_id = NULL; 
		return;
    }

    IWbemServices *pSvc = NULL;
    hres                = pLoc->ConnectServer( _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0,  NULL, 0, 0, &pSvc );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
        processor_id = NULL; 
		return;
    }


    hres = CoSetProxyBlanket(  pSvc,  RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE  );

    if (FAILED(hres))
    {
       
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
		processor_id = NULL; 
        return;           
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery( bstr_t("WQL"),  bstr_t("SELECT * FROM Win32_Processor "),  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL, &pEnumerator );
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
		processor_id = NULL;  
        return; 
    }
 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn             = 0;
	int max_size = 1024;
	processor_id = new char[max_size];
	memset(processor_id,0,max_size);
	char *start = processor_id;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"ProcessorId", 0, &vtProp, 0, 0);
		if( vtProp.bstrVal )
		{
			char *temp_processor_id = WcharToUtf8( (const wchar_t *)vtProp.bstrVal ); 
			if( temp_processor_id )
			{
				int len = strlen( temp_processor_id );
				memcpy( start, temp_processor_id, len );
				start += len;
				delete[] temp_processor_id;
			}
		}
		
        VariantClear(&vtProp);
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

/**
 * @ Ó²ÅÌµÄÐòÁÐºÅ
 */
void get_serial_number( char *&serial_number )
{
    HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
		serial_number = NULL;
        return; 
    }

    hres =  CoInitializeSecurity( NULL,  -1, NULL,   NULL,  RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,  NULL,  EOAC_NONE,  NULL );

	if (FAILED(hres))
    {
        CoUninitialize();
		serial_number = NULL;
        return;                    
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance( CLSID_WbemLocator, 0,  CLSCTX_INPROC_SERVER,  IID_IWbemLocator, (LPVOID *) &pLoc );
 
    if (FAILED(hres))
    {
        CoUninitialize();
		serial_number = NULL;
        return;                
    }


    IWbemServices *pSvc = NULL;
 
    hres = pLoc->ConnectServer(  _bstr_t(L"ROOT\\CIMV2"),  NULL, NULL,   0,  NULL,  0, 0,  &pSvc  );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
		serial_number = NULL;
        return;                
    }


    hres = CoSetProxyBlanket( pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
		serial_number = NULL;
        return;              
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery( bstr_t("WQL"),  bstr_t("SELECT * FROM Win32_PhysicalMedia"),  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,  NULL, &pEnumerator );
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
		serial_number = NULL;
        return;               
    }

	int max_size = 1024;
	serial_number = new char[max_size];
	memset(serial_number,0,max_size);

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
	char *start = serial_number;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);

		if( vtProp.bstrVal )
		{	
			char *temp_serial_number = WcharToUtf8( (const wchar_t *)vtProp.bstrVal );
			if(temp_serial_number){
				int len = strlen(temp_serial_number);
				memcpy(start,temp_serial_number,len);
				start+=len;
				delete[] temp_serial_number;
			}
		}
	
        VariantClear(&vtProp);
		if(pclsObj!=NULL)
		{
			pclsObj->Release();
			pclsObj=NULL;
		}
    }

	wing_trim(serial_number);

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