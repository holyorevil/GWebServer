#include <stdio.h>  
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <fcntl.h>  
#include <time.h>  
#include <stddef.h>   
#include <sys/stat.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/select.h>
#include <arpa/inet.h>  
#include <sys/time.h>
 
#define DSTADDR     "127.0.0.1"
#define DSTPORT     80
 
int main(int argc, char* argv[])
{  
    int ret = 0;
    int sockfd = 0;  
    char buffer[1024] = {0};  
    struct sockaddr_in servAddr;  
    socklen_t addr_size = 0;  
    char destAddr[16] = {0};
    int destPort = 0;
    int recv_len = 0;
    char who[64] = {0};
    char sndbuffer[256] = {0};
    struct timeval tvBegin;
    struct timeval tvEnd;
 
    if(argc < 3)
    {
	strcpy(destAddr, DSTADDR);
      	destPort = DSTPORT;
    }
    else
    {
      	strcpy(destAddr, argv[1]);
      	destPort = atoi(argv[2]);
    }
  
    /*---- Create socket ----*/  
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
    	perror("socket");
    	exit(-1);
    }
 
    /*---- Set non-block ----*/
    int options = fcntl(sockfd, F_GETFL, 0); 
    fcntl(sockfd, F_SETFL, options | O_NONBLOCK); 
    
    /*---- Configure settings of connection ----*/  
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;  
    servAddr.sin_port = htons(destPort);  
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr.s_addr);//将ip转换成int型存入sockaddr_in结构体中  
    /* Set padding field to 0 */  
    //memset(servAddr.sin_zero, 0, sizeof(servAddr.sin_zero));    
  
    /*---- Connect ----*/  
    ret = connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));  
    if(ret == 0)
    {
      	printf("connected.\n");
    }
    else if(ret < 0)
    {
      	//perror("connect"); 
      	if(errno == EINPROGRESS)
      	{
            printf("connecting...%s:%d\n", destAddr,destPort);
 
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);                 
 
            struct timeval timeout;         
            timeout.tv_sec = 6; 
            timeout.tv_usec = 0;     
 
            ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout );
            if(ret < 0)
            {
                perror("select");
                close(sockfd);
                exit(-1);
            }
            if (ret == 0)                         
            {               
                printf( "connection timeout\n" );                
                close(sockfd);
                exit(-1);
            }
            else
            {
                if(!FD_ISSET(sockfd, &writefds))
                {
                     printf("err, no events found!\n");        
                     close(sockfd);
                     exit(-1);
                }
                else
                {   
                    int err = 0;
                    socklen_t elen = sizeof(err);
                    ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&err, &elen);
                    if(ret < 0)
                    {
                        perror("getsockopt");
                        close(sockfd);
                        exit(-1);
                    }
                    if(err != 0)
                    {
                        printf("connect failed with the error: (%d)%s\n", err, strerror(err));
                        close(sockfd);
                        exit(-1);
                    }
                    else
                    {
                        printf("connected.\n");
                    }
                }
            }
        }
    }
  
    /*---- Send ----*/
    strcpy(sndbuffer,"Hello!\r\n");  
    if((ret = send(sockfd, sndbuffer, strlen(sndbuffer), 0)) > 0)
    {
      	printf(" >>: %s", sndbuffer);
    }
    else
    {
      	perror("send");
      	close(sockfd);
      	exit(-1);
    }
 
    gettimeofday(&tvBegin, NULL);
 
    while(1)
    {
      	/*---- Recv ----*/  
      	recv_len = recv(sockfd, buffer, 1024, 0);  
        if(recv_len < 0)
        {
            if((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
            {
                //printf("again..\n");
            	usleep(10);
            	//continue;
            }
            else
            {
            	printf("recv err1!\n");
            	break;
            }
        }
        else if(recv_len == 0)
        {
            printf("recv err2!\n");
            break;
        }
        else
        {
            // print recv msg
            buffer[recv_len] = '\0';
            printf(" <<: %s\n",buffer);
            char *p = NULL;
            if(p = strstr(buffer, "Login OK."))
            {
             	//strncpy(who, p+9);
             	printf("== login success[%s] ==\n\n", who);
            }
            else if(strstr(buffer, "heartbeat"))
            {
             	//printf("heartbeat.\n");
 
                // do sth response heartbeat and other.
             	memset(sndbuffer, 0, sizeof(sndbuffer));
             	sprintf(sndbuffer, "%s response heartbeat\r\n", who);
             	if((ret = send(sockfd,sndbuffer,strlen(sndbuffer),0)) > 0)
             	{
                    printf(" >>: %s", sndbuffer);
                }
 
                usleep(100);
                memset(sndbuffer, 0, sizeof(sndbuffer));
             	sprintf(sndbuffer, "%s request other...\r\n", who);
             	if((ret = send(sockfd,sndbuffer,strlen(sndbuffer),0)) > 0)
             	{
                    printf(" >>: %s", sndbuffer);
             	}
            }
        }
       
        //
        gettimeofday(&tvEnd, NULL); 
        unsigned int subTime = tvEnd.tv_sec-tvBegin.tv_sec;
        //printf(".............%d....\n", subTime);
        if(subTime > 3)
        { 
            gettimeofday(&tvBegin, NULL);
            memset(sndbuffer, 0, sizeof(sndbuffer));
            sprintf(sndbuffer, "I am %s,still alive.\r\n", who);
            if((ret = send(sockfd,sndbuffer,strlen(sndbuffer),0)) > 0)
            {
              	printf("->>: %s", sndbuffer);
            } 
        }
 
 
        usleep(1000);
    }
  
    return 0;  
} 

