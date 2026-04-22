#pragma once

#include <pthread.h>
#include <unistd.h>

#include <functional>
#include <iostream>

#include "log.hpp"
using namespace sym;

using task_t = std::function<void()>;

void HttpParse() {
    char name[64];
    pthread_getname_np(pthread_self(), name, sizeof name);
    LOG(log_level_t::INFO) << "Http is parsing by " << name;
    sleep(3);
}

void JsonParse() {
    char name[64];
    pthread_getname_np(pthread_self(), name, sizeof name);
    LOG(log_level_t::INFO) << "Json is parsing by " << name;
    sleep(5);
}