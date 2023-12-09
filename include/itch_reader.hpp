#ifndef ORDER_BOOK_ITCH_READER_HPP
#define ORDER_BOOK_ITCH_READER_HPP

#include <cstdint>
#include <cstddef>

namespace ITCH {

// Reads Nasdaq BinaryFILE and retrieves message data segments
class Reader {
public:
    Reader()                                                    = delete;   // must provide filename
    Reader(char const * _filename);
    Reader(char const * _filename, size_t _bufferSize);

    Reader(const Reader& p)                                     = delete;
    Reader& operator=(const Reader& p)                          = delete;

    ~Reader();

    char const *nextMessage();

private:
    static constexpr size_t defaultBufferSize   = 2048;
    static constexpr size_t messageHeaderLength = 2;
    static constexpr size_t maxITCHMessageSize  = 50;

    int             fdItch;
    size_t          bufferSize;
    char * const    buffer;
    char *         _buffer;
};

} // namespace ITCH

#endif // ORDER_BOOK_ITCH_READER_HPP
