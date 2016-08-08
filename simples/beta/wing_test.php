<?php
/**
 * @author yuyi
 * @created 2016/8/8 20:57
 * @email 297341015@qq.com
 */
$memuse = memory_get_usage();
echo $memuse,"\r\n";
function test(){
    $b=123;
    echo $b,"\r\n";
}
wing_test( function()  {
    global $memuse;
    echo "增加了",(memory_get_usage() - $memuse),"\r\n" ;
    test();
    echo "增加了",(memory_get_usage() - $memuse),"\r\n" ;
    $memuse = memory_get_usage();
    echo $memuse,"\r\n";
} );
