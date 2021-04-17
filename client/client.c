#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>


#define MAX_SIZE 100
#define READ_SIZE 100
#define ok_size 3

void reverse(char *in)
{
    int len = strlen(in);
    for(int i=0,j=len-1;i<len/2;i++,j--)
    {
        char temp = in[i];
        in[i]=in[j];
        in[j]=temp;
        
    }
}

int main(int argc, char *argv[])
{
    int connectfd;
    char check_connec[5];
    long arg;
    if(argc < 3)
    {
        printf("please enter the parameters ip address, port number.\n");
        exit(1);
    }

    char *address = argv[1];
    printf("%s\n",address);
    int port_no = atoi(argv[2]);
    char message [MAX_SIZE];
    char content [READ_SIZE];

    struct sockaddr_in servaddr;
    int sockfd;
    // int connectfd;
    FILE* fp;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        printf("error in creating a socket.\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_no);
    servaddr.sin_addr.s_addr = inet_addr(address);

    if((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))<0)
    {
        printf("failed to connect.\n");
        exit(3);
    }

    sleep(2);

    while (1)
    {
        memset(check_connec,0,sizeof(check_connec));

        if (read(sockfd,check_connec,sizeof(check_connec))<0)
        {
            exit(10);
        }
        if(strcmp("exit",check_connec)==0)      //server refused a connection because queue size was >4
        {
            printf("connection refused\n");
            break;   
        }
        // printf("b\n");


        memset(message, 0, MAX_SIZE);
        memset(content, 0, READ_SIZE);
        printf("enter the message to be sent to server and press enter: ");
        fgets(message, MAX_SIZE, stdin);

        if(strcmp("exit\n",message)==0)
        {
            if(write(sockfd, "exit", strlen("exit"))<0)
            {
                printf("error in sending file\n");
                exit(4);
            }
            break;
        }

        for(int i=0;i<MAX_SIZE;i++)
        {
            if(message[i]=='\n')
            {
                message[i] = '\0';
                break;
            }
        }

        if(write(sockfd, message, strlen(message))<0)
        {
            printf("error in sending message.\n");
            exit(4);
        }

        if(read(sockfd, content, READ_SIZE)<1)
        {
            printf("error in reading.\n");
            exit(5);
        }
        if(strcmp("",content)!=0)
        {
            // printf("%s\n",content);
            
            printf("Message has been read from the server!\n");
            reverse(content);
            printf("server : %s\n",content);
        }
        else
        {
            printf("The server sent an empty string!\n");
        }

    }
    

    return 0;
}