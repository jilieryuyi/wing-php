<?php
/**
 * @author yuyi
 * @created 2016/8/25 21:52
 * @email 297341015@qq.com
 */
$attr = [
    "AdminPasswordStatus",
    "AutomaticManagedPagefile",
    "AutomaticResetBootOption",
    "AutomaticResetCapability",
    "BootOptionOnLimit",
    "BootOptionOnWatchDog",
    "BootROMSupported",
    "BootupState",
    "BootStatus",
    "Caption",
    "ChassisBootupState",
    "ChassisSKUNumber",
    "CreationClassName",
    "CurrentTimeZone",
    "DaylightInEffect",
    "Description",
    "DNSHostName",
    "Domain",
    "DomainRole",
    "EnableDaylightSavingsTime",
    "FrontPanelResetStatus",
    "HypervisorPresent",
    "InfraredSupported",
    "InitialLoadInfo",
    "InstallDate",
    "KeyboardPasswordStatus",
    "LastLoadInfo",
    "Manufacturer",
    "Model",
    "Name",
    "NameFormat",
    "NetworkServerModeEnabled",
    "NumberOfLogicalProcessors",
    "NumberOfProcessors",
    "OEMLogoBitmap",
    "OEMStringArray",
    "PartOfDomain",
    "PauseAfterReset",
    "PCSystemType",
    "PCSystemTypeEx",
    "PowerManagementCapabilities",
    "PowerManagementSupported",
    "PowerOnPasswordStatus",
    "PowerState",
    "PowerSupplyState",
    "PrimaryOwnerContact",
    "PrimaryOwnerName",
    "ResetCapability",
    "ResetCount",
    "ResetLimit",
    "Roles",
    "Status",
    "SupportContactDescription",
    "SystemFamily",
    "SystemSKUNumber",
    "SystemStartupDelay",
    "SystemStartupOptions",
    "SystemStartupSetting",
    "SystemType",
    "ThermalState",
    "TotalPhysicalMemory",
    "UserName",
    "WakeUpType",
    "Workgroup",
];
$sql = 'SELECT * FROM Win32_ComputerSystem';
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