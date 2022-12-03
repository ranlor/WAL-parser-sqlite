#include "RecordHeaderDataType.h"
#include "Converters/FromData.h"
#include "Converters/Endian.h"

using namespace wal::readers;
            
RecordHeaderDataType::RecordHeaderDataType(const types::RecordSerialTypes& type, 
                                           const FixedRuntimeArray<uint8_t>& data):
    type(type),
    data(data)
{}

std::string RecordHeaderDataType::asString() const
{
    if (types::RecordSerialTypes::String != type) { return {}; } 
    std::string ret;
    ret.resize(data.size());
    std::memcpy(ret.data(),(const void *) data.data(), data.size());
    return ret;
}

std::vector<uint8_t> RecordHeaderDataType::asData() const
{
    if (types::RecordSerialTypes::Blob != type) { return {}; } 
    std::vector<uint8_t> ret;
    ret.reserve(data.size());
    for (auto& b: data) { ret.push_back(b); }
    return ret;
}

uint8_t RecordHeaderDataType::asUInt8() const
{
    if (types::RecordSerialTypes::One == type) { return 1; } 

    if (types::RecordSerialTypes::ByteInt != type) { return 0; } 
    return data[0];
}

uint16_t RecordHeaderDataType::asUInt16() const
{
    if (types::RecordSerialTypes::TwoBytesIntBE == type) 
    {
        return converters::Endian::fromBig(converters::FromData::toUInt16(data));
    }
    return static_cast<uint16_t>(asUInt8());
}

uint32_t RecordHeaderDataType::asUInt32() const
{
    if (types::RecordSerialTypes::ThreeBytesIntBE == type) 
    {
        return converters::Endian::fromBig(converters::FromData::toUInt32FromThreeBytes(data));
    }
    if (types::RecordSerialTypes::FourBytesIntBE == type) 
    {
        return converters::Endian::fromBig(converters::FromData::toUInt32(data));
    }
    return static_cast<uint32_t>(asUInt16());
}

uint64_t RecordHeaderDataType::asUInt64() const
{
    if (types::RecordSerialTypes::SixBytesIntBE == type) 
    {
        return converters::Endian::fromBig(converters::FromData::toUInt64FromSixBytes(data));
    }
    if (types::RecordSerialTypes::EightBytesIntBE == type) 
    {
        return converters::Endian::fromBig(converters::FromData::toUInt64(data));
    }
    return static_cast<uint64_t>(asUInt32());
}

wal::converters::float64_t RecordHeaderDataType::asFloat64() const
{
    if (types::RecordSerialTypes::FloatBE == type)
    {
        return converters::Endian::fromBig(converters::FromData::toFloat64(data));
    }
    return static_cast<converters::float64_t>(asUInt64());
}

