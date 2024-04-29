#include "itch_reader.hpp"
#include "itch_common.hpp"
#include <cstring>                  // memcpy
#include <fcntl.h>                  // open
#include <unistd.h>                 // read
#include <endian.h>                 // be16toh
#include <stdexcept>

#define ASSERT  false
#if ASSERT
#include <cassert>
#endif

static constexpr size_t MESSAGE_HEADER_LENGTH = 2;
static constexpr size_t DEFAULT_BUFFER_SIZE   = 2048;

ITCH::Reader::Reader(char const * _filename) : Reader(_filename, DEFAULT_BUFFER_SIZE) {
}

ITCH::Reader::Reader(char const * _filename, size_t _bufferSize)
    : fdItch(open(_filename, O_RDONLY)),
    bufferSize(_bufferSize),
    buffer(new char[_bufferSize]),
    _buffer(buffer),
    validBytes(0),
    totalBytesRead(0) {
#if ASSERT
    assert(bufferSize > MESSAGE_HEADER_LENGTH + maxITCHMessageSize);
#endif
    if (fdItch == -1) { delete[] buffer; throw std::invalid_argument(std::string("Failed to open file: ") + _filename); }
    if (read(fdItch, buffer, bufferSize) <= 0) { delete[] buffer; throw std::invalid_argument(std::string("Failed to read from file: ") + _filename); }
}

ITCH::Reader::~Reader() {
    close(fdItch);
    delete[] buffer;
}

