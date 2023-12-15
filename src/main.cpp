#include "itch_common.hpp"
#include "itch_reader.hpp"
#include "order_book.hpp"
#include <iostream>

#define LOG     true
#define BENCH   true

#if BENCH
#include <chrono>
#endif

int main(int argc, char** argv) {
    if (argc < 2) { std::cout << "Usage: " << argv[0] << " itch_filename" << std::endl; return EXIT_FAILURE; }
    std::cout << "Processing " << argv[1] << std::endl;
    ITCH::Reader reader(argv[1], 16384);

    OrderBook allSym;

    char const * messageData;
    char dummytype;
    unsigned long countA = 0;
    unsigned long countB = 0;
    unsigned long countC = 0;
    unsigned long countD = 0;
    unsigned long countE = 0;
    unsigned long countF = 0;
    unsigned long countG = 0;
    unsigned long countH = 0;
    unsigned long countI = 0;
    unsigned long countJ = 0;

#if BENCH
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;
    long long messageCount = 0;
    auto t1 = high_resolution_clock::now();
#endif
    while((messageData = reader.nextMessage())) {
#if BENCH
        ++messageCount;
#endif
        char messageType = messageData[ITCH::messageTypeIndex];
        messageData += ITCH::messageHeaderLength;
        switch (messageType) {
            [[likely]] case ITCH::AddOrderMessageType: {
                ITCH::AddOrderMessage m = ITCH::Parser::createAddOrderMessage(messageData);
                dummytype = m.messageType;
                ++countA;
#if LOG
                std::cout << m << std::endl;
#endif
                allSym.addOrder(m);
                break;
            }
            case ITCH::AddOrderMPIDAttributionMessageType: {
                ITCH::AddOrderMPIDAttributionMessage m = ITCH::Parser::createAddOrderMPIDAttributionMessage(messageData);
                dummytype = m.messageType;
                ++countB;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[likely]] case ITCH::OrderExecutedMessageType: {
                ITCH::OrderExecutedMessage m = ITCH::Parser::createOrderExecutedMessage(messageData);
                dummytype = m.messageType;
                ++countC;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            case ITCH::OrderExecutedWithPriceMessageType: {
                ITCH::OrderExecutedWithPriceMessage m = ITCH::Parser::createOrderExecutedWithPriceMessage(messageData);
                dummytype = m.messageType;
                ++countD;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            case ITCH::OrderCancelMessageType: {
                ITCH::OrderCancelMessage m = ITCH::Parser::createOrderCancelMessage(messageData);
                dummytype = m.messageType;
                ++countE;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[likely]] case ITCH::OrderDeleteMessageType: {
                ITCH::OrderDeleteMessage m = ITCH::Parser::createOrderDeleteMessage(messageData);
                dummytype = m.messageType;
                ++countF;
#if LOG
                std::cout << m << std::endl;
#endif
                allSym.deleteOrder(m.orderReferenceNumber);
                break;
            }
            [[likely]] case ITCH::OrderReplaceMessageType: {
                ITCH::OrderReplaceMessage m = ITCH::Parser::createOrderReplaceMessage(messageData);
                dummytype = m.messageType;
                ++countG;
#if LOG
                std::cout << m << std::endl;
#endif
                allSym.replaceOrder(m);
                break;
            }
            case ITCH::TradeMessageType: {
                ITCH::TradeMessage m = ITCH::Parser::createTradeMessage(messageData);
                dummytype = m.messageType;
                ++countH;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[unlikely]] case ITCH::CrossTradeMessageType: {
                ITCH::CrossTradeMessage m = ITCH::Parser::createCrossTradeMessage(messageData);
                dummytype = m.messageType;
                ++countI;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            [[unlikely]] case ITCH::BrokenTradeMessageType: {
                ITCH::BrokenTradeMessage m = ITCH::Parser::createBrokenTradeMessage(messageData);
                dummytype = m.messageType;
                ++countJ;
#if LOG
                std::cout << m << std::endl;
#endif
                break;
            }
            default: {
                break;
            }
        };
    }

    std::cout << dummytype << std::endl;
    std::cout << "AddOrder " << countA << std::endl;
    std::cout << "AddOrderMPID " << countB << std::endl;
    std::cout << "OrderExc " << countC << std::endl;
    std::cout << "OrderExcPrc " << countD << std::endl;
    std::cout << "OrderCnl " << countE << std::endl;
    std::cout << "OrderDlt " << countF << std::endl;
    std::cout << "OrderRpl " << countG << std::endl;
    std::cout << "TradeMsg " << countH << std::endl;
    std::cout << "CrxTrade " << countI << std::endl;
    std::cout << "BroknTrd " << countJ << std::endl;

#if BENCH
    auto t2 = high_resolution_clock::now();
    duration<double, std::nano> ms_double = t2 - t1;
    std::cout << "processed " << messageCount << " messages in " << ms_double.count() << " nanoseconds" << std::endl;
#endif
}

