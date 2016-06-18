#ifndef PHP_WING_SOCKET
#define PHP_WING_SOCKET
#include <Winsock2.h>
#include "Windows.h"
#include "Winbase.h"

#include "tlhelp32.h"
#include "Psapi.h"
#include "Winternl.h"
#include "Processthreadsapi.h"
#include "Shlwapi.h"
#include "Strsafe.h"
#include "Mmsystem.h"
#include "mstcpip.h"
#include "process.h"
#include <mswsock.h>


#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#define DATA_BUFSIZE 1024
#define OPE_RECV 1
#define OPE_SEND 2
#define OPE_ACCEPT 3


#define WM_ONCONNECT		WM_USER+60
#define WM_ACCEPT_ERROR		WM_USER+61
#define WM_ONERROR			WM_USER+62
#define WM_ONCLOSE			WM_USER+63
#define WM_ONRECV			WM_USER+64
#define WM_ONQUIT           WM_USER+65
#define WM_ONCLOSE_EX		WM_USER+66
#define WM_ONSEND			WM_USER+67

#define WM_TEST				WM_USER+68
#define WM_TEST2			WM_USER+69
#define WM_TEST3			WM_USER+70

#define WING_ERROR_CLOSE_SOCKET -4001
#define WING_ERROR_ACCEPT		-4002
#define WING_ERROR_MALLOC		-4003
#define WING_BAD_ERROR			-4


#define WING_SOCKET_IS_ALIVE 1
#define WING_SOCKET_IS_SLEEP 0

struct MYOVERLAPPED{
	OVERLAPPED	m_ol;
	int			m_iOpType;
	SOCKET		m_skServer;
	SOCKET		m_skClient;
	DWORD		m_recvBytes;
	char		m_pBuf[DATA_BUFSIZE];
	WSABUF		m_DataBuf; 
	int			m_timeout;
	SOCKADDR_IN m_addrClient;
	SOCKADDR_IN m_addrServer;
	int			m_isUsed;//标志是否已被激活使用 1已被激活 0待激活
};


typedef struct{
	long len;
	char *msg;//[DATA_BUFSIZE+1]; 
} RECV_MSG;

VOID CALLBACK wing_icop_thread(DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped);

//-------------socket MYOVERLAPPED map------------------------------------
extern void				wing_add_to_sockets_map(unsigned long socket,unsigned long ovl);
extern unsigned long	wing_get_from_sockets_map(unsigned long socket);
extern void				wing_remove_from_sockets_map(unsigned long socket);
extern unsigned int		wing_get_sockets_map_size();


void   wing_socket_throw_error( int error_code );
SOCKET wing_socket_init(const char *listen_ip,const int port,const int max_connect,const int timeout);
void   wing_socket_clear();
void   wing_socket_on_close(MYOVERLAPPED*  &pMyOL);
void   wing_post_queue_msg(int message_id,unsigned long wparam=0,unsigned long lparam=0);

BOOL WingDisconnectEx(SOCKET hSocket,LPOVERLAPPED lpOverlapped,DWORD dwFlags,DWORD reserved);
BOOL WingAcceptEx(SOCKET sListenSocket,SOCKET sAcceptSocket,PVOID lpOutputBuffer,DWORD dwReceiveDataLength,DWORD dwLocalAddressLength,DWORD dwRemoteAddressLength,LPDWORD lpdwBytesReceived,LPOVERLAPPED lpOverlapped);

#endif