<?php
/**
 * @author yuyi
 * @created 2016/7/30 20:00
 * @email 297341015@qq.com
 */
//获取当前进程id
$current_process_id  = wing_get_current_process_id();
$data                = wing_get_process_params(); //从父进程接收数据
$env                 = wing_get_env("data"); //通过环境变量从父进程接收数据

file_put_contents("wing_create_process.log",
    "进程id：".$current_process_id."\r\n从父进程获取到的参数：".
    $data."\r\n从父进程获取到的环境变量：".$env."\r\n");
file_put_contents("wing_create_process_cmd.log",
    wing_get_command_line());

$handle = 0;
for( $i=0; $i<$argc; $i++ ){
    if( strpos($argv[$i],"handle=") === 0 ) {
        $temp   = explode("=",$argv[$i]);
        $handle = $temp[1];
    }
}


for( $i=0; $i<$argc; $i++ )
file_put_contents("wing_create_process_arg.log",
    $argv[$i]."\r\n\r\n",FILE_APPEND);

sleep(3);
exit(999);