/**
 * Created by Administrator on 2016/1/28.
 */
var http={
    /**
     * @跨越同步post api
     * @url post 目标url
     * @data 将要发送的数据 如 a=1&b=2
     * @header object 自定义http头 如 {"a":"abc","Accept":"yuyi"}
     * @（以php为例）服务端将收到 $_SERVER["HTTP_A"]="abc" $_SERVER["HTTP_ACCEPT"]="yuyi"
     * @with_headers 将返回内容和http消息头 默认为0 不返回http消息头 1返回消息头
     * @return object 包含body 和header两部分
     **/
    post:function(url,data,header,callback){
        return api.Post(url,data,header,callback);
    }
}