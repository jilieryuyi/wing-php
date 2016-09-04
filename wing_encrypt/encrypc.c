#include "Windows.h"
#include <ctype.h>  
#include "Shlwapi.h"
#include <Iphlpapi.h>  
#include <string.h>  
#include <comdef.h>
#include <Wbemidl.h>
#include "wing_string.h"
#include <stdio.h>  
#include <string.h>  
#include <conio.h>  
#include <wincrypt.h> 
#include "io.h"

#pragma comment( lib, "Shlwapi.lib" )
#pragma comment( lib, "wbemuuid.lib" )


#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)  
#define KEYLENGTH  0x00800000  
#define ENCRYPT_ALGORITHM CALG_RC4   
#define ENCRYPT_BLOCK_SIZE 8   
 

HCRYPTKEY GenKeyByPassword(HCRYPTPROV hCryptProv,PCHAR szPassword);  
HCRYPTPROV GetCryptProv();    

/**
 * @ 解密文件 szDestination 参数为得到的文件内容，无需分配内存，会自动按需分配 使用完需要delete[] szDestination
 * @ szSource为需要解密的文件路径
 * @ szPassword为解密密码
 */
BOOL WingDecryptFile( PCHAR szSource, PCHAR &szDestination, PCHAR szPassword)   
{  
	szDestination = NULL;

    FILE *hSource;     
    HCRYPTPROV hCryptProv;   
    HCRYPTKEY hKey;   
    PBYTE pbBuffer;   
    size_t dwBlockLen;   
    size_t dwBufferLen;   
    size_t dwCount;   
    int size = 0;
   
    fopen_s( &hSource,szSource, "rb" );   
  
	size = _filelength(_fileno(hSource));
	
	szDestination = new char[size+1];
	if( !szDestination )
	{	
		szDestination = NULL;
		return 0;
	}

	memset( szDestination,0,size+1);

    hCryptProv = GetCryptProv(); 
	if( NULL == hCryptProv)
		return  0;

    hKey = GenKeyByPassword( hCryptProv, szPassword);  

	if( NULL == hKey )
		return 0;
      
    dwBlockLen  = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;   
    dwBufferLen = dwBlockLen;   
 
  
    if(!(pbBuffer = (BYTE *)malloc(dwBufferLen)))  
    {  
		return 0;  
    }  

	char *start = szDestination;
	size_t readdata = 0;
    do 
	{   
        dwCount = fread( pbBuffer, (size_t)1, dwBlockLen, hSource );   
        if(ferror(hSource))  
        {  
            return 0;
        }  

        // 解密 数据  
        if( !CryptDecrypt( hKey, 0, feof(hSource), 0, pbBuffer, (DWORD*)&dwCount ))  
        {  
			return 0; 
        }  

		memcpy( start, pbBuffer, dwCount );
		start += dwCount;	

		readdata += dwCount;
		if( readdata >= size) break;

    } while( !feof( hSource ) );   
  
    
    if( hSource )  
    {  
        fclose(hSource);   
    }   

    if( pbBuffer )   
    {    
		free(pbBuffer);
	}
   
    if( hKey )  
    {  
        CryptDestroyKey(hKey); 
    }   
  
    if( hCryptProv )  
    {  
        CryptReleaseContext(hCryptProv, 0); 
    }   
  
    return 1;  
} 
  
   
/**
 * @ 加密 szSource文件，加密后的数据存储在 szDestination 文件中   
 * @ szSource 原文文件名  
 * @ szDestination 加密后数据存储文件  
 * @ szPassword 加密秘钥 
 */
