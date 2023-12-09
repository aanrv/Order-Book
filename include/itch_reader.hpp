#ifndef ORDER_BOOK_ITCH_READER_HPP
#define ORDER_BOOK_ITCH_READER_HPP

#include <cstdint>
#include <cstddef>

namespace ITCH {

// Reads Nasdaq BinaryFILE and retrieves message data segments
class Reader {
public:
    Reader()                                            = delete;   // must provide filename

    Reader(char const * _filename);
    Reader(char const * _filename, size_t _bufferSize);

    Reader(const Reader& p)                             = delete;
    Reader& operator=(const Reader& p)                  = delete;

    ~Reader();

    char const * nextMessage();

private:
    int const       fdItch;
    size_t const    bufferSize;
    char * const    buffer;
    char *          _buffer;
    size_t          validBytes;
};

} // namespace ITCH

#endif // ORDER_BOOK_ITCH_READER_HPP
