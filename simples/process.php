<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/21
 * Time: 23:07
 * @create deamon process
 * @创建守护进程
 */
$php        = wing_get_command_path("php");
$process_id = wing_create_process($php,__DIR__."/process_run.php");
echo "process id:",$process_id,PHP_EOL;
echo "now we check the process ",$process_id," is running?",PHP_EOL;
$is_running = wing_process_isalive($process_id);
if($is_running){
    echo "the process ",$process_id," is still running",PHP_EOL;
}

sleep(2);

echo "now we try to kill the process",PHP_EOL;
$kill_status = wing_process_kill($process_id);
if($kill_status){
    echo "kill success",PHP_EOL;
}
echo "check the process ",$process_id," is running again",PHP_EOL;
$is_running = wing_process_isalive($process_id);
//now process chould be is still running,why? besause id chould be reuse by the windows system
//so this check does not mean anything
if($is_running){
    echo "the process ",$process_id," is still running",PHP_EOL;
}else{
    echo 'the process is not running',PHP_EOL;
}
