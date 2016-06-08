<?php
/**
 * @author yuyi
 * @created 2016/6/5 20:42
 * @email 297341015@qq.com
 */
$i=0;
$a = memory_get_usage();
while($i<100000){
    ob_start();
    require "index.php";
    $__content = ob_get_contents();
    ob_end_clean();
    unset($__content);
    $i++;
    echo (memory_get_usage()-$a),"\r\n";
}
