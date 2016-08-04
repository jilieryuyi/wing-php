<?php
/**
 * @author yuyi
 * @created 2016/8/4 10:32
 * @email 297341015@qq.com
 * @使用select模型的tcp服务器
 */
$server = new wing_select_server( "0.0.0.0" , 9998 , 20000, 1000, 3000, 1000 );
$server->on( "onreceive" , function( $client , $recv_msg ) {
    echo "recv from:",$client->socket,"=>",$recv_msg,"\r\n";
    // $client->send( "hello client\r\n" );

    $response_content   = "hello from wing php";
    $headers            = [
        "HTTP/1.1 200 OK",
        "Connection: Close",
        "Server: wing php ".WING_VERSION,
        "Date: " . gmdate("D,d M Y H:m:s")." GMT",
        "Content-Type: text/html",
        "Content-Length: " . strlen($response_content)
    ];

    //输出信息到http请求页面
    $client->send( implode("\r\n",$headers)."\r\n\r\n".$response_content );
});
$server->on( "onsend" , function( $client , $send_status ){
    //http协议 发送完毕之后 close掉
    echo $client->socket;
    if( $send_status ) echo "发送成功";
    else echo "发送失败";
    $client->close();
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
$server->on("tick" , function() {
    echo "on tick\r\n";
});
$server->start();