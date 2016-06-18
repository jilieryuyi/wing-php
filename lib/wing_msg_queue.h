/*******************************
 *@author yuyi
 *@email 297341015@qq.com
 *@created 2016-06-18
 ******************************/
#ifndef PHP_WING_MSG_QUEUE_H
#define PHP_WING_MSG_QUEUE_H
#include "Windows.h"

typedef struct _thread_msg{
	int message_id;
	unsigned long lparam;
	unsigned long wparam;
	unsigned int size;

} wing_msg_queue_element;

typedef struct nodet   
{  
    wing_msg_queue_element *data;  
    struct nodet * next;  
} node_t;            // 节点的结构  
  
typedef struct queuet  
{  
    node_t * head;  
    node_t * tail;  
} queue_t;          // 队列的结构  




void wing_msg_queue_init();
int wing_msg_queue_lpush(wing_msg_queue_element *x)  ;
void wing_msg_queue_pop_msg(wing_msg_queue_element **temp);
wing_msg_queue_element *wing_peek_msg_queue() ;
int wing_msg_queue_is_empty()  ;
void wing_msg_queue_clear()  ;

#endif