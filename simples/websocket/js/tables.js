/**
 * Created by Administrator on 2016/1/27.
 */
var tables={
    db:null,
    //提供依赖注入接口
    setDb:function(_db){
        this.db=_db;
    },
    add:function(tablename,tablestartnum,tablepeoplenum,tablepeoplemin,tablepeoplemax){
        var status = this.db.insert("insert into tables(tablename,tablepre,tablestartnum,tablepeoplenum,tablepeoplemin,tablepeoplemax,tableuse,createtime)" +
            " values('" + tablename + "','" + "" + "','" + tablestartnum + "'," + tablepeoplenum + ","+tablepeoplemin+","+tablepeoplemax+",1,'" + window.date.getTime(false) + "')");
        if(!status)return false;
        return waitid;
    }
}