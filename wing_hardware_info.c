#include "php_wing.h"
#include "wing_utf8.h"
#include <Iphlpapi.h>  
#pragma comment (lib, "Iphlpapi")  
#pragma comment (lib, "ws2_32")  

#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
  
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


void ToHex(const unsigned char * szOrigin, int nSize, char * szHex)
{
 char szTemp[10];
 for(int nIndex = 0; nIndex < nSize; nIndex ++)
 {
  sprintf(szTemp, "%02X", szOrigin[nIndex]);
  if(nIndex == 0)
  {
   strcpy(szHex, szTemp);
  }
  else
  {
   strcat(szHex, szTemp);
  }
 }
}

ZEND_FUNCTION( wing_get_cpu_id ){
	
	 HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
        return; 
    }

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

	if (FAILED(hres))
    {
        CoUninitialize();
        return;                    // Program has failed.
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        CoUninitialize();
        return;                 // Program has failed.
    }


    IWbemServices *pSvc = NULL;
 
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
        return;                // Program has failed.
    }


    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
       
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;               // Program has failed.
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_Processor "),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;               // Program has failed.
    }

 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
	char *senumber = NULL;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"ProcessorId", 0, &vtProp, 0, 0);

		//这个就是cpu id
		//wcout << 
			char *nu = WcharToUtf8( (const wchar_t *)vtProp.bstrVal ); //<< endl;
			
			if(nu){
			spprintf(&senumber,0,"%s",nu);
			delete[] nu;
			}
			

        VariantClear(&vtProp);
    }


    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
	if(pclsObj!=NULL)
	pclsObj->Release();
    CoUninitialize();
	RETURN_STRING(senumber,0);
}


ZEND_FUNCTION( wing_get_serial_number ){
	//硬盘的序列号
    HRESULT hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if( FAILED(hres) )
    {
        return; 
    }

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

	if (FAILED(hres))
    {
        CoUninitialize();
        return;                    // Program has failed.
    }
    
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        CoUninitialize();
        return;                 // Program has failed.
    }


    IWbemServices *pSvc = NULL;
 
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        pLoc->Release();     
        CoUninitialize();
        return;                // Program has failed.
    }


    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
       
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return;               // Program has failed.
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_PhysicalMedia"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;               // Program has failed.
    }

 
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;
	char *senumber = NULL;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);

		//这个就是硬盘序列号啦
		//wcout << 
			char *nu = WcharToUtf8( (const wchar_t *)vtProp.bstrVal ); //<< endl;
			
			if(nu){
			spprintf(&senumber,0,"%s",nu);
			delete[] nu;
			}
			

        VariantClear(&vtProp);
    }


    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
	if(pclsObj!=NULL)
	pclsObj->Release();
    CoUninitialize();
	RETURN_STRING(senumber,0);
}