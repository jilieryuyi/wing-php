#ifndef __WING__MEMORY__
#define __WING__MEMORY__

unsigned long queue_times = 0;
unsigned long memory_add_times = 0;
unsigned long memory_sub_times = 0;
unsigned long accept_times = 0;

unsigned long accept_add_times = 0;
unsigned long accept_sub_times = 0;

unsigned long send_times = 0;
unsigned long recv_times = 0;


unsigned long queue_add_times = 0;
unsigned long queue_sub_times = 0;

unsigned long node_add_times = 0;
unsigned long node_sub_times = 0;


unsigned long accept_add_times_ex = 0;
unsigned long accept_sub_times_ex = 0;

void memory_add(char *debug=0);
void memory_sub(char *debug=0);

void memory_accept_add_times();
void memory_accept_sub_times();

void _queue_add_times();//{}
void _queue_sub_times();//{}
void _node_add_times();
void _node_sub_times();

void _accept_add_times_ex();
void _accept_sub_times_ex();

#endif