#pragma once

namespace Sym {
    struct Food {
        int score;
        int percent;
        int duration_time;
        char signal;

        Food(
            const int score = -1, const int weight = 0,
            const int duration_seconds = -1, const char signal = '*')
            : score(score),
              percent(weight),
              duration_time(duration_seconds),
              signal(signal) {
        }
    };
}
