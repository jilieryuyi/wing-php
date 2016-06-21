<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/21 6:48
 * @email 297341015@qq.com
 */
class Cookie{
    private static $cookie;
    private static $cookie_key = '';
    public static function set($key,$value){
        self::$cookie[$key] = $value;
    }
    public static function get($key){
        return self::$cookie[$key];
    }
    public static function getAll(){
        return self::$cookie;
    }
    public static function setCookie($cookie){
        self::$cookie = $cookie;
        $_COOKIE = $_SESSION = $cookie;
    }
    public static function clearCookie(){
        self::$cookie = [];
        self::$cookie_key = '';
    }
    public static function setKey($key){
        self::$cookie_key = $key;
    }
    public static function getKey(){
        return self::$cookie_key;
    }
    /*private static  function createUnique(){
        $randcode = "";
        while(strlen($randcode)<64){
            $md5 = md5(chr(rand(0,127)).chr(rand(0,127)).chr(rand(0,127)).chr(rand(0,127)));
            $randcode.=substr($md5,rand(0,strlen($md5)-8),8);
        }
        return $randcode."_".time();
    }*/
}