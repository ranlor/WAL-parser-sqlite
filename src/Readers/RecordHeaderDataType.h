#include <cstdint>
#include <vector>
#include "Types.h"
#include "Utils/FixedRuntimeArray.h"
#include "Converters/FromData.h"
#pragma once

namespace wal::readers {

    class RecordHeaderDataType 
    {
        friend class RecordHeaderReader;
        public:
            
            RecordHeaderDataType(const types::RecordSerialTypes& type, 
                                    const FixedRuntimeArray<uint8_t>& data);

            types::RecordSerialTypes getType() const { return type; }

            std::string asString() const;

            std::vector<uint8_t> asData() const;

            bool isNull() const { return types::RecordSerialTypes::Null == type; }

            uint8_t asUInt8() const;
            
            uint16_t asUInt16() const;

            uint32_t asUInt32() const;

            uint64_t asUInt64() const;

            converters::float64_t asFloat64() const;

            // return the data as is without any type checks
            const FixedRuntimeArray<uint8_t>& asRawData() const { return data; }

        private:
            FixedRuntimeArray<uint8_t> data;
            types::RecordSerialTypes type;
    };
}