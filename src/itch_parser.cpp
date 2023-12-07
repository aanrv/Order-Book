#include "include/itch_parser.hpp"
#include <cstring>                  // memcpy
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>

#include <iostream>
using namespace std;

void printchar(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        cout << s[i];
    }
    cout << endl;
}

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
    // read file in chunks of bufferSize bytes (offset handles incomplete message from previous buffer)
    while (!endOfSession && (bytesRead = read(fdItch, buffer + offset, bufferSize - offset)) > 0) {
        offset = 0;
        char* _buffer = buffer;
        // read buffer message by message
        while ((size_t)(_buffer - buffer) < bufferSize) {               // todo can size_t _buffer - buffer overflow/cause issues?
            if ((_buffer + 1) >= (buffer + bufferSize)) {               // handle case: message size (2 bytes) data is partial i.e. only 1 byte left in buffer
                offset = 1;
                cout << "offset 1 message size" << endl;
                buffer[0] = *_buffer;
                break;
            }
            uint16_t msglen = be16toh(*(uint16_t *)_buffer);
            if (msglen == 0) {                                          // check end of session
                cout << "0 msg len: " << endl;
                endOfSession = true;
                break;
            }
//            cout << "length " << msglen << endl;
            if ((_buffer + 2 + msglen) >= (buffer + bufferSize + 1)) {      // handle case: this message extends past remaining buffer
//                cout << "message larger than remaining buffer" << endl;
                offset = ((buffer + bufferSize) - _buffer);             // length of partial message in buffer
                cout << "offset " << offset << endl;
                std::memcpy(buffer, _buffer, offset);                        // copy length of partial message (offset) from end of _buffer to start of buffer
                                                                        // exit loop, next read destination will start from buffer + offset, and read offset fewer bytes
                break;
            }
            _buffer += 2;
            char type = *_buffer;
            cout << "type " << type << " len " << msglen << endl;
            //printchar(_buffer, msglen);
            _buffer += (msglen);
            totalbytes += (msglen + 2);
//            cout << "buffer size: " << bufferSize << ", buffer needed: " << _buffer - buffer << endl;
        }
        cout << "total bytes read " << totalbytes << endl;
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

