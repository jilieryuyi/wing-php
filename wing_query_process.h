#ifndef __WING_QUERY_PROCESS__
#define __WING_QUERY_PROCESS__
typedef struct _PROCESSINFO {
	char *process_name;
	char *command_line;
	char *file_name;
	char *file_path;
	int process_id;
	int parent_process_id;
	unsigned long working_set_size;
	unsigned long base_priority;//基本的优先级
	unsigned long thread_count ;
	unsigned long handle_count ;
	unsigned long cpu_time;
} PROCESSINFO;

unsigned long  WingQueryProcess( PROCESSINFO *&all_process , int max_count );

#endif