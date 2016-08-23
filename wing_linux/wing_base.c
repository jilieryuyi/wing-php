#include <stdio.h>
#include "inttypes.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int wing_get_module_file_name( char* sModuleName, char* sFileName, int nSize)
{
	int ret = -1;
	if( strchr( sModuleName,'/' ) != NULL )
		strcpy( sFileName, sModuleName );
	else
	{
		char* sPath = getenv("PATH");
		char* pHead = sPath;
		char* pTail = NULL;
		while( pHead != NULL && *pHead != '\x0' )
		{
			pTail = strchr( pHead, ':' );
			if( pTail != NULL )
			{
				strncpy( sFileName, pHead, pTail-pHead );
				sFileName[pTail-pHead] = '\x0';
				pHead = pTail+1;
			}
			else
			{
				strcpy( sFileName, pHead );
				pHead = NULL;
			}

			int nLen = strlen(sFileName);
			if( sFileName[nLen] != '/' )sFileName[nLen] = '/';
			strcpy( sFileName+nLen+1,sModuleName);
			if( 0 == access( sFileName, F_OK ) )
			{
				ret = 0;
				break;
			}
		}
	}
	return ret;
}