<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/3 21:55
 * @email 297341015@qq.com
 * @http服务器
 */
class Http{

    private $config = [
        "document_root"     => HOME_PATH."/www", //把这个路径修改为您的个人网站试试
        "index"             => "index.html index.htm index.php",
        "server_name"       => "www.wingphp.com",
        "port"              => 9998,
        "listen"            => "0.0.0.0",
        "max_connect"       => 1000 ,
        "timeout"           => 3000 ,
        "active_timeout"    => 3000 ,
        "error_log"         => HOME_PATH."/log/wing_error.log",
        "cookie"            => HOME_PATH."/cookie",
        "debug"             => true
    ];

    private $request_mode;         //请求方式 get post delete put options
    private $request_file;         //请求的文件
    private $request_query;        //查询
    private $http_version;         //http协议版本
    private $http_accept = [];     //可解析的content-type类型
    private $headers     = [];     //所有注册的headers
    private $connection  = "close";//connection close或者keep-alive
    private $cookies     = [];
    private $cookie_key;

    /**
     * @构造函数 传入配置覆盖默认配置
     */
    public function __construct( array $configs = [] )
    {
        if( isset( $this->config["debug"] ) && $this->config["debug"] )
            error_reporting(E_ALL);
        foreach ( $configs as $key => $config ) {
            $this->config[$key] = $config;
        }
    }

    private function parseCookie( $cookie_str ) {
        $_temp = explode(";",$cookie_str);
        //WINGPHPSESSID=a53tchtk9su5v8a1n0ssff67r3;
        $cookie_key = '';
        foreach( $_temp as $item ){
            $item = trim($item);
            if( strpos($item,"WINGPHPSESSID=") === 0 ){
                $phpcookie  = explode("=",$item);
                $cookie_key = $phpcookie[1];
                break;
            }
        }
        if( !$cookie_key )
            return;
        //解析还原cookie实现$_SESSiON
        if( isset( $this->cookies[ $cookie_key ] ) ) {
            $_SESSION = $this->cookies[ $cookie_key ];
        }/*else if( file_exists($this->config["cookie"]."/".$cookie_key.".cookie") ){
            $cookie_json = file_get_contents( $this->config["cookie"]."/".$cookie_key.".cookie");
            $_SESSION    = json_decode( $cookie_json, true );
        }*/
        $this->cookie_key = $cookie_key;

    }

    private function parseAccept( $accept_str ) {
        $this->http_accept = explode( ",", $accept_str );
    }
    private function parseConnection( $connection_str ) {
        $this->connection = $connection_str;
    }


    /**
     * @获取请求的资源文件绝对路径
     */
    private function getRealPath( $_request_path ){

        $document_root  = $this->config["document_root"];
        $not_fund       = isset( $this->config["404"] ) ? $this->config["404"] : "";
        $index          = $this->config["index"];

        //请求的不是根路径
        if( $_request_path != "/" ) {
            $path = $document_root . $_request_path;
            if( file_exists( $path ) )
            {
                return str_replace("\\","/",$path);
            }
            else if( $not_fund )//404
            {
                $path = str_replace("\\","/",$not_fund);
                if( file_exists( $path ) )
                    return str_replace("\\","/",$path);
            }
            else
            {
                return "";
            }

        }

        //请求的是根路径 根据配置的index得到请求文件
        $indexs = explode(" ", $index);
        foreach ( $indexs as $index ) {
            $_file = $document_root."/".trim($index);
            if( file_exists($_file) ) {
                return str_replace("\\","/",$_file);
            }
        }


        //如果文件不存在 返回配置为 404 文件
        if( $not_fund )//404
        {
            $path = str_replace("\\","/",$not_fund);
            if( file_exists($_file) )
                return str_replace("\\","/",$path);
        }

        return "";

    }

    //获取请求资源的mime type
    public function getMimitype( $path ){
        $mime_type = "text/html";

        if( !class_exists("finfo") && !function_exists("mime_content_type") ) {
            echo "Warning : both class \"finfo\" not fund and function \"mime_content_type\" does not support";
        }

        if( class_exists("finfo") ) {
            $fi = new \finfo(FILEINFO_MIME_TYPE);
            $mime_type = $fi->file($path);
            unset($fi);
            return $mime_type;
        }

        if( function_exists("mime_content_type") ) {
            $mime_type = mime_content_type($path);
            return $mime_type;
        }
        return $mime_type;
    }
    /**
     * @设置header，支持数组和字符串
     */
    public function setHeaders( $headers ) {
        if( is_array( $headers ) ) {
            foreach( $headers as $key => $header ) {
                $this->headers[] = ucfirst($key).": ".ucfirst($header);
            }
            return;
        }
        $this->headers[] = $headers;
    }

