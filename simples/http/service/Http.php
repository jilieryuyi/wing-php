<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/3 21:55
 * @email 297341015@qq.com
 */
class Http{
    private $config = [
        "port"      => 6998,
        "listen"    => "0.0.0.0",
        "output"    => HOME_PATH."/log/wing_http.log",
        "error_log" => HOME_PATH."/log/wing_error.log"
    ];
    private $response;
    public function __construct()
    {
        $this->response = new Response();
    }
    //输出重定向
    public function outputLog(){
        $handle = fopen($this->config["output"], "a");
        if ( $handle ) {
            unset($handle);
            @fclose(STDOUT);
            @fclose(STDERR);
            $STDOUT = $STDERR = fopen($this->config["output"], "a");
        }
    }

    public function onreceive($http_client,$http_msg){
        echo $http_msg,"\r\n\r\n";

        $_GET = $_POST = $_REQUEST = $_SERVER = [];

        $this->response->explain( $http_msg );
        $this->response->output( $http_client );
    }
    public function start(){
        $_self = $this;
        $params["onreceive"]    = function($client,$msg) use($_self){
            $_self->onreceive($client,$msg);
        };
        $params["onconnect"]    = function($client, $client_ip, $client_port, $client_family, $client_sign_zero){
            //也可以通过 wing_socket_info($client) 得到连接进来的客户端相关信息
            $info = wing_socket_info($client);
            unset($info);
        };
        $params["onclose"]      = function($client){
           //此处不可以通过 wing_socket_info($client) 获取客户端相关的信息 因为已经掉线了
        };
        $params["onerror"]      = function($client,$error_code,$last_error){
            $error_content = "{$client} some error happened:{$error_code},{$last_error}\r\n";
            file_put_contents($this->config["error_log"],$error_content,FILE_APPEND);
            echo $error_content;
        };
        $params["port"]         = $this->config["port"];
        $params["listen"]       = $this->config["listen"];
        //创建1000个备用socket 也就是最大并发数
        //也就是所谓的socket池概念 性能好 稳定
        $params["max_connect"]  = 1000;
        register_shutdown_function(function(){
            wing_service_stop();
        });
        wing_service($params);
    }
}