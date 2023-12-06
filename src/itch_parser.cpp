#include "include/itch_parser.hpp"
#include <endian.h>

#include <iostream>
using namespace std;

ITCH::Parser::Parser(const std::string& _filename) : Parser(_filename, defaultBufferSize) {
}

ITCH::Parser::Parser(const std::string& _filename, size_t _bufferSize) : bufferSize(_bufferSize), buffer(new char[bufferSize]) {
    itchFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    itchFile.open(_filename, std::ifstream::in | std::ifstream::binary);
}

ITCH::Parser::~Parser() {
    itchFile.close();
    delete buffer;
}

void ITCH::Parser::process() {
    itchFile.read(buffer, bufferSize);

    char* _buffer = buffer;
    while ((size_t)(_buffer - buffer) < bufferSize) {
        uint16_t msglen = be16toh(*(uint16_t *)_buffer);
        _buffer += 2;
        cout << "length " << msglen << endl;
        char type = *_buffer;
        cout << "type " << type << endl;
        _buffer += (msglen);
    }
}

