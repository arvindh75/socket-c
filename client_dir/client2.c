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

void getf(int socketfd) {
    char *arg;
    char filename [BUFSIZE];
    char size [BUFSIZE];
    char buff [BUFSIZE];
    arg = strtok(NULL, " \t");
    while(arg != NULL) {
        char prostr[1000];
        memset(&filename, '\0', sizeof(filename));
        strcpy(filename, arg);
        printf("\nFilename: %s\n", arg);
        if(send(socketfd, filename, BUFSIZE, 0) == -1) {
            perror("Sending filename");
            return;
        }

        memset(&size, '\0', sizeof(size));
        if(recv(socketfd, size, BUFSIZE, 0) == -1) {
            perror("Reading from buffer");
            return;
        }
        if(!strcmp(size, "0")) {
            printf("File not found!\n");
            return;
        }
        printf("File size: %d\n", atoi(size));
        
        FILE* fp2 = fopen(arg, "wb");
        ssize_t n;
        int size_i = (int) atoi(size);
        memset(&size, '\0', sizeof(size));
        int written_lines = 1;
        int rem = size_i % BUFSIZE;
        int num_loops = (size_i/BUFSIZE);
        float progress = 0.0;
        int read_ret = 0;
        memset(&buff, '\0', sizeof(buff));
        
        if(rem != 0) {
            if((read_ret = read(socketfd, buff, BUFSIZE)) == -1) {
                perror("Reading from buffer");
                break;
            }
            if(fwrite(buff, sizeof(char), read_ret, fp2) < read_ret) {
                perror("Writing content");
                return;
            }
            progress += (float)(read_ret)/size_i;
            sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
            write(1, prostr, strlen(prostr));
            printf("\nWriting a line in rem - %d\n", written_lines);
            written_lines++;
            memset(&buff, '\0', sizeof(buff));
        }
        
        printf("Num loops: %d\n", num_loops);
        int rec_size;
        int rec = 0;
        int rec_total = 0;
        while(num_loops--) {
            if((read_ret = read(socketfd, buff, BUFSIZE)) == -1) {
                perror("Reading from buffer");
                break;
            }
            if(fwrite(buff, sizeof(char), read_ret, fp2) < read_ret) {
                perror("Writing content");
                return;
            }
            printf("\nWriting a line - %d [%d]\n", written_lines, read_ret);
            progress += (float)(read_ret)/size_i;
            sprintf(prostr, "\rProgress : %.2f %c", progress * 100, '%');
            write(1, prostr, strlen(prostr));
            written_lines++;
            memset(&buff, '\0', sizeof(buff));
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
                getf(sock);
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
    return 0;
}
