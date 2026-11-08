#include <cstdio>
#include <cstring>
#include <exception>
#include <meta>
#include <tuple>

import scrivlr_application;

int main(int argc, char** argv) {
    try {
        const bool SMOKE_TEST =
            argc == 2 && std::strcmp(argv[1], "--smoke-test") == 0;
        Scrivlr::Application app(SMOKE_TEST);
        return app.run();
    } catch (const std::exception& error) {
        std::ignore = std::fprintf(stderr, "scrivlr: %s\n", error.what());
        return 1;
    }
}
