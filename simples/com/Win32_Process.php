<?php
/**
 * @author yuyi
 * @created 2016/8/25 16:31
 * @email 297341015@qq.com
 */

$sql = 'SELECT * FROM Win32_Process';
$com = new wing_com();
$com->query( $sql );
while( $com->next() ){
    echo $com->get("PeakWorkingSetSize"),"\r\n";
}