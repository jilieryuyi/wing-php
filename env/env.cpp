// env.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
void _get_command_path(const char *name,char *output){

	int len		= GetEnvironmentVariable("PATH",NULL,0)+1;
	char *var	= new char[len];
	GetEnvironmentVariable("PATH",var,len);

	char *temp;
	char *start,*var_begin;
	start		= var;
	var_begin	= var;
	char t[MAX_PATH];

	while(temp=strchr(var_begin,';')){
		long len_temp	= temp-start;
		long _len_temp	= len_temp+sizeof("\\")+sizeof(".exe")+1;
		
		memset(t,0,sizeof(t));
		
		strncpy_s(t,_len_temp,var_begin,len_temp);

	   // if(t[len-1]=='\\')
		//	t[len-1]='\0';
		//else 
		//	t[len]='\0';

		memset(output,0,sizeof(t));
		sprintf_s(output,MAX_PATH,"%s\\%s.exe\0",t,name);
		if(PathFileExists(output)){
			free(var);
		
			return;
		}

		memset(output,0,sizeof(t));
	sprintf_s(output,MAX_PATH,"%s\\%s.bat\0",t,name);

		if(PathFileExists(output)){
			free(var);
			
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}
	free(var);
	return;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char path[MAX_PATH];
	const char *name="php\0";
	_get_command_path("php\0",path);

	printf("%s\n",path);
	return 0;
}

