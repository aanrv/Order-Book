#ifndef ORDER_BOOK_ITCH_READER_HPP
#define ORDER_BOOK_ITCH_READER_HPP

#include "include/itch_common.hpp"
#include <cstdint>
#include <cstddef>

namespace ITCH {

template<typename MessageType> constexpr uint16_t MessageLength                 = -1;
template <> constexpr uint16_t MessageLength<SystemEventMessage>                = 12;
template <> constexpr uint16_t MessageLength<StockDirectoryMessage>             = 39;
template <> constexpr uint16_t MessageLength<StockTradingActionMessage>         = 25;
template <> constexpr uint16_t MessageLength<RegSHORestrictionMessage>          = 20;
template <> constexpr uint16_t MessageLength<MarketParticipantPositionMessage>  = 26;
template <> constexpr uint16_t MessageLength<MWCBDeclineLevelMessage>           = 35;
template <> constexpr uint16_t MessageLength<MWCBStatusMessage>                 = 12;
template <> constexpr uint16_t MessageLength<IPOQuotingPeriodUpdateMessage>     = 28;
template <> constexpr uint16_t MessageLength<LULDAuctionCollarMessage>          = 35;
template <> constexpr uint16_t MessageLength<OperationalHaltMessage>            = 21;
template <> constexpr uint16_t MessageLength<AddOrderMessage>                   = 36;
template <> constexpr uint16_t MessageLength<AddOrderMPIDAttributionMessage>    = 40;
template <> constexpr uint16_t MessageLength<OrderExecutedMessage>              = 31;
template <> constexpr uint16_t MessageLength<OrderExecutedWithPriceMessage>     = 36;
template <> constexpr uint16_t MessageLength<OrderCancelMessage>                = 23;
template <> constexpr uint16_t MessageLength<OrderDeleteMessage>                = 19;
template <> constexpr uint16_t MessageLength<OrderReplaceMessage>               = 35;
template <> constexpr uint16_t MessageLength<TradeMessage>                      = 44;
template <> constexpr uint16_t MessageLength<CrossTradeMessage>                 = 40;
template <> constexpr uint16_t MessageLength<BrokenTradeMessage>                = 19;
template <> constexpr uint16_t MessageLength<NOIIMessage>                       = 50;
template <> constexpr uint16_t MessageLength<RetailInterestMessage>             = 20;
template <> constexpr uint16_t MessageLength<DirectListingWithCapitalRaisePriceDiscoveryMessage> = 48;

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

private:
    int const       fdItch;
    size_t const    bufferSize;
    char * const    buffer;
    char *          _buffer;
    size_t          validBytes;
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
