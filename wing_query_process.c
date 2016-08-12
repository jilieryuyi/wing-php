/**
 *@进程搜索、枚举
 */
#include "Windows.h"
#include "wing_ntdll.h"
#include "stdio.h"


NTSTATUS WingOpenProcess(
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ HANDLE ProcessId
)
{
    NTSTATUS          status;
    OBJECT_ATTRIBUTES objectAttributes;
    CLIENT_ID         clientId;

    clientId.UniqueProcess = ProcessId;
    clientId.UniqueThread  = NULL;

	HMODULE	hNtDll              = GetModuleHandleA("ntdll.dll");
	NTOPENPROCESS NtOpenProcess = (NTOPENPROCESS)GetProcAddress( hNtDll, "NtOpenProcess" );

	InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
    
	status = NtOpenProcess( ProcessHandle, DesiredAccess, &objectAttributes, &clientId );
    
    return status;
}


NTSTATUS WingQueryProcessVariableSize(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_ PVOID *Buffer
)
{
    NTSTATUS status;
    PVOID    buffer;
    ULONG    returnLength = 0;

	HMODULE	hNtDll = GetModuleHandleA("ntdll.dll");
	NTQUERYINFORMATIONPROCESS NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress( hNtDll, "NtQueryInformationProcess" );

    status = NtQueryInformationProcess( ProcessHandle, ProcessInformationClass, NULL, 0, &returnLength );

    if ( status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL && status != STATUS_INFO_LENGTH_MISMATCH )
		return status;

    buffer = ( LPVOID )malloc( returnLength );
	memset(buffer,0,returnLength);
    status = NtQueryInformationProcess( ProcessHandle, ProcessInformationClass, buffer, returnLength, &returnLength );

    if ( NT_SUCCESS( status ) )
    {
        *Buffer = buffer;
    }
    else
    {
        free(buffer);
    }
    return status;
}



	
	
extern char* WcharToUtf8(const wchar_t *pwStr);
extern void gbk_to_utf8( char *in_str,char *&out_str);

struct PROCESSINFO {
	char *process_name;
	char *command_line;
	char *file_name;
	char *file_path;
	int process_id;
	int parent_process_id;
	unsigned long working_set_size;
	unsigned long base_priority;//基本的优先级
	unsigned long thread_count ;
	unsigned long handle_count ;
	unsigned long cpu_time;
};

//#define WING_MAX_PROCESS_COUNT 1024
/**
 *@枚举进程 all_process 参数为null时 只返回进城数量
 */
