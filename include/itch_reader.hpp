#ifndef ORDER_BOOK_ITCH_READER_HPP
#define ORDER_BOOK_ITCH_READER_HPP

#include "itch_common.hpp"
#include <cstdint>
#include <cstddef>

namespace ITCH {

// Reads Nasdaq BinaryFILE and retrieves message data segments
class Reader {
public:
    Reader()                                            = delete;   // must provide filename

    Reader(char const * _filename);
    Reader(char const * _filename, size_t _bufferSize);

    Reader(const Reader& p)                             = delete;
    Reader& operator=(const Reader& p)                  = delete;

    ~Reader();

    char const * nextMessage();
    long long getTotalBytesRead() const;

private:
    int const       fdItch;
    size_t const    bufferSize;
    char * const    buffer;
    char *          _buffer;
    size_t          validBytes;
    long long       totalBytesRead;
};

namespace Parser {
    SystemEventMessage                  createSystemEventMessage(char const * data);
    StockDirectoryMessage               createStockDirectoryMessage(char const *);
    StockTradingActionMessage           createStockTradingActionMessage(char const *);
    RegSHORestrictionMessage            createRegSHORestrictionMessage(char const *);
    MarketParticipantPositionMessage    createMarketParticipantPositionMessage(char const *);
    MWCBDeclineLevelMessage             createMWCBDeclineLevelMessage(char const *);
    MWCBStatusMessage                   createMWCBStatusMessage(char const *);
    IPOQuotingPeriodUpdateMessage       createIPOQuotingPeriodUpdateMessage(char const *);
    LULDAuctionCollarMessage            createLULDAuctionCollarMessage(char const *);
    OperationalHaltMessage              createOperationalHaltMessage(char const *);
    AddOrderMessage                     createAddOrderMessage(char const *);
    AddOrderMPIDAttributionMessage      createAddOrderMPIDAttributionMessage(char const *);
    OrderExecutedMessage                createOrderExecutedMessage(char const *);
    OrderExecutedWithPriceMessage       createOrderExecutedWithPriceMessage(char const *);
    OrderCancelMessage                  createOrderCancelMessage(char const *);
    OrderDeleteMessage                  createOrderDeleteMessage(char const *);
    OrderReplaceMessage                 createOrderReplaceMessage(char const *);
    TradeMessage                        createTradeMessage(char const *);
    CrossTradeMessage                   createCrossTradeMessage(char const *);
    BrokenTradeMessage                  createBrokenTradeMessage(char const *);
    NOIIMessage                         createNOIIMessage(char const *);
    RetailInterestMessage               createRetailInterestMessage(char const *);
    DirectListingWithCapitalRaisePriceDiscoveryMessage createDirectListingWithCapitalRaisePriceDiscoveryMessage(char const *);
};

} // namespace ITCH

#endif // ORDER_BOOK_ITCH_READER_HPP
