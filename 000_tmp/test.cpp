#include <iostream>
#include <filesystem>
int main() {
    std::string pwd = std::filesystem::current_path();
    std::cout << pwd << "\n";
    return 0;
}