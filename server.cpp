// server.cpp
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include <string>
#include <map>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

using namespace std;

int server_main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета!" << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка bind!" << endl;
#ifdef _WIN32
        closesocket(serverSock);
        WSACleanup();
#else
        close(serverSock);
#endif
        return 1;
    }

    listen(serverSock, 5);
    cout << "Сервер запущен на порту 5000" << endl;

    fd_set master;
    FD_ZERO(&master);
    FD_SET(serverSock, &master);

    map<SOCKET, string> clientNames;

    while (true) {
        fd_set copy = master;
        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++) {
            SOCKET sock = copy.fd_array[i];

            if (sock == serverSock) {
                // новый клиент
                SOCKET client = accept(serverSock, nullptr, nullptr);
                FD_SET(client, &master);

                // ждём первый пакет (логин)
                char nameBuf[1024];
                int bytes = recv(client, nameBuf, sizeof(nameBuf) - 1, 0);
                if (bytes > 0) {
                    nameBuf[bytes] = '\0';
                    clientNames[client] = nameBuf;
                }
                else {
                    clientNames[client] = "guest";
                }

                string msg = "[Сервер] " + clientNames[client] + " подключился\n";
                cout << msg;

                for (u_int j = 0; j < master.fd_count; j++) {
                    SOCKET outSock = master.fd_array[j];
                    if (outSock != serverSock && outSock != client) {
                        send(outSock, msg.c_str(), (int)msg.size(), 0);
                    }
                }
            }
            else {
                char buffer[1024];
                int bytesIn = recv(sock, buffer, sizeof(buffer), 0);

                if (bytesIn <= 0) {
                    string name = clientNames[sock];
                    string msg = "[Сервер] " + name + " отключился\n";
                    cout << msg;

                    clientNames.erase(sock);
                    FD_CLR(sock, &master);
#ifdef _WIN32
                    closesocket(sock);
#else
                    close(sock);
#endif

                    for (u_int j = 0; j < master.fd_count; j++) {
                        SOCKET outSock = master.fd_array[j];
                        if (outSock != serverSock) {
                            send(outSock, msg.c_str(), (int)msg.size(), 0);
                        }
                    }
                }
                else {
                    buffer[bytesIn] = '\0';
                    string msg = "[" + clientNames[sock] + "] " + buffer + "\n";
                    cout << msg;

                    for (u_int j = 0; j < master.fd_count; j++) {
                        SOCKET outSock = master.fd_array[j];
                        if (outSock != serverSock && outSock != sock) {
                            send(outSock, msg.c_str(), (int)msg.size(), 0);
                        }
                    }
                }
            }
        }
    }

#ifdef _WIN32
    closesocket(serverSock);
    WSACleanup();
#else
    close(serverSock);
#endif
    return 0;
}
