<?php
/**
 * @author yuyi
 * @created 2016/8/12 22:13
 * @email 297341015@qq.com
 * @进程枚举与搜索
    #define WING_SEARCH_BY_PROCESS_EXE_FILE  1
    #define WING_SEARCH_BY_PROCESS_NAME      1

    #define WING_SEARCH_BY_PROCESS_ID        2
    #define WING_SEARCH_BY_PARENT_PROCESS_ID 3
    #define WING_SEARCH_BY_COMMAND_LINE      4

    #define WING_SEARCH_BY_PROCESS_EXE_PATH  5
    #define WING_SEARCH_BY_PROCESS_FILE_PATH 5
 *
process_id进程id
parent_process_id 父进程id
working_set_size 占用工作内存 /1024 =k 单位为字节
base_priority 基本的优先级
thread_count 线程数
handle_count 句柄数
cpu_time cpu时间 工作时长 单位为妙
 */
//wing_create_process_ex(__DIR__."/wing_create_process_runer.php");
//为空时 枚举系统所有的进程
//var_dump( wing_query_process(__DIR__."/wing_create_process_runer.php") );

//查询某个关键字 不指定搜索指定的字段
//var_dump( wing_query_process("PhpStorm.exe") );

//查询某个关键字 指定搜索进程名称
//var_dump( wing_query_process("PhpStorm.exe",WING_SEARCH_BY_PROCESS_NAME) );

//var_dump( wing_query_process("PhpStorm.exe",WING_SEARCH_BY_PROCESS_FILE_PATH) );

//var_dump( wing_query_process(7680,WING_SEARCH_BY_PARENT_PROCESS_ID) );

var_dump( wing_query_process() );