char const * ITCH::Reader::nextMessage() {
    // if entire buffer processed, perform read and reset _buffer
    if (_buffer == (buffer + bufferSize)) {
        ssize_t readBytes = read(fdItch, buffer, bufferSize);
        if (readBytes <= 0) return nullptr;
        validBytes = readBytes;
        _buffer = buffer;
    }

    // message header is 2 bytes
    // if attempting to read header will go out of bounds
    // copy first byte of header to beginning of buffer
    // and fill in the remaining buffer bytes with a new read
    if ((_buffer + MESSAGE_HEADER_LENGTH) > (buffer + bufferSize)) {
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
    if ((_buffer + MESSAGE_HEADER_LENGTH + messageLength) > (buffer + bufferSize)) {
        size_t offset = (buffer + bufferSize - _buffer);    // length of partial message remaining in buffer
        std::memcpy(buffer, _buffer, offset);               // copy length of partial message (offset) from end of _buffer to start of buffer
        ssize_t readBytes = read(fdItch, buffer + offset, bufferSize - offset);
        if (readBytes <= 0) {
            if (readBytes == 0) return nullptr;
            if (readBytes == -1) { delete[] buffer; throw std::runtime_error("Failed to read from file"); }
        }
        validBytes = readBytes + offset;
        _buffer = buffer;
    }

#if ASSERT
    switch (_buffer[messageTypeIndex]) {
        case SystemEventMessageType:
            assert(messageLength == MessageLength<SystemEventMessageType>);
            break;
        case StockDirectoryMessageType:
            assert(messageLength == MessageLength<StockDirectoryMessageType>);
            break;
        case StockTradingActionMessageType:
            assert(messageLength == MessageLength<StockTradingActionMessageType>);
            break;
        case RegSHORestrictionMessageType:
            assert(messageLength == MessageLength<RegSHORestrictionMessageType>);
            break;
        case MarketParticipantPositionMessageType:
            assert(messageLength == MessageLength<MarketParticipantPositionMessageType>);
            break;
        case MWCBDeclineLevelMessageType:
            assert(messageLength == MessageLength<MWCBDeclineLevelMessageType>);
            break;
        case MWCBStatusMessageType:
            assert(messageLength == MessageLength<MWCBStatusMessageType>);
            break;
        case IPOQuotingPeriodUpdateMessageType:
            assert(messageLength == MessageLength<IPOQuotingPeriodUpdateMessageType>);
            break;
        case LULDAuctionCollarMessageType:
            assert(messageLength == MessageLength<LULDAuctionCollarMessageType>);
            break;
        case OperationalHaltMessageType:
            assert(messageLength == MessageLength<OperationalHaltMessageType>);
            break;
        case AddOrderMessageType:
            assert(messageLength == MessageLength<AddOrderMessageType>);
            break;
        case AddOrderMPIDAttributionMessageType:
            assert(messageLength == MessageLength<AddOrderMPIDAttributionMessageType>);
            break;
        case OrderExecutedMessageType:
            assert(messageLength == MessageLength<OrderExecutedMessageType>);
            break;
        case OrderExecutedWithPriceMessageType:
            assert(messageLength == MessageLength<OrderExecutedWithPriceMessageType>);
            break;
        case OrderCancelMessageType:
            assert(messageLength == MessageLength<OrderCancelMessageType>);
            break;
        case OrderDeleteMessageType:
            assert(messageLength == MessageLength<OrderDeleteMessageType>);
            break;
        case OrderReplaceMessageType:
            assert(messageLength == MessageLength<OrderReplaceMessageType>);
            break;
        case TradeMessageType:
            assert(messageLength == MessageLength<TradeMessageType>);
            break;
        case CrossTradeMessageType:
            assert(messageLength == MessageLength<CrossTradeMessageType>);
            break;
        case BrokenTradeMessageType:
            assert(messageLength == MessageLength<BrokenTradeMessageType>);
            break;
        case NOIIMessageType:
            assert(messageLength == MessageLength<NOIIMessageType>);
            break;
        case RetailInterestMessageType:
            assert(messageLength == MessageLength<RetailInterestMessageType>);
            break;
        case DirectListingWithCapitalRaisePriceDiscoveryMessageType:
            assert(messageLength == MessageLength<DirectListingWithCapitalRaisePriceDiscoveryMessageType>);
            break;
        default:
            assert(false);
            break;
    };
#endif

    char const *out = _buffer;
    _buffer += (MESSAGE_HEADER_LENGTH + messageLength);

    totalBytesRead += (MESSAGE_HEADER_LENGTH + messageLength);

#if ASSERT
    assert(_buffer <= (buffer + bufferSize));
#endif

    return out;
}

long long ITCH::Reader::getTotalBytesRead() const {
    return totalBytesRead;
}

ITCH::AddOrderMessage ITCH::Parser::createAddOrderMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
    return ITCH::AddOrderMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::AddOrderMPIDAttributionMessage ITCH::Parser::createAddOrderMPIDAttributionMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
    return ITCH::AddOrderMPIDAttributionMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::OrderExecutedMessage ITCH::Parser::createOrderExecutedMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t executedShares         = be32toh(*(uint32_t *)(data + 19));
    return ITCH::OrderExecutedMessage{messageType, stockLocate, timestamp, orderReferenceNumber, executedShares};
}
ITCH::OrderExecutedWithPriceMessage ITCH::Parser::createOrderExecutedWithPriceMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t executedShares         = be32toh(*(uint32_t *)(data + 19));
    uint32_t executionPrice         = be32toh(*(uint32_t *)(data + 32));
    return ITCH::OrderExecutedWithPriceMessage{messageType, stockLocate, timestamp, orderReferenceNumber, executedShares, executionPrice};
}
ITCH::OrderCancelMessage ITCH::Parser::createOrderCancelMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint32_t cancelledShares        = be32toh(*(uint32_t *)(data + 19));
    return ITCH::OrderCancelMessage{messageType, stockLocate, timestamp, orderReferenceNumber, cancelledShares};
}
ITCH::OrderDeleteMessage ITCH::Parser::createOrderDeleteMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    return ITCH::OrderDeleteMessage{messageType, stockLocate, timestamp, orderReferenceNumber};
}
ITCH::OrderReplaceMessage ITCH::Parser::createOrderReplaceMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                        = *data;
    uint16_t stockLocate                    = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp                      = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t originalOrderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint64_t newOrderReferenceNumber        = be64toh(*(uint64_t *)(data + 19));
    uint32_t shares                         = be32toh(*(uint32_t *)(data + 27));
    uint32_t price                          = be32toh(*(uint32_t *)(data + 31));
    return ITCH::OrderReplaceMessage{messageType, stockLocate, timestamp, originalOrderReferenceNumber, newOrderReferenceNumber, shares, price};
}
ITCH::TradeMessage ITCH::Parser::createTradeMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    char side                       = *(data + 19);
    uint32_t shares                 = be32toh(*(uint32_t *)(data + 20));
    uint32_t price                  = be32toh(*(uint32_t *)(data + 32));
    return ITCH::TradeMessage{messageType, stockLocate, timestamp, orderReferenceNumber, side, shares, price};
}
ITCH::CrossTradeMessage ITCH::Parser::createCrossTradeMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType                = *data;
    uint16_t stockLocate            = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp              = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t orderReferenceNumber   = be64toh(*(uint64_t *)(data + 11));
    uint64_t shares                 = be64toh(*(uint64_t *)(data + 11));
    uint32_t crossPrice             = be32toh(*(uint32_t *)(data + 27));
    return ITCH::CrossTradeMessage{messageType, stockLocate, timestamp, orderReferenceNumber, shares, crossPrice};
}
ITCH::BrokenTradeMessage ITCH::Parser::createBrokenTradeMessage(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    char messageType        = *data;
    uint16_t stockLocate    = be16toh(*(uint16_t *)(data + 1));
    uint64_t timestamp      = be64toh(*(uint64_t *)(data + 5)) >> 16;
    uint64_t matchNumber    = be64toh(*(uint64_t *)(data + 11));
    return ITCH::BrokenTradeMessage{messageType, stockLocate, timestamp, matchNumber};
}

ITCH::MessageType_t ITCH::Parser::getDataMessageType(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    return *data;
}

ITCH::Timestamp_t ITCH::Parser::getDataTimestamp(char const * data) {
    data += MESSAGE_HEADER_LENGTH;
    return be64toh(*(uint64_t *)(data + 5)) >> 16;
}

