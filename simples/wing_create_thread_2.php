<?php
/**
 * @author Administrator
 * @created 2016/5/18 5:40
 * @email huangxiaoan@yonglibao.com
 */
$threads[] = wing_create_thread(function(){
    $count = 0;
    while(1) {
        file_put_contents(__DIR__ . "/wing_create_thread_1.log", date("Y-m-d H:i:s") . "\r\n", FILE_APPEND);
        sleep(1);
        $count++;
        if($count>10)break;
    }
    exit(1);
});

$threads[] = wing_create_thread(function(){
    $count = 0;
    while(1) {
        file_put_contents(__DIR__ . "/wing_create_thread_2.log", date("Y-m-d H:i:s") . "\r\n", FILE_APPEND);
        sleep(1);
        $count++;
        if($count>10)break;
    }
    exit(2);
});

while(1){
    foreach( $threads as $key => $thread_id ){
        //100 Milliseconds timeout 等待100毫秒
        //第二个参数不传或者设置为常量 WING_INFINITE 则永不超时
        $exit_code = wing_thread_wait($thread_id,100);
        //if return wait timeout,thread is still running
        //如果返回超时 说明线程还在运行
        if( WING_WAIT_TIMEOUT == $exit_code )usleep(10000);
        else {
            unset($threads[$key]);
            echo $thread_id," exit with code ",$exit_code,"\r\n";
        }
    }
    //if thread is all exit 如果线程已经全部退出
    if( count($threads) <= 0 )
        break;
}