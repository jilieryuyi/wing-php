// linux_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
#ifdef WIN32
	printf("windows");
#else
	printf("other");
#endif
	return 0;
}

