/**
 *@½ø³ÌËÑË÷¡¢Ã¶¾Ù
 */
#include "Windows.h"
#include "wing_ntdll.h"
#include "stdio.h"

#define InitializeObjectAttributes(p, n, a, r, s) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
    (p)->RootDirectory = r; \
    (p)->Attributes = a; \
    (p)->ObjectName = n; \
    (p)->SecurityDescriptor = s; \
    (p)->SecurityQualityOfService = NULL; \
    }

NTSTATUS PhOpenProcess(
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ HANDLE ProcessId
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    CLIENT_ID clientId;

    clientId.UniqueProcess = ProcessId;
    clientId.UniqueThread = NULL;

		HMODULE						hNtDll         = NULL;
	hNtDll =GetModuleHandleA("ntdll.dll");

	NTOPENPROCESS NtOpenProcess = (NTOPENPROCESS)GetProcAddress(hNtDll,"NtOpenProcess");

	
    
    
        InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
       status = NtOpenProcess(
            ProcessHandle,
            DesiredAccess,
            (POBJECT_ATTRIBUTES)&objectAttributes,
            &clientId);
    

    return status;
}






NTSTATUS PhpQueryProcessVariableSize(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_ PVOID *Buffer
    )
{
    NTSTATUS status;
    PVOID buffer;
    ULONG returnLength = 0;

	HMODULE						hNtDll         = NULL;
	hNtDll =GetModuleHandleA("ntdll.dll");

	NTQUERYINFORMATIONPROCESS NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(hNtDll,"NtQueryInformationProcess");

    status = NtQueryInformationProcess(
        ProcessHandle,
        ProcessInformationClass,
        NULL,
        0,
        &returnLength
        );

    if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL && status != STATUS_INFO_LENGTH_MISMATCH)
        return status;

    buffer = (LPVOID)malloc(returnLength);
    status = NtQueryInformationProcess(
        ProcessHandle,
        ProcessInformationClass,
        buffer,
        returnLength,
        &returnLength
        );

    if (NT_SUCCESS(status))
    {

        *Buffer = buffer;
    }
    else
    {
        free(buffer);
    }

    return status;
}



NTSTATUS PhGetProcessCommandLine(
    _In_ HANDLE ProcessHandle,
    _Out_ char *&CommandLine
    )
{

	//if( ProcessHandle == INVALID_HANDLE_VALUE ) 
		printf(" error %ld\r\n",GetLastError());

    NTSTATUS status;

  //  if (WindowsVersion >= WINDOWS_8_1)
    {
        PUNICODE_STRING commandLine;
		SetLastError(0);
        status = PhpQueryProcessVariableSize(
            ProcessHandle,
            ProcessCommandLineInformation,
            (PVOID *)&commandLine
            );

        if (NT_SUCCESS(status))
        {
			//printf( "---->%s\n", &commandLine->Buffer );
			//printf( "%.*S\n", pi->Name.Length/2, &pi->Name.Buffer );
			//wprintf_s(L"--->%s\r\n",commandLine->Buffer);

			CommandLine = new char[commandLine->Length+1];
			memset(CommandLine,0,commandLine->Length+1);
			ULONG i =0;
			 for (i = 0; i < (ULONG)commandLine->Length / 2; i++)
            {
                if (commandLine->Buffer[i] == 0)
                    commandLine->Buffer[i] = ' ';
            }

			//CommandLine = (char*)commandLine->Buffer;//PhCreateStringFromUnicodeString(commandLine);
			memcpy(CommandLine,commandLine->Buffer,commandLine->Length);

			//wprintf_s(L"===>--->%s\r\n",CommandLine);

            free(commandLine);

            return status;
		}else{
			printf("get command error %ld\r\n",GetLastError());
		}
    }
	return 0;//

    //return PhGetProcessPebString(ProcessHandle, PhpoCommandLine, CommandLine);
}



DWORD EnumProcess()
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
             printf("LoadLibrary Error: %d\n",GetLastError());
			__leave;
		}

		NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(hNtDll,"NtQuerySystemInformation");
		if(NtQuerySystemInformation == NULL)
		{
			printf("GetProcAddress for NtQuerySystemInformation Error: %d\n",GetLastError());
			__leave;
		}

		lpSystemInfo = (LPVOID)malloc(dwNumberBytes);
		Status = NtQuerySystemInformation( SystemProcessInformation, lpSystemInfo, dwNumberBytes, &dwReturnLength);
		if(Status == STATUS_INFO_LENGTH_MISMATCH)
		{
			printf("STATUS_INFO_LENGTH_MISMATCH\n");
			__leave;
		}
		else if(Status != STATUS_SUCCESS)
		{
			printf("NtQuerySystemInformation Error: %d\n",GetLastError());
			__leave;
		}

		printf("%-20s%6s%7s%8s%6s%7s%7s%13s\n","ProcessName","PID","PPID","WsSize","Prio.","Thread","Handle","CPU Time");
		printf("--------------------------------------------------------------------------\n");
		pSystemProc = (PSYSTEM_PROCESSES)lpSystemInfo;

		HANDLE hProcess;

		while(pSystemProc->NextEntryDelta != 0)
		{
			if(pSystemProc->ProcessId != 0)
			{
				wprintf(L"%-20s",pSystemProc->ProcessName.Buffer);
			}
			else
			{
				wprintf(L"%-20s",L"System Idle Process");
			}

			// hProcess = ::OpenProcess(  PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ, FALSE, pSystemProc->ProcessId );
//
			 PhOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,(HANDLE)pSystemProc->ProcessId );

			 char *commandline = NULL;
			PhGetProcessCommandLine( hProcess ,commandline );


			//printf("commandline:%s\r\n",commandline);
			wprintf_s(L"commandline==>%s\r\n",commandline);

			if(commandline) delete[] commandline;


			printf("%6d",pSystemProc->ProcessId);
			printf("%7d",pSystemProc->InheritedFromProcessId);
			printf("%7dK",pSystemProc->VmCounters.WorkingSetSize/1024);
			printf("%6d",pSystemProc->BasePriority);
			printf("%7d",pSystemProc->ThreadCount);
			printf("%7d",pSystemProc->HandleCount);
			llTempTime  = pSystemProc->KernelTime.QuadPart + pSystemProc->UserTime.QuadPart;
			llTempTime /= 10000;
			printf("%3d:",llTempTime/(60*60*1000));
			llTempTime %= 60*60*1000;
			printf("%.2d:",llTempTime/(60*1000));
			llTempTime %= 60*1000;
			printf("%.2d.",llTempTime/1000);
			llTempTime %= 1000;
			printf("%.3d",llTempTime);

			printf("\n");
			dwTotalProcess ++;
			pSystemProc = (PSYSTEM_PROCESSES)((char *)pSystemProc + pSystemProc->NextEntryDelta);
		}

		printf("--------------------------------------------------------------------------\n");
		printf("\nTotal %d Process(es) !\n\n",dwTotalProcess);
		printf("PID\t ==> Process Identification\n");
		printf("PPID\t ==> Parent Process Identification\n");
		printf("WsSize\t ==> Working Set Size\n");
		printf("Prio.\t ==> Base Priority\n");
		printf("Thread\t ==> Thread Count\n");
		printf("Handle\t ==> Handle Count\n");
		printf("CPU Time ==> Processor Time\n");
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

	return 0;
}



