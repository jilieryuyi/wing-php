#ifndef __WING__QUEUE__
#define __WING__QUEUE__
#include <stdio.h>  
#include <stdlib.h>  
#include <Windows.h>
CRITICAL_SECTION queue_lock;
//typedef int elemType;  
/**************************/  
/*           */  
/**************************/  
typedef struct _thread_msg{
	int message_id;
	unsigned long lparam;
	unsigned long wparam;

} THREAD_MSG,elemType;

typedef struct nodet   
{  
    elemType *data;  
    struct nodet * next;  
} node_t;            // 节点的结构  
  
typedef struct queuet  
{  
    node_t * head;  
    node_t * tail;  
} queue_t;          // 队列的结构  




void initQueue(queue_t * queue_eg) ;
int enQueue(queue_t *hq, elemType *x) ;
void outQueue(queue_t * hq,elemType *b);
void clearQueue(queue_t * hq) ;
int is_emptyQueue(queue_t * hq)  ;
#endif