#include "TestBase.h"
#include "Formatters/Factory.h"
#include "Formatters/SchemaFormatter.h"
#include "Formatters/Input/StringInput.h"
#include "Readers/RecordHeaderReader.h"

using namespace wal::types;
using formatException = wal::formatters::Formatter::FormatterException;
using schemaCode = wal::formatters::SchemaFormatter::SchemaFormatterException::ErrorCode;

TEST(SchemaFormatterTests,SimpleCreateStatement)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} }
       },
       0 //rowid
    };

    constexpr auto expectedOutput = "INSERT INTO foo (col1) VALUES (1);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);

    // make sure that we get the same output again:
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}

TEST(SchemaFormatterTests,MultipleSimpleCreateStatement)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER);"
    "CREATE TABLE foo1 (other_col1 INTEGER, other_col2 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} },
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       0 //rowid
    };
    // currently we only process the first statment
    constexpr auto expectedOutput = "INSERT INTO foo (col1, col2) VALUES (1, 2);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}

TEST(SchemaFormatterTests,TableWithBlob)
{
    constexpr auto sql = "CREATE TABLE foo (col1 BLOB);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Blob, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x56} }
       },
       0 //rowid
    };
    // currently we only process the first statment
    constexpr auto expectedOutput = "INSERT INTO foo (col1) VALUES (0x0A0A0A0A0A0A0A0A0A0A0A0A0A0A0V);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}

TEST(SchemaFormatterTests,FailOnMissingTableName)
{
    constexpr auto sql = "CREATE TABLE (col1 INTEGER, col2 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} },
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       0 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ( std::to_underlying(schemaCode::MalformedTableName), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,PrimaryKeyPattern1)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER PRIMARY KEY, col2 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Null, {}},
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       44 //rowid
    };

    constexpr auto expectedOutput = "INSERT INTO foo (col1, col2) VALUES (44, 2);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}


TEST(SchemaFormatterTests,PrimaryKeyPattern2)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER, PRIMARY KEY(col1 ASC));";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Null, {}},
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       46 //rowid
    };

    constexpr auto expectedOutput = "INSERT INTO foo (col1, col2) VALUES (46, 2);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}


TEST(SchemaFormatterTests,PrimaryKeyPattern1NotFirst)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER PRIMARY KEY);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} },
        {RecordSerialTypes::Null, {}}
       },
       45 //rowid
    };

    constexpr auto expectedOutput = "INSERT INTO foo (col1, col2) VALUES (2, 45);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}


TEST(SchemaFormatterTests,PrimaryKeyPattern2NotFirst)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER, PRIMARY KEY(col2 ASC));";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} },
        {RecordSerialTypes::Null, {}}
       },
       47 //rowid
    };

    constexpr auto expectedOutput = "INSERT INTO foo (col1, col2) VALUES (2, 47);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}



TEST(SchemaFormatterTests,PrimaryKeyWrongPlacement)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER, PRIMARY KEY(col1 ASC));";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} },
        {RecordSerialTypes::Null, {}}
       },
       47 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::InvalidPrimaryKeyPosition), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,PrimaryKeyPattern1Invalid)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER PRIMARY BOO, col2 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Null, {}},
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       46 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::MalformedPrimaryKey), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,PrimaryKeyPattern1InvalidAlternate)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTER PRIMARY KEY, col2 INTEGER);";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Null, {}},
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       46 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::MalformedPrimaryKey), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,PrimaryKeyPattern2Invalid)
{
    constexpr auto sql = "CREATE TABLE foo (col1 INTEGER, col2 INTEGER, PRIMARY KEY col1 );";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::Null, {}},
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       46 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::MalformedPrimaryKey), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}


TEST(SchemaFormatterTests,MalformedTableParentesis)
{
    constexpr auto sql = "CREATE TABLE foo ();";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} },
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       0 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::MalformedParentheses), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,MalformedTableParentesis2)
{
    constexpr auto sql = "CREATE TABLE foo (a )INT());";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} },
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }
       },
       0 //rowid
    };

    try { formatter->generateOutput(data); }
    catch (formatException& e)
    {
        ASSERT_EQ(std::to_underlying(schemaCode::MalformedParentheses), e.code());
        return;
    }
    FAILURE("generation passed when it should have failed");
}

