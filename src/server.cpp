#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm> // std::find_if
#include <cstring>   // strncpy

#include "employee.h"
#include "protocol.h"

std::vector<employee> db;
std::mutex db_mutex;

void clientHandler(HANDLE pipe) {
    DWORD bytesRead = 0, bytesWritten = 0;
    Request req;
    Response res;

    while (true) {
        if (!ReadFile(pipe, &req, sizeof(req), &bytesRead, NULL) || bytesRead == 0)
            break;

        {
            std::unique_lock<std::mutex> lock(db_mutex);
            auto it = std::find_if(db.begin(), db.end(),
                [&](const employee &e) { return e.num == req.employeeId; });

            if (it != db.end()) {
                res.success = true;
                res.emp = *it;

                if (req.type == OperationType::MODIFY) {
                    std::cout << "Modifying ID=" << it->num << std::endl;
                    std::cout << "New name: ";
                    std::string nm; std::cin >> nm;
                    std::strncpy(it->name, nm.c_str(), sizeof(it->name)-1);
                    it->name[sizeof(it->name)-1] = '\0';

                    std::cout << "New hours: ";
                    std::cin >> it->hours;
                    res.emp = *it;
                }
            } else {
                res.success = false;
            }
        }

        WriteFile(pipe, &res, sizeof(res), &bytesWritten, NULL);
    }

    FlushFileBuffers(pipe);
    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
}

int main() {
    std::string filename;
    std::cout << "Enter filename: ";
    std::cin >> filename;

    int n;
    std::cout << "Enter number of employees: ";
    std::cin >> n;
    db.resize(n);
    for (int i = 0; i < n; ++i) {
        std::cout << "ID, name, hours: ";
        std::cin >> db[i].num >> db[i].name >> db[i].hours;
    }

    // Запись начальных данных в файл
    {
        HANDLE hFile = CreateFileA(
            filename.c_str(),
            GENERIC_WRITE,
            0, NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        DWORD written = 0;
        WriteFile(hFile, db.data(), DWORD(db.size() * sizeof(employee)), &written, NULL);
        CloseHandle(hFile);
    }

    std::cout << "Server listening on " << PIPE_NAME << std::endl;

    // Создаём именованный канал
    HANDLE pipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        512, 512,
        0, NULL
    );
    if (pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create pipe\n";
        return 1;
    }

    while (true) {
        if (ConnectNamedPipe(pipe, NULL) ||
            GetLastError() == ERROR_PIPE_CONNECTED) {
            std::thread(clientHandler, pipe).detach();

            // Создаём новый экземпляр для следующих клиентов
            pipe = CreateNamedPipeA(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                512, 512,
                0, NULL
            );
            if (pipe == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to create pipe instance\n";
                break;
            }
        }
    }
    return 0;
}
