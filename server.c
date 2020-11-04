#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 10000
#define LINSTENPORT 7572
#define SERVERPORT 8000

void sendf(int socketfd) {
    
    ssize_t n;
    sleep(5);
    char buff[BUFSIZE];
    char filename[100];
    int it=0;
    if((n = recv(socketfd, buff, BUFSIZE, 0)) != -1) {
        for(int i=0;i<BUFSIZE;i++) {
            if(buff[i] > 0) {
                filename[it++]=buff[i];
            }
        }
    }
    else {
        perror("Filename");
        return;
    }
    char line[BUFSIZE];
    printf("Finished getting the name\n");
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        perror("File");
    }
    while((n = fread(line, sizeof(char), BUFSIZE, fp)) > 0) {
        if(ferror(fp)) {
            perror("File read");
            return;
        }
        if(send(socketfd, line, n, 0) == -1) {
            perror("Sending line");
            return;
        }
        memset(line, 0, BUFSIZE);
    }
}

int main(int argc, char* argv[]) {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1) {
        perror("Socket file descriptor");
        return 0;
    }
    struct sockaddr_in  serverad;
    memset(&serverad, 0, sizeof(serverad));
    serverad.sin_family = AF_INET;
    serverad.sin_port = htons(SERVERPORT);
    socklen_t adlen = sizeof(serverad);
    if (inet_pton(AF_INET, "127.0.0.1", &serverad.sin_addr) < 0) {
        perror("IP address conversion");
        return 0;
    }
    int connectfd = connect(socketfd, (struct sockaddr *) &serverad, sizeof(serverad));
    if(connectfd == -1) {
        perror("Connection");
        close(socketfd);
        return 0;
    }
    sendf(socketfd);
    close(socketfd);
    return 0;
}
