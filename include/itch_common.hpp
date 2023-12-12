#ifndef ORDER_BOOK_ITCH_COMMON
#define ORDER_BOOK_ITCH_COMMON

#include <cstddef>

namespace ITCH {

constexpr size_t maxITCHMessageSize     = 50;
constexpr size_t messageHeaderLength    = 2;
constexpr size_t messageTypeIndex       = 2;

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
}

#endif // ORDER_BOOK_ITCH_COMMON
