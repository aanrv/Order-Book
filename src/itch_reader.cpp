#include "include/itch_reader.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh

#include <chrono>

#include <iostream>                 // todo remove

#define LOG     true
#define ASSERT  true

#if ASSERT
#include <cassert>
#endif

static constexpr size_t defaultBufferSize   = 2048;
static constexpr size_t messageHeaderLength = 2;
static constexpr size_t maxITCHMessageSize  = 50;

ITCH::Reader::Reader(char const * _filename) : Reader(_filename, defaultBufferSize) {
}

ITCH::Reader::Reader(char const * _filename, size_t _bufferSize)
    : fdItch(open(_filename, O_RDONLY)),
    bufferSize(_bufferSize),
    buffer(new char[_bufferSize]),
    _buffer(buffer) {
#if ASSERT
    assert(bufferSize > messageHeaderLength + maxITCHMessageSize);
#endif
    if (fdItch == -1) { delete buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
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
        ssize_t readBytes = read(fdItch, buffer + remainingLength, bufferSize - remainingLength);
        if (readBytes <= 0) return nullptr;
        validBytes = readBytes + remainingLength;
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
        validBytes = readBytes + offset;
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
        ssize_t readBytes = read(fdItch, buffer, bufferSize);
        if (readBytes <= 0) return nullptr;
        validBytes = readBytes;
        _buffer = buffer;
    }

    return out;
}

