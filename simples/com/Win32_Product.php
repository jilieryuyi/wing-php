<?php
/**
 * @author yuyi
 * @created 2016/8/25 21:18
 * @email 297341015@qq.com
 */
$attr=[
    "AssignmentType",
    "Caption",
    "Description",
    "IdentifyingNumber",
    "InstallDate",
    "InstallDate2",
    "InstallLocation",
    "InstallState",
    "HelpLink",
    "HelpTelephone",
    "InstallSource",
    "Language",
    "LocalPackage",
    "Name",
    "PackageCache",
    "PackageCode",
    "PackageName",
    "ProductID",
    "RegOwner",
    "RegCompany",
    "SKUNumber",
    "Transforms",
    "URLInfoAbout",
    "URLUpdateInfo",
    "Vendor",
    "WordCount",
    "Version"
];
$sql = 'SELECT * FROM Win32_Product';
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