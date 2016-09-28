// wmic_getnames.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Windows.h"
#include <comdef.h>
#include <Wbemidl.h>

#define WING_CHAR_SIZE(str)  (strlen((char*)str)+1)*sizeof(char)
#define WING_WCHAR_SIZE(str) (wcslen((wchar_t*)str)+1)*sizeof(wchar_t)

/**
 *@wchar_t转换编码为utf8
 */
char* wing_str_wchar_to_utf8( _In_ const wchar_t* _str ){
	if( _str == NULL )
		return NULL;
	int nLen = WideCharToMultiByte(CP_UTF8, 0, _str, -1, NULL, 0, NULL, NULL);
	if( nLen <= 0 ) return NULL;
	char*  m_cUtf8 = (char*)malloc(nLen + 1);  
	memset(m_cUtf8,0,nLen + 1);
	WideCharToMultiByte (CP_UTF8, 0, _str, -1, m_cUtf8, nLen, NULL,NULL); 
	return m_cUtf8;

}

/**
 *@wchar_t 转换为 char
 */
char* wing_str_wchar_to_char( _In_ const wchar_t* _str ){
	 if( _str == NULL )
		 return NULL;
	 int nLen = WideCharToMultiByte(CP_OEMCP,NULL, _str,-1,NULL,0,NULL,FALSE);  
	 if( nLen <= 0 ) return NULL;
     char * m_cDest = (char*)malloc(nLen); 
	 memset(m_cDest,0,nLen);
     WideCharToMultiByte (CP_OEMCP,NULL,_str,-1, m_cDest, nLen,NULL,FALSE);  
	 return m_cDest;	
}
wchar_t* wing_str_char_to_wchar( _In_ const char* _str ){
	
	int size     = WING_CHAR_SIZE( _str );
	int len      = MultiByteToWideChar(CP_ACP,0,(const char *)_str,size-1,NULL,0);

	int buf_size = (len+1)*sizeof(wchar_t);
	wchar_t* buf = (wchar_t*)malloc( buf_size );
	memset( buf, 0x0, buf_size );
	MultiByteToWideChar( CP_ACP,0,(const char *)_str,size-1,buf,len);   

	return buf;
}
char* wing_str_char_to_utf8( _In_ const char* str ){
	
	if( str == NULL )
		return NULL;

	wchar_t* unicode_str = NULL;
	int utf8_str_size    = 0;

	utf8_str_size      = ::MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, NULL );                   //获取转换到Unicode编码后所需要的字符空间长度
	unicode_str        = (wchar_t*)malloc((utf8_str_size + 1)*sizeof(wchar_t));                     //为Unicode字符串空间
	memset( unicode_str, 0x0, (utf8_str_size + 1)*sizeof(wchar_t) );
	utf8_str_size      = ::MultiByteToWideChar( CP_ACP, 0, str, -1, unicode_str, utf8_str_size );   //转换到Unicode编码
	
	if( !utf8_str_size )                                                                                 //转换失败则出错退出
	{
		if( unicode_str ) 
			delete[] unicode_str;
		return 0;
	}

	utf8_str_size  = WideCharToMultiByte(CP_UTF8,0,unicode_str,-1,NULL,0,NULL,NULL);                    //获取转换到UTF8编码后所需要的字符空间长度
	char *utf8_str = (char*)malloc(utf8_str_size+1);

	memset(utf8_str,0,utf8_str_size+1);

	utf8_str_size = WideCharToMultiByte( CP_UTF8, 0, unicode_str, -1, (char *)utf8_str, utf8_str_size+1, NULL, NULL );  
	                                                                                                    //转换到UTF8编码
	if( unicode_str )
		delete []unicode_str;

	if( !utf8_str_size )
		return 0;

	return utf8_str;
}

/**
 *@去除字符串两端空格
 */
