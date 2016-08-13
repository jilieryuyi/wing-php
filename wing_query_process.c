/**
 *@进程搜索、枚举
 */
#include "Windows.h"
#include "wing_ntdll.h"
#include "wing_query_process.h"
#include "stdio.h"

NTSTATUS WingOpenProcess(
	_In_ HMODULE	hNtDll,
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

	NTOPENPROCESS NtOpenProcess = (NTOPENPROCESS)GetProcAddress( hNtDll, "NtOpenProcess" );

	InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
    
	status = NtOpenProcess( ProcessHandle, DesiredAccess, &objectAttributes, &clientId );
    
    return status;
}


NTSTATUS WingQueryProcessVariableSize(
	_In_ HMODULE	hNtDll,
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_ PVOID *Buffer
)
{
    NTSTATUS status;
    PVOID    buffer;
    ULONG    returnLength = 0;


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


/**
 *@枚举进程 all_process 参数为null时 只返回进程数量
 */
unsigned long WingQueryProcess( PROCESSINFO *&all_process , int max_count )
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
		hNtDll = GetModuleHandleA("ntdll.dll");
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

		pSystemProc = (PSYSTEM_PROCESSES)lpSystemInfo;

		HANDLE hProcess;

		while( pSystemProc->NextEntryDelta != 0 )
		{
			if( all_process == NULL ) {
				dwTotalProcess++;
				pSystemProc = (PSYSTEM_PROCESSES)((char *)pSystemProc + pSystemProc->NextEntryDelta);
				continue;
			}

			PROCESSINFO *process_item = &all_process[dwTotalProcess];

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


			process_item->command_line = NULL;
			process_item->file_name    = NULL;
			process_item->file_path    = NULL;


			if( NT_SUCCESS(WingOpenProcess( hNtDll, &hProcess, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,(HANDLE)pSystemProc->ProcessId )))
			{
				
				LPVOID commandline = NULL;
				if( NT_SUCCESS( WingQueryProcessVariableSize( hNtDll, hProcess, ProcessCommandLineInformation, (PVOID *)&commandline ) ) )
				{	
					process_item->command_line = WcharToUtf8( (const wchar_t*)((PUNICODE_STRING)commandline)->Buffer );
					free(commandline);
					commandline = NULL;
				}
			
				PUNICODE_STRING fileName;
				if( NT_SUCCESS( WingQueryProcessVariableSize( hNtDll, hProcess, ProcessImageFileName, (PVOID*)&fileName ))){
					process_item->file_name = WcharToUtf8( (const wchar_t*)fileName->Buffer );
					free(fileName);
				}

			
				PUNICODE_STRING filepath;
				if( NT_SUCCESS( WingQueryProcessVariableSize( hNtDll, hProcess, ProcessImageFileNameWin32, (PVOID*)&filepath ))){
					process_item->file_path = WcharToUtf8( (const wchar_t*)filepath->Buffer );
					free(filepath);
				}
			}

			process_item->process_id        = pSystemProc->ProcessId;
			process_item->parent_process_id = pSystemProc->InheritedFromProcessId;
			process_item->working_set_size  = pSystemProc->VmCounters.WorkingSetSize;
			process_item->base_priority     = (unsigned long)pSystemProc->BasePriority;
			process_item->thread_count      = pSystemProc->ThreadCount;
			process_item->handle_count      = pSystemProc->HandleCount;


			llTempTime  = pSystemProc->KernelTime.QuadPart + pSystemProc->UserTime.QuadPart;
			llTempTime /= 10000;
			llTempTime /= 1000; //精确到秒

			process_item->cpu_time = (unsigned long)llTempTime;

			dwTotalProcess ++;
			if( dwTotalProcess > max_count ) break;
			pSystemProc = (PSYSTEM_PROCESSES)((char *)pSystemProc + pSystemProc->NextEntryDelta);

			
		}
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