    private  function createUnique(){
        $randcode = "";
        while(strlen($randcode)<64){
            $md5 = md5(chr(rand(0,127)).chr(rand(0,127)).chr(rand(0,127)).chr(rand(0,127)));
            $randcode.=substr($md5,rand(0,strlen($md5)-8),8);
        }
        return $randcode."_".time();
    }
    /**
     * @收到消息，解析http协议并响应请求
     */
    public function onreceive( $http_client, $http_msg ) {

        //重置一些全局对象
        $_SERVER  =
        $_GET     =
        $_POST    =
        $_COOKIE  =
        $_SESSION =
        $_REQUEST =
        $this->headers = [];
        $this->cookie_key = '';



        $msgs        = explode("\r\n\r\n" , $http_msg );           //http协议 header 和 body 之间 通过两个 \r\n\r\n 分割（两个回车符）
        $headers_str = array_shift( $msgs );                       //请求过来的headers字符串，接下来要对这个字符串逐一解析
        $content_str = $msgs[0];                                   //请求过来的消息实体内容 get传输的话 一般都为空

        $request_headers     = explode("\r\n" , $headers_str );    //得到所有的headers数组
        $_request_mode_str   = array_shift( $request_headers );    //得到请求方式/请求文件/http协议版本行
        $_request_mode       = explode( " ", $_request_mode_str ); //解析得到请求方式/请求文件/http协议版本行
        $this->request_mode  = strtolower( $_request_mode[0] );    //http请求方式 get post
        $_request_query      = parse_url($_request_mode[1]);       //解析请求文件行 得到请求文件和get查询
        $this->request_file  = $_request_query["path"];            //请求的文件
        $this->request_query = isset( $_request_query["query"]) ? $_request_query["query"] : "";
                                                                   //get参数
        $this->http_version  = $_request_mode[2];                  //http协议版本
        $http_request_file   = $this->getRealPath( $this->request_file );
                                                                   //http请求的文件绝对路径
        //解析其他headers
        foreach ( $request_headers as $_header ) {

            $_header_temp  = explode(": ",$_header);
            $_header_key   = strtolower( $_header_temp[0] );
            $_header_value = $_header_temp[1];
            $parseFunc     = "parse".ucfirst($_header_key);

            //独立解析，某个header 回调函数 ， 如果有注册回调函数的话
            if( is_callable( [ $this, $parseFunc ] ) ) {
                call_user_func( [ $this,$parseFunc ], $_header_value );
            }

            $_SERVER["HTTP_".strtoupper( str_replace( "-","_",$_header_key ) )] = $_header_temp[1];
        }

        //$_SERVER 全局变量初始化
        $_SERVER["SERVER_PORT"]     = $this->config["port"];
        $_SERVER["REQUEST_TIME"]    = time();
        $_SERVER["SERVER_PROTOCOL"] = $_request_mode[2];
        $_SERVER["SCRIPT_NAME"]     = $_request_query["path"];
        $_SERVER["PHP_SELF"]        = $_request_query["path"];
        $_SERVER["REQUEST_URI"]     = $_request_mode[1];
        $_SERVER["REQUEST_METHOD"]  = strtoupper( $_request_mode[0] );//http请求方式 get post
        $_SERVER["SCRIPT_FILENAME"] = $http_request_file;
        $_SERVER["DOCUMENT_ROOT"]   = str_replace("\\","/",$this->config["document_root"]);

        if( isset( $_request_query["query"] ) )
            parse_str( $_request_query["query"] ,$_GET );         //解析get参数

        if( !empty($content_str) )
            parse_str($content_str, $_POST);                      //解析post参数

        $_REQUEST = array_merge($_GET,$_POST);                    //合并get和post得到request

        $response_mime_type = $this->getMimitype( $http_request_file );
        $response_content   = '';

        if( !in_array( $response_mime_type, ["text/x-php","text/html"] ) ) {
            if( $http_request_file && file_exists( $http_request_file ) )
            {
                $response_content = file_get_contents( $http_request_file );
            }else{
                $response_content = "404 not fund";
            }
        }else {
            ob_start();
            if( $http_request_file && file_exists( $http_request_file ) ) {
                include $http_request_file;
                $response_content = ob_get_contents();
            }else{
                echo "404 not fund";
            }
            ob_end_clean();
        }

        if( $response_mime_type == "text/x-php" )
            $response_mime_type = "text/html";

        $this->setHeaders( [
            "Connection"     => $this->connection,
            "Server"         => "wing php ".WING_VERSION,
            "Date"           => date("Y-m-d H:i:s"),
            "Content-Type"   => $response_mime_type,
            "Content-Length" => strlen($response_content)
        ] );

        //如果$_SESSION不为空 需要设置一下cookie
        if( $_SESSION ) {
            $cookie_key = $this->cookie_key?$this->cookie_key:$this->createUnique();
            $this->setHeaders(["Set-Cookie" => "WINGPHPSESSID=" . $cookie_key ]);
            $this->cookies[ $cookie_key ] = $_SESSION;
           /* file_put_contents( $this->config["cookie"]."/".$cookie_key.".cookie",
                json_encode($_SESSION) );*/
        }


        $http_response_content =
            "HTTP/1.1 200 OK\r\n".
            implode("\r\n", $this->headers) . "\r\n\r\n" .
            $response_content;

        $http_client->send( $http_response_content );
    }
    public function start(){
        $_self  = $this;

        set_error_handler( function( $errno, $errstr, $errfile, $errline) use( $_self ) {
            $error =
                date("Y-m-d H:i:s")."==>\r\nerrorno:".$errno."\r\n".
                "errorsr:".$errstr."\r\nerrorfile:".$errfile."\r\n".
                "errorline:".$errline."\r\n\r\n";
            file_put_contents( $_self->config["error_log"] , $error, FILE_APPEND );
        },E_ALL);

        $server = new \wing_select_server(
                $this->config["listen"],
                $this->config["port"] ,
                $this->config["max_connect"],
                $this->config["timeout"],
                $this->config["active_timeout"]
            );

        $server->on( "onreceive" , function( $client , $recv_msg ) use( $_self ) {
            $_self->onreceive($client,$recv_msg);
        });

        $server->on( "onsend" , function( $client , $send_status ){

        });

        $server->on( "onconnect",function( $client ) {

        });

        $server->on( "onclose",function( $client ) {

        });

        $server->on( "onerror", function( $client, $error_code, $error_msg ) use($_self) {
            $error = date("Y-m-d H:i:s")."==>socket:".$client->socket.",code:".$error_code.",msg:".$error_msg."\r\n\r\n";
            file_put_contents( $_self->config["error_log"] , $error, FILE_APPEND );
        });

        $server->on( "ontimeout" , function( $client ) {

        });

        $server->start();

    }
}