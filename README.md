###about wing php
	wing php是一个windows平台下实现php多进程、守护进程等功能的函数集合。
	php version 5.6.20 win32 编译php版本，win32 x86 5.6.20
	download(下载url):http://windows.php.net/downloads/releases/php-5.6.20-Win32-VC11-x86.zip
	vc11(visual studio 2012) 编译环境vc11(visual studio 2012)

###download release dll 发布的版本
	php_wing.dll in the root directory of the master branch
	php_wing.dll 编译发布文件位于版本库根目录

###wing_version                                     
	get wing extension version 
	获取扩展版本号

###wing_create_process                              
	create deamon process , if success then return process id,else return false 
	创建守护进程，成功返回进程id，失败返回false
	demo:
	$process_id = wing_create_process("C:/php/php.exe",__DIR__."/process_run.php");

###wing_process_kill                                
	kill process by processid 
	通过进程id杀死进程
	demo:
	$process_id = wing_create_process("C:/php/php.exe",__DIR__."/process_run.php");
	var_dump($process_id);
	sleep(1);
	wing_process_kill($process_id);

###wing_process_isalive                             
	check process is running,return true means process is still running 
	判断进程是否正常运行，返回true 说明进程还在运行
	demo:
	var_dump(wing_process_isalive($process_id));

###wing_enum_processes                              
	enum all processes 
	枚举所有的进程
	demo:
	var_dump(wing_enum_processes());
	return:
	  [39]=>
		  array(2) {
			["process_path"]=>
			string(60) "D:\php\php-sdk\phpdev\vc11\x86\php-5.6.20\Release_TS\php.exe"
			["process_id"]=>
			int(24684)
		  }
		  
	or use the code with com to get all processes:
	或者使用php com组件枚举所有的进程

	$obj                   = new COM ( 'winmgmts://localhost/root/CIMV2' );
	$processes             = $obj->ExecQuery("Select * from Win32_Process");

	foreach($processes as $process){
		echo "process id:".sprintf("%' 6d",$process->ProcessId) ."=>". $process->CommandLine,"\r\n";
	}
###wing_getpid
	get current process id 获取当前进程id
###wing_process_exit
	exit a process 退出进程
	demo：
		$i=0;
		file_put_contents("D:/processid.pid",wing_getpid());
		while(1){
			$i++;
			file_put_contents("D:/1.html",$i,FILE_APPEND);
			sleep(1);
			if($i>10)break;
		}
		//exit process with code 400
		wing_process_exit(400);
	save as file  process_run.php 保存为文件 process_run.php
	and create a process like 创建一个守护进程
	$process_id = wing_create_process("C:/php/php.exe",__DIR__."/process_run.php");	
###wing_create_mutex、wing_close_mutex
	wing_create_mutex params not empty string, return >0 create success,===0 process is running, <0 create failue
	wing_create_mutex 参数为非空字符串 返回值 0程序正在运行 -1 获取参数错误 -2 参数不能为空 -3创建互斥锁失败 >0创建互斥锁成功
	
	wing_close_mutex params is wing_create_mutex return value,return true close success
	wing_close_mutex的参数是 wing_create_mutex的返回值，返回true说明关闭成功
	
	run this demo twice at the same time
	同一时间运行此demo两次 查看互斥量的效果
	$handle = wing_create_mutex("a test mutex");
	// 0程序正在运行 -1 获取参数错误 -2 参数不能为空 -3创建互斥锁失败 >0创建互斥锁成功
	echo $handle,"\n";
	if($handle===0){
		echo "process is running\n";
		exit;
	}
	if($handle<0){
		echo "create mutex error\n";
		exit;
	}

	register_shutdown_function(function() use($handle){
		wing_close_mutex($handle);
	});
	$i=0;
	while(1){
		echo $i,"\n";
		$i++;
		if($i>10)break;
		sleep(1);
	}
###wing_get_command_path 
	get command path 获取命令所在路径
	demo:
	wing_get_command_path("php");
	//C:\xampp\php\php.exe
###wing_get_env 
	get environment variable 获取环境变量
	demo:
	wing_get_env("PATH");
###wing_set_env
	set environment variable 设置环境变量
	demo:
	var_dump(wing_set_env("yuyi_test","12"));
	echo wing_get_env("yuyi_test");
###wing_create_thread 
	创建一个异步线程执行php  create async thread 
	$thread_id = wing_create_thread(function(){
    $i=0;
    while(1){
        file_put_contents(__DIR__."/thread_test.log",$i++,FILE_APPEND);
        if($i>3)break;
        sleep(1);
    }

    exit(9);
	});
	if($thread_id<0){
		echo "create thread fail";
		exit;
	}
	if($thread_id == 0) {
		echo "child thread ";
		echo $thread_id;
		//child thread just exit
		exit;
	}

	//$thread_id parent continue running

	//wait for the thread exit
	$exit_code =  wing_thread_wait($thread_id,-1);

	echo $thread_id," is exit with ",$exit_code,"\n";
###about me
	QQ&Email 297341015@qq.com
	QQ群:535218312