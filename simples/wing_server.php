<?php
/**
 * @author yuyi
 * @created 2016/6/24 6:17
 * @email 297341015@qq.com
 */
$server = new wing_server( "0.0.0.0" , 9998 );
$server->on( "onreceive" , function( $client , $recv_msg ){

    echo "recv from:",$client->socket,"=>",$recv_msg,"\r\n";

    $client->send( "hello client\r\n" );
});
$server->on("onconnect",function($client){
    echo $client->socket," connect\r\n";
});
$server->on("onclose",function($client){
    echo $client->socket," close \r\n";
});
$server->on("onerror",function($client,$error_code,$last_error_num){
    echo $client->socket," some error happened,",$error_code,",",$last_error_num, "\r\n";
});
$server->start();