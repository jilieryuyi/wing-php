#ifndef __WING_BASE__
#define __WING_BASE__
#include "Windows.h"

void wing_get_command_path( const char *name ,char *&path );
void get_error_msg( char *&error_msg, int last_error = 0 );
void wing_call_func( zval **func TSRMLS_DC ,int params_count  ,zval **params );

#endif