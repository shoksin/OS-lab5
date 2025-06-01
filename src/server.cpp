#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <limits>

#include "employee.h"
#include "protocol.h"

std::vector<employee> db;
std::mutex db_mutex;

void clientHandler(HANDLE pipe)
{
    DWORD bytesRead = 0, bytesWritten = 0;
    Request req;
    Response res;

    while (true)
    {
        if (!ReadFile(pipe, &req, sizeof(req), &bytesRead, NULL) || bytesRead == 0)
            break;

        {
            std::unique_lock<std::mutex> lock(db_mutex);
            auto it = std::find_if(db.begin(), db.end(),
                                   [&](const employee &e)
                                   { return e.num == req.employeeId; });

            if (it != db.end())
            {
                res.success = true;
                res.emp = *it;

                if (req.type == OperationType::MODIFY)
                {
                    std::cout << "Modifying ID=" << it->num << std::endl;
                    std::cout << "New name: ";
                    std::string nm;
                    std::cin >> nm;
                    std::strncpy(it->name, nm.c_str(), sizeof(it->name) - 1);
                    it->name[sizeof(it->name) - 1] = '\0';

                    std::cout << "New hours: ";
                    std::cin >> it->hours;
                    res.emp = *it;
                }
            }
            else
            {
                res.success = false;
            }
        }

        WriteFile(pipe, &res, sizeof(res), &bytesWritten, NULL);
    }

    FlushFileBuffers(pipe);
    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
}

int main()
{
    std::string filename;
    std::cout << "Enter filename: ";
    std::getline(std::cin, filename);

    int n;
    std::cout << "Enter number of employees: ";
    std::string n_str;
    while (true)
    {
        std::getline(std::cin, n_str);
        try
        {
            n = std::stoi(n_str);
            if (n <= 0)
            {
                std::cout << "Invalid input. Enter positive number: ";
                continue;
            }
            break;
        }
        catch (...)
        {
            std::cout << "Invalid input. Enter positive number: ";
        }
    }

    db.resize(n);
    for (int i = 0; i < n; ++i)
    {
        std::cout << "Employee " << (i + 1) << ":\n";

        std::cout << "ID: ";
        std::string id_str;
        while (true)
        {
            std::getline(std::cin, id_str);
            try
            {
                db[i].num = std::stoi(id_str);
                break;
            }
            catch (...)
            {
                std::cout << "Invalid ID. Enter integer:\n";
            }
        }

        std::cout << "Name: ";
        std::string name_str;
        std::getline(std::cin, name_str);
        std::strncpy(db[i].name, name_str.c_str(), sizeof(db[i].name) - 1);
        db[i].name[sizeof(db[i].name) - 1] = '\0';

        std::cout << "Hours: ";
        std::string hours_str;
        while (true)
        {
            std::getline(std::cin, hours_str);
            try
            {
                db[i].hours = std::stoi(hours_str);
                if (db[i].hours < 0)
                {
                    std::cout << "Hours must be non-negative. Try again:\n";
                    continue;
                }
                break;
            }
            catch (...)
            {
                std::cout << "Invalid hours. Enter non-negative number.\n";
            }
        }
    }

    {
        HANDLE hFile = CreateFileA(
            filename.c_str(),
            GENERIC_WRITE,
            0, NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        DWORD written = 0;
        WriteFile(hFile, db.data(), DWORD(db.size() * sizeof(employee)), &written, NULL);
        CloseHandle(hFile);
    }

    std::cout << "Server listening on " << PIPE_NAME << std::endl;

    HANDLE pipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        512, 512,
        0, NULL);
    if (pipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to create pipe\n";
        return 1;
    }

    while (true)
    {
        if (ConnectNamedPipe(pipe, NULL) ||
            GetLastError() == ERROR_PIPE_CONNECTED)
        {
            std::thread(clientHandler, pipe).detach();

            pipe = CreateNamedPipeA(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                512, 512,
                0, NULL);
            if (pipe == INVALID_HANDLE_VALUE)
            {
                std::cerr << "Failed to create pipe instance\n";
                break;
            }
        }
    }
    return 0;
}
