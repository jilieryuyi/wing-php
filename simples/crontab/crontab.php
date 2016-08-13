<?php
/**
 * @author yuyi
 * @created 2016/8/7 7:49
 * @email 297341015@qq.com
 * @crontab定时任务管理器
 */
$pid_file       = __DIR__."/wing_crontab.pid";
$contab_file    = __DIR__."/wing_contab.conf";

function command_support($command,$callback){
    global $argv;
    $_command = isset($argv[1])?trim($argv[1],"-"):"";
    if( $_command == $command ) {
        call_user_func($callback);
        exit;
    }
}

//l指令支持
command_support("l",function(){
    global $contab_file;
    $contents       = file_get_contents($contab_file);
    $crontabs       = explode("\n",$contents);
    foreach ( $crontabs as $crontab ) {
        $_crontab = trim($crontab);
        if (strpos($_crontab, "#") === 0 || !$_crontab)
            continue;
        echo $_crontab,"\r\n";
    }
});

//stop 停止crontab
command_support("stop",function(){
    global $pid_file;
    $pid = file_get_contents($pid_file);
    wing_process_kill($pid);
    $process_info = wing_query_process("wing_crontab.php");
    if( !$process_info )
    {
        echo "crontab was stop \r\n";
        unlink($pid_file);
    }
    else
        echo "crontab stop fail\r\n";
});

//start 启动crontab
command_support("start",function(){
    global $pid_file;
    $pid = wing_create_process_ex(__DIR__."/wing_crontab.php");
    file_put_contents( $pid_file, $pid );
    $process_info = wing_query_process("wing_crontab.php");
    if( $process_info )
        echo "crontab is running \r\n";
    else
        echo "crontab start fail\r\n";
});