<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/24
 * Time: 18:09
 * @just a test,does not work ok
 */
//create a thread
//参数错误 WING_ERROR_PARAMETER_ERROR
//失败返回值 WING_ERROR_FAILED
//可以忽略的返回值 WING_NOTICE_IGNORE
//执行线程函数失败 WING_ERROR_CALLBACK_FAILED
//执行回调函数成功 WING_CALLBACK_SUCCESS
$thread_id = wing_create_thread(function(){
    $i = 0;
    while(1){

        file_put_contents(__DIR__."/wing_create_thread.log",($i++)."\n",FILE_APPEND);
        if( $i > 10 ) break;
        sleep(1);

    }
    exit(9);
});

if($thread_id == WING_ERROR_PARAMETER_ERROR){
    echo "parameter error";
    exit;
}

if($thread_id == WING_ERROR_FAILED){
    echo "create thread error";
    exit;
}

if($thread_id == WING_ERROR_CALLBACK_FAILED){
    echo "call func error";
    exit;
}

if($thread_id == WING_CALLBACK_SUCCESS){
    echo "call func success";
}


//WING_INFINITE 永不超时 即线程不退出 一直等待
$exit_code =  wing_thread_wait($thread_id,WING_INFINITE);

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

echo $thread_id," is exit with ",$exit_code,"\n";
