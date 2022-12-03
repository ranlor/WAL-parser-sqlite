#pragma once
#include <vector>
#include <array>
#include "Types.h"
#include "Utils/FixedRuntimeArray.h"

namespace wal::readers {

    class BTreeReader
    {
        public:
            BTreeReader() = default;
            ~BTreeReader() = default;

            void readHeader(FixedRuntimeArray<uint8_t>::iterator& dataIt);

            bool isInteriorType() const;

            void readPointerArray(FixedRuntimeArray<uint8_t>::iterator& dataIt);

            std::vector<uint16_t> getPointerArray() { return _pointerArray; }

            uint32_t getCellCountOverflow() { return _cellCountStartOverflow; }

            wal::types::BTreeNodePageType getBTreeNodeType() { return _btreeHeader.type; }

        private:

            struct __attribute((packed)) BTreePageHeader {
                wal::types::BTreeNodePageType type;
                uint16_t firstFreeBlockPos = 0;
                uint16_t cellCount = 0;
                uint16_t cellContentStart = 0;
                uint8_t freeByteCount = 0; // fragmented bytes within the cell content area
            };

            BTreePageHeader _btreeHeader;
            std::vector<uint16_t> _pointerArray;
            uint32_t _cellCountStartOverflow = 0;

            void convertToLittleEndian();
    };
}