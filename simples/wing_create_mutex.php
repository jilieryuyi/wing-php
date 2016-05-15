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
// 0程序正在运行 -1 获取参数错误 -2 参数不能为空 -3创建互斥锁失败 >0创建互斥锁成功

register_shutdown_function(function() use($handle){
    wing_close_mutex($handle);
});

echo $handle,"\n";
if($handle===0){
    echo "process is running\n";
    exit;
}
if($handle<0){
    echo "create mutex error\n";
    exit;
}

$i=0;
while(1){
    echo $i,"\n";
    $i++;
    if($i>10)break;
    sleep(1);
}