<?php
/**
 * @author yuyi
 * @created 2016/6/5 8:59
 * @email 297341015@qq.com
 * @执行 php service.php 启动服务
 * @打开浏览器 访问 http://127.0.0.1:6998/
 */

require_once "service/Response.php";
require_once "service/Http.php";
define("HOME_PATH",__DIR__);

//网站配置
$web_config = [
    "document_root"     => HOME_PATH."/www",
    "index"             => "index.html index.htm index.php",
    "404"               =>  HOME_PATH."/www/404.html",
];

$server_config = [
    "port"              => 6998,
    "listen"            => "0.0.0.0",
    "max_connect"       => 1000 ,
    "error_log"         => HOME_PATH."/log/wing_error.log",
];

$response               = new \Service\Response($web_config);
$http                   = new \Service\Http($response);

//全部配置均有默认值 调用此api会覆盖默认的配置
$http->setConfig($server_config);

echo "启动占用：",wing_get_memory_used()/1024,"k=",wing_get_memory_used()/1024/1024,"M\r\n";
$http->start();