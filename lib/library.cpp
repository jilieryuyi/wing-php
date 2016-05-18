/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-14
 ******************************/
#include "Windows.h"
#include "library.h"
//获取命令的绝对路径
void get_command_path(const char *name,char *output){

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