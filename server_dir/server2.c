#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define PORT 8000

#define BUFSIZE 131072

void sendf(int socketfd) {
    char filename[BUFSIZE];
    char ack[BUFSIZE];
    char buff [BUFSIZE];
    char line [BUFSIZE];
    int it=0;
    memset(&buff, '\0', sizeof(buff));
    memset(&filename, '\0', sizeof(filename));
    memset(&ack, '\0', sizeof(ack));
    
    //Reading file name
    if(read(socketfd, filename, BUFSIZE) == -1) {
        perror("Reading file name");
        memset(&ack, '\0', sizeof(ack));
        strcpy(ack, "no");
        if(send(socketfd, ack, sizeof(ack), 0) == -1) {
            perror("Sending ack");
            return;
        }
    }
    
    //Sending ack
    memset(&ack, '\0', sizeof(ack));
    strcpy(ack, "ok");
    printf("Sending ok ack for filename\n");
    if(send(socketfd, ack, sizeof(ack), 0) == -1) {
        perror("Sending ack");
        return;
    }
    printf("Finished getting the name - [%s]\n", filename);
    
    //Opening files
    int fp = open(filename, O_RDONLY);
    FILE* fp2 = fopen(filename, "r");
    if(fp2 == NULL) {
        perror("File");
        memset(&ack, '\0', sizeof(ack));
        strcpy(ack, "0");
        printf("File not found\n");
        if(send(socketfd, ack, sizeof(ack), 0) == -1) {
            perror("Sending 0 size");
            return;
        }
        return;
    }
    
    //Getting file size
    int fsize = lseek(fp, 0, SEEK_END);
    close(fp);
    int length = snprintf(NULL, 0,"%d",fsize);
    char fsize_st[length+1];
    sprintf(fsize_st, "%d", fsize);
    memset(&buff, '\0', sizeof(buff));
    strcpy(buff, fsize_st);
    printf("Size: %s\n", buff);
    
    //Sending file size
    if(send(socketfd, buff, sizeof(buff), 0) == -1) {
        perror("Sending size");
        return;
    }
    memset(&ack, '\0', sizeof(ack));
    
    //Waiting for ack
    if(recv(socketfd, ack, BUFSIZE, 0) == -1) {
        perror("Receiving ack");
        return;
    }
    printf("Ack for filesize: [%s]\n", ack);
    if(strcmp(ack, "ok")) {
        printf("\nNot synchronised, Retry\n");
        return;
    }
    
    //Sending file
    printf("Sending file\n");
    //int num_lines=1;
    int read_size;
    memset(&line, '\0', sizeof(line));
    int sent_size;
    while ((read_size = fread(line, sizeof(char), BUFSIZE, fp2)) > 0) {
        if((sent_size = send(socketfd, line, read_size, 0)) == -1) {
            perror("Sending line");
            return;
        }
        //printf("Sent a line - %d [%d]\n", num_lines, sent_size);
        //num_lines++;
        memset(&line, '\0', sizeof(line));
    }
    fclose(fp2);
    printf("Sent file \n");
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
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
    while(1) {
        sendf(new_socket);
        sleep(5);
    }
    return 0;
}
