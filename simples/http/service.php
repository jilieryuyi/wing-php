<?php
/**
 * @author yuyi
 * @created 2016/6/5 8:59
 * @email 297341015@qq.com
 * @执行 php service.php 启动服务 默认6998
 * @打开浏览器 访问 http://127.0.0.1:6998/
 */

require_once "service/Response.php";
require_once "service/Http.php";
require_once "service/Header.php";
require_once "service/Cookie.php";
define("HOME_PATH",__DIR__);

//cookie/session api
//\Service\Cookie::set("user","yuyi");
//echo \Service\Cookie::get("user");

//网站配置
$web_config = [
    "document_root"     => HOME_PATH."/www", //把这个路径修改为您的个人网站试试
    "index"             => "index.html index.htm index.php",
    "404"               =>  HOME_PATH."/www/404.html",
    "server_name"       => "www.wingphp.com"
];

$server_config = [
    "port"              => 6998,
    "listen"            => "0.0.0.0",
    "max_connect"       => 1000 ,
    "error_log"         => HOME_PATH."/log/wing_error.log",
    "cookie"            => HOME_PATH."/cookie"
];

$response               = new \Service\Response($web_config);
$http                   = new \Service\Http($response);

//全部配置均有默认值 调用此api会覆盖默认的配置
$http->setConfig($server_config);

//代替传统的设置header 您应该使用如
//\Service\Header::set(["Anthor"=>"yuyi","Email"=>"297341015@qq.com"]);
//\Service\Header::set("Version: ".WING_VERSION);
//\Service\Header::set api设置header
\Service\Header::$response = $response;

$memory_used = wing_get_memory_used();
echo "启动占用：",$memory_used/1024,"k=",$memory_used/1024/1024,"M\r\n";
$http->start();