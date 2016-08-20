// WingString.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Windows.h"
#include <locale.h>
#define WING_STR_IS_CHAR   1
#define WING_STR_IS_WCHAR  2
#define WING_STR_IS_UNKNOW 3

#define WING_CHAR_SIZE(str)  (strlen((char*)str)+1)*sizeof(char)
#define WING_WCHAR_SIZE(str) (wcslen((wchar_t*)str)+1)*sizeof(wchar_t)

/**
 *@size为字符串长度,或者sizeof 都可以 ，已经兼容并且二进制安全
 */
void      wing_str_trim( _Inout_ char* str ,int size = 0 );
char*     wing_str_wchar_to_char( _In_ const wchar_t* str );
wchar_t*  wing_str_char_to_wchar( _In_ const char* str );
char*     wing_str_char_to_utf8( _In_ const char* str );
char*     wing_str_wchar_to_utf8( _In_ const wchar_t* str );

/**
 *@---- WingString ----
 *@字符串处理封装 
 *@注意 size 均为占用内存字节 并非字符串长度，如 size=sizeof(*data) 或者 size = (strlen(str)+1)*sizeof(char)
 */
class WingString{

private:
	void *str;
	unsigned int str_size;
	unsigned int str_type;

public:

	//构造函数
	WingString( char *_str, int _size = 0 );
	WingString( wchar_t *_str, int _size = 0 );
	WingString( );
	~WingString( );

	unsigned int size();
	unsigned int length();

	void* copy();
	void* data();
	int   type();
	char* c_str();
	wchar_t* w_str();

	void append( const char *_str, int size = 0 );
	void append( WingString *_str );
	void append( const wchar_t *_str,int size = 0 );

	BOOL toUTF8( );
	void print();
	void trim();
};

WingString::WingString( char *_str, int _size ){
	if( _size <= 0 ) 
		_size = WING_CHAR_SIZE( _str );
	this->str      = malloc( _size );
	this->str_size = _size;
	this->str_type = WING_STR_IS_CHAR;

	memset( this->str, 0, _size );
	memcpy( this->str, _str, _size ); 
}

WingString::WingString( wchar_t *_str, int _size ){
	if( _size <= 0 ) 
		_size = WING_WCHAR_SIZE( _str );
	this->str      = malloc( _size );
	this->str_size = _size;
	this->str_type = WING_STR_IS_WCHAR;

	memset( this->str, 0x0, _size );
	memcpy( this->str, _str, _size ); 
}

WingString::WingString(){
	this->str      = NULL;
	this->str_size = 0;
	this->str_type = WING_STR_IS_UNKNOW;
}

WingString::~WingString(){
	if( this->str != NULL ) {
		free( this->str );
		this->str = NULL;
	}
	this->str_size = 0;
	this->str_type = WING_STR_IS_UNKNOW;
}

unsigned int WingString::size(){
	return this->str_size;
}
unsigned int WingString::length(){
	
	switch( this->str_type )
	{
		case WING_STR_IS_CHAR:
			return (unsigned int)(  this->str_size/sizeof(char)  -1 );
			break;
		case WING_STR_IS_WCHAR:
			return (unsigned int)(  this->str_size/sizeof(wchar_t) -1 );
			break;
		default:
			return 0;
	}
	return 0;
}




/**
 *@追加字符串
 */
void WingString::append( const wchar_t *_str, int size ){
		
	if( _str == NULL )
    {
		return;
    }

	if( size <=0 ) 
		size = WING_WCHAR_SIZE( _str );

	if( this->str_type == WING_STR_IS_UNKNOW )
	{
		this->str_type = WING_STR_IS_WCHAR;
		this->str      = malloc( size );
		this->str_size = size;

		memcpy( this->str, _str, size );
		return;
	}

	if( this->str_type == WING_STR_IS_CHAR ){
		
		char *res = wing_str_wchar_to_char( (const wchar_t*)_str );

		int len       = WING_CHAR_SIZE( res );
		int new_len   = this->str_size + len - 1 ;

		char *new_str = (char*)malloc(new_len);

		memset( new_str , 0 , new_len );

		char *str_begin = new_str;
		memcpy( str_begin , this->str , this->str_size - 1 );

		str_begin += (this->str_size - 1);
		memcpy( str_begin , res , len );

		free( this->str );
		free( res );

		this->str      = new_str;
		this->str_size = new_len;
		return;
	}

	if( this->str_type == WING_STR_IS_WCHAR ) {
	    
		int wl       = sizeof(wchar_t);
		int new_size = this->str_size + size - wl;
		
		wchar_t* res = (wchar_t*)malloc( new_size );

		memset( res, 0x0, new_size );

		wsprintfW( res, L"%s%s", this->str, _str );
	
		free( this->str );

		this->str      = res;
		this->str_size = new_size;
		return;
	}

}

