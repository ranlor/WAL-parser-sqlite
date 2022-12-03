#include <cstdint>
#include <array>
#pragma once

namespace wal::converters {

    struct Endian
    {
        protected:
            template<typename TYPE, std::array<int,sizeof(TYPE)> MV_ARRAY>
            static inline TYPE fromBig(const TYPE& value)
            {
                static_assert(std::is_integral_v<TYPE>, "converting type must be integral type");

                TYPE ret = 0;
                TYPE mask = ret | 0xFF;
                for (int i=0; i<MV_ARRAY.size(); ++i) 
                {
                    ret |= (MV_ARRAY[i] > 0 ? (mask & value) << MV_ARRAY[i] : (mask & value) >> (MV_ARRAY[i]*-1));
                    mask = mask << 8;
                }
                return ret;
            }
        public:
            static inline double fromBig(const double& value)
            {
                auto i = static_cast<uint64_t>(value);
                return static_cast<double>(fromBig<uint64_t, {56,40,24,8,-8,-24,-40,-56} >(i));
            }

            static inline uint64_t fromBig(const uint64_t& value)
            {
                return fromBig<uint64_t, {56,40,24,8,-8,-24,-40,-56} >(value);
            }

            static inline uint32_t fromBig(const uint32_t& value)
            {
                return fromBig<uint32_t, {24,8,-8,-24} >(value);
            }

            static inline uint16_t fromBig(const uint16_t& value)
            {
                return fromBig<uint16_t, {8,-8} >(value);
            }
    };
}