<?php
/**
 * @author yuyi
 * @created 2016/5/27 17:08
 * @email 297341015@qq.com
 * @windows crontab
 */

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
        array_pop($splits);

        $times_tab  = implode(" ",$splits);

        $splits     = explode(" ",$_crontab,7);
        $commands   = array_pop($splits);
        $commands   = trim($commands);

        $_commands  = explode(" ",$commands);
        $exe_path   = $_commands[0];
        if(strpos($commands,"\"")===0) {
            preg_match("/\"(.*?)\"/", $commands, $matches);
            $exe_path = $matches[0];
        }
        $exe_path   = trim($exe_path);
        $exe_path   = trim($exe_path,"\"");

        if( !file_exists($exe_path) ){
            $exe_path = wing_get_command_path($exe_path);
        }

        $splits     = explode(" ",$_crontab);
        $params     = array_pop($splits);

        if( $_crontab[strlen($_crontab)-1] == "\"" ){
            preg_match_all("/\"(.*?)\"/", $commands, $matches);
            $params = array_pop(array_pop($matches));
            $params = trim($params);
            $params = trim($params,"\"");
        }

        $crontabs_format[] =[$times_tab,$exe_path,$params];
    }
    return $crontabs_format;
}

function isTimeRule($times_tab){
    debuglog("check:".$times_tab);
    //##秒 分 时 日 月  星期几
    //*/1  *  * *  *   *
    $tabs       = explode(" ",$times_tab);
    $times_b    = [date("s"), date("m"), date("H"), date("d"), date("m"), date("w")];
    $checks     = [];

    foreach( $tabs as $index => $tab )
    {

        $check_run          = false;
        $times_b[$index]    = intval($times_b[$index]);
        if ( $tab != "*" ) {
            if ( strpos($tab, "/") !== false ) {
                $temp       = explode("/", $tab);
                if ( $temp[0] == "*" ) {
                    //每多少单位执行
                    //本来这里需要获取上次执行的时间 然后判断是否已经满足
                    //每多少秒执行一次，此处不用这种方法
                    if ($times_b[$index] % $temp[1] == 0) {
                        $check_run = true;
                        debuglog("check:".($times_b[$index] % $temp[1]).":true");
                    }else{
                        debuglog("check:".($times_b[$index] % $temp[1]).":false");
                    }
                } else if (strpos($temp[0], "-") !== false) {
                    $s_temp = explode("-", $temp[0]);
                    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
                        //格式错误
                        $check_run = false;
                    }else {
                        if ($s_temp[0] <= $times_b[$index] && $times_b[$index] <= $s_temp[1]) {
                            //每多少单位执行
                            if ($times_b[$index] % $temp[1] == 0) {
                                $check_run = true;
                            }
                        } else {
                            $check_run = false;
                        }
                    }
                }
            } else {
                //ok
                if (strpos($tab, ",") !== false) {
                    debuglog($tab." check , ok");
                    //离散
                    $s_temp = explode(",", $tab);
                    foreach ($s_temp as $s) {

                        debuglog($times_b[$index] ."==". intval($s).
                        "=".(($times_b[$index] == intval($s))?"true":"false"));

                        if ($times_b[$index] == intval($s)) {
                            debuglog($tab." check , ok true");
                            $check_run = true;
                            break;
                        }
                    }
                } else {
                    debuglog($tab." check ".$times_b[$index] ."==". intval($tab));
                    $check_run = $times_b[$index] == intval($tab);
                }
            }
        }else{
            $check_run = true;
        }

        $checks[] = $check_run;
    }

    foreach ($checks as $check){
        if(!$check)return false;
    }

    return true;
}
function debuglog($content){
    file_put_contents(__DIR__."/wing_crontab_debug.log",$content."\r\n\r\n",FILE_APPEND);
}
function crontab(){
    $crontab_contents   = file_get_contents(__DIR__."/wing_contab.conf");
    $crontabs_format    = paramsFormat($crontab_contents);

    foreach ($crontabs_format as $crontab){
        if(isTimeRule($crontab[0])){
            debuglog("run: ".$crontab[1]." ".$crontab[2]);
            wing_create_process($crontab[1],$crontab[2]);
        }
    }
}

wing_timer(1000,function(){
    crontab();
});