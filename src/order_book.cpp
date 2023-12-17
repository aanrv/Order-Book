#include "order_book.hpp"
#include "itch_common.hpp"
#include <cstdint>
#include <tuple>
#include <glog/logging.h>

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
    DLOG_ASSERT(!orders.contains(msg.orderReferenceNumber));
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
    DLOG_ASSERT(newOrder);
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(!orders.contains(msg.orderReferenceNumber));
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
    DLOG_ASSERT(newOrder);
    if (!addOrder(newOrder)) {
        ordersmem.destroy(newOrder);
    }
}

void OrderBook::handleOrderExecutedMessage(ITCH::OrderExecutedMessage const & msg) {
    DLOG(INFO) << msg;
}
void OrderBook::handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg) {
    DLOG(INFO) << msg;
}
void OrderBook::handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg) {
    DLOG(INFO) << msg;
}

void OrderBook::handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg) {
    DLOG(INFO) << msg;
    deleteOrder(msg.orderReferenceNumber);
}

void OrderBook::handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(orders.count(msg.originalOrderReferenceNumber));
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
    DLOG_ASSERT(newOrder);
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
    DLOG_ASSERT(orderRes.second);
    auto & levels = newOrder->side == ITCH::Side::BUY ? levelBids : levelOffers;
    // create level if doesnt exist
    if (!levels.contains(newOrder->price)) {
        // add level to mempool
        Level * const newLevel = levelsmem.construct(newOrder->price);
        DLOG_ASSERT(newLevel);
        // insert into price,level map
        auto const levelRes = levels.insert(std::pair(newLevel->price, newLevel));
        (void)levelRes;
        DLOG_ASSERT(levelRes.second);

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

    DLOG_ASSERT(levels.count(newOrder->price));

    // get level for price and add order to end of level
    Level * const orderLevel = levels.at(newOrder->price);
    if (!orderLevel->last) {
        // TODO temp check
        DLOG_ASSERT(!orderLevel->first);
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
    DLOG_ASSERT(orders.count(orderReferenceNumber));
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
    DLOG_ASSERT(levels.count(target->price));
    Level * const level = levels.at(target->price);
    DLOG_ASSERT(level);
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

        DLOG_ASSERT(target->side == ITCH::Side::BUY || target->side == ITCH::Side::SELL);
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

