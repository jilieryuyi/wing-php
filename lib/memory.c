#include <Windows.h>
//#include <iostream>
//#include <fstream>
#include "memory.h"




using namespace std;

void memory_add(char *debug){

	//memory_add_times++;
    InterlockedIncrement(&memory_add_times);

	if(debug!=0){
		//ofstream fout("D:/add_memory.log",ios::app);
		//fout<<debug;
	}

}

void memory_sub(char *debug){
	//memory_sub_times++;
    InterlockedIncrement(&memory_sub_times);

	if(debug!=0){
		//ofstream fout("D:/sub_memory.log",ios::app);
		//fout<<debug;
	}
}

void memory_accept_add_times(){
	//accept_add_times++;
	 InterlockedIncrement(&accept_add_times);
}
void memory_accept_sub_times(){
	//accept_sub_times++;
	InterlockedIncrement(&accept_sub_times);
}



void _queue_add_times(){
	//queue_add_times++;
InterlockedIncrement(&queue_add_times);
}
void _queue_sub_times(){
	//queue_sub_times++;
InterlockedIncrement(&queue_sub_times);
}

void _node_add_times(){
	//node_add_times++;
InterlockedIncrement(&node_add_times);
}
void _node_sub_times(){
	//node_sub_times++;
InterlockedIncrement(&node_sub_times);
}

void _accept_add_times_ex(){
	InterlockedIncrement(&accept_add_times_ex);
}
void _accept_sub_times_ex(){
	InterlockedIncrement(&accept_sub_times_ex);
}

void _recv_add_times(){
	InterlockedIncrement(&recv_add_times);
}
void _recv_sub_times(){
	InterlockedIncrement(&recv_sub_times);
}

void _send_msg_add_times(){
	InterlockedIncrement(&send_msg_add_times);
}
void _send_msg_sub_times(){
	InterlockedIncrement(&send_msg_sub_times);
}