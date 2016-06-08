#include "queue.h"
#include "memory.h"

  
/*1. 初始化链队列*/  
// 其  初始化的 操作就是初始化队列的队头和队尾的两个标志位，
// 所以就有删除或是插入的时候，会判断有没有 队列为空的时候。  
void initQueue(queue_t * queue_eg)  
{  
	InitializeCriticalSection(&queue_lock);
    queue_eg->head = NULL; //队头标志位  
    queue_eg->tail = NULL; //队尾标志位  
}  
  
/*2.向链队的<span style="background-color: rgb(255, 102, 102);">队尾插入</span>一个元素x*/  
int enQueue(queue_t *hq, elemType *x)  
{  
	EnterCriticalSection(&queue_lock);
	if(hq == NULL)
		return 0;

    node_t * new_p = new node_t();//(node_t *)malloc(sizeof(queue_t));  
	
    if (new_p == NULL )  
    {  
		LeaveCriticalSection(&queue_lock);
        return 0;
    }  
    new_p->data = x;  
    new_p->next = NULL;  
    if (hq->head == NULL)  
    {  
        hq->head = new_p;  
        hq->tail = new_p;  
    } else {  
        hq->tail->next = new_p;  
        hq->tail = new_p;  
    }  
	
	memory_add();
	LeaveCriticalSection(&queue_lock);
    return 1;  
}  
  
/*3. 从列队中<span style="background-color: rgb(255, 153, 102);">队首删除</span>一个元素*/  
void outQueue(queue_t * hq,elemType *temp)  
{  EnterCriticalSection(&queue_lock);
    node_t * p;  
   // elemType temp;//=new elemType();  queue_times++;
    if (hq->head == NULL)  
    {  
       // printf("队列为空，不能删除！");  
       // exit(1);  
		//return;// NULL;
		temp=NULL;LeaveCriticalSection(&queue_lock);
		return;//0;
    }  

    temp = hq->head->data;  
    p = hq->head;  
    hq->head = hq->head->next;  
    if(hq->head == NULL)  
    {  
        hq->tail = NULL;  
    }  
    free(p);  
	memory_sub();
	LeaveCriticalSection(&queue_lock);
}  
  
/*4. 读取队首元素 */  
elemType *peekQueue(queue_t * hq)  
{  
    if (hq->head == NULL)  
    {  
        printf("队列为空。");  
        exit(1);  
    }   
    return hq->head->data;  
}  
  
/*5. 检查队列是否为空，若是空返回1，若不为空返回0 。*/  
int is_emptyQueue(queue_t * hq)  
{  
    if (hq->head == NULL)  
    {  
        return 1;  
    } else {  
        return 0;  
    }  
}  
  
/*6. 清除链队中的所有元素*/  
  
void clearQueue(queue_t * hq)  
{  
    node_t * p = hq->head;  
    while(p != NULL)  
    {  
        hq->head = hq->head->next;  
        free(p);  memory_sub("sub memory clearQueue 93\r\n");
        p = hq->head;  
    }  
    hq->tail = NULL;  
    return;  
}  
  
/*main()函数*/  
/*int main(int argc, char* argv[])  
{  
    queue_t q;  
    int a[8] = {1, 2, 3, 4, 5, 6, 7, 8};  
    int i;  
    initQueue(&q);  
    for(i=0; i<8; i++)  
    {  
        enQueue(&q, a[i]);  
    }  
    //printf("%d",outQueue(&q));  
  
    enQueue(&q, 68);  
    //printf("%d", peekQueue(&q));  
      
    while (!is_emptyQueue(&q))  
    {  
        printf("%d.\n", outQueue(&q));  
    }  
  
    printf(" \n");  
    clearQueue(&q);  
    system("pause");  
    system("pause");  
    system("pause");  
}  */