// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Server gui file

#include <asm-generic/socket.h>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const int PORT = 9000;
const int BUFFER_SIZE = 1024;

void * threadFunc(void * args) {
    int sock = *((int *) args);
    char * buff = new char[BUFFER_SIZE];
    int buffLeng;
    bool getFileName = false;
    char * fileName = new char[BUFFER_SIZE];
    int fileNameLength;
    fstream file;
    long fileSize;

    free(args);
    pthread_detach(pthread_self());

    while(true) {
        buffLeng = read(sock, buff, BUFFER_SIZE);
        buff[buffLeng] = '\0';
        cout << "Client request: " << buff << endl;

        if (buffLeng <= 0 || strcmp(buff, "quit") == 0) {
            close(sock);
            cout << "A client has just disconnected." << endl;
            break;
        }

        if (getFileName) {
            strcpy(fileName, buff);
            fileNameLength = strlen(fileName);
            file.open(fileName, ios::in | ios::binary | ios::ate);

            if (file.is_open()) {
                fileSize = file.tellg();
                file.seekg(0, ios::beg);
                if (fileSize < 0) {
                    fileSize = 0;
                }

                memcpy(buff, &fileNameLength, sizeof(int));
                write(sock, buff, sizeof(int));
                strcpy(buff, fileName);
                write(sock, buff, strlen(buff));
                memcpy(buff, &fileSize, sizeof(long));
                write(sock, buff, sizeof(long));

                if (fileSize > 0) {
                    while (fileSize >= BUFFER_SIZE) {
                        file.read(buff, BUFFER_SIZE);
                        write(sock, buff, BUFFER_SIZE);

                        fileSize -= BUFFER_SIZE;
                    }

                    file.read(buff, fileSize);
                    write(sock, buff, fileSize);
                }

                file.close();

                strcpy(buff, "@ok");
            } else {
                fileNameLength = 0;
                memcpy(buff, &fileNameLength, sizeof(int));
                write(sock, buff, sizeof(int));
                strcpy(buff, "@notFound");
            }
            
            getFileName = false;
        } else {
            if (strcmp(buff, "get") == 0) {
                getFileName = true;
                strcpy(buff, "@getName");
            } else {
                strcpy(buff, "@invalidCommand");
            }
        }

        write(sock, buff, strlen(buff));
        cout << "Server response: " << buff << endl;
    }
    
    close(sock);

    return NULL;
}

int main(int argc, char ** argv) {
    int port = PORT;
    int servSock;
    int * sock;
    struct sockaddr_in servSockAddr, sockAddr;
    int servSockAddrLeng, sockAddrLeng;
    pthread_t tid;
    int flag = 1;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    servSock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servSockAddr, sizeof(servSockAddr));
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port = htons(port);
    servSockAddrLeng = sizeof(servSockAddr);

    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        cout << "Fail to set socket option !!" << endl;
        exit(1);
    }

    if (bind(servSock, (struct sockaddr *) &servSockAddr, servSockAddrLeng)) {
        cout << "Fail to bind address !!" << endl;
        exit(1);
    }

    if (listen(servSock, 10) < 0) {
        cout << "Fail to listen a socket !!" << endl;
        exit(1);
    }

    while (true) {
        cout << "Waiting for a client ..." << endl;
        sock = new int;
        * sock = accept(servSock, (struct sockaddr *) &sockAddr, (socklen_t *) &sockAddrLeng);
        cout << inet_ntoa(sockAddr.sin_addr) << " has just connected." << endl;
        pthread_create(&tid, NULL, &threadFunc, (void *) sock);
    }

    return 0;
}
