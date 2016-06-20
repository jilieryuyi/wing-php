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
        chdir($this->web_config["document_root"]);
    }

    //去掉重复的header
    public function clearHeaders(){
        $this->headers = array_flip($this->headers);
        $this->headers = array_flip($this->headers);
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

    //获取请求的资源文件绝对路径
    private function getPath( $_request_path ,$host = ''){

        $host && list( $host , $port )  =explode(":",$host);

        $document_root  = $this->web_config["document_root"];
        $not_fund       = $this->web_config["404"];
        $index          = $this->web_config["index"];

        if( isset( $this->web_config["virtual"] ) && is_array( $this->web_config["virtual"] ) &&
            count( $this->web_config["virtual"] ) >0 ){
            foreach ( $this->web_config["virtual"] as $virtual ){
                if( $host == $virtual["server_name"] ) {
                    $document_root  = $virtual["document_root"];
                    $not_fund       = $virtual["404"];
                    $index          = $virtual["index"];
                    break;
                }
            }
        }

        if( $_request_path != "/" ) {
            $path = $document_root . $_request_path;
            if( !file_exists($path) || !is_file($path) )//404
            {
                $path = str_replace("\\","/",$not_fund);
            }
            return str_replace("\\","/",$path);
        }

        $indexs = explode(" ", $index);
        $path   = '';

        foreach ( $indexs as $index ) {
            $_file = $document_root."/".trim($index);
            if( file_exists($_file) && is_file($_file) ) {
                $path = $_file;
                break;
            }
        }

        if( $path == '' )//404
        {
            $path = str_replace("\\","/",$not_fund);
        }
        return str_replace("\\","/",$path);
    }

    //获取请求资源的mime type
    public function getMimitype( $path ){

        $mime_type = "text/html";

        if( class_exists("finfo") ) {
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
        $_GET       = $get_request;
        $_POST      = $post_request;
        $_REQUEST   = array_merge($_GET,$_POST);
    }
    private function serverParse( $request , $http_request_file ){
        $headers        = $request["http_headers"];
        $server_config  = $request["http_server_config"];

        foreach ($headers as $key => $header ){
            $_SERVER["HTTP_".strtoupper(str_replace("-","_",$key))] = $header;
        }

        //这里要使用 PHPSESSID=a53tchtk9su5v8a1n0ssff67r3; 做session支持 暂未支持
        //"PHPSESSID=a53tchtk9su5v8a1n0ssff67r3;
        // _ga=GA1.2.659666694.1463541755;
        // Hm_lvt_c4fb630bdc21e7a693a06c26ba5651c6=1465352549,1465894583,1466069135,1466335394;
        // Hm_lpvt_c4fb630bdc21e7a693a06c26ba5651c6=1466335479
        $_SERVER["SERVER_PORT"]     = $server_config["port"];
        $_SERVER["DOCUMENT_ROOT"]   = $this->web_config["document_root"];
        $_SERVER["REQUEST_URI"]     = $request["http_request_uri"];
        $_SERVER["SCRIPT_NAME"]     = $_SERVER["PHP_SELF"] = $request["http_request_file"];
        $_SERVER["SCRIPT_FILENAME"] = $http_request_file;
        $_SERVER["REQUEST_TIME"]    = time();
    }



    public function output( $http_request_info ){
        //暂不支持 100-continue
        $_GET = $_POST = $_SERVER = $_COOKIE = $_REQUEST = [];
        $this->headers = [
            "HTTP/1.1 200 OK",
            "Connection: Close"
        ];

        $this->requestParse( $http_request_info["http_get_query"],$http_request_info["http_post_query"] );

        //请求文件 Host
        $http_request_file  = $this->getPath( $http_request_info["http_request_file"] ,$http_request_info["http_headers"]["Host"]);
        $this->serverParse( $http_request_info ,$http_request_file );

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

        $this->setHeaders("Server: wing php ".WING_VERSION);
        $this->setHeaders("Date: " . gmdate("D,d M Y H:m:s")." GMT");
        $this->setHeaders("Content-Type: ".$response_mime_type);
        $this->setHeaders("Content-Length: " . strlen($response_content) );

        unset($_GET , $_POST , $_SERVER , $_COOKIE , $_REQUEST);
        return implode("\r\n", $this->headers) . "\r\n\r\n" . $response_content;
    }

}