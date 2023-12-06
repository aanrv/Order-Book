#include "include/itch_parser.hpp"

int main() {
    ITCH::Parser parser("test.txt", 8192);
    parser.process();
}

