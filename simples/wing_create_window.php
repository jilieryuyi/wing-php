<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/27
 * Time: 5:46
 * @create a window
 */
//wing_message_box("first windows php","php");
//创建一个窗口
$hwnd = wing_create_window("yuyi123456789");
//退出程序时销毁
register_shutdown_function(function() use($hwnd){
    wing_destory_window($hwnd);
});
//启动窗口消息循环
wing_message_loop();
