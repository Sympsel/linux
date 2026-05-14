#include <memory>

#include "Game.hpp"

int main(const int argc, char *argv[]) {
    const auto game = std::make_unique<Sym::Game>(
        60, 20, 5
    );
    game->Run();
    return 0;
}