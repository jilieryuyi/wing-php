// WingWmic.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#define _WIN32_DCOM
#include <atlcomcli.h>
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"

# pragma comment(lib, "wbemuuid.lib")

class WingWmic{
private:
	 IWbemServices *pSvc; 
	 int has_error; 
	 IEnumWbemClassObject* pEnumerator; 
	 IWbemLocator *pLoc; 
	 char *query_table;
public:
	WingWmic();
	~WingWmic();
	void query( const char *sql );
	BOOL next();
	char* get( const char *key);
	//void print( const char* sql );
};

WingWmic::WingWmic(){
	 
	this->pSvc			= NULL;
	this->pEnumerator	= NULL;
	this->has_error		= 0;
	this->pLoc			= NULL;
	this->query_table   = NULL;

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

	printf("table=%s\r\n",this->query_table);

	HRESULT hres = pSvc->ExecQuery( bstr_t("WQL"), bstr_t(sql),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,&this->pEnumerator);
    
	if (FAILED(hres))
	{
		this->has_error = 1;
	}

	delete[] sql;
}

BOOL WingWmic::next(){
	return !!pEnumerator;
}

char* WingWmic::get( const char *key){
	
	SetLastError(0);
	
	if( this->has_error ) 
	{
		printf("has error %ld\r\n",GetLastError());
		pEnumerator->Release();
		pEnumerator = NULL;
		return NULL;
	}

	IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

	HRESULT hr = pEnumerator->Next( WBEM_INFINITE, 1,  &pclsObj, &uReturn );

	if( 0 == uReturn )
	{
		printf("\r\nfail %ld\r\n",GetLastError());
		if( pclsObj != NULL ) {
			pclsObj->Release();
			pclsObj=NULL;
		}
		pEnumerator->Release();
		pEnumerator = NULL;
        return NULL;
	}

	VARIANT vtProp;
	size_t len = 0;
	size_t nu   = strlen(key);
	 len  =(size_t)MultiByteToWideChar(CP_ACP,0,key,(int)nu,NULL,0)+1;

	wchar_t *wkey = new wchar_t[len];
    memset( wkey, 0, len );
	MultiByteToWideChar(CP_ACP,0,key,(int)nu,wkey, len );
	wkey[len-1]=L'\0';

	hr = pclsObj->Get( wkey , 0, &vtProp, 0, 0);

	wcout<<"key=>"<<wkey<<"<=="<<endl;

	char *res = NULL;
	if( SUCCEEDED( hr ) && vtProp.bstrVal )
	{	
		len = WideCharToMultiByte(CP_UTF8, 0, vtProp.bstrVal, -1, NULL, 0, NULL, NULL);
		if (len > 0)
		{
			res = new char[len+1];
			memset( res, 0, len+1 );
			WideCharToMultiByte( CP_UTF8, 0, vtProp.bstrVal, -1, res, len, NULL, NULL );
		}else
			wcout<<"convert fail key=>"<<wkey<<endl;
	}else{
		wcout<<"fail key=>"<<wkey<<endl;
	}
		
	VariantClear(&vtProp);

	if( pclsObj != NULL ) {
		pclsObj->Release();
		pclsObj=NULL;
	}

	return res;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char sql[] = "SELECT * FROM Win32_Process\0";
 
	WingWmic mic;
	
	mic.query(sql);
	int count = 1;
	char *c = NULL;

	while( mic.next() ) {
		
		c = mic.get("Caption");
		printf("\r\nCaption:%ld=>%s\r\n",count,c);

		//c = mic.get(L"CreationClassName");
		//printf("CreationClassName:%ld=>%s\r\n",count,c);

		//c = mic.get(L"CommandLine");
		//printf("CommandLine:%ld=>%s\r\n",count,c);

		count++;
	}
   
	return 0;
}

