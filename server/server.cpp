#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<SOCKET> clients;
mutex clientsMutex;

void RemoveClient(SOCKET clientSocket) {
    lock_guard<mutex> lock(clientsMutex);
    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
        cout << "Клиент удален. Всего клиентов: " << clients.size() << endl;
    }
}

void BroadcastMessage(string message, SOCKET senderSocket) {
    lock_guard<mutex> lock(clientsMutex);
    for (SOCKET client : clients) {
        if (client != senderSocket) {
            int sendResult = send(client, message.c_str(), message.size() + 1, 0);
            if (sendResult == SOCKET_ERROR) {
               
            }
        }
    }
}

void HandleClient(SOCKET clientSocket) {
    char buffer[4096];
    
    while (true) {
        ZeroMemory(buffer, 4096);
        
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);

        if (bytesReceived <= 0) {
            cout << "Клиент отключился." << endl;
            break; 
        }

        string msg(buffer, 0, bytesReceived);
        cout << "Получено: " << msg << endl;
        BroadcastMessage(msg, clientSocket);
    }

    closesocket(clientSocket);
    RemoveClient(clientSocket);
}

int main() {
    SetConsoleOutputCP(65001);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Ошибка WSAStartup." << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Ошибка создания сокета." << endl;
        return 1;
    }
    
    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(54000);
    serverHint.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        cout << "Ошибка привязки (bind)." << endl;
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Ошибка прослушивания (listen)." << endl;
        return 1;
    }

    cout << "Сервер запущен и слушает порт 54000..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) continue;

        char host[NI_MAXHOST];      
        char service[NI_MAXSERV]; 
        ZeroMemory(host, NI_MAXHOST); 
        ZeroMemory(service, NI_MAXSERV);

        if (getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            cout << host << " подключился через порт " << service << endl;
        } else {
            inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
            cout << host << " подключился через порт " << ntohs(clientAddr.sin_port) << endl;
        }
        
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        thread t(HandleClient, clientSocket);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}