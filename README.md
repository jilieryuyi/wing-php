###关于wing php
	wing php windows平台下实现php多进程、源码加密、tcp、http、websocket、com...
	php编译版本5.6.20 编译环境为Visual Studio 2012 X86 (VC11-x86)

###发布的版本
	php_wing.dll 编译发布dll文件位于版本库master分支根目录

###文档                               
	http://www.itdfy.com/git/

##QQ群
    535218312
	
##创建守护进程
    $process_id = wing_create_process_ex(__DIR__ . "/service.php start ", __DIR__ . "\\log\\output_base.log");
    第一个参数为需要在守护进程中执行的程序指令，第二个参数为进程输出重定向到指定文件，就是如此简单^ˇ^	
	与wing_create_process的唯一区别是，wing_create_process需要指定可执行文件，wing_create_process_ex不需要
	如：$process_id = wing_create_process_ex("C:/php/bin/php.exe ".__DIR__ . "/service.php start ", __DIR__ . "\\log\\output_base.log");
	
    $wing_version = wing_version()
	获取版本号api，echo wing_version();也可以使用常量WING_VERSION
	
    $error_code = wing_get_last_error();//$error_code = wing_wsa_get_last_error();
	两个api均为返回最后发生的错误编码
	
   
   $error_msg = wing_get_error_msg($error_code)
   将wing_get_last_error、wing_wsa_get_last_error转换为错误字符串
   
   $memory_usage = wing_get_memory_used();
   获取进程实际占用的内存大小，单位为字节

##一个例子：
   $handle = wing_create_mutex("a test mutex"); //这里的内核对象 $handle 会被子进程继承

    wing_set_env("data","这句话将被传到子进程");
    $command = WING_PHP." ".__DIR__."/wing_create_process_runer.php";
    //$process_id = wing_create_process( $command, __DIR__."\\process_output.log");
    $process_id = wing_create_process_ex(__DIR__."/wing_create_process_runer.php",__DIR__."\\process_output.log");
    //wing_create_process_ex专属php文件的创建进程方式 即把php文件作为一个单独的进程中执行
    echo "进程id:",$process_id,"\r\n";


    //wing_process_kill( $process_id ); //可以是用这个api杀死正在运行的进程



    /*if( com_find_process( $command ) ) { //为了测试这段代码 可以去掉 wing_process_kill的注释
    //建议使用这种方式判断进程是否正在运行 可靠
    echo $process_id,"正在运行\r\n";
    }else{
    echo $process_id,"未运行\r\n";
    }*/

    //sleep(60);
    //查看引用计数器
    echo "引用计数",wing_query_object( $handle ),"\r\n";


    //这种方法也很可靠 不过有些进程由于安全权限的原因 可能无法正常获取 造成失败 这个时候com是一个不错的选择
    $process_command = wing_query_process( "wing_create_process_runer.php" ) ;
    var_dump( $process_command );
    if( is_array( $process_command) && count( $process_command) > 0 ) {
    echo $process_id,"正在运行\r\n";
    }



    $wait_code = wing_process_wait( $process_id, WING_INFINITE ); //永不超时等待子进程退出 第二个参数为可选参数 默认为WING_INFINITE
    switch( $wait_code ) {
    case WING_ERROR_FAILED :
            echo "等待失败\r\n";
        break;

    case WING_WAIT_ABANDONED :
            echo '没有释放mutex（当hHandle为mutex时，如果拥有mutex的线程在结束时没有释放核心对象会引发此返回值。）\r\n';
        break;

    case WING_WAIT_TIMEOUT://仅当wing_process_wait第二个参数不为 WING_INFINITE有效
        echo "等待超时\r\n";
        break;
    
    default:
        echo "进程退出码：",$wait_code,"\r\n"; //在子进程调用exit时传入的参数
    }


    //查看引用计数器 比子进程退出之前小了1 也可以通过这种方式去判断子进程是否还在运行~
    echo "引用计数",wing_query_object( $handle ),"\r\n";
    //wing_query_object 无法识别$handle是否有效 请在wing_close_mutex调用前使用此函数

    wing_close_mutex($handle);
    $handle = 0; //记住 close之后 $handle=0 清理 很重要 防止后面 被误用



    wing_process_kill( $process_id );
	强制终止一个进程，很暴力的做法，实际应用需要考虑一些事务性的问题，防止数据丢失
	
    $current_process_id = wing_get_current_process_id();
	获取当前进程id
	
    $quote_times = wing_query_object( $handle );
	获取对象句柄的引用次数
	
    $handle = wing_create_mutex() ;
	创建一个互斥内核对象

	wing_close_mutex($handle);
	关闭一个互斥内核对象
	
    $process_info = wing_query_process( $key_word );
	查询系统进程

    $env = wing_get_env($key);
	获取环境变量
	
    wing_set_env( $key, $value );
	设置环境变量
	
    $path = wing_get_command_path( $aommand );
	获取命令所在的完整路径
	
    $command_line = wing_get_command_line();
	获取启动命令
	
    wing_override_function($func,$param,$code);
	重写系统函数，如：wing_override_function("header",'$header','global $http; $http->setHeaders($header);');
	重写了系统的header函数，header被重写为参数为$header，函数的具体实现为global $http; $http->setHeaders($header);，
	即 function header( $header ){global $http; $http->setHeaders($header);}
						
    wing_windows_send_msg($title,$msg);
	跨进程给窗体程序发送消息，实现原理为windows的copydata
	
    wing_windows_version();
	获取windows的系统版本
	$version = wing_windows_version();
	switch( $version ) {
		case WING_WINDOWS_ANCIENT:      echo "windows ancient\r\n";break;
		case WING_WINDOWS_XP:           echo "windows xp\r\n";break;
		case WING_WINDOWS_SERVER_2003:  echo "windows server 2003\r\n";break;
		case WING_WINDOWS_VISTA:        echo "windows vista\r\n";break;
		case WING_WINDOWS_7:            echo "windows 7\r\n";break;
		case WING_WINDOWS_8:            echo "windows 8\r\n";break;
		case WING_WINDOWS_8_1:          echo "windows 8.1\r\n";break;
		case WING_WINDOWS_10:           echo "windows 10\r\n";break;
		case WING_WINDOWS_NEW:          echo "windows new\r\n";break;
		case WING_ERROR_NT:             echo "some error happened\r\n"; break;
		default :                       echo "unknow version\r\n";
	}

