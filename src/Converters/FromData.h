#include <cstdint>
#include <stdexcept>
#include "Utils/FixedRuntimeArray.h"
#pragma once

namespace wal::converters {
    // to keep inline with intergral types. 
    // c++23 should have std::float*_t
    using float64_t = double;
    
    struct FromData 
    {
        private:
            template <size_t N>
            struct StringLiteral
            {
                constexpr StringLiteral(const char (& str)[N]) {
                    std::copy_n(str, N, value);
                }
                char value[N] = {0};
            };

            template<size_t SIZE, typename RET, StringLiteral TYPE_STR>
            static inline RET toThis(const FixedRuntimeArray<uint8_t>& data)
            {
                static_assert(std::is_integral_v<RET>, "return time must be integral type");
                if (data.size() < SIZE ) 
                { 
                    std::string message = "Unable to convert data vector to ";
                    message += TYPE_STR.value;
                    message += " from " + std::to_string(data.size()) + " bytes, not enough data";
                    throw std::runtime_error(message); 
                }

                RET v = 0;
                for (int i=0; i < SIZE; ++i)
                {
                    RET c = data[i];
                    v |= c << i*8;
                }
                return v;

            }

        public:
            static inline uint16_t toUInt16(const FixedRuntimeArray<uint8_t>& data)
            {
                return toThis<sizeof(uint16_t),uint16_t, "uint16_t">(data);
            }

            static inline uint32_t toUInt32(const FixedRuntimeArray<uint8_t>& data)
            {
                return toThis<sizeof(uint32_t),uint32_t, "uint32_t">(data);
            }

            static inline uint32_t toUInt32FromThreeBytes(const FixedRuntimeArray<uint8_t>& data)
            {
                return toThis<3 ,uint32_t, "uint32_t">(data);
            }

            static inline uint64_t toUInt64(const FixedRuntimeArray<uint8_t>& data)
            {
                return toThis<sizeof(uint64_t),uint64_t, "uint64_t">(data);
            }

            static inline uint64_t toUInt64FromSixBytes(const FixedRuntimeArray<uint8_t>& data)
            {
                return toThis<6,uint64_t, "uint64_t">(data);
            }

            static inline float64_t toFloat64(const FixedRuntimeArray<uint8_t>& data)
            {
                return static_cast<float64_t>(toUInt64(data));
            }

            template<typename TYPE, int COUNT>
            static inline TYPE fromIteratorToType(FixedRuntimeArray<uint8_t>::iterator& it)
            {
                FixedRuntimeArray<uint8_t> data(it,COUNT);
                if constexpr (std::is_same_v<TYPE,uint16_t>) { return toUInt16(data); }
                if constexpr (std::is_same_v<TYPE,uint32_t>) { return toUInt32(data); }
                if constexpr (std::is_same_v<TYPE,uint64_t> && COUNT == 3)  { return toUInt32FromThreeBytes(data); }
                if constexpr (std::is_same_v<TYPE,uint64_t>) { return toUInt64(data); }
                if constexpr (std::is_same_v<TYPE,uint64_t> && COUNT == 6)  { return toUInt64FromSixBytes(data); }
            }
    };
}