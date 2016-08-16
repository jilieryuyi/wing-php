// 解密文件测试
//

#include "stdafx.h"

#include <stdio.h>  
#include <string.h>  
#include <conio.h>  
#include <windows.h>  
#include <wincrypt.h>  
#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)  
#define KEYLENGTH  0x00800000  
void HandleError(char *s);  
HCRYPTPROV GetCryptProv();  
  
#define ENCRYPT_ALGORITHM CALG_RC4   
#define ENCRYPT_BLOCK_SIZE 8   
   
BOOL DecryptFile(  
     PCHAR szSource,   
     PCHAR szDestination,   
     CHAR *szPassword);   
  
HCRYPTKEY GenKeyByPassword(HCRYPTPROV hCryptProv,PCHAR szPassword);  
HCRYPTKEY GenKeyFromFile(HCRYPTPROV hCryptProv,FILE* hSource);  
  
void main(void)   
{   
    //--------------------------------------------------------------------  
    // 变量申明与初始化.  
  
    PCHAR szSource = "D:/2.txt";   
    PCHAR szDestination = "D:/3.txt";   
    PCHAR szPassword = "123456";   
    char  response;  
  
   
    //解密文件  
    if(!DecryptFile(szSource, szDestination, szPassword))  
    {  
        printf("\nError decrypting file. \n");   
    }  
    else  
    {   
        printf("\n对文件 %s 的解密成功了. \n", szSource);  
        printf("被解密的文件是 %s .\n",szDestination);  
    }  
  
} // End main  
  
//-------------------------------------------------------------------  
// 功能：解密密文szSource文件，解密后的数据存储在szDestination文件中  
// 参数:  
//  szSource：密文文件名  
//  szDestination：解密后数据存储文件  
//  szPassword：用户输入的密码  
  
static BOOL DecryptFile(  
     PCHAR szSource,   
     PCHAR szDestination,   
     PCHAR szPassword)   
{   
    //--------------------------------------------------------------------  
    // 局部变量申明与初始化.  
  
    FILE *hSource;   
    FILE *hDestination;   
  
    HCRYPTPROV hCryptProv;   
    HCRYPTKEY hKey;   
  
    PBYTE pbBuffer;   
    DWORD dwBlockLen;   
    DWORD dwBufferLen;   
    DWORD dwCount;   
  
    BOOL status = FALSE;   
   
    //--------------------------------------------------------------------  
    // 打开密文文件.   
    fopen_s(&hSource,szSource,"rb");   
   
    //--------------------------------------------------------------------  
    // 打开目标文件，用于存储解密后的数据.   
  
   fopen_s(&hDestination,szDestination,"wb");
    
    //获取加密服务者句柄  
    hCryptProv = GetCryptProv();  
  
    //获取或创建会话密钥  
    if(!szPassword|| strcmp(szPassword,"")==0 )   
    {   
        //--------------------------------------------------------------------  
        //从密文文件导入保存的会话密钥   
  
        hKey = GenKeyFromFile( hCryptProv,hSource);  
          
    }   
    else   
    {   
        //--------------------------------------------------------------------  
        // 通过输入密码重新创建会话密钥.   
   
        hKey=GenKeyByPassword( hCryptProv, szPassword);  
    }   
   
  
    // 计算一次解密的数据长度，它是ENCRYPT_BLOCK_SIZE 的整数倍  
  
    dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;   
    dwBufferLen = dwBlockLen;   
  
    //--------------------------------------------------------------------  
    // 分配内存空间.   
  
    if(!(pbBuffer = (BYTE *)malloc(dwBufferLen)))  
    {  
       HandleError("所需内存不够!\n");   
    }  
    //--------------------------------------------------------------------  
    // 解密密文文件，解密后数据保存在目标文件   
  
    do {   
        //--------------------------------------------------------------------  
        // 每次从密文文件中读取dwBlockLen字节数据.   
  
        dwCount = fread(  
             pbBuffer,   
             1,   
             dwBlockLen,   
             hSource);   
        if(ferror(hSource))  
        {  
            HandleError("读取密文文件出错!");  
        }  
        //--------------------------------------------------------------------  
        // 解密 数据  
        if(!CryptDecrypt(  
              hKey,   
              0,   
              feof(hSource),   
              0,   
              pbBuffer,   
              &dwCount))  
        {  
           HandleError("Error during CryptDecrypt!");   
        }  
        //--------------------------------------------------------------------  
        // 把解密后的数据写入目标文件中.   
  
        fwrite(  
            pbBuffer,   
            1,   
            dwCount,   
            hDestination);   
        if(ferror(hDestination))  
        {  
           HandleError("Error writing plaintext!");   
        }  
    }   while(!feof(hSource));   
  
    status = TRUE;   
  
    //--------------------------------------------------------------------  
    // 关闭文件  
    if(hSource)  
    {  
        if(fclose(hSource))  
            HandleError("关闭原文件出错");  
    }  
    if(hDestination)  
    {  
        if(fclose(hDestination))  
            HandleError("关闭目标文件出错");  
    }   
   
    //--------------------------------------------------------------------  
    // 释放内存空间   
  
    if(pbBuffer)   
         free(pbBuffer);   
   
    //--------------------------------------------------------------------  
    // 销毁会话密钥  
  
    if(hKey)  
    {  
        if(!(CryptDestroyKey(hKey)))  
            HandleError("Error during CryptDestroyKey");  
    }   
  
    //--------------------------------------------------------------------  
    // 释放CSP句柄  
    if(hCryptProv)  
    {  
        if(!(CryptReleaseContext(hCryptProv, 0)))  
            HandleError("Error during CryptReleaseContext");  
    }   
  
    return status;  
} // end Decryptfile  
  
  
//获取加密提供者句柄  
HCRYPTPROV GetCryptProv()  
{  
    HCRYPTPROV hCryptProv;                      // 加密服务提供者句柄  
      
    //获取加密提供者句柄  
    if(CryptAcquireContext(  
                &hCryptProv,         // 加密服务提供者句柄  
                NULL,                // 密钥容器名,这里使用登陆用户名  
                MS_ENHANCED_PROV,         // 加密服务提供者       
                PROV_RSA_FULL,       // 加密服务提供者类型,可以提供加密和签名等功能  
                0))                  // 标志  
    {  
        printf("加密服务提供者句柄获取成功!\n");  
    }  
    else  
    {  
          
        //重新建立一个新的密钥集  
        if(!CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))  
        {  
           HandleError("重新建立一个新的密钥集出错!");  
        }  
   
    }  
    return hCryptProv;  
}  
  
  
//  HandleError：错误处理函数，打印错误信息，并退出程序  
void HandleError(char *s)  
{  
    printf("程序执行发生错误!\n");  
    printf("%s\n",s);  
    printf("错误代码为: %x.\n",GetLastError());  
    printf("程序终止执行!\n");  
    exit(1);  
}  
  