/**
 *@追加字符串
 */
void WingString::append( const char *_str, int size ){
		
	if( _str == NULL )
    {
		return;
    }

	if( size <=0 ) 
		size = WING_CHAR_SIZE( _str );

	//如果构造的时候 没有初始化
	if( this->str_type == WING_STR_IS_UNKNOW )
	{
		this->str_type = WING_STR_IS_CHAR;
		this->str      = malloc( size );
		this->str_size = size;

		memcpy( this->str, _str, size );
		return;
	}

	if( this->str_type == WING_STR_IS_CHAR ){
		
		int new_size = this->str_size - 1 + size;
		char *res = (char*)malloc( new_size );
		memset( res, 0, new_size );

		char *str_start = res;
		memcpy( str_start, this->str, this->str_size - 1 );
		str_start +=  this->str_size - 1;

		memcpy( str_start, _str, size );

		free( this->str );

		this->str      = res;
		this->str_size = new_size;

		return;
	}

	if( this->str_type == WING_STR_IS_WCHAR ) {

		wchar_t* buf = wing_str_char_to_wchar( (const char *)_str );
		int new_size = WING_WCHAR_SIZE( buf ) - sizeof(wchar_t) + this->str_size;

		wchar_t* buffer = (wchar_t*)malloc(new_size);
		memset( buffer, 0x0, new_size );

		wsprintfW( buffer, L"%s%s", this->str, buf );
		free( this->str );
		free( buf );

		this->str      = buffer;
		this->str_size = new_size;
	}

}


/**
 *@追加字符串
 */
void WingString::append( WingString *_str ){
		
	if( this->str_type == WING_STR_IS_UNKNOW ) {

		int size       = _str->size();
		this->str      = malloc( size );
		this->str_size = size;
		this->str_type = _str->type();

		memset( this->str, 0 , size );
		memcpy( this->str, _str->data(), size );
		return;
	}

	else if( this->str_type == WING_STR_IS_CHAR ){

		if( _str->type() == WING_STR_IS_UNKNOW )
			return;

		else if( _str->type() == WING_STR_IS_CHAR ){
			
			int new_size = this->str_size - 1 + _str->size();
			char *res = (char*)malloc( new_size );
			memset( res, 0, new_size );

			char *str_start = res;
			memcpy( str_start, this->str, this->str_size - 1 );
			str_start +=  this->str_size - 1;

			memcpy( str_start, _str->data(), _str->size() );

			free( this->str );

			this->str      = res;
			this->str_size = new_size;

			return;
		}
		else if( _str->type() == WING_STR_IS_WCHAR ){

			char *res = wing_str_wchar_to_char( (const wchar_t*)_str->data() );

			int len       = WING_CHAR_SIZE( res );
			int new_len   = this->str_size + len - 1 ;

			char *new_str = (char*)malloc(new_len);

			memset( new_str , 0 , new_len );

			char *str_begin = new_str;
			memcpy( str_begin , this->str , this->str_size - 1 );

			str_begin += (this->str_size - 1);
			memcpy( str_begin , res , len );

			free( this->str );
			free( res );

			this->str      = new_str;
			this->str_size = new_len;
			return;

		}
		return;
	}

	else if( this->str_type == WING_STR_IS_WCHAR ){
		
		if( _str->type() == WING_STR_IS_UNKNOW )
			return;

		else if( _str->type() == WING_STR_IS_WCHAR ) {
			
			int wl       = sizeof(wchar_t);
			int new_size = this->str_size + _str->size() - wl;
		
			wchar_t* res = (wchar_t*)malloc( new_size );

			memset( res, 0x0, new_size );

			wsprintfW( res, L"%s%s", this->str, _str->data() );
	
			free( this->str );

			this->str      = res;
			this->str_size = new_size;
			return;
		}

		else if( _str->type() == WING_STR_IS_CHAR ) {
			
			wchar_t* buf = wing_str_char_to_wchar( (const char *)_str->data() );
			int new_size = WING_WCHAR_SIZE( buf ) - sizeof(wchar_t) + this->str_size;

			wchar_t* buffer = (wchar_t*)malloc(new_size);
			memset( buffer, 0x0, new_size );

			wsprintfW( buffer, L"%s%s", this->str, buf );
			free( this->str );
			free( buf );

			this->str      = buffer;
			this->str_size = new_size;
			return;

		}
		return;
	}


}
void * WingString::data(){
	return this->str;
}
int WingString::type(){
	return this->str_type;
}

