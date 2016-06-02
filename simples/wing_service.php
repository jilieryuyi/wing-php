<?php
/**
 * @author yuyi
 * @created 2016/6/1 6:37
 * @email 297341015@qq.com
 */
$rc = 0;
$onreceive = function($client,$msg){
    global $rc;
    file_put_contents("wing_service_onrecv.log",$rc."、".$msg."\r\n"
        ,FILE_APPEND);
    wing_socket_send_msg($client,"2、hello client\n");
    $rc++;
};
$onconnect = function($client){
    file_put_contents("wing_service_oneonnect.log",$client."\r\ninfo:".
        json_encode(wing_socket_info($client))."\r\n"
        ,FILE_APPEND);
    wing_socket_send_msg($client,"1、hello client\n");
};
$onclose = function($client){
    file_put_contents("wing_service_onclose.log",$client."\r\ninfo:".
        json_encode(wing_socket_info($client))."\r\n"
        ,FILE_APPEND);

};
$onerror = function($client,$error){
    file_put_contents("wing_service_onerror.log",$client."\r\ninfo:".
        json_encode(wing_socket_info($client))."\r\n".
        "error:".$error."\r\n\r\n"
        ,FILE_APPEND);
};
wing_service($onreceive,$onconnect,$onclose,$onerror);