<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/24
 * Time: 19:33
 * @get all process from system
 */

$obj                   = new COM ( 'winmgmts://localhost/root/CIMV2' );
$processes             = $obj->ExecQuery("Select * from Win32_Process");

foreach($processes as $process){
    echo "process id:".sprintf("%' 6d",$process->ProcessId) ."=>". $process->CommandLine,"\r\n";
}


/**
 * stop all php.exe processes
 * $processes             =    $obj->ExecQuery("Select * from Win32_Process Where Name=\"php.exe\"");
    foreach($processes as $process){
            $process->Terminate();
    }
    echo "stop is run\r\n";
 * */
