<?php
/**
 * @author yuyi
 * @created 2016/6/22 7:31
 * @email 297341015@qq.com
 * @tcp 服务器
 */
$server = new wing_server("0.0.0.0",9998,10000);
$server->on("onreceive",function($client,$recv_meg){

    echo "recv from:",$client->socket,"=>",$recv_meg,"\r\n";
    $response_content = "hello from wing php";
    $headers = [
        "HTTP/1.1 200 OK",
        "Connection: Close",
        "Server: wing php ".WING_VERSION,
        "Date: " . gmdate("D,d M Y H:m:s")." GMT",
        "Content-Type: text/html",
        "Content-Length: " . strlen($response_content)
    ];

    //输出信息到http请求页面
    $client->send( implode("\r\n",$headers)."\r\n\r\n".$response_content );
    //直接关闭连接
    $client->close();
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