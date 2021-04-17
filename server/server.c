#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <semaphore.h>

#define F_MAX 100
#define READ_SIZE 100
#define SA struct sockaddr 
#define ok_size 3

struct thread_data
{
    int connectfd;
    char ip_address[20];
    int port_number;
};


void error_helper(int code)         //function to handle errors
{
    switch (code)
    {
    case 1:
        printf("No port number specified\n");
        exit(1);
        break;
    case 2:
        printf("Error creating a socket\n");
        exit(2);
        break;
    case 3:
        printf("Could not bind.\n");
        exit(3);
        break;
    case 4:
        printf("Could not listen.\n");
        exit(4);
        break;
    case 5:
        printf("Could not read from client.\n");
        exit(5);
        break;
    case 6:
        printf("Could not send to client.\n");
        exit(6);
        break;
    case 7:
        printf("Could not complete handshake with client.\n");
        exit(7);
        break;
    
    default:
        break;
    }
    exit(7);
}

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

sem_t semaphore;        //semaphore used to ensure only 4 connections are allowed at a time
char check_connec[] = {'o','k','a','y','\0'};   //data which is passed to check if connection is alive

void* helper(void *arg)     //function executed by individual threads
{
    struct thread_data *data = (struct thread_data*)arg;
    char ip_address[40];
    strcpy(ip_address,data->ip_address);
    int port_number = data->port_number;
    int connectfd = data->connectfd;     //connection socket fd 
    free(arg);
    if(sem_trywait(&semaphore)!=0)  //use try wait to decrement semaphore if possible else exit
    {
        char ex[]="exit";
        if(write(connectfd,ex,sizeof(ex))<0)
        {
            exit(10);
        }
        shutdown(connectfd, SHUT_RDWR);
        close(connectfd);
        printf("Unfortunately, 4 other clients are already connected to the server!\n");
        return NULL;
    }
    char contents[F_MAX];       //normal logic for sending data in reverse etc
    char FILE_CONTENTS[READ_SIZE];
    while(1)
    {
        if((write(connectfd, check_connec, sizeof(check_connec)))<0)
        {
            error_helper(6);
        }

        memset(contents, 0, F_MAX);
        memset(FILE_CONTENTS, 0, READ_SIZE);

        if((read(connectfd, contents, sizeof(contents)))<0)
        {
            error_helper(5);
        }
        if(strcmp("exit",contents)==0)
        {
            break;
        }
        reverse(contents);
        printf("%s:%d says : %s\n",ip_address,port_number,contents);
        fflush(stdout);
        printf("Enter the contents to be sent to the %s:%d: ",ip_address,port_number);
        fgets(FILE_CONTENTS, READ_SIZE, stdin);
        for(int i=0;i<READ_SIZE;i++)
        {
            if(FILE_CONTENTS[i]=='\n')
            {
                FILE_CONTENTS[i] = '\0';
                break;
            }
        }
        if((write(connectfd, FILE_CONTENTS, sizeof(FILE_CONTENTS)))<0)
        {
            error_helper(6);
        }

    }
    printf("%s:%d has exited.\n",ip_address,port_number);
    fflush(stdout);
    close(connectfd);
    sem_post(&semaphore);
    return NULL;
}

int main(int argc, char *argv[])
{
    
    if(argc<2)              // no port number specified 
    {
        error_helper(1);
    }

    sem_init(&semaphore, 0, 4);     //initialise the semaphore 

    int port_no = atoi(argv[1]);    

    printf("%d\n", port_no);
    struct sockaddr_in servaddr;
    int sockfd;
    int connectfd;
    FILE *fp;



    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)      //creating a tcp socket 
    {
        error_helper(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);       //respond to any client
    servaddr.sin_port = htons(port_no);

    if(bind(sockfd, (SA*)&servaddr, sizeof(servaddr))!=0)        //binding and listening to incoming connections 
    {
        error_helper(3);
    }

    if((listen(sockfd, 1))<0)
    {
        error_helper(4);
    }

    while(1)
    {
        printf("waiting for a client!\n");
        fflush(stdout);
        struct sockaddr_in client_addr;
        socklen_t slen = sizeof(client_addr);


        if((connectfd = accept(sockfd, (struct sockaddr *)&client_addr, &slen))<0)           //fd which allows us to talk to the client
        {
            printf("could not connect to the client!\n");
            fflush(stdout);
            continue;

            // error_helper(7);
        }

        printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("port is: %d\n", (int) ntohs(client_addr.sin_port));

        printf("trying to connect to a client!\n");
        fflush(stdout);

        struct thread_data *data = malloc(sizeof(struct thread_data));

        data->port_number = (int) ntohs(client_addr.sin_port);
        strcpy(data->ip_address,inet_ntoa(client_addr.sin_addr));
        data->connectfd = connectfd;


        pthread_t t;
        int *args = malloc(sizeof(int));
        *args = connectfd;
        pthread_create(&t, NULL, helper, (void *)data);
    }
    
    sem_destroy(&semaphore);

    return 0;
}
