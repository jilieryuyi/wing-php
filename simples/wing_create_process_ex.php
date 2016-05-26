<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/12
 * Time: 8:28
 */
$process_id = wing_create_process_ex(__DIR__."/wing_create_process_runner.php",
    "hello"//send hello to child process
);

if($process_id == WING_ERROR_PARAMETER_ERROR){
    echo "parameter error";
    exit;
}

if($process_id == WING_ERROR_FAILED){
    echo "create process fail";
    exit;
}

echo "exit with ",wing_process_wait($process_id),"\n";