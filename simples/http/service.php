<?php
/**
 * @author yuyi
 * @created 2016/6/5 8:59
 * @email 297341015@qq.com
 * @执行 php service.php 启动服务
 * @打开浏览器 访问 http://127.0.0.1:6998/
 */
require_once "service/Http.php";
$http = new \Service\Http();
$http->start();