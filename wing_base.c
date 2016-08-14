#include "wing_base.h"
#include "stdio.h"
#include "Shlwapi.h"
#pragma comment(lib,"Shlwapi.lib")


void wing_get_command_path( const char *name ,char *&path ){

	path = (char*)malloc(MAX_PATH);
	
	int   size      = GetEnvironmentVariableA("PATH",NULL,0);
	char *env_var	= (char*)malloc(size);
	char *temp      = NULL;
	char *start     = NULL;
	char *var_begin = NULL;
	
	ZeroMemory( env_var,size );

	GetEnvironmentVariableA("PATH",env_var,size);

	start		= env_var;
	var_begin	= env_var;

	char _temp_path[MAX_PATH] = {0};

	while( temp = strchr(var_begin,';') ) {
		
		long len_temp	= temp-start;
		long _len_temp	= len_temp+sizeof("\\")+sizeof(".exe")+1;
		
		ZeroMemory( path, MAX_PATH );

		strncpy_s( _temp_path, _len_temp,var_begin, len_temp );
		sprintf_s( path, MAX_PATH, "%s\\%s.exe\0", _temp_path, name );

		if( PathFileExists( path ) ) {
			free( env_var );
			env_var = NULL;
			return;
		}

		ZeroMemory( path , MAX_PATH );
		sprintf_s( path, MAX_PATH , "%s\\%s.bat\0", _temp_path, name );

		if( PathFileExists( path ) ) {
			free( env_var );
			env_var = NULL;
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}

	free( env_var );
	env_var = NULL;

	sprintf_s( path, MAX_PATH, "" );
	return;
}
