<?php
/**
 * @author yuyi
 * @created 2016/8/18 22:18
 * @email 297341015@qq.com
 */
$obj       = new COM ( 'winmgmts://localhost/root/CIMV2' );
$processes = $obj->ExecQuery("Select * from Win32_Process");
$count     = 1;
foreach ($processes as $process) {
    echo ($count++),$process->Caption,"\r\n";
}
