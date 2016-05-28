<?php
/**
 * @author yuyi
 * @created 2016/5/27 17:08
 * @email 297341015@qq.com
 * @windows crontab
 */
date_default_timezone_set("PRC");

//-l 或者 l指令支持
$show_crontab_list = ( isset($argv[1]) && ( $argv[1] == "-l" || $argv[1] == "l") )? true:false;
if( $show_crontab_list ){
    $contents       = file_get_contents(__DIR__."/wing_contab.conf");
    $crontabs       = explode("\n",$contents);
    foreach ( $crontabs as $crontab ) {
        $_crontab = trim($crontab);
        if (strpos($_crontab, "#") === 0)
            continue;
        echo $_crontab,"\r\n";
    }
    exit;
}

/**
 * @crontab 参数标准化格式化
 */
function paramsFormat($crontab_contents){
    $crontabs       = explode("\n",$crontab_contents);
    foreach ( $crontabs as $crontab ){

        $_crontab   = trim($crontab);
        if( strpos($_crontab,"#") === 0 || empty($_crontab))
            continue;

        $_crontab   = preg_replace("/ +/"," ",$_crontab);
        $splits     = explode(" ",$_crontab,7);
        $__params   = array_pop($splits);



        $times_tab  = implode(" ",$splits);



        $splits     = explode(" ",$_crontab,7);
        $commands   = array_pop($splits);
        $commands   = trim($commands);

        $_commands  = explode(" ",$commands);
        $exe_path   = $_commands[0];
        $params     = ltrim($__params,$exe_path);
        if(strpos($commands,"\"")===0) {
            preg_match("/\"(.*?)\"/", $commands, $matches);
            $exe_path = $matches[0];
            $params     = ltrim($__params,$exe_path);
        }
        $exe_path   = trim($exe_path);
        $exe_path   = trim($exe_path,"\"");

        if( !file_exists($exe_path) ){
            $exe_path = wing_get_command_path($exe_path);
        }

        $params = trim($params);

        $crontabs_format[] =[$times_tab,$exe_path,$params];
    }

    return $crontabs_format;
}


/**
 * @秒规则校验
 */
function checkSecondRule($times_tab){
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[0];
    $times_b    = date("s");

    if ( $tab == "*" )
        return true;

    if ( strpos($tab, "/") === false ) {
        if (strpos($tab, ",") !== false) {
            //离散
            $s_temp = explode(",", $tab);
            foreach ($s_temp as $s) {
                if ($times_b == intval($s)) {
                    return true;
                }
            }
        } else {
            return $times_b == intval($tab);
        }
    }


    $temp  = explode("/", $tab);
    if ( $temp[0] == "*" ) {
        //每多少单位执行
        //本来这里需要获取上次执行的时间 然后判断是否已经满足
        //每多少秒执行一次，此处不用这种方法
        if ($times_b % $temp[1] == 0) {
            return true;
        }
    }

    if (strpos($temp[0], "-") === false)
        return false;

    $s_temp = explode("-", $temp[0]);
    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
        //格式错误
        return false;
    }

    if ($s_temp[0] <= $times_b && $times_b <= $s_temp[1]) {
        //每多少单位执行
        if ($times_b % $temp[1]  == 0) {
            return true;
        }
    }

    return false;
}

function checkMinuteRule($times_tab){
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[1];
    $times_b    = time();

    if ( $tab == "*" )
        return true;

    if ( strpos($tab, "/") === false ) {
        if (strpos($tab, ",") !== false) {
            //离散
            $s_temp = explode(",", $tab);
            foreach ($s_temp as $s) {
                if ($times_b == strtotime(date("Y-m-d H:$s:0"))) {
                    return true;
                }
            }
        } else {
            return $times_b == strtotime(date("Y-m-d H:$tab:0"));
        }
    }


    $temp  = explode("/", $tab);
    if ( $temp[0] == "*" ) {
        //每多少单位执行
        //本来这里需要获取上次执行的时间 然后判断是否已经满足
        //每多少秒执行一次，此处不用这种方法
        if ($times_b % ($temp[1]*60) == 0) {
            return true;
        }
    }

    if (strpos($temp[0], "-") === false)
        return false;

    $s_temp = explode("-", $temp[0]);
    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
        //格式错误
        return false;
    }

    if ( strtotime(date("Y-m-d H:".$s_temp[0].":0")) <= $times_b &&
         $times_b <= strtotime(date("Y-m-d H:".$s_temp[1].":0"))
    ) {
        //每多少单位执行
        if ($times_b % ($temp[1]*60)  == 0) {
            return true;
        }
    }

    return false;
}




function checkHourRule($times_tab){
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[2];
    $times_b    = time();

    if ( $tab == "*" )
        return true;

    if ( strpos($tab, "/") === false ) {
        if (strpos($tab, ",") !== false) {
            //离散
            $s_temp = explode(",", $tab);
            foreach ($s_temp as $s) {
                if ($times_b == strtotime(date("Y-m-d $s:0:0"))) {
                    return true;
                }
            }
        } else {
            return $times_b == strtotime(date("Y-m-d $tab:0:0"));
        }
    }


    $temp  = explode("/", $tab);
    if ( $temp[0] == "*" ) {
        //每多少单位执行
        //本来这里需要获取上次执行的时间 然后判断是否已经满足
        //每多少秒执行一次，此处不用这种方法
        if ($times_b % ($temp[1]*3600) == 0) {
            return true;
        }
    }

    if (strpos($temp[0], "-") === false)
        return false;

    $s_temp = explode("-", $temp[0]);
    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
        //格式错误
        return false;
    }

    if ( strtotime(date("Y-m-d ".$s_temp[0].":0:0")) <= $times_b &&
        $times_b <= strtotime(date("Y-m-d ".$s_temp[1].":0:0"))
    ) {
        //每多少单位执行
        if ($times_b % ($temp[1]*3600)  == 0) {
            return true;
        }
    }

    return false;
}


