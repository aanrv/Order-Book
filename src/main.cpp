#include "itch_common.hpp"
#include "itch_reader.hpp"
#include "order_book.hpp"
#include <iostream>
#include <vector>
#include <boost/pool/object_pool.hpp>
#include <sparsehash/dense_hash_map>

#define BENCH false

#if BENCH
#include <chrono>
#endif

void showBooks(google::dense_hash_map<uint16_t, OrderBook*> const);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " itch_filename [snapshot_timestamp...]" << std::endl;
        std::cout << "\n" << "where" << '\n'
            << "\t" << "itch_filename: a NasdaqTotalViewITCH file in BinaryFILE format" << '\n'
            << "\t" << "snapshot_timestamp: time, in nanoseconds since midnight, at which to print a snapshot of the order book"
            << std::endl;
        return EXIT_FAILURE;
    }

#if BENCH
    std::cout << "Processing " << argv[1] << std::endl;
#endif

#if !BENCH
    // store timestamp args in order
    std::vector<ITCH::Timestamp_t> timestamps;
    for (int i = 2; i < argc; ++i) {
        timestamps.push_back(ITCH::Parser::strToTimestamp(argv[i]));
    }
    sort(timestamps.begin(),
            timestamps.end(),
            [](ITCH::Timestamp_t a, ITCH::Timestamp_t b) { return b < a; });
#endif

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
    using std::chrono::milliseconds;
    long long messageCount = 0;
    auto t1 = high_resolution_clock::now();
#endif

    while((messageData = reader.nextMessage())) {
#if !BENCH
        ITCH::Timestamp_t messageTimestamp = ITCH::Parser::getDataTimestamp(messageData);
        if (!timestamps.empty() && messageTimestamp > timestamps.back()) {
            std::cout << "timestamp " << timestamps.back() << std::endl;
            showBooks(books);
            timestamps.pop_back();
        }
#endif

        ITCH::MessageType_t messageType = ITCH::Parser::getDataMessageType(messageData);
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
            [[unlikely]] case ITCH::AddOrderMPIDAttributionMessageType: {
                ITCH::AddOrderMPIDAttributionMessage m = ITCH::Parser::createAddOrderMPIDAttributionMessage(messageData);
                if (!books.count(m.stockLocate)) {
                    OrderBook * const newBook = booksmem.construct();
                    books.insert(std::pair(m.stockLocate, newBook));
                }
                books[m.stockLocate]->handleAddOrderMPIDAttributionMessage(m);
                break;
            }
            case ITCH::OrderCancelMessageType: {
                ITCH::OrderCancelMessage m = ITCH::Parser::createOrderCancelMessage(messageData);
                books[m.stockLocate]->handleOrderCancelMessage(m);
                if (!books[m.stockLocate]->orderCount()) {
                    OrderBook * const removeBook = books.find(m.stockLocate)->second;
                    books.erase(m.stockLocate);
                    booksmem.destroy(removeBook);
                }
                break;
            }
            [[likely]] case ITCH::OrderDeleteMessageType: {
                ITCH::OrderDeleteMessage m = ITCH::Parser::createOrderDeleteMessage(messageData);
                books[m.stockLocate]->handleOrderDeleteMessage(m);
                if (!books[m.stockLocate]->orderCount()) {
                    OrderBook * const removeBook = books.find(m.stockLocate)->second;
                    books.erase(m.stockLocate);
                    booksmem.destroy(removeBook);
                }
                break;
            }
            case ITCH::OrderReplaceMessageType: {
                ITCH::OrderReplaceMessage m = ITCH::Parser::createOrderReplaceMessage(messageData);
                books[m.stockLocate]->handleOrderReplaceMessage(m);
                break;
            }
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
    std::cout << "processed " << messageCount << " messages (" << reader.getTotalBytesRead()  << " bytes) in " << duration_cast<milliseconds>(t2 - t1).count() << " milliseconds" << std::endl;
#endif
#if !BENCH
    std::cout << "timestamp " << "eod" << std::endl;
    showBooks(books);
#endif
}

void showBooks(google::dense_hash_map<uint16_t, OrderBook*> const books) {
    const char * header = "address,referenceNumber,stockLocate,timestamp,side,shares,price,previous,next";
    std::cout << header << std::endl;
    for (const auto& [symbol, book] : books) {
        std::cout << *book << std::endl;
    }
}

