/**
 * Created by Administrator on 2016/2/23.
 */
$(document).keydown(function (event) {
    if (event.keyCode == 116) {
        window.location.reload();
    }

    if(event.keyCode==86){
        //alert('你按下了CTRL+V');
        //return false;
        window.setTimeout(function(){
           /* $("img").each(function(i,v){
                var src=$(v).attr("src").replace("file:///","");
                var encode=api.ImageBase64Encode(src);
                if(encode!="")
                $(v).attr("src","data:image/png;base64,"+encode);
                //var src=$(v).attr("src").replace("file:///","");
                //api.WriteFil("D:/1.html",$("body").html());
                //$("body").append(src);
            });*/
            $("#RichTextbox").append(api.GetImageFromClipboard());
        },500);
    }
})