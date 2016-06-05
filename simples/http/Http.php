<?php
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
    public function onreceive($client,$msg){
        $_GET = [];$_POST = [];$_REQUEST = [];
        echo $msg,"\r\n\r\n";
        $http   = trim(strtolower(substr($msg,0,4)));
        if( $http == "get" ){
            /**
            GET /?id=12asdgfsdfgsdfg%20asdf%20asf24%20234 HTTP/1.1
            Host: 127.0.0.1:6998
            Connection: keep-alive
            Cache-Control: max-age=0
            Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,;q=0.8
            Upgrade-Insecure-Requests: 1
            User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.94 Safari/537.36
            Accept-Encoding: gzip, deflate, sdch
            Accept-Language: zh-CN,zh;q=0.8
             */
            //解析http get协议
            $msgs_spalit    = explode("\r\n",$msg);
            $_get_tab       = array_shift($msgs_spalit);
            $get_tabs       = explode(" ",$_get_tab);
            $get            = parse_url($get_tabs[1]);
            $get["path"]    =    trim($get["path"]);
            $path = trim($get["path"],"/");

            if($path == "")$path ="index.php";


            $mime_type="text/html";



            if(file_exists($path)){
                $fi = new finfo(FILEINFO_MIME_TYPE);
                $mime_type = $fi->file($path);
                //echo $mime_type; // image/jpeg

                echo "mime:",$mime_type,"\r\n";

                if($mime_type != "text/x-php") {
                    $fi = null;
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
                    //wing_close_socket($client);
                    return;
                }

                $mime_type="text/html";

            }



            /*if($get["path"] == "/favicon.ico"){
//image/x-icon
                $file = file_get_contents(trim($get["path"],"/"));
                $headers = [
                    "HTTP/1.1 200 OK",
                    "Server: wing php/1.0",
                    "Date: " . date("D,d M YH:m:s e"),
                    "Connection: Keep-Alive",
                    "Content-Length: " . strlen($file),
                    "Content-Type: image/x-icon"
                ];
				wing_socket_send_msg($client,implode("\r\n",$headers)."\r\n\r\n".$file);
				//wing_close_socket($client);
                return;
            }*/
			//image/png
			/* if($get["path"] == "/1.jpg"){
//image/x-icon
                $file = file_get_contents(trim($get["path"],"/"));
                $headers = [
                    "HTTP/1.1 200 OK",
                    "Server: wing php/1.0",
                    "Date: " . date("D,d M YH:m:s e"),
                    "Connection: Keep-Alive",
                    "Content-Length: " . strlen($file),
                    "Content-Type: image/jpg"
                ];
				wing_socket_send_msg($client,implode("\r\n",$headers)."\r\n\r\n".$file);
				//wing_close_socket($client);
                return;
            }*/
			
			
            //var_dump($get);
            parse_str($get["query"],$_GET);
            $_REQUEST = array_merge($_GET,$_REQUEST);
            //var_dump($_REQUEST);
            //call_user_func_array($this->config["onreceive"],[$client,$msg]);
            $content = '';
            if(!file_exists("cache/".md5($path).".cache")) {
                ob_start();
                require_once $path;
                $content = ob_get_contents();
                ob_end_clean();
                file_put_contents("cache/".md5($path).".cache",$content);
            }else{
                $content = file_get_contents("cache/".md5($path).".cache");
            }

            $headers = [
                "HTTP/1.1 200 OK",
                "Server: wing php/1.0",
                "Date: " . date("D,d M YH:m:s e"),
                "Connection: Keep-Alive",
                "Content-Length: " . strlen($content),
                "Content-Type: $mime_type"
            ];


           // echo implode("\r\n", $headers) . "\r\n\r\n" . $content;
            wing_socket_send_msg($client, implode("\r\n", $headers) . "\r\n\r\n" . $content);
            //wing_close_socket($client);
            unset($_GET,$_POST,$_REQUEST);
            unset($content,$mime_type,$headers);
            return;
        }

        if( $http == "post" ){
            $headers = explode("\r\n\r\n",$msg);
            $content = array_pop($headers);
            $headers = explode("\r\n",$headers[0]);

            //Content-Type:
            foreach($headers as $header){
                if(strpos($header,"Content-Type") === 0){
                    $temp = explode("boundary=",$header);
                    $this->http_post_split = $temp[1];
                }
            }



            preg_match("/Content-Length:(.*)\r\n/",$msg,$out);
            $this->content_len = trim($out[1]);
            echo "content len:",$this->content_len,"\r\n";
            echo "content split:",$this->http_post_split,"\r\n";
            if( strlen($content) >= $this->content_len){
                //echo $content;
                parse_str($content,$_POST);
                $_REQUEST = array_merge($_POST,$_REQUEST);
               // var_dump($_REQUEST);
                //echo "post content:",$content,"\r\n\r\n";
                //call_user_func_array($this->config["onreceive"],[$client,""]);

                wing_socket_send_msg($client, implode("\r\n", $headers) . "\r\n\r\n" . $file);
                //wing_close_socket($client);

                unset($_GET,$_POST,$_REQUEST);
            }
            return;

        }



       // echo "=========================================\r\n";

        $forms  = explode("--".$this->http_post_split,$msg);
        array_pop($forms);
        array_shift($forms);
        foreach($forms as $_f){
            $_temp = explode("\r\n\r\n",$_f);
            preg_match("/\"(.*)\"/",$_temp[0],$_out);
            //var_dump($_out);
            $_POST[$_out[1]] = trim($_temp[1],"\r\n");
        }
        //var_dump($_POST);
        $_REQUEST = array_merge($_POST,$_REQUEST);
       // echo "=========================================\r\n\r\n\r\n";

        //call_user_func_array($this->config["onreceive"],[$client,""]);
        unset($_GET,$_POST,$_REQUEST);
    }
    public function start(){
        $_self = $this;
        $params["onreceive"] = function($client,$msg) use($_self){
            $_self->onreceive($client,$msg);
        };
        $params["onconnect"]    = function($client){};
        $params["onclose"]      = function(){};
        $params["onerror"]      = function(){};
        $params["port"]         = $this->config["port"];
        $params["listen"]       = $this->config["listen"];
        /* foreach($this->callbacks as $callback){
             if( !isset( $params[$callback] ) || !is_callable($this->config[$callback])){
                 $this->config[$callback] = function($a=0,$b='',$c=''){};
             }
         }*/
        register_shutdown_function(function(){
            wing_service_stop();
        });
        wing_service($params);
    }
}