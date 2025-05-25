#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <netinet/tcp.h>
#include <time.h>
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define ONEMB 1048576
#define TOTALSIZEOFPACKETS 2097151
#define X 100
#define PORT 8080
#define SA struct sockaddr


void func(int sockfd, int isWarmup)
{


    char recvBuff[1];

    // For each size of block
    for (int i = 1; i <= ONEMB; i*=2) {

        char *buff = malloc(i);

        struct timespec start, end;
        bzero(buff, i);

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int k = 0; k < X; k++) {
            write(sockfd, buff, i);
        }

        read(sockfd, recvBuff, 1);

        // Received the ack
        clock_gettime(CLOCK_MONOTONIC, &end);

        // The amount of seconds passed:
        double elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1e9);
        double avg = elapsed / X;

        double mbs = (i * 8) / (avg * 1048576);

        if (!isWarmup) {
            printf("%d\t%.5lf\tMb/s\n", i, mbs);
        }
        free(buff);

    }
}

int main(int argc, char *argv[])
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        bzero(&servaddr, sizeof(servaddr));

    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
        }


    // function for chat
    for (int j = 0; j < 15; j++) {
        func(sockfd, 1);
    }

    func(sockfd, 0);

    // close the socket
    close(sockfd);
}