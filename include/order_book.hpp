#ifndef ORDER_BOOK_ORDER_BOOK_HPP
#define ORDER_BOOK_ORDER_BOOK_HPP

#include "itch_common.hpp"
#include <map>
#include <list>
#include <cstdint>
#include <tuple>
#include <boost/pool/object_pool.hpp>
#include <sparsehash/dense_hash_map>
#define NDEBUG

struct Order {
    uint64_t    referenceNumber;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    char        side;
    uint32_t    shares;
    uint32_t    price;
    Order*      prev;
    Order*      next;
    Order(const std::tuple<uint64_t, uint16_t, uint64_t, char, uint32_t, uint32_t, Order*, Order*> &);
};

struct Level {
    uint32_t price;
    uint32_t limitVolume;
    // either both are nullptr or both are populated
    // i.e. if orders in Level == 1, both pointers are ==
    Order* first;
    Order* last;
    Level(uint32_t _price);
};

// price-time LOB
class OrderBook {
public:
    void handleSystemEventMessage(ITCH::SystemEventMessage const &);
    void handleStockDirectoryMessage(ITCH::StockDirectoryMessage const &);
    void handleStockTradingActionMessage(ITCH::StockTradingActionMessage const &);
    void handleRegSHORestrictionMessage(ITCH::RegSHORestrictionMessage const &);
    void handleMarketParticipantPositionMessage(ITCH::MarketParticipantPositionMessage const &);
    void handleMWCBDeclineLevelMessage(ITCH::MWCBDeclineLevelMessage const &);
    void handleMWCBStatusMessage(ITCH::MWCBStatusMessage const &);
    void handleIPOQuotingPeriodUpdateMessage(ITCH::IPOQuotingPeriodUpdateMessage const &);
    void handleLULDAuctionCollarMessage(ITCH::LULDAuctionCollarMessage const &);
    void handleOperationalHaltMessage(ITCH::OperationalHaltMessage const &);
    void handleAddOrderMessage(ITCH::AddOrderMessage const &);
    void handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const &);
    void handleOrderExecutedMessage(ITCH::OrderExecutedMessage const &);
    void handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const &);
    void handleOrderCancelMessage(ITCH::OrderCancelMessage const &);
    void handleOrderDeleteMessage(ITCH::OrderDeleteMessage const &);
    void handleOrderReplaceMessage(ITCH::OrderReplaceMessage const &);
    void handleTradeMessage(ITCH::TradeMessage const &);
    void handleCrossTradeMessage(ITCH::CrossTradeMessage const &);
    void handleBrokenTradeMessage(ITCH::BrokenTradeMessage const &);
    void handleNOIIMessage(ITCH::NOIIMessage const &);
    void handleRetailInterestMessage(ITCH::RetailInterestMessage const &);
    void handleDirectListingWithCapitalRaisePriceDiscoveryMessage(ITCH::DirectListingWithCapitalRaisePriceDiscoveryMessage const &);

    uint32_t getLimitVolume(char side, uint32_t limitprice) const; // keep track Level obj
    uint32_t getBestBid() const;   // largest map
    uint32_t getBestAsk() const;   // lowest map
    uint32_t getLastExecutedPrice() const;  // keep track in book
    uint32_t getLastExecutedSize() const;   // keep track in book

    OrderBook();

private:

    void addOrder(Order*);
    bool deleteOrder(uint64_t orderReferenceNumber);

    // <price, Level> sorted log(n)
    std::map<uint32_t, Level*> bids;
    std::map<uint32_t, Level*> offers;

    // quick access
    // <referenceNumber, Order>
    google::dense_hash_map<uint64_t, Order*> orders;
    // <price, Level> o(1)
    // map for each side in case same price
    google::dense_hash_map<uint32_t, Level*> levelBids;
    google::dense_hash_map<uint32_t, Level*> levelOffers;

    boost::object_pool<Order> ordersmem;
    boost::object_pool<Level> levelsmem;
};

template <typename OStream>
inline OStream& operator<<(OStream& os, Order const & o) {
    os <<
        "Order: " << &o <<
        " referenceNumer " << o.referenceNumber <<
        " stockLocate " << o.stockLocate <<
        " timestamp " << o.timestamp <<
        " side " << o.side <<
        " shares " << o.shares <<
        " price " << o.price <<
        " prev " << o.prev <<
        " next " << o.next;
    return os;
}

template <typename OStream>
inline OStream& operator<<(OStream& os, Level const & l) {
    os << "Level: " << &l << " " << "first: " << l.first << " last: " << l.last << " volume: " << l.limitVolume << std::endl;
    Order const * it = l.first;
    std::string prefix = ">";
    while (it) {
        os << prefix << it << ": " << *it << std::endl;
        prefix += ">";
        it = it->next;
    }
    return os;
}

#endif // ORDER_BOOK_ORDER_BOOK_HPP
