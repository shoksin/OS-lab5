#include "protocol.h"
#include <gtest/gtest.h>

TEST(ProtocolTest, EmployeeCompare) {
    employee e1{1, "John", 40.0};
    employee e2{1, "John", 40.0};
    EXPECT_EQ(e1.num, e2.num);
    EXPECT_STREQ(e1.name, e2.name);
    EXPECT_EQ(e1.hours, e2.hours);
}

TEST(ProtocolTest, RequestSerialization) {
    Request r{OperationType::MODIFY, 123};
    EXPECT_EQ(r.type, OperationType::MODIFY);
    EXPECT_EQ(r.employeeId, 123);
}