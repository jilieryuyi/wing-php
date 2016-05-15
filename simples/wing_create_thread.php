<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/24
 * Time: 18:09
 * @just a test,does not work ok
 */
//create a thread
foreach($argv as $v)
{
    file_put_contents(__DIR__."/thread_test.log",$v."\n",FILE_APPEND);
}

$thread_id = wing_create_thread(function(){
    $i=0;
    while(1){
        file_put_contents(__DIR__."/thread_test.log",($i++)."\n",FILE_APPEND);
        if($i>3)break;
        sleep(1);
    }
    exit(9);
});
if($thread_id<0){
    echo "create thread fail";
    exit;
}
if($thread_id == 0) {
    echo "child thread ";
    echo $thread_id;
    //child thread just exit
    exit;
}

//$thread_id parent continue running

//wait for the thread exit
//If you want a simple asynchronous, comment this line
//return ===false wait failue
//return ===-1 wait timeout
//-1 Never timed out >0 Milliseconds timeout -1永不超时 >0毫秒超时
$exit_code =  wing_thread_wait($thread_id,-1);
if($exit_code === false){
    echo "wait failue \n";
    exit;
}
if($exit_code === -1){
    echo "wait timeout \n";
}

echo $thread_id," is exit with ",$exit_code,"\n";
