<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/14
 * Time: 16:48
 */
wing_set_env("email","297341015@qq.com");
$process_id = wing_create_process(
    WING_PHP,
    __DIR__."/wing_create_process_runner.php",
    "hello,i come from parent parent process"
);

/*$process_id = wing_create_process_ex(
    __DIR__."/wing_create_process_runner.php",
    "hello,i come from parent parent process"
);*/

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
