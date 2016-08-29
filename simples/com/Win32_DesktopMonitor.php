<?php
/**
 * @author yuyi
 * @created 2016/8/25 21:46
 * @email 297341015@qq.com
 */

$attr=[
    "Availability",
    "Bandwidth",
    "Caption",
    "ConfigManagerErrorCode",
    "ConfigManagerUserConfig",
    "CreationClassName",
    "Description",
    "DeviceID",
    "DisplayType",
    "ErrorCleared",
    "ErrorDescription",
    "InstallDate",
    "IsLocked",
    "LastErrorCode",
    "MonitorManufacturer",
    "MonitorType",
    "Name",
    "PixelsPerXLogicalInch",
    "PixelsPerYLogicalInch",
    "PNPDeviceID",
    "PowerManagementCapabilities[]",
    "PowerManagementSupported",
    "ScreenHeight",
    "ScreenWidth",
    "Status",
    "StatusInfo",
    "SystemCreationClassName",
    "SystemName"
];
$sql = 'SELECT * FROM Win32_DesktopMonitor';
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