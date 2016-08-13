/**
 * Created by Administrator on 2016/1/26.
 */
var db={
    path:null,
    init:function(dbpath){
        this.path=dbpath;
    },
    //return bool
    createTable:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.CreateTable(_path,sql);
    },
    insert:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.Exec(_path,sql);
    },
    update:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.Exec(_path,sql);
    },
    delete:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.Exec(_path,sql);
    },
    query:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.Query(_path, sql);
    },
    row:function(sql,_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.Query(_path,sql)[0];
    },
    getLastError:function(){
        return api.LastSqlError();
    },
    beginTransaction:function(_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.BeginTransaction(_path);
    },
    commitTransaction:function(_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.CommitTransaction(_path);
    },
    rollbackTransaction:function(_path){
        if(typeof _path=="undefined")_path=this.path;
        return api.RollbackTransaction(_path);
    }

}