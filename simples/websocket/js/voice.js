/**
 * Created by Administrator on 2016/2/6.
 */
var voice={
    remind1:function(){
        return voice.path()+"voice/w1.wav"
    },
    remind2:function(){
        return voice.path()+"voice/w2.wav"
    },
    remind3:function(){
        return voice.path()+"voice/w3.wav"
    },
    initAudio:function(){
        var _audio;
        if(window['Audio'] && (_audio=document.createElement('audio'))){
            _audio.autoplay=true;
            _audio.addEventListener('error',voice.onError,false);
            _audio.addEventListener('ended',voice.onEnded,false);
            _audio.id="voice-play";

            _audio.volume=1;
            $("#voice-play").remove();
            window.document.body.appendChild(_audio);

            if(voice.audioEl!=null){
                voice.time=0;
                voice.audioEl.pause();
                voice.audioEl=null;
            }
            voice.audioEl=_audio;
        }
    },
    path:function(){
        return "file:///"+api.ExePath();
    },
    audioEl:null,
    prevVolume:null,
    time:0,//延时播放 毫秒
    play:function(path){
        voice.initAudio();
        if(voice.time>0){
            window.setTimeout(function(){ voice.audioEl.src=path;},voice.time);
        }
        else{
            voice.audioEl.src=path;
        }
    },
    list:[],
    index:0,
    playlist:function(num){
        voice.list=[];
        var n = num.split("");
        voice.list.push(voice.path()+"voice/y.wav");
        for(var i=0;i<n.length;i++){
            voice.list.push(voice.path()+"voice/"+n[i]+".wav");
        }
        voice.list.push(voice.path()+"voice/h.wav");
        voice.index=0;
    },
    callNum:function(num){
        voice.initAudio();
        voice.playlist(num);
        voice.play(voice.list[0]);
    },
    clear:function(){
        voice.list=[];
        $("#voice-play").remove();
    },
    onEnded:function(){
        voice.index++;
        if(voice.index>voice.list.length-1){
            voice.clear();
            return false;
        }
        $("#voice-play").remove();
        voice.play(voice.list[voice.index]);
    },
    onError:function(){
        voice.onEnded();
    }
};