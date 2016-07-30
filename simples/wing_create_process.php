<?php
/**
 * @author yuyi
 * @created 2016/7/30 20:00
 * @email 297341015@qq.com
 */
wing_set_env("data","这句话将被传到子进程");
$command = WING_PHP." ".__DIR__."/wing_create_process_runer.php";
$process_id = wing_create_process( $command, "这句话也会被传到子进程");
//也可以这么写
echo "进程id:",$process_id,"\r\n";
//$process_id = wing_create_process_ex(__DIR__."/wing_create_process_runer.php","这句话将被传到子进程");

//wing_process_kill( $process_id ); //可以是用这个api杀死正在运行的进程

if( WING_ERROR_PROCESS_IS_RUNNING == wing_process_isalive( $process_id ) ) {
    //wing_process_isalive 不要太依赖此api，因为进程id的复用性，进程退出后，有可能相同的进程id立马被创建
    echo $process_id,"正在运行\r\n";
}



$wait_code = wing_process_wait( $process_id, WING_INFINITE ); //永不超时等待子进程退出
switch( $wait_code ) {
    case WING_ERROR_FAILED :
            echo "等待失败\r\n";
        break;
    default:
        echo "进程退出码：",$wait_code,"\r\n"; //在子进程调用exit时传入的参数
}

