<?php
/**
 * @author yuyi
 * @created 2016/8/15 22:03
 * @email 297341015@qq.com
 * @源码加密与执行
 */
$input_file  = __DIR__."\\wing_encrypt_file_test.php";
$ouput_file  = __DIR__."\\wing_encrypt_file_test.encrypt.php";
$encode_pass = "123456";

/*//加密一个php源码文件
var_dump( wing_encrypt_file( $input_file , $ouput_file , $encode_pass ) );

//执行一个加密过的php源码
var_dump( wing_run_file( $ouput_file, $encode_pass ) );*/

//不传密码时 自动使用硬件绑定密码 也就是说 在那台电脑上生成的加密文件 此加密文件只能在这台电脑执行
var_dump( wing_encrypt_file( $input_file , $ouput_file  ) );
//var_dump( wing_run_file( $ouput_file ) );

