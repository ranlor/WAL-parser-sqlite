#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include "Types.h"
#include "RecordHeaderDataType.h"
#pragma once

namespace wal::readers {

    class RecordHeaderReader
    {
        public:
            struct RecordData 
            {
                std::vector< RecordHeaderDataType > headerData;
                uint64_t rowid;
            };

            RecordHeaderReader(const wal::types::BTreeNodePageType& nodeType);
            ~RecordHeaderReader() = default;

            // reading header and populate data in this object
            void read(FixedRuntimeArray<uint8_t>::iterator& dataIt);

            void printOut();

            RecordData headerData() { return {_headerData, _rowid }; }

        private:
            std::vector< RecordHeaderDataType > _headerData;
            wal::types::BTreeNodePageType _nodeType;
            uint64_t _rowid;

            void readHeader(FixedRuntimeArray<uint8_t>::iterator& dataIt, std::vector<types::RecordHeaderFormat> headerFormats);
            types::RecordHeaderFormat recordHeaderByteToType(const uint64_t& headerByte);
    };
}