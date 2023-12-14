#include "order_book.hpp"
#include "itch_common.hpp"
#include <cstdint>
#include <tuple>

#include <iostream>
using namespace std;

Order::Order(std::tuple<uint64_t, uint16_t, uint64_t, char, uint32_t, uint32_t, Order*, Order*> args) :
    referenceNumber(std::get<0>(args)),
    stockLocate(std::get<1>(args)),
    timestamp(std::get<2>(args)),
    side(std::get<3>(args)),
    shares(std::get<4>(args)),
    price(std::get<5>(args)),
    next(std::get<6>(args)),
    prev(std::get<7>(args))
{}

bool OrderBook::addOrder(ITCH::AddOrderMessage const & msg) {
    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has a duplicate
        // consider erroneous for now
        return false;
    }
    auto orderArgs = std::make_tuple(
        msg.orderReferenceNumber,
        msg.stockLocate,
        msg.timestamp,
        msg.buySellIndicator, 
        msg.shares,
        msg.price,
        nullptr,
        nullptr
    );
    // construct Order in mempool
    // add Order* to unordered_map
    Order * const newOrder = ordersmem.construct(orderArgs);
    if (!newOrder) { return false; }
    //cout << "created order" << endl;
    auto const & orderRes = orders.insert({newOrder->referenceNumber, newOrder});
    if (!orderRes.second) { ordersmem.destroy(newOrder); return false; }
    //cout << "inserted order" << endl;

    // create level if needed
    if (!levels.contains(newOrder->price)) {
        Level * const newLevel = levelsmem.construct(newOrder->price);
        //cout << "created level" << endl;
        auto const & levelRes = levels.insert({newOrder->price, newLevel});
        if (!levelRes.second) {
            //cout << "destroying level res" << endl;
            ordersmem.destroy(newOrder);
            levelsmem.destroy(newLevel);
            orders.erase(orderRes.first);
            return false;
        }
        //cout << "inserted level" << endl;
    }
    // get level for price, add order to end, update num shares
    Level * const orderLevel = levels.at(newOrder->price);
    // if level is empty, insert is both first and last
    if (!orderLevel->last) {
        assert(!orderLevel->first);
        orderLevel->first = newOrder;
        orderLevel->last = newOrder;
    } else {
        orderLevel->last->next = newOrder;
    }
//    cout << "created order " << newOrder->referenceNumber << " at level " << orderLevel->price << endl;
    return true;
}
/*
void OrderBook::deleteOrder(uint64_t orderReferenceNumber) {

}
void OrderBook::cancelOrder(uint64_t orderReferenceNumber, uint32_t shares) {

}
void OrderBook::replaceOrder(uint64_t oldOrderReferenceNumber, const Order& newOrder) {

}

uint32_t OrderBook::getLimitVolume(uint32_t price) const {

}
Order const * OrderBook::getBestBid() const;
Order const * OrderBook::getBestAsk() const;
uint32_t OrderBook::getLastExecutedPrice() const;
uint32_t OrderBook::getLastExecutedSize() const;
*/

Level::Level(uint32_t _price) :
    price(_price),
    limitVolume(0),
    first(nullptr),
    last(nullptr)
{}
















