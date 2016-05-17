<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/14
 * Time: 11:01
 */
//最后一个参数为可选参数，如果需要保存文件，请使用完整路径
//The last parameter is optional. If you need to save the file, use the full path.
//return base64 encode image str
echo  wing_qrencode("http://www.baidu.com/",600,__DIR__."/qr_test.png");