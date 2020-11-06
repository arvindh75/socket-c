#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT 8000
#define BUFSIZE 100000

void sendf(int socketfd) {
    ssize_t n;
    char filename[BUFSIZE];
    char buff [BUFSIZE];
    char line [BUFSIZE];
    int it=0;
    memset(&buff, '\0', sizeof(buff));
    memset(&filename, '\0', sizeof(filename));
    //Reading file name
    if(read(socketfd, filename, BUFSIZE) == -1) {
        perror("Reading from buffer");
    }
    printf("Finished getting the name - [%s]\n", filename);
    int fp = open(filename, O_RDONLY);
    FILE* fp2 = fopen(filename, "r");
    if(fp == -1) {
        perror("File");
        if(send(socketfd, "0", 1, 0) == -1) {
            perror("Sending size");
        }
        return;
    }
    //Getting file size
    int fsize = lseek(fp, 0, SEEK_END);
    int length = snprintf(NULL, 0,"%d",fsize);
    char fsize_st[length+1];
    sprintf(fsize_st, "%d", fsize);
    printf("Size: %d\n", fsize);
    if(send(socketfd, fsize_st, BUFSIZE, 0) == -1) {
        perror("Sending size");
        return;
    }
    lseek(fp, 0, SEEK_SET);
    //Sending file
    printf("Sending file\n");
    int num_lines=1;
    int read_size;
    int rem = fsize % BUFSIZE;
    int num_loops = (fsize/BUFSIZE);
    memset(&line, '\0', sizeof(line));
    if(rem != 0) {
        if((read_size = fread(line, sizeof(char), rem, fp2)) < 0) {
            printf("Reading file error\n");
            return;
        }
        if(send(socketfd, line, read_size, 0) == -1) {
            perror("Sending line");
            return;
        }
        printf("Sent a line from rem - %d\n", num_lines);
        num_lines++;
    }
    
    memset(&line, '\0', sizeof(line));
    printf("Num loops: %d\n", num_loops);
    int sent_size;
    while(num_loops--) {
        if((read_size = fread(line, sizeof(char), BUFSIZE, fp2)) < 0) {
            printf("Reading file error\n");
            return;
        }
        if((sent_size = send(socketfd, line, read_size, 0)) == -1) {
            perror("Sending line");
            return;
        }
        printf("Sent a line - %d [%d]\n", num_lines, sent_size);
        num_lines++;
        memset(&line, '\0', sizeof(line));
    }
    printf("Sent file \n");
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket file descriptor");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Binding");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    while(1)
        sendf(new_socket);
    return 0;
}
