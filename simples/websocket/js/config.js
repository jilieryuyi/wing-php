/**
 * Created by Administrator on 2016/1/26.
 */
var config={
    file:null,
    setConfig:function(file){
        this.file=file;
    },
    //返回所有的配置信息 或者指定的key
    //return string or object
    get:function(tab,key){
       return api.ReadConfig(tab,key,this.file);
    },
    //return bool 文件不存在时自动创建 但是目录必须事先存在
    write:function(tab,key,value){
        return api.WriteConfig(tab,key,value,this.file);
    }
}