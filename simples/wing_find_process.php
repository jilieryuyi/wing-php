<?php
/**
 * @author yuyi
 * @created 2016/7/30 21:00
 * @email 297341015@qq.com
 * @查询进程 返回进程的启动参数
 * @函数原型 wing_find_process( int|string keyword , int search_by )
 * @第一个参数为需要搜索的关键字 如果搜索指定的进程id 可以直接搜索数字，也可以搜索某个指定的字符串关键字
 * @第二个字段是想按照什么字段搜索，必须是一下常量之一 默认为0 ，即不指定搜索条件
 * @#define WING_SEARCH_BY_PROCESS_EXE_FILE  1
 * @#define WING_SEARCH_BY_PROCESS_ID        2
 * @#define WING_SEARCH_BY_PARENT_PROCESS_ID 3
 * @#define WING_SEARCH_BY_COMMAND_LINE      4
 * @#define WING_SEARCH_BY_PROCESS_EXE_PATH  5
 */



//var_dump( wing_find_process("cloudmusic.exe\"",WING_SEARCH_BY_COMMAND_LINE ));

var_dump( wing_find_process( "18204" ));
//指定进程id 返回进程的启动参数 string
//var_dump( wing_find_process( 47756 ) );

//echo "\r\n\r\n";

//执行关键字查询进程 返回数组
//var_dump( wing_find_process( "Note" ) );



//echo "\r\n\r\n";
//参数为空 获取所有能获取的进程启动参数
//var_dump( wing_find_process("cloudmusic.exe"),WING_SEARCH_BY_PROCESS_EXE_FILE );

/*使用com查找的方式 CommandLine就是进程的启动参数
 * function com_find_process( $keyword ){
    $obj       = new COM ( 'winmgmts://localhost/root/CIMV2' );
    $processes = $obj->ExecQuery("Select * from Win32_Process Where Name=\"php.exe\" ");

    foreach ($processes as $process) {
        if ( strpos($process->CommandLine, $keyword) !== false ) {
             return true;
        }
    }
    return false;
}*/