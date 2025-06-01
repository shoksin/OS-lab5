#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "employee.h"
#include <string>

enum class OperationType { READ, MODIFY, EXIT };

struct Request {
    OperationType type;
    int employeeId;
};

struct Response {
    bool success;
    employee emp;
};

static const char* PIPE_NAME = "\\\\.\\pipe\\EmployeePipe"; // "\\.\pipe\EmployeePipe" 


#endif // PROTOCOL_H
