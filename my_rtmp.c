#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<netdb.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
  
#include "/opt/xuetong/rtmpdump/librtmp/rtmp.h"  
#include "/opt/xuetong/rtmpdump/librtmp/log.h"  
  
int printfAVal(const AVal al)  
{  
    int i = 0;  
    for(i = 0;i <al.av_len;i++)  
        printf("%c",al.av_val[i]);  
    printf("\n");  
}  
  
const char RTMPProtocolStringsLower_1[][7] = {  
  "rtmp",  
  "rtmpt",  
  "rtmpe",  
  "rtmpte",  
  "rtmps",  
  "rtmpts",  
  "",  
  "",  
  "rtmfp"  
};  
  
#define DEF_TIMEOUT 30  /* seconds */  
  
  
int main(int argc,char * argv[])  
{  
    int Ret = -1;  
    RTMP my_rtmp;  
    AVal Host, App, Playpath;  
    unsigned int Port = 0;  
    int Protocol = RTMP_PROTOCOL_UNDEFINED;  
      
    AVal sockshost = { 0, 0 };  
    AVal tcUrl = { 0, 0 };  
    AVal swfUrl = { 0, 0 };  
    AVal pageUrl = { 0, 0 };  
    AVal auth = { 0, 0 };  
    AVal swfSHA256Hash = { 0, 0 };  
    AVal flashVer = { 0, 0 };  
    AVal subscribepath = { 0, 0 };  
    AVal usherToken = { 0, 0 };  
    uint32_t swfSize = 0;  
    uint32_t dSeek = 0;       // seek position in resume mode, 0 otherwise  
    int bLiveStream = FALSE;  // is it a live stream? then we can't seek/resume  
    uint32_t dStopOffset = 0;  
    long int timeout = DEF_TIMEOUT;   // timeout connection after 120 seconds  
  
      
    int fd = 0;  
  
    char *input_rtmp_url = NULL;  
    char RTMP_RUL[] = "rtmp://live.hkstv.hk.lxdns.com/live/hks";  
    if(argv[1]==NULL){  
        input_rtmp_url = RTMP_RUL;  
    }else{  
        input_rtmp_url = argv[1];  
    }  
          
    printf("run %s\n",(char*)argv[0]);  
    printf("input_rtmp_url == %s\n",input_rtmp_url);  
  
  
  
    RTMP_Init(&my_rtmp);  
  
    //InitSockets();  
      
    Ret = RTMP_ParseURL(input_rtmp_url, &Protocol, &Host, &Port,  
        &Playpath, &App);  
    if(Ret == TRUE){  
        printfAVal(Host);  
        printfAVal(App);  
        printfAVal(Playpath);  
        printf("%d\n",Port);  
    }else{  
        printf("url(%s) con`t parsed!\n",input_rtmp_url);\  
        goto EXIT;  
    }  
      
    if (Port == 0)  
      {  
        if (Protocol & RTMP_FEATURE_SSL)  
      Port = 443;  
        else if (Protocol & RTMP_FEATURE_HTTP)  
      Port = 80;  
        else  
      Port = 1935;  
      }  
  
    if (tcUrl.av_len == 0)  
      {  
        tcUrl.av_len = strlen(RTMPProtocolStringsLower_1[Protocol]) +  
          Host.av_len + App.av_len + sizeof("://:65535/");  
        tcUrl.av_val = (char *) malloc(tcUrl.av_len);  
        if (!tcUrl.av_val)  
          return -1;  
        tcUrl.av_len = snprintf(tcUrl.av_val, tcUrl.av_len, "%s://%.*s:%d/%.*s",  
             RTMPProtocolStringsLower_1[Protocol], Host.av_len,  
             Host.av_val,Port, App.av_len, App.av_val);  
      }  
      
    RTMP_SetupStream(&my_rtmp,  
             Protocol,  
             &Host,  
             Port,  
             &sockshost,  
             &Playpath,  
             &tcUrl,  
             &swfUrl,  
             &pageUrl,  
             &App,  
             &auth,  
             &swfSHA256Hash,  
             swfSize,  
             &flashVer,  
             &subscribepath,  
             &usherToken,  
             dSeek,  
             dStopOffset,bLiveStream, timeout);  
  
    RTMP_Connect(&my_rtmp,NULL);  
  
    RTMP_ConnectStream(&my_rtmp,dSeek);  
  
  
    fd = open("test.flv",O_CREAT|O_RDWR);  
      
    if(fd){  
        char buf[1024*1024] = {0};  
        while(1){  
            memset(buf,0,1024*1024);  
            Ret = RTMP_Read(&my_rtmp,buf,1024*1024);  
            printf("read size %d\n",Ret);  
            if(Ret <= 0)  
                break;  
            else{  
                write(fd,buf,Ret);  
            }  
        }  
    }  
EXIT:  
    if(fd)  
        close(fd);  
    RTMP_Close(&my_rtmp);  
    return 0;  
}  
