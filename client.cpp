// client.cpp
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
typedef int SOCKET;
#endif

using namespace std;

atomic<bool> running(true);

// 🔹 поток приёма сообщений
void receiveLoop(SOCKET sock) {
    char buffer[1024];
    while (running) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            cout << "\n" << buffer << "\n> ";
            cout.flush();
        }
        else if (bytes == 0) {
            cout << "\n[Сервер отключился]\n";
            running = false;
            break;
        }
    }
}

int client_main() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Ошибка инициализации Winsock\n";
        return 1;
    }
#endif

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) != 1) {
        cerr << "inet_pton: некорректный IP-адрес\n";
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Не удалось подключиться к серверу\n";
        return 1;
    }

    // 🔹 всегда спрашиваем логин
    string login;
    cout << "Введите ваш логин: ";
    getline(cin, login);

    // отправляем логин серверу
    send(sock, login.c_str(), (int)login.size(), 0);

    cout << "Теперь можно писать сообщения (exit для выхода):\n";

    // 🔹 запускаем поток приёма
    thread receiver(receiveLoop, sock);

    string msg;
    while (running) {
        cout << "> ";
        if (!getline(cin, msg)) break;
        if (msg == "exit") {
            running = false;
            break;
        }
        send(sock, msg.c_str(), (int)msg.size(), 0);
    }

    receiver.join();

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}
