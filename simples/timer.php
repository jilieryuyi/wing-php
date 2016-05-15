<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/28
 * Time: 17:58
 */
$php        = wing_get_command_path("php");
$i=0;
while(true){
    //micro seconds timer
    $process_id = wing_create_process($php,__DIR__."/timer_runner.php ".($i++));
    usleep(1000);
}