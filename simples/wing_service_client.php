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
//要发送到服务端的信息。
$send_data = "This data will Send to server!";
//客户端去连接服务端并接受服务端返回的数据，如果返回的数据保护not connect就提示不能连接。
while ($buffer = @socket_read($socket, 1024, PHP_NORMAL_READ)) {
    if (preg_match("/not connect/",$buffer)) {
        echo "don`t connect\n";
        break;
    } else {
        //服务端传来的信息
        echo "Buffer Data: " . $buffer . "\n";
        echo "Writing to Socket\n";
        // 将客户的信息写到通道中，传给服务器端
        if (!socket_write($socket, "$send_data\n")) {
            echo "Write failed\n";
        }
        //服务器端收到信息后，客户端接收服务端传给客户端的回应信息。
        while ($buffer = socket_read($socket, 1024, PHP_NORMAL_READ)) {
            echo "sent to server:$send_data\n response from server was:" . $buffer . "\n";
        }

    }
}