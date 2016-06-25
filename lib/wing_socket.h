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

#include <ws2tcpip.h>//ipv4 ipv6
//int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result );
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms742203(v=vs.85).aspx

#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#define DATA_BUFSIZE	1024
//#define SEND_DATA_BUFSIZE 1048576 //1024x1024 1M

//iocp的动作类型 accept 新的连接 recv 收到消息 send异步发送
#define OPE_RECV		1
#define OPE_SEND		2
#define OPE_ACCEPT		3

//自定义消息
#define WM_ONCONNECT		WM_USER + 60
#define WM_ACCEPT_ERROR		WM_USER + 61
#define WM_ONERROR			WM_USER + 62
#define WM_ONCLOSE			WM_USER + 63
#define WM_ONRECV			WM_USER + 64
#define WM_ONQUIT           WM_USER + 65
#define WM_ONCLOSE_EX		WM_USER + 66
#define WM_ONSEND			WM_USER + 67
#define WM_THREAD_RUN		WM_USER + 71
#define WM_TIMEOUT          WM_USER + 72
#define WM_ADD_CLIENT       WM_USER + 73

//测试消息
#define WM_TEST				WM_USER+68
#define WM_TEST2			WM_USER+69
#define WM_TEST3			WM_USER+70

//错误码
#define WING_ERROR_CLOSE_SOCKET -4001
#define WING_ERROR_ACCEPT		-4002
#define WING_ERROR_MALLOC		-4003
#define WING_BAD_ERROR			-4
#define WING_ERROR_KEEP_ALIVE	-4004
#define WING_ERROR_POST_RECV    -4005
#define WING_BAD_ERROR          -4006


#define WING_SOCKET_IS_ALIVE 1 //标记socket是否激活状态
#define WING_SOCKET_IS_SLEEP 0 //非激活状态

//iocp消息结构体
struct MYOVERLAPPED{
	OVERLAPPED	m_ol;                          //异步依赖
	int			m_iOpType;                     //操作类型
	SOCKET		m_skServer;                    //服务端socket
	SOCKET		m_skClient;                    //客户端
	DWORD		m_recvBytes;                   //接收的消息长度
	char		m_pBuf[DATA_BUFSIZE];          //接收消息的缓冲区
	WSABUF		m_DataBuf;                     //消息缓冲
	int			m_timeout;                     //设置超时
	SOCKADDR_IN m_addrClient;                  //客户端实际的地址
	SOCKADDR_IN m_addrServer;                  //服务端实际的地址
	int			m_isUsed;                      //标志是否已被激活使用 1已被激活 0待激活
	unsigned    m_active;                      //最后的活动时间
	//LPVOID      m_client;                    //wing_client 对象
};
typedef SOCKET wing_socket;
typedef MYOVERLAPPED wing_myoverlapped;
typedef LPOVERLAPPED wing_lpoverlapped;
//消息队列传递的消息
typedef struct{
	long len;
	char *msg;//[DATA_BUFSIZE+1]; 
} RECV_MSG;
typedef RECV_MSG wing_msg;

//iocp 线程池 工作线程
VOID CALLBACK wing_icop_thread(DWORD dwErrorCode,DWORD dwBytesTrans,LPOVERLAPPED lpOverlapped);

//socket MYOVERLAPPED hash map 用来存储对应的关系映射
//-------------socket MYOVERLAPPED map------------------------------------
//添加映射关系
 void				wing_add_to_sockets_map(unsigned long socket,unsigned long ovl);
//通过socket获取ovl
 unsigned long	wing_get_from_sockets_map(unsigned long socket);
//移除映射关系
 void				wing_remove_from_sockets_map(unsigned long socket);
//获取hash map长度 可以用来调试 一般如果发生错误 某些socket会被移除掉 造成可用的socket越来越少
//不过加上了socket异常关闭的增补方案 也就是说socket池的socket由于异常被清理之后 会自动添加新的补上
 unsigned int		wing_get_sockets_map_size();

//异常退出
void   wing_socket_throw_error( int error_code );
//服务端socket初始化
SOCKET wing_socket_init(const char *listen_ip,const int port,const int max_connect = 10000,const int timeout = 0);
//socket结束后的资源清理
void   wing_socket_clear();

unsigned int __stdcall wing_socket_check_active_timeout(PVOID params);
void wing_socket_add_client(wing_myoverlapped *&pMyOL);

//掉线回调函数
void   wing_socket_on_close(MYOVERLAPPED*  &pMyOL);
//post消息到 消息队列
void   wing_post_queue_msg(int message_id,unsigned long wparam=0,unsigned long lparam=0,unsigned long eparam=0);
//断开连接
BOOL WingDisconnectEx(SOCKET hSocket,LPOVERLAPPED lpOverlapped,DWORD dwFlags,DWORD reserved);
//接受新的连接
BOOL WingAcceptEx(SOCKET sListenSocket,SOCKET sAcceptSocket,PVOID lpOutputBuffer,DWORD dwReceiveDataLength,DWORD dwLocalAddressLength,DWORD dwRemoteAddressLength,LPDWORD lpdwBytesReceived,LPOVERLAPPED lpOverlapped);

#endif