#include <cstdint>
#include <iostream>

#pragma once

#ifndef TYPES_H_
#define TYPES_H_

namespace wal::types {

    enum class RecordSerialTypes : uint64_t {
        Null = 0,
        ByteInt = 1,
        TwoBytesIntBE = 2,
        ThreeBytesIntBE = 3,
        FourBytesIntBE = 4,
        SixBytesIntBE = 5,
        EightBytesIntBE = 6,
        FloatBE = 7,
        Zero = 8,
        One = 9,
        SQLInternal1 = 10,
        SQLInternal2 = 11,
        Blob = 12,
        String = 13,
        Bad = 14
    };

    inline std::ostream& operator<<(std::ostream& os, const RecordSerialTypes& type)
    {
        switch (type)
        {
            case RecordSerialTypes::Null: os << "Null"; return os;
            case RecordSerialTypes::ByteInt: os << "ByteInt"; return os;
            case RecordSerialTypes::TwoBytesIntBE: os << "TwoBytesIntBE"; return os;
            case RecordSerialTypes::ThreeBytesIntBE: os << "ThreeBytesIntBE"; return os;
            case RecordSerialTypes::FourBytesIntBE: os << "FourBytesIntBE"; return os;
            case RecordSerialTypes::SixBytesIntBE: os << "SixBytesIntBE"; return os;
            case RecordSerialTypes::EightBytesIntBE: os << "EightBytesIntBE"; return os;
            case RecordSerialTypes::FloatBE: os << "FloatBE"; return os;
            case RecordSerialTypes::Zero: os << "Zero"; return os;
            case RecordSerialTypes::One: os << "One"; return os;
            case RecordSerialTypes::SQLInternal1: os << "SQLInternal1"; return os;
            case RecordSerialTypes::SQLInternal2: os << "SQLInternal2"; return os;
            case RecordSerialTypes::Blob: os << "Blob"; return os;
            case RecordSerialTypes::String: os << "String"; return os;
            case RecordSerialTypes::Bad: os << "Bad"; return os;
        }
        return os;
    }

    struct RecordHeaderFormat 
    {
        RecordSerialTypes type;
        uint64_t byteSize;
    };


    enum class BTreeNodePageType : uint8_t {
        leafTable = 0x0d,
        leafIndex = 0x0a,
        interiorTable = 0x05,
        interiorIndex = 0x02,
        uknown = 0
    };

    inline std::ostream& operator<<(std::ostream& os, const BTreeNodePageType& node)
    {
        switch (node)
        {
            case BTreeNodePageType::leafTable: os << "Leaf Table [0x0D]"; return os;
            case BTreeNodePageType::leafIndex: os << "Leaf Index [0x0A]"; return os;
            case BTreeNodePageType::interiorTable: os << "Interior Table [0x05]"; return os;
            case BTreeNodePageType::interiorIndex: os << "Interior Index [0x02]"; return os;
            case BTreeNodePageType::uknown: os << "Uknown value [0x00]"; return os;
        }
        return os;
    }
}

#endif