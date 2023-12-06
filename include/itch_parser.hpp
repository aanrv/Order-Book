#ifndef ORDER_BOOK_ITCH_PARSER_HPP
#define ORDER_BOOK_ITCH_PARSER_HPP

#include <cstdint>
#include <cstddef>
#include <fstream>

namespace ITCH {

struct SystemEventMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     eventCode;
};

struct StockDirectoryMessage {
    uint8_t     messageType;
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
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     tradingState;
    uint8_t     reserved;
    uint8_t     reason[4];
};

struct RegSHORestrictionMessage {
    uint8_t     messageType;
    uint16_t    locateCode;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     RegSHOAction;
};

struct MarketParticipantPositionMessage {
    uint8_t     messageType;
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
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    level1;
    uint64_t    level2;
    uint64_t    level3;
};

struct MWCBStatusMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     breachedLevel;
};

struct IPOQuotingPeriodUpdateMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint32_t    IPOQuotationReleaseTime;
    uint8_t     IPOQuotationReleaseQualifier;
    uint32_t    IPOPrice;
};

struct LULDAuctionCollarMessage {
    uint8_t     messageType;
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
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     marketCode;
    uint8_t     operationalHaltAction;
};

struct AddOrderMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint8_t     buySellIndicator;
    uint32_t    shares;
    uint8_t     stock[8];
    uint32_t    price;
};

struct AddOrderMPIDAttributionMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint8_t     buySellIndicator;
    uint32_t    shares;
    uint8_t     stock[8];
    uint32_t    price;
    uint8_t     attribution[4];
};

struct OrderExecutedMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    executedShares;
    uint64_t    matchNumber;
};

struct OrderExecutedWithPriceMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    executedShares;
    uint64_t    matchNumber;
    uint8_t     printable;
    uint32_t    executionPrice;
};

struct OrderCancelMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint32_t    cancelledShares;
};

struct OrderDeleteMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
};

struct OrderReplaceMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    originalOrderReferenceNumber;
    uint64_t    newOrderReferenceNumber;
    uint32_t    shares;
    uint32_t    price;
};

struct TradeMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    orderReferenceNumber;
    uint8_t     buySellIndicator;
    uint32_t    shares;
    uint8_t     stock[8];
    uint32_t    price;
    uint64_t    matchNumber;
};

struct CrossTradeMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    shares;
    uint8_t     stock[8];
    uint32_t    crossPrice;
    uint64_t    matchNumber;
    uint8_t     crossType;
};

struct BrokenTradeMessage {
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint64_t    matchNumber;
};

struct NOIIMessage {
    uint8_t     messageType;
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
    uint8_t     messageType;
    uint16_t    stockLocate;
    uint16_t    trackingNumber;
    uint64_t    timestamp;
    uint8_t     stock[8];
    uint8_t     InterestFlag;
};

struct DirectListingWithCapitalRaisePriceDiscoveryMessage {
    uint8_t     messageType;
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

// Parses Nasdaq BinaryFILE for ITCH Data
class Parser {
public:
    Parser()                                                    = delete;   // must provide filename
    Parser(char const * _filename);
    Parser(char const * _filename, size_t _bufferSize);

    Parser(const Parser& p)                                     = delete;
    Parser& operator=(const Parser& p)                          = delete;

    ~Parser();

    void process();

private:
    static constexpr size_t defaultBufferSize = 2048;

    int     fdItch;
    size_t  bufferSize;
    char*   buffer;
};

} // namespace ITCH

#endif // ORDER_BOOK_ITCH_PARSER_HPP
