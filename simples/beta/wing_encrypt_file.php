<?php
/**
 * @author yuyi
 * @created 2016/8/15 22:03
 * @email 297341015@qq.com
 */
$input_file  = __DIR__."\\wing_encrypt_file_test.php";
$ouput_file  = __DIR__."\\wing_encrypt_file_test.encrypt.php";
$encode_pass = "123456";

var_dump( wing_encrypt_file( $input_file , $ouput_file , $encode_pass ) );
var_dump( wing_run_file($ouput_file,$encode_pass) );
