#include "order_book.hpp"
#include "itch_common.hpp"
#include <cstdint>
#include <tuple>
#include <glog/logging.h>

// TODO use boost log or glog instead
#define ASSERT false

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
    DLOG(INFO) << msg;
#if ASSERT
    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has duplicates
        // consider erroneous and ignore
        DLOG(ERROR) << "ERR addOrder: duplicate order with reference number ---\n";
        DLOG(ERROR) << msg;
        throw std::runtime_error("duplicate order");
        return;
    }
#endif
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
#if ASSERT
    if (!newOrder) throw std::runtime_error("addOrder: failed to construct order " + std::to_string(msg.orderReferenceNumber));
#endif
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const & msg) {
    DLOG(INFO) << msg;
#if ASSERT
    if(orders.contains(msg.orderReferenceNumber)) {
        // spec says ref num is day-unique
        // but file has a duplicate
        // consider erroneous for now
        DLOG(ERROR) << "ERR addOrder: duplicate order with reference number ---\n";
        DLOG(ERROR) << msg << "\n";
        throw std::runtime_error("duplicate order");
        return;
    }
#endif
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
#if ASSERT
    if (!newOrder) throw std::runtime_error("addOrder: failed to construct order " + std::to_string(msg.orderReferenceNumber));
#endif
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleOrderExecutedMessage(ITCH::OrderExecutedMessage const & msg) {
    DLOG(INFO) << msg;
    (void)msg;
}
void OrderBook::handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg) {
    DLOG(INFO) << msg;
    (void)msg;
}
void OrderBook::handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg) {
    DLOG(INFO) << msg;
    (void)msg;
}

void OrderBook::handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg) {
    DLOG(INFO) << msg;
    deleteOrder(msg.orderReferenceNumber);
}

void OrderBook::handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg) {
    DLOG(INFO) << msg;
#if ASSERT
    if (!orders.count(msg.originalOrderReferenceNumber)) {
        DLOG(ERROR) << "ERR replaceOrder: failed to find original message " << msg.originalOrderReferenceNumber << ", unable to add new order";
        throw std::runtime_error("order");
        return;
    }
#endif
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
#if ASSERT
    if (!newOrder) throw std::runtime_error("addOrder: failed to construct order " + std::to_string(msg.newOrderReferenceNumber));
#endif
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
        DLOG(ERROR) << "ERR unable to add order " << *newOrder << ", invalid side";
        return false;
    }
    // add order to id,order map
    auto const orderRes = orders.insert(std::pair(newOrder->referenceNumber, newOrder));
    (void)orderRes;
#if ASSERT
    if (!orderRes.second) throw std::runtime_error("addOrder: failed to insert order " + std::to_string(newOrder->referenceNumber));
#endif
    auto & levels = newOrder->side == ITCH::Side::BUY ? levelBids : levelOffers;
    // create level if doesnt exist
    if (!levels.contains(newOrder->price)) {
        // add level to mempool
        Level * const newLevel = levelsmem.construct(newOrder->price);
#if ASSERT
        if (!newLevel) throw std::runtime_error("addOrder: failed to construct level " + std::to_string(newOrder->price));
#endif
        // insert into price,level map
        auto const levelRes = levels.insert(std::pair(newLevel->price, newLevel));
        (void)levelRes;
#if ASSERT
        if (!levelRes.second) {
            // remove references to order
            // destroy order and level
            orders.erase(newOrder->referenceNumber);
            ordersmem.destroy(newOrder);
            levelsmem.destroy(newLevel);
            throw std::runtime_error("addOrder: failed to insert level " + std::to_string(newOrder->price));
        }
#endif

        // insert level into corresponding bid/ask tree
        auto & targetMap = newOrder->side == ITCH::Side::BUY
            ? bids
            : offers;
        if (!targetMap.insert(std::pair(newLevel->price, newLevel)).second) {
            DLOG(ERROR) << "---";
            DLOG(ERROR) << *newLevel;
            DLOG(ERROR) << *newOrder;
            throw std::runtime_error("failed to add level to bids offers");
        }
        DLOG(INFO) << "LVL added " << *newLevel;
    }

#if ASSERT
    assert(levels.count(newOrder->price));
#endif

    // get level for price and add order to end of level
    Level * const orderLevel = levels.at(newOrder->price);
    if (!orderLevel->last) {
        // TODO temp check
#if ASSERT
        if (orderLevel->first) {
            DLOG(ERROR) << "ERR";
            DLOG(ERROR) << *orderLevel;
            throw std::runtime_error("addOrder: non empty level with null last");
        }
#endif
        // if level is empty, inserted order is both first and last
        orderLevel->first = newOrder;
        orderLevel->last = newOrder;
    } else {
        // otherwise just append and update last
        orderLevel->last->next = newOrder;
        newOrder->prev = orderLevel->last;
        orderLevel->last = newOrder;
    }
    DLOG(INFO) << "ADD added order " << newOrder->referenceNumber << " to level " << orderLevel;
    return true;
}

bool OrderBook::deleteOrder(uint64_t orderReferenceNumber) {
#if ASSERT
    // get order to delete
    if (!orders.count(orderReferenceNumber)) {
        DLOG(ERROR) << "ERR Order: " << orderReferenceNumber << " not found for deletion";
        throw std::runtime_error("order not found for delete");
        return false;
    }
#endif
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

    auto & levels = target->side == ITCH::Side::BUY ? bids : offers;
    // remove from level pointers if first/last
    // TODO assert flag and handle with if
#if ASSERT
    assert(levels.count(target->price));
#endif
    Level * const level = levels.at(target->price);
#if ASSERT
    if (!level) {
        DLOG(ERROR) << "ERR deleteOrder: failed to find level " << target->price;
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
        DLOG(INFO) << "LVL deleting level " << level->price << " side " << target->side;
        // TODO handle properly
        if (!levels.erase(level->price)) {
            DLOG(ERROR) << "ERR deleteOrder: failed to erase level for destroy " << level->price;
            throw std::runtime_error("failed to erase level");
        }

        // TODO assert flag
#if ASSERT
        assert(target->side == ITCH::Side::BUY || target->side == ITCH::Side::SELL);
#endif
        auto & targetMap = target->side == ITCH::Side::BUY
            ? levelBids
            : levelOffers;
        DLOG(INFO) << "LVL deleting from map " << target->side << " " << target->price;
        if (!targetMap.erase(level->price)) {
            DLOG(ERROR) << "---";
            DLOG(ERROR) << *level;
            DLOG(ERROR) << *target;
            throw std::runtime_error("failed to delete level from bids offers");
        }
        levelsmem.destroy(level);
    }
    DLOG(INFO) << "DEL deleted order " << target->referenceNumber << " from level " << level;
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

