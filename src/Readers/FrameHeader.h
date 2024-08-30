#include <cstdint>
#include "Converters/Endian.h"
#pragma once

#define PROP(NAME) uint32_t NAME() const\
{\
    static auto tmp = converters::Endian::fromBig(_header.NAME);\
    return tmp;\
}

namespace wal::readers {

    class FrameHeader
    {
        public:
            FrameHeader():_header({0}) {};
            ~FrameHeader() = default;

            operator char*() { return reinterpret_cast<char*>(&_header); }

            std::size_t sizeOf() { return sizeof(_header); }

            PROP(pageNumber)
            PROP(sizeInPage)
            PROP(salt1)
            PROP(salt2)
            PROP(checksum1)
            PROP(checksum2)

        private:
            struct WalFrameHeader {
                uint32_t pageNumber = 0;
                uint32_t sizeInPage = 0;
                uint32_t salt1 = 0;
                uint32_t salt2 = 0;
                uint32_t checksum1 = 0;
                uint32_t checksum2 = 0;
            };

            WalFrameHeader _header;
    };
}
