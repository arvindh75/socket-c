#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 10000
#define LINSTENPORT 7572
#define SERVERPORT 7572

void get(int socketfd) {
    char* arg = strtok(NULL, " \t");
    FILE *fp = fopen(arg, "w+");
    if(send(socketfd, arg, sizeof(arg), 0) == -1) {
        perror("Sending filename");
        return;
    }
    ssize_t n;
    sleep(5);
    char buff[BUFSIZE];
    while((n = recv(socketfd, buff, BUFSIZE, 0)) != -1) {
        if(fwrite(buff, sizeof(char), n, fp) != n) {
            perror("Writing content");
            return;
        }
        memset(buff, 0, BUFSIZE);
    }
    printf("Finished writing the contents\n");
}

int main(int argc, char* argv[]) {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1) {
        perror("Socket file descriptor");
        return 0;
    }
    struct sockaddr_in clientad, serverad;
    memset(&serverad, 0, sizeof(serverad));
    serverad.sin_family = AF_INET;
    serverad.sin_addr.s_addr = htonl(INADDR_ANY);
    serverad.sin_port = htons(SERVERPORT);
    printf("\nBinding\n");
    if(bind(socketfd, (struct sockaddr *) &serverad, sizeof(serverad)) == -1) {
        perror("Binding");
        close(socketfd);
        return 0;
    }
    printf("\nBinding successful\n");
    printf("\nListening\n");
    if(listen(socketfd, 3) == -1) {
        perror("Listening");
        close(socketfd);
        return 0;
    }
    printf("\nListening successful\n");
    socklen_t adlen = sizeof(serverad);
    int connectfd = accept(socketfd, (struct sockaddr *) &serverad, &adlen);
    printf("\nConnecting\n");
    if(connectfd == -1) {
        perror("Connection");
        close(socketfd);
        return 0;
    }
    printf("\nConnection successful\n");
    char input[10000];
    int exit_read = 0; 
    int leninp;
    char* inp;
    char c;
    while(1) {
        leninp = 0;
        exit_read = 0;
        printf("client> ");
        while (exit_read == 0) {
            c = getchar();
            if (c == EOF) {
                return 0;
            }
            if (c > 127 || c < 0)
                continue;
            if (c != '\n') {
                input[leninp] = c;
                leninp++;
            }
            else {
                input[leninp] = '\0';
                leninp++;
                exit_read = 1;
            }
        }
        inp = strtok(input, " \t");
        if(inp) {
            if(!strcmp(inp, "get")) {
                get(socketfd);
            }
            else if (!strcmp(inp, "exit")) {
                break;
            }
            else {
                printf("Command not found !\n");
            }
        }
    }
    close(socketfd);
    close(connectfd);
    return 0;
}
