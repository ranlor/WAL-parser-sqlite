#include "RecordHeaderReader.h"
#include "Converters/VarInt.h"
#include "Utils/Log.h"
#include "Converters/FromData.h"
#include "Converters/Endian.h"

using namespace wal::readers; 
using namespace wal::types;



RecordHeaderReader::RecordHeaderReader(const wal::types::BTreeNodePageType& nodeType):
    _nodeType(nodeType) ,
    _headerData({}),
    _rowid(0)
{}

void RecordHeaderReader::read(FixedRuntimeArray<uint8_t>::iterator& dataIt)
{
    auto payloadSize = converters::VarInt::readVarInt(dataIt);
    wal::Log::get().debug() << "payload size: " <<  payloadSize;

    if (wal::types::BTreeNodePageType::leafTable == _nodeType)
    {
        _rowid = converters::VarInt::readVarInt(dataIt);
        wal::Log::get().debug() << "row id: " <<  _rowid;
    }

    auto startPos = dataIt;

    auto headerSize = converters::VarInt::readVarInt(dataIt); //size in bytes (including the bytes making up headerSize)
    wal::Log::get().debug() << "read record header size : " << headerSize;
    
    if (headerSize < 2) 
    { 
        wal::Log::get().err() << "empty header. skipping";
        return; 
    }
    
    std::vector<types::RecordHeaderFormat> headerFormats;
    while (std::distance(startPos, dataIt) < headerSize)
    {
        uint64_t headerByte = converters::VarInt::readVarInt(dataIt);
        headerFormats.push_back(recordHeaderByteToType(headerByte));
    }
    readHeader(dataIt, headerFormats);
}

void RecordHeaderReader::readHeader(FixedRuntimeArray<uint8_t>::iterator& dataIt, 
                                    std::vector<types::RecordHeaderFormat> headerFormats)
{
    _headerData.clear();
    for (const auto& format : headerFormats)
    {
        if (format.byteSize == 0) 
        {
            _headerData.push_back({ format.type, {}});    
            continue;
        }

        FixedRuntimeArray<uint8_t> data(format.byteSize);
        std::copy_n(dataIt, format.byteSize, data.begin());
        dataIt+=format.byteSize;
        _headerData.push_back({ format.type, data});
    }
}

RecordHeaderFormat RecordHeaderReader::recordHeaderByteToType(const uint64_t& headerByte)
{
    auto serialType = static_cast<RecordSerialTypes>(headerByte);
    wal::Log::get().debug() << "processing record header byte: " << headerByte << " : " << serialType;
    
    if (headerByte > std::to_underlying(RecordSerialTypes::String))
    {
        if (headerByte & 0x00000000000001) // if odd
        {
            return {RecordSerialTypes::String, ((headerByte - 13)/2) };
        }
        else // if even
        {
            return {RecordSerialTypes::Blob, ((headerByte - 12)/2) };
        }
    }
    
    switch (serialType)
    {
        case RecordSerialTypes::Zero: 
        case RecordSerialTypes::One: 
        case RecordSerialTypes::SQLInternal1: 
        case RecordSerialTypes::SQLInternal2: 
        case RecordSerialTypes::Blob: // if we got here bolb uses zero bytes
        case RecordSerialTypes::String:  // if we got here string uses zero bytes
        case RecordSerialTypes::Null: return { serialType, 0 };
        
        case RecordSerialTypes::ByteInt: return { serialType, 1 };
        case RecordSerialTypes::TwoBytesIntBE: return { serialType, 2 };
        case RecordSerialTypes::ThreeBytesIntBE: return { serialType, 3 };
        case RecordSerialTypes::FourBytesIntBE: return { serialType, 4 };
        case RecordSerialTypes::SixBytesIntBE: return { serialType, 6 };
        case RecordSerialTypes::FloatBE:
        case RecordSerialTypes::EightBytesIntBE: return { serialType, 8 };
     
    }
    return {RecordSerialTypes::Bad, 0};
}

void RecordHeaderReader::printOut() 
{
    for (const auto& recordTuple: _headerData)
    {
        switch (recordTuple.type)
        {
            case RecordSerialTypes::Null: 
                wal::Log::get().info() << "NULL: NULL";
            break;
            case RecordSerialTypes::ByteInt: 
            {
                // first you need to cast it to signed byte, and then to an integer so cout prints it as an integer and not a charater
                // since all those std::dec,std::hex etc. are interger operator they won't work on int8_t/uint8_t/char/uchar type >:(
                wal::Log::get().info() << "Int: " << recordTuple.asUInt32() ;
            }
            break;
            case RecordSerialTypes::TwoBytesIntBE: 
                wal::Log::get().info() << "Int[2]: " << recordTuple.asUInt16();
            break;
            case RecordSerialTypes::ThreeBytesIntBE: 
                wal::Log::get().info() << "Int[3]: " << recordTuple.asUInt32();
            break;
            case RecordSerialTypes::FourBytesIntBE:
                wal::Log::get().info() << "Int[4]: " << recordTuple.asUInt32();
            break;
            case RecordSerialTypes::SixBytesIntBE:
                wal::Log::get().info() << "Int[6]: " << recordTuple.asUInt64();
            break;
            case RecordSerialTypes::EightBytesIntBE:
                wal::Log::get().info() << "Int[8]: " << recordTuple.asUInt64();
            break;
            case RecordSerialTypes::FloatBE:
                wal::Log::get().info() << "Float[8]: " << recordTuple.asFloat64();
            break;
            case RecordSerialTypes::Zero: 
                wal::Log::get().info() << "Int: 0";
            break;
            case RecordSerialTypes::One: 
                wal::Log::get().info() << "Int: 1";
            break;
            case RecordSerialTypes::SQLInternal2:
            case RecordSerialTypes::SQLInternal1: 
                wal::Log::get().info() << "SQL Internal";
            break;
            case RecordSerialTypes::Blob: 
                wal::Log::get().info() << "Bolb: ";
                for (const auto& byte : recordTuple.asData()) 
                {
                    printf("0x%X ",byte);
                }
                wal::Log::get().info();
            break;
            case RecordSerialTypes::String: 
                wal::Log::get().info() << "String: " << recordTuple.asString(); // assuming ascii
            break;
        }
    }
}    
