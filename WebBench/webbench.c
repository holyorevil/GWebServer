/*
* (C) Radim Kolar 1997-2004
* This is free software, see GNU Public License version 2 for
* details.
*
* Simple forking WWW Server benchmark:
*
* Usage:
*   webbench --help
*
* Return codes:
*    0 - sucess
*    1 - benchmark failed (server is not on-line)
*    2 - bad param
*    3 - internal error, fork failed
* 
*/ 

#include "socket.c"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
/* values */
volatile int timerexpired=0;
int speed=0;
int failed=0;
int bytes=0;

/* globals */
int http10=1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
/* Allow: GET, HEAD, OPTIONS, TRACE */
//http请求方法
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"
//存储命令行参数的全局变量，并设置默认值
int method=METHOD_GET;
int clients=1;
int force=0;
int force_reload=0;
int proxyport=80;
char *proxyhost=NULL;
int benchtime=30;

bool keep_alive = false;

/* internal */
//用于父子进程通信的管道
int mypipe[2];
//存放目标服务器的网络地址
char host[MAXHOSTNAMELEN];

//存放请求报文的字节流
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];

//构造长选项，基本与短选项是对应的
static const struct option long_options[]=
{
    {"force",no_argument,&force,1},//当使用长选项--force时，会把1赋值给force变量
    {"reload",no_argument,&force_reload,1},
    {"time",required_argument,NULL,'t'},//当使用--time时，会让getopt_long函数返回字符t，相当于把短选项-t和长选项--time绑定起来
    {"help",no_argument,NULL,'?'},
    {"http09",no_argument,NULL,'9'},
    {"http10",no_argument,NULL,'1'},
    {"http11",no_argument,NULL,'2'},
    {"get",no_argument,&method,METHOD_GET},
    {"head",no_argument,&method,METHOD_HEAD},
    {"options",no_argument,&method,METHOD_OPTIONS},
    {"trace",no_argument,&method,METHOD_TRACE},
    {"version",no_argument,NULL,'V'},
    {"proxy",required_argument,NULL,'p'},
    {"clients",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};

/* prototypes */
static void benchcore(const char* host,const int port, const char *request);
static int bench(void);
static void build_request(const char *url);

static void alarm_handler(int signal)
{
    timerexpired=1;
}	

static void usage(void)
{
    fprintf(stderr,
            "webbench [option]... URL\n"
            "  -f|--force               Don't wait for reply from server.\n"
            "  -r|--reload              Send reload request - Pragma: no-cache.\n"
            "  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
            "  -p|--proxy <server:port> Use proxy server for request.\n"
            "  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
            "  -k|--keep                Keep-Alive\n"
            "  -9|--http09              Use HTTP/0.9 style requests.\n"
            "  -1|--http10              Use HTTP/1.0 protocol.\n"
            "  -2|--http11              Use HTTP/1.1 protocol.\n"
            "  --get                    Use GET request method.\n"
            "  --head                   Use HEAD request method.\n"
            "  --options                Use OPTIONS request method.\n"
            "  --trace                  Use TRACE request method.\n"
            "  -?|-h|--help             This information.\n"
            "  -V|--version             Display program version.\n"
           );
}

int main(int argc, char *argv[])
{
    int opt=0;
    int options_index=0;
    char *tmp=NULL;
    //首先进行命令行参数的处理
    //1.没有输入选项
    if(argc==1)
    {
        usage();
        return 2;
    } 
    
    //2.有输入选项则一个一个解析
    while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?hk",long_options,&options_index))!=EOF )
    {
        switch(opt)
        {
            case  0 : break;
            case 'f': force=1;break;
            case 'r': force_reload=1;break; 
            case '9': http10=0;break;
            case '1': http10=1;break;
            case '2': http10=2;break;
            case 'V': printf(PROGRAM_VERSION"\n");exit(0);
            case 't': benchtime=atoi(optarg);break;	
            case 'k': keep_alive = true;break;     
            case 'p': 
            /* proxy server parsing server:port */
            tmp=strrchr(optarg,':');
            proxyhost=optarg;
            if(tmp==NULL)
            {
                break;
            }
            if(tmp==optarg)
            {
                fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
                return 2;
            }
            if(tmp==optarg+strlen(optarg)-1)
            {
                fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
                return 2;
            }
            *tmp='\0';
            proxyport=atoi(tmp+1);break;
            case ':':
            case 'h':
            case '?': usage();return 2;break;
            case 'c': clients=atoi(optarg);break;
        }
    }

    //选项参数解析完毕后，刚好是读到URL，此时argv[optind]指向URL
    if(optind==argc) {
        fprintf(stderr,"webbench: Missing URL!\n");
        usage();
        return 2;
    }

    if(clients==0) clients=1;
    if(benchtime==0) benchtime=30;
 
    /* Copyright */
    fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
            "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n"
            );
    //构造http请求报文
    build_request(argv[optind]);
 
    // print request info ,do it in function build_request
    /*printf("Benchmarking: ");
 
    switch(method)
    {
        case METHOD_GET:
        default:
        printf("GET");break;
        case METHOD_OPTIONS:
        printf("OPTIONS");break;
        case METHOD_HEAD:
        printf("HEAD");break;
        case METHOD_TRACE:
        printf("TRACE");break;
    }
    
    printf(" %s",argv[optind]);
    
    switch(http10)
    {
        case 0: printf(" (using HTTP/0.9)");break;
        case 2: printf(" (using HTTP/1.1)");break;
    }
 
    printf("\n");
    */

    printf("Runing info: ");

    if(clients==1) 
        printf("1 client");
    else
        printf("%d clients",clients);

    printf(", running %d sec", benchtime);
    
    if(force) printf(", early socket close");
    if(proxyhost!=NULL) printf(", via proxy server %s:%d",proxyhost,proxyport);
    if(force_reload) printf(", forcing reload");
    
    printf(".\n");
    
    return bench();
}

