#include "include/itch_reader.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh

#include <chrono>

#include <iostream>                 // todo remove

#define LOG     true
#define BENCH   true
#define ASSERT  true

#if ASSERT
#include <cassert>
#endif

ITCH::Reader::Reader(char const * _filename) : Reader(_filename, defaultBufferSize) {
}

ITCH::Reader::Reader(char const * _filename, size_t _bufferSize) : bufferSize(_bufferSize), buffer(new char[_bufferSize]), _buffer(buffer) {
#if ASSERT
    assert(bufferSize > messageHeaderLength + maxITCHMessageSize);
#endif
    if ((fdItch = open(_filename, O_RDONLY)) == -1) { delete buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
    if (read(fdItch, buffer, bufferSize) <= 0) { delete buffer; throw std::invalid_argument(std::string("Failed to read from file: ") + _filename); }
}

ITCH::Reader::~Reader() {
    close(fdItch);
    delete buffer;
}

char const * ITCH::Reader::nextMessage() {
#if LOG
    static long long totalBytesRead;
#endif
#if ASSERT
    assert(_buffer < (buffer + bufferSize));
#endif

    // message header is 2 bytes
    // if attempting to read header will go out of bounds
    // copy first byte of header to beginning of buffer
    // and fill in the remaining buffer bytes with a new read
    if ((_buffer + messageHeaderLength) > (buffer + bufferSize)) {
        buffer[0] = *_buffer;
        constexpr size_t remainingLength = 1;
        if (read(fdItch, buffer + remainingLength, bufferSize - remainingLength) <= 0) return nullptr;
        _buffer = buffer;
    }

    // message header is 2 byte big endian number containing message length
    uint16_t messageLength = be16toh(*(uint16_t *)_buffer);
    // 0 message size indicates end of session
    if (messageLength == 0) return nullptr;

    // handle case if current message is partial
    // i.e. message extends past last byte in buffer
    // copy current message to beginning of this buffer
    // fill remaining buffer with read
    // this will overflow if bufferSize > 2+messageLength, but this is already checked for in constructor
    // update maxITCHMessageSize according to NASDAQ spec
    if ((_buffer + messageHeaderLength + messageLength) > (buffer + bufferSize)) {
        size_t offset = (buffer + bufferSize - _buffer);    // length of partial message remaining in buffer
        std::memcpy(buffer, _buffer, offset);               // copy length of partial message (offset) from end of _buffer to start of buffer
        ssize_t readBytes = read(fdItch, buffer + offset, bufferSize - offset);
        if (readBytes <= 0) {
            if (readBytes == 0) return nullptr;
            if (readBytes == -1) { delete buffer; throw std::ios_base::failure("Failed to read from file"); }
        }
        _buffer = buffer;
    }

    char const *out = _buffer;
    _buffer += (messageHeaderLength + messageLength);

#if LOG
    totalBytesRead += (messageHeaderLength + messageLength);
    std::cout << "msgtype " << *(out + 2) << " len " << messageLength << " totalbytesread " << totalBytesRead << " buffer remaining " << (buffer + bufferSize - _buffer) << std::endl;
#endif

#if ASSERT
    assert(_buffer <= (buffer + bufferSize));
#endif

    // if entire buffer processed, perform read and reset _buffer
    // do this at end so _buffer pointer is left in a valid state after function call
    if (_buffer == (buffer + bufferSize)) {
        if (read(fdItch, buffer, bufferSize) <= 0) return nullptr;
        _buffer = buffer;
    }

    return out;
}
/*
void ITCH::Reader::nextMessage() {
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
        std::cerr << "Stopping ITCH::Reader::process()" << std::endl;
    }
}
*/
