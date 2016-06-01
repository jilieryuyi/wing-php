/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-05-20
 ******************************/
#ifndef __WING_SOCKET__
#define __WING_SOCKET__


#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Winbase.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "process.h"
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")


#define DATA_BUFSIZE 8192
typedef struct  
{ 

  OVERLAPPED OVerlapped; 
  WSABUF DATABuf; 
  CHAR Buffer[DATA_BUFSIZE]; 
  DWORD BytesSend,BytesRecv; 

}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

 
typedef struct{ 
  SOCKET Socket;
} PER_HANDLE_DATA,*LPPER_HANDLE_DATA; 

typedef struct{ 
  SOCKET Socket;
  HANDLE IOCompletionPort;
  DWORD threadid;
} COMPARAMS; 

#endif // !__WING_SOCKET__