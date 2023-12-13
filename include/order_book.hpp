#ifndef ORDER_BOOK_ORDER_BOOK_HPP
#define ORDER_BOOK_ORDER_BOOK_HPP

#include <map>
#include <list>
#include <unordered_map>
#include <cstdint>

struct Order {
    uint64_t    referenceNumber;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    char        side;
    uint32_t    shares;
    uint32_t    price;
};

struct Level {
    uint32_t limitPrice;
    uint32_t limitVolume;
    std::list<Order> orders;
};

// price-time LOB
class OrderBook {
public:
    void addOrder(const Order& o);
    void deleteOrder(uint64_t orderReferenceNumber);
    void cancelOrder(uint64_t orderReferenceNumber, uint32_t shares);
    void replaceOrder(uint64_t oldOrderReferenceNumber, const Order& newOrder);

    uint32_t getLimitVolume(uint32_t limitprice) const;
    Order const * getBestBid() const;
    Order const * getBestAsk() const;
    uint32_t getLastExecutedPrice() const;
    uint32_t getLastExecutedSize() const;
private:
    // <price, Level>
    std::map<uint32_t, Level> bids;
    std::map<uint32_t, Level> offers;

    // quick access
    // <referenceNumber, Order>
    std::unordered_map<uint64_t, Order> orders;
    // <limitPrice, Level>
    std::unordered_map<uint32_t, Level> levels;
};

#endif // ORDER_BOOK_ORDER_BOOK_HPP
