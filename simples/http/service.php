<?php
/**
 * @author yuyi
 * @created 2016/6/5 8:59
 * @email 297341015@qq.com
 * @执行 php service.php 启动服务 默认9998
 * @打开浏览器 访问 http://127.0.0.1:9998/
 * @暂不支持 100-continue
 * @支持原生的$_SERVER $_GET $_POST $_REQUEST 和 $_SESSION
 */

date_default_timezone_set("PRC");
require_once "service/Http.php";
define("HOME_PATH",__DIR__);

//网站配置
$config      = require_once "config/web.php";
$http        = new \Service\Http( $config );

//重写header函数 使header函数也能正常设置header
wing_override_function(
    "header",
    '$header',
    'global $http; $http->setHeaders($header);'
);

$memory_used = wing_get_memory_used();
echo "启动占用：",$memory_used/1024,"k=",$memory_used/1024/1024,"M\r\n";
$http->start();