/**
 * @ 生成秘钥
 */

#include "stdafx.h"
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
 
char* WcharToUtf8( const wchar_t *pwStr )
{
    if( pwStr == NULL )
    {
        return NULL;
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, pwStr, -1, NULL, 0, NULL, NULL);
    if (len <= 0)
    {
        return NULL;
    }
    char *pStr = new char[len+1];
	memset( pStr, 0, len+1 );
    WideCharToMultiByte( CP_UTF8, 0, pwStr, -1, pStr, len, NULL, NULL );
    return pStr;
}
void trim(char *s)   
{  
    char *start;  
    char *end;  
    int len = strlen(s);  
      
    start = s;  
    end = s + len - 1;  
  
    while (1)   
    {     
        char c = *start;  
        if (!isspace(c))  
            break;  
  
        start++;  
        if (start > end)  
        {     
            s[0] = '\0';  
            return;  
        }     
    }     
  
  
    while (1)   
    {     
        char c = *end;  
        if (!isspace(c))  
            break;  
  
        end--;  
        if (start > end)  
        {     
            s[0] = '\0';  
            return;  
        }  
    }  
  
    memmove(s, start, end - start + 1);  
    s[end - start + 1] = '\0';  
}  
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
		if( vtProp.bstrVal ){
			char *temp_processor_id = WcharToUtf8( (const wchar_t *)vtProp.bstrVal ); 
			if(temp_processor_id){
				int len = strlen(temp_processor_id);
				memcpy(start,temp_processor_id,len);
				start+=len;
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
 * @ 硬盘的序列号
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

	trim(serial_number);

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

int _tmain(int argc, _TCHAR* argv[])
{
	char *processor_id;
	char *serial_number;

	get_cpu_id( processor_id );
	get_serial_number( serial_number );
	int size = strlen(serial_number) +  strlen(processor_id) +1;

	char *encrypt_password = new char[size];

	
	sprintf_s( encrypt_password, size, "%s%s" , processor_id, serial_number  );

	printf("%s\r\n",encrypt_password);

	delete[] encrypt_password;
	delete[] processor_id;
	delete[] serial_number;

	return 0;
}

