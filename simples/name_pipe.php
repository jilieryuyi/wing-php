<?php
/**
 * Created by PhpStorm.
 * User: Administrator
 * Date: 2016/4/27
 * Time: 9:20
 */

include_once "define.php";
$INSTANCES = 4;
$pipe_name = "\\\\.\\Pipe\\test";
$Pipe=[];
$hEvents=[];
define("CONNECTING_STATE",0);
define("READING_STATE", 1 );
define("WRITING_STATE", 2 );
define("INSTANCES", 4 );
define("PIPE_TIMEOUT", 5000);
define("BUFSIZE", 4096);



function DisconnectAndReconnect($i)
{
// Disconnect the pipe instance.
    global $Pipe;
    if (! wing_disconnect_name_pipe($Pipe[$i]["hPipeInst"]) )
   {
       printf("DisconnectNamedPipe failed with %d.\n", wing_get_last_error());
   }

// Call a subroutine to connect to the new client.

   $Pipe[$i]["fPendingIO"] = ConnectToNewClient(
           $Pipe[$i]["hPipeInst"],
       $Pipe[$i]["hEvent"],
       $Pipe[$i]["oOverlap"]);

   $Pipe[$i]["dwState"] = $Pipe[$i]["fPendingIO"] ?
    CONNECTING_STATE : // still connecting
    READING_STATE;     // ready to read
}


function  GetAnswerToRequest(&$pipe)
{
    printf( "[%d] %s\n", $pipe["hPipeInst"], $pipe["chRequest"]);
    $pipe["chReply"] = "Default answer from server";
    $pipe["cbToWrite"] = (strlen($pipe["chReply"])+1);
}

function ConnectToNewClient($hPipe,$event, &$lpo){
    $fConnected =false;
    $fPendingIO = FALSE;

// Start an overlapped connection for this pipe instance.
    $result = 0;
   $fConnected = wing_connect_name_pipe($hPipe,$event, $result);
    $lpo = $result;



// Overlapped ConnectNamedPipe should return zero.
   if (!$fConnected)
   {
       printf("ConnectNamedPipe failed with %d.\n", wing_get_last_error());
       return 0;
   }
echo "last error:",wing_get_last_error(),"\n\n";
   switch (wing_get_last_error())
   {
       // The overlapped connection in progress.
       case ERROR_IO_PENDING:
           $fPendingIO = TRUE;
           break;

       // Client is already connected, so signal an event.

       case ERROR_PIPE_CONNECTED:
           if (!wing_set_event($lpo))
            break;

       // If an error occurs during the connect operation...
       default:
       {
           printf("1==ConnectNamedPipe failed with %d.\n", wing_get_last_error());
           return 0;
       }
   }

   return $fPendingIO;
}


for ($i = 0; $i < $INSTANCES; $i++)
   {

       // Create an event object for this instance.

       $hEvents[$i] = wing_create_event();   // unnamed event object


      if ($hEvents[$i] == NULL)
      {
          printf("CreateEvent failed with %d.", wing_get_last_error());
          exit;
      }

      $Pipe[$i]["hEvent"] = $hEvents[$i];

      $Pipe[$i]["hPipeInst"] = wing_create_name_pipe(
          $pipe_name,            // pipe name
         PIPE_ACCESS_DUPLEX |     // read/write access
         FILE_FLAG_OVERLAPPED,    // overlapped mode
         PIPE_TYPE_MESSAGE |      // message-type pipe
         PIPE_READMODE_MESSAGE |  // message-read mode
         PIPE_WAIT,               // blocking mode
         INSTANCES,               // number of instances
         0,   // 0 is use default output buffer size
         0,   // 0 is use default  input buffer size
         PIPE_TIMEOUT,            // client time-out
         0);                   // default security attributes

      if ($Pipe[$i]["hPipeInst"] == wing_get_invalid_handle_value())
      {
          printf("CreateNamedPipe failed with %d.\n invalid value:%ld", wing_get_last_error(),wing_get_invalid_handle_value());
          return 0;
      }

       $Pipe[$i]["oOverlap"] = 0;

   // Call the subroutine to connect to the new client



       $Pipe[$i]["fPendingIO"] = ConnectToNewClient($Pipe[$i]["hPipeInst"], $Pipe[$i]["hEvent"],$Pipe[$i]["oOverlap"]);



       $Pipe[$i]["dwState"] = $Pipe[$i]["fPendingIO"] ?
       CONNECTING_STATE : // still connecting
       READING_STATE;     // ready to read
   }



