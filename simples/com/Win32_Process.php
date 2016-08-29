<?php
/**
 * @author yuyi
 * @created 2016/8/25 16:31
 * @email 297341015@qq.com
 * @系统进程查询
 */
//关于属性这里有详细的说明 http://wenku.baidu.com/link?url=Hg73CvDD0EvOOJSgT8AvzaNdCKFtcfvBX8NUkeHXnY2Wu434306alLCIFlCDW9eKF4eLUn3Si1h0t7eHXiudb1Ot1ynUBOqeQpzjUvqEs4_
$attr = [
    "CreationClassName",  //表示用来创建范例的类别或子类别的名称
    "Caption",            //属性为对象的简短文字描述
    "CommandLine",        //如果可用的话，CommandLine 属性指定了启动某个特定进程所要用到命令行
    "CreationDate",       //进程开始执行的时间
    "CSCreationClassName",//包含作用域计算机系统的创建类别名称
    "CSName",             //作用域计算机系统的名称
    "Description",        //属性提供对象的简短文字描述
    "ExecutablePath",     //属性表示进程的可执行文件的路径
    "ExecutionState",     //表示当前进程的操作条件。值包含就绪(2)、运行(3)和受阻(4)及其它
    "Handle",
    "HandleCount",
    "InstallDate",
    "KernelModeTime",
    "MaximumWorkingSetSize",
    "MinimumWorkingSetSize",
    "Name",
    "OSCreationClassName",
    "OSName",
    "OtherOperationCount",
    "OtherTransferCount",
    "PageFaults",
    "PageFileUsage",
    "ParentProcessId",
    "PeakPageFileUsage",
    "PeakVirtualSize",
    "PeakWorkingSetSize",
    "Priority",
    "PrivatePageCount",
    "ProcessId",
    "QuotaNonPagedPoolUsage",
    "QuotaPagedPoolUsage",
    "QuotaPeakNonPagedPoolUsage",
    "QuotaPeakPagedPoolUsage",
    "ReadOperationCount",
    "ReadTransferCount",
    "SessionId",
    "Status",
    "TerminationDate",
    "ThreadCount",
    "UserModeTime",
    "VirtualSize",
    "WindowsVersion",
    "WorkingSetSize",
    "WriteOperationCount",
    "WriteTransferCount"
];
$sql = 'SELECT * FROM Win32_Process where Caption="php.exe"';
$com = new wing_com();
$com->query( $sql );
$count =1;
while( $com->next() ){
    echo $count++,"=>";
    foreach ( $attr as $at )
    echo $at,"=>",$com->get($at),"\r\n";

    echo "error=>",wing_get_error_msg(wing_get_last_error()),"\r\n";
    echo "==========================================================\r\n\r\n";
}

