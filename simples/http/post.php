<?php
/**
 * @author yuyi
 * @created 2016/6/4 11:07
 * @email 297341015@qq.com
 */
$uri = "http://127.0.0.1:6998/abc";
// 参数数组
$data = array (
    'name' => '123',
    "password"=>"123456789"
);

$ch = curl_init ();
// print_r($ch);
curl_setopt ( $ch, CURLOPT_URL, $uri );
curl_setopt ( $ch, CURLOPT_POST, 1 );
curl_setopt ( $ch, CURLOPT_HEADER, 0 );
curl_setopt ( $ch, CURLOPT_RETURNTRANSFER, 1 );
curl_setopt ( $ch, CURLOPT_POSTFIELDS, "id=123");
$return = curl_exec ( $ch );
curl_close ( $ch );

print_r($return);