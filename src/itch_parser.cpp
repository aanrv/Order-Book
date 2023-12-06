#include "include/itch_parser.hpp"
#include <cstring>                  // memcpy
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>

#include <iostream>
using namespace std;

ITCH::Parser::Parser(char const * _filename) : Parser(_filename, defaultBufferSize) {
}

ITCH::Parser::Parser(char const * _filename, size_t _bufferSize) : bufferSize(_bufferSize), buffer(new char[bufferSize]) {
    if ((fdItch = open(_filename, O_RDONLY)) == -1) { delete buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
}

ITCH::Parser::~Parser() {
    close(fdItch);
    delete buffer;
}

void ITCH::Parser::process() {
    ssize_t bytesRead;
    int offset = 0;
    // read file in chunks of bufferSize bytes (offset handles incomplete message from previous buffer)
    while ((bytesRead = read(fdItch, buffer + offset, bufferSize - offset)) > 0) {
        offset = 0;
        char* _buffer = buffer;
        // read buffer message by message
        while ((size_t)(_buffer - buffer) < bufferSize) {               // todo can size_t _buffer - buffer overflow/cause issues?
            uint16_t msglen = be16toh(*(uint16_t *)_buffer);
//            cout << "length " << msglen << endl;
            if ((_buffer + 2 + msglen) >= (buffer + bufferSize)) {      // if this message extends past remaining buffer
//                cout << "message larger than remaining buffer" << endl;
                offset = ((buffer + bufferSize) - _buffer);             // length of partial message in buffer
                cout << "offset " << offset << endl;
                memcpy(buffer, _buffer, offset);                        // copy length of partial message (offset) from end of _buffer to start of buffer
                                                                        // exit loop, next read destination will start from buffer + offset, and read offset fewer bytes
                break;
            }
            _buffer += 2;
            char type = *_buffer;
            cout << "type " << type << " len " << msglen << endl;
            _buffer += (msglen);
//            cout << "buffer size: " << bufferSize << ", buffer needed: " << _buffer - buffer << endl;
            offset = 0;
        }
        _buffer = nullptr;
        if (bytesRead == -1) {
            cerr << "Failed to read bytes from file" << endl;
            cerr << "Stopping ITCH::Parser::process()" << endl;
        }
    }
    if (bytesRead == -1) {
        cerr << "Failed to read bytes from start of file" << endl;
        cerr << "Stopping ITCH::Parser::process()" << endl;
    }
}
// 0 4 1 2 3 4
// . . . . .
