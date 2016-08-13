/**
 * Created by Administrator on 2016/2/17.
 * @屏幕截取器
 * @可以将软件屏幕部分保存截图或者打印
 * @调用api selecter.init() 即可使用鼠标拖拽选择区域 保存为图片或者打印
 */
var selecter={
    init:function(){
        $("body").append(
'<div id="selecter-bg" style="position: absolute; left: 0; top:0;width: 100%;height: 100%; background:#000; opacity: 0.6;"></div>'+
'<div id="selecter-selected" style="position: absolute; left: 0; top:0;width: 0%;height: 0%; background:#000; opacity: 0.01;"></div>'+
'<div class="selecter-background" id="selecter-lefter" '+
        'style="width: 0%;height: 100%; position: absolute; left: 0; top:0; background: #000; opacity:0.6;">'+
        ' </div>'+
        ' <div class="selecter-background" id="selecter-righter" style="width: 0%;height: 100%; position: absolute; ' +
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
        ' position:absolute; width: 300px; height: 50px; left: 0px; top:0px; opacity: 1;">'+
        ' <div>'+
        '<button id="selecter-function-cancel" class="no-selecter-1234567890">取消</button>'+
        '<button id="selecter-function-print" class="no-selecter-1234567890">打印</button>'+
        '<button class="no-selecter-1234567890" id="selecter-save-image">保存图片</button>'+
        '<button class="no-selecter-1234567890" id="selecter-set-to-clipboard-image">完成</button>'+

        '</div>'+
        '</div>'
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
            select_function.css({"left":(x-300)+"px","top":(y)+"px"}).show();

            $("#selecter-lefter").css({"width":start_x+"px",top:0,left:0});
            $("#selecter-righter").css({"width":($(document).width()-x)+"px",top:0,right:0});
            $("#selecter-toper").css({"width":"0px","left":start_x+"px","height":start_y+"px"});
            $("#selecter-bottomer").css({"width":"0px","left":start_x+"px",
                "height":($(document).height()-y)+"px",top:y+"px"});

            $("#selecter-selected").css({"width":width+"px","height":height+"px","left":start_x+"px","top":start_y+"px"});


            start_select=true;
            $("#selecter-bg").hide();
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
            select_function.css({"left":(x-300)+"px","top":(y)+"px"}).show();

            $("#selecter-lefter").css({"width":start_x+"px",top:0,left:0});
            $("#selecter-righter").css({"width":($(document).width()-x)+"px",top:0,right:0});
            $("#selecter-toper").css({"width":width+"px","left":start_x+"px","height":start_y+"px"});
            $("#selecter-bottomer").css({"width":width+"px","left":start_x+"px",
                "height":($(document).height()-y)+"px",top:y+"px"});

            $("#selecter-selected").css({"width":width+"px","height":height+"px","left":start_x+"px","top":start_y+"px"});

        });

        $("#selecter-function-print").click(function(){
            var screen = api.GetScreenSize();
            api.ScreenPrint("Microsoft Print to PDF",start_x*screen.scale,start_y*screen.scale,width*screen.scale,height*screen.scale);
            window.event.stopPropagation();
            return false;
        });

        $("#selecter-save-image").click(function(){
           // var path=api.AppPath()+"testsaveimage.png";
            var screen = api.GetScreenSize();
            api.ScreenToImageEx("",start_x*screen.scale,start_y*screen.scale,width*screen.scale,height*screen.scale);
           // alert("图片已保存到"+path);
            window.event.stopPropagation();
            return false;
        });

        $("#selecter-function-cancel").live("click",function(){
            api.Quit();
        });

        $("#selecter-set-to-clipboard-image").click(function(){
            var screen = api.GetScreenSize();
            api.ScreenToImageEx("",start_x*screen.scale,start_y*screen.scale,width*screen.scale,height*screen.scale,1);
            api.Quit();
            window.event.stopPropagation();
            return false;
        });

    }
}
