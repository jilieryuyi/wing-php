/**
 * Created by Administrator on 2016/1/26.
 */
var waitlist = {
    db: null,
    lastNum: 0,
    //提供依赖注入接口
    setDb: function (_db) {
        this.db = _db;
    },
    guid: function () {
        var s4 = function () {
            return Math.floor((1 + Math.random()) * 0x10000)
                .toString(16)
                .substring(1);
        }
        var uuid = s4() + s4() + '-' + s4() + '-' + s4() + '-' +
            s4() + '-' + s4() + s4() + s4();
        return uuid;
    },
    add: function (waitnum, phoneNum, peopleNum, come) {
        var self = this;
        var waitid = self.guid();
        var status = this.db.insert("insert into waitlist(waitid,num,phone,peoplenum,status,come,createtime,onseattime)" +
            " values('" + waitid + "','" + waitnum + "','" + phoneNum + "'," + peopleNum + ",0,'本地','" + window.date.getTime(false) + "','')");
        if (!status)return false;
        return waitid;
    },
    getLastNum: function () {
        var sql = "select id,waitid,num,phone,peoplenum,status,come,createtime,onseattime from waitlist where status=0 order by id desc;";
        var result = db.query(sql);
        return result[0]["num"];
    },
    search: function (keyword) {
        keyword = keyword.trim();
        var sql = "select waitid,num,phone,peoplenum,status,come,createtime,onseattime from waitlist where 1";
        if (keyword != "") {
            sql += " and (";
            sql += " num like '%" + keyword + "%' or";
            sql += " phone like '%" + keyword + "%' or";
            sql += " peoplenum like '%" + keyword + "%' or";
            sql += " status like '%" + keyword + "%' or";
            sql += " come like '%" + keyword + "%' or";
            sql += " createtime like '%" + keyword + "%' or";
            sql += " onseattime like '%" + keyword + "%'";
            sql += ")";
        }
        sql += " order by id asc";
        sql = sql + ";";
        return db.query(sql);
    }


}