void wing_str_trim( _Inout_ char* str ,int size ){
	if( str == NULL ) 
		return;
	if( size <= 0 )
		size = strlen( str );
	
	int len     = size;
	char *start = str;  
    char *end   = str + len - 1;  
  
	//找到第一个不为空的字符
    while (1)   
    {     
        char c = *start;  
        if (!isspace(c))  
            break;  
  
        start++;  
        if (start > end)  
        {     
            str[0] = '\0';  
            return;  
        }    
    }     
  
	//找到最后一个不为空的字符
    while (1)   
    {     
        char c = *end;  
        if ( !isspace(c) && c != '\0' )  //兼容传入的是size或是strlen
            break;  
  
        end--;  
        if (start > end)  
        {     
            str[0] = '\0';  
            return;  
        }  
    }  
  
	//复制区间
    memmove(str, start, end - start + 1);  
	//最后一个值清零
    str[end - start + 1] = '\0';  
}


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
#include "OleAuto.h"
#pragma comment(lib,"OleAut32.lib")

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
	/*char *sql = _strdup( _sql );

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
	}*/

	HRESULT hres = pSvc->ExecQuery( bstr_t("WQL"), bstr_t(_sql),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,&this->pEnumerator);

	if (FAILED(hres))
	{
		this->has_error = 1;
	}

	//delete[] sql;
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




void DecimalToString(VARIANT var,char *&buf)  
{              
               
	UINT64 u64  = (UINT64)var.decVal.Lo64;
	UINT64 iMod = 1; //原文章是int  iMod = 1;，改动一下
	UINT64 ui   = (UINT64)var.decVal.Lo64;       
	int _min    = min(var.decVal.signscale,var.decVal.scale);
	for( int  i = 0; i < _min ; i++ )              
	{                   

		ui /= 10;                  
		iMod *= 10;               
	}               

	UINT64 ud     = (UINT64)var.decVal.Lo64 % iMod;                  
	char  sz0[64] = {0};               
	
	_ui64toa_s(ui, sz0, 64, 10); 

	char sz1[64] = {0};
	char sz2[64] = {0};                 

	_ui64toa_s(ud, sz1,64, 10);               

	buf = (char*)malloc(128);
	memset(buf,0,128);

	int zeroSize = var.decVal.scale-strlen(sz1);
	for( int i=0; i<zeroSize; i++ )    //小数部分左边填充0
	{
		sprintf_s(sz2,64, "0%s", sz1);
		memcpy(sz1, sz2,64);
	}                                                                                                  

	if( var.decVal.sign < 128 )              
	{                   

		if (var.decVal.signscale > 0)                   
		{                       
			sprintf_s(buf,128, "%s.%s" , sz0, sz1);                   
		}                   

		else                    {                       
			sprintf_s(buf, 128,"%s" , sz0);                   

		}               
	}               

	else               
	{                   
		if (var.decVal.signscale > 0)                   
		{                       
			sprintf_s(buf,128, "-%s.%s" , sz0, sz1);                   
		}                   
		else                    
		{                       
			sprintf_s(buf,128, "-%s" , sz0);                   
		}               
	}                 

}  





/**
 * @ 返回值需要使用free释放
 */