TEST(SchemaFormatterTests,allTypes)
{
    constexpr auto sql = "CREATE TABLE foo (a INTEGER,"
    "b INT,"
    "c TINYINT,"
    "d SMALLINT,"
    "e MEDIUMINT,"
    "f BIGINT,"
    "g UNSIGNED BIG INT,"
    "h INT2,"
    "j INT8,"
    "k CHARACTER(20),"
    "l VARCHAR(255),"
    "m VARYING CHARACTER(255),"
    "n NCHAR(55),"
    "o NATIVE CHARACTER(70),"
    "p NVARCHAR(100),"
    "q TEXT,"
    "r CLOB,"
    "s BLOB,"
    "t REAL,"
    "u DOUBLE,"
    "v DOUBLE PRECISION,"
    "w FLOAT,"
    "x NUMERIC,"
    "y DECIMAL(10,5),"
    "z BOOLEAN,"
    "aa DATE,"
    "ab DATETIME"
    ");";
    auto formatter = wal::formatters::Factory::instance().getFormatter(wal::formatters::SchemaFormatter::id);
    formatter->setInput(std::make_unique<wal::formatters::inputs::StringInput>(sql));

    wal::readers::RecordHeaderReader::RecordData data = {
       {
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x01} }, // INTEGER
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x02} }, // INT
        {RecordSerialTypes::ByteInt, {0x03} }, // TINYINT
        {RecordSerialTypes::TwoBytesIntBE, {0x00,0x04} }, // SMALLINT
        {RecordSerialTypes::ThreeBytesIntBE, {0x00,0x00,0x05} }, // MEDIUM INT
        {RecordSerialTypes::SixBytesIntBE, {0x00,0x00,0x00,0x00,0x00,0x06} }, // BIG INT
        {RecordSerialTypes::SixBytesIntBE, {0x00,0x00,0x00,0x00,0x00,0x06} }, // UBIG INT
        {RecordSerialTypes::TwoBytesIntBE, {0x00,0x07} },  // INT2
        {RecordSerialTypes::EightBytesIntBE, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08} },  // INT8
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x49} }, //CHARACTER
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x50} }, //VCHARACTER
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x51} }, //VARING CHARACTER
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x52} }, //NCHAR
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x53} }, //NATIVE CHAR
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x54} }, //NATIVE VAR CHAR
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x55} }, //TEXT
        {RecordSerialTypes::String, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x55} }, //CLOB
        {RecordSerialTypes::Blob, {0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x56} }, //BLOB
        {RecordSerialTypes::FloatBE, {0x01,0x02,0x03,0x04,0x01,0x00,0x00,0x00} }, // REAL
        {RecordSerialTypes::FloatBE, {0x02,0x02,0x03,0x04,0x01,0x00,0x00,0x00} }, // DOUBLE
        {RecordSerialTypes::FloatBE, {0x03,0x02,0x03,0x04,0x01,0x00,0x00,0x00} }, // DOUBLE PRECISION
        {RecordSerialTypes::FloatBE, {0x04,0x02,0x03,0x04,0x01,0x00,0x00,0x00} }, // FLOAT
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x10} }, // NUMERIC
        {RecordSerialTypes::FloatBE, {0x06,0x02,0x03,0x04,0x01,0x00,0x00,0x00} }, // DECIMAL
        {RecordSerialTypes::One, {} }, // BOOLEAN
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x11} }, // DATE
        {RecordSerialTypes::FourBytesIntBE, {0x00,0x00,0x00,0x12} } // DATETIME
       },
       0 //rowid
    };

    // currently we only process the first statment
    constexpr auto expectedOutput = "INSERT INTO foo "
    "(a, b, c, d, e, f, g, h, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, aa, ab) "
    "VALUES "
    "(1, 2, 3, 4, 1280, 393216, 393216, 7, 8, \"AAAAAAAAAAAAAAI\", \"AAAAAAAAAAAAAAP\", \"AAAAAAAAAAAAAAQ\", \"AAAAAAAAAAAAAAR\", \"AAAAAAAAAAAAAAS\", \"AAAAAAAAAAAAAAT\", \"AAAAAAAAAAAAAAU\", \"AAAAAAAAAAAAAAU\", 0x0A0A0A0A0A0A0A0A0A0A0A0A0A0A0V, 7.26239e+16, 1.44681e+17, 2.16739e+17, 2.88797e+17, 16, 4.32912e+17, 1, 17, 18);";
    ASSERT_EQ(formatter->generateOutput(data), expectedOutput);
}
