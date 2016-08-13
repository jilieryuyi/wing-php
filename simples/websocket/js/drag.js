/**
 * Created by Administrator on 2016/2/16.
 */
var _drag={
    fix:function(){
        $("input,textarea,.no-drag,.ke-content").mousedown(function(){
            api.DisableDrag();
        }).mouseup(function(){
            api.EnableDrag();
        });
    }
}