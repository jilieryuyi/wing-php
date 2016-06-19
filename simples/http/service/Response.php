<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/19 12:37
 * @email 297341015@qq.com
 */
class Response{

    private $web_config;

    public function __construct($web_config)
    {
        $this->web_config = $web_config;
        $this->preResponse();
    }

    private function preResponse(){
        chdir($this->web_config["document_root"]);
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

    private function getParse(){
        //解析生成 $_GET
    }
    private function postParse(){

    }
    private function cookieParse(){

    }
    private function requestParse(){

    }



    public function output( $http_request_info ){
        //暂不支持 100-continue
        $_GET = $_POST = $_SERVER = $_COOKIE = $_REQUEST = [];
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

        $headers = [
            "HTTP/1.1 200 OK",
            "Server: wing php ".WING_VERSION,
            "Date: " . gmdate("D,d M Y H:m:s")." GMT",
            "Connection: Close",
            "Content-Type: {$response_mime_type}",
            "Content-Length: " . strlen($response_content)
        ];

        unset($_GET , $_POST , $_SERVER , $_COOKIE , $_REQUEST);
        return implode("\r\n", $headers) . "\r\n\r\n" . $response_content;
    }

}