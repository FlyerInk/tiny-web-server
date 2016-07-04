#include "csapp.h"

void do_it(int connectfd);
void *thread_routine(void *arg);

static const int DEFAULT_PORT = 8007;

void sigchild_handler(int signal) {
    while(waitpid(-1, 0, WNOHANG) > 0) {
        ;
    }
    return;
}

int main(int argc, char const *argv[]){
    int port = 0;
    switch(argc) {
        case 1: {
            //fprintf(stderr, "usage: %s <port>\n", argv[0]);
            port = DEFAULT_PORT;
            fprintf(stdout, "usage: %s <port> (default port is %d)\n", argv[0], DEFAULT_PORT);
            break;
        }
        case 2: {
            port = atoi(argv[1]);
            break;
        }
        default:
            break;
    }

    if (port == 0) {
        fprintf(stderr, "error port: %d\n", port);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return -1;
    }

    signal(SIGCHLD, sigchild_handler);

    int listenfd = open_listenfd(port);
    if (listenfd < 0) {
        fprintf(stderr, "open_listenfd error\n");
        return -1;
    }

    struct sockaddr_in addr;
    MEMSET(&addr);
    size_t addrlen = sizeof(addr);
    int connectfd;
    struct hostent *hp;
    char *hp_name;

    pthread_t tid;
    int *fdp = NULL;
    while (1) {
        printf("------ ready to accept ------\n");
        fdp = (int *)malloc(sizeof(int));
        *fdp = accept(listenfd, (SA *) &addr, (socklen_t *)&addrlen);
        if ((*fdp) > 0) {
            pthread_create(&tid, NULL, thread_routine, (void *)fdp);
        } else {
            printf("error, accept *fdp  < 0");
            break;
        }
        hp = gethostbyaddr((const char *) &addr.sin_addr.s_addr, sizeof(addr.sin_addr.s_addr), AF_INET);
        hp_name = inet_ntoa(addr.sin_addr);
        printf("server connected to %s (%s)\n", hp->h_name, hp_name);
    }
    printf("------ exit ------\n");
    return 0;
}

void *thread_routine(void *arg) {
    int connectfd = *((int *)arg);
    pthread_detach(pthread_self());
    free(arg);
    do_it(connectfd);
    close(connectfd);
    return NULL;
}


void do_it(int fd) {
    char buf[MAXLINE];
    MEMSET(buf);

    rio_t rio;
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        // method does not equal with 'GET', only support 'GET' now
        client_error(fd, method, "501", "Not implemented", "web server do not implement this method");
        return;
    }

    parse_req_headerline(&rio);

    char filename[MAXLINE], cgiargs[MAXLINE];
    int is_static = parse_uri(uri, filename, cgiargs);

    struct stat filestat;
    printf("[Request] %s %s\n", method, uri);
    if (stat(filename, &filestat) < 0) {
        client_error(fd, filename, "404", "Not Found", "web server could not find this file");
        return;
    }

    if (is_static) {
        // not a regular file or do not have permission to read this file
        if (!(S_ISREG(filestat.st_mode))
            || !(S_IRUSR & filestat.st_mode) ) {
            client_error(fd, filename, "403", "Forbidden", "web server could not read this file");
            return;
        }
        serve_static(fd, filename, filestat.st_size);

    } else {
        // not a regular file or do not have permission to execute this file
        if (
            !(S_ISREG(filestat.st_mode))
            || !(S_IXUSR & filestat.st_mode) ) {

            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}
