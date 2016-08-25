// WingWmic.cpp : 定义控制台应用程序的入口点。
// author yuyi
// email 297341015@qq.com
// 项目地址 http://www.itdfy.com/git/

#include "stdafx.h"
#define _WIN32_DCOM
#include <atlcomcli.h>
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"

#pragma comment(lib, "wbemuuid.lib")

class WingWmic{
private:
	 IWbemServices *pSvc; 
	 int has_error; 
	 IEnumWbemClassObject* pEnumerator; 
	 IWbemLocator *pLoc; 
	 char *query_table;
	 IWbemClassObject *pclsObj;
public:
	WingWmic();
	~WingWmic();
	void query( const char *sql );
	BOOL next();
	char* get( const char *key);
};

WingWmic::WingWmic(){
	 
	this->pSvc			= NULL;
	this->pEnumerator	= NULL;
	this->has_error		= 0;
	this->pLoc			= NULL;
	this->query_table   = NULL;
	this->pclsObj       = NULL;

	HRESULT hres;
    hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hres))
    {
		this->has_error = 1;
        return;             
    }

    hres =  CoInitializeSecurity( NULL, -1,  NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );

	if (FAILED(hres))
    {
		this->has_error = 1;
        return ;
    }

   
    hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,  IID_IWbemLocator, (LPVOID *) &pLoc );
 
    if (FAILED(hres))
    {
		this->has_error = 1;
        return ;            
    }

    hres = pLoc->ConnectServer( _bstr_t(L"ROOT\\CIMV2"), NULL, NULL,  0,  NULL,  0,  0,   &this->pSvc  );
    
    if (FAILED(hres))
    {   
		this->has_error = 1;
        return ;             
    }

    hres = CoSetProxyBlanket( pSvc,  RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,  NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,  NULL,  EOAC_NONE );

    if (FAILED(hres))
    {    
		this->has_error = 1;
        return;        
    }

}

WingWmic::~WingWmic(){
	if( pSvc != NULL ) 
		pSvc->Release();
    if( pLoc != NULL ) 
		pLoc->Release();
    if( pEnumerator != NULL )
		pEnumerator->Release();
	if( this->query_table != NULL ) 
		delete[] this->query_table;
	if( this->pclsObj != NULL)
		this->pclsObj->Release();
    CoUninitialize();
}


void WingWmic::query( const char* _sql ){
   
	if( this->has_error ) return;
	char *sql = _strdup( _sql );

	int i = 0;
	while( sql[i] != '\0' ){
		sql[i] = tolower(sql[i]);
		i++;
	}

	char *from = strstr( sql , "from" )+4;
	while( 1 ) {
		char c = *from;
		if( !isspace(c) ) break;
		from++;
	}

	//得到查询的是那张表
	this->query_table = new char[32];
	memset( this->query_table , 0 , 32);
	int index = 0;
	while(1) {
		char c = *from;
		if( isspace(c) || c == '\0' ) break;
		this->query_table[index] = c;
		index++;
		from++;
	}

	HRESULT hres = pSvc->ExecQuery( bstr_t("WQL"), bstr_t(sql),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,&this->pEnumerator);
    
	if (FAILED(hres))
	{
		this->has_error = 1;
	}

	delete[] sql;
}

BOOL WingWmic::next(){

	if( pclsObj != NULL ) {
		pclsObj->Release();
		pclsObj=NULL;
	}

	ULONG uReturn = 0;
	HRESULT hr = pEnumerator->Next( WBEM_INFINITE, 1,  &pclsObj, &uReturn );

	return uReturn;
}

char* WingWmic::get( const char *key){

	if( this->has_error ) 
	{
		pEnumerator->Release();
		pEnumerator = NULL;
		return NULL;
	}


	VARIANT vtProp;

	wchar_t *wkey = wing_str_char_to_wchar( key );


	HRESULT hr = pclsObj->Get( wkey , 0, &vtProp, 0, 0);

	char *res = NULL;
	if( SUCCEEDED( hr ) && vtProp.bstrVal )
	{	
		printf("%ld\r\n",vtProp.vt);
		//res = wing_str_wchar_to_char( (const wchar_t*)vtProp.bstrVal );
		switch ( vtProp.vt ){  
		case VT_BSTR: {  
			res = wing_str_wchar_to_char( (const wchar_t*)vtProp.bstrVal );
					  }break;  
		case VT_I1:  
		case VT_I2:  
		case VT_I4:  
		case VT_I8:   
		case VT_INT: {  
			res = (char*)malloc(10);
			memset(res,0,10);
			//wprintf(L"%d\r\n",vtProp.intVal);
			//sprintf(res,"%d",vtProp.intVal);

					 }break;  
		case VT_UI8:  
		case VT_UI1:      
		case VT_UI2:  
		case VT_UI4:  
		case VT_UINT:{  
			//wprintf(L"0x%u\r\n",vtProp.intVal); 
			res = (char*)malloc(32);
			memset(res,0,32);
			//wprintf(L"%d\r\n",vtProp.intVal);
//			sprintf(res,"0x%u",vtProp.intVal);

					 }break;  
		case VT_BOOL:{  
			//wprintf(L"%s\r\n", vtProp.boolVal ? L"TRUE" : L"FASLE" );  
			res = (char*)malloc(10);
			memset(res,0,10);
			//wprintf(L"%d\r\n",vtProp.intVal);
		//	sprintf(res,"%s",vtProp.boolVal ? "TRUE" : "FASLE" );

					 }break;  
		default:{  
			//ATLASSERT(FALSE);  
				};  
		}  

	}

	VariantClear(&vtProp);
	if(wkey) free(wkey);

	return res;
}


int _tmain(int argc, _TCHAR* argv[])
{
	char sql[] = "SELECT * FROM Win32_Process";
 
	//char *sql = "SELECT * FROM Win32_Processor";

	WingWmic mic;
	
	mic.query(sql);
	int count = 1;
	char *c = NULL;

	while( mic.next() ) {
		
		//c = mic.get("CommandLine");
		//printf("CommandLine:%ld=>%s\r\n",count,c);
		//c = mic.get("Caption");
		//printf("Caption:%ld=>%s\r\n",count,c);
		//c = mic.get("CSCreationClassName");
		//printf("CSCreationClassName:%ld=>%s\r\n",count,c);

		c = mic.get("CreationDate");
		printf("CreationDate:%ld=>%s\r\n",count,c);

		count++;
	}
   
	return 0;
}

