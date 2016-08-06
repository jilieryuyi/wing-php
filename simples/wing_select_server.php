<?php
/**
 * @author yuyi
 * @created 2016/8/4 10:32
 * @email 297341015@qq.com
 * @使用select模型的tcp服务器
 * @可以用项目里面的socket_client.exe测试，也可以直接浏览器打开 http://127.0.0.1:9998/
 */
$server = new wing_select_server(
    "0.0.0.0" ,  //ip 默认值 0.0.0.0
    9998 ,       //端口 默认值 6998
    20000,       //最大连接数限制 默认值 1000
    1000,        //发送/接收超时时间 默认值 0 不限
    3000         //未活动超时时间 默认值 0 不限
);

$server->on( "onreceive" , function( $client , $recv_msg ) {

    echo "recv from:",$client->socket,"=>",$recv_msg,"\r\n";

    /*$response_content   = "hello from wing php";
    $headers            = [
        "HTTP/1.1 200 OK",
        "Connection: Close",
        "Server: wing php ".WING_VERSION,
        "Date: " . gmdate("D,d M Y H:m:s")." GMT",
        "Content-Type: text/html",
        "Content-Length: " . strlen($response_content)
    ];

    $client->send( implode("\r\n",$headers)."\r\n\r\n".$response_content );*/
    $fileName = "D:/php/php-sdk/phpdev/vc11/x86/php-5.6.20/ext/wing/simples/beta/http/www/images/1.png";
    //$response_content   = file_get_contents("D:/php/php-sdk/phpdev/vc11/x86/php-5.6.20/ext/wing/simples/beta/http/www/images/1.png") ;
    $size = filesize("D:/php/php-sdk/phpdev/vc11/x86/php-5.6.20/ext/wing/simples/beta/http/www/images/1.png");

    $handle=fopen($fileName,"r");//使用打开模式为r

    $response_content=fread($handle,$size);//读为二进制


    $headers            = [
        "HTTP/1.1 200 OK",
        "Connection: Close",
        "Server: wing php ".WING_VERSION,
        "Date: " . gmdate("D,d M Y H:m:s")." GMT",
        "Content-Type: image/png",
        "Content-Length: ". $size
    ];


    echo implode("\r\n",$headers)."\r\n\r\n".$response_content;
    echo "================================================================\r\n\r\n";
    //输出信息到http请求页面
    $client->send( implode("\r\n",$headers)."\r\n\r\n".$response_content);

});

$server->on( "onsend" , function( $client , $send_status ){
    echo $client->socket;
    if( $send_status ) echo "发送成功";
    else echo "发送失败";
    echo "\r\n";
});
$server->on( "onconnect",function( $client ) {
    echo "===============>",$client->socket," connect\r\n";
});

$server->on( "onclose",function( $client ) {
    echo $client->socket," close \r\n";
});

$server->on( "onerror", function( $client, $error_code, $error_msg ) {
    echo $client->socket," some error happened,",$error_code,",",
    $error_msg, "\r\n";
});

$server->on( "ontimeout" , function( $client ) {
    echo $client->socket," is timeout\r\n";
});

$server->start();