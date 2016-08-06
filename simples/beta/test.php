<?php
/**
 * @author yuyi
 * @created 2016/8/5 14:16
 * @email 297341015@qq.com
 */
while( 1 ) {
    //echo file_get_contents("http://127.0.0.1:9998/");

    $s = time();
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, "http://127.0.0.1:6998/");
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
    $return = curl_exec($ch);
    curl_close($ch);

    echo $return,"\r\n";
}