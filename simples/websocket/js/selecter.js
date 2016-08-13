/**
 * Created by Administrator on 2016/2/17.
 * @屏幕截取器
 * @可以将软件屏幕部分保存截图或者打印
 * @调用api selecter.init() 即可使用鼠标拖拽选择区域 保存为图片或者打印
 */
var selecter={
    init:function(){
        $("body").append(

        '<div class="selecter-background" id="selecter-lefter" '+
        'style="width: 0;height: 100%; position: absolute; left: 0; top:0; background: #000; opacity:0.6;">'+
        ' </div>'+
        ' <div class="selecter-background" id="selecter-righter" style="width: 0;height: 100%; position: absolute; ' +
        ' background: #000; opacity:0.6;">'+
        '   </div>'+
        '   <div class="selecter-background" id="selecter-toper" style="width: 0;height: 0; position: absolute; ' +
        'left: 0; top:0; background: #000; opacity:0.6;">'+
        '   </div>'+
        '   <div class="selecter-background" id="selecter-bottomer" style="width: 0;height: 0; position: absolute; ' +
        'left: 0; top:0; background: #000; opacity:0.6;">'+
        '   </div>'+
        '   <div id="selecter-123456789000" style=" position:absolute; width: 0px;'+
        '  height: 0px; left: 0px; top:0px; opacity: 0;">'+
        ' <div style="width: 100%; height: 100%; background: #fff; "></div>'+
        '     </div>'+
        '    <div id="selecter-function-1234567890" style=" display: none; color: #f00; text-align: right;'+
        ' position:absolute; width: 200px; height: 50px; left: 0px; top:0px; opacity: 1;">'+
        ' <div><button id="selecter-function-cancel" class="no-selecter-1234567890">取消</button><button id="selecter-function-print" class="no-selecter-1234567890">打印</button>'+
        '   <button class="no-selecter-1234567890" id="selecter-save-image">保存图片</button></div>'+
        '    </div>'
        );
        api.DisableDrag();
        var select = $("#selecter-123456789000");
        var select_function =$("#selecter-function-1234567890");
        var start_x =0;
        var start_y =0;
        var start_select=false;
        var width= 0,height=0;
        $(document).mousedown(function(){
            if($(window.event.target).hasClass("no-selecter-1234567890"))return;
            var x=window.event.clientX;
            var y=window.event.clientY;
            start_x=x;
            start_y=y;
            select.css({"left":x+"px","top":y+"px"});
            start_select=true;
        }).mouseup(function(){
            start_select=false;
        }).mousemove(function(){
            if(!start_select)return;
            var x=window.event.clientX;
            if(x>$(document).width())return;
            var y=window.event.clientY;
            if(y>$(document).height())return;
            width=(x-start_x);
            height=(y-start_y);
            select.css({"width":(x-start_x)+"px","height":(y-start_y)+"px"});
            select_function.css({"left":(x-200)+"px","top":(y)+"px"}).show();

            $("#selecter-lefter").css({"width":start_x+"px",top:0,left:0});
            $("#selecter-righter").css({"width":($(document).width()-x)+"px",top:0,right:0});
            $("#selecter-toper").css({"width":width+"px","left":start_x+"px","height":start_y+"px"});
            $("#selecter-bottomer").css({"width":width+"px","left":start_x+"px",
                "height":(window.document.body.clientHeight-y)+"px",top:y+"px"});

        });

        $("#selecter-function-print").click(function(){
            api.PrintEx("Microsoft Print to PDF",start_x,start_y,width,height);
            window.event.stopPropagation();
            return false;
        });

        $("#selecter-save-image").click(function(){
           // var path=api.AppPath()+"testsaveimage.png";
            api.ToImageEx("",start_x,start_y,width,height);
           // alert("图片已保存到"+path);
            window.event.stopPropagation();
            return false;
        });

        $("#selecter-function-cancel").live("click",function(){
            start_select=false;
            select.css({"width":"0px","height":"0px"});
            select.css({"left":"0px","top":"0px"});
            select_function.css({"left":"0px","top":"0px"}).hide();

            $("#selecter-lefter").css({"width":"0px",top:0,left:0});
            $("#selecter-righter").css({"width":"0px",top:0,right:0});
            $("#selecter-toper").css({"width":"0px","left":"0px","height":"0px"});
            $("#selecter-bottomer").css({"width":"0px","left":"0px",
                "height":"0px",top:"0px"});
        });

    }
}
