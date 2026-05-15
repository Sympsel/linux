#include <memory>

#include "Conf.hpp"
#include "Game.hpp"
#include "Log.hpp"

Sym::Conf conf;

int main(const int argc, char *argv[]) {
    LOG_INFO() << "Config loading!";
    conf.Load();
    LOG_INFO() << "Config loaded!";

    const auto game = std::make_unique<Sym::Game>(
    conf["width"], conf["height"], conf["def_len"]
    );
    game->Run();
    return 0;
}
