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
    //sleep(5);
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
    printf("Finished getting the name - [%s]\n", filename);
    int fp = open(filename, O_RDONLY);
    char no_file[BUFSIZE];
    if(fp == -1) {
        perror("File");
        memset(no_file, 0, BUFSIZE);
        if(send(socketfd, "0\0", 2, 0) == -1) {
            perror("Sending size");
        }
        return;
    }
    int fsize = lseek(fp, 0, SEEK_END);
    int length = snprintf(NULL, 0,"%d",fsize);
    char fsize_st[length+1];
    sprintf(fsize_st, "%d", fsize);
    printf("Size: %d\n", fsize);
    if(send(socketfd, fsize_st, strlen(fsize_st), 0) == -1) {
        perror("Sending size");
        return;
    }
    lseek(fp, 0, SEEK_SET);
    char line[BUFSIZE] = {0};
    printf("Sending file\n");
    int num_lines=1;
    int rem = fsize % BUFSIZE;
    int num_loops = (fsize/BUFSIZE) + 1;
    if(rem != 0) {
        if((n = read(fp, line, fsize%BUFSIZE)) < 0) {
            printf("Reading file error\n");
            return;
        }
        if(n != fsize%BUFSIZE) {
            perror("File read");
            return;
        }
        if(send(socketfd, line, BUFSIZE, 0) == -1) {
            perror("Sending line");
            return;
        }
        printf("Sent a line from rem - %d\n", num_lines);
        num_lines++;
    }
    printf("Num loops: %d\n", num_loops);
    int sent_size;
    while(num_loops--) {
        if((n = read(fp, line, BUFSIZE)) < 0) {
            printf("Reading file error\n");
            return;
        }
        if(n != BUFSIZE) {
            perror("File read");
            return;
        }
        if((sent_size = send(socketfd, line, BUFSIZE, 0)) == -1) {
            perror("Sending line");
            return;
        }
        printf("Sent a line - %d [%d]\n", num_lines, sent_size);
        num_lines++;
        memset(line, 0, BUFSIZE);
    }
    printf("Sent file \n");
    usleep(0.5 * 1000);
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
    //valread = read(new_socket , buffer, 1024);  // read infromation received into the buffer
    //printf("%s\n",buffer);
    //send(new_socket , hello , strlen(hello) , 0 );  // use sendto() and recvfrom() for DGRAM
    //printf("Hello message sent\n");
    return 0;
}
