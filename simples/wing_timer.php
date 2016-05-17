<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/15
 * Time: 17:57
 * Milliseconds Timer 毫秒级别定时器
 */

wing_timer(500,function(){
    static $count =1;
    file_put_contents(__DIR__."/wing_timer1.log",($count++)."=>".date("Y-m-d H:i:s")."\r\n",FILE_APPEND);
},10/*max run times,default 0,no limit*/);

wing_timer(500,function(){
    static $count =1;
    file_put_contents(__DIR__."/wing_timer2.log",($count++)."=>".date("Y-m-d H:i:s")."\r\n",FILE_APPEND);
},100/*max run times,default 0,no limit*/);

wing_timer(500,function(){
    static $count =1;
    file_put_contents(__DIR__."/wing_timer3.log",($count++)."=>".date("Y-m-d H:i:s")."\r\n",FILE_APPEND);
},0/*max run times,default 0,no limit*/);