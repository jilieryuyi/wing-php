<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/19 12:37
 * @email 297341015@qq.com
 */
class Response{

    private $web_config;
    private $headers  = [];

    public function __construct($web_config)
    {
        $this->web_config = $web_config;
        $this->preResponse();
    }

    public function setHeaders($headers){
        if(is_array($headers)) {
            foreach( $headers as $key => $header ) {
                $this->headers[] = ucfirst($key).": ".ucfirst($header);
            }
            return;
        }
        $this->headers[] = $headers;
    }
    private function preResponse(){
        chdir($this->web_config["document_root"]);
        $this->setHeaders("Server: wing php ".WING_VERSION);
        $this->setHeaders("Date: " . gmdate("D,d M Y H:m:s")." GMT");
    }

    //获取请求的资源文件绝对路径
    private function getPath( $_request_path ){

        if( $_request_path != "/" ) {
            $path = $this->web_config["document_root"] . $_request_path;
            if( !file_exists($path) )
            {
                //404
                return str_replace("\\","/",$this->web_config["404"]);
            }
            return str_replace("\\","/",$path);
        }

        $indexs = explode(" ", $this->web_config["index"]);
        $path   = '';

        foreach ( $indexs as $index ) {
            $_file = $this->web_config["document_root"]."/".trim($index);
            if( file_exists($_file)) {
                $path = $_file;
                break;
            }
        }

        if( $path == '' )
        {
            //404
            return str_replace("\\","/",$this->web_config["404"]);
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

    private function cookieParse(){

    }
    private function requestParse($get_request,$post_request){
        $_GET = $get_request;
        $_POST = $post_request;
        $_REQUEST = array_merge($_GET,$_POST);
    }
    private function serverParse( $headers ){
        foreach ($headers as $key => $header ){
            $_SERVER["HTTP_".strtoupper(str_replace("-","_",$key))] = $header;
        }
    }



    public function output( $http_request_info ){
        //暂不支持 100-continue
        $_GET = $_POST = $_SERVER = $_COOKIE = $_REQUEST = [];
        $this->headers = [
            "HTTP/1.1 200 OK",
            "Connection: Close"
        ];

        $this->requestParse( $http_request_info["http_get_query"],$http_request_info["http_post_query"] );
        $this->serverParse( $http_request_info["http_headers"] );
        //请求文件
        $http_request_file  = $this->getPath( $http_request_info["http_request_file"] );
        $response_mime_type = $this->getMimitype( $http_request_file );

        $response_content   = '';
        if( !in_array( $response_mime_type, ["text/x-php","text/html"] )) {
            $response_content = file_get_contents( $http_request_file );
        }else {
            ob_start();
            include $http_request_file;
            $response_content = ob_get_contents();
            ob_end_clean();
        }


        if( $response_mime_type == "text/x-php" )
            $response_mime_type = "text/html";


        $this->setHeaders("Content-Type: ".$response_mime_type);
        $this->setHeaders("Content-Length: " . strlen($response_content) );

        unset($_GET , $_POST , $_SERVER , $_COOKIE , $_REQUEST);
        return implode("\r\n", $this->headers) . "\r\n\r\n" . $response_content;
    }

}