DWORD WingQueryProcess( PROCESSINFO *&all_process , int max_count )
{
	PSYSTEM_PROCESSES			pSystemProc;
	HMODULE						hNtDll         = NULL;
	LPVOID						lpSystemInfo   = NULL;
	DWORD						dwNumberBytes  = MAX_INFO_BUF_LEN;
	DWORD						dwTotalProcess = 0;
	DWORD						dwReturnLength;
	NTSTATUS					Status; 
	LONGLONG					llTempTime;
	NTQUERYSYSTEMINFORMATION	NtQuerySystemInformation;
	__try
	{
		hNtDll =GetModuleHandleA("ntdll.dll");
		if(hNtDll == NULL)
		{
			__leave;
		}

		NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress( hNtDll, "NtQuerySystemInformation" );
		if(NtQuerySystemInformation == NULL)
		{
			__leave;
		}

		lpSystemInfo = (LPVOID)malloc(dwNumberBytes);
		Status = NtQuerySystemInformation( SystemProcessInformation, lpSystemInfo, dwNumberBytes, &dwReturnLength);
		if( Status == STATUS_INFO_LENGTH_MISMATCH )
		{
			__leave;
		}
		else if( Status != STATUS_SUCCESS )
		{
			__leave;
		}

		//printf("%-20s%6s%7s%8s%6s%7s%7s%13s\n","ProcessName","PID","PPID","WsSize","Prio.","Thread","Handle","CPU Time");
		//printf("--------------------------------------------------------------------------\n");
		pSystemProc = (PSYSTEM_PROCESSES)lpSystemInfo;

		HANDLE hProcess;

		//PROCESSINFO **all_process;// = new PROCESSINFO[WING_MAX_PROCESS_COUNT];

		while( pSystemProc->NextEntryDelta != 0 )
		{
			if( all_process == NULL ) {
				dwTotalProcess++;
				pSystemProc = (PSYSTEM_PROCESSES)((char *)pSystemProc + pSystemProc->NextEntryDelta);
				continue;
			}
			//count++;
			PROCESSINFO *process_item = &all_process[dwTotalProcess];
			// = *process_item;

			if( pSystemProc->ProcessId != 0 )
			{
				process_item->process_name = WcharToUtf8( pSystemProc->ProcessName.Buffer );
			}
			else
			{
				char *process_name = "System Idle Process\0";
				int len = strlen( process_name );
				process_item->process_name = new char[len+1];
				memset( process_item->process_name , 0 , len+1 );
				sprintf( process_item->process_name , "%s" , process_name );
			}


			WingOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,(HANDLE)pSystemProc->ProcessId );

			LPVOID commandline = NULL;

			if( NT_SUCCESS( WingQueryProcessVariableSize( hProcess, ProcessCommandLineInformation, (PVOID *)&commandline ) ) )
			{	
				process_item->command_line = WcharToUtf8( (const wchar_t*)((PUNICODE_STRING)commandline)->Buffer );
				free(commandline);
				commandline = NULL;
			}
			else
				process_item->command_line = NULL;





    PUNICODE_STRING fileName;
	if( NT_SUCCESS( WingQueryProcessVariableSize( hProcess, ProcessImageFileName, (PVOID*)&fileName ))){
		process_item->file_name = WcharToUtf8( (const wchar_t*)fileName->Buffer );
		free(fileName);
	}else{
		process_item->file_name = NULL;
	}

	//ProcessImageFileNameWin32
	 PUNICODE_STRING filepath;
	if( NT_SUCCESS( WingQueryProcessVariableSize( hProcess, ProcessImageFileNameWin32, (PVOID*)&filepath ))){
		process_item->file_path = WcharToUtf8( (const wchar_t*)filepath->Buffer );
		free(filepath);
	}else{
		process_item->file_path = NULL;
	}



			process_item->process_id        = pSystemProc->ProcessId;
			process_item->parent_process_id = pSystemProc->InheritedFromProcessId;
			//printf("%6d",pSystemProc->ProcessId);
			//printf("%7d",pSystemProc->InheritedFromProcessId);


			//printf("%7dK",pSystemProc->VmCounters.WorkingSetSize/1024);
			process_item->working_set_size = pSystemProc->VmCounters.WorkingSetSize;


			//printf("%6d",pSystemProc->BasePriority);
			process_item->base_priority = (unsigned long)pSystemProc->BasePriority;


			//printf("%7d",pSystemProc->ThreadCount);
			//printf("%7d",pSystemProc->HandleCount);
			process_item->thread_count = pSystemProc->ThreadCount;
			process_item->handle_count = pSystemProc->HandleCount;


			llTempTime  = pSystemProc->KernelTime.QuadPart + pSystemProc->UserTime.QuadPart;
			llTempTime /= 10000;
			llTempTime /= 1000; //精确到秒

			process_item->cpu_time = (unsigned long)llTempTime;

			/*printf("%3d:",llTempTime/(60*60*1000));
			llTempTime %= 60*60*1000;
			printf("%.2d:",llTempTime/(60*1000));
			llTempTime %= 60*1000;
			printf("%.2d.",llTempTime/1000);
			llTempTime %= 1000;
			printf("%.3d",llTempTime);*/

			//printf("\n");
			dwTotalProcess ++;
			if( dwTotalProcess > max_count ) break;
			pSystemProc = (PSYSTEM_PROCESSES)((char *)pSystemProc + pSystemProc->NextEntryDelta);
		}

		/*printf("--------------------------------------------------------------------------\n");
		printf("\nTotal %d Process(es) !\n\n",dwTotalProcess);
		printf("PID\t ==> Process Identification\n");
		printf("PPID\t ==> Parent Process Identification\n");
		printf("WsSize\t ==> Working Set Size\n");
		printf("Prio.\t ==> Base Priority\n");
		printf("Thread\t ==> Thread Count\n");
		printf("Handle\t ==> Handle Count\n");
		printf("CPU Time ==> Processor Time\n");*/
	}
	__finally
	{
		if(lpSystemInfo != NULL)
		{
			free(lpSystemInfo);
		}
		if(hNtDll != NULL)
		{
			FreeLibrary(hNtDll);
		}
	}

	return dwTotalProcess;
}



