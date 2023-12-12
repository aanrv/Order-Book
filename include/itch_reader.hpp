#ifndef ORDER_BOOK_ITCH_READER_HPP
#define ORDER_BOOK_ITCH_READER_HPP

#include "include/itch_common.hpp"
#include <cstdint>
#include <cstddef>

namespace ITCH {

struct SystemEventMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     eventCode;
};

struct StockDirectoryMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     marketCategory;
    uint8_t     financialStatusIndicator;
    uint32_t    roundLotSize;
    uint8_t     roundLotsOnly;
    uint8_t     issueClassification;
    uint8_t     issueSubType[2];
    uint8_t     authenticity;
    uint8_t     shortSaleThresholdIndicator;
    uint8_t     IPOFlag;
    uint8_t     LULDReferencePriceTier;
    uint8_t     ETPFlag;
    uint32_t    ETPLeverageFactor;
    uint8_t     inverseIndicator;
};

struct StockTradingActionMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     tradingState;
    uint8_t     reserved;
    uint8_t     reason[4];
};

struct RegSHORestrictionMessage {
    char        messageType;
    uint16_t    locateCode;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     RegSHOAction;
};

struct MarketParticipantPositionMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     MPID[4];
    uint8_t     stock[8];
    uint8_t     primaryMarketMaker;
    uint8_t     marketMakerMode;
    uint8_t     marketParticipantState;
};

struct MWCBDeclineLevelMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    level1;
    uint64_t    level2;
    uint64_t    level3;
};

struct MWCBStatusMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     breachedLevel;
};

struct IPOQuotingPeriodUpdateMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint32_t    IPOQuotationReleaseTime;
    uint8_t     IPOQuotationReleaseQualifier;
    uint32_t    IPOPrice;
};

struct LULDAuctionCollarMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint32_t    auctionCollarReferencePrice;
    uint32_t    upperAuctionCollarPrice;
    uint32_t    lowerAuctionCollarPrice;
    uint32_t    auctionCollarExtension;
};

struct OperationalHaltMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     marketCode;
    uint8_t     operationalHaltAction;
};

struct AddOrderMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    char        buySellIndicator;
    uint32_t    shares;
    uint32_t    price;
};

struct AddOrderMPIDAttributionMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    char        buySellIndicator;
    uint32_t    shares;
    uint32_t    price;
};

struct OrderExecutedMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    executedShares;
};

struct OrderExecutedWithPriceMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    executedShares;
    uint32_t    executionPrice;
};

struct OrderCancelMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    cancelledShares;
};

struct OrderDeleteMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
};

struct OrderReplaceMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    originalOrderReferenceNumber;
    uint64_t    newOrderReferenceNumber;
    uint32_t    shares;
    uint32_t    price;
};

struct TradeMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    char        buySellIndicator;
    uint32_t    shares;
    uint32_t    price;
};

struct CrossTradeMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint64_t    shares;
    uint32_t    crossPrice;
};

struct BrokenTradeMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
};

struct NOIIMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    pairedShares;
    uint64_t    imbalanceShares;
    uint8_t     imbalanceDirection;
    uint8_t     stock[8];
    uint32_t    farPrice;
    uint32_t    nearPrice;
    uint32_t    currentReferencePrice;
    uint8_t     crossType;
    uint8_t     priceVariationIndicator;
};

struct RetailInterestMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     InterestFlag;
};

struct DirectListingWithCapitalRaisePriceDiscoveryMessage {
    char        messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     openEligibilityStatus;
    uint32_t    minimumAllowablePrice;
    uint32_t    maximumAllowablePrice;
    uint32_t    nearExecutionPrice;
    uint64_t    nearExecutionTime;
    uint32_t    lowerPriceRangeCollar;
    uint32_t    upperPriceRangeCollar;
};

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
