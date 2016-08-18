/**
 *@字符串处理
 */
#include "wing_string.h"
#include "stdio.h"

WingString::WingString( const wchar_t *_str ){
	if( _str == NULL )
    {
		this->str = NULL;
		this->len = 0;
		return;
    }
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
}
WingString::WingString( const char *_str, int size ){
	this->str = _strdup( _str );
	this->len = size;
}

WingString::WingString( char *_str , int size , int dup ){
	if( dup )
	{	
		this->str = new char[size+1];
		memset( this->str, size+1 , 0 );
		memcpy( this->str , _str , size+1 );
	}
	else
	{
		this->str = (char*)_str;
	}
	this->len = size;
}

WingString::WingString(){
	this->str = NULL;
	this->len = 0;
}

WingString::~WingString(){
	if( this->str != NULL ) {
		delete[] this->str;
		this->str = NULL;
	}
	this->len = 0;
}

unsigned WingString::length(){
	return this->len;
}

char* WingString::c_str(){
	return this->str;
}


/**
 *@追加字符串
 */
void WingString::append( const wchar_t *_str ){
		
	if( _str == NULL )
    {
		return;
    }
    int len = WideCharToMultiByte( CP_UTF8, 0, _str, -1, NULL, 0, NULL, NULL );
    if (len <= 0)
    {
		return;
    }
    char *res = new char[len+1];
	memset( res, 0, len+1 );
    WideCharToMultiByte( CP_UTF8, 0, _str, -1, res, len, NULL, NULL );


	int new_len   = len + this->len + 1;
	char *new_str = new char[new_len];

	memset( new_str , 0 , new_len );

	char *str_begin = new_str;
	memcpy( str_begin , this->str , this->len );

	str_begin += this->len;
	memcpy( str_begin , res , len );
	delete[] this->str;
	delete[] res;

	this->str = new_str;
	this->len = new_len - 1;

}

/**
 *@追加字符串
 */
void WingString::append( const char *_str, int size ){
		
	int new_len   = size+this->len+1;
	char *new_str = new char[new_len];
	memset( new_str , 0 , new_len );

	char *str_begin = new_str;
	memcpy( str_begin , this->str , this->len );

	str_begin += this->len;
	memcpy( str_begin , _str , size );
	delete[] this->str;

	this->str = new_str;
	this->len = new_len - 1;

}


/**
 *@追加字符串
 */
void WingString::append( WingString *_str ){
		
	int new_len   = _str->length() + this->len+1;
	char *new_str = new char[new_len];
	memset( new_str , 0 , new_len );

	char *str_begin = new_str;
	memcpy( str_begin , this->str , this->len );

	str_begin += this->len;
	memcpy( str_begin , _str->c_str() , _str->length() );
	delete[] this->str;

	this->str = new_str;
	this->len = new_len - 1;

}

/**
 *@打印字符串
 */
void WingString::print(){
	printf("---len=%ld , %s---\r\n",this->len,this->str);
}

/**
 * @字符串转换为utf8编码
 */
BOOL WingString::toUTF8()
{
	if( this->str == NULL )
		return 0;

	wchar_t* unicode_str = NULL;
	int utf8_str_size    = 0;

	utf8_str_size      = ::MultiByteToWideChar( CP_ACP, 0,this->str, -1, NULL, NULL );                   //获取转换到Unicode编码后所需要的字符空间长度
	unicode_str        = new wchar_t[utf8_str_size + 1];                                                 //为Unicode字符串空间
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
	this->len = utf8_str_size;
	return 1;
}

void WingString::trim(){
	
	if( this->str == NULL || this->len <= 0 ) 
		return;
	
	int len     = this->len;  
	char *start = this->str;  
    char *end   = this->str + len - 1;  
  
	//找到第一个不为空的字符
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
  
	//找到最后一个不为空的字符
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
  
	//复制区间
    memmove(this->str, start, end - start + 1);  
	//最后一个值清零
    this->str[end - start + 1] = '\0';  
	this->len = end - start + 1;
}


