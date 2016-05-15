<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/27
 * Time: 13:55
 */
define("BUFSIZE", 512);

include_once "define.php";
$hPipe          ="";
$lpvMessage     ="Default message from client.";
$chBuf          ="";
$fSuccess       =false;
$cbRead         ="";
$cbToWrite      ="";
$cbWritten      ="";
$dwMode         ="";
$lpszPipename   ="\\\\.\\Pipe\\test";


$lpvMessage = "hello";

// Try to open a named pipe; wait for it, if necessary.

   while (1)
   {
       $hPipe =  wing_create_file(
           $lpszPipename,
           GENERIC_READ | GENERIC_WRITE|FILE_FLAG_OVERLAPPED,
           0,
           0,
           OPEN_EXISTING,
           FILE_ATTRIBUTE_NORMAL
       );
       register_shutdown_function(function() use($hPipe){wing_close_handle($hPipe);});

       // Break if the pipe handle is valid.


       if ($hPipe == INVALID_HANDLE_VALUE) {
           echo "create pipe error,code ".wing_get_last_error()."\n";
           break;
       }

       // Exit if an error other than ERROR_PIPE_BUSY occurs.



       if (wing_get_last_error() == ERROR_PIPE_BUSY)
       {
           printf("Could not open pipe. GLE=%d\n", wing_get_last_error() );
           exit;
       }

       // All pipe instances are busy, so wait for 20 seconds.

       if ( ! wing_wait_name_pipe($lpszPipename,20000))
       {
           printf("Could not open pipe: 20 second wait timed out. %ld", wing_get_last_error() );
           exit;
       }
   }

// The pipe connected; change to message-read mode.

   $dwMode = PIPE_READMODE_MESSAGE;
$fSuccess = wing_set_name_pipe_handle_state(
    $hPipe,    // pipe handle
    $dwMode // new pipe mode
      );    // don't set maximum time
   if ( ! $fSuccess)
   {
       printf("SetNamedPipeHandleState failed. GLE=%d\n", wing_get_last_error() );
       exit;
   }

// Send a message to the pipe server.

   $cbToWrite = strlen($lpvMessage)+1;
   printf("Sending %d byte message: \"%s\"\n", $cbToWrite, $lpvMessage);

   $fSuccess = wing_write_file(
       $hPipe,                  // pipe handle
       $lpvMessage,             // message
       $cbToWrite,              // message length
       $cbWritten,             // bytes written
       $_ttt);                  // not overlapped

   if ( ! $fSuccess)
   {
       printf("WriteFile to pipe failed. GLE=%d\n", wing_get_last_error() );
       exit;
   }

   printf("\nMessage sent to server, receiving reply as follows:\n");

   do
   {
       // Read from the pipe.

       $fSuccess = wing_read_file(
           $hPipe,    // pipe handle
           $chBuf,    // buffer to receive reply
           $cbRead,  // number of bytes read
           $oOverlap);    // not overlapped

       if ( ! $fSuccess && wing_get_last_error() != ERROR_MORE_DATA )
           break;

       printf("\"%s\"\n", $chBuf );
   } while ( ! $fSuccess);  // repeat loop if ERROR_MORE_DATA

   if ( ! $fSuccess)
   {
       printf("ReadFile from pipe failed. GLE=%d\n", wing_get_last_error() );
       exit;
   }

   printf("End of message, press ENTER to terminate connection and exit");

wing_close_handle($hPipe);