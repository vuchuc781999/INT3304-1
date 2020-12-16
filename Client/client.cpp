// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Client

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
#include <pthread.h>

using namespace std;

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

struct MessageHeader
{
    int senderId;
    int receiverId;
};

void *receiveMessage(void *args)
{
    pthread_detach(pthread_self());

    int connectionSocket = *(int *)args;
    MessageHeader header;
    char *buffer = new char[BUFFER_SIZE];
    int bufferLength = 0;

    while (true)
    {
        read(connectionSocket, buffer, sizeof(MessageHeader));
        memcpy(&header, buffer, sizeof(MessageHeader));
        if (header.senderId == header.receiverId)
        {
            cout << "This user is offline !" << endl;
            continue;
        }
        bufferLength = read(connectionSocket, buffer, BUFFER_SIZE);
        buffer[bufferLength] = '\0';
        cout << "User " << header.senderId << ": " << buffer << endl;
    }

    delete[] buffer;

    return NULL;
}

int main(int argc, char **argv)
{
    char addr[16];
    int port;
    int connectionSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];
    int dataLength;
    bool getId = false;
    bool hadId = false;
    pthread_t tid;
    MessageHeader header;

    if (argc >= 3)
    {
        strcpy(addr, argv[1]);
        port = atoi(argv[2]);
    }
    else
    {
        strcpy(addr, "127.0.0.1");
        port = SERVER_PORT;
    }

    connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocket < 0)
    {
        cout << "Fail to create socket" << endl;
        exit(1);
    }

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(addr);
    serverAddress.sin_port = htons(port);

    if (connect(connectionSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cout << "Fail to connect" << endl;
        exit(1);
    }

    cout << "Connected" << endl;

    while (true)
    {
        if (hadId)
        {
            while (true)
            {
                cout << " -- Enter receiver's ID (N*) then enter your message -- " << endl;
                cin.getline(buffer, BUFFER_SIZE - 1);
                header.receiverId = atoi(buffer);
                if (header.receiverId > 0)
                {
                    break;
                }
            }

            if (header.receiverId == header.senderId)
            {
                cout << "Don't send yourself !" << endl;
                continue;
            }

            memcpy(buffer, &header, sizeof(MessageHeader));
            write(connectionSocket, buffer, sizeof(MessageHeader));
            cin.getline(buffer, BUFFER_SIZE - 1);

            if (strcmp(buffer, "quit") == 0)
            {
                write(connectionSocket, buffer, strlen(buffer));
                break;
            }

            dataLength = write(connectionSocket, buffer, strlen(buffer));
            if (dataLength < 0)
            {
                break;
            }
        }
        else
        {
            int userQuantity;
            int temp;

            cout << "Client: ";
            cin.getline(buffer, BUFFER_SIZE - 1);

            if (strlen(buffer) > 0)
            {
                if (strcmp(buffer, "quit") == 0)
                {
                    write(connectionSocket, buffer, strlen(buffer));
                    break;
                }

                dataLength = write(connectionSocket, buffer, strlen(buffer));
                if (dataLength < 0)
                {
                    cout << "Fail to sent" << endl;
                    break;
                }

                if (getId)
                {
                    header.senderId = atoi(buffer);
                    read(connectionSocket, buffer, sizeof(int));
                    memcpy(&userQuantity, buffer, sizeof(int));
                    if (userQuantity > 0)
                    {
                        cout << "It's having " << userQuantity << " online users:" << endl;
                        for (int i = 0; i < userQuantity; i++)
                        {
                            read(connectionSocket, buffer, sizeof(int));
                            memcpy(&temp, buffer, sizeof(int));
                            cout << temp << endl;
                        }

                        hadId = true;
                        pthread_create(&tid, NULL, &receiveMessage, (void *)&connectionSocket);
                    }
                }
                else
                {
                    getId = strcmp(buffer, "start") == 0;
                }

                dataLength = read(connectionSocket, buffer, sizeof(buffer));
                if (dataLength < 0)
                {
                    cout << "Fail to receive" << endl;
                    break;
                }
                buffer[dataLength] = '\0';
                cout << "Server: " << buffer << endl;
            }
        }
    }

    close(connectionSocket);

    return 0;
}
