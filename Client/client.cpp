// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Chuong trinh client

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char ** argv) {
    char addr[16];
    int port;
    int connectionSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];
    int dataLength;
    char * fileName;
    int fileSize;
    fstream file;
    bool getFileName = false;
    
    if (argc >= 3) {
        strcpy(addr, argv[1]);
        port = atoi(argv[2]);
    } else {
        strcpy(addr, "127.0.0.1");
        port = SERVER_PORT;
    }

    connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocket < 0) {
        cout << "Fail to create socket" << endl;
        exit(1);
    }

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(addr);
    serverAddress.sin_port = htons(port);

    if (connect(connectionSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Fail to connect" << endl;
        exit(1);
    }

    cout << "Connected" << endl;

    fileName = new char[100];

    while (true) {
        cout << "Client: ";
        cin.getline(buffer, BUFFER_SIZE - 1);

        if (strlen(buffer) > 0) {
            if (strcmp(buffer, "quit") == 0) {
                write(connectionSocket, buffer, strlen(buffer));
                break;
            }

            dataLength = write(connectionSocket, buffer, strlen(buffer));
            if (dataLength < 0) {
                cout << "Fail to sent" << endl;
                break;
            }

            if (getFileName) {
                read(connectionSocket, buffer, 5);
                dataLength = atoi(buffer);
                if (dataLength > 0) {
                    dataLength = read(connectionSocket, buffer, dataLength);
                    buffer[dataLength] = '\0';
                    cout << "Server: " << buffer << endl;

                    read(connectionSocket, buffer, 5);
                    dataLength = atoi(buffer);
                    dataLength = read(connectionSocket, buffer, dataLength);
                    buffer[dataLength] = '\0';
                    strcpy(fileName, buffer);
                    cout << "Server: \"" << fileName << "\"" << endl;

                    read(connectionSocket, buffer, 5);
                    dataLength = atoi(buffer);
                    dataLength = read(connectionSocket, buffer, dataLength);
                    buffer[dataLength] = '\0';
                    fileSize = atoi(buffer);
                    cout << "Server: " << fileSize << " bytes" << endl;

                    file.open(fileName, ios::out | ios::binary | ios::trunc);

                    if (file.is_open() && fileSize > 0) {
                        for (int i = BUFFER_SIZE; i < fileSize; i += BUFFER_SIZE) {
                            read(connectionSocket, buffer, BUFFER_SIZE);
                            file.write(buffer, BUFFER_SIZE);
                        }

                        read(connectionSocket, buffer, fileSize % BUFFER_SIZE);
                        file.write(buffer, fileSize % BUFFER_SIZE);
                    }

                    file.close();
                }             

                getFileName = false;
            }

            dataLength = read(connectionSocket, buffer, sizeof(buffer));
            if (dataLength < 0) {
                cout << "Fail to receive" << endl;
                break;
            }
            buffer[dataLength] = '\0';
            cout << "Server: " << buffer << endl;


            if (strcmp(buffer, "@getFileName") == 0) {
                getFileName = true;
            }
        }
    }

    close(connectionSocket);
    delete [] fileName;

    return 0;
}
