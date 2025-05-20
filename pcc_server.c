#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>

u_int16_t* pcc_total;

void mySignalHandler(int signum) {
  int i = 0;
  int status;

  wait(&status);

  while (i < 95) {
    printf("char '%c' : %hu times\n", i + 32, pcc_total[i]);
    i += 1;
  }

  exit(0);
}

int main(int argc, char* argv[]) {
  int sd;
  struct sockaddr_in sin;
  struct sockaddr_in client;
  int new_sd;
  ssize_t notread;
  ssize_t bytes_read;
  ssize_t totalread;
  u_int16_t N;
  u_int16_t nboN;
  char* buf;
  u_int16_t C;
  u_int16_t nboC;
  ssize_t i;
  ssize_t nsent;
  ssize_t notwritten;
  ssize_t totalsent;
  pid_t pid;
  int status;
  socklen_t addrsize = sizeof(struct sockaddr_in);
  struct sigaction newAction = {.sa_handler = mySignalHandler, .sa_flags = SA_RESTART};

  pcc_total = mmap(NULL, 95 * sizeof(u_int16_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  if (pcc_total == MAP_FAILED) {
    perror(strerror(errno));
    exit(1);
  }
  memset(pcc_total, 0, 95 * sizeof(u_int16_t));

  // Overwrite default behavior for ctrl + c
  if (sigaction(SIGINT, &newAction, NULL) == -1) {
    perror(strerror(errno));
    exit(1);
  }
  
  if (argc != 2) {
    perror("the number of command line arguments passed is not correct");
    exit(1);
  }

  sd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void*) 1, sizeof(int));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(atoi(argv[1]));
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
    perror(strerror(errno));
    exit(1);
  }
  if (listen(sd, 10) == -1) {
    perror(strerror(errno));
    exit(1);
  }

  while(1) {
    C = 0;
    totalread = 0;
    notread = 2;
    i = 0;
    notwritten = 2;
    totalsent = 0;

    new_sd = accept(sd, (struct sockaddr*) &client, &addrsize);

    pid = fork();
    switch(pid) {
    case 0:
      // Child code
      if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
	perror(strerror(errno));
	exit(1);
      }
      
      while(notread > 0) {
	bytes_read = read(new_sd, &nboN + totalread, notread);
	if (bytes_read == 0 || (bytes_read == -1 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
	  perror(strerror(errno));
	  close(new_sd);
	  exit(1);
	}

	if (bytes_read >= 0) {
	  totalread += bytes_read;
	  notread -= bytes_read;
	}
      }

      N = ntohs(nboN);
      buf = malloc(N);
      totalread = 0;
      notread = N;
      while (notread > 0) {
	bytes_read = read(new_sd, buf + totalread, notread);
	if (bytes_read == 0 || (bytes_read == -1 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
	  perror(strerror(errno));
	  close(new_sd);
	  exit(1);
	}

	if (bytes_read >= 0) {
	  totalread += bytes_read;
	  notread -= bytes_read;
	}
      }

      while (i < N) {
	if (32 <= buf[i] && buf[i] <= 126) {
	  C += 1;
	}
	i += 1;
      }

      nboC = htons(C);
      while(notwritten > 0) {
	nsent = write(new_sd, &nboC + totalsent, notwritten);
	if (nsent == -1 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)) {
	  perror(strerror(errno));
	  close(new_sd);
	  exit(1);
	}

	if (nsent >= 0) {
	  totalsent += nsent;
	  notwritten -= nsent;
	}
      }
      
      i = 0;
      while (i < N) {
	if (32 <= buf[i] && buf[i] <= 126) {
	  pcc_total[buf[i]-32] += 1;
	}
	i += 1;
      }

      close(new_sd);
      exit(0);
    default:
      // Parent code
      wait(&status);
    }
  }
}
