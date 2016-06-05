<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/19
 * Time: 12:38
 * @run hopp as daeman process
 * @this function needs Com suport
 * @add this config at php.ini last
 *
 *  [PHP_COM_DOTNET]
 *  extension=php_com_dotnet.dll
 *
 */


$obj               = new COM ( 'winmgmts://localhost/root/CIMV2' );



        $processes = $obj->ExecQuery("Select * from Win32_Process Where Name=\"php.exe\"");
        foreach ($processes as $process) {
            echo $process->CommandLine,"\r\n";
                    $process->Terminate();

        }
