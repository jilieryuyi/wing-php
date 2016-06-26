#ifndef PHP_WING_LIB
#define PHP_WING_LIB
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



void get_command_path(const char *name,char *&output);
const char* create_guid() ;

#endif