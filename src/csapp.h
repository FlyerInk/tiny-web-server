#include <stdio.h>          // standard IO functions
#include <stdlib.h>         // standard lib
#include <fcntl.h>          // file control


#include <errno.h>          // error number

#include <string.h>         // string operations, and memset, memcpy
#include <sys/types.h>      // types
#include <unistd.h>         // stdin, stdout, stderr, STD_FILEIN, STD_FILEOUT, STD_FILEERR
                            // stat

#include <netdb.h>          // dns, gethostbyname gethostbyaddr
#include <netinet/in.h>     // sockaddr_in
#include <sys/socket.h>     // socket
#include <arpa/inet.h>      // htonl, ntohl... byte order for network and host;

#include <sys/mman.h>       // mmap, munmap
#include <sys/stat.h>       // S_ISREG() S_ISDIR()

#include <signal.h>

#include <pthread.h>


#define MEMSET(x) (memset(x, 0, sizeof(x)))

#define MAXLINE (8125)
#define MAXCONNECT (1024)


typedef struct sockaddr SA;

typedef struct{
    int rio_fd;                 /* file descriptor  */
    int rio_cnt;                /* size of the unread bytes */
    char *rio_bufptr;           /* beginning of the unread bytes */
    char rio_buf[MAXLINE];      /* buffer */
} rio_t;

int rio_readn(int fd, void *usrbuf, size_t n);
int rio_writen(int fd, void *usrbuf, size_t n);


void rio_readinitb(rio_t *rp, int fd);
int rio_readnb(rio_t *rp, void *usrbuf, size_t);
int rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);


int open_clientfd(const char *host, int port);    /* open and connect to host on port */
int open_listenfd(int port);                /* listen on every interface */


int parse_uri(char *uri, char *filename, char *cgiargs);
void client_error(int fd, char *cause, char *code, char *shortmsg, char *longmsg);

void serve_static(int fd, char *filename, int filesize);
void serve_dynamic(int fd, char *filename, char *cgiargs);

void parse_req_headerline(rio_t *rp);
void get_filetype(char *filename, char *filetype);
