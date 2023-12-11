#include "include/itch_reader.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh

#include <chrono>

#include <iostream>                 // todo remove

#define LOG     false
#define ASSERT  true

#if ASSERT
#include <cassert>
#endif

static constexpr size_t defaultBufferSize   = 2048;

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
    if (messageLength == 0) { std::cout << "session end" << '\n'; return nullptr; }

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
//    std::cout << "msgtype " << *(out + 2) << " len " << messageLength << " totalbytesread " << totalBytesRead << " buffer remaining " << (buffer + bufferSize - _buffer) << '\n';
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

ITCH::AddOrderMessage ITCH::Parser::createAddOrderMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
#if LOG
    std::cout <<
        "add order " << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " side " << side << 
        " shares " << shares << 
        " price " << price << 
        '\n';
#endif
    return ITCH::AddOrderMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::AddOrderMPIDAttributionMessage ITCH::Parser::createAddOrderMPIDAttributionMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
#if LOG
    std::cout <<
        "add order mpid" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " side " << side << 
        " shares " << shares << 
        " price " << price << 
        '\n';
#endif
    return ITCH::AddOrderMPIDAttributionMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::OrderExecutedMessage ITCH::Parser::createOrderExecutedMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t executedShares         = be32toh(*(uint32_t *)(data + 19));
#if LOG
    std::cout <<
        "order exec msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " exec shares " << executedShares << 
        '\n';
#endif
    return ITCH::OrderExecutedMessage{messageType, stockLocate, timestamp, orderReferenceNumber, executedShares};
}
ITCH::OrderExecutedWithPriceMessage ITCH::Parser::createOrderExecutedWithPriceMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t executedShares         = be32toh(*(uint32_t *)(data + 19));
    uint32_t executionPrice         = be32toh(*(uint32_t *)(data + 32));
#if LOG
    std::cout <<
        "order exec price msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " exec shares " << executedShares <<
        " exec price " << executionPrice <<
        '\n';
#endif
    return ITCH::OrderExecutedWithPriceMessage{messageType, stockLocate, timestamp, orderReferenceNumber, executedShares, executionPrice};
}
ITCH::OrderCancelMessage ITCH::Parser::createOrderCancelMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t cancelledShares        = be32toh(*(uint32_t *)(data + 19));
#if LOG
    std::cout <<
        "order cancel msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " cancelled shares " << cancelledShares <<
        '\n';
#endif
    return ITCH::OrderCancelMessage{messageType, stockLocate, timestamp, orderReferenceNumber, cancelledShares};
}
ITCH::OrderDeleteMessage ITCH::Parser::createOrderDeleteMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
#if LOG
    std::cout <<
        "order delete msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        '\n';
#endif
    return ITCH::OrderDeleteMessage{messageType, stockLocate, timestamp, orderReferenceNumber};
}
ITCH::OrderReplaceMessage ITCH::Parser::createOrderReplaceMessage(char const * data) {
    char messageType                        = *data;
    uint16_t stockLocate                    = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp                      = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t originalOrderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint64_t newOrderReferenceNumber        = be64toh(*(uint64_t *)(data + 19));
    uint32_t shares                         = be32toh(*(uint32_t *)(data + 27));
    uint32_t price                          = be32toh(*(uint32_t *)(data + 31));
#if LOG
    std::cout <<
        "order replace msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " orig order " << originalOrderReferenceNumber <<
        " new order " << newOrderReferenceNumber <<
        " shares " << shares << 
        " price " << price << 
        '\n';
#endif
    return ITCH::OrderReplaceMessage{messageType, stockLocate, timestamp, originalOrderReferenceNumber, newOrderReferenceNumber, shares, price};
}
ITCH::TradeMessage ITCH::Parser::createTradeMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
#if LOG
    std::cout <<
        "trade msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " side " << side << 
        " shares " << shares << 
        " price " << price << 
        '\n';
#endif
    return ITCH::TradeMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::CrossTradeMessage ITCH::Parser::createCrossTradeMessage(char const * data) {
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint64_t shares                 = be32toh(*(uint64_t *)(data + 11));
    uint32_t crossPrice             = be32toh(*(uint32_t *)(data + 27));
#if LOG
    std::cout <<
        "cross trade msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " order reference number " << orderReferenceNumber << 
        " shares " << shares << 
        " cross price " << crossPrice << 
        '\n';
#endif
    return ITCH::CrossTradeMessage{messageType, stockLocate, timestamp, orderReferenceNumber, shares, crossPrice};
}
ITCH::BrokenTradeMessage ITCH::Parser::createBrokenTradeMessage(char const * data) {
    char messageType        = *data;
    uint16_t stockLocate    = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp      = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t matchNumber    = be64toh(*(uint64_t *)(data + 11));
#if LOG
    std::cout <<
        "broken trade msg" << 
        " type " << messageType << 
        " stock locate " << stockLocate << 
        " timestamp " << timestamp << 
        " match number " << matchNumber << 
        '\n';
#endif
    return ITCH::BrokenTradeMessage{messageType, stockLocate, timestamp, matchNumber};
}

