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
    wing_create_process_ex(__DIR__ . "/service.php start ", __DIR__ . "\\log\\output_base.log");
    第一个参数为需要在守护进程中执行的程序指令，第二个参数为进程输出重定向到指定文件，就是如此简单^ˇ^	