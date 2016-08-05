<?php
/**
 * @author yuyi
 * @created 2016/6/3 21:59
 * @email 297341015@qq.com
 */
\Service\Header::set(["Anthor"=>"yuyi","Email"=>"297341015@qq.com"]);
\Service\Header::set("Version: ".WING_VERSION);
//var_dump($_SERVER);
//\Service\Cookie::set("user","yuyi");
//echo \Service\Cookie::get("user");
?>
<html>
<head>
  <title>wing php</title>
  <style>
    body{
      height: 98%;
    }
    .wing-php{
      background: url('images/wing.jpg') center no-repeat;
      text-align: center;
      height: 100%;
      line-height: 100%;
      vertical-align: middle;
    }
  </style>
</head>
<body>
<div class="wing-php">
</div>
</body>
</html>