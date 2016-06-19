<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/19 12:37
 * @email 297341015@qq.com
 */
class Response{
    //是否为get请求
    private $is_get     = false;
    //是否为post请求
    private $is_post    = false;
    private $headers    = [];
    private $http_protocols = '';

    private $default_root_path = HOME_PATH.'/www';//"D:/web/yonglibao/bigbluewhare";
    private $document_index = 'index.html index.htm index.php';
    private $not_fund_404 = '404.html';

    private $response_file = '';
    private $response_mime_type = '';

    private $http_server_name = 'wing php '.WING_VERSION                ;
    private $last_error = '';
    private $support = [ "get","post" ];
    private $include_mime_type = ["text/x-php","text/html"];
    private $php_mime_type = "text/x-php";
    private $default_mime_type = "text/html";
    private $response_http_status = "200 OK";
    private $is_http_100continue = false;

    public function __construct()
    {
        //设置工作目录
        chdir($this->default_root_path);
    }

    public function getHeaders(){
        //获取请求头信息
        return $this->headers;
    }

    public function setLastError($error){
        $this->last_error = $error;
    }
    public function getLastError(){
        return $this->getLastError();
    }
    public function hasError(){
        return $this->last_error != '';
    }

    //获取请求的资源文件绝对路径
    public function getPath( $_request_path ){

        if( $_request_path != "/" ) {
            $path = $this->default_root_path . $_request_path;
            if( !file_exists($path) ){
                //404
                return str_replace("\\","/",$this->default_root_path."/".$this->not_fund_404);
            }
            return str_replace("\\","/",$path);
        }

        $indexs = explode(" ", $this->document_index);
        $path = '';
        foreach ( $indexs as $index ) {
            $_file = $this->default_root_path."/".trim($index);
            if( file_exists($_file)) {
                $path = $_file;
                break;
            }
        }

        if( $path == '' ) {
            //404
            return str_replace("\\","/",$this->default_root_path."/".$this->not_fund_404);
        }
        return str_replace("\\","/",$path);
    }

    //获取请求资源的mime type
    public function getMimitype( $path ){
        $mime_type="text/html";

        if(class_exists("finfo")) {
            $fi = new \finfo(FILEINFO_MIME_TYPE);
            $mime_type = $fi->file($path);
            unset($fi);
            return $mime_type;
        }

        if( function_exists("mime_content_type")) {
            $mime_type = mime_content_type($path);
            return $mime_type;
        }
        return $mime_type;
    }

    //解析http协议
    public function explain($http_reponse){
        $service = strtolower( trim(substr($http_reponse,0,4)) );

        //得到协议 get 或者 post 一般http头 前三位|四位就是
        //如果不是 get或者post 直接返回错误 不支持
        /*if ( !in_array($service,$this->support )) {
            $this->setLastError("not support service");
            return;
        }*/

        //设置是否为get或者post
        $this->is_get   = $service == "get";
        $this->is_post  = $service == "post";
        $this->http_protocols = $service;

        //http协议 header 和 body 之间
        //通过两个 \r\n\r\n 分割（两个回车符）
        $headers_content = explode("\r\n\r\n" , $http_reponse );
        $headers_tab = explode("\r\n" , $headers_content[0] );

        $_request_content = $headers_content[1] ;
        echo "request content:\r\n",$_request_content,"\r\n\r\n";

        //分析这行得到请求文件
        $service_tab = array_shift($headers_tab);
        //SERVER_PROTOCOL
        $_service_temp = explode(" ", $service_tab);
        array_shift($_service_temp);
        $_SERVER["SERVER_PROTOCOL"] = $_service_temp[1];

        $_SERVER["REQUEST_URI"] = $_service_temp[0];
        $_request = parse_url($_service_temp[0]);

        //得到响应的文件和类型
        $this->response_file = $_SERVER["SCRIPT_FILENAME"] = $this->getPath( $_request["path"] ); //请求的资源
        $this->response_mime_type = $this->getMimitype( $this->response_file );

        foreach ($headers_tab as $_header) {
            $_header_temp = explode(": ",$_header);
            $this->headers[$_header_temp[0]] = $_header_temp[1];

            $_server_key = "HTTP_".str_replace("-","_",strtoupper($_header_temp[0]));
            $_SERVER[$_server_key] = $_header_temp[1];
        }

        var_dump( $this->headers );
        var_dump( $_SERVER );

        //get查询信息解析
        isset( $_request["query"] ) && parse_str( $_request["query"] ,$_GET );
        $_REQUEST = array_merge($_GET,$_REQUEST);

        //post查询信息解析
        if( $this->is_post ) {
            !empty($_request_content) && parse_str($_request_content, $_POST);
            $_REQUEST = array_merge($_POST, $_REQUEST);
        }
    }
    public function output( $client ){
        //暂不支持 100-continue
        echo $this->response_file,"\r\n";

        if( !in_array( $this->response_mime_type, $this->include_mime_type )) {
            $content = file_get_contents($this->response_file);
        }else {
            ob_start();
            require $this->response_file;
            $content = ob_get_contents();
            ob_end_clean();
        }

        $mime_type = $this->response_mime_type;
        if( $mime_type == $this->php_mime_type )
            $mime_type = $this->default_mime_type;

        $headers = [
            "HTTP/1.1 ".$this->response_http_status,
            "Server: ".$this->http_server_name,
            "Date: " . gmdate("D,d M Y H:m:s")." GMT",
            "Connection: Close",
            "Content-Type: ".$mime_type,
            "Content-Length: " . strlen($content)
        ];



        $ouput_content = implode("\r\n", $headers) . "\r\n\r\n" . $content;

        var_dump($_GET,$_POST,$_REQUEST);

        wing_socket_send_msg($client, $ouput_content);
        wing_close_socket($client);

    }

}