<?php
/**
 * @author yuyi
 * @created 2016/6/1 6:39
 * @email 297341015@qq.com
 */

// 建立客户端的socet连接
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
//连接服务器端socket
$connection = socket_connect($socket, '127.0.0.1', 6998);
//sleep(5);
$c =0;
while(1)
 {

     file_get_contents("http://127.0.0.1:6998/?id=12asdgfsdfgsdfg%20asdf%20asf24%20234");
//要发送到服务端的信息。
   $send_data = "This data will Send to server!";
    if (!socket_write($socket, "$send_data\n")) {
       // file_put_contents("wing_service_client_send_error.log", $c."\r\n",FILE_APPEND);
  //    //  break;
    }
    $c++;

     //echo $c,"\n";
    // if ($buffer = socket_read($socket, 1024, PHP_NORMAL_READ)) {
     //    file_put_contents("wing_service_client_recv.log", $buffer . "\r\n",FILE_APPEND);
    // }
     if($c>100)break;
}
