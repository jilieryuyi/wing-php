<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/28
 * Time: 18:03
 */
$php        = wing_get_command_path("php");
$process_id = wing_create_process($php,__DIR__."/timer.php");