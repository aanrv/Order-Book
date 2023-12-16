#ifndef ORDER_BOOK_ITCH_COMMON
#define ORDER_BOOK_ITCH_COMMON

#include <cstddef>
#include <cstdint>
#include <ostream>

namespace ITCH {

constexpr size_t maxITCHMessageSize     = 50;
constexpr size_t messageHeaderLength    = 2;
constexpr size_t messageTypeIndex       = 2;

namespace Side {
    constexpr char BUY  = 'B';
    constexpr char SELL = 'S';
}

constexpr char SystemEventMessageType               = 'S';
constexpr char StockDirectoryMessageType            = 'R';
constexpr char StockTradingActionMessageType        = 'H';
constexpr char RegSHORestrictionMessageType         = 'Y';
constexpr char MarketParticipantPositionMessageType = 'L';
constexpr char MWCBDeclineLevelMessageType          = 'V';
constexpr char MWCBStatusMessageType                = 'W';
constexpr char IPOQuotingPeriodUpdateMessageType    = 'K';
constexpr char LULDAuctionCollarMessageType         = 'J';
constexpr char OperationalHaltMessageType           = 'h';
constexpr char AddOrderMessageType                  = 'A';
constexpr char AddOrderMPIDAttributionMessageType   = 'F';
constexpr char OrderExecutedMessageType             = 'E';
constexpr char OrderExecutedWithPriceMessageType    = 'C';
constexpr char OrderCancelMessageType               = 'X';
constexpr char OrderDeleteMessageType               = 'D';
constexpr char OrderReplaceMessageType              = 'U';
constexpr char TradeMessageType                     = 'P';
constexpr char CrossTradeMessageType                = 'Q';
constexpr char BrokenTradeMessageType               = 'B';
constexpr char NOIIMessageType                      = 'I';
constexpr char RetailInterestMessageType            = 'N';
constexpr char DirectListingWithCapitalRaisePriceDiscoveryMessageType = 'O';

template<char MessageType> constexpr char * TypeTag = MessageType + "  ";
template <> constexpr char const * TypeTag<SystemEventMessageType>               = "SYS";
template <> constexpr char const * TypeTag<StockDirectoryMessageType>            = "DIR";
template <> constexpr char const * TypeTag<StockTradingActionMessageType>        = "ACT";
template <> constexpr char const * TypeTag<RegSHORestrictionMessageType>         = "REG";
template <> constexpr char const * TypeTag<MarketParticipantPositionMessageType> = "POS";
template <> constexpr char const * TypeTag<MWCBDeclineLevelMessageType>          = "DCL";
template <> constexpr char const * TypeTag<MWCBStatusMessageType>                = "STS";
template <> constexpr char const * TypeTag<IPOQuotingPeriodUpdateMessageType>    = "IPO";
template <> constexpr char const * TypeTag<LULDAuctionCollarMessageType>         = "COL";
template <> constexpr char const * TypeTag<OperationalHaltMessageType>           = "HLT";
template <> constexpr char const * TypeTag<AddOrderMessageType>                  = "ADD";
template <> constexpr char const * TypeTag<AddOrderMPIDAttributionMessageType>   = "ADM";
template <> constexpr char const * TypeTag<OrderExecutedMessageType>             = "EXC";
template <> constexpr char const * TypeTag<OrderExecutedWithPriceMessageType>    = "EXP";
template <> constexpr char const * TypeTag<OrderCancelMessageType>               = "CNL";
template <> constexpr char const * TypeTag<OrderDeleteMessageType>               = "DEL";
template <> constexpr char const * TypeTag<OrderReplaceMessageType>              = "RPL";
template <> constexpr char const * TypeTag<TradeMessageType>                     = "TRD";
template <> constexpr char const * TypeTag<CrossTradeMessageType>                = "CRX";
template <> constexpr char const * TypeTag<BrokenTradeMessageType>               = "BRK";
template <> constexpr char const * TypeTag<NOIIMessageType>                      = "NOI";
template <> constexpr char const * TypeTag<RetailInterestMessageType>            = "RTL";
template <> constexpr char const * TypeTag<DirectListingWithCapitalRaisePriceDiscoveryMessageType> = "DSC";

template<char MessageType> constexpr uint16_t MessageLength = -1;
template <> constexpr uint16_t MessageLength<SystemEventMessageType>                = 12;
template <> constexpr uint16_t MessageLength<StockDirectoryMessageType>             = 39;
template <> constexpr uint16_t MessageLength<StockTradingActionMessageType>         = 25;
template <> constexpr uint16_t MessageLength<RegSHORestrictionMessageType>          = 20;
template <> constexpr uint16_t MessageLength<MarketParticipantPositionMessageType>  = 26;
template <> constexpr uint16_t MessageLength<MWCBDeclineLevelMessageType>           = 35;
template <> constexpr uint16_t MessageLength<MWCBStatusMessageType>                 = 12;
template <> constexpr uint16_t MessageLength<IPOQuotingPeriodUpdateMessageType>     = 28;
template <> constexpr uint16_t MessageLength<LULDAuctionCollarMessageType>          = 35;
template <> constexpr uint16_t MessageLength<OperationalHaltMessageType>            = 21;
template <> constexpr uint16_t MessageLength<AddOrderMessageType>                   = 36;
template <> constexpr uint16_t MessageLength<AddOrderMPIDAttributionMessageType>    = 40;
template <> constexpr uint16_t MessageLength<OrderExecutedMessageType>              = 31;
template <> constexpr uint16_t MessageLength<OrderExecutedWithPriceMessageType>     = 36;
template <> constexpr uint16_t MessageLength<OrderCancelMessageType>                = 23;
template <> constexpr uint16_t MessageLength<OrderDeleteMessageType>                = 19;
template <> constexpr uint16_t MessageLength<OrderReplaceMessageType>               = 35;
template <> constexpr uint16_t MessageLength<TradeMessageType>                      = 44;
template <> constexpr uint16_t MessageLength<CrossTradeMessageType>                 = 40;
template <> constexpr uint16_t MessageLength<BrokenTradeMessageType>                = 19;
template <> constexpr uint16_t MessageLength<NOIIMessageType>                       = 50;
template <> constexpr uint16_t MessageLength<RetailInterestMessageType>             = 20;
template <> constexpr uint16_t MessageLength<DirectListingWithCapitalRaisePriceDiscoveryMessageType> = 48;

/*
 * Unable to use this as types may differ accross message types
 * e.g. shares are 64-bit in cross trade message
 *
using StockLocateType           = uint16_t;
using TimestampType             = uint64_t;
using OrderReferenceNumberType  = uint64_t;
using BuySellIndicatorType      = char;
using SharesType                = uint32_t;
using PriceType                 = uint32_t;
*/

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

} // namespace ITCH

