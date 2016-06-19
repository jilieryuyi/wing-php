#include "wing_lib.h"



//-------------内存计数器-----------------------
unsigned long memory_add_times = 0;
unsigned long memory_sub_times = 0;


//CRITICAL_SECTION bytes_lock;
void memory_add(){
    InterlockedIncrement(&memory_add_times);
}
void memory_sub(){
    InterlockedIncrement(&memory_sub_times);
}

//-------------内存计数器-----------------------

/**************************************
 * @获取命令的绝对路径
 **************************************/
void get_command_path(const char *name,char *output){

	int env_len		= GetEnvironmentVariable("PATH",NULL,0)+1;
	char *env_var	= new char[env_len];
	ZeroMemory(env_var,sizeof(char)*(env_len));

	GetEnvironmentVariable("PATH",env_var,env_len);

	char *temp = NULL;
	char *start = NULL,*var_begin = NULL;

	start		= env_var;
	var_begin	= env_var;

	char _temp_path[MAX_PATH] = {0};

	while( temp = strchr(var_begin,';') ){
		
		long len_temp	= temp-start;
		long _len_temp	= len_temp+sizeof("\\")+sizeof(".exe")+1;
		
		ZeroMemory(output,sizeof(char)*MAX_PATH);

		strncpy_s(_temp_path,_len_temp,var_begin,len_temp);
		sprintf_s(output,MAX_PATH,"%s\\%s.exe\0",_temp_path,name);

		if(PathFileExists(output)){
			delete[] env_var;
			env_var = NULL;
			return;
		}

		ZeroMemory(output,sizeof(char)*MAX_PATH);
		sprintf_s(output,MAX_PATH,"%s\\%s.bat\0",_temp_path,name);

		if(PathFileExists(output)){
			delete[] env_var;
			env_var = NULL;
			return;
		}
		var_begin	= temp+1;
		start		= temp+1;
	}
	delete[] env_var;
	env_var = NULL;
}


/*********************************************
 * @生成随机字符串
 *********************************************/
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
