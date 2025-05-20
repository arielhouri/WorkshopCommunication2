#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[]) {
  int fd;
  int sd;
  u_int16_t N;
  u_int16_t nboN;
  u_int16_t nboC;
  char buf[65536];
  ssize_t totalsent = 0;
  ssize_t notwritten = 2;
  ssize_t nsent;
  ssize_t totalread = 0;
  ssize_t notread = 2;
  ssize_t bytes_read;
  struct sockaddr_in dst;
  dst.sin_family = AF_INET;
  dst.sin_port = htons(atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &(dst.sin_addr));
  
  if (argc != 4) {
    perror("the number of command line arguments passed is not correct");
    exit(1);
  }

  fd = open(argv[3], O_RDONLY);
  if (fd == -1) {
    perror(strerror(errno));
    exit(1);
  }

  sd = socket(AF_INET, SOCK_STREAM, 0);


  if (connect(sd, (struct sockaddr*) &dst, sizeof(dst)) == -1) {
    perror(strerror(errno));
  }

  N = (uint16_t) read(fd, buf, 65535);
  nboN = htons(N);
  close(fd);

  // keep looping until nothing left to write
  while (notwritten > 0) {
    // notwritten = how much we have left to write
    // totalsent = how much we've written so far
    // nsent = how much we've written in last write() call
    nsent = write(sd, &nboN + totalsent, notwritten);
    // check if error occured
    if (nsent < 0) {
      perror(strerror(errno));
      close(sd);
      exit(1);
    }

    totalsent += nsent;
    notwritten -= nsent;
  }

  totalsent = 0;
  while (N > 0) {
    nsent = write(sd, buf + totalsent, N);
    if (nsent < 0) {
      perror(strerror(errno));
      close(sd);
      exit(1);
    }

    totalsent += nsent;
    N -= nsent;
  }

  // read data from server into C_buff
  // block until there's something to read
  while (notread > 0) {
    bytes_read = read(sd, &nboC + totalread, notread);
    if (bytes_read < 0) {
      perror(strerror(errno));
      close(sd);
      exit(1);
    }

    totalread += bytes_read;
    notread -= bytes_read;
  }
    
  close(sd);

  printf("# of printable characters: %hu\n", ntohs(nboC));

  exit(0);
}
