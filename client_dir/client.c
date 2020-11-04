#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#define PORT 8000
#define BUFSIZE 100000

void get(int socketfd) {
    char* arg = strtok(NULL, " \t");
    char filename[BUFSIZE];
    strcpy(filename, arg);
    printf("Filename: %s\n", filename);
    if(send(socketfd, filename, strlen(filename), 0) == -1) {
        perror("Sending filename");
        return;
    }
    char size[BUFSIZE];
    recv(socketfd, size, BUFSIZE, 0);
    printf("File size: %d\n", atoi(size));
    int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t n;
    char buff[BUFSIZE];
    int written_lines=0;
    int num_loops = (atoi(size)/BUFSIZE) + 1;
    //printf("Num loops: %d\n", num_loops);
    float progress = 0.0;
    char prostr[1000];
    while(num_loops > 0) {
        if(num_loops == 1) {
            if((n = recv(socketfd, buff, atoi(size) % BUFSIZE, 0)) == -1) {
                perror("Reading from buffer");
                break;
            }
            progress += (float)(n/atoi(size));
            if(write(fp, buff, atoi(size) % BUFSIZE) != n) {
                perror("Writing content");
                return;
            }
        }
        else {
            if((n = recv(socketfd, buff, BUFSIZE, 0)) == -1) {
                perror("Reading from buffer");
                break;
            }
            progress += (float)(n/atoi(size));
            if(write(fp, buff, BUFSIZE) != n) {
                perror("Writing content");
                return;
            }
        } 
        sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
        write(1, prostr, strlen(prostr));
        printf("\nWriting a line - %d\n", written_lines);
        written_lines++;
        //if(written_lines > 0)
        //    break;
        num_loops--;
        memset(buff, 0, BUFSIZE);
    }
    close(fp);
    printf("Finished writing the contents\n");
}


int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    //char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
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
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
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
                get(sock);
            }
            else if (!strcmp(inp, "exit")) {
                break;
            }
            else {
                printf("Command not found !\n");
            }
        }
    }
    close(sock);
    //send(sock , hello , strlen(hello) , 0 );
    //printf("Hello message sent\n");
    //valread = read( sock , buffer, 1024);
    //printf("%s\n",buffer);
    return 0;
}
