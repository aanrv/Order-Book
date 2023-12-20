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

Order::Order(const std::tuple<uint64_t, uint16_t, uint64_t, char, uint32_t, uint32_t, Order*, Order*> & args) :
    referenceNumber(std::get<0>(args)),
    stockLocate(std::get<1>(args)),
    timestamp(std::get<2>(args)),
    side(std::get<3>(args)),
    shares(std::get<4>(args)),
    price(std::get<5>(args)),
    prev(std::get<6>(args)),
    next(std::get<7>(args))
{}

OrderBook::OrderBook() {
    orders.set_empty_key(0);
    levelBids.set_empty_key(0);
    levelOffers.set_empty_key(0);
    orders.set_deleted_key(-1);
    levelBids.set_deleted_key(-1);
    levelOffers.set_deleted_key(-1);

}

/*
 * Check if order already exists, do nothing and return if does
 * Construct order using mempool
 * addOrder()
 */
void OrderBook::handleAddOrderMessage(ITCH::AddOrderMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(!orders.count(msg.orderReferenceNumber));
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
    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 0);
    addOrder(newOrder);
    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 1);
}

void OrderBook::handleAddOrderMPIDAttributionMessage(ITCH::AddOrderMPIDAttributionMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(!orders.count(msg.orderReferenceNumber));
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
    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 0);
    addOrder(newOrder);
    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 1);
}

/*void OrderBook::handleOrderExecutedMessage(ITCH::OrderExecutedMessage const & msg) {
    DLOG(INFO) << msg;
}
void OrderBook::handleOrderExecutedWithPriceMessage(ITCH::OrderExecutedWithPriceMessage const & msg) {
    DLOG(INFO) << msg;
}*/
void OrderBook::handleOrderCancelMessage(ITCH::OrderCancelMessage const & msg) {
    DLOG(INFO) << msg;
    Order * o = orders.find(msg.orderReferenceNumber)->second;
    DLOG_ASSERT(o);
    DLOG_ASSERT(msg.cancelledShares < o->shares);
    o->shares -= msg.cancelledShares;
    auto levels = o->side == ITCH::Side::BUY ? levelBids : levelOffers;
    levels[o->price]->limitVolume -= msg.cancelledShares;
}

void OrderBook::handleOrderDeleteMessage(ITCH::OrderDeleteMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(orders.count(msg.orderReferenceNumber) == 1);
    deleteOrder(msg.orderReferenceNumber);
    DLOG_ASSERT(orders.count(msg.orderReferenceNumber) == 0);
}

void OrderBook::handleOrderReplaceMessage(ITCH::OrderReplaceMessage const & msg) {
    DLOG(INFO) << msg;
    DLOG_ASSERT(orders.count(msg.originalOrderReferenceNumber));
    Order const * oldOrder = orders.find(msg.originalOrderReferenceNumber)->second;
    DLOG_ASSERT(oldOrder);
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

    DLOG_ASSERT(orders.count(msg.originalOrderReferenceNumber) == 1);
    deleteOrder(msg.originalOrderReferenceNumber);
    DLOG_ASSERT(orders.count(msg.originalOrderReferenceNumber) == 0);

    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 0);
    addOrder(newOrder);
    DLOG_ASSERT(orders.count(newOrder->referenceNumber) == 1);
}

