#ifndef ORDER_BOOK_ITCH_PARSER_HPP
#define ORDER_BOOK_ITCH_PARSER_HPP

#include <cstdint>
#include <cstddef>

namespace ITCH {

// Parses Nasdaq BinaryFILE and retrieves message data segments
class Parser {
public:
    Parser()                                                    = delete;   // must provide filename
    Parser(char const * _filename);
    Parser(char const * _filename, size_t _bufferSize);

    Parser(const Parser& p)                                     = delete;
    Parser& operator=(const Parser& p)                          = delete;

    ~Parser();

    void parseITCHFile();

private:
    static constexpr size_t defaultBufferSize = 2048;

    int     fdItch;
    size_t  bufferSize;
    char*   buffer;
};

} // namespace ITCH

#endif // ORDER_BOOK_ITCH_PARSER_HPP
