# TCP multithreaded messenger

TCP messenger written in C using threads and semaphores where atmost 4 clients can communicate with server

## Compilation

The files can be compiled traditionally using gcc
```bash
gcc -o server.out server.c -lpthread
gcc -o client.out client.c
```

## Usage

```bash
/server.out 8082
#here 8082 is the port at which server is running
/client.out 127.0.0.1 8082
#client tries to connect at localhost port 8082
```