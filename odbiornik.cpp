// main.cpp
// Kompiluj (na Windows) za pomocą: 
// g++ main.cpp -o odbiornik.exe -lws2_32
// lub w Visual Studio (pamiętaj o linkowaniu biblioteki ws2_32.lib)

#include <iostream>
#include <string>
#include <sstream> // Do łatwego parsowania stringa
#include <winsock2.h>
#include <ws2tcpip.h>

// Mówimy kompilatorowi, żeby linkował bibliotekę Winsock
#pragma comment(lib, "ws2_32.lib")

#define PORT 9999      // Ten sam port, na który wysyła Python
#define BUFFER_SIZE 1024 // Wystarczająco duży bufor na nasze dane

int main() {
    WSADATA wsaData;
    SOCKET recvSocket = INVALID_SOCKET;
    sockaddr_in service;

    // --- 1. Inicjalizacja Winsock (tylko na Windows) ---
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    std::cout << "--- Odbiornik C++ ---" << std::endl;
    std::cout << "Inicjalizacja Winsock OK." << std::endl;

    // --- 2. Tworzenie gniazda UDP ---
    recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (recvSocket == INVALID_SOCKET) {
        std::cerr << "Tworzenie gniazda nie powiodlo sie: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // --- 3. Bindowanie gniazda do portu 9999 ---
    service.sin_family = AF_INET;
    service.sin_port = htons(PORT);
    service.sin_addr.s_addr = inet_addr("127.0.0.1"); // Słuchaj tylko na localhost

    if (bind(recvSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Nasluchuje na udp://127.0.0.1:" << PORT << std::endl;

    // --- 4. Główna pętla odbioru danych ---
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytesReceived = recv(recvSocket, buffer, BUFFER_SIZE, 0);
        
        if (bytesReceived > 0) {
            // Zamień surowe dane na string (dodaj terminator null)
            buffer[bytesReceived] = '\0';
            std::string message(buffer);

            // Użyj stringstream do parsowania danych oddzielonych spacją
            std::stringstream ss(message);
            double pitch, roll, yaw, ax, ay, az;

            if (ss >> pitch >> roll >> yaw >> ax >> ay >> az) {
                // Mamy dane! Zrób z nimi, co chcesz.
                std::cout << "Odebrano w C++: P=" << pitch << " R=" << roll << " Y=" << yaw 
                          << " | Ax=" << ax << " Ay=" << ay << " Az=" << az << std::endl;
            } else {
                std::cerr << "Blad parsowania stringa: " << message << std::endl;
            }
        }
    }

    // (Nigdy tu nie dojdziemy, ale tak się sprząta)
    closesocket(recvSocket);
    WSACleanup();
    return 0;
}