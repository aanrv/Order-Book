#include "order_book.hpp"
#include "itch_common.hpp"
#include <cstdint>
#include <tuple>

// TODO use boost log or glog instead
#include <iostream>
#define LOG true

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

/*
 * Check if order already exists, do nothing and return if does
 * Construct order using mempool
 * addOrder()
 */
void OrderBook::handleAddOrderMessage(ITCH::AddOrderMessage const & msg) {
#if LOG
    std::cout << msg << std::endl;
#endif

    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has duplicates
        // consider erroneous and ignore
#if LOG
        std::cerr << "ERR addOrder: duplicate order with reference number ---\n";
        std::cerr << msg << "\n";
        throw std::runtime_error("duplicate order");
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
#if LOG
    std::cout << msg << std::endl;
#endif

    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has a duplicate
        // consider erroneous for now
#if LOG
        std::cerr << "ERR addOrder: duplicate order with reference number ---\n";
        std::cerr << msg << "\n";
        throw std::runtime_error("duplicate order");
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
#if LOG
    std::cout << msg << std::endl;
#endif
    (void)msg;
}
void OrderBook::handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg) {
#if LOG
    std::cout << msg << std::endl;
#endif
    (void)msg;
}
void OrderBook::handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg) {
#if LOG
    std::cout << msg << std::endl;
#endif
    (void)msg;
}

void OrderBook::handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg) {
#if LOG
    std::cout << msg << std::endl;
#endif
    deleteOrder(msg.orderReferenceNumber);
}

void OrderBook::handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg) {
#if LOG
    std::cout << msg << std::endl;
#endif
    if (!orders.count(msg.originalOrderReferenceNumber)) {
#if LOG
        std::cerr << "ERR replaceOrder: failed to find original message " << msg.originalOrderReferenceNumber << ", unable to add new order" << std::endl;
#endif
        throw std::runtime_error("order");
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

// TODO change throws to return false once done testing
// adds order to id map and level obj
// creates level obj if needed
// if any failures, state is reverted, order level objects destroyed
bool OrderBook::addOrder(Order* newOrder) {
    if (newOrder->side != ITCH::Side::BUY && newOrder->side != ITCH::Side::SELL) {
        std::cout << "ERR unable to add order " << *newOrder << ", invalid side" << std::endl;
        return false;
    }
    // add order to id,order map
    auto const orderRes = orders.insert(std::pair(newOrder->referenceNumber, newOrder));
    if (!orderRes.second) throw std::runtime_error("addOrder: failed to insert order " + std::to_string(newOrder->referenceNumber));

    // create level if doesnt exist
    if (!levels.contains(newOrder->price)) {
        // add level to mempool
        Level * const newLevel = levelsmem.construct(newOrder->price);
        if (!newLevel) throw std::runtime_error("addOrder: failed to construct level " + std::to_string(newOrder->price));

        // insert into price,level map
        auto const levelRes = levels.insert(std::pair(newLevel->price, newLevel));
        if (!levelRes.second) {
            // remove references to order
            // destroy order and level
            orders.erase(newOrder->referenceNumber);
            ordersmem.destroy(newOrder);
            levelsmem.destroy(newLevel);
            throw std::runtime_error("addOrder: failed to insert level " + std::to_string(newOrder->price));
        }

        // insert level into corresponding bid/ask tree
        auto & targetMap = newOrder->side == ITCH::Side::BUY
            ? bids
            : offers;
        if (!targetMap.insert(std::pair(newLevel->price, newLevel)).second) {
            std::cout << "---" << std::endl;
            std::cout << *newLevel << std::endl;
            std::cout << *newOrder << std::endl;
            throw std::runtime_error("failed to add level to bids offers");
        }
        std::cout << "LVL added " << *newLevel << std::endl;
    }

    // TODO add assert flag
    assert(levels.count(newOrder->price));

    // get level for price and add order to end of level
    Level * const orderLevel = levels.at(newOrder->price);
    if (!orderLevel->last) {
        // TODO temp check
        if (orderLevel->first) {
#if LOG
            std::cerr << "ERR" << std::endl;
            std::cerr << *orderLevel << std::endl;
#endif
            throw std::runtime_error("addOrder: non empty level with null last");
        }
        // if level is empty, inserted order is both first and last
        orderLevel->first = newOrder;
        orderLevel->last = newOrder;
    } else {
        // otherwise just append and update last
        orderLevel->last->next = newOrder;
        newOrder->prev = orderLevel->last;
        orderLevel->last = newOrder;
    }
    std::cout << "ADD added order " << newOrder->referenceNumber << " to level " << orderLevel << std::endl;
    return true;
}

bool OrderBook::deleteOrder(uint64_t orderReferenceNumber) {
    // get order to delete
    if (!orders.count(orderReferenceNumber)) {
#if LOG
        std::cerr << "ERR Order: " << orderReferenceNumber << " not found for deletion" << std::endl;
#endif
        throw std::runtime_error("order not found for delete");
        return false;
    }
    Order * const target = orders.at(orderReferenceNumber);
    // remove order from map
    if (!orders.erase(orderReferenceNumber)) throw std::runtime_error("deleteOrder: found order but failed to erase order " + std::to_string(orderReferenceNumber));

    // remove order from level list, connect remaining nodes
    if (target->prev) {
        target->prev->next = target->next;
    }
    if (target->next) {
        target->next->prev = target->prev;
    }

    // remove from level pointers if first/last
    // TODO assert flag and handle with if
    assert(levels.count(target->price));
    Level * const level = levels.at(target->price);
#if LOG
    if (!level) {
        std::cerr << "ERR deleteOrder: failed to find level " << target->price << std::endl;
        throw std::runtime_error("level not found for delete");
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
#if LOG
        std::cout << "LVL deleting level " << level->price << " side " << target->side << std::endl;
#endif
        // TODO handle properly
        if (!levels.erase(level->price)) {
            std::cerr << "ERR deleteOrder: failed to erase level for destroy " << level->price << std::endl;
            throw std::runtime_error("failed to erase level");
        }

        // TODO assert flag
        assert(target->side == ITCH::Side::BUY || target->side == ITCH::Side::SELL);
        auto & targetMap = target->side == ITCH::Side::BUY
            ? bids
            : offers;
#if LOG
        std::cout << "LVL deleting from map " << target->side << " " << target->price << std::endl;
#endif
        if (!targetMap.erase(level->price)) {
            std::cout << "---" << std::endl;
            std::cout << *level << std::endl;
            std::cout << *target << std::endl;
            throw std::runtime_error("failed to delete level from bids offers");
        }
        levelsmem.destroy(level);
    }
    std::cout << "DEL deleted order " << target->referenceNumber << " from level " << level << std::endl;
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

