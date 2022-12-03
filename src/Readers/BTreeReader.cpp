#include "BTreeReader.h"
#include "Converters/Endian.h"
#include "Converters/FromData.h"

using namespace wal::readers;

void BTreeReader::readHeader(FixedRuntimeArray<uint8_t>::iterator& dataIt) 
{
    std::array<uint8_t, sizeof(_btreeHeader)> buffer;
    std::copy(dataIt, dataIt+sizeof( _btreeHeader ), buffer.begin());
    dataIt+=sizeof( _btreeHeader ); // advance pointer
    _btreeHeader = *(reinterpret_cast<BTreePageHeader*>(buffer.data()));
    convertToLittleEndian();
    if (_btreeHeader.cellContentStart == 0) { _cellCountStartOverflow =  0xFFFF+1; }
}


bool BTreeReader::isInteriorType() const
{
    return _btreeHeader.type == wal::types::BTreeNodePageType::interiorIndex ||
           _btreeHeader.type == wal::types::BTreeNodePageType::interiorTable;
}


void BTreeReader::convertToLittleEndian()
{
    _btreeHeader.firstFreeBlockPos  = wal::converters::Endian::fromBig(_btreeHeader.firstFreeBlockPos);
    _btreeHeader.cellCount  = wal::converters::Endian::fromBig(_btreeHeader.cellCount);
    _btreeHeader.cellContentStart  = wal::converters::Endian::fromBig(_btreeHeader.cellContentStart);
}

void BTreeReader::readPointerArray(FixedRuntimeArray<uint8_t>::iterator& dataIt)
{
    using namespace wal::converters;
    if (_btreeHeader.cellCount == 0 ) {return;}
    _pointerArray.reserve(_btreeHeader.cellCount);
    for (int i=0; i<_btreeHeader.cellCount; ++i)
    {
        _pointerArray.emplace_back(
            Endian::fromBig(
                FromData::fromIteratorToType<uint16_t,sizeof(uint16_t)>(dataIt)
            )
        );
    }
}

