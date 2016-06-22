<?php
/**
 * @author yuyi
 * @created 2016/6/22 7:31
 * @email 297341015@qq.com
 * @tcp 服务器
 */
$server = new wing_server("0.0.0.0",9998,10000);
$server->on("onreceive",function($client,$recv_meg){

});
$server->on("onconnect",function($client){

});
$server->on("onclose",function($client){

});
$server->on("onerror",function($client,$error_code,$last_error_num){

});
$server->start();