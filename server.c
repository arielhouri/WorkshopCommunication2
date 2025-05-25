#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#define MAX 1048576
#define PORT 8081
#define SA struct sockaddr 
  
// Function designed for chat between client and server. 
void func(int connfd) 
{
    char buff[MAX];
    // infinite loop for chat 
    for (int i = 1; i <= MAX; i*=2) {
        int X = (10 * 1024 * 1024 / i);
        if (X < 1000) {
            X = 1000;
        }
        int n = 0;
        // read the message from client and copy it in buffer
        while (n < (i * X)) {
            n += read(connfd, buff, sizeof(buff));
        }
  
        // and send that buffer to client 
        write(connfd, "a", 1);
    }
}
  
// Driver function 
int main()
{
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    }
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    }
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    }
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    }

    int flag = 1;
    setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    // Function for chatting between client and server
    // for warmup, doing 15 iterations of the same process.
    // It is enough to set up the TCP connection.
    for (int i = 0; i < 20; i++) {
        func(connfd);
    }

    // for testing
    func(connfd);
  
    // After chatting close the socket 
    close(sockfd);
    fflush(stdout);
}