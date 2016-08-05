<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/20 20:53
 * @email 297341015@qq.com
 * @实现header的设置和修改
 * \Service\Header::set(["Anthor"=>"yuyi","Email"=>"297341015@qq.com"]);
 * \Service\Header::set("Version: ".WING_VERSION);
 */
class Header{
    public static $response;
    public static function set($headers){
        self::$response->setHeaders($headers);
        self::$response->clearHeaders();
    }
}