<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/25
 * Time: 20:17
 * @run this demo twice at the same time
 * @同一时间运行此demo两次 查看互斥量的效果
 */
$handle = wing_create_mutex("a test mutex");
// WING_ERROR_ALREADY_EXISTS 程序正在运行
// WING_ERROR_PARAMETER_ERROR 参数错误
// WING_ERROR_FAILED 创建互斥锁失败
// 其他 创建互斥锁成功


echo $handle,"\n";
if( $handle === WING_ERROR_ALREADY_EXISTS ){
    echo "process is running\n";
    exit;
}

if( $handle === WING_ERROR_PARAMETER_ERROR){
    echo "parameter error\n";
    exit;
}

if( $handle === WING_ERROR_FAILED ){
    echo "create mutex error\n";
    exit;
}

register_shutdown_function(function() use($handle){
    wing_close_mutex($handle);
});

$i = 0;
while(1){
    echo $i,"\n";
    $i++;
    if($i>10)break;
    sleep(1);
}