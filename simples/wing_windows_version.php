<?php
/**
 * @author yuyi
 * @created 2016/8/12 17:39
 * @email 297341015@qq.com
 * @获取windows版本
 */
/*
#define  WING_WINDOWS_ANCIENT           0
#define  WING_WINDOWS_XP                51
#define  WING_WINDOWS_SERVER_2003       52
#define  WING_WINDOWS_VISTA             60
#define  WING_WINDOWS_7                 61
#define  WING_WINDOWS_8                 62
#define  WING_WINDOWS_8_1               63
#define  WING_WINDOWS_10                100
#define  WING_WINDOWS_NEW               MAXLONG
*/
$version = wing_windows_version();
switch( $version ){
    case WING_WINDOWS_ANCIENT:      echo "windows ancient\r\n";break;
    case WING_WINDOWS_XP:           echo "windows xp\r\n";break;
    case WING_WINDOWS_SERVER_2003:  echo "windows server 2003\r\n";break;
    case WING_WINDOWS_VISTA:        echo "windows vista\r\n";break;
    case WING_WINDOWS_7:            echo "windows 7\r\n";break;
    case WING_WINDOWS_8:            echo "windows 8\r\n";break;
    case WING_WINDOWS_8_1:          echo "windows 8.1\r\n";break;
    case WING_WINDOWS_10:           echo "windows 10\r\n";break;
    case WING_WINDOWS_NEW:          echo "windows new\r\n";break;
    default : echo "unknow version\r\n";
}