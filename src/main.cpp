#include "include/itch_reader.hpp"

int main() {
    ITCH::Parser parser("test.txt", 16384);
    parser.parseITCHFile();
}

