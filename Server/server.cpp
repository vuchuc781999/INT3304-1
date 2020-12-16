// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Server xu ly messages

#include <asm-generic/socket.h>
#include <cstddef>
#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <vector>
#include <cstring>

using namespace std;

const int PORT = 9000;
const int BUFFER_SIZE = 1024;

pthread_mutex_t usersMutex = PTHREAD_MUTEX_INITIALIZER;

struct MessageHeader
{
    int senderId;
    int receiverId;
};

struct User
{
    int userId;
    int sockId;
    MessageHeader *headerBuffer;
    char *buffer;
    pthread_mutex_t bufferMutex;
    pthread_mutex_t sendingMutex;
    pthread_cond_t sendingCond;
};

vector<User *> onlineUsers;

void *receiveMessage(void *args)
{
    User *user = (User *)args;
    char *tempBuff = new char[sizeof(MessageHeader)];

    pthread_detach(pthread_self());

    pthread_mutex_lock(&(user->bufferMutex));
    while (true)
    {
        pthread_cond_wait(&(user->sendingCond), &(user->bufferMutex));
        int i = 0;
        for (; i < onlineUsers.size(); i++)
        {
            if (onlineUsers[i]->userId == user->userId)
            {
                break;
            }
        }
        if (i == onlineUsers.size())
        {
            break;
        }

        memcpy(tempBuff, user->headerBuffer, sizeof(MessageHeader));
        write(user->sockId, tempBuff, sizeof(MessageHeader));
        write(user->sockId, user->buffer, strlen(user->buffer));
    }

    pthread_mutex_unlock(&(user->bufferMutex));

    cout << "An user has just disconnected." << endl;

    delete[] tempBuff;
    delete user;

    return NULL;
}

