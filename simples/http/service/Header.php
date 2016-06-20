<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/20 20:53
 * @email 297341015@qq.com
 */
class Header{
    public static $response;
    public static function set($headers){
        self::$response->setHeaders($headers);
    }
}