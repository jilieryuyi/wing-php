<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/21
 * Time: 23:09
 * @process runner
 */
file_put_contents(__DIR__."/wing_create_process.log",
    wing_getpid()."\r\n=>".wing_get_process_params());
