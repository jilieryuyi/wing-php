<?php
/**
 * @author yuyi
 * @created 2016/6/22 15:36
 * @email 297341015@qq.com
 * @wing_client 原型
 */
class wing_client{
    //客户端相关信息
    public $sin_addr = "";
    public $sin_port = 0;
    public $sin_family = 0;
    public $sin_zero = "";
    public $socket = 0;
    //发送消息
    public function send($msg){}
    //关闭连接
    public function close(){}
}