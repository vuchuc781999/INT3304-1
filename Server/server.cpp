// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Chuong trinh server dung de gui file

#include <asm-generic/socket.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

using namespace std;

#define PORT 9000
#define BUFFER_SIZE 1024

char * itoa(int n) {
    char * a = new char[10];
    int i = 0, length;
    char temp;

    while (n > 0) {
        a[i] = n % 10 + '0';
        n /= 10;
        i++;
    }
    length = i;
    a[length] ='\0';

    i /= 2;

    for (int j = 0; j < i; j++) {
        temp = a[j];
        a[j] = a[length - 1 - j];
        a[length - 1 - j] = temp;
    }

    return a;
}

int main(int argc, char ** argv) {
    int port = PORT;
    int serverSocket, clientSocket;
    struct sockaddr_in serverSocketAddress, clientSocketAddress;
    int serverSocketAddressLength, clientSocketAddressLength;
    char * buffer;
    int bufferLength;
    int flag = 1;
    bool getFileName = false;
    char * fileName = new char[100];
    fstream file;
    int fileSize;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverSocketAddress, sizeof(serverSocketAddress));
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverSocketAddress.sin_port = htons(port);
    serverSocketAddressLength = sizeof(serverSocketAddress);

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        cout << "Fail to set socket option !!" << endl;
        exit(1);
    }

    if (bind(serverSocket, (struct sockaddr *) &serverSocketAddress, serverSocketAddressLength) < 0) {
        cout << "Fail to bind address !!" << endl;
        exit(1);
    }

    if (listen(serverSocket, 10) < 0) {
        cout << "Fail to listen a socket !!" << endl;
        exit(1);
    }

    buffer = new char[BUFFER_SIZE];

    while (true) {
        cout << "Waiting for a client..." << endl;

        clientSocket = accept(serverSocket, (struct sockaddr *) &clientSocketAddress, (socklen_t *) &clientSocketAddressLength);

        cout << "Client " << inet_ntoa(clientSocketAddress.sin_addr) << " connected." << endl;

        while (true) {
            bufferLength = read(clientSocket, buffer, BUFFER_SIZE);
            buffer[bufferLength] = '\0';

            if (bufferLength <= 0 || strcmp(buffer, "quit") == 0) {
                close(clientSocket);
                cout << "A client has just disconnected !!" << endl;
                break;
            }
            
            cout << "Client requested: " << buffer << endl;

            if (getFileName) {
                strcpy(fileName, buffer);
                file.open(fileName, ios::in | ios::binary | ios::ate);

                if (file.is_open()) {
                    strcpy(buffer, itoa(7));
                    write(clientSocket, buffer, 5);
                    strcpy(buffer, "@fileOk");
                    write(clientSocket, buffer, strlen(buffer));

                    strcpy(buffer, itoa(strlen(fileName)));
                    write(clientSocket, buffer, 5);
                    write(clientSocket, fileName, strlen(fileName));

                    fileSize = file.tellg();
                    file.seekg(0, ios::beg);
                    
                    if (fileSize < 0) {
                        fileSize = 0;
                    }

                    cout << "File: \"" << fileName << "\" - " << fileSize << " bytes" << endl;

                    strcpy(buffer, itoa(strlen(itoa(fileSize))));
                    write(clientSocket, buffer, 5);
                    strcpy(buffer, itoa(fileSize));
                    write(clientSocket, buffer, strlen(buffer));

                    if (fileSize > 0) {
                        for (int i = BUFFER_SIZE; i < fileSize; i += BUFFER_SIZE) {
                            file.read(buffer, BUFFER_SIZE);
                            write(clientSocket, buffer, BUFFER_SIZE);
                        }

                        file.read(buffer, fileSize % BUFFER_SIZE);
                        write(clientSocket, buffer, fileSize % BUFFER_SIZE);
                    }

                    file.close();

                    strcpy(buffer, "@endOfFile");
                } else {
                    strcpy(buffer, "0");
                    write(clientSocket, buffer, 5);
                    strcpy(buffer, "@fileNotFound");
                }
                getFileName = false;
            } else {
                if (strcmp(buffer, "get") == 0) {
                    getFileName = true;
                    strcpy(buffer, "@getFileName");
                } else {
                    strcpy(buffer, "@invalidCommand");
                }
            }

            write(clientSocket, buffer, strlen(buffer));
            cout << "Server responded: " << buffer << endl;
        }
    }

    return 1;
}
