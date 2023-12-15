#include "order_book.hpp"
#include "itch_common.hpp"
#include <cstdint>
#include <tuple>

#include <iostream>
using namespace std;

#define LOG false

Level::Level(uint32_t _price) :
    price(_price),
    limitVolume(0),
    first(nullptr),
    last(nullptr)
{}

Order::Order(std::tuple<uint64_t, uint16_t, uint64_t, char, uint32_t, uint32_t, Order*, Order*> args) :
    referenceNumber(std::get<0>(args)),
    stockLocate(std::get<1>(args)),
    timestamp(std::get<2>(args)),
    side(std::get<3>(args)),
    shares(std::get<4>(args)),
    price(std::get<5>(args)),
    prev(std::get<6>(args)),
    next(std::get<7>(args))
{}

void OrderBook::handleAddOrderMessage(ITCH::AddOrderMessage const & msg) {
    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has a duplicate
        // consider erroneous for now
#if LOG
        cerr << "ERR addOrder: duplicate order with reference number ---\n";
        cerr << msg << "\n";
#endif
        return;
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
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const & msg) {
    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has a duplicate
        // consider erroneous for now
#if LOG
        cerr << "ERR addOrder: duplicate order with reference number ---\n";
        cerr << msg << "\n";
#endif
        return;
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
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleOrderExecutedMessage(ITCH::OrderExecutedMessage const & msg) {
    (void)msg;
}
void OrderBook::handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg) {
    (void)msg;
}
void OrderBook::handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg) {
    (void)msg;
}

void OrderBook::handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg) {
    deleteOrder(msg.orderReferenceNumber);
}

void OrderBook::handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg) {
    if (!orders.count(msg.originalOrderReferenceNumber)) {
#if LOG
        cerr << "ERR replaceOrder: failed to find original message " << msg.originalOrderReferenceNumber << ", unable to add new order" << endl;
#endif
        return;
    }
    Order const * oldOrder = orders.at(msg.originalOrderReferenceNumber);
    auto orderArgs = std::make_tuple(
        msg.newOrderReferenceNumber,
        oldOrder->stockLocate,
        msg.timestamp,
        oldOrder->side,
        msg.shares,
        msg.price,
        nullptr,
        nullptr
        );
    Order * const newOrder = ordersmem.construct(orderArgs);
    if (!newOrder) throw std::runtime_error("addOrder: failed to construct order " + std::to_string(msg.newOrderReferenceNumber));
    if (!deleteOrder(msg.originalOrderReferenceNumber)) {
        ordersmem.destroy(newOrder);
    }
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

// TODO add levels to bids offers
// TODO change throws to return false?
bool OrderBook::addOrder(Order* newOrder) {
    auto const & orderRes = orders.insert({newOrder->referenceNumber, newOrder});
    if (!orderRes.second) throw std::runtime_error("addOrder: failed to insert order " + std::to_string(newOrder->referenceNumber));

    // create level if doesnt exist
    if (!levels.contains(newOrder->price)) {
        Level * const newLevel = levelsmem.construct(newOrder->price);
        if (!newLevel) throw std::runtime_error("addOrder: failed to construct level " + std::to_string(newOrder->price));
        // insert into map
        auto const & levelRes = levels.insert({newOrder->price, newLevel});
        if (!levelRes.second) {
            orders.erase(newOrder->referenceNumber);
            levelsmem.destroy(newLevel);
            throw std::runtime_error("addOrder: failed to insert level " + std::to_string(newOrder->price));
        }
        // insert level into bids/offers map
        if(newOrder->side != ITCH::Side::BUY && newOrder->side != ITCH::Side::SELL) {
            cerr << *newOrder << endl;
            cerr << "invalid side" << endl;
            orders.erase(orderRes.first);
            levels.erase(newOrder->price);
            levelsmem.destroy(newLevel);
            return false;
        }
        std::map<uint32_t, Level*> & bidsOrOffers = newOrder->side == ITCH::Side::BUY
            ? bids
            : offers;
        auto const & sideRes = bidsOrOffers.insert({newLevel->price, newLevel});
        if (!sideRes.second) {
            orders.erase(orderRes.first);
            levels.erase(newOrder->price);
            levelsmem.destroy(newLevel);
            throw std::runtime_error("addOrder: failed to insert level for price " + std::to_string(newOrder->price));
        }
    }
    // get level for price, add order to end, update num shares
    assert(levels.count(newOrder->price));
    Level * const orderLevel = levels.at(newOrder->price);
    assert(orderLevel != nullptr);
    if (!orderLevel->last) {
        if (orderLevel->first) {
#if LOG
            std::cerr << "ERR" << std::endl;
            std::cerr << *orderLevel << std::endl;
#endif
            throw std::runtime_error("addOrder: non empty level with null last");
        }
        // if level is empty, insert is both first and last
        orderLevel->first = newOrder;
        orderLevel->last = newOrder;
    } else {
        // otherwise append and update last
        orderLevel->last->next = newOrder;
        newOrder->prev = orderLevel->last;
        orderLevel->last = newOrder;
    }
    return true;
}

bool OrderBook::deleteOrder(uint64_t orderReferenceNumber) {
    // get order to delete
    if (!orders.count(orderReferenceNumber)) {
#if LOG
        std::cerr << "ERR Order: " << orderReferenceNumber << " not found for deletion" << std::endl;
#endif
        return false;
    }
    Order * const target = orders.at(orderReferenceNumber);
    // remove order from map
    if (!orders.erase(orderReferenceNumber)) throw std::runtime_error("deleteOrder: failed to erase order " + std::to_string(orderReferenceNumber));

    // remove order from list, connect remaining nodes
    if (target->prev) {
        target->prev->next = target->next;
    }
    if (target->next) {
        target->next->prev = target->prev;
    }
    // remove from level pointers if first/last
    assert(levels.count(target->price));
    Level * const level = levels.at(target->price);
#if LOG
    if (!level) {
        cerr << "ERR deleteOrder: failed to find level " << target->price << endl;
    }
#endif
    if (level->first == target) {
        level->first = target->next;
    }
    if (level->last == target) {
        level->last = target->prev;
    }
    // remove and destroy level if empty
    // consider not destroying when empty, more memory but better performance if more orders with same price come in
    if (!level->first && !level->last) {
        std::map<uint32_t, Level*> & bidsOrOffers = target->side == ITCH::Side::BUY
            ? bids
            : offers;
        if (!bidsOrOffers.erase(level->price)) {
            cout << "------------" << endl;
            cout << *target << endl;
            cout << *level << endl;
            throw std::runtime_error("erased from levels but not from bid/ask");
        }
#if LOG
        cout << "deleting level " << level->price << " side " << target->side << endl;
#endif
        if (!levels.erase(level->price)) {
            cerr << "ERR deleteOrder: failed to erase level for destroy " << level->price << endl;
        }
        levelsmem.destroy(level);
    }
    ordersmem.destroy(target);
    return true;
}

/*
uint32_t OrderBook::getLimitVolume(uint32_t price) const;
Order const * OrderBook::getBestBid() const;
Order const * OrderBook::getBestAsk() const;
uint32_t OrderBook::getLastExecutedPrice() const;
uint32_t OrderBook::getLastExecutedSize() const;
*/

