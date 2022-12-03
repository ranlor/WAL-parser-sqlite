#include "TestBase.h"
#include "Utils/FixedRuntimeArray.h"
#include "Readers/RecordHeaderReader.h"
#include "Converters/FromData.h"
#include "Converters/Endian.h"

TEST(RecordHeaderReaderTests, ReadHeaderTwo16UIntsLeftIndexNode)
{
    wal::FixedRuntimeArray<uint8_t> data = {
        0x07, // total payload size
        0x03, // header total size 
        (uint8_t) wal::types::RecordSerialTypes::TwoBytesIntBE, 
        (uint8_t)wal::types::RecordSerialTypes::TwoBytesIntBE, 
        0x01, 0x02,
        0x03, 0x04};

    const std::vector< wal::readers::RecordHeaderDataType > expected = {
        {wal::types::RecordSerialTypes::TwoBytesIntBE, {0x02,0x01} },
        {wal::types::RecordSerialTypes::TwoBytesIntBE, {0x04,0x03} }
    };

    wal::readers::RecordHeaderReader read(wal::types::BTreeNodePageType::leafIndex);
    wal::Log::get().setLogLevel(wal::Log::ReportLevel::None);
    auto it = data.begin();
    read.read(it);
    auto result = read.headerData();
    ASSERT_TRUE(result.headerData.size() == expected.size(), "result size not matching expcted size");
    for (int i=0; i<expected.size(); ++i)
    {
        ASSERT_TRUE(result.headerData[i].getType() == expected[i].getType(), "result type mismatch expected type");
        
        auto uint16value = wal::converters::FromData::toUInt16(expected[i].asRawData());
        ASSERT_TRUE(result.headerData[i].asUInt16() == uint16value, "result value mismatch expected value");
    }

}


TEST(RecordHeaderReaderTests, ReadHeaderTwo16UIntsLeftTableNode)
{
    wal::FixedRuntimeArray<uint8_t> data = {
        0x08, // total payload size
        0x02, // row id
        0x03, // header total size 
        (uint8_t) wal::types::RecordSerialTypes::TwoBytesIntBE, 
        (uint8_t)wal::types::RecordSerialTypes::TwoBytesIntBE, 
        0x01, 0x02,
        0x03, 0x04};

    const std::vector< wal::readers::RecordHeaderDataType > expected = {
        {wal::types::RecordSerialTypes::TwoBytesIntBE, {0x02,0x01} },
        {wal::types::RecordSerialTypes::TwoBytesIntBE, {0x04,0x03} }
    };

    wal::readers::RecordHeaderReader read(wal::types::BTreeNodePageType::leafTable);
    wal::Log::get().setLogLevel(wal::Log::ReportLevel::None);
    auto it = data.begin();
    read.read(it);
    auto result = read.headerData();
    ASSERT_TRUE(result.headerData.size() == expected.size(), "result size not matching expcted size");
    for (int i=0; i<expected.size(); ++i)
    {
        ASSERT_TRUE(result.headerData[i].getType() == expected[i].getType(), "result type mismatch expected type");
        
        auto uint16value = wal::converters::FromData::toUInt16(expected[i].asRawData());
        ASSERT_TRUE(result.headerData[i].asUInt16() == uint16value, "result value mismatch expected value");
    }

}