void *sendMessage(void *args)
{
    User *user = new User(*((User *)args));
    char *buff = new char[BUFFER_SIZE];
    int buffLeng;
    int headerLen;
    bool getUserId = false;
    bool hadUserId = false;
    MessageHeader *header = new MessageHeader;

    pthread_detach(pthread_self());

    while (true)
    {
        if (hadUserId)
        {
            User *receivingUser = NULL;

            headerLen = read(user->sockId, buff, sizeof(MessageHeader));

            if (headerLen <= 0)
            {
                close(user->sockId);

                pthread_mutex_lock(&usersMutex);
                for (int i = 0; i < onlineUsers.size(); i++)
                {
                    if (onlineUsers[i]->userId == user->userId)
                    {
                        onlineUsers.erase(onlineUsers.begin() + i);
                        break;
                    }
                }
                pthread_mutex_unlock(&usersMutex);
                pthread_cond_signal(&(user->sendingCond));

                break;
            }

            memcpy(header, buff, sizeof(MessageHeader));
            buffLeng = read(user->sockId, buff, BUFFER_SIZE);
            buff[buffLeng] = '\0';

            if (buffLeng <= 0 || strcmp(buff, "quit") == 0)
            {
                close(user->sockId);

                pthread_mutex_lock(&usersMutex);
                for (int i = 0; i < onlineUsers.size(); i++)
                {
                    if (onlineUsers[i]->userId == user->userId)
                    {
                        onlineUsers.erase(onlineUsers.begin() + i);
                        break;
                    }
                }
                pthread_mutex_unlock(&usersMutex);
                pthread_cond_signal(&(user->sendingCond));

                break;
            }

            header->senderId = user->userId;
            for (int i = 0; i < onlineUsers.size(); i++)
            {
                if (header->receiverId == onlineUsers[i]->userId)
                {
                    receivingUser = onlineUsers[i];
                    break;
                }
            }

            if (receivingUser)
            {
                pthread_mutex_lock(&(receivingUser->sendingMutex));
                pthread_mutex_lock(&(receivingUser->bufferMutex));
                receivingUser->headerBuffer = header;
                strcpy(receivingUser->buffer, buff);
                pthread_cond_signal(&(receivingUser->sendingCond));
                pthread_mutex_unlock(&(receivingUser->bufferMutex));
                pthread_mutex_unlock(&(receivingUser->sendingMutex));
            }
            else
            {
                pthread_mutex_lock(&(user->sendingMutex));
                header->receiverId = header->senderId;
                write(user->sockId, header, sizeof(MessageHeader));
                pthread_mutex_unlock(&(user->sendingMutex));
            }
        }
        else
        {
            buffLeng = read(user->sockId, buff, BUFFER_SIZE);
            buff[buffLeng] = '\0';

            if (buffLeng <= 0 || strcmp(buff, "quit") == 0)
            {
                close(user->sockId);
                cout << "An user has just disconnected." << endl;
                break;
            }

            if (getUserId)
            {
                int userQuantity = 0;
                user->userId = atoi(buff);
                pthread_t pid;

                if (user->userId > 0)
                {
                    pthread_mutex_lock(&usersMutex);

                    int i = 0;
                    for (; i < onlineUsers.size(); i++)
                    {
                        if (user->userId == onlineUsers[i]->userId)
                        {
                            break;
                        }
                    }

                    if (i == onlineUsers.size())
                    {
                        hadUserId = true;
                        onlineUsers.insert(onlineUsers.end(), user);
                    }
                    pthread_mutex_unlock(&usersMutex);

                    if (hadUserId)
                    {
                        user->buffer = new char[BUFFER_SIZE];
                        user->bufferMutex = PTHREAD_MUTEX_INITIALIZER;
                        user->sendingMutex = PTHREAD_MUTEX_INITIALIZER;
                        user->sendingCond = PTHREAD_COND_INITIALIZER;

                        pthread_create(&pid, NULL, &receiveMessage, (void *)user);

                        userQuantity = onlineUsers.size();
                    }

                    memcpy(buff, &userQuantity, sizeof(int));
                    write(user->sockId, buff, sizeof(int));

                    if (userQuantity > 0)
                    {
                        for (int j = 0; j < userQuantity; j++)
                        {
                            memcpy(buff, &(onlineUsers[j]->userId), sizeof(int));
                            write(user->sockId, buff, sizeof(int));
                        }
                        strcpy(buff, "@youWereJoined");
                    }
                    else
                    {
                        strcpy(buff, "@userExsisted");
                    }
                }
                else
                {
                    memcpy(buff, &userQuantity, sizeof(int));
                    write(user->sockId, buff, sizeof(int));
                    strcpy(buff, "@idMustBeInteger");
                }
            }
            else
            {
                if (strcmp(buff, "start") == 0)
                {
                    getUserId = true;
                    strcpy(buff, "@getUserId");
                }
                else
                {
                    strcpy(buff, "@invalidCommand");
                }
            }

            write(user->sockId, buff, strlen(buff));
        }
    }

    delete[] buff;
    delete header;

    return NULL;
}

int main(int argc, char **argv)
{
    int port = PORT;
    int servSock;
    struct sockaddr_in servSockAddr, sockAddr;
    int servSockAddrLeng, sockAddrLeng;
    int flag = 1;
    pthread_t tid;

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    servSock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servSockAddr, sizeof(servSockAddr));
    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port = htons(port);
    servSockAddrLeng = sizeof(servSockAddr);

    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
    {
        cout << "Fail to set socket option !!" << endl;
        exit(1);
    }

    if (bind(servSock, (struct sockaddr *)&servSockAddr, servSockAddrLeng))
    {
        cout << "Fail to bind address !!" << endl;
        exit(1);
    }

    if (listen(servSock, 10) < 0)
    {
        cout << "Fail to listen a socket !!" << endl;
        exit(1);
    }

    while (true)
    {
        User user;

        cout << "Waiting for a client ..." << endl;
        user.sockId = accept(servSock, (struct sockaddr *)&sockAddr, (socklen_t *)&sockAddrLeng);
        cout << inet_ntoa(sockAddr.sin_addr) << " has just connected." << endl;
        pthread_create(&tid, NULL, &sendMessage, (void *)&user);
    }

    return 0;
}