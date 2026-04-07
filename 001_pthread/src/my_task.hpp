#pragma once

#include <iostream>

class MyTask {
    public:
    MyTask(const std::string &who, int x, int y) : _who(who), _x(x), _y(y) {}
    MyTask() : _x(0), _y(0) {}

    void operator()() const {
        std::cout << "MyTask: " << _x << " + " << _y << " = " << _x + _y << std::endl;
    }

    private:
    int _x, _y;
    std::string _who;
};