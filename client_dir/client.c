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
    char* arg;
    arg = strtok(NULL, " \t");
    while(arg != NULL) {
        char filename[BUFSIZE];
        char size[BUFSIZE];
        char buff[BUFSIZE];
        char prostr[1000];
        strcpy(filename, arg);
        printf("\nFilename: %s\n", filename);
        if(send(socketfd, filename, strlen(filename), 0) == -1) {
            perror("Sending filename");
            return;
        }
        if(recv(socketfd, size, BUFSIZE, 0) == -1) {
            perror("Reading from buffer");
            return;
        }
        if(atoi(size) == 0) {
            printf("File not found!\n");
            return;
        }
        printf("File size: %d\n", atoi(size));
        int fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ssize_t n;
        int size_i = (int) atoi(size);
        int written_lines=1;
        int rem = size_i % BUFSIZE;
        int num_loops = (size_i/BUFSIZE);
        float progress = 0.0;
        memset(buff, '\0', BUFSIZE);
        if(rem != 0) {
            if((n = recv(socketfd, buff, size_i % BUFSIZE, 0)) == -1) {
                perror("Reading from buffer");
                break;
            }
            if(n != BUFSIZE) {
                perror("Reading BUFSIZE");
            }
            if(write(fp, buff, size_i % BUFSIZE) != n) {
                perror("Writing content");
                return;
            }
            progress += (float)(size_i % BUFSIZE)/size_i;
            sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
            write(1, prostr, strlen(prostr));
            printf("\nWriting a line in rem - %d\n", written_lines);
            written_lines++;
        }
        memset(buff, '\0', BUFSIZE);
        printf("Num loops: %d\n", num_loops);
        int rec_size;
        int rec = 0;
        int rec_total = 0;
        memset(buff, 0, BUFSIZE);
        while(num_loops--) {
            rec=0;
            do {
                if((rec_size = recv(socketfd, buff, BUFSIZE, 0)) == -1) {
                    perror("Reading from buffer");
                    break;
                }
                //if(rec_size != BUFSIZE) {
                //    perror("Reading BUFSIZE");
                //}
                if(write(fp, buff, BUFSIZE) != BUFSIZE) {
                    perror("Writing content");
                    return;
                }
                rec += rec_size;
            } while (rec < BUFSIZE);
            rec_total += rec;
            progress += (float)(rec)/size_i;
            printf("\nWriting a line - %d [%d] [%d]\n", written_lines, rec, rec_total);
            sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
            write(1, prostr, strlen(prostr));
            written_lines++;
            memset(buff, '\0', BUFSIZE);
            //memset(buff, 0, BUFSIZE);
        }
        printf("\nFinished writing the contents\n");
        close(fp);
        arg = strtok(NULL, " \t");
    }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    //char *hello = "Hello from client";
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
