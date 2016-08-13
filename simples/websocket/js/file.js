/**
 * Created by Administrator on 2016/1/27.
 */
var file={
    //判断文件是否存在 true存在 false 不存在
    exists:function(path){
        return api.FileExists(path);
    },
    //创建文件 return true false
    create:function(path){
        return api.CreateFile(path);
    },
    dirExists:function(dir){
        return api.DirExists(dir);
    },
    mkdir:function(dir){
        return api.Mkdir(dir);
    },
}