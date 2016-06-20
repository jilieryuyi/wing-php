<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/3 21:55
 * @email 297341015@qq.com
 * @http协议解析
 */
class Http{

    private $config = [
        "port"          => 6998,
        "listen"        => "0.0.0.0",
        "max_connect"   => 1000,
        "error_log"     => HOME_PATH."/log/wing_error.log",
    ];
    private $response;

    public function __construct($response)
    {
        $this->response = $response;
    }

    public function setConfig($configs){
        foreach ($configs as $key => $config){
            $this->config[$key] = $config;
        }
    }

    private function parseCookie($cookie_str){
        $_temp = explode(";",$cookie_str);
        //PHPSESSID=a53tchtk9su5v8a1n0ssff67r3;
        foreach( $_temp as $item ){
            $item = trim($item);
            if( strpos($item,"PHPSESSID=") === 0 ){
                $phpcookie = explode("=",$item);

                $cookie_file = $this->config["cookie"]."/".$phpcookie[1];
                if(file_exists($cookie_file)) {
                    $content = file_get_contents($cookie_file);
                    $cookie = json_decode($content,true);
                    Cookie::setCookie($cookie);
                    unlink($cookie_file);
                }
            }
        }
    }

    //解析http协议
    public function httpParse( $http_request_msg ,&$http_request_file ,&$_host){
        $_SERVER = $_GET = $_POST  = $_COOKIE = $_REQUEST = [];

        //http协议 header 和 body 之间
        //通过两个 \r\n\r\n 分割（两个回车符）
        $headers_content    = explode("\r\n\r\n" , $http_request_msg );

        //headers
        $headers_tab        = explode("\r\n" , $headers_content[0] );
        //content
        $request_content    = $headers_content[1] ;
        echo "request content:\r\n",$request_content,"\r\n\r\n";

        //分析这行得到请求文件
        $service_tab        = array_shift($headers_tab);
        //SERVER_PROTOCOL
        $_service_temp      = explode(" ", $service_tab);


        //协议 get|post
        $_SERVER["REQUEST_METHOD"]  =  strtoupper($_service_temp[0]);
        //得到http 1.1
        $_SERVER["SERVER_PROTOCOL"] = $_service_temp[2];

        //get查询信息
        $_request = parse_url($_service_temp[1]);

        //请求资源文件
        $http_request_file          = $_request["path"];
        $_SERVER["SCRIPT_NAME"]     = $_SERVER["PHP_SELF"] = $http_request_file;
        $_SERVER["REQUEST_URI"]     = $_service_temp[1];
        $_SERVER["REQUEST_TIME"]    = time();


        //get查询信息解析
        isset( $_request["query"] ) && parse_str( $_request["query"] ,$_GET );

        //post查询信息解析
        !empty($_request_content) && parse_str($_request_content, $_POST);
        $_REQUEST   = array_merge($_GET,$_POST);


        $headers        = [];
        foreach ( $headers_tab as $_header ) {
            $_header_temp = explode(": ",$_header);
            $headers[$_header_temp[0]] = $_header_temp[1];

            if( strtolower($_header_temp[0]) == "cookie" )
                $this->parseCookie($_header_temp[1]);

            $_SERVER["HTTP_".strtoupper(str_replace("-","_",$_header_temp[0]))] = $_header_temp[1];
        }
        $_host  = isset($headers["Host"]) ? $headers["Host"]:"";

        $_SERVER["SERVER_PORT"]     = $this->config["port"];

    }


    public function onreceive($http_client,$http_msg){
        echo $http_msg,"\r\n\r\n";

        $http_request_file      = '';
        $_host = '';
        //http 协议解析
         $this->httpParse( $http_msg ,$http_request_file , $_host);
        //构建网站输出
        $http_response_content  = $this->response->output( $http_request_file,$_host ,$this->config["cookie"]);

        //输出信息到http请求页面
        wing_socket_send_msg( $http_client, $http_response_content );
        //直接关闭连接
        wing_close_socket( $http_client );

    }
    public function start(){
        $_self = $this;
        $params["onreceive"]    = function($client,$msg) use($_self){
            $_self->onreceive($client,$msg);
        };
        $params["onconnect"]    = function($client, $client_ip, $client_port, $client_family, $client_sign_zero){
            //也可以通过 wing_socket_info($client) 得到连接进来的客户端相关信息
            //$info = wing_socket_info($client);
            //unset($info);
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
        $params["max_connect"]  = $this->config["max_connect"];
        register_shutdown_function(function(){
            wing_service_stop();
        });
        wing_service($params);
    }
}