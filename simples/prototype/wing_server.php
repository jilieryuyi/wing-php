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
    private $active_timeout = 0;
    public function __construct(
        $listen_ip,
        $port,
        $max_connect = 1000,
        $timeout = 0 ,        //发送和接收数据的超时时间 单位为毫秒 0为永不超时
        $active_timeout = 0 , //检测很长时间没有活动的 socket 多长时间没有活动提醒 单位毫秒 0为永不超时 ontimeout事件
        $tick = 0             //时钟 如果启用 请设置大于零的值 如1000 毫秒 表示 1秒提醒一次 ontick事件
    )
    {
    }

    //注册回调事件
    public function on($event,$callback){}
    //启动服务
    public function start(){}
}