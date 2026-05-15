#pragma once

#include <string>

namespace Sym {
    struct Food {
        int score;
        // 权重百分位数, 不是百分率
        int percent;
        int duration_time;
        std::string signal;

        Food(const int score = -1, const int weight = 0,
             const int duration_seconds = -1, std::string sig = "●")
            : score(score), percent(weight),
              duration_time(duration_seconds), signal(std::move(sig)) {
        }
    };
}
