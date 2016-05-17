<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/26
 * Time: 19:17
 * @send message to some other windows program use WM_COPYDATA
 * @ first param wing_test_title is the  WindowClass name 第一个参数是WindowClass的名称
 * @ two param 0 is the CopyData.dwData 第二个参数是字段 CopyData.dwData
 * @ third param is the message 第三个参数是消息内容
 */
while(1) {
    var_dump(wing_send_msg("wing_test_title", 0, "hello,this msg come from php!"));
    echo wing_get_last_error(),"\n";
    sleep(1);
}