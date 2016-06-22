<?php
/**
 * @author yuyi
 * @created 2016/6/22 7:31
 * @email 297341015@qq.com
 * @tcp 服务器
 */
$server = new wing_server("0.0.0.0",9998,10000);
$server->on("onreceive",function($client,$recv_meg){
    echo "recv from:",$client,"=>",$recv_meg,"\r\n";
    //输出信息到http请求页面
    wing_socket_send_msg( $client, "hello from wing php" );
    //直接关闭连接
    wing_close_socket( $client );
});
$server->on("onconnect",function($client){
    echo $client," connect\r\n";
});
$server->on("onclose",function($client){
    echo $client," close \r\n";
});
$server->on("onerror",function($client,$error_code,$last_error_num){
    echo $client," some error happened,",$error_code,",",$last_error_num, "\r\n";
});
$server->start();