void build_request(const char *url)
{
    char tmp[10];
    int i;

    //bzero(host,MAXHOSTNAMELEN);
    //bzero(request,REQUEST_SIZE);
    memset(host,0,MAXHOSTNAMELEN);
    memset(request,0,REQUEST_SIZE);

    if(force_reload && proxyhost!=NULL && http10<1) http10=1;
    if(method==METHOD_HEAD && http10<1) http10=1;
    if(method==METHOD_OPTIONS && http10<2) http10=2;
    if(method==METHOD_TRACE && http10<2) http10=2;

    //开始填写http请求
    //请求行
    //填写请求方法
    switch(method)
    {
        default:
        case METHOD_GET: strcpy(request,"GET");break;
        case METHOD_HEAD: strcpy(request,"HEAD");break;
        case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
        case METHOD_TRACE: strcpy(request,"TRACE");break;
    }

    strcat(request," ");

    //判断URL的合法性
    //1.URL中没有"://"
    if(NULL==strstr(url,"://"))
    {
        fprintf(stderr, "\n%s: is not a valid URL.\n",url);
        exit(2);
    }
    //2.URL过长
    if(strlen(url)>1500)
    {
        fprintf(stderr,"URL is too long.\n");
        exit(2);
    }
    if (0!=strncasecmp("http://",url,7)) 
    { 
        fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
        exit(2);
    }
    
    /* protocol/host delimiter */
    //定位url中主机名开始的位置
    //比如  http://www.xxx.com/
    i=strstr(url,"://")-url+3;

    //4.在主机名开始的位置找是否有'/'，若没有则非法
    if(strchr(url+i,'/')==NULL) {
        fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
        exit(2);
    }
    
    //判断完URL合法性后继续填写URL到请求行
    
    //无代理时
    if(proxyhost==NULL)
    {
        /* get port from hostname */
        //有端口号时，填写端口号
        if(index(url+i,':')!=NULL && index(url+i,':')<index(url+i,'/'))
        {
            //设置域名或IP
            strncpy(host,url+i,strchr(url+i,':')-url-i);
            //bzero(tmp,10);
            memset(tmp,0,10);
            strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);
            /* printf("tmp=%s\n",tmp); */
            //设置端口号
            proxyport=atoi(tmp);
            if(proxyport==0) proxyport=80;
        } 
        else//无端口号
        {
            strncpy(host,url+i,strcspn(url+i,"/"));
        }
        // printf("Host=%s\n",host);
        strcat(request+strlen(request),url+i+strcspn(url+i,"/"));
    } 
    else//有代理服务器就简单了，直接填就行，不用自己处理
    {
        // printf("ProxyHost=%s\nProxyPort=%d\n",proxyhost,proxyport);
        strcat(request,url);
    }

    if(http10==1)
        strcat(request," HTTP/1.0");
    else if (http10==2)
        strcat(request," HTTP/1.1");
  
    strcat(request,"\r\n");
  
    if(http10>0)
        strcat(request,"User-Agent: WebBench "PROGRAM_VERSION"\r\n");
    if(proxyhost==NULL && http10>0)
    {
        strcat(request,"Host: ");
        strcat(request,host);
        strcat(request,"\r\n");
    }
 
    if(force_reload && proxyhost!=NULL)
    {
        strcat(request,"Pragma: no-cache\r\n");
    }
  
    if(http10>1)
    {
        if (!keep_alive)
            strcat(request,"Connection: close\r\n");
        else
            strcat(request,"Connection: Keep-Alive\r\n");
    }
        
    
    /* add empty line at end */
    if(http10>0) strcat(request,"\r\n"); 
    
    printf("\nRequest:\n%s\n",request);
}