// GenKeyFromFile：从密文文件中导出会话密钥  
// 参数：hCryptProv CSP句柄  
//       hSource   保存会话密钥的文件  
HCRYPTKEY GenKeyFromFile(HCRYPTPROV hCryptProv,FILE* hSource)  
{  
    HCRYPTKEY hKey;   
  
    PBYTE pbKeyBlob;   
    DWORD dwKeyBlobLen;   
  
    //从密文文件中获取密钥数据块长度，并分配内存空间.   
    fread(&dwKeyBlobLen, sizeof(DWORD), 1, hSource);   
    if(ferror(hSource) || feof(hSource))  
    {  
        HandleError("读取密文文件中密钥数据块长度出错!");   
    }  
    if(!(pbKeyBlob = (BYTE *)malloc(dwKeyBlobLen)))  
    {  
        HandleError("内存分配出错.");   
    }  
    //--------------------------------------------------------------------  
    // 从密文文件中获取密钥数据块  
  
    fread(pbKeyBlob, 1, dwKeyBlobLen, hSource);   
    if(ferror(hSource) || feof(hSource))  
    {  
        HandleError("读取密文文件中密钥数据块出错!\n");   
    }  
    //--------------------------------------------------------------------  
    // 导入会话密钥到 CSP.   
    if(!CryptImportKey(  
          hCryptProv,   
          pbKeyBlob,   
          dwKeyBlobLen,   
          0,   
          0,   
          &hKey))  
    {  
       HandleError("Error during CryptImportKey!");   
    }  
  
    if(pbKeyBlob)   
        free(pbKeyBlob);  
      
    //返回导出的会话密钥  
    return hKey;  
}  
  
// GenKeyByPassword：通过输入密码创建会话密钥  
// 参数：hCryptProv CSP句柄  
//       szPassword 输入密码  
HCRYPTKEY GenKeyByPassword(HCRYPTPROV hCryptProv,PCHAR szPassword)  
{  
    HCRYPTKEY hKey;   
    HCRYPTHASH hHash;  
    //-------------------------------------------------------------------  
    // 创建哈希句柄.    
  
    if(CryptCreateHash(  
           hCryptProv,   
           CALG_MD5,   
           0,   
           0,   
           &hHash))  
        {  
            printf("一个哈希句柄已经被创建. \n");  
        }  
        else  
        {   
             HandleError("Error during CryptCreateHash!\n");  
        }    
    //-------------------------------------------------------------------  
    // 计算输入密码的哈希值.   
  
    if(CryptHashData(  
           hHash,   
           (BYTE *)szPassword,   
           strlen(szPassword),   
           0))  
     {  
        printf("此密码已经被添加到了哈希表中. \n");  
     }  
     else  
     {  
        HandleError("Error during CryptHashData. \n");   
     }  
    //-------------------------------------------------------------------  
    // 通过哈希值创建会话密钥.  
  
    if(CryptDeriveKey(  
           hCryptProv,   
           ENCRYPT_ALGORITHM,   
           hHash,   
           KEYLENGTH,   
           &hKey))  
     {  
       printf("从这个密码的哈希值获得了一个加密密钥. \n");   
     }  
     else  
     {  
       HandleError("Error during CryptDeriveKey!\n");   
     }  
    //-------------------------------------------------------------------  
    // 销毁哈希句柄.   
  
    if(hHash)   
    {  
        if(!(CryptDestroyHash(hHash)))  
           HandleError("Error during CryptDestroyHash");   
        hHash = 0;  
    }  
          
    //返回创建的会话密钥  
    return hKey;  
}  