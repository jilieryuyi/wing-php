<?php
/**
 * @author yuyi
 * @created 2016/6/22 15:32
 * @email 297341015@qq.com
 * @wing_server 类原型
 */
class wing_server{
    //事件
    private $onreceive = null;
    private $onconnect = null;
    private $onclose = null;
    private $onerror = null;
    //端口和监听ip地址 默认值为 6998 和 0.0.0.0 最大连接数默认值为 1000
    private $port=6998;
    private $listen="0.0.0.0";
    private $max_connect = 1000;
    public function __construct(
        $listen_ip='0.0.0.0',
        $port=6998,
        $max_connect=1000,
        $timeout=0/*发送和接收数据的超时时间*/
    )
    {
    }

    //注册回调事件
    public function on($event,$callback){}
    //启动服务
    public function start(){}
}