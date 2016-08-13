/**
 * Created by Administrator on 2016/1/25.
 */
var app = {
    drag:true,//使用变量锁 防止频繁调用 enableDrag stopDrag 多进程读取冲突
    //生成guid随机字符串
    guid:function () {
        var s4 = function () {
            return Math.floor((1 + Math.random()) * 0x10000)
                .toString(16)
                .substring(1);
        }
        var uuid = s4() + s4() + '-' + s4() + '-' + s4() + '-' +
            s4() + '-' + s4() + s4() + s4();
        return uuid;
    },
    //设置只是箭头的位置
    setPointerPos: function (time) {
        if($(".realshow .realinfolist").length<=0)return;
        if (typeof time == "undefined")time = 1;
        var tab = $(".tab-pointer").attr("tab");
        if(tab==""){
            tab=$(".realshow .realinfolist").eq(0).attr("tab");
        }
        var target = $(".realinfolist[tab='" + tab + "']");
        var __top =
            target.position().top +
            (target.height() - $(".tab-pointer").height()) / 2
            + parseFloat(target.css("margin-top").replace("px", ""));
        var __left = target.position().left - $(".tab-pointer").width();
        $(".tab-pointer").animate({top: __top + "px", left: __left + "px"}, time,function(){
            $(this).show();
        });
    },
    //窗体大小发生改变时触发事件
    resize: function () {
        var self = this;
        $("body").css("height", ($(window).height()) + "px");

        $("#confirmExit").css("top", (($(window).height() - $("#confirmExit").height()) / 2) + "px")
            .css("left", (($(window).width() - $("#confirmExit").width()) / 2) + "px");//.show();
        $(".popWindow").each(function (i, v) {
            if(typeof $(v).attr("w")=="undefined"){
                $(v).attr("w",$(v).width());
            }
            if(typeof $(v).attr("h")=="undefined"){
                $(v).attr("h",$(v).height());
            }
            var left=(($(window).width() - $(v).width()) / 2);
            left=left<=0?0:left;
            var top = (($(window).height() - $(v).height()) / 2);
            top=top<=0?0:top;
            $(v).css("top",  top+ "px")
                .css("left",  left+ "px");
        });
        self.setPointerPos(1);
    },
    //确认退出程序弹窗
    showConfirmExit: function () {
        $("#publicBg").show();
        $("#confirmExit").css("top", (($(window).height() - $("#confirmExit").height()) / 2) + "px")
            .css("left", (($(window).width() - $("#confirmExit").width()) / 2) + "px").show();
    },
    //快捷方式事件响应
    f5:function(){
        window.location.reload();
    },
    lineHeight:function(){
        return ($("#datalist .list").eq(0).height()+1);
    },
    listHeight:function(){
        return ($(".mwList").height()-$(".mwList ._title").height());
    },
    ///左箭头快捷事件 用于切换上下选中行
    left:function(){
        var self=this;
        var index=$("#datalist .selected").index();
        var len=$("#datalist .list").length;
        if(typeof index=="undefined")index=len;
        index--;
        if(index<0){
            $("#datalist").scrollTop(len*self.lineHeight());
            $("#srolltap").css({"top": self.scrollBarHeight()+"px"});
            index=len-1;
        }
        $("#datalist .list").removeClass("selected");
        $("#datalist .list").eq(index).addClass("selected");

        var x=(len-index)*self.lineHeight()-self.listHeight();
        if(x>0){
            $("#datalist").scrollTop(index*self.lineHeight());
            var s = (index*self.lineHeight())/self.scrollHeight()*self.scrollBarHeight();
            $("#srolltap").css({"top": s + "px"});
        }
    },
    //右箭头 用于切换上下选中行
    right:function(){
        var self=this;
        var index=$("#datalist .selected").index();
        if(typeof index=="undefined")index=-1;
        index++;
        var len=$("#datalist .list").length;

        if(index>len-1){
            $("#datalist").scrollTop(0);
            $("#srolltap").css({"top": "0px"});
            index=0;
        }

        $("#datalist .list").removeClass("selected");
        $("#datalist .list").eq(index).addClass("selected");


        var x = (self.lineHeight()*(index+1))-($("#datalist").scrollTop()+$(".mwList").height()-$(".mwList ._title").height()-7);
        if(x<=0)x=0;

       // var s=Math.ceil(($("#datalist").scrollTop()+x)/self.lineHeight())*self.lineHeight();

        $("#datalist").scrollTop(($("#datalist").scrollTop()+x));
        var s = $("#datalist").scrollTop()/self.scrollHeight()*self.scrollBarHeight();
        $("#srolltap").css({"top": s + "px"});

    },
    //下箭头
    down:function(){
        var self=this;
        var a = -5;
        this.speed -= 0.5;
        //当持续按住向下向上箭头时 加速效果
        a += this.speed;
        $("#datalist").scrollTop($("#datalist").scrollTop() + a);

        var b = $("#datalist").scrollTop() / self.scrollHeight();
        var s = b * self.scrollBarHeight();
        if (s <= 0)s=0;
        $("#srolltap").css({"top": s + "px"});
    },
    //上箭头
    up:function(){
        var self=this;
        var a = 5;

        this.speed += 0.5;

        //当持续按住向下向上箭头时 加速效果
        a += this.speed;
        $("#datalist").scrollTop($("#datalist").scrollTop() + a);
        var b = $("#datalist").scrollTop() /  self.scrollHeight();;
        var s = b * self.scrollBarHeight();
        if (s <= 0)s=0;
        $("#srolltap").css({"top": s + "px"});
    },
    //加速
    speed :0,
    //当前列表可以滚动的高度
    scrollHeight:function(){
        //当前列表可以滚动的高度 列表的高度减去窗口的高度
       return (this.lineHeight() * $("#datalist .list").length)- ($(".mwList").height()-$(".mwList ._title").height());
    },
    //当前滚动条可以滚动的高度
    scrollBarHeight:function(){
        return ($(".mwList").height()-$(".mwList ._title").height() - $("#srolltap").height());
    },
    //列表自定义滚动条
    scroll:function(){
        var self=this;
        //----------------列表自定义滚动条--start---
        $("#datalist").bind('mousewheel', function () {
            //步长
            var step = (window.event.wheelDelta) / 30;
            $(this).scrollTop($(this).scrollTop() - step);

            //计算当前滚动的比例
            var b = ($(this).scrollTop()) / self.scrollHeight();
            //按照比例计算滚动条应该滚动的高度
            var s = b * self.scrollBarHeight();

            if (s < 0)s=0;
            $("#srolltap").css({"top": s + "px"});
        });
        var startdrag = false,iY;
        $("#srolltap").mousedown(function (e) {
            if(self.drag){
                api.DisableDrag();
            }
            self.drag=false;
            startdrag = true;
            iY = window.event.clientY - $("#srolltap").position().top;
        }).mouseup(function () {
            startdrag = false;
            if(!self.drag){
                api.EnableDrag();
            }
            self.drag=true;
        });
        $(document).mousemove(function (e) {
            var e = e || window.event;
            var oY = e.clientY - iY;
            if (!startdrag){
                return false;
            }
            if (oY < 0){
                oY=0;
            }
            if (oY > self.scrollBarHeight()){
                oY=self.scrollBarHeight();
            }

            $("#srolltap").css({"top": oY + "px"});

            var b = oY / self.scrollBarHeight();

            var s = b *  self.scrollHeight();
            $("#datalist").scrollTop(s);
        }).mouseup(function () {
            startdrag = false;

            if (!self.drag){
                api.EnableDrag();
            }

            self.drag=true;
        }).keydown(function (event) {

            if (event.keyCode == 116) {
                self.f5();
            }
            if(event.keyCode==39){
                self.right();
            }
            if(event.keyCode==37){
                self.left();
            }
            if (event.keyCode == 40){
                self.up();
            }
            if(event.keyCode == 38) {
                self.down();
            }
        }).keyup(function () {
            //松开时加速效果恢复
            self.speed = 0;
        });
        //----------------列表自定义滚动条--end---
    },
    addPop:function(){
        var self=this;
        input=$("#phoneNum");
        $("#phoneNum").bind("keyup keydown",function(){
            $(this).val($(this).val().replace(/\D/,""));
            if($(this).val().length>=11){
                $("#peopleNum").focus();
                input=$("#peopleNum");
            }
        });

        $("#peopleNum").bind("keyup keydown",function(){
            $(this).val($(this).val().replace(/\D/,""));
            if($(this).val().length>2){
                $(this).val( $(this).val().substr(0,2));
            }
            self.loadEx($(this).val());
        });

        $(".numpoint span,#zero").click(function(){
            if(input.val().length<11)
                input.val(input.val()+$(this).text());
            else{
                input=$("#peopleNum");
                input.val(input.val()+$(this).text());
            }
            input.keydown();
            input.keyup();
            if(input.attr("id")=="peopleNum")
            self.loadEx(input.val());
        });
        $("#BackSpace").click(function(){
            input.val( input.val().substr(0,input.val().length-1));
            if(input.val()==""){
                if($("#phoneNum").val()!="")input=$("#phoneNum");
                else if($("#peopleNum").val()!="")input=$("#peopleNum");
                else input=$("#phoneNum");
            }

            if(input.attr("id")=="peopleNum")
                self.loadEx(input.val());
        });

        $("#ClearAll").click(function(){
            input.val("");
            if(input.val()==""){
                if($("#phoneNum").val()!="")input=$("#phoneNum");
                else if($("#peopleNum").val()!="")input=$("#peopleNum");
                else input=$("#phoneNum");
            }
            if(input.attr("id")=="peopleNum")
                self.loadEx(input.val());
        });
       $(window).keydown(function(){
            e=window.event;
           // $("#XmppOnlineStatus").html(e.keyCode);

            if((105-e.keyCode)>=0&&(105-e.keyCode)<=9&&!input.is(":focus")){
                input.val(input.val()+(e.keyCode-96));
                if(input.hasClass("phone")){
                    if(input.val().length>=11){
                       // $("#peopleNum").focus();
                        input=$("#peopleNum");
                    }
                }else{
                    if(input.val().length>2){
                        input.val( input.val().substr(0,2));
                    }
                }

                if(input.attr("id")=="peopleNum")
                    self.loadEx(input.val());
                /*input.keydown();
                input.keyup();*/
            }
           if(e.keyCode==8){
               input.val( input.val().substr(0,input.val().length));
               if(input.val()==""){
                   if($("#phoneNum").val()!=""){
                       $("#phoneNum").focus();
                       input=$("#phoneNum");
                   }
                   else if($("#peopleNum").val()!=""){
                       $("#peopleNum").focus();
                       input=$("#peopleNum");
                   }
                   else{
                       $("#phoneNum").focus();
                       input=$("#phoneNum");
                   }
               }
               if(input.attr("id")=="peopleNum")
                   self.loadEx(input.val());
           }

           if (event.keyCode == 40||event.keyCode == 38||event.keyCode == 37||event.keyCode == 39){
              if( $("#phoneNum").is(":focus")){
                  $("#peopleNum").focus();
                  input=$("#peopleNum");
              }else{
                  $("#phoneNum").focus();
                  input=$("#phoneNum");
              }
           }


        });
    },
    bindTableClick:function(){
        var self = this;
        $(".realinfolist").click(function () {
            var tab = $(this).attr("tab");
            $(".tab-pointer").attr("tab", tab);
            self.setPointerPos(300);
            var id=$(this).attr("data-id");
            var sql=' select * from tables where id='+id;
            var data = db.row(sql,api.AppPath()+"data/tables.db");
            if(typeof data!="object")return false;
            if(typeof data.people_min=="undefined")return false;
            self.load(data.people_min,data.people_max);
        });
    },
    //执行程序 绑定和初始化事件
    start: function () {
        var self = this;
        self.resize();

        window.onresize = function () {
            self.resize();
        }


        closeBth.onclick =function(){
            self.showConfirmExit();
        }

        sureExit.onclick = function(){
            api.Quit();
        }
        notExit.onclick = function () {
            $("#publicBg").hide();
            $("#confirmExit").hide();
        }
        addWaitNum.onclick = function () {
            $("#publicBg").show();
            $("#AddNumPop").show();
            $("#phoneNum").focus();
        }
        $(".closeAddWaitPop").click(function () {
            $("#publicBg").hide();
            $("#AddNumPop").hide();
        });
        refresh.onclick = self.f5;
        self.scroll();
        self.listEvent();
        self.addPop();
        $(".minBth").click(function(){
            api.Mini();
        });
        AddNum.onclick=function(){
            var

                phoneNum = $("#phoneNum").val(),
                peopleNum = $("#peopleNum").val(),
                come="本地";

            var waitnum = self.getNum(peopleNum);
            if(waitnum===false){
                return alert("没有适合"+peopleNum+"人的座位，请联系服务员分配！");
            }

            var waitid=waitlist.add(waitnum,phoneNum,peopleNum,come);
            if(waitid===false){
                return false;
            }

            $("#datalist").append(
                '<div waitid="' + waitid + '" class="list" >' +
                '<span class="num">' + waitnum + '</span>' +
                '<span class="phone" title="' + phoneNum + '">' + phoneNum + '</span>' +
                '<span class="peoplenum">' + peopleNum + '</span>' +
                '<span class="time" title="' + window.date.getTime(false) + '">' + window.date.getTime(false) + '</span>' +
                '<span class="come">' + come + '</span></div>'
            );
            if($("#datalist .list.selected").length<=0){
                $("#datalist .list").eq(0).addClass("selected");
            }
            self.loadNextAndTotal();
            self.listEvent();
        },
        ShowConfigDlg.onclick=function(){
            self.loadTables();
            $("#publicBg").show();
            $("#ConfigDlg").show();

            var printers = api.GetPrinters();
            var len = printers.length;
            $("#printerlist").html("");
            for(var i=0;i<len;i++){
                $("#printerlist").append("<span class='printer'><input type=\"checkbox\" id=\"printer_"+i+"\"/><label for=\"printer_"+i+"\">"+printers[i]+"</label></span>");
            }


            var sql='select * from printers';
            var printers = db.query(sql,api.AppPath()+"data/config.db");
            if(typeof printers=="object"||printers.length>0){
                for(var i in printers){
                    $(".printer").each(function(index,v){
                        if($(v).text()==printers[i].name)$(v).find("input:checkbox").attr("checked",true);
                    });
                }

            }


            var sql='select * from print_content';
            var obj_content = db.row(sql,api.AppPath()+"data/config.db");

            if(typeof obj_content=="object"&&typeof obj_content.content!="undefined")
            editor.html(obj_content.content);

        }
        CloseConfigDlg.onclick=function(){
            $("#publicBg").hide();
            $("#ConfigDlg").hide();
        }

        $("#ConfigDlg .tab span").click(function(){
            $("#ConfigDlg .tab span").removeClass("selected");
            $(this).addClass("selected");
            $("#tabcontrol .item").hide();
            $("#tabcontrol .item").eq($(this).index()).show();
        });

        SaveTable.onclick=function(){
            /*
            *   "name NVARCHAR(64)," +
             "people_num INT," +
             "people_min INT," +
             "people_max INT," +
             "start_num INT," +
             "prev NVARCHAR(8)," +
             "createtime NVARCHAR(30));";
            * */
            var name        = $("#table-name").val().trim();
            var people_num  = $("#table-people-num").val().trim();
            var start_num   = $("#table-start-num").val().trim();
            var people_min  = $("#table-people-min").val().trim();
            var people_max  = $("#table-people-max").val().trim();
            var sort        = $("#table-sort").val().trim();
            var prev        = $("#table-prev").val().trim();
            var id          = $("#SaveTable").attr("data-id");
            var status      = false;

            if(name==""||people_num<=0||people_num==""||start_num<=0||start_num==""||people_min<=0||people_min==""||
                people_max<=0||people_max==""){
                return alert("请填写全部参数！");
            }

            if(id=="") {
                status = db.insert("insert into tables(name,people_num,people_min,people_max,start_num,sort,prev,createtime,updatetime)" +
                    " values('" + name + "','" + people_num + "','" + people_min + "'," + people_max + "," + start_num + "," + sort + ",'"+prev+"','" + window.date.getTime(false) + "','')", api.AppPath() + "data/tables.db");
            }else{
                //update
                status = db.update("update tables set "+
                    "name='"+name+"',"+
                    "people_num="+people_num+","+
                    "people_min="+people_min+","+
                    "people_max="+people_max+","+
                    "start_num="+start_num+","+
                    "sort="+sort+","+
                    "prev='"+prev+"' where id="+id, api.AppPath() + "data/tables.db");
            }
            if(status){
                $("#savetabletip").show();
                self.loadTables();
                window.setTimeout(function(){
                    $("#savetabletip").hide();
                },1000);
            }
        }

        $("#cancel-edit").click(function(){
            $("#SaveTable").attr("data-id","");
            $("#show-edit-info").hide();

            $("#table-name").val("");
            $("#table-people-num").val("");
            $("#table-start-num").val("");
            $("#table-people-min").val("");
            $("#table-people-max").val("");
            $("#table-sort").val("");
            $("#edit-table-name").html("");
            $("#table-prev").val("");//.trim();
        });

        remind_1.onclick=function(){
            voice.play(voice.remind1());
        }
        remind_2.onclick=function(){
            voice.play(voice.remind2());
        }
        remind_3.onclick=function(){
            voice.play(voice.remind3());
        }

        callNum.onclick=function(){
            voice.callNum($("#datalist .list.selected").find(".num").text());
        }

        $(".updateStatus").click(function(){
            var status = $(this).attr("data-status");
            var waitid = $("#datalist .list.selected").attr("waitid");
            var num = $("#datalist .list.selected").find(".num").text();

            if(status==1)
                if(!confirm("【"+num+"】确认入座？"))return;

            if(status==4)
                if(!confirm("【"+num+"】确认作废？"))return;

            var sql = 'update waitlist set status ='+ status+' where waitid="'+waitid+'"';
            //alert(sql);
            if( db.update(sql) ){
                var index = $("#datalist .list.selected").index();
                $("#datalist .list.selected").remove();
                $("#datalist .list").eq(0).addClass("selected");
                self.loadNextAndTotal();
            }
        });
        //保存打印机和打印内容
        $("#save-printers").click(function(){
            //打印机表
            db.beginTransaction(api.AppPath()+"data/config.db");
            var sql='delete from printers where 1';
            var error=false;
            if(!db.delete(sql,api.AppPath()+"data/config.db")){
                error=true;
            }
            var sql='delete from print_content where 1';
            if(!db.delete(sql,api.AppPath()+"data/config.db")){
                error=true;
            }

            $(".printer input:checked").each(function(i,v){
                var name= $(v).parents("span").text();
                var sql='insert into printers(name,createtime) values("'+name+'","'+window.date.getTime()+'")';
                if(!db.insert(sql,api.AppPath()+"data/config.db")){
                    error=true;
                }
            });

            var content = editor.html();
            var sql='insert into print_content(content,createtime) values("'+content+'","'+window.date.getTime()+'")';
            if(!db.insert(sql,api.AppPath()+"data/config.db")){
                error=true;
            }

            if(error) db.rollbackTransaction(api.AppPath()+"data/config.db");

            if(!error)$("#saveprinttip").html("保存成功").show();
            else $("#saveprinttip").html("保存失败").show();

            if(!error)db.commitTransaction(api.AppPath()+"data/config.db");

            window.setTimeout(function(){
                $("#saveprinttip").hide();
            },3000);
        });

        //测试打印
        $("#test-printer").click(function(){
            var printer = $(".printer input:checked").eq(0).parents("span").text();
            var content =  editor.html();
            api.Print(printer,content);
        });

        SearchHistoryDlg.onclick=function(){
            $("#publicBg").show();
            $("#history").show();
            $("#searchtext").focus();
        }
        CloseHistoryDlg.onclick=function(){
            $("#publicBg").hide();
            $("#history").hide();
        }

    },
    listEvent:function(){
        $("#datalist .list").click(function(){
            $("#datalist .list").removeClass("selected");
            $(this).addClass("selected");
        });
    },
    loadEx:function(people_num){
        var self=this;
        var sql = "select * from tables where people_min<="+people_num+" and people_max>="+people_num+"  order by sort desc;";
        var data = db.row(sql,api.AppPath()+"data/tables.db");
        if(typeof data!="object")return false;
        else if(typeof data.people_min=="undefined")return false;

        self.load(data.people_min,data.people_max);
    },
    load:function(people_min,people_max){
        var self = this;
        //id,waitid,num,phone,peoplenum,status,come,createtime,onseattime
        var sql = "select * from waitlist where status=0 and peoplenum>="+people_min+" and peoplenum<="+people_max+"  order by id asc;";
        var result = db.query(sql);
        var len = result.length;
        $("#datalist").html("");
        for (var i = 0; i < len; i++) {
            var _item = result[i];
            $("#datalist").append(
                '<div data-id="'+_item.id+'" waitid="' + _item.waitid + '" class="list">' +
                '<span class="num">' + _item.num + '</span>' +
                '<span class="phone" title="' + _item.phone + '">' + _item.phone + '</span>' +
                '<span class="peoplenum">' + _item.peoplenum + '</span>' +
                '<span class="time" title="' + _item.createtime + '">' + _item.createtime + '</span>' +
                '<span class="come">' + _item.come + '</span></div>'
            );
        }
        //selected 选中第一条
        $("#datalist .list:first").addClass("selected");
        self.listEvent();

    },
    bindDelEvent:function(){
        var self=this;
        $(".table-del").click(function(){
            var tr=$(this).parents("tr");
            var table_name = tr.find(".table-name").text();
            if(!confirm("确认删除【"+table_name+"】？")){
                return;
            }
            var id=$(this).attr("data-id");
            var sql = "delete from tables where id="+id+";";
            if(db.delete(sql,api.AppPath()+"data/tables.db")){
                tr.remove();
                self.loadTables();
            }
        });
    },
    bindEditEvent:function(){
        $(".table-edit").click(function(){
            var id=$(this).attr("data-id");
            var tr=$(this).parents("tr");
            var table_name = tr.find(".table-name").text(),
                table_people_num = tr.find(".table-people-num").text(),
                table_start_num = tr.find(".table-start-num").text(),
                table_people_min = tr.find(".table-people-min").text(),
                table_people_max = tr.find(".table-people-max").text(),
                table_sort = tr.find(".table-sort").text(),
                table_prev = tr.find(".table-prev").text();
            $("#table-name").val(table_name);
            $("#table-people-num").val(table_people_num);
            $("#table-start-num").val(table_start_num);
            $("#table-people-min").val(table_people_min);
            $("#table-people-max").val(table_people_max);
            $("#table-sort").val(table_sort);
            $("#SaveTable").attr("data-id",id);
            $("#edit-table-name").html(table_name);
            $("#table-prev").val(table_prev);
            $("#show-edit-info").show();
        });
    },
    loadTables:function(){
        var self = this;
        var sql = "select * from tables order by sort desc;";
        $("#tables-all").html('<tr><td>正在加载...</td></tr>');
        $(".realshow").html('');
        var result =  db.query(sql,api.AppPath()+"data/tables.db");
        var len = result.length;
        $("#tables-all").html('<thead><tr>'+
        '<th><div class="th-inner ">名称</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">人数</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">起始</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">最小</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">小大</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">顺序</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">前缀</div><div class="fht-cell"></div></th>'+
        '<th><div class="th-inner ">操作</div><div class="fht-cell"></div></th>'+
        '</tr></thead><tbody>');
        for (var i = 0; i < len; i++) {
            var _item = result[i];
            $("#tables-all").append('<tr>'+
                '<td class="table-name">'+_item.name+'</td>'+
                '<td class="table-people-num">'+_item.people_num+'</td>'+
                '<td class="table-start-num">'+_item.start_num+'</td>'+
                '<td class="table-people-min">'+_item.people_min+'</td>'+
                '<td class="table-people-max">'+_item.people_max+'</td>'+
                '<td class="table-sort">'+_item.sort+'</td>'+
                '<td class="table-prev">'+_item.prev+'</td>'+
                '<td><a class="table-edit" data-id="'+_item.id+'">编辑</a><a class="table-del" data-id="'+_item.id+'">删除</a></td>'+
                '</tr>'
            );

            $(".realshow").append(
                '<div class="realinfolist table_'+_item.id+'" data-id="'+_item.id+'" tab="table_'+_item.id+'">'+
                    '<div>'+
                        '<label>'+
                        '<span class="nextdis">下一位：</span>'+
                        ' <span id="table1_next_num" class="nextnum"></span>'+
                        '</label>'+
                    '</div>'+
                    '<div>'+
                        '<span id="table1_name" class="tablename">'+_item.name+'</span>'+
                        '<span class="waitnumdis">排队总数：</span>'+
                        '<span id="table1_waitnumlen" class="waitnumlen"></span>'+
                    '</div>'+
                '</div>');
        }
        $("#tables-all").append('</tbody>');
        $(".realinfolist").css("height",(100/$(".realinfolist").length-1)+"%");
        self.bindDelEvent();
        self.bindEditEvent();
        self.bindTableClick();
        self.loadNextAndTotal();
    },
    loadNextAndTotal:function(){


        var sql = "select * from tables order by sort desc;";
        var result =db.query(sql,api.AppPath()+"data/tables.db");
        var len =result.length;
        for (var i = 0; i < len; i++) {
            var _item = result[i];
            var table =$(".table_"+_item.id);
            var sql = "select * from waitlist where status=0 and peoplenum>="+_item.people_min+" and peoplenum<="+_item.people_max+"  order by id asc;";
            var list = db.query(sql);
           // alert(list.length);
            table.find(".waitnumlen").html(list.length);
            if(list.length>0)
            table.find(".nextnum").html(list[0].num);
        }



    },
    getNum:function(people_num){
        var sql=' select * from tables where people_min<='+people_num+' and people_max>='+people_num+' order by sort desc';
        var data = db.row(sql,api.AppPath()+"data/tables.db");
        if(typeof data!="object")return false;

        sql='select * from waitlist where peoplenum>='+data.people_min+' and peoplenum<='+data.people_max+' order by id desc';
        var wdata = db.row(sql);
        if(typeof wdata!="object"){
            return data.prev+data.start_num;
        }

        var num =(parseInt(wdata.num.replace(/\D/,""))+1)+"";
        return data.prev+num;

    },

    //程序初始化完毕事件
    appReady: function () {
        var self=this;
        db.init(api.AppPath()+"data/mw_"+window.date.getDayEx()+".db");
        waitlist.setDb(db);
        config.setConfig(api.AppPath()+"config/config.ini");
        self.loadTables();
        self.listEvent();

        //初始化完毕之后创建数据表
        var createTable = "CREATE TABLE IF NOT EXISTS waitlist(" +
            "id INTEGER PRIMARY KEY," +
            "waitid NVARCHAR(20)," +
            "num NVARCHAR(20)," +
            "phone NVARCHAR(30)," +
            "peoplenum INT," +
            "status INT," +
            "come NVARCHAR(256) ," +
            "createtime NVARCHAR(30)," +
            "onseattime NVARCHAR(30));";
        db.createTable(createTable);
        createTable = "CREATE TABLE IF NOT EXISTS tables(" +
            "id INTEGER PRIMARY KEY," +
            "name NVARCHAR(64)," +
            "people_num INT," +
            "people_min INT," +
            "people_max INT," +
            "start_num INT," +
            "sort INT," +
            "prev NVARCHAR(8)," +
            "createtime NVARCHAR(30),updatetime NVARCHAR(30) );";
        db.createTable(createTable,api.AppPath()+"data/tables.db");
        //打印机表
        createTable = "CREATE TABLE IF NOT EXISTS printers(" +
            "id INTEGER PRIMARY KEY," +
            "name NVARCHAR(64)," +
            "createtime NVARCHAR(30) );";
        //配置信息数据库
        db.createTable(createTable,api.AppPath()+"data/config.db");
        createTable = "CREATE TABLE IF NOT EXISTS print_content(" +
            "id INTEGER PRIMARY KEY," +
            "content NVARCHAR(64)," +
            "createtime NVARCHAR(30) );";
        //配置信息数据库
        db.createTable(createTable,api.AppPath()+"data/config.db");
        var sql=' select * from tables order by sort desc';
        var data = db.row(sql,api.AppPath()+"data/tables.db");
        self.load(data.people_min,data.people_max);
        self.setPointerPos();
        //api.ToImage("");
        //document.body.clientWidth
       // document.body.clientHeight; "D:/webtop/Release/app/temp/client.html");//
       // api.Print("Microsoft Print to PDF","http://www.chinadmd.com/search.do?nkey=%E8%BF%90%E7%BB%B4%E4%BA%8B%E6%95%85%E6%8A%A5%E5%91%8A%E6%A8%A1%E6%9D%BF");
        api.EnableTask();
        _drag.fix();
        api.EnableDrag();

        //实现软件界面截屏
        //selecter.init();

        //打印支持
        //api.Print("Microsoft Print to PDF","http://www.jb51.net/article/55954.htm",20,20,100,100);

        //api.SetTextToClipboard("如下两句实现屏幕截图");
       // api.SetBitmapToClipboard("C:\\Users\\Administrator\\Desktop\\11.png");
        //如下两句实现屏幕截图
        var screen = api.GetScreenSize();
       // api.CreateBrowser(api.AppPath()+"app\\richTextBox.html", screen.width/4,screen.height/4,screen.width/2,screen.height/2,1,1);
        //api.CreateBrowser(api.AppPath()+"app\\screenshot.html", 0,0,screen.width,screen.height,0,0);
       // api.QrEncode("http://www.baidu.com/",600,1,"D:/1.jpg");

       // $("body").append(api.QrEncode("http://WWW.baidu.com/你好1111111123hellp",600,1,'D:/1.jpg'));
      //  $("#qr-code-test").click(function(){
         //   alert(api.QrDeocde("D:/1.jpg"));
       // });

       // var clibard=api.GetTextFromClipboard();// XPS Document Writer
       // alert(clibard);
       // Wing.createPrinter("Microsoft Print to PDF","http://blog.csdn.net/jiftlixu/article/details/7202660");
       /* $("body").prepend("<div id=\"process\">"+data.process+"</div>");
        api.HtmlToImage("http://www.yonglibao.com/","D:/1234.jpg",function(data){
            $("#process").html(data.process);
        });


        $("body").prepend("<div id=\"process2\">"+data.process+"</div>");
        api.HtmlToPdf("http://www.yonglibao.com/","D:/1234.pdf",function(data){
            $("#process2").html(data.process);
        });*/

      /*  api.HtmlToImage("http://www.yonglibao.com/","D:/1234.pdf",function(data){
            $("#process").html(data.process);
        });*/

    },
    error:function(sMessage,sUrl,sLine){
        var error = "js错误："+sMessage+"文件："+sUrl+"行："+sLine+"，请联系管理员处理！";
        this.log(error);
        alert(error);
    },
    log:function(content){
        var file=api.AppPath()+"log/js_error_"+window.date.getDayEx()+".log";
        api.Mkdir(api.AppPath()+"log");
        return api.WriteFile(file,"时间："+window.date.getTime()+" "+content+"\r\n");
    },
    //断网事件 掉一次 触发一次
    onNetClose:function(){
        //alert("断网");
        this.log("网络异常掉线");
    },
    //当断网后 重新联网 触发
    onNetOpen:function(){
       // alert("连网");
        this.log("网络异常掉线后恢复正常");
    }

}

$(document).ready(function () {
    app.start();
    addEventListener("WingReady", function(){
        app.appReady();
    });
    window.onerror=function(sMessage,sUrl,sLine){
        app.error(sMessage,sUrl,sLine);
    }
    //断网事件
    addEventListener("WingOnNetClose",function(){
        app.onNetClose();
    });

    addEventListener("WingOnNetOpen",function(){
        app.onNetOpen();
    });
});