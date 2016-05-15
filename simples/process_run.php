<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/21
 * Time: 23:09
 * @process runner
 */
$i=0;
//try to save the current process id
file_put_contents(__DIR__."/wing.pid",wing_getpid());
while(1){
    $i++;
    //do something in the process
    file_put_contents(__DIR__."/1.html",$i,FILE_APPEND);
    sleep(1);
    if($i>3)break;
}

exit(10);