#include "include/itch_reader.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh

#include <chrono>

#include <iostream>                 // todo remove

#define BENCH   true
#define ASSERT  true

#if ASSERT
#include <cassert>
#endif

ITCH::Parser::Parser(char const * _filename) : Parser(_filename, defaultBufferSize) {
}

ITCH::Parser::Parser(char const * _filename, size_t _bufferSize) : bufferSize(_bufferSize), buffer(new char[_bufferSize]) {
    if ((fdItch = open(_filename, O_RDONLY)) == -1) { delete buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
}

ITCH::Parser::~Parser() {
    close(fdItch);
    delete buffer;
}

void ITCH::Parser::parseITCHFile() {
#if BENCH
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;
    unsigned long long totalbytes = 0;
    unsigned long long numMessages = 0;
    auto t1 = high_resolution_clock::now();
#endif

    ssize_t bytesRead;
    size_t offset = 0;
    bool endOfSession = false;
    constexpr size_t messageHeaderLength = 2;
    // read file in chunks of bufferSize bytes (offset handles incomplete message from previous buffer)
    while (!endOfSession && (bytesRead = read(fdItch, buffer + offset, bufferSize - offset)) > 0) {
        size_t numValidBytes = bytesRead + offset;                              // number of bytes to process
                                                                                // handles case where bytesRead < attempted and possibly offset > 0 i.e. memcpy from previous loop
        offset = 0;
#if ASSERT
        assert(numValidBytes <= bufferSize);
#endif

        char* _buffer = buffer;
        // read buffer message by message
        while (_buffer < (buffer + numValidBytes)) {                            // while _buffer pointer has not gone out of bounds for buffer
            if ((_buffer + messageHeaderLength) > (buffer + numValidBytes)) {
                // handle case message size is partial
                // message sizes are 2 bytes but only 1 byte left in buffer
                offset = 1;
                buffer[0] = *_buffer;
                break;
            }
            uint16_t messageLength = be16toh(*(uint16_t *)_buffer);             // message header is a 2 byte big endian number containing message length
            if (messageLength == 0) {
                // check end of session
                endOfSession = true;
                break;
            }
            if ((_buffer + messageHeaderLength + messageLength) > (buffer + numValidBytes)) {
                // handle case current message is partial
                // message extends past last byte in buffer
                offset = ((buffer + numValidBytes) - _buffer);  // length of partial message currently in buffer
                std::memcpy(buffer, _buffer, offset);           // copy length of partial message (offset) from end of _buffer to start of buffer
                                                                // exit loop, next read destination will start from buffer + offset, and read offset fewer bytes
                break;
            }
            _buffer += messageHeaderLength;
            //char type = *_buffer;
            _buffer += (messageLength);
#if ASSERT
            assert(_buffer <= (buffer + bufferSize));
#endif

#if BENCH
            ++numMessages;
            totalbytes += (messageHeaderLength + messageLength);
#endif
        }
    }
#if BENCH
    auto t2 = high_resolution_clock::now();
    duration<double, std::nano> ns_double = t2 - t1;
    std::cout << "processed " << numMessages << " in " << ns_double.count() << " ns" << std::endl;
    std::cout << "total bytes read " << totalbytes << std::endl;
#endif
    if (bytesRead == -1) {
        std::cerr << "Failed to read bytes from start of file" << std::endl;
        std::cerr << "Stopping ITCH::Parser::process()" << std::endl;
    }
}

