<?php namespace Service;
/**
 * @author yuyi
 * @created 2016/6/3 21:55
 * @email 297341015@qq.com
 */
class Http{
    private $config = [
        "port" => 6998,
        "listen" => "0.0.0.0"
    ];
    private $callbacks = ["onconnect","onclose","onerror"];


    private $content_len        = 0;
    private $http_post_split    = '';
    public function parse($body,$mime="text/html")
    {

        $headers = [
            "HTTP/1.1 200 OK",
            "Server: wing php/1.0",
            "Date: " . date("D,d M YH:m:s e"),
            "Connection: Keep-Alive",
            "Content-Length: " . strlen($body),
            "Content-Type: $mime"
        ];

        return implode("\r\n",$headers)."\r\n\r\n".$body;
    }



    public function registerCallback($callback_key,$callback_func){
        $this->config[$callback_key] = $callback_func;
    }
    private function get($client,$msg){
        //解析http get协议
        $msgs_spalit    = explode("\r\n",$msg);
        $_get_tab       = array_shift($msgs_spalit);
        $get_tabs       = explode(" ",$_get_tab);
        $get            = parse_url($get_tabs[1]);
        $get["path"]    = trim($get["path"]);
        $path           = trim($get["path"],"/");

        if($path == "")
            $path ="index.php";

        $mime_type="text/html";

        if(!file_exists($path))
            $path = "404.html";

        if(class_exists("finfo")) {
            $fi = new \finfo(FILEINFO_MIME_TYPE);
            $mime_type = $fi->file($path);
            unset($fi);
        }

        if($mime_type != "text/x-php") {
            //其他资源
            $file = file_get_contents($path);
            $headers = [
                "HTTP/1.1 200 OK",
                "Server: wing php/1.0",
                "Date: " . date("D,d M YH:m:s e"),
                "Connection: Keep-Alive",
                "Content-Length: " . strlen($file),
                "Content-Type: $mime_type"
            ];

            wing_socket_send_msg($client, implode("\r\n", $headers) . "\r\n\r\n" . $file);
            wing_close_socket($client);

            unset($msgs_spalit,$_get_tab,$get_tabs,$get,$path,
                $mime_type,$file,$headers,$client,$msg);
            unset($_GET,$_POST,$_REQUEST);
            return;
        }else{
            $mime_type="text/html";
        }

        isset($get["query"]) && parse_str($get["query"],$_GET);
        $_REQUEST = array_merge($_GET,$_REQUEST);

        ob_start();
        require $path;
        $content = ob_get_contents();
        ob_end_clean();

        $headers = [
                "HTTP/1.1 200 OK",
                "Server: wing php/1.0",
                "Date: " . date("D,d M YH:m:s e"),
                "Connection: Keep-Alive",
                "Content-Length: " . strlen($content),
                "Content-Type: $mime_type"
        ];

        wing_socket_send_msg($client, implode("\r\n", $headers) . "\r\n\r\n" . $content);
        wing_close_socket($client);
        unset($content,$msgs_spalit,$_get_tab,$get_tabs,$get,$path,
            $mime_type,$file,$headers,$client,$msg);
        unset($_GET,$_POST,$_REQUEST);
    }

