<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/27
 * Time: 18:57
 */
include_once "define.php";
const PIPE_NAME ="\\\\.\\Pipe\\test";

$g_hPipe = INVALID_HANDLE_VALUE;


    $buffer = "hello_";
    $WriteNum = 0;

    printf("test server.\n");
    $g_hPipe = wing_create_name_pipe(PIPE_NAME,
        PIPE_ACCESS_DUPLEX,  PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE, 1, 0, 0, 1000, 0);

    register_shutdown_function(function() use($g_hPipe){wing_close_handle($g_hPipe);});

    if($g_hPipe == INVALID_HANDLE_VALUE)
    {
        printf("Create name pipe failed!\n");
        exit;
    }

    printf("Wait for connect...\n");
    if(!wing_connect_name_pipe($g_hPipe, 0))
    {
        printf("Connect failed!\n");
        exit;
    }
    printf("Connected.\n");

    $i =0;
    while(1)
    {
        $_buffer=$buffer.$i;
        $i++;
        $WriteNum=0;
        if(wing_write_file($g_hPipe, $_buffer, $WriteNum, 0) == FALSE)
        {
            printf("Write failed!\n");
            break;
        }
        echo "write num:".$WriteNum."==>".$_buffer,"\n";
        sleep(1);
    }

