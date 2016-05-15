<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/25
 * Time: 22:38
 */

function GetCommandPath($command){
    $env = wing_get_env("PATH");
    $env = explode(";",$env);
    foreach($env as $path) {
        if (file_exists(trim($path, "\\") . "\\" . $command) ) {
            return trim($path, "\\") . "\\" . $command;
        }
        if(file_exists(trim($path, "\\") . "\\" . $command . ".exe")){
            return trim($path, "\\") . "\\" . $command . ".exe";
        }
    }
}

//get php command path
//echo "GetCommandPath:",GetCommandPath("php");
echo "\n";
//get php command path
echo wing_get_command_path("php");

echo "\n";

var_dump(wing_set_env("yuyi_test","12"));

echo wing_get_env("yuyi_test");
