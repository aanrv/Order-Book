#include "include/itch_reader.hpp"

int main() {
    ITCH::Reader reader("test.txt", 16384);
    reader.readITCHFile();
}

