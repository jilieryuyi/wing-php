<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/24
 * Time: 18:07
 * @get var
 * @获取变量
 */
function aa()
{
    $a = array(1, "12");
    var_dump(wing_var_get("a"));


    //wing_var_create("bsss", "123123");

    //var_dump($bsss);
    //var_dump(wing_var_get("bsss"));
}

aa();