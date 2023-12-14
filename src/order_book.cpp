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
        cerr << "--- duplicate order with reference number ---\n";
        cerr << msg << "\n";
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
    if (!newOrder) throw std::runtime_error("addOrder: failed to construct order " + std::to_string(msg.orderReferenceNumber));
    auto const & orderRes = orders.insert({newOrder->referenceNumber, newOrder});
    if (!orderRes.second) throw std::runtime_error("addOrder: failed to insert order " + std::to_string(newOrder->referenceNumber));

    // create level if doesnt exist
    if (!levels.contains(newOrder->price)) {
        Level * const newLevel = levelsmem.construct(newOrder->price);
        if (!newLevel) throw std::runtime_error("addOrder: failed to construct level " + std::to_string(newOrder->price));
        // insert into map
        auto const & levelRes = levels.insert({newOrder->price, newLevel});
        if (!levelRes.second) {
            ordersmem.destroy(newOrder);
            levelsmem.destroy(newLevel);
            orders.erase(orderRes.first);
            throw std::runtime_error("addOrder: failed to insert level " + std::to_string(newOrder->price));
        }
    }
    // get level for price, add order to end, update num shares
    Level * const orderLevel = levels.at(newOrder->price);
    assert(orderLevel != nullptr);
    if (!orderLevel->last) {
        if (orderLevel->first) {
            std::cout << *orderLevel << std::endl;
            throw std::runtime_error("addOrder: non empty level with null last");
        }
        // if level is empty, insert is both first and last
        orderLevel->first = newOrder;
        orderLevel->last = newOrder;
    } else {
        // otherwise append and update last
        orderLevel->last->next = newOrder;
        orderLevel->last = newOrder;
    }
    return true;
}

bool OrderBook::deleteOrder(uint64_t orderReferenceNumber) {
    // get order to delete
    Order * const target = orders.at(orderReferenceNumber);
    if (!target) {
        cerr << "deleteOrder: failed to find order " << orderReferenceNumber << endl;
        return false;
    }
    // remove order from map
    if (!orders.erase(orderReferenceNumber)) {
        cerr << "deleteOrder: failed to erase order " << orderReferenceNumber << endl;
        return false;
    }
    // remove order from list, connect remaining nodes
    if (target->prev) {
        target->prev->next = target->next;
    }
    if (target->next) {
        target->next->prev = target->prev;
    }
    // remove from level pointers if first/last
    Level * const level = levels.at(target->price);
    if (!level) {
        cerr << "deleteOrder: failed to find level " << target->price << endl;
    }
    if (level->first == target) {
        level->first = target->next;
    }
    if (level->last == target) {
        level->last = target->prev;
    }

    // remove and destroy level if empty
    if (!level->first && !level->last) {
        if (!levels.erase(level->price)) {
            cerr << "deleteOrder: failed to erase level for destroy " << level->price << endl;
        }
        levelsmem.destroy(level);
    }
    ordersmem.destroy(target);
    return true;
}
/*
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
















