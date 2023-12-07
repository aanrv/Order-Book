#include "include/itch_parser.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh

#include <iostream>                 // todo remove

ITCH::Parser::Parser(char const * _filename) : Parser(_filename, defaultBufferSize) {
}

ITCH::Parser::Parser(char const * _filename, size_t _bufferSize) : bufferSize(_bufferSize), buffer(new char[_bufferSize]) {
    if ((fdItch = open(_filename, O_RDONLY)) == -1) { delete buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
}

ITCH::Parser::~Parser() {
    close(fdItch);
    delete buffer;
}

void ITCH::Parser::process() {
    ssize_t bytesRead;
    int offset = 0;
    bool endOfSession = false;
    unsigned long long totalbytes = 0;
    constexpr size_t numBytesMessageLength = 2;
    // read file in chunks of bufferSize bytes (offset handles incomplete message from previous buffer)
    while (!endOfSession && (bytesRead = read(fdItch, buffer + offset, bufferSize - offset)) > 0) {
        offset = 0;
        char* _buffer = buffer;
        // read buffer message by message
        while (_buffer < (buffer + bufferSize)) {                       // while _buffer pointer has not gone out of bounds for buffer
            if ((_buffer + numBytesMessageLength) > (buffer + bufferSize)) {
                // handle case message size is partial
                // message sizes are 2 bytes but only 1 byte left in buffer
                offset = 1;
                buffer[0] = *_buffer;
                break;
            }
            uint16_t msglen = be16toh(*(uint16_t *)_buffer);
            if (msglen == 0) {
                // check end of session
                endOfSession = true;
                break;
            }
            if ((_buffer + numBytesMessageLength + msglen) > (buffer + bufferSize)) {
                // handle case current message is partial
                // message extends past last byte in buffer
                offset = ((buffer + bufferSize) - _buffer);             // length of partial message currently in buffer
                std::memcpy(buffer, _buffer, offset);                   // copy length of partial message (offset) from end of _buffer to start of buffer
                                                                        // exit loop, next read destination will start from buffer + offset, and read offset fewer bytes
                break;
            }
            _buffer += numBytesMessageLength;
            //char type = *_buffer;
            _buffer += (msglen);
            totalbytes += (numBytesMessageLength + msglen);
        }
    }
    std::cout << "total bytes read " << totalbytes << std::endl;
    if (bytesRead == -1) {
        std::cerr << "Failed to read bytes from start of file" << std::endl;
        std::cerr << "Stopping ITCH::Parser::process()" << std::endl;
    }
}

