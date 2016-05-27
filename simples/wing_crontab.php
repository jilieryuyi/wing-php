<?php
/**
 * @author yuyi
 * @created 2016/5/27 17:08
 * @email 297341015@qq.com
 * @windows crontab
 */

//-l 或者 l指令支持
$show_crontab_list = (isset($argv[1])&&($argv[1]=="-l"||$argv[1]=="l"))?true:false;
if( $show_crontab_list ){
    $contents = file_get_contents(__DIR__."/wing_contab.conf");
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
    $tabs = explode(" ",$times_tab);
    /*for($index = count($tabs)-1;$index >= 0;$index--){
        if($tabs[$index] == "*") continue;
        if(strpos($tabs[$index],",") !== false){

        }
    }*/
    //除了数字还有几个个特殊的符号就是"*"、"/"和"-"、","，
    //*代表所有的取值范围内的数字，"/"代表每的意思,"*/5"表示每5个单位，
    //"-"代表从某个数字到某个数字,","分开几个离散的数字


    $times_b=[
        date("s"),
        date("m"),
        date("H"),
        date("d"),
        date("m"),
        date("w")
    ];

    $checks = [];
    for($i=0;$i<count($tabs);$i++) {

        $run_second = false;
        $times_b[$i] = intval($times_b[$i]);
        if ($tabs[$i] != "*") {
            if (strpos($tabs[$i], "/") !== false) {
                $temp = explode("/", $tabs[$i]);
                if ($temp[0] == "*") {
                    //每多少秒执行
                    //$temp[1]; //获取上次的执行时间
                    //本来这里需要获取上次执行的时间 然后判断是否已经满足
                    //每多少秒执行一次，此处不用这种方法

                    if ($times_b[$i] % $temp[1] == 0) {
                        $run_second = true;
                        debuglog("check:".($times_b[$i] % $temp[1]).":true");
                    }else{
                        debuglog("check:".($times_b[$i] % $temp[1]).":false");
                    }
                } else if (strpos($temp[0], "-") !== false) {
                    $s_temp = explode("-", $temp[0]);
                    if (($s_temp[1] - $s_temp[0]) < $temp[1]) {
                        //格式错误
                        $run_second = false;
                    }else {
                        if ($s_temp[0] <= $times_b[$i] && $times_b[$i] <= $s_temp[1]) {
                            //每多少秒执行
                            // $temp[1]; //获取上次的执行时间
                            if ($times_b[$i] % $temp[1] == 0) {
                                $run_second = true;
                            }
                        } else {
                            $run_second = false;
                        }
                    }
                }
            } else {
                //ok
                if (strpos($tabs[$i], ",") !== false) {
                    debuglog($tabs[$i]." check , ok");
                    //离散
                    $s_temp = explode(",", $tabs[$i]);
                    foreach ($s_temp as $s) {

                        debuglog($times_b[$i] ."==". intval($s).
                        "=".(($times_b[$i] == intval($s))?"true":"false"));

                        if ($times_b[$i] == intval($s)) {
                            debuglog($tabs[$i]." check , ok true");
                            $run_second = true;
                            break;
                        }
                    }
                } else {
                    debuglog($tabs[$i]." check ".$times_b[$i] ."==". intval($tabs[$i]));
                    $run_second = $times_b[$i] == intval($tabs[$i]);
                }
            }
        }else{
            $run_second = true;
        }

        $checks[] = $run_second;
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