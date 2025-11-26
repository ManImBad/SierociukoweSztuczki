#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

// Linkowanie biblioteki Winsock (dla Visual Studio / MSVC)
#pragma comment(lib, "ws2_32.lib")

#define PORT 9999       // Port nasłuchujący
#define BUFFER_SIZE 2048 // Zwiększony bufor dla pewności

int main() {
    // --- 1. Inicjalizacja Winsock ---
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    std::cout << "--- Odbiornik C++ (Raw Sensors) ---" << std::endl;

    // --- 2. Tworzenie gniazda UDP ---
    SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (recvSocket == INVALID_SOCKET) {
        std::cerr << "Socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // --- 3. Bindowanie (Nasłuchiwanie) ---
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_port = htons(PORT);
    service.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(recvSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Czekam na dane na porcie " << PORT << "..." << std::endl;

    // --- 4. Pętla Odbioru ---
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    char buffer[BUFFER_SIZE];

    while (true) {
        // Odbierz dane
        int bytesReceived = recvfrom(recvSocket, buffer, BUFFER_SIZE, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);

        if (bytesReceived > 0) {
            // Null-terminate string
            buffer[bytesReceived] = '\0';
            std::string message(buffer);

            // Zmienne na dane
            double ax, ay, az;
            double gx, gy, gz;
            double mx, my, mz;
            long long timestamp; // Timestamp to duża liczba całkowita

            // Parsowanie (kolejność musi być identyczna jak w Pythonie!)
            std::stringstream ss(message);
            if (ss >> ax >> ay >> az >> gx >> gy >> gz >> mx >> my >> mz >> timestamp) {
                
                // --- TU BĘDZIE TWOJA FUZJA DANYCH (np. Filtr Madgwicka) ---
                
                // Na razie tylko wyświetlamy:
                std::cout << "TS: " << timestamp << " | "
                          << "Acc: [" << ax << ", " << ay << ", " << az << "] "
                          << "Gyr: [" << gx << ", " << gy << ", " << gz << "] "
                          << "Mag: [" << mx << ", " << my << ", " << mz << "]" 
                          << std::endl;

            } else {
                std::cerr << "Blad parsowania pakietu: " << message << std::endl;
            }
        }
    }

    closesocket(recvSocket);
    WSACleanup();
    return 0;
}