#ifndef ORDER_BOOK_ORDER_BOOK_HPP
#define ORDER_BOOK_ORDER_BOOK_HPP

#include "itch_common.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <cstdint>
#include <tuple>
#include <boost/pool/object_pool.hpp>

struct Order {
    uint64_t    referenceNumber;
    uint16_t    stockLocate;
    uint64_t    timestamp;
    char        side;
    uint32_t    shares;
    uint32_t    price;
    Order*      next;
    Order*      prev;
    Order(std::tuple<uint64_t, uint16_t, uint64_t, char, uint32_t, uint32_t, Order*, Order*>);
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
    bool addOrder(ITCH::AddOrderMessage const & msg);  // add to o1 orders map, get level from o1 levels map and append
    void deleteOrder(uint64_t orderReferenceNumber);    // get order from id map, set prev=next, delete from map
    void cancelOrder(uint64_t orderReferenceNumber, uint32_t shares); // get orde from id map, sub shares
    void replaceOrder(uint64_t oldOrderReferenceNumber, const Order& newOrder); // delete, add

    uint32_t getLimitVolume(uint32_t limitprice) const; // keep track Level obj
    Order const * getBestBid() const;   // largest map
    Order const * getBestAsk() const;   // lowest map
    uint32_t getLastExecutedPrice() const;  // keep track in book
    uint32_t getLastExecutedSize() const;   // keep track in book

private:
    // <price, Level>
    std::map<uint32_t, Level*> bids;
    std::map<uint32_t, Level*> offers;

    // quick access
    // <referenceNumber, Order>
    std::unordered_map<uint64_t, Order*> orders;
    // <price, Level>
    std::unordered_map<uint32_t, Level*> levels;

    boost::object_pool<Order> ordersmem;
    boost::object_pool<Level> levelsmem;
};

#endif // ORDER_BOOK_ORDER_BOOK_HPP
