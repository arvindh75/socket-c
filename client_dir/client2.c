#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 8000
#define BUFSIZE 16384

void sigpip_hand(int signum) {
    fprintf(stdout, "\nServer was shutdown unexpectedly.\n");
    exit(EXIT_FAILURE);
}

void sigint_hand(int signum) {
    fprintf(stdout, "\n<Ctrl-C> attempt detected, Please quit using exit command\n");
    //exit(EXIT_FAILURE);
}

void getf(int socketfd) {
    char *arg;
    arg = strtok(NULL, " \t");
    while(arg != NULL) {
        char filename [BUFSIZE];
        char ack[BUFSIZE];
        char size [BUFSIZE];
        char buff [BUFSIZE];
        char prostr[1000];
        memset(&filename, '\0', sizeof(filename));
        strcpy(filename, arg);
        printf("\nFilename: %s\n", arg);
        //Sending filename
        if(send(socketfd, filename, BUFSIZE, 0) == -1) {
            perror("Sending filename");
            arg = strtok(NULL, " \t");
            continue;
        }
        //Reading file size
        memset(&size, '\0', sizeof(size));
        if(recv(socketfd, size, BUFSIZE, 0) == -1 || sizeof(size) == 0) {
            perror("Reading from buffer");
            return;
        }
        printf("File size: [%s]\n", size);
        if(!strcmp(size, "0")) {
            printf("File not found!\n");
            return;
        }
        //Opening file
        FILE* fp2 = fopen(arg, "wb");
        int size_i = (int) atoi(size);
        //int written_lines = 1;
        double progress = 0.0;
        int read_ret = 0;
        memset(&buff, '\0', sizeof(buff));
        //Reading data
        while((read_ret = recv(socketfd, buff, BUFSIZE,0)) > 0) {
            if(fwrite(buff, sizeof(char), read_ret, fp2) < read_ret) {
                perror("Writing content");
                return;
            }
            //printf("\nWriting a line - %d [%d]\n", written_lines, read_ret);
            progress += (double)(read_ret)/size_i;
            sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
            write(1, prostr, strlen(prostr));
            //written_lines++;
            memset(&buff, '\0', sizeof(buff));
            if(1.0 - progress <= (double)0.01/size_i) {
                break;
            }
        }
        printf("\nFinished writing the contents\n");
        fclose(fp2);
        arg = strtok(NULL, " \t");
    }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    printf("Waiting for a server\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connection established with the server\n");
    signal(SIGPIPE, sigpip_hand);
    signal(SIGINT, sigint_hand);
    char input[10000];
    int exit_read = 0; 
    int leninp;
    char* inp;
    char c;
    while(1) {
        leninp = 0;
        exit_read = 0;
        printf("\nclient> ");
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
                getf(sock);
            }
            else if (!strcmp(inp, "exit")) {
                char exit_f [BUFSIZE];
                memset(&exit_f, '\0', sizeof(exit_f));
                strcpy(exit_f, "exit");
                if(send(sock, exit_f, BUFSIZE, 0) == -1) {
                    perror("Sending exit");
                }
                break;
            }
            else {
                printf("Command not found !\n");
            }
        }
    }
    close(sock);
    return 0;
}