/* vraci system rc error kod */
//父进程的作用：创建子进程，读子进程测试到的数据，然后处理
static int bench(void)
{
    int i,j,k;	
    pid_t pid=0;
    FILE *f;

    /* check avaibility of target server */
    //尝试建立连接一次
    i=Socket(proxyhost==NULL?host:proxyhost,proxyport);//创建客户端并与服务端连接
    if(i<0) { 
        fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
        return 1;
    }
    close(i);//尝试连接成功了，关闭该连接
    
    /* create pipe */
    //建立父子进程通信的管道
    if(pipe(mypipe))
    {
        perror("pipe failed.");
        return 3;
    }

    /* not needed, since we have alarm() in childrens */
    /* wait 4 next system clock tick */
    /*
    cas=time(NULL);
    while(time(NULL)==cas)
    sched_yield();
    */

    /* fork childs */
    //开启子进程
    for(i=0;i<clients;i++)
    {
        pid=fork();
        if(pid <= (pid_t) 0)
        {
                // if (errno == 11 && pid != 0)
                // {
                //     printf("here i = %d\n", i);
                //     while (fork() < 0);
                //     continue;
                // }
            /* child process or error*/
            sleep(1); /* make childs faster */
            break;
        }
    }
    //处理fork失败的情况
    if( pid < (pid_t) 0)
    {
        fprintf(stderr,"problems forking worker no. %d\n",i);
        perror("fork failed.");
        return 3;
    }

    //子进程的执行函数
    if(pid == (pid_t) 0)
    {
        /* I am a child */
        //由子进程来发出请求报文
        if(proxyhost==NULL)
            benchcore(host,proxyport,request);
        else
            benchcore(proxyhost,proxyport,request);

        /* write results to pipe */
        f=fdopen(mypipe[1],"w");
        if(f==NULL)
        {
            perror("open pipe for writing failed.");
            return 3;
        }
        /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
        //向管道中写入该子进程在一定时间内请求成功的次数
        //失败的次数
        //读取到的服务器回复的总字节数
        fprintf(f,"%d %d %d\n",speed,failed,bytes);
        fclose(f);

        return 0;
    } 
    else//父进程执行函数
    {
        //父进程获得管道读端的文件指针
        f=fdopen(mypipe[0],"r");
        if(f==NULL) 
        {
            perror("open pipe for reading failed.");
            return 3;
        }
        
        setvbuf(f,NULL,_IONBF,0);
        
        speed=0;//连接成功的总次数，后面除以时间可以得到速度
        failed=0;//失败的请求数
        bytes=0;//服务器回复的总字节数
        
        //父进程不停的读
        while(1)
        {
            pid=fscanf(f,"%d %d %d",&i,&j,&k);
            if(pid<2)
            {
                fprintf(stderr,"Some of our childrens died.\n");
                break;
            }
            
            speed+=i;
            failed+=j;
            bytes+=k;
        
            /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
            if(--clients==0) break;
        }
    
        fclose(f);

        //统计处理结果
        printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
            (int)((speed+failed)/(benchtime/60.0f)),
            (int)(bytes/(float)benchtime),
            speed,
            failed);
    }
    
    return i;
}

