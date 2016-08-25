<?php
/**
 * @author yuyi
 * @created 2016/8/25 16:31
 * @email 297341015@qq.com
 */

$sql = 'SELECT * FROM Win32_Process';
$com = new wing_com();
$com->query( $sql );
$count =1;
while( $com->next() ){
    echo $count++,"=>";
    echo $com->get("Caption"),"\r\n";
    echo $com->get("CommandLine"),"\r\n";
    echo "==========================================================\r\n\r\n";
}