void* WingString::copy(){
	if( str == NULL || str_size <= 0 )
		return NULL;
	void* res = malloc(str_size);
	memset(res,0,str_size);
	memcpy(res,str,str_size);
	return res;
}

/**
 *@此处返回char* 需要使用free释放，并且不改变自身的值
 */
char* WingString::c_str(){

	char* res = NULL;

	switch( this->str_type ) {
	
	case WING_STR_IS_CHAR:
		{
			res = (char*)this->copy();
		}
		break;
	case WING_STR_IS_WCHAR:
		{
			res = wing_str_wchar_to_char( (const wchar_t*)str );	   
		}
		break;
	}

	return res;
}
/**
 *@此处返回wchar_t* 需要使用free释放，并且不改变自身的值
 */
wchar_t* WingString::w_str(){

	wchar_t* res = NULL;

	switch( this->str_type ) {
	
	case WING_STR_IS_CHAR:
		{
			res = wing_str_char_to_wchar( (const char*)this->str );
		}
		break;
	case WING_STR_IS_WCHAR:
		{
			res = (wchar_t*)this->copy(); 
		}
		break;
	}

	return res;
}
/**
 *@打印字符串
 */
void WingString::print(){
	 setlocale(LC_ALL, "chs");
	if( this->str_type == WING_STR_IS_CHAR )
		printf("---char:size=%ld,len=%ld,%s---\r\n",this->size(),this->length(),this->str);
	else if( this->str_type == WING_STR_IS_WCHAR )
		wprintf(L"---wchar_t:size=%ld,len=%ld,%s---\r\n",this->size(),this->length(),this->str);
}

/**
 * @字符串转换为utf8编码，会改变自身
 */
BOOL WingString::toUTF8()
{
	char *utf8_str = NULL;
	switch( this->str_type ){
	case WING_STR_IS_CHAR:
		{
			utf8_str = wing_str_char_to_utf8( ( const char* )str );
		}
		break;
	case WING_STR_IS_UNKNOW:
		return 1;
		break;
	case WING_STR_IS_WCHAR:
		{
			utf8_str = wing_str_wchar_to_utf8( (const wchar_t*)this->str );
		}
		break;
	}

	if( utf8_str != NULL ){
		free( str );
		this->str      = utf8_str;
		this->str_size = WING_CHAR_SIZE( utf8_str );
		this->str_type = WING_STR_IS_CHAR;
		return 1;
	}else
		return 0;
}

/**
 *@去除两端空格 ，会改变自身
 */
void WingString::trim(){
	
	if( this->str == NULL || this->str_size <= 0 ) 
		return;
	if( this->str_type == WING_STR_IS_CHAR )
	{
		wing_str_trim( (char*)this->str );
		this->str_size = strlen((char*)this->str )+1;
	}
}
//----WingString end------------------------



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

int _tmain(int argc, _TCHAR* argv[])
{
	WingString a("123");
	a.append(L"456");
	a.print();

	//WingString a2(L"123");
	//a2.append(L"456");
	//a2.print();

	WingString a3(L"123");
	a3.append("456");
	a3.print();


	//char s[] = " 999   ";
	//wing_str_trim( s, strlen(s)+1 );
	//printf("==>%s<==\r\n",s);

	//WingString a5("哈哈哈");
	//a5.print();
	//a5.toUTF8();
	//a5.print();

	//WingString a6(" 999   ");
	//a6.print();
	//a6.trim();
	//a6.print();

	WingString a7;
	WingString a8(L"456");
	a7.append(&a8);
	a7.print();


	WingString a9("哈哈哈");
	//a9.print();
	//a9.toUTF8();
	//a9.print();
	wchar_t *res = a9.w_str();
	wprintf(res);
	free(res);

	WingString a91(L"哈哈哈");
	//a91.print();
	//a91.toUTF8();
	//a91.print();

	wchar_t *res1 = a91.w_str();
	wprintf(res1);
	free(res1);


	return 0;
}



