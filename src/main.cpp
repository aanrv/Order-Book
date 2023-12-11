#include "include/itch_reader.hpp"
#include <iostream>

#define BENCH true

#if BENCH
#include <chrono>
#endif

using std::cout; using std::endl;

int main() {
    ITCH::Reader reader("spec/12302019.NASDAQ_ITCH50", 16384);

#if BENCH
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;
    long long messageCount = 0;
    auto t1 = high_resolution_clock::now();
#endif

    char const * messageData;
    while((messageData = reader.nextMessage())) {
#if BENCH
        ++messageCount;
#endif
        char dummytype = 0;
        char messageType = messageData[ITCH::messageTypeIndex];
        messageData += ITCH::messageHeaderLength;
        switch (messageType) {
            case ITCH::AddOrderMessageType: {
                ITCH::AddOrderMessage m = ITCH::Parser::createAddOrderMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::AddOrderMPIDAttributionMessageType: {
                ITCH::AddOrderMPIDAttributionMessage m = ITCH::Parser::createAddOrderMPIDAttributionMessage(messageData);
                dummytype = m.messageType;
                break;
            }
/*            case ITCH::OrderExecutedMessageType: {
                ITCH::OrderExecutedMessage m = ITCH::Parser::createOrderExecutedMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::OrderExecutedWithPriceMessageType: {
                ITCH::OrderExecutedWithPriceMessage m = ITCH::Parser::createOrderExecutedWithPriceMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::OrderCancelMessageType: {
                ITCH::OrderCancelMessage m = ITCH::Parser::createOrderCancelMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::OrderDeleteMessageType: {
                ITCH::OrderDeleteMessage m = ITCH::Parser::createOrderDeleteMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::OrderReplaceMessageType: {
                ITCH::OrderReplaceMessage m = ITCH::Parser::createOrderReplaceMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::TradeMessageType: {
                ITCH::TradeMessage m = ITCH::Parser::createTradeMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::CrossTradeMessageType: {
                ITCH::CrossTradeMessage m = ITCH::Parser::createCrossTradeMessage(messageData);
                dummytype = m.messageType;
                break;
            }
            case ITCH::BrokenTradeMessageType: {
                ITCH::BrokenTradeMessage m = ITCH::Parser::createBrokenTradeMessage(messageData);
                dummytype = m.messageType;
                break;
            }*/
            default: {
                cout << dummytype;
                break;
            }
        };

    }

#if BENCH
    auto t2 = high_resolution_clock::now();
    duration<double, std::nano> ms_double = t2 - t1;
    std::cout << "processed " << messageCount << " messages in " << ms_double.count() << " nanoseconds" << std::endl;
#endif

}

