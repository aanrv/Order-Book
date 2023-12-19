#ifndef ORDER_BOOK_ORDER_BOOK_HPP
#define ORDER_BOOK_ORDER_BOOK_HPP

#include "itch_common.hpp"
#include <map>
#include <list>
#include <cstdint>
#include <tuple>
#include <boost/pool/object_pool.hpp>
#include <sparsehash/dense_hash_map>

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
    void handleSystemEventMessage(ITCH::SystemEventMessage const & msg);
    void handleStockDirectoryMessage(ITCH::StockDirectoryMessage const & msg);
    void handleStockTradingActionMessage(ITCH::StockTradingActionMessage const & msg);
    void handleRegSHORestrictionMessage(ITCH::RegSHORestrictionMessage const & msg);
    void handleMarketParticipantPositionMessage(ITCH::MarketParticipantPositionMessage const & msg);
    void handleMWCBDeclineLevelMessage(ITCH::MWCBDeclineLevelMessage const & msg);
    void handleMWCBStatusMessage(ITCH::MWCBStatusMessage const & msg);
    void handleIPOQuotingPeriodUpdateMessage(ITCH::IPOQuotingPeriodUpdateMessage const & msg);
    void handleLULDAuctionCollarMessage(ITCH::LULDAuctionCollarMessage const & msg);
    void handleOperationalHaltMessage(ITCH::OperationalHaltMessage const & msg);
    void handleAddOrderMessage(ITCH::AddOrderMessage const & msg);
    void handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const & msg);
    void handleOrderExecutedMessage(ITCH::OrderExecutedMessage const & msg);
    void handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg);
    void handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg);
    void handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg);
    void handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg);
    void handleTradeMessage(ITCH::TradeMessage const & msg);
    void handleCrossTradeMessage(ITCH::CrossTradeMessage const & msg);
    void handleBrokenTradeMessage(ITCH::BrokenTradeMessage const & msg);
    void handleNOIIMessage(ITCH::NOIIMessage const & msg);
    void handleRetailInterestMessage(ITCH::RetailInterestMessage const & msg);
    void handleDirectListingWithCapitalRaisePriceDiscoveryMessage(ITCH::DirectListingWithCapitalRaisePriceDiscoveryMessage const & msg);

    uint32_t getLimitVolume(char side, uint32_t limitprice) const; // keep track Level obj
    uint32_t getBestBid() const;   // largest map
    uint32_t getBestAsk() const;   // lowest map
    uint32_t getLastExecutedPrice() const;  // keep track in book
    uint32_t getLastExecutedSize() const;   // keep track in book

    OrderBook();

private:

    bool addOrder(Order*);
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
    os << "Level: " << &l << std::endl;
    os << "first: " << l.first << ", last: " << l.last << std::endl;
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
