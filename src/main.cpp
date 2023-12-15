#include "itch_common.hpp"
#include "itch_reader.hpp"
#include "order_book.hpp"
#include <iostream>
#include <unordered_map>

#define LOG     false
#define BENCH   true

#if BENCH
#include <chrono>
#endif

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "Usage: " << argv[0] << " itch_filename" << std::endl; return EXIT_FAILURE; }
    std::cout << "Processing " << argv[1] << std::endl;

    ITCH::Reader reader(argv[1], 16384);
    std::unordered_map<uint16_t, OrderBook> books;
    char const * messageData;

#if BENCH
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;
    long long messageCount = 0;
    auto t1 = high_resolution_clock::now();
#endif

    while((messageData = reader.nextMessage())) {
        char messageType = messageData[ITCH::messageTypeIndex];
        messageData += ITCH::messageHeaderLength;
        switch (messageType) {
            [[likely]] case ITCH::AddOrderMessageType: {
                ITCH::AddOrderMessage m = ITCH::Parser::createAddOrderMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                books[m.stockLocate].addOrder(m);
                break;
            }
/*            case ITCH::AddOrderMPIDAttributionMessageType: {
                ITCH::AddOrderMPIDAttributionMessage m = ITCH::Parser::createAddOrderMPIDAttributionMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[likely]] case ITCH::OrderExecutedMessageType: {
                ITCH::OrderExecutedMessage m = ITCH::Parser::createOrderExecutedMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            case ITCH::OrderExecutedWithPriceMessageType: {
                ITCH::OrderExecutedWithPriceMessage m = ITCH::Parser::createOrderExecutedWithPriceMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            case ITCH::OrderCancelMessageType: {
                ITCH::OrderCancelMessage m = ITCH::Parser::createOrderCancelMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }*/
            [[likely]] case ITCH::OrderDeleteMessageType: {
                ITCH::OrderDeleteMessage m = ITCH::Parser::createOrderDeleteMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                books[m.stockLocate].deleteOrder(m.orderReferenceNumber);
                break;
            }
            [[likely]] case ITCH::OrderReplaceMessageType: {
                ITCH::OrderReplaceMessage m = ITCH::Parser::createOrderReplaceMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                books[m.stockLocate].replaceOrder(m);
                break;
            }
/*            case ITCH::TradeMessageType: {
                ITCH::TradeMessage m = ITCH::Parser::createTradeMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[unlikely]] case ITCH::CrossTradeMessageType: {
                ITCH::CrossTradeMessage m = ITCH::Parser::createCrossTradeMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[unlikely]] case ITCH::BrokenTradeMessageType: {
                ITCH::BrokenTradeMessage m = ITCH::Parser::createBrokenTradeMessage(messageData);
#if LOG
                std::cout << m << std::endl;
#endif
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
    duration<double, std::nano> ms_double = t2 - t1;
    std::cout << "processed " << messageCount << " messages (" << reader.getTotalBytesRead()  << " bytes) in " << ms_double.count() << " nanoseconds" << std::endl;
#endif
}