BOOL WingEncryptFile( PCHAR szSource, PCHAR szDestination,  PCHAR szPassword)  
{  

    FILE *hSource;   
    FILE *hDestination;   
    HCRYPTPROV hCryptProv;   
    HCRYPTKEY hKey;   
    PBYTE pbBuffer;   
    size_t dwBlockLen;   
    size_t dwBufferLen;   
    size_t dwCount;   
   
	fopen_s( &hSource,szSource,"rb" );
	fopen_s( &hDestination,szDestination,"wb" );
  
	size_t file_size = _filelength(_fileno(hSource));

    //获取加密服务者句柄  
    hCryptProv = GetCryptProv();  
	if( NULL == hCryptProv )
		return 0;
  
    hKey = GenKeyByPassword( hCryptProv, szPassword);  
	if( NULL == hKey )
		return  0;
         
    // 因为加密算法按ENCRYPT_BLOCK_SIZE 大小块加密，所以被加密的  
    // 数据长度必须是ENCRYPT_BLOCK_SIZE 的整数倍。下面计算一次加密的  
    // 数据长度。  
  
    dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;   

    // 确定加密后密文数据块大小. 若是分组密码模式，则必须有容纳额外块的空间    

    if(ENCRYPT_BLOCK_SIZE > 1)   
        dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE;   
    else   
        dwBufferLen = dwBlockLen;   
      
    // 分配内存空间.   
    if( !(pbBuffer = (BYTE *)malloc(dwBufferLen)) )  
    {  
        return 0;    
	}  
   

	size_t readdata = 0;
    do   
    {      
        dwCount = fread(pbBuffer, 1, dwBlockLen, hSource);   
        if( ferror(hSource) )  
        {   
            return 0;
        }  
		if(dwCount<=0)break;
		
        if(!CryptEncrypt(  
            hKey,           //密钥  
            0,              //如果数据同时进行散列和加密，这里传入一个散列对象  
            feof(hSource),  //如果是最后一个被加密的块，输入TRUE.如果不是输入FALSE.  
                            //这里通过判断是否到文件尾来决定是否为最后一块。  
            0,              //保留  
            pbBuffer,       //输入被加密数据，输出加密后的数据  
            (DWORD *)&dwCount,       //输入被加密数据实际长度，输出加密后数据长度  
            (DWORD)dwBufferLen))   //pbBuffer的大小。  
        {   
           return 0;
        }   
  //return FALSE;
        // 把加密后数据写到密文文件中   
        fwrite(pbBuffer, 1, dwCount, hDestination); 
		
        if(ferror(hDestination))  
        {   
            return 0;
        }  
		readdata +=dwCount;
		if(readdata>=file_size) break;
    }   while(!feof(hSource));   
  

    if(hSource)  
    {  
        fclose(hSource);
    }  
    if( hDestination )  
    {  
        fclose(hDestination);  
    }  
  
    if( pbBuffer )   
    {    
		free(pbBuffer);  
	}
 
    if( hKey )  
    {  
        CryptDestroyKey(hKey);  
    }   
  
    if( hCryptProv )  
    {  
        CryptReleaseContext(hCryptProv, 0);  
    }  
    return 1;   
}
  
  
/**
 * @ 获取加密提供者句柄
 */
HCRYPTPROV GetCryptProv()  
{  
    HCRYPTPROV hCryptProv;          

    if( !CryptAcquireContext( &hCryptProv, NULL,  MS_ENHANCED_PROV, PROV_RSA_FULL,  0) )
    {  
        if(!CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))  
        {  
           return NULL;  
        }  
    }  
    return hCryptProv;  
}  
 
  
HCRYPTKEY GenKeyByPassword(HCRYPTPROV hCryptProv,PCHAR szPassword)  
{  
    HCRYPTKEY hKey;   
    HCRYPTHASH hHash;  

    if( !CryptCreateHash( hCryptProv, CALG_MD5, 0, 0, &hHash ) )  
    {  
		return NULL;
    }    

    if( !CryptHashData( hHash, (BYTE *)szPassword, (DWORD)strlen(szPassword), 0))  
    {  
        return NULL;  
    }  

    if( !CryptDeriveKey( hCryptProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey ) )  
    {  
       return NULL;  
    }  

    if( hHash )   
    {  
        CryptDestroyHash(hHash);   
        hHash = 0;  
    }  
 
    return hKey;  
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
    hres = pSvc->ExecQuery( _bstr_t("WQL"),  _bstr_t("SELECT * FROM Win32_Processor "),  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL, &pEnumerator );
    
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
				size_t len = strlen(temp_processor_id);
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
			if( temp_serial_number ) {
				size_t len = strlen(temp_serial_number);
				memcpy( start, temp_serial_number, len );
				start += len;
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

	wing_trim( serial_number );

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