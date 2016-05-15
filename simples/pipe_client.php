<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/27
 * Time: 19:31
 */
include_once "define.php";
const PIPE_NAME = "\\\\.\\Pipe\\test";

$g_hPipe = INVALID_HANDLE_VALUE;


 $buffer = '';
$ReadNum=0;

    printf("test client.\n");

    $g_hPipe = wing_create_file(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL
    );
register_shutdown_function(function() use($g_hPipe){wing_close_handle($g_hPipe);});

    if ($g_hPipe == INVALID_HANDLE_VALUE)
    {
        printf("Connect pipe failed!\n");
        exit;
    }
    printf("Connected.\n");

  while(1)
    {
        $ReadNum=0;
        if(wing_read_file($g_hPipe, $buffer, $ReadNum, 0) == FALSE)
        {
            echo "read faile\n";
            break;
        }
        printf("%d==>%s\n", $ReadNum,$buffer);



    }