template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::AddOrderMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::AddOrderMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " side " << m.buySellIndicator <<
        " shares " << m.shares <<
        " price " << m.price;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::AddOrderMPIDAttributionMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::AddOrderMPIDAttributionMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " side " << m.buySellIndicator <<
        " shares " << m.shares <<
        " price " << m.price;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::OrderExecutedMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::OrderExecutedMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " exec shares " << m.executedShares;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::OrderExecutedWithPriceMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::OrderExecutedWithPriceMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " exec shares " << m.executedShares <<
        " exec price " << m.executionPrice;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::OrderCancelMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::OrderCancelMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " cancelled shares " << m.cancelledShares;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::OrderDeleteMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::OrderDeleteMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::OrderReplaceMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::OrderReplaceMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " orig order " << m.originalOrderReferenceNumber <<
        " new order " << m.newOrderReferenceNumber <<
        " shares " << m.shares <<
        " price " << m.price;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::TradeMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::TradeMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " side " << m.buySellIndicator <<
        " shares " << m.shares <<
        " price " << m.price;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::CrossTradeMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::CrossTradeMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp <<
        " order reference number " << m.orderReferenceNumber <<
        " shares " << m.shares <<
        " cross price " << m.crossPrice;
    return os;
}
template <typename OStream>
inline OStream& operator<<(OStream& os, ITCH::BrokenTradeMessage const & m) {
    os <<
        ITCH::TypeTag<ITCH::BrokenTradeMessageType> <<
        " type " << m.messageType <<
        " stock locate " << m.stockLocate <<
        " timestamp " << m.timestamp;
    return os;
}

#endif // ORDER_BOOK_ITCH_COMMON

