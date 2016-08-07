<?php
/**
 * @author yuyi
 * @created 2016/8/4 10:32
 * @email 297341015@qq.com
 * @使用select模型的tcp服务器
 * @可以用项目里面的socket_client.exe测试，也可以直接浏览器打开 http://127.0.0.1:9998/
 * @简单的http服务器
 */
$server = new wing_select_server(
    "0.0.0.0" ,  //ip 默认值 0.0.0.0
    9998 ,       //端口 默认值 6998
    20000,       //最大连接数限制 默认值 1000
    1000,        //发送/接收超时时间 默认值 0 不限
    3000         //未活动超时时间 默认值 0 不限
);

$server->on( "onreceive" , function( $client , $recv_msg ) {

    echo "收到来自",$client->socket,"的消息：",$recv_msg,"\r\n\r\n";

    $response_content   = "hello from wing php";
    $headers            = [
        "HTTP/1.1 200 OK",
        "Connection: Close",
        "Server: wing php ".WING_VERSION,
        "Date: " . gmdate("D,d M Y H:m:s")." GMT",
        "Content-Type: text/html",
        "Content-Length: " . strlen($response_content)
    ];

    $client->send( implode("\r\n",$headers)."\r\n\r\n".$response_content);
});

$server->on( "onsend" , function( $client , $send_status ){
    echo $client->socket;
    if( $send_status )
        echo "发送成功";
    else
        echo "发送失败";
    echo "\r\n";
});
$server->on( "onconnect",function( $client ) {
    echo $client->socket,"连接进来了\r\n";
});

$server->on( "onclose",function( $client ) {
    echo $client->socket,"掉线啦\r\n";
});

$server->on( "onerror", function( $client, $error_code, $error_msg ) {
    echo $client->socket,"发生了错误，错误码",$error_code,"，错误内容：", $error_msg, "\r\n";
});

$server->on( "ontimeout" , function( $client ) {
    echo $client->socket,"好长时间没活动啦\r\n";
});

$server->start();