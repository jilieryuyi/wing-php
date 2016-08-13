/**
 * Created by Administrator on 2016/3/29.
 */
var im={
    my:null,
    online:false,
    msg_count:0,
    showStatus:function(str){
      $(".client-status").html(str);
    },
    sendMessage:function(sendto,msg){
        this.log("\nsend msg==>sendto:"+sendto+"\nmsg:"+msg);
      //  if(this.my==null){
         //   return false;
       // }
       // if(this.online==false){
           // return false;
      //  }
        var msg_id = sendto+"-"+(new Date().getTime());
        var _msg='{"service":"sendMessage","to":"'+sendto+'","msg":"'+encodeURIComponent(msg)+'","msg_id":"'+msg_id+'"}';

        $(".msg-win").append(
            '<div msg-id="'+msg_id+'" class="msg-list" style="bottom:'+(this.msg_count*30)+'px;text-align: right;">'+
            '<div class="msg-time">'+window.date.getTime()+'</div>'+
            '<div class="msg-from">my</div>'+
            '<div class="msg-content">'+
            this.msg_count+"=>"+ msg+
            '</div>'+
            '</div>');
        this.msg_count++;
        $(".msg-win").scrollTop(999999);
        return api.Send(_msg);
    },
    login:function(username,password){
        var _msg='{"service":"login","username":"'+username+'","password":"'+password+'"}';
        return api.Send(_msg);
    },
    onDisConnect:function(){
        this.log("onDisConnect");
        this.online=false;
        this.showStatus("离线");
    },
    onConnect:function(){
        this.log("onConnect");
        this.online=true;
        this.showStatus("在线");
    },
    onLogin:function(status,err_msg,user_id){
        this.log("\nlogin==>status:"+status+"\nerr_msg:"+err_msg+"\nuser_id:"+user_id);
        if(status==1000)
        {
            //登陆成功
            this.my=user_id;
            //this.onMessage("system","system","login success");
            im.onConnect();
        }
        else{
            //登陆失败 用户名或者密码错误
            //alert(err_msg);
        }
    },
    onMessage:function(from,username,msg){

        this.log("\nrecive msg==>from:"+from+"\nusername:"+username+"\nmsg:"+msg);
        $(".msg-win").append(
            '<div class="msg-list" style="bottom:'+(this.msg_count*30)+'px;">'+
            '<div class="msg-time">'+window.date.getTime()+'</div>'+
            '<div class="msg-from">'+username+'</div>'+
            '<div class="msg-content">'+
            this.msg_count+"=>"+ decodeURIComponent(msg)+
            '</div>'+
            '</div>');
        this.msg_count++;
        $(".msg-win").scrollTop(999999);
    },
    log:function(content){
        var file=api.AppPath()+"log/socket_"+window.date.getDayEx()+".log";
        api.Mkdir(api.AppPath()+"log");
        return api.WriteFile(file,"时间："+window.date.getTime()+" "+content+"\r\n");
    },
    onSendError:function(msg){
        /*$(".msg-list").each(function(i,v){
            if($(v).attr("msg-id")==msg.msg_id)
                $(v).css("background","#f00");
        });*/
    }
};

var message_temp = [];
addEventListener("OnWingMessage",function(e){
    var msg =e.EventMsg;//eval("("+e.EventMsg+")");
    /*if(typeof msg.user_id=="undefined"){
        msg.user_id=0;
    }*/
    switch(msg.service){
        case "login":{
            im.onLogin(parseInt(msg.status),msg.msg,msg.user_id);
            im.onMessage("system","system",msg.msg);
        }break;
        case "sendMessage":{
            if(typeof msg.step!="undefined"&&msg.max!="undefined"){
                if(typeof message_temp[msg.from]=="undefined"){
                    message_temp[msg.from]="";
                }
                message_temp[msg.from]+=msg.msg;

                im.onMessage("length", "length", message_temp[msg.from].length);

                var step = msg.step;
                var max  = msg.max;
                if(step>=max){
                    im.onMessage(msg.from, msg.username, message_temp[msg.from]);
                    message_temp[msg.from]="";
                }
            }else {
                im.onMessage(msg.from, msg.username, (msg.msg));
            }
        }break;
        case "onLogin":{
            im.onMessage("system","system",(msg.user_id+":"+msg.msg));
        }break;
        case "onClose":{
            im.onMessage("system","system",(msg.user_id+":"+msg.msg));
        }break;
        case "onSendError":
            im.onSendError(msg);
            break;
    }
});

/*

function OnWingMessage(msg){
    if(typeof msg.user_id=="undefined"){
        msg.user_id=0;
    }
    switch(msg.service){
        case "login":{
            im.onLogin(parseInt(msg.status),msg.msg,msg.user_id);
            im.onMessage("system","system",msg.msg);
        }break;
        case "sendMessage":{
            im.onMessage(msg.from,msg.username,(msg.msg));
        }break;
        case "online":{
            im.onMessage("system","system",(msg.user_id+":"+msg.msg));
        }break;
        case "offline":{
            im.onMessage("system","system",(msg.user_id+":"+msg.msg));
        }break;
    }
}
*/

addEventListener("WingReady", function() {

    api.EnableTask();
    window.setInterval(function(){
        //$(".msg-win").append(window.date.getTime()+"=>"+api.ClientOnline()+"<br/>");
        if(!api.ClientOnline()){
            //连接服务器
            api.TcpConnect("114.55.56.167", 2347);
        }
       // Wing.send("\0");
    },1000);

    if(api.ClientOnline()){
        im.onConnect();
    }
    //连接服务器
    api.TcpConnect("114.55.56.167", 2347);

});

addEventListener("onWingDisConnect",function(){
    im.onDisConnect();
});

addEventListener("onWingConnect",function(){
    im.login("root", "123456");
   /* for(var i=0;i<100;i++)
    im.sendMessage("0","是大法官的sdfgsdfg2345234523523452435132452345345129999999");*/
});


$(document).ready(function(){
    $(".send-bth").click(function(){
        im.sendMessage(2,$(".send-msg").html());
        $(".send-msg").html("");
    });
});


/*

$(document).ready(function(){
    var client=true;
    if(typeof api=="undefined"){
        client=false;
    }else if(typeof api.TcpConnect=="undefined"){
        client=false;
    }

    if(!client){
       var ws = new WebSocket("ws://114.55.56.167:2348");
        ws.onopen = function() {
        };
        ws.onmessage = function(e) {
            alert("收到服务端的消息：" + e.data);
        };
    }

});*/
