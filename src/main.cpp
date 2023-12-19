#include "itch_common.hpp"
#include "itch_reader.hpp"
#include "order_book.hpp"
#include <iostream>
#include <boost/pool/object_pool.hpp>
#include <sparsehash/dense_hash_map>

#define BENCH true

#if BENCH
#include <chrono>
#endif

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "Usage: " << argv[0] << " itch_filename" << std::endl; return EXIT_FAILURE; }
    std::cout << "Processing " << argv[1] << std::endl;

    ITCH::Reader reader(argv[1], 16384);
    boost::object_pool<OrderBook> booksmem;
    google::dense_hash_map<uint16_t, OrderBook*> books;
    books.set_empty_key(0);
    books.set_deleted_key(-1);
    char const * messageData;

#if BENCH
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::seconds;
    long long messageCount = 0;
    auto t1 = high_resolution_clock::now();
#endif

    while((messageData = reader.nextMessage())) {
        char messageType = messageData[ITCH::messageTypeIndex];
        messageData += ITCH::messageHeaderLength;
        switch (messageType) {
            [[likely]] case ITCH::AddOrderMessageType: {
                ITCH::AddOrderMessage m = ITCH::Parser::createAddOrderMessage(messageData);
                if (!books.count(m.stockLocate)) {
                    OrderBook * const newBook = booksmem.construct();
                    books.insert(std::pair(m.stockLocate, newBook));
                }
                books[m.stockLocate]->handleAddOrderMessage(m);
                break;
            }
            case ITCH::AddOrderMPIDAttributionMessageType: {
                ITCH::AddOrderMPIDAttributionMessage m = ITCH::Parser::createAddOrderMPIDAttributionMessage(messageData);
                if (!books.count(m.stockLocate)) {
                    OrderBook * const newBook = booksmem.construct();
                    books.insert(std::pair(m.stockLocate, newBook));
                }
                books[m.stockLocate]->handleAddOrderMPIDAttributionMessage(m);
                break;
            }
/*            case ITCH::OrderExecutedMessageType: {
                ITCH::OrderExecutedMessage m = ITCH::Parser::createOrderExecutedMessage(messageData);
                books[m.stockLocate]->handleOrderExecutedMessage(m);
                break;
            }
            case ITCH::OrderExecutedWithPriceMessageType: {
                ITCH::OrderExecutedWithPriceMessage m = ITCH::Parser::createOrderExecutedWithPriceMessage(messageData);
                books[m.stockLocate]->handleOrderExecutedWithPriceMessage(m);
                break;
            }*/
            case ITCH::OrderCancelMessageType: {
                ITCH::OrderCancelMessage m = ITCH::Parser::createOrderCancelMessage(messageData);
                books[m.stockLocate]->handleOrderCancelMessage(m);
                break;
            }
            [[likely]] case ITCH::OrderDeleteMessageType: {
                ITCH::OrderDeleteMessage m = ITCH::Parser::createOrderDeleteMessage(messageData);
                books[m.stockLocate]->handleOrderDeleteMessage(m);
                break;
            }
            [[likely]] case ITCH::OrderReplaceMessageType: {
                ITCH::OrderReplaceMessage m = ITCH::Parser::createOrderReplaceMessage(messageData);
                books[m.stockLocate]->handleOrderReplaceMessage(m);
                break;
            }
/*            case ITCH::TradeMessageType: {
                ITCH::TradeMessage m = ITCH::Parser::createTradeMessage(messageData);
                break;
            }
            [[unlikely]] case ITCH::CrossTradeMessageType: {
                ITCH::CrossTradeMessage m = ITCH::Parser::createCrossTradeMessage(messageData);
                break;
            }
            [[unlikely]] case ITCH::BrokenTradeMessageType: {
                ITCH::BrokenTradeMessage m = ITCH::Parser::createBrokenTradeMessage(messageData);
                break;
            }*/
            default: {
                break;
            }
        };
#if BENCH
    ++messageCount;
#endif
    }

#if BENCH
    auto t2 = high_resolution_clock::now();
    std::cout << "processed " << messageCount << " messages (" << reader.getTotalBytesRead()  << " bytes) in " << duration_cast<seconds>(t2 - t1).count() << " seconds" << std::endl;
#endif
}