void OrderBook::addOrder(Order* newOrder) {
    DLOG_ASSERT(newOrder->side == ITCH::Side::BUY || newOrder->side == ITCH::Side::SELL);
    // add order to id,order map
    [[maybe_unused]] auto const orderRes = orders.insert(std::pair(newOrder->referenceNumber, newOrder));
    DLOG_ASSERT(orderRes.second);
    auto & levels = newOrder->side == ITCH::Side::BUY ? levelBids : levelOffers;
    // create level if doesnt exist
    if (!levels.count(newOrder->price)) {
        // add level to mempool
        Level * const newLevel = levelsmem.construct(newOrder->price);
        DLOG_ASSERT(newLevel);
        // insert into price,level map
        [[maybe_unused]] auto const levelRes = levels.insert(std::pair(newLevel->price, newLevel));
        DLOG_ASSERT(levelRes.second);

        // insert level into corresponding bid/ask tree
        auto & targetMap = newOrder->side == ITCH::Side::BUY
            ? bids
            : offers;
        [[maybe_unused]] auto const priceRes = targetMap.insert(std::pair(newLevel->price, newLevel));
        DLOG_ASSERT(priceRes.second);
        DLOG(INFO) << "LVL added " << *newLevel;
        // if level is empty, inserted order is both first and last
        newLevel->first = newOrder;
        newLevel->last = newOrder;
        newLevel->limitVolume += newOrder->shares;
        DLOG(INFO) << "ADD added order " << newOrder->referenceNumber << " to level " << newLevel;
    } else {
        // get level for price and add order to end of level
        Level * const orderLevel = levels.find(newOrder->price)->second;
        DLOG_ASSERT(orderLevel);
        // otherwise just append and update last
        orderLevel->last->next = newOrder;
        newOrder->prev = orderLevel->last;
        orderLevel->last = newOrder;
        orderLevel->limitVolume += newOrder->shares;
        DLOG(INFO) << "ADD added order " << newOrder->referenceNumber << " to level " << orderLevel;
    }
}

void OrderBook::deleteOrder(uint64_t orderReferenceNumber) {
    DLOG_ASSERT(orders.count(orderReferenceNumber));
    Order * const target = orders.find(orderReferenceNumber)->second;
    DLOG_ASSERT(target);
    // remove order from map
    [[maybe_unused]] size_t orderEraseNum = orders.erase(orderReferenceNumber);
    DLOG_ASSERT(orderEraseNum);

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
    Level * const level = levels.find(target->price)->second;
    DLOG_ASSERT(level);
    level->limitVolume -= target->shares;
    DLOG_ASSERT(level);
    if (level->first == target) {
        level->first = target->next;
    }
    if (level->last == target) {
        level->last = target->prev;
    }
    // remove and destroy level if empty
    // consider not destroying when empty, more memory but better performance if more orders with same price come in
    if (!level->limitVolume) {
        DLOG(INFO) << "LVL deleting level " << level->price << " side " << target->side;
        [[maybe_unused]] size_t levelEraseNum = levels.erase(level->price);
        DLOG_ASSERT(levelEraseNum);
        DLOG_ASSERT(target->side == ITCH::Side::BUY || target->side == ITCH::Side::SELL);
        auto & targetMap = target->side == ITCH::Side::BUY
            ? levelBids
            : levelOffers;
        DLOG(INFO) << "LVL deleting from map " << target->side << " " << target->price;
        [[maybe_unused]] size_t priceEraseNum = targetMap.erase(level->price);
        DLOG_ASSERT(priceEraseNum);
        levelsmem.destroy(level);
    }
    DLOG(INFO) << "DEL deleted order " << target->referenceNumber << " from level " << level;
    ordersmem.destroy(target);
}


uint32_t OrderBook::getLimitVolume(char side, uint32_t price) const {
    auto const & levelSideMap = side == ITCH::Side::BUY ? levelBids : levelOffers;
    Level const * level = levelSideMap.find(price)->second;
    DLOG_ASSERT(level);
    return level ? level->limitVolume : 0;
}

uint32_t OrderBook::getBestBid() const {
    auto const & it = bids.rbegin();
    return it != bids.rend() ? bids.rbegin()->second->price : 0;
}
uint32_t OrderBook::getBestAsk() const {
    auto const & it = offers.begin();
    return it != offers.end() ? offers.begin()->second->price : 0;
}
/*
uint32_t OrderBook::getLastExecutedPrice() const;
uint32_t OrderBook::getLastExecutedSize() const;
*/