while (1)
{
    // Wait for the event object to be signaled, indicating
    // completion of an overlapped read, write, or
    // connect operation.
var_dump($hEvents);
    $dwWait = wing_wait_multi_objects($hEvents,false,-1);    // waits indefinitely
var_dump($dwWait);
    // dwWait shows which pipe completed the operation.
echo "--------------\n";

echo "error:",wing_get_last_error(),"\n";
    echo "--------------\n";
    $i = $dwWait - 0;  // determines which pipe
    if ($i < 0 || $i > ($INSTANCES - 1))
    {
        printf("Index out of range.\n");
        exit();
    }

    // Get the result if the operation was pending.

    if ($Pipe[$i]["fPendingIO"])
      {
          $cbRet=0;
          $fSuccess = wing_get_overlapped_result(
                    $Pipe[$i]["hPipeInst"], // handle to pipe
                    $Pipe[$i]["oOverlap"], // OVERLAPPED structure
                    $cbRet);

         switch ($Pipe[$i]["dwState"])
         {
             // Pending connect operation
         case CONNECTING_STATE:
             if (! $fSuccess)
             {
                 printf("Error %d.\n", wing_get_last_error());
                 exit;
             }
             $Pipe[$i]["dwState"] = READING_STATE;
               break;

         // Pending read operation
            case READING_STATE:
               if (! $fSuccess || $cbRet == 0)
               {
                   DisconnectAndReconnect($i);
                   continue;
               }
                $Pipe[$i]["cbRead"] = $cbRet;
                $Pipe[$i]["dwState"] = WRITING_STATE;
               break;

         // Pending write operation
            case WRITING_STATE:
               if (! $fSuccess || $cbRet != $Pipe[$i]["cbToWrite"])
               {
                   DisconnectAndReconnect($i);
                   continue;
               }
                $Pipe[$i]["dwState"] = READING_STATE;
               break;

            default:
            {
                printf("Invalid pipe state.\n");
                exit;
            }
         }
      }

   // The pipe state determines which operation to do next.

      switch ($Pipe[$i]["dwState"])
      {
          // READING_STATE:
          // The pipe instance is connected to the client
          // and is ready to read a request from the client.

      case READING_STATE:
          $fSuccess = wing_read_file(
                        $Pipe[$i]["hPipeInst"],
                        $Pipe[$i]["chRequest"],
                        $Pipe[$i]["cbRead"],
                        $Pipe[$i]["oOverlap"]);

         // The read operation completed successfully.

            if ($fSuccess && $Pipe[$i]["cbRead"] != 0)
            {
                $Pipe[$i]["fPendingIO"] = FALSE;
                $Pipe[$i]["dwState"] = WRITING_STATE;
               continue;
            }

         // The read operation is still pending.

          $dwErr = wing_get_last_error();
            if (! $fSuccess && ($dwErr == wing_get_io_pending()))
            {
                $Pipe[$i]["fPendingIO"] = TRUE;
               continue;
            }

         // An error occurred; disconnect from the client.

            DisconnectAndReconnect($i);
            break;

      // WRITING_STATE:
      // The request was successfully read from the client.
      // Get the reply data and write it to the client.

         case WRITING_STATE:
            GetAnswerToRequest($Pipe[$i]);

            $fSuccess = wing_write_file(
                            $Pipe[$i]["hPipeInst"],
                            $Pipe[$i]["chReply"],
                            $Pipe[$i]["cbToWrite"],
                            $cbRet,
                            $Pipe[$i]["oOverlap"]);

         // The write operation completed successfully.

            if ($fSuccess && $cbRet == $Pipe[$i]["cbToWrite"])
            {
                $Pipe[$i]["fPendingIO"] = FALSE;
                $Pipe[$i]["dwState"] = READING_STATE;
               continue;
            }

         // The write operation is still pending.

            $dwErr = wing_get_last_error();
            if (! $fSuccess && ($dwErr == wing_get_io_pending()))
            {
                $Pipe[$i]["fPendingIO"] = TRUE;
               continue;
            }

         // An error occurred; disconnect from the client.

            DisconnectAndReconnect($i);
            break;

         default:
         {
             printf("Invalid pipe state.\n");
             exit;
         }
      }
  }
