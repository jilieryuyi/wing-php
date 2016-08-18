#ifndef __WING_STRING_H__
#define __WING_STRING_H__

#include "Windows.h"

void  WingTrim( char *str )  ; 
char* WingWcharToUtf8( const wchar_t *pwStr );
void  WingGBKToUTF8( char *in_str,char *&out_str );

#endif