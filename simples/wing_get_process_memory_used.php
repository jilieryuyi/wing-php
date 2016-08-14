<?php
/**
 * @author yuyi
 * @created 2016/8/13 17:49
 * @email 297341015@qq.com
 */
//获取当前进程
echo wing_get_process_memory_used()/1024,"k\r\n";
//获取指定进程
echo wing_get_process_memory_used(11060)/1024,"k\r\n";