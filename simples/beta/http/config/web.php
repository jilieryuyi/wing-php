<?php
/**
 * @author yuyi
 * @created 2016/8/7 13:34
 * @email 297341015@qq.com
 */
return  [
    "document_root"     => HOME_PATH."/www", //把这个路径修改为您的个人网站试试
    "index"             => "index.html index.htm index.php",
    "server_name"       => "www.wingphp.com",
    "port"              => 9998,
    "listen"            => "0.0.0.0",
    "max_connect"       => 1000 ,
    "error_log"         => HOME_PATH."/log/wing_error.log",
    "cookie"            => HOME_PATH."/cookie"
];