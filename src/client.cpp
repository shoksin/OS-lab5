#include <windows.h>
#include <iostream>
#include "protocol.h"

int main()
{
    std::cout << "Connecting to server...\n";
    HANDLE pipe;
    while (true)
    {
        pipe = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
        if (pipe != INVALID_HANDLE_VALUE)
            break;
        Sleep(100);
    }
    std::cout << "Connected!\n";

    DWORD written = 0, readBytes = 0;
    Request req;
    Response res;

    while (true)
    {
        std::cout << "1-read, 2-modify, 3-exit: ";

        std::string op_str;
        std::getline(std::cin, op_str);

        if (op_str.empty())
        {
            continue;
        }

        int op;

        try
        {
            op = std::stoi(op_str);
        }
        catch (...)
        {
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (op == 3)
            break;
        if (op < 1 || op > 2)
        {
            std::cout << "Incorrect operation\n";
            continue;
        }

        req.type = (op == 1 ? OperationType::READ : OperationType::MODIFY);
        std::cout << "Enter ID: ";
        //std::cin >> req.employeeId;
        //std::cin.ignore(); // чтобы игнорить \n

        std::string id_str;
        while (true)
        {
            std::getline(std::cin, id_str);
            try
            {
                req.employeeId = std::stoi(id_str);
                break;
            }
            catch (...)
            {
                std::cout << "Invalid ID. Enter integer: ";
            }
        }





        WriteFile(pipe, &req, sizeof(req), &written, NULL);

        ReadFile(pipe, &res, sizeof(res), &readBytes, NULL);

        if (res.success)
        {
            std::cout << "Employee: " << res.emp.num << " " << res.emp.name << " " << res.emp.hours << "\n";
        }
        else
        {
            std::cout << "Not found\n";
        }
    }

    CloseHandle(pipe);
    return 0;
}