char* WingWmic::get( const char *key){

	if( this->has_error ) 
	{
		pEnumerator->Release();
		pEnumerator = NULL;
		return NULL;
	}


	VARIANT vtProp;

	wchar_t *wkey = wing_str_char_to_wchar( key );

	SAFEARRAY  *names;
	 SAFEARRAYBOUND saBounds;

	  saBounds.cElements = 1024;
    saBounds.lLbound = 0;
    names = SafeArrayCreate(VT_BSTR, 1, &saBounds);



	
	pclsObj->GetNames( 0,
                     WBEM_FLAG_ALWAYS,
                     0,
                     &names);
	BSTR buf[256];
	memset(buf,0x0,256*(sizeof(BSTR)));

	for( long i =0;i<1024;i++){
		SafeArrayGetElement(names,&i,buf);
		wprintf(L"%s\r\n",buf);
	}

	return "";

	HRESULT hr    = pclsObj->Get( wkey , 0, &vtProp, 0, 0);
	char *res     = NULL;


	if( SUCCEEDED( hr ) && vtProp.bstrVal )
	{	
		//根据不同的类型进行格式化
		switch ( V_VT( &vtProp ) ){  

			case VT_BSTR:   //字符串
				res = wing_str_wchar_to_utf8( (const wchar_t*)vtProp.bstrVal );
				break;
			case VT_LPSTR:  //字符串
				{
					int size = sizeof( strlen((char*)vtProp.pvRecord)+1 );
					res = (char*)malloc(size);
					memset( res, 0, size );
					memcpy( res, vtProp.pvRecord, size-1 );
				}
				break;
			case VT_LPWSTR: //字符串
				{
					res = wing_str_wchar_to_utf8( (const wchar_t*)vtProp.pvRecord );
				}
				break;
			case VT_I1:
			case VT_UI1:
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%d", vtProp.bVal);
				}
				break;

			case VT_I2://短整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%d", vtProp.iVal);
				}
				break;

			case VT_UI2://无符号短整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%d", vtProp.uiVal);
				}
				break;

			case VT_INT://整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%d", vtProp.intVal);
				}
				break;

			case VT_I4: //整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%d", vtProp.lVal);
				}
				break;

			case VT_I8: //长整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32,"%ld", vtProp.bVal);
				}
				break;

			case VT_UINT://无符号整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res, 32,"%u", vtProp.uintVal);
				}
				break;

			case VT_UI4: //无符号整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res, 32,"%u", vtProp.ulVal);
				}
				break;

			case VT_UI8: //无符号长整型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res, 32,"%u", vtProp.ulVal);
				}
				break;

			case VT_VOID:
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%8x", (unsigned int)vtProp.byref);
				}
				break;

			case VT_R4://浮点型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res, 32,"%.4f", vtProp.fltVal);
				}
				break;

			case VT_R8://双精度型
				{
					res = (char*)malloc(32);
					memset(res,0,32);
					sprintf_s(res,32, "%.8f", vtProp.dblVal);
				}
				break;

			case VT_DECIMAL: //小数
			
				DecimalToString(vtProp,res);

				break;

			case VT_CY:
				{
					//vtProp.cyVal.Hi
					//COleCurrency cy = vtProp.cyVal;
					//strValue = cy.Format();
				}
				break;

			case VT_BLOB:
			case VT_BLOB_OBJECT:
			case 0x2011:
				//strValue = "[BLOB]";
				break;

			case VT_BOOL://布尔型
				{
					res = (char*)malloc(5);
					memset(res,0,5);
					sprintf_s(res,5,"%s",vtProp.boolVal ? "TRUE" : "FASLE" );
				}
				break;

			case VT_DATE: //日期型
				{
					SYSTEMTIME st = {0};
					res = (char*)malloc(32);
					memset(res,0,32);
					VariantTimeToSystemTime(vtProp.date,&st);
					sprintf_s(res,32,"%04d-%02d-%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,   st.wHour, st.wMinute, st.wSecond);
	
				}
				break;

			case VT_NULL://NULL值
				//strValue = "VT_NULL";
				break;

			case VT_EMPTY://空
				//strValue = "";
				break;

			case VT_UNKNOWN://未知类型
			default:
				//strValue = "UN_KNOWN";
				break;
			
		}  

	}

	VariantClear(&vtProp);
	if(wkey) free(wkey);

	return res;
}


int _tmain(int argc, _TCHAR* argv[])
{
	WingWmic *wcom = new WingWmic();//(WingWmic *)com;
	wcom->query("SELECT * FROM Win32_Process");
	//while(wcom->next()){
	wcom->next();
		printf("%s\r\n",wcom->get("CommandLine"));
	//}

	return 0;
}

