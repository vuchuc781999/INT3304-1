// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Client nhan file

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
    char * buffer;
    int dataLength;
    char * fileName;
    long fileSize;
    fstream file;
    bool getFile = false;
    
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

    buffer = new char[BUFFER_SIZE];
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

            if (getFile) {
                read(connectionSocket, buffer, sizeof(int));
                memcpy(&dataLength, buffer, sizeof(int));

                if (dataLength > 0) {
                    dataLength = read(connectionSocket, buffer, dataLength);
                    buffer[dataLength] = '\0';
                    strcpy(fileName, buffer);
                    cout << "Server: \"" << fileName << "\"" << endl;

                    read(connectionSocket, buffer, sizeof(long));
                    memcpy(&fileSize, buffer, sizeof(long));
                    cout << "Server: " << fileSize << " bytes" << endl;

                    file.open(fileName, ios::out | ios::binary | ios::trunc);

                    if (file.is_open() && fileSize > 0) {
                        while (fileSize >= BUFFER_SIZE) {
                            read(connectionSocket, buffer, BUFFER_SIZE);
                            file.write(buffer, BUFFER_SIZE);
                            fileSize -= BUFFER_SIZE;
                        }

                        read(connectionSocket, buffer, fileSize);
                        file.write(buffer, fileSize);
                    }

                    file.close();
                }

                getFile = false;
            }

            dataLength = read(connectionSocket, buffer, BUFFER_SIZE);
            if (dataLength < 0) {
                cout << "Fail to receive" << endl;
                break;
            }
            buffer[dataLength] = '\0';
            cout << "Server: " << buffer << endl;

            if (strcmp(buffer, "@getName") == 0) {
                getFile = true;
            }
        }
    }

    close(connectionSocket);
    delete [] fileName;

    return 0;
}
