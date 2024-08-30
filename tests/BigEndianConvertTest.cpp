#include "TestBase.h"
#include "Converters/Endian.h"

TEST(BigEndianConvert,TestUint16Convertion)
{
    uint16_t beValue = 0x0102;
    uint16_t leValue = 0x0201;

    ASSERT_TRUE(wal::converters::Endian::fromBig(beValue) == leValue, "failed to convert from big endian");
}


TEST(BigEndianConvert,TestUint32Convertion)
{
    uint32_t beValue = 0x01020304;
    uint32_t leValue = 0x04030201;

    ASSERT_TRUE(wal::converters::Endian::fromBig(beValue) == leValue, "failed to convert from big endian");
}

TEST(BigEndianConvert,TestUint64Convertion)
{
    uint64_t beValue = 0x0102030405060708;
    uint64_t leValue = 0x0807060504030201;

    ASSERT_TRUE(wal::converters::Endian::fromBig(beValue) == leValue, "failed to convert from big endian");
}

TEST(BigEndianConvert,TestFloat64Convertion)
{
    // although double is 64bit it can only hold 53bit range, these values are in that range
    // so we don't get into an overflow,
    double beValue = 0x1.02030401p+32; //0x0102030401000000;
    double leValue = 0x1.04030201p+56; //0x0000000104030201;

    ASSERT_TRUE(wal::converters::Endian::fromBig(beValue) == leValue, "failed to convert from big endian");
}
