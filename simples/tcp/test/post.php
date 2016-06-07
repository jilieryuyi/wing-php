<?php
/**
 * @author yuyi
 * @created 2016/6/4 11:07
 * @email 297341015@qq.com
 */
$uri = "http://127.0.0.1:6998/index.php";

$connt = 0;
$rand = rand(1,10000);
while (1) {
    file_get_contents("http://127.0.0.1:6998/?id=12asdgfsdfgsdfg%20asdf%20asf24%20234");
    //if($connt>100000)break;
}
exit;
// 参数数组
$data = array (
    'name' => '123',
    "password"=>"123456789"
);
$connt = 0;
$rand = rand(1,10000);
while (1) {
    $s = time();
    $ch = curl_init();
// print_r($ch);
    curl_setopt($ch, CURLOPT_URL, $uri);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
    $return = curl_exec($ch);
    curl_close($ch);

    file_put_contents("test_$rand.log", $connt."==>".$return . "\r\n\r\n\r\n", FILE_APPEND);
    $connt++;
    if((time()-$s)>1 || empty($return)){
        $CC=0;
        if(file_exists("error.log"))$CC=file_get_contents("error.log");
        $CC++;
        file_put_contents("error.log",$CC);
    }
    if($connt>100000)break;
}