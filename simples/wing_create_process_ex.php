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


//WING_INFINITE 永不超时 即线程不退出 一直等待
$exit_code =  wing_thread_wait($process_id,WING_INFINITE);

if($exit_code == WING_WAIT_TIMEOUT){
    echo "wait timeout";
    exit;
}

if($exit_code == WING_WAIT_FAILED){
    echo "wait fail";
    exit;
}

if($exit_code == WING_WAIT_ABANDONED){
    echo "mutex not release";
    exit;
}

if($exit_code == WING_ERROR_FAILED){
    echo "get exit code fail";
    exit;
}

echo $process_id," is exit with ",$exit_code,"\n";