/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-06-18
 ******************************/

#include "wing_msg_queue.h"
//----消息队列----------------------------
CRITICAL_SECTION queue_lock;
queue_t *message_queue = NULL;

void wing_msg_queue_init()  
{  
	message_queue = new queue_t();
	InitializeCriticalSection(&queue_lock);
    message_queue->head = NULL; //队头标志位  
    message_queue->tail = NULL; //队尾标志位  
}  
   
int wing_msg_queue_lpush(wing_msg_queue_element *element)  
{  
	//queue_t *hq = message_queue;
	if( message_queue == NULL || NULL == element)
		return 0;

	EnterCriticalSection(&queue_lock);

    node_t * nnode = new node_t();
	
    if (nnode == NULL )  
    {  
		LeaveCriticalSection(&queue_lock);
        return 0;
    } 

    nnode->data = element;  
    nnode->next = NULL;  
    if (message_queue->head == NULL)  
    {  
        message_queue->head = nnode;  
        message_queue->tail = nnode;  
    } else {  
        message_queue->tail->next = nnode;  
        message_queue->tail = nnode;  
    }  
	LeaveCriticalSection(&queue_lock);
    return 1;  
}  
  
void wing_msg_queue_pop_msg(wing_msg_queue_element **temp)  
{  
	//queue_t * hq = message_queue;
	if( NULL == message_queue) return;

	EnterCriticalSection(&queue_lock);
    node_t * p = NULL;  

    if (message_queue->head == NULL)  
    {  
		*temp = NULL;
		LeaveCriticalSection(&queue_lock);
		return;
    }  

    *temp = message_queue->head->data;  
    p = message_queue->head;  
    message_queue->head = message_queue->head->next;  
    if(message_queue->head == NULL)  
    {  
        message_queue->tail = NULL;  
    }  

    delete p;  
	p  = NULL;
	LeaveCriticalSection(&queue_lock);
}  
  
wing_msg_queue_element *wing_peek_msg_queue()  
{  
	if( NULL == message_queue ) return NULL;

    if (message_queue->head == NULL)  
    {  
        return NULL; 
    }   
    return message_queue->head->data;  
}  
  
int wing_msg_queue_is_empty()  
{  
	if( NULL == message_queue ) return 1;

    if (message_queue->head == NULL)  
    {  
        return 1;  
    } else {  
        return 0;  
    }  
}  
    
void wing_msg_queue_clear()  
{  
	if( NULL == message_queue ) return;

    node_t * p = message_queue->head;  
    while(p != NULL)  
    {  
        message_queue->head = message_queue->head->next;  

        delete p; 
		p = NULL;
        p = message_queue->head;  
    }  
    message_queue->tail = NULL;  
	DeleteCriticalSection(&queue_lock);
	delete message_queue;
	message_queue = NULL;
    return;  
}  
 
//----消息队列----------end------------------