function checkDayRule($times_tab){
    //##秒 分 时 日 月  星期几
    //*  *  * *  *   *
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[3];
    $times_b    = time();

    if ( $tab == "*" )
        return true;

    if ( strpos($tab, "/") === false ) {
        if (strpos($tab, ",") !== false) {
            //离散
            $s_temp = explode(",", $tab);
            foreach ($s_temp as $s) {
                if ($times_b == strtotime(date("Y-m-$s 0:0:0"))) {
                    return true;
                }
            }
        } else {
            return $times_b == strtotime(date("Y-m-$tab 0:0:0"));
        }
    }


    $temp  = explode("/", $tab);
    if ( $temp[0] == "*" ) {
        //每多少单位执行
        //本来这里需要获取上次执行的时间 然后判断是否已经满足
        //每多少秒执行一次，此处不用这种方法
        if ($times_b % ($temp[1]*86400) == 0) {
            return true;
        }
    }

    if (strpos($temp[0], "-") === false)
        return false;

    $s_temp = explode("-", $temp[0]);
    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
        //格式错误
        return false;
    }

    if ( strtotime(date("Y-m-".$s_temp[0]." 0:0:0")) <= $times_b &&
        $times_b <= strtotime(date("Y-m-".$s_temp[1]." 0:0:0"))
    ) {
        //每多少单位执行
        if ($times_b % ($temp[1]*86400)  == 0) {
            return true;
        }
    }

    return false;
}

//每月默认按照30天计算
function checkMonthRule($times_tab){
    //##秒 分 时 日 月  星期几
    //*  *  * *  *   *
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[4];
    $times_b    = time();

    if ( $tab == "*" )
        return true;

    if ( strpos($tab, "/") === false ) {
        if (strpos($tab, ",") !== false) {
            //离散
            $s_temp = explode(",", $tab);
            foreach ($s_temp as $s) {
                if ($times_b == strtotime(date("Y-$s-1 0:0:0"))) {
                    return true;
                }
            }
        } else {
            return $times_b == strtotime(date("Y-$tab-1 0:0:0"));
        }
    }


    $temp  = explode("/", $tab);
    if ( $temp[0] == "*" ) {
        //每多少单位执行
        //本来这里需要获取上次执行的时间 然后判断是否已经满足
        //每多少秒执行一次，此处不用这种方法
        if ($times_b % ($temp[1]*2592000) == 0) {
            return true;
        }
    }

    if (strpos($temp[0], "-") === false)
        return false;

    $s_temp = explode("-", $temp[0]);
    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
        //格式错误
        return false;
    }

    if ( strtotime(date("Y-".$s_temp[0]."-1 0:0:0")) <= $times_b &&
        $times_b <= strtotime(date("Y-".$s_temp[1]."-1 0:0:0"))
    ) {
        //每多少单位执行
        if ($times_b % ($temp[1]*2592000)  == 0) {
            return true;
        }
    }

    return false;
}


function getNextWeekTime($week){
    $i=1;
    while ($i<14){
        if(date("w",time()+86400*$i) == $week){
            return strtotime(date("Y-m-d 00:00:00",time()+86400*$i));
        }
        $i++;
    }
    return false;
}

function checkWeekRule($times_tab){
    //##秒 分 时 日 月  星期几
    //*  *  * *  *   *
    $tabs       = explode(" ",$times_tab);
    $tab        = $tabs[5];
    $times_b    = time();
    $week       = date("w");

    if ( $tab == "*" )
        return true;

    if (strpos($tab, ",") !== false) {
        //离散
        $s_temp = explode(",", $tab);
        foreach ($s_temp as $s) {
            $_time = getNextWeekTime($s);
            debuglog("1、".$times_b."=".$_time);
            if ($week == $s && $times_b == $_time) {
                return true;
            }
        }
    } else {
        $_time = getNextWeekTime($tab);
        debuglog("2、".$times_b."=".$_time);
        return $week == $tab && $times_b == $_time;
    }

    return false;
}

function debuglog($content){
    file_put_contents(__DIR__."/wing_crontab_debug.log",$content."\r\n\r\n",FILE_APPEND);
}
function crontab(){
    $crontab_contents   = file_get_contents(__DIR__."/wing_contab.conf");
    $crontabs_format    = paramsFormat($crontab_contents);

    foreach ($crontabs_format as $crontab){
        if( checkSecondRule($crontab[0]) &&
            checkMinuteRule($crontab[0]) &&
            checkHourRule($crontab[0]) &&
            checkDayRule($crontab[0]) &&
            checkMonthRule($crontab[0]) &&
            checkWeekRule($crontab[0])
        ){
            debuglog("run: ".$crontab[1]." ".$crontab[2]);
            wing_create_process($crontab[1],$crontab[2]);
        }
    }
    unset($crontabs_format,$crontab_contents);
}

wing_timer(1000,function(){
    crontab();
});