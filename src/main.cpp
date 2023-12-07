#include "include/itch_parser.hpp"

int main() {
    ITCH::Parser parser("test.txt", 16384);
    parser.process();
}

