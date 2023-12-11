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
        char messageType = messageData[ITCH::messageTypeIndex];
        messageData += 2;
        switch (messageType) {
            case 'A': {
                cout << '\n';
                ITCH::AddOrderMessage m = ITCH::Parser::createAddOrderMessage(messageData);
                cout << m.timestamp << endl;
                break;
                      }
            default:
                cout << messageType;
                break;
        };

    }

#if BENCH
    auto t2 = high_resolution_clock::now();
    duration<double, std::nano> ms_double = t2 - t1;
    std::cout << "processed " << messageCount << " messages in " << ms_double.count() << " nanoseconds" << std::endl;
#endif

}

