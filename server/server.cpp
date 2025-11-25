#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>        // для работы с char массивами
#include <algorithm>      // для поиска в векторе
#include <winsock2.h>     // библиотека сокетов

// Линковка библиотеки (чтобы работало в Visual Studio)
// Для VS Code все равно нужен флаг -lws2_32
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Глобальные переменные (чтобы были видны везде)
vector<SOCKET> connections; 
mutex mtx; // мьютекс для защиты списка от одновременного доступа

// Функция для работы с отдельным клиентом
void ClientHandler(SOCKET current_client) {
    char msg[1024]; // буфер для сообщения

    while (true) {
        // Очищаем буфер перед приемом
        memset(msg, 0, sizeof(msg));

        // Ждем сообщение от клиента
        // recv останавливает программу тут, пока не придут данные
        int bytes = recv(current_client, msg, sizeof(msg), 0);

        // Если bytes <= 0, значит ошибка или клиент вышел
        if (bytes <= 0) {
            cout << "Client disconnected." << endl;
            break; 
        }

        // Выводим в консоль сервера
        cout << "Message: " << msg << endl;

        // РАССЫЛКА ВСЕМ (Broadcasting)
        // Блокируем доступ, чтобы другие потоки не мешали
        mtx.lock();
        for (int i = 0; i < connections.size(); i++) {
            // Отправляем всем, кроме самого отправителя
            if (connections[i] != current_client) {
                send(connections[i], msg, bytes, 0);
            }
        }
        mtx.unlock(); // Обязательно разблокируем!
    }

    // Когда цикл закончился (клиент вышел):
    closesocket(current_client);

    // Удаляем его из списка
    mtx.lock();
    auto it = find(connections.begin(), connections.end(), current_client);
    if (it != connections.end()) {
        connections.erase(it);
    }
    mtx.unlock();
}

int main() {
    // Включаем поддержку русского языка в консоли
    setlocale(LC_ALL, "Russian");
    
    // 1. Загрузка библиотеки Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Error: WSAStartup failed" << endl;
        return 1;
    }

    // 2. Создаем сокет сервера
    // AF_INET = интернет (IP), SOCK_STREAM = TCP
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cout << "Error: Socket creation failed" << endl;
        return 1;
    }

    // 3. Настраиваем адрес и порт
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(54000); // Порт 54000
    addr.sin_addr.s_addr = INADDR_ANY; // Слушаем любой IP

    // 4. Привязываем сокет (bind)
    if (bind(server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Error: Bind failed. Port busy?" << endl;
        return 1;
    }

    // 5. Запускаем прослушку
    listen(server_socket, SOMAXCONN);
    cout << "Server started on port 54000..." << endl;

    // Главный цикл приема подключений
    while (true) {
        sockaddr_in client_addr;
        int addr_size = sizeof(client_addr);

        // Ждем подключения (программа стоит тут)
        SOCKET new_conn = accept(server_socket, (sockaddr*)&client_addr, &addr_size);

        if (new_conn == INVALID_SOCKET) {
            continue; // Если ошибка, просто идем дальше
        }

        cout << "New client connected!" << endl;

        // Добавляем в общий список
        mtx.lock();
        connections.push_back(new_conn);
        mtx.unlock();

        // Создаем поток для этого клиента
        thread t(ClientHandler, new_conn);
        t.detach(); // Отпускаем поток в свободное плавание
    }

    // Чистим за собой (хотя до сюда код не дойдет из-за while(true))
    closesocket(server_socket);
    WSACleanup();
    return 0;
}