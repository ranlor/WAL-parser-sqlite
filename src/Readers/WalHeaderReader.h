#include <cstdint>
#include "Converters/Endian.h"
#pragma once

#define PROP(NAME) uint32_t NAME() const\
{\
    static auto tmp = converters::Endian::fromBig(_header.NAME);\
    return tmp;\
}

namespace wal::readers {

    class WalHeaderReader
    {
        public:
            WalHeaderReader():_header({0}) {};
            ~WalHeaderReader() = default;

            operator char*() { return reinterpret_cast<char*>(&_header); }

            std::size_t sizeOf() { return sizeof(_header); }

            PROP(header)
            PROP(version)
            PROP(page_size)
            PROP(sequence)
            PROP(salt1)
            PROP(salt2)
            PROP(checksum1)
            PROP(checksum2)

        private:
            struct WalHeader {
                uint32_t header = 0;
                uint32_t version = 0;
                uint32_t page_size = 0;
                uint32_t sequence = 0;
                uint32_t salt1 = 0;
                uint32_t salt2 = 0;
                uint32_t checksum1 = 0;
                uint32_t checksum2 = 0;
            };

            WalHeader _header;
    };
}
