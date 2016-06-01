<?php
/**
 * @author yuyi
 * @created 2016/6/1 6:37
 * @email 297341015@qq.com
 */
$onreceive = function(){};
$onconnect = function($client){
    file_put_contents("wing_service_oneonnect.log",$client."\r\ninfo:".
        json_encode(wing_socket_info($client))."\r\n"
        ,FILE_APPEND);
};
wing_service($onreceive,$onconnect);