<?php
/**
 * @author yuyi
 * @created 2016/8/25 21:34
 * @email 297341015@qq.com
 * @Win32_OperatingSystem
 */

$attr = [
    "BootDevice",
    "BuildNumber",
    "BuildType",
    "Caption",
    "CodeSet",
    "CountryCode",
    "CreationClassName",
    "CSCreationClassName",
    "CSDVersion",
    "CSName",
    "CurrentTimeZone",
    "DataExecutionPrevention_Available",
    "DataExecutionPrevention_32BitApplications",
    "DataExecutionPrevention_Drivers",
    "DataExecutionPrevention_SupportPolicy",
    "Debug",
    "Description",
    "Distributed",
    "EncryptionLevel",
    "ForegroundApplicationBoost ",
    "FreePhysicalMemory",
    "FreeSpaceInPagingFiles",
    "FreeVirtualMemory",
    "InstallDate",
    "LargeSystemCache",
    "LastBootUpTime",
    "LocalDateTime",
    "Locale",
    "Manufacturer",
    "MaxNumberOfProcesses",
    "MaxProcessMemorySize",
    "MUILanguages",
    "Name",
    "NumberOfLicensedUsers",
    "NumberOfProcesses",
    "NumberOfUsers",
    "OperatingSystemSKU",
    "Organization",
    "OSArchitecture",
    "OSLanguage",
    "OSProductSuite",
    "OSType",
    "OtherTypeDescription",
    "PAEEnabled",
    "PlusProductID",
    "PlusVersionNumber",
    "PortableOperatingSystem",
    "Primary",
    "ProductType",
    "RegisteredUser",
    "SerialNumber",
    "ServicePackMajorVersion",
    "ServicePackMinorVersion",
    "SizeStoredInPagingFiles",
    "Status",
    "SuiteMask",
    "SystemDevice",
    "SystemDirectory",
    "SystemDrive",
    "TotalSwapSpaceSize",
    "TotalVirtualMemorySize",
    "TotalVisibleMemorySize",
    "Version",
    "WindowsDirectory",
    "QuantumLength",
    "QuantumType"
];

$sql = 'SELECT * FROM Win32_OperatingSystem';
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