// CalcTypes.hpp —— 无依赖的纯数据结构
#pragma once

struct CalcRequest {
    int x = 0;
    int y = 0;
    char oper = '+';
};

enum class CalcStatus : int {
    SUCCESS = 0,
    DIV_ZERO = 1,
    MOD_ZERO = 2,
    UNKNOWN_OP = 3
};

struct CalcResponse {
    int result = 0;
    CalcStatus status = CalcStatus::UNKNOWN_OP;
};