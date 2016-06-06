/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-14
 ******************************/
#include "Windows.h"
#include "library.h"
#include "memory.h"
//获取命令的绝对路径
void get_command_path(const char *name,char *output){

	int len		= GetEnvironmentVariable("PATH",NULL,0)+1;
	char *var	= new char[len];
	memory_add("add memory get_command_path 13\r\n");
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
			free(var);memory_sub("sub memory get_command_path 32\r\n");
			return;
		}

		memset(output,0,sizeof(t));
		sprintf_s(output,MAX_PATH,"%s\\%s.bat\0",t,name);

		if(PathFileExists(output)){
			free(var);memory_sub("sub memory get_command_path 40\r\n");
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}
	free(var);memory_sub("sub memory get_command_path 46\r\n");
	return;
}

const char* create_guid()  
{  
	 CoInitialize(NULL);  
	 static char buf[64] = {0};  
	 GUID guid;  
	 if (S_OK == ::CoCreateGuid(&guid))  
	 {  
	  _snprintf(buf, sizeof(buf), "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"  , 
		guid.Data1  , 
		guid.Data2  , 
		guid.Data3   , 
		guid.Data4[0], 
		guid.Data4[1]  , 
		guid.Data4[2], 
		guid.Data4[3],
		guid.Data4[4], 
		guid.Data4[5]  , 
		guid.Data4[6], 
		guid.Data4[7]  
	   );  
	 }  
	 CoUninitialize();  
	 return (const char*)buf;  
}  
