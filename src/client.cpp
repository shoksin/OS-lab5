#include <windows.h>
#include <iostream>
#include "protocol.h"

int main() {
    std::cout << "Connecting to server...\n";
    HANDLE pipe;
    while (true) {
        pipe = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,            // no sharing
            NULL,         // default security
            OPEN_EXISTING,
            0,            // no attributes
            NULL
        );
        if (pipe != INVALID_HANDLE_VALUE)
            break;
        Sleep(100);
    }
    std::cout << "Connected!\n";

    DWORD written = 0, readBytes = 0;
    Request req;
    Response res;

    while (true) {
        std::cout << "1-read, 2-modify, 3-exit: ";
        int op; std::cin >> op;
        if (op == 3) break;

        req.type = (op == 1 ? OperationType::READ : OperationType::MODIFY);
        std::cout << "Enter ID: ";
        std::cin >> req.employeeId;

        // посылаем запрос
        WriteFile(pipe, &req, sizeof(req), &written, NULL);

        // получаем ответ
        ReadFile(pipe, &res, sizeof(res), &readBytes, NULL);

        if (res.success) {
            std::cout << "Employee: "
                      << res.emp.num << " "
                      << res.emp.name << " "
                      << res.emp.hours << "\n";
        } else {
            std::cout << "Not found\n";
        }
    }

    CloseHandle(pipe);
    return 0;
}