    private function post($client,$msg){

        $headers        = explode("\r\n\r\n",$msg);
        $headers        = explode("\r\n",$headers[0]);

        $content        = strpos($msg,"--") == 0 ? $msg:substr($msg,strpos($msg,"\r\n\r\n"));

        $_get_tab       = array_shift($headers);
        $get_tabs       = explode(" ",$_get_tab);
        $get            = [];
        $path           = '';

        isset($get_tabs[1]) && $get         = parse_url($get_tabs[1]);
        isset($get["path"]) && $get["path"] = trim($get["path"]);
        isset($get["path"]) && $path        = trim($get["path"],"/");

        if($path == "")
            $path ="index.php";

        if(!file_exists($path))
            $path = "404.html";

        $mime_type="text/html";

        if(class_exists("finfo")) {
            $fi = new \finfo(FILEINFO_MIME_TYPE);
            $mime_type = $fi->file($path);
            unset($fi);
        }

        if($mime_type != "text/x-php") {
            //其他资源
            $file = file_get_contents($path);
            $_headers = [
                "HTTP/1.1 200 OK",
                "Server: wing php/1.0",
                "Date: " . date("D,d M YH:m:s e"),
                "Connection: Keep-Alive",
                "Content-Length: " . strlen($file),
                "Content-Type: $mime_type"
            ];

            wing_socket_send_msg($client, implode("\r\n", $_headers) . "\r\n\r\n" . $file);
            //usleep(10000);
            wing_close_socket($client);

            unset($headers,$content,$_get_tab,$get_tabs,$get,$path,
                $mime_type,$file,$headers,$client,$msg,$_headers);
            unset($_GET,$_POST,$_REQUEST);
            return;
        }else{
            $mime_type="text/html";
        }

        //Content-Type:
        foreach($headers as $header){
            if(strpos($header,"Content-Type") === 0){
                $temp = explode("boundary=",$header);
                isset($temp[1]) && $this->http_post_split = $temp[1];
                unset($temp);
            }
        }
        preg_match("/Content-Length:(.*)\r\n/",$msg,$out);
        isset($out[1]) && $this->content_len = trim($out[1]);
        unset($out);

        //100 响应
        //100 Continue
        //客户端应当继续发送请求。这个临时响应是用来通知客户端它的部分请求已经被服务器接收，
        //且仍未被拒绝。客户端应当继续发送请求的剩余部分，或者如果请求已经完成，忽略这个响应。
        //服务器必须在请求完成后向客户端发送一个最终响应。
        if( strlen($content) >= $this->content_len ){
            if(strpos($content,"--")!==0) {
                parse_str($content, $_POST);
                $_REQUEST = array_merge($_POST, $_REQUEST);
            }else {
                $forms = explode("--" . $this->http_post_split, $content);
                array_pop($forms);
                array_shift($forms);

                foreach ($forms as $_f) {
                    $_temp = explode("\r\n\r\n", $_f);
                    preg_match("/\"(.*)\"/", $_temp[0], $_out);
                    $_POST[$_out[1]] = trim($_temp[1], "\r\n");
                    unset($_temp);
                }
                $_REQUEST = array_merge($_POST, $_REQUEST);
                unset($forms);
            }

            ob_start();
            require $path;
            $__content = ob_get_contents();
            ob_end_clean();

            $_headers = [
                "HTTP/1.1 200 OK",
                "Server: wing php/1.0",
                "Date: " . date("D,d M YH:m:s e"),
                "Connection: Keep-Alive",
                "Content-Length: " . strlen($__content),
                "Content-Type: $mime_type"
            ];

            wing_socket_send_msg($client, implode("\r\n", $_headers) . "\r\n\r\n" .$__content);
            //usleep(10000);
            wing_close_socket($client);
            unset($__content);
        }

        unset($headers,$content,$_get_tab,$get_tabs,$get,$path,
            $mime_type,$file,$headers,$client,$msg,$_headers);
        unset($_GET,$_POST,$_REQUEST);
    }
    public function onreceive($client,$msg){
        //echo "onreceive==>",$msg,"\r\n\r\n";
        $_GET = $_POST = $_REQUEST = [];
        $http   = trim(strtolower(substr($msg,0,4)));
        if(!in_array($http,["get","post"]))
            $http = 'post';
        call_user_func_array([$this,$http],[$client,$msg]);
        unset($http,$client,$msg);
    }
    public function start(){
        $_self = $this;
        $params["onreceive"]    = function($client,$msg) use($_self){
            $_self->onreceive($client,$msg);
        };
        $params["onconnect"]    = function($client){};
        $params["onclose"]      = function($client){
           // $info = wing_socket_info($client);
           // file_put_contents(__DIR__."/onclose.log",json_encode($info)."\r\n\r\n",FILE_APPEND);
        };
        $params["onerror"]      = function($error_code){};
        /*$params["call_cycle"]   = function(){

            global $ms;
            echo   "more：",(memory_get_usage()-$ms)/1024,"k\r\n";

        };*/
        $params["port"]         = $this->config["port"];
        $params["listen"]       = $this->config["listen"];
        //$params["timeout"]      = 500;//500毫秒超时
        register_shutdown_function(function(){
            wing_service_stop();
        });
        wing_service($params);
    }
}