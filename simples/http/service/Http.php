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

    //解析http协议
    public function httpParse( $http_request_msg ){
        $response           = [];

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
        $response["http_request_type"]  = $_service_temp[0];
        //得到http 1.1
        $response["http_protocols"]     = $_service_temp[2];

        //get查询信息
        $_request = parse_url($_service_temp[1]);

        //请求资源文件
        $response["http_request_file"]  = $_request["path"];


        $get_output     = [];
        //get查询信息解析
        isset( $_request["query"] ) && parse_str( $_request["query"] ,$get_output );
        $response["http_get_query"] = $get_output;

        //post查询信息解析
        $post_output    = [];
        !empty($_request_content) && parse_str($_request_content, $post_output);
        $response["http_post_query"] = $post_output;


        $headers        = [];
        foreach ( $headers_tab as $_header ) {
            $_header_temp = explode(": ",$_header);
            $headers[$_header_temp[0]] = $_header_temp[1];
        }
        $response["http_headers"] = $headers;

        return $response;
    }


    public function onreceive($http_client,$http_msg){
        echo $http_msg,"\r\n\r\n";

        //http 协议解析
        $http_parse_info        = $this->httpParse( $http_msg );
        //构建网站输出
        $http_response_content  = $this->response->output($http_parse_info);

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