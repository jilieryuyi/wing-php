
#include <stdio.h>
#include "inttypes.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int GetModuleFileName( char* sModuleName, char* sFileName, int nSize)
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

void main(){ 
	char* PHP_PATH = (char*)malloc(260);
	memset( PHP_PATH, 0, 260 );
	//getcwd( PHP_PATH, 260 );
	//strcat(PHP_PATH,"/php");
	//printf(PHP_PATH);

	GetModuleFileName("php",PHP_PATH,260);

	printf(PHP_PATH);

	char* sPath = getenv("PATH");
	printf(sPath);

	free(PHP_PATH);
}