//子进程真正的向服务器发出请求报文并以其得到此期间的相关数据
void benchcore(const char *host,const int port,const char *req)
{
    int rlen;
    char buf[1500];
    int s,i;
    struct sigaction sa;

    /* setup alarm signal handler */
    //安装闹钟信号的处理函数
    sa.sa_handler=alarm_handler;
    sa.sa_flags=0;
    if(sigaction(SIGALRM,&sa,NULL))
        exit(3);

    //设置闹钟函数  
    alarm(benchtime); // after benchtime,then exit

    rlen=strlen(req);

    //printf("keep_alive\n");


    if (keep_alive)//长连接
    {
        while ((s = Socket(host,port)) == -1);  
        nexttry1:while(1)
        {
            if(timerexpired)
            {
                if(failed>0)
                {
                    /* fprintf(stderr,"Correcting failed by signal\n"); */
                    failed--;
                }
                return;
            }
           
            if(s<0) { failed++;continue;} 
            if(rlen!=write(s,req,rlen)) {
                failed++;
                close(s);
                while ((s = Socket(host,port)) == -1);
                continue;
            }
            if(force==0) 
            {
                /* read all available data from socket */
                while(1)
                {
                    if(timerexpired) break; 
                    i=read(s,buf,1500);
                    /* fprintf(stderr,"%d\n",i); */
                    if(i<0) 
                    { 
                        failed++;
                        close(s);
                        //while ((s = Socket(host,port)) == -1);
                        goto nexttry1;
                    }
                    else
                    if(i==0) break;
                    else
                    bytes+=i;
                    // Supposed reveived bytes were less than 1500
                    //if (i < 1500) 
                        break;
                }
            }
            speed++;
        }
    }
    else //短连接
    {
        nexttry:while(1)
        {
            //只有在收到闹钟信号后会使 timerexpired = 1
            //即该子进程的工作结束了

            if(timerexpired)
            {
                if(failed>0)
                {
                    /* fprintf(stderr,"Correcting failed by signal\n"); */
                    failed--;
                }
                return;
            }
            //建立到目标网站服务器的tcp连接，发送http请求
            s=Socket(host,port);        
            if(s<0) { failed++;continue;} //连接失败
            //发送请求报文
            if(rlen!=write(s,req,rlen)) {failed++;close(s);continue;}
            //http0.9的特殊处理
            //因为http0.9是在服务器回复后自动断开连接的，不keep-alive
            //在此可以提前先彻底关闭套接字的写的一半，如果失败了那么肯定是个不正常的状态,
            //如果关闭成功则继续往后，因为可能还有需要接收服务器的恢复内容
            //但是写这一半是一定可以关闭了，作为客户端进程上不需要再写了
            //因此我们主动破坏套接字的写端，但是这不是关闭套接字，关闭还是得close
            //事实上，关闭写端后，服务器没写完的数据也不会再写了，这个就不考虑了
            if(http10==0) 
            if(shutdown(s,1)) { failed++;close(s);continue;}
            //-f没有设置时默认等待服务器的回复
            if(force==0) 
            {
                /* read all available data from socket */
                while(1)
                {
                    if(timerexpired) break;
                    printf("开始读取\n");
                    i=read(s,buf,1500);//读服务器发回的数据到buf中
                    printf("读取结束\n");
                    if(i<0) 
                    { 
                        failed++;
                        close(s);
                        goto nexttry;//这次失败了那么继续下一次连接，与发出请求
                    }
                    else
                    if(i==0) break;
                    else
                    bytes+=i;//统计服务器回复的字节数
                }
            }
            if(close(s)) {failed++; continue;}
            speed++;
        }
    }
        
    
}