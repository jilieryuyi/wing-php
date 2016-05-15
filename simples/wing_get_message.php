<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/15
 * Time: 17:37
 */

while(1){
    $message = wing_get_message();
    //msg.message 区分消息类型
    //msg.lParam;
    //msg.wParam;
    //msg.time;
    if(!is_array($message))continue;
    file_put_contents(__DIR__."/wing_get_message.log",
        "message=>".$message["message"]."\r\n".
        "lParam=>".$message["lParam"]."\r\n".
        "wParam=>".$message["wParam"]."\r\n".
        "time=>".date("Y-m-d H:i:s",$message["time"])."===".(time()-$message["time"])."\r\n\r\n----------------------------------\r\n\r\n",
        FILE_APPEND
    );
}