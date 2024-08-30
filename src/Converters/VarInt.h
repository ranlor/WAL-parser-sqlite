#include <cstdint>
#include <fstream>
#include <vector>
#pragma once

namespace wal::converters {

    struct VarInt
    {
        public:

            static inline uint64_t readVarIntSql4(FixedRuntimeArray<uint8_t>::iterator& dataIt)
            {
                auto localIt = dataIt;

                if (localIt[0] >= 0 && localIt[0] <= 240)
                {
                    ++dataIt;
                    return localIt[0];
                }

                if (localIt[0] >= 241 && localIt[0] <= 248)
                {
                    dataIt+=2;
                    return 240+256*(localIt[0]-241)+localIt[1];
                }

                if (localIt[0] == 249)
                {
                    dataIt+=3;
                    return 2288+256*localIt[1]+localIt[2];
                }

                uint64_t v = 0;

                if (localIt[0] == 255) { v = (v | localIt[8]) << 8; ++dataIt;}
                if (localIt[0] >= 254) { v = (v | localIt[7]) << 8; ++dataIt;}
                if (localIt[0] >= 253) { v = (v | localIt[6]) << 8; ++dataIt;}
                if (localIt[0] >= 252) { v = (v | localIt[5]) << 8; ++dataIt;}
                if (localIt[0] >= 251) { v = (v | localIt[4]) << 8; ++dataIt;}

                if (localIt[0] >= 250)
                {
                    v = (v | localIt[3]) << 8;
                    v = (v | localIt[2]) << 8;
                    v = (v | localIt[1]);
                    dataIt+=4;
                }

                return v;
            }

            static inline uint64_t readVarInt(FixedRuntimeArray<uint8_t>::iterator& dataIt)
            {
                constexpr uint8_t lastSevenBits = 0x7f; //127 = 01111111
                constexpr int maxSize = 9;

                auto localIt = dataIt;
                ++dataIt;
                uint64_t v = 0;
                for (int i=0; i<maxSize-1; ++i)
                {
                    if (localIt[i] <= lastSevenBits)
                    {
                        v |= localIt[i];
                        return v;
                    }

                    v |= (localIt[i] & lastSevenBits);
                    v = v << 7;

                    ++dataIt;
                }

                // process 9-th byte
                v = v << 8;
                v |= localIt[8];

                ++dataIt;

                return v;
            }
    };

}
