<?php
/**
 * @author yuyi
 * @created 2016/6/8 17:39
 * @email 297341015@qq.com
 */
$uri = "http://127.0.0.1:6998/";
$data = array (
    'name' => '123',
    "password"=>"123456789"
);

while(1)
{
    echo file_get_contents("http://127.0.0.1:9998/"),"\r\n\r\n";
    //exec("ab -n 20000 -c 20000 http://127.0.0.1:6998/");
   // echo 1,"\n";http://wing.ylb.com/
    // sleep(1);
    //file_get_contents("http://127.0.0.1:6998/?id=12asdgfsdfgsdfg%20asdf%20asf24%20234");
   /* $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $uri);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch,CURLOPT_TIMEOUT,1);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, ($data));//http_build_query
    $return = curl_exec($ch);
    curl_close($ch);
    echo $return;*/
}