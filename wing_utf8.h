#ifndef PHP_IOCP_UTF8_H
#define PHP_IOCP_UTF8_H
#include "Windows.h"
/**
 *@将gbk字符串转换为utf8字符串
 *@out_str无需初始化，api内部根据需要自动初始化
 *@注：使用完之后 记住需要 delete[] out_str;释放内存
 */
void iocp_gbk_to_utf8( _In_ char *in_str, _Out_ char *&out_str );//_Inout_
char* WcharToUtf8( const wchar_t *pwStr );
#endif
