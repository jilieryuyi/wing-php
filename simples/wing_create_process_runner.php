<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/21
 * Time: 23:09
 * @process runner
 * @简单的进程间通信
 * wing_get_process_params()
 * wing_set_env("email","297341015@qq.com") wing_get_env("email")
 */
file_put_contents(__DIR__."/wing_create_process.log",
    wing_get_current_process_id()."\r\n=>".
    wing_get_process_params()."\r\n".
    wing_get_env("email"));

exit(999);
