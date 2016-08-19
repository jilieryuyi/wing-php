// WingString.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Windows.h"

#define WING_STR_IS_CHAR   1
#define WING_STR_IS_WCHAR  2
#define WING_STR_IS_UNKNOW 3

class WingString{

private:
	void *str;
	unsigned int str_size;
	unsigned int str_type;

public:

	//构造函数
	WingString( char *_str, int _size );
	WingString( wchar_t *_str, int _size );
	WingString( );
	~WingString( );

	unsigned int size();
	unsigned int length();

	char* c_str();
	wchar_t* w_str();

	void append( const char *_str, int size );
	void append( WingString *_str );
	void append( const wchar_t *_str,int size );

	BOOL toUTF8( );
	void print();
	void trim();
};

/**
to char
int len = WideCharToMultiByte(CP_UTF8, 0, _str, -1, NULL, 0, NULL, NULL);
    if (len <= 0)
    {
        this->str = NULL;
		this->len = 0;
		return;
    }
    this->str = new char[len+1];
	this->len = len;
	memset( this->str, 0, len+1 );
    WideCharToMultiByte( CP_UTF8, 0, _str, -1, this->str, len, NULL, NULL );
*/

WingString::WingString( char *_str, int _size ){
	
	this->str      = malloc( _size );
	this->str_size = _size;
	this->str_type = WING_STR_IS_CHAR;

	memset( this->str, 0, _size );
	memcpy( this->str, _str, _size ); 
}

WingString::WingString( wchar_t *_str, int _size ){
	
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
	switch( this->str_type ){
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

char* WingString::c_str(){
	return (char*)this->str;
}
wchar_t* WingString::w_str(){
	return (wchar_t*)this->str;
}


/**
 *@追加字符串
 */
void WingString::append( const wchar_t *_str, int size ){
		
	if( _str == NULL || size <= 0 )
    {
		return;
    }
	if( this->str_type == WING_STR_IS_UNKNOW )
	{
		this->str_type = WING_STR_IS_WCHAR;
		this->str      = malloc( size );
		this->str_size = size;

		memcpy( this->str, _str, size );
		return;
	}

	if( this->str_type == WING_STR_IS_CHAR ){
		
		//这里得到容纳目标字符串的长度空间 包含 \0结束符
		int len = WideCharToMultiByte( CP_UTF8, 0, _str, -1, NULL, 0, NULL, NULL );
		if (len <= 0)
		{
			return;
		}

		char *res = (char*)malloc( len );
		memset( res, 0, len );

		WideCharToMultiByte( CP_UTF8, 0, _str, -1, res, len, NULL, NULL );

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

		wsprintf( res, L"%s%s", this->str, _str );
	
		free( this->str );

		this->str      = res;
		this->str_size = new_size;
	}

}

/**
 *@追加字符串
 */
void WingString::append( const char *_str, int size ){
		
	if( _str == NULL || size <= 0 )
    {
		return;
    }
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

		int len      = MultiByteToWideChar(CP_ACP,0,(const char *)_str,size-1,NULL,0);
		int new_size = len*sizeof(wchar_t) + this->str_size;

		wchar_t* buffer = (wchar_t*)malloc(new_size);
		memset( buffer, 0x0, new_size );

		int buf_size = (len+1)*sizeof(wchar_t);
		wchar_t* buf = (wchar_t*)malloc( buf_size );
		memset( buf, 0x0, buf_size );
		MultiByteToWideChar( CP_ACP,0,(const char *)_str,size-1,buf,len);   

		wsprintf( buffer, L"%s%s", this->str, buf );
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
		
	/*int new_len   = _str->length() + this->len+1;
	char *new_str = new char[new_len];
	memset( new_str , 0 , new_len );

	char *str_begin = new_str;
	memcpy( str_begin , this->str , this->len );

	str_begin += this->len;
	memcpy( str_begin , _str->c_str() , _str->length() );
	delete[] this->str;

	this->str = new_str;
	this->len = new_len - 1;*/

}

/**
 *@打印字符串
 */
void WingString::print(){

	if( this->str_type == WING_STR_IS_CHAR )
		printf("---size=%ld,len=%ld,%s---\r\n",this->size(),this->length(),this->str);
	else if( this->str_type == WING_STR_IS_WCHAR )
		wprintf_s(L"---size=%ld,len=%ld,%s---\r\n",this->size(),this->length(),this->str);
}

/**
 * @字符串转换为utf8编码
 */
BOOL WingString::toUTF8()
{
	/*if( this->str == NULL )
		return 0;

	wchar_t* unicode_str = NULL;
	int utf8_str_size    = 0;

	utf8_str_size      = ::MultiByteToWideChar( CP_ACP, 0,this->str, -1, NULL, NULL );                   //获取转换到Unicode编码后所需要的字符空间长度
	unicode_str        = new wchar_t[utf8_str_size + 1];                                                   //为Unicode字符串空间
	utf8_str_size      = ::MultiByteToWideChar( CP_ACP, 0, this->str, -1, unicode_str, utf8_str_size );  //转换到Unicode编码
	
	if( !utf8_str_size )                                                                                 //转换失败则出错退出
	{
		if( unicode_str ) 
			delete[] unicode_str;
		return 0;
	}

	utf8_str_size  = WideCharToMultiByte(CP_UTF8,0,unicode_str,-1,NULL,0,NULL,NULL);                    //获取转换到UTF8编码后所需要的字符空间长度
	char *utf8_str = new char[utf8_str_size+1];

	memset(utf8_str,0,utf8_str_size+1);

	utf8_str_size = WideCharToMultiByte( CP_UTF8, 0, unicode_str, -1, (char *)utf8_str, utf8_str_size+1, NULL, NULL );  
	                                                                                                    //转换到UTF8编码
	if( unicode_str )
		delete []unicode_str;

	if( !utf8_str_size )
		return 0;

	delete[] this->str;

	this->str = utf8_str;
	this->len = utf8_str_size;*/
	return 1;
}

void WingString::trim(){
	
	/*if( this->str == NULL || this->len <= 0 ) 
		return;
	
	int len     = this->len;  
	char *start = this->str;  
    char *end   = this->str + len - 1;  
  
    while (1)   
    {     
        char c = *start;  
        if (!isspace(c))  
            break;  
  
        start++;  
        if (start > end)  
        {     
            this->str[0] = '\0';  
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
            this->str[0] = '\0';  
            return;  
        }  
    }  
  
    memmove(this->str, start, end - start + 1);  
    this->str[end - start + 1] = '\0';  
	this->len = end - start + 1;*/
}



int _tmain(int argc, _TCHAR* argv[])
{
	//WingString a("123",sizeof("123"));
	//a.append(L"456",sizeof(L"456"));
	//a.print();

	//WingString a2(L"123",sizeof(L"123"));
	//a2.append(L"456",sizeof(L"456"));
	//a2.print();

	WingString a(L"123",sizeof(L"123"));
	a.print();
	a.append("456",sizeof("456"));
	a.print();

	printf("%ld\r\n",sizeof(L"123456"));

	return 0;
}

