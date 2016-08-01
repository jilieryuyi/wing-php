<?php
/**
 * @author yuyi
 * @created 2016/7/30 21:00
 * @email 297341015@qq.com
 * @查询进程 返回进程的启动参数
 */
//指定进程id 返回进程的启动参数 string
var_dump( wing_find_process( 47756 ) );

echo "\r\n\r\n";

//执行关键字查询进程 返回数组
var_dump( wing_find_process( "php" ) );



echo "\r\n\r\n";
//参数为空 获取所有能获取的进程启动参数
var_dump( wing_find_process( ) );

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