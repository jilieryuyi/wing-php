#ifndef __WING__QUEUE__
#define __WING__QUEUE__
#include <stdio.h>  
#include <stdlib.h>  
#include <Windows.h>



void initQueue(queue_t * queue_eg) ;
int enQueue(queue_t *hq, elemType *x) ;
void outQueue(queue_t * hq,elemType *b);
void clearQueue(queue_t * hq) ;
int is_emptyQueue(queue_t * hq)  ;
#endif