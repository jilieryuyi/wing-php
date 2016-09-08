#define _CRT_GETPUTWCHAR_NOINLINE
#include "encrypt.h"
#include <stdio.h>  
#include <string.h>  
#include <conio.h>  
#include <wincrypt.h> 
#include <io.h>
#include "wing_malloc.h"

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)  
#define KEYLENGTH  0x00800000  
#define ENCRYPT_ALGORITHM CALG_RC4   
#define ENCRYPT_BLOCK_SIZE 8   
 

HCRYPTKEY GenKeyByPassword(HCRYPTPROV hCryptProv,PCHAR szPassword);  
HCRYPTPROV GetCryptProv();    
/**
 *@解密文件 szDestination参数为得到的文件内容，无需分配内存，会自动按需分配 使用完需要delete[] szDestination
 */
BOOL WingDecryptFile( PCHAR szSource, PCHAR &szDestination, PCHAR szPassword)   
{   
    FILE *hSource;     
    HCRYPTPROV hCryptProv;   
    HCRYPTKEY hKey;   
    PBYTE pbBuffer;   
    size_t dwBlockLen;   
    DWORD dwBufferLen;   
    DWORD dwCount;   
    int size = 0;
   
    fopen_s( &hSource,szSource, "rb" );   
  
	size = _filelength(_fileno(hSource));
	

	szDestination = (PCHAR)wing_malloc( size+1 );
	if( !szDestination )
	{	
		szDestination = NULL;
		return 0;
	}

	memset( szDestination,0,size+1);

    hCryptProv = GetCryptProv(); 
	if( NULL == hCryptProv)
		return  0;


	

    hKey       = GenKeyByPassword( hCryptProv, szPassword);  

	if( NULL == hKey )
		return 0;
      
    dwBlockLen  = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;   
    dwBufferLen = dwBlockLen;   
 
  
    if(!(pbBuffer = (BYTE *)malloc(dwBufferLen)))  
    {  
		return 0;  
    }  

	char *start = szDestination;
	
	size_t esize = 1;
	size_t read_size = 0;
    do {   
        dwCount = fread( pbBuffer, esize , dwBlockLen, hSource );   
        if(ferror(hSource))  
        {  
            return 0;
        }  

		

        // 解密 数据  
        if( !CryptDecrypt( hKey, 0, feof(hSource), 0, pbBuffer, &dwCount ))  
        {  
			return 0; 
        }  

		memcpy(start,pbBuffer,dwCount);
		start += dwCount;
		
		read_size+=dwCount;
		if( read_size >= (size_t)size ) 
			break;
		

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
 * @加密原文szSource文件，加密后的数据存储在szDestination文件中   
 * @szSource：原文文件名  
 * @szDestination：加密后数据存储文件  
 * @szPassword：用户输入的密码 
 */
BOOL WingEncryptFile( PCHAR szSource, PCHAR szDestination,  PCHAR szPassword)  
{  

    FILE *hSource;   
    FILE *hDestination;   
  
    HCRYPTPROV hCryptProv;   
    HCRYPTKEY hKey;   
  
  
    PBYTE pbBuffer;   
    size_t dwBlockLen;   
    DWORD dwBufferLen;   
    DWORD dwCount;   
   
	fopen_s( &hSource,szSource,"rb" );
	fopen_s( &hDestination,szDestination,"wb" );

	size_t size = _filelength(_fileno(hSource));
  
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
  
	size_t read_size = 0;

    do   
    {      
        dwCount = fread(pbBuffer, 1, dwBlockLen, hSource);   
        if( ferror(hSource) )  
        {   
            return 0;
        }  

        if(!CryptEncrypt(  
            hKey,           //密钥  
            0,              //如果数据同时进行散列和加密，这里传入一个散列对象  
            feof(hSource),  //如果是最后一个被加密的块，输入TRUE.如果不是输入FALSE.  
                            //这里通过判断是否到文件尾来决定是否为最后一块。  
            0,              //保留  
            pbBuffer,       //输入被加密数据，输出加密后的数据  
            &dwCount,       //输入被加密数据实际长度，输出加密后数据长度  
            dwBufferLen))   //pbBuffer的大小。  
        {   
           return 0;
        }   
  
        // 把加密后数据写到密文文件中   
  
        fwrite(pbBuffer, 1, dwCount, hDestination);   
        if(ferror(hDestination))  
        {   
            return 0;
        }  

		read_size += dwCount;
		if( read_size >= size ) 
			break;
  
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
 
    if(hKey)  
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