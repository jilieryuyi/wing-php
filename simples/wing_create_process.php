<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/5/14
 * Time: 16:48
 */
wing_set_env("email","297341015@qq.com");
echo wing_create_process(
    wing_get_command_path("php"),
    __DIR__."/wing_create_process_runner.php",
    "hello,i come from parent parent process"
);