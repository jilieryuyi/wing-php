#include "wing_utf8.h"

int GBKToUTF8(unsigned char * lpGBKStr,unsigned char * lpUTF8Str,int nUTF8StrLen)
{
	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;

	if(!lpGBKStr)  //如果GBK字符串为NULL则出错退出
	return 0;

	nRetLen = ::MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,NULL,NULL);  //获取转换到Unicode编码后所需要的字符空间长度
	lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
	nRetLen = ::MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,lpUnicodeStr,nRetLen);  //转换到Unicode编码
	if(!nRetLen)  //转换失败则出错退出
	return 0;

	nRetLen = ::WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,NULL,0,NULL,NULL);  //获取转换到UTF8编码后所需要的字符空间长度

	if(!lpUTF8Str)  //输出缓冲区为空则返回转换后需要的空间大小
	{
	if(lpUnicodeStr)
	delete []lpUnicodeStr;
	return nRetLen;
	}

	if(nUTF8StrLen < nRetLen)  //如果输出缓冲区长度不够则退出
	{
	if(lpUnicodeStr)
	delete []lpUnicodeStr;
	return 0;
	}

	nRetLen = ::WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,(char *)lpUTF8Str,nUTF8StrLen,NULL,NULL);  //转换到UTF8编码

	if(lpUnicodeStr)
	delete []lpUnicodeStr;

	return nRetLen;
	}


void iocp_gbk_to_utf8( char *in_str,char *&out_str){
	int len = GBKToUTF8((unsigned char *)in_str,NULL,NULL);

	out_str = new char[len + 1];
	memset(out_str,0,len + 1);
	len = GBKToUTF8((unsigned char *)in_str,(unsigned char *)out_str,len);
	if(!len)
	{
		delete[] out_str;
		out_str = NULL;
	}

}
