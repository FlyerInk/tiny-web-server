#include "csapp.h"

static int rio_read(rio_t *rp, char *usrbuf, size_t n) {
    if (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno == EINTR) {
                rp->rio_cnt = 0;
            } else {
                return -1;
            }
        } else if (rp->rio_cnt == 0) {
            return 0;
        } else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }
    size_t nread = n;
    if (rp->rio_cnt < nread) {
        nread = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, nread);
    rp->rio_cnt -= nread;
    rp->rio_bufptr += nread;
    return nread;
}

int rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    char *ptr = (char *) usrbuf;

    ssize_t nread;
    while (nleft > 0) {
        nread = read(fd, ptr, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                // signal handler, continue to read
                nread = 0;
            } else {
                // error
                return -1;
            }
        } else if (nread == 0) {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}

int rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    char *ptr = (char *) usrbuf;
    ssize_t nwritten;
    while(nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten < 0) {
            if (errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

int rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    char *ptr = (char *)usrbuf;
    ssize_t nread;
    while(nleft > 0) {
        nread = rio_read(rp, ptr, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else {
                return -1;
            }
        } else if (nread == 0) {
            break;
        }
        ptr += nread;
        nleft -= nread;
    }
    return (n - nleft);
}

int rio_readlineb(rio_t *rp, void *usrbuf, size_t n) {
    ssize_t nread;
    int i;

    char *ptr = (char *)usrbuf;
    char c;

    for(i = 1; i < n; i++) {
        nread = rio_read(rp, &c, 1);
        if (nread == 0) {
            if (i == 1) {
                return 0; // no data read
            } else {
                break;
            }
        } else if (nread == 1) {
            *ptr = c;
            ptr++;
            if (c == '\n') {
                break;
            }
        } else {
            return -1;
        }
    }
    *ptr = 0;
    return i;
}


int open_clientfd(const char *host, int port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0) {
        printf("socket error\n");
        return -1;
    }

    struct hostent *hp = gethostbyname(host);
    if (hp == NULL) {
        printf("gethostbyname error\n");
        return -1;
    }

    struct sockaddr_in addr;
    MEMSET(&addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    memcpy((void *) &addr.sin_addr.s_addr, (const void *)hp->h_addr_list[0], hp->h_length);

    int connect_rc = connect(clientfd, (SA *) &addr, sizeof(addr));
    if (connect_rc < 0) {
        printf("connect error, %s\n", strerror(errno));
        return -1;
    }
    return clientfd;
}

int open_listenfd(int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        return -1;
    }

    struct sockaddr_in addr;
    MEMSET(&addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int bind_rc = bind(listenfd, (SA *) &addr, sizeof(addr));
    if (bind_rc < 0) {
        return -1;
    }

    int listen_rc = listen(listenfd, MAXCONNECT);
    if (listen_rc < 0) {
        return -1;
    }
    printf("listenfd: %d, port: %d\n", listenfd, port);
    return listenfd;
}



/* web server */

/*
    @return true if the uri points to a static file, 0 if dynamic content
*/
int parse_uri(char *uri, char *filename, char *cgiargs) {
    if (!strstr(uri, "cgi-bin")) {
        // static files
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "res/index.html");
        }
        return 1;
    }

    char *sep = index(uri, '?');
    if (sep) {
        strcpy(cgiargs, sep + 1);
        *sep = 0; // the uri string is changed from here!!!
    } else {
        strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcpy(filename, uri);
    return 0;
}



void parse_req_headerline(rio_t *rp) {
    char buf[MAXLINE];
    MEMSET(buf);

    rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        //printf("%s", buf);
    }
}


void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXLINE];
    MEMSET(body);
    MEMSET(buf);

    /* build the http body */
    sprintf(body, "<html><title>Error</title>");
    sprintf(body, "%s<body>\r\n", body);
    sprintf(body, "%s<p>%s: %s</p>", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s</p>", body, longmsg, cause);
    sprintf(body, "%s<br><hr><em>the web server</em></body></html>", body);

    /* headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-Type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-Length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

void serve_static(int fd, char *filename, int filesize) {
    char filetype[MAXLINE];
    char buf[MAXLINE];
    MEMSET(buf);
    MEMSET(filetype);

    get_filetype(filename, filetype);
    /* headers */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
    sprintf(buf, "%sContent-Length: %d\r\n\r\n", buf, filesize);
    rio_writen(fd, buf, strlen(buf));

    /* reponse body */
    int srcfd = open(filename, O_RDONLY, 0);

    char *srcp = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    } else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    } else if(strstr(filename, ".jpg") || strstr(filename, ".jpeg")) {
        strcpy(filetype, "image/jpeg");
    } else if(strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    } else {
        strcpy(filetype, "text/plain");
    }
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
    // not implemented yet
    return;
}


















