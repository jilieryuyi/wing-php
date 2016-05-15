<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/26
 * Time: 18:51
 * @just a test, does not work ok
 */

//wing_srart_msg_service();
//wing_set_console_title("yuyi123456789");
//var_dump(wing_send_msg("yuyi123456789",0x004A,0,"hello"));
//echo wing_get_last_error();
//wing_message_loop();
wing_get_msg(function($msg){
    var_dump($msg);
});