<?php
/**
 * @author yuyi
 * @created 2016/8/7 17:13
 * @email 297341015@qq.com
 * @wing http service worker
 */
$pid_file       = __DIR__."/http.pid";
$worker_file    = __DIR__."/service.php";

function command_support($command,$callback){
    global $argv;
    $_command = isset($argv[1])?trim($argv[1],"-"):"";
    if( $_command == $command ) {
        call_user_func($callback);
        exit;
    }
}

//stop 停止http service
command_support("stop",function(){
    global $pid_file;
    global $worker_file;
    $pid = file_get_contents($pid_file);
    wing_process_kill($pid);
    $process_info = wing_query_process($worker_file);
    if( !$process_info )
    {
        echo "wing http service was stop \r\n";
        unlink($pid_file);
    }
    else
        echo "wing http service stop fail\r\n";
});

//start 启动http service
command_support("start",function(){
    global $pid_file;
    global $worker_file;
    $pid = wing_create_process_ex( $worker_file );
    file_put_contents( $pid_file, $pid );
    $process_info = wing_query_process( $worker_file );
    if( $process_info )
        echo "wing http service is running \r\n";
    else
        echo "wing http service start fail\r\n";
});

command_support("restart",function(){
    
    global $pid_file;
    global $worker_file;
    $pid = file_get_contents($pid_file);
    wing_process_kill($pid);
    $process_info = wing_query_process($worker_file);
    if( !$process_info )
    {
        echo "wing http service was stop \r\n";
        unlink($pid_file);
    }
    else
        echo "wing http service stop fail\r\n";

    $pid = wing_create_process_ex( $worker_file );
    file_put_contents( $pid_file, $pid );
    $process_info = wing_query_process( $worker_file );
    if( $process_info )
        echo "wing http service is running \r\n";
    else
        echo "wing http service start fail\r\n";
});