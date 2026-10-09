#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <string>
int main() {
    const char* configured = std::getenv("HG_FAKE_UCI_MODE");
    const std::string mode = configured ? configured : "associated";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") std::cout << "id name HG Protocol Fixture\nuciok\n" << std::flush;
        else if (line == "isready") std::cout << "readyok\n" << std::flush;
        else if (line == "quit") break;
        else if (line.rfind("go ", 0) == 0) {
            if (mode == "crash") return 53;
            if (mode == "timeout") { Sleep(5000); continue; }
            if (mode == "illegal") { std::cout << "bestmove e2e4q\n" << std::flush; continue; }
            if (mode == "associated" || mode == "bound")
                std::cout << "info depth 8 multipv 2 score cp 42 " << (mode == "bound" ? "lowerbound " : "") << "nodes 321 time 1 pv e2e4 e7e5\n";
            std::cout << "info depth 9 multipv 1 score mate 6 nodes 654 time 2 pv d2d4 d7d5\n"
                      << "bestmove e2e4\n" << std::flush;
        }
    }
}
