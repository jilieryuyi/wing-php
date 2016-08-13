/**
 * Created by Administrator on 2016/2/14.
 */
if(typeof Wing=="undefined"){
    Wing={};
    Wing.query=function(a,b){return false;}
    Wing.appPath=function(){return "";}
}
var api={
    ////////////////////////////////////////配置文件相关
    /**
     * @读取配置文件 返回全部配置信息 或者返回单个配置信息 tab key
     * @不传参数返回全部
     * @如：
     *   [user]
         brand=10
         shopnum=8002
         password=123456
         autologin=1
     * @param tab 是指[user] 获取user标签则传 user字符串
     * @param key 要获取对应tab下面的那个key
     * @param config_file 要读取的配置文件 要完整路径 如 D:/config.ini
     * @return object|string 如：{user:{brand:10,shopnum:8002,password:123456,autologin:1}} 如果获取单个 则直接返回 对应的值
     **/
    ReadConfig:function(tab,key,config_file){
        var all= Wing.readConfig(config_file);
        if(typeof tab!="undefined"||typeof key=="undefined"){
            return all;
        }
        return all[tab][key];
    },
    /**
     * @写配置文件
     * @param tab 写入的配置文件标签
     * @param key 写入的配置文件key
     * @param value 写入的配置值 不可以包含换行符
     * @param config_file 要写入的配置文件
     **/
    WriteConfig:function(tab,key,value,config_file){
        return Wing.WritePrivateProfileString(tab,key,config_file);
    },
    /**
     * @追加方式写入文件 可用于写日志 可以配合Mkdir使用 文件不存在时自动创建
     * */
    WriteFile:function(file,content,append){
        if(typeof append=="undefined")append=true;
        return Wing.writeFile(file,content,append);
    },

    ///////////////////////////////////////数据库相关
    /**
     * @创建数据库|表
     * @demo  var createTable = "CREATE TABLE IF NOT EXISTS waitlist(" +
                             "id INTEGER PRIMARY KEY," +
                             "waitid NVARCHAR(20)," +
                             "num NVARCHAR(20)," +
                             "phone NVARCHAR(30)," +
                             "peoplenum INT," +
                             "status INT," +
                             "come NVARCHAR(256) ," +
                             "createtime NVARCHAR(30)," +
                             "onseattime NVARCHAR(30));";
             api.CreateTable('D:/m.db',createTable);
     **/
    CreateTable:function(db_path,sql){
        return Wing.createTable(db_path,sql)
    },
    /**
     *@执行 插入 更新 删除
     **/
    Exec:function(db_path,sql){
        return Wing.exec(db_path,sql)
    },
    /**
     * @执行查询操作
     * @return object
     * */
    Query:function(db_path,sql){
        return Wing.query(db_path, sql);
    },
    /**
     * @获取最后的错误
     * */
    LastSqlError:function(){
        return Wing.lastSqlError();
    },
    /**
     * @事务相关
     * */
    BeginTransaction:function(db_path){
        return Wing.BeginTransaction(db_path);
    },
    CommitTransaction:function(db_path){
        return Wing.CommitTransaction(db_path);
    },
    RollbackTransaction:function(db_path){
        return Wing.RollbackTransaction(db_path);
    },
    ////////////////////////////////////////////文件以及路径相关
    /**
     * @获取程序所在路径
     * */
    AppPath:function(){
        return Wing.appPath();
    },
    FileExists:function(path){
        return Wing.fileExists(path);
    },
    //创建文件 return true false
    CreateFile:function(path){
        return Wing.CreateFile(path);
    },
    DirExists:function(dir){
        return Wing.dirExists(dir);
    },
    Mkdir:function(dir){
        return Wing.mkdir(dir);
    },
    ExePath:function(){
        return Wing.getCurrentDirectory().replace("\\","/");
    },
    ////////////////////////////网络相关
    /**
     * @param 同步跨域post api
     * @param url post url
     * @param 可选参数 timeout超时时间 毫秒
     * @param 可选参数 data 要post的数据 如 a=1&b=3&s=123
     * @param 可选参数 header 自定义消息头 如{a:1,b:2,"ACCEPT-USER":"sys"}
     * */
    Post:function(url,data,header,timeout){
        if(!header||typeof header!="object"){
            header=null;
        }
        if(typeof timeout=="undefined"){
            timeout=1000;
        }
        if(typeof data=="undefined"){
            data="";
        }
        return Wing.post(url,data,header,timeout);
    },

    ////////////////////////////系统相关
    /**
     * @禁用拖动
     * */
    DisableDrag:function(){
        return Wing.stopDrag();
    },
    /**
     * @启用拖动
     * */
    EnableDrag:function(){
        return Wing.enableDrag();
    },
    /**
     * @退出系统
     * */
    Quit:function(){
        return Wing.quit();
    },
    /**
     * @最小化
     * */
    Mini:function(){
        return Wing.mini();
    },
    /**
     * @获取系统所有的打印机 返回数组
     * */
    GetPrinters:function(){
        return Wing.getPrinterList();
    },
    /**
     * @打印 采用多进程模式 可以打印url或者html内容 或者本地html文件
     * @param printer 打印机名称
     * @param url_or_content 打印url或者html内容、本地html文件路径
     * */
    Print:function(printer,url_or_content,x,y,width,height){
        if(typeof x=="undefined")       x       =0;
        if(typeof y=="undefined")       y       =0;
        if(typeof width=="undefined")   width   =0;
        if(typeof height=="undefined")  height  =0;
        return Wing.print(printer,url_or_content,x,y,width,height);
    },
    /**
     * @适合于截屏打印 参考D:\webtop\Release\app\js\selecter.js
     * */
    PrintEx:function(printer,x,y,width,height){
        return Wing.printEx(printer,x,y,width,height);
    },

    ScreenPrint:function(printer,x,y,width,height){
        return Wing.screenPrint(printer,x,y,width,height);
    },

    /**
     * @程序截屏 path 为保存路径  path 为空 ""时 将弹出另存为对话框
     * */
    ToImage:function(path){
        if(typeof path=="undefined")path="";
        Wing.toImage(path);
    },
    /**
     * @局部截屏 path 为空 ""时 将弹出另存为对话框
     * */
    ToImageEx:function(path,start_x,start_y,width,height){
        Wing.toImageEx(path,start_x,start_y,width,height);
    },
    /**
     * @屏幕截屏
     * @path 保存路径 如果为空将弹出保存对话框
     * @start_x,start_y,width,height 截屏的范围 坐标和宽高
     * @to_lipboard 复制到剪切板 1为执行复制到剪切板 0为不复制 如果为1 参数path将被忽略
     * */
    ScreenToImageEx:function(path,start_x,start_y,width,height,to_lipboard){
        if(typeof to_lipboard=="undefined") to_lipboard=0;
        Wing.ScreenToImageEx(path,start_x,start_y,width,height,to_lipboard);
    },
    /**
     * @启用任务栏图标
     * @启用后 当关闭程序时 程序将最小化到任务栏
     * */
    EnableTask:function(){
        Wing.enableTask(1);
    },
    /**
     * @创建一个多进程浏览器 调用此api将打开一个基于多进程的新窗口
     * @参数可以是字符串或者url
     * */
    CreateBrowser:function(content_or_url,x,y,width,height,drag,resize){
        if(typeof x=="undefined")           x       ="0";
        if(typeof y=="undefined")           y       ="0";
        if(typeof width=="undefined")       width   ="0";
        if(typeof height=="undefined")      height  ="0";
        if(typeof drag=="undefined")        drag    ="1";
        if(typeof resize=="undefined")      resize  ="1";
        return Wing.createBrowser(content_or_url,x,y,width,height,drag,resize);
    },
    GetScreenSize:function(){
        return Wing.screen();
    },
    SetSize:function(w,h,handle){
        if(typeof handle!="undefined")handle=0;
        Wing.setSize(w,h,handle);

    },
    ImageBase64Encode:function(path){
        return Wing.imageBase64Encode(path);
    },
    GetImageFromClipboard:function(){
        var data=Wing.getImageFromClipboard();
        if(data==""){
            return;
        }
        return '<img src="data:image/png;base64,'+data+'"/>';
    },
    /**
     * @获取剪切板的文本数据
     * */
    GetTextFromClipboard:function(){
        return Wing.getTextFromClipboard();
    },
    /**
     * *@将为本设置到剪切板
     * */
    SetTextToClipboard:function(str){
        Wing.setTextToClipboard(str);
    },
    /***
     * *@将图片设置到剪切板 path为图片绝对路径
     * */
    SetBitmapToClipboard:function(path){
        if(typeof path=="undefined")path="";
        return Wing.setBitmapToClipboard(path);
    },
    /**
     * @将指定的字符串转换为二维码图片 返回的是base64编码后的字符串 此处直接返回img标签
     * @由于二维码宽高一致 此处只需要传需要的大小即可 类型为 int 整形数字
     * */
    QrEncode:function(str,width,save_to_file,path){
        if(typeof width=="undefined"||width<=0) width       =600;
        if(typeof save_to_file=="undefined")    save_to_file=0;
        if(typeof path=="undefined")            path        ='';
        var  data= Wing.qrEncode(str,width,save_to_file,path);
        return '<img id="qr-code-test" style="position: absolute;left:0;top:0;z-index: 9999;" src="data:image/png;base64,'+data+'"/>';
    },
    QrDeocde:function(image){
        image = image.replace(/data:image.*base64,/,'');
        return Wing.qrDecode(image);
    },
    HtmlToPdf:function(url,saveToPath,callback){
        addEventListener("OnPdfProcess",function(e){
            //e.CustomEventData "{type:1,process:\"%3d%\",url:\"%s\",path:\"%s\"}"
            if(typeof callback=="function")callback(e.CustomEventData);
        });
        return Wing.htmlToPdf(url,saveToPath);
    },
    HtmlToImage:function(url,saveToPath,callback){
        addEventListener("OnImageProcess",function(e){
            //e.CustomEventData "{type:1,process:\"%3d%\",url:\"%s\",path:\"%s\"}"
            if(typeof callback=="function")callback(e.CustomEventData);
        });
        return Wing.htmlToImage(url,saveToPath);
    },
    TcpConnect:function(server_ip,server_port){
        return Wing.tcpConnect(server_ip,server_port);
    },
    Send:function(msg){
        return Wing.send(msg);
    },
    ClientOnline:function(){
        return Wing.clientStatus();
    }




}