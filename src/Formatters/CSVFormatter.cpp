#include "CSVFormatter.h"
#include "Factory.h"
#include "Types.h"
#include "utils/Tokenizers.h"
#include "Utils/Log.h"
#include <sstream>
#include <iomanip>
#include <ranges>

using namespace wal::formatters;

// self register this formatterto the factory - using the static intialization order
Formatter* CSVFormatter::_ref = Factory::instance().registerFormatter(CSVFormatter::id, new CSVFormatter() );

std::string CSVFormatter::generateOutput(const readers::RecordHeaderReader::RecordData& record)
{
    // output data:
    constexpr auto COMMA = ", ";
    size_t columnIndex = 0;
    std::stringstream ss;
    std::string comma = "";


    // parse content:
    if ( didInputChange() )
    {
        _tableColumns.clear();
        parseColumns();

        for (const auto& column: _tableColumns)
        {
            ss << comma;
            ss << column;
            comma = COMMA;
        }

        if (!_tableColumns.empty()) { ss << std::endl; }

        comma = "";
    }

    if (record.headerData.size() != _tableColumns.size() && _strict) 
    { 
        Log::get().err() << "mismatch between given csv columns and data on file: expected " << _tableColumns.size() << " columns got: " << record.headerData.size() << ". skipping...";
        return ss.str();
    }

    for (auto& rec : record.headerData)
    {
        ss << comma ;
        switch (rec.getType())
        {
            case wal::types::RecordSerialTypes::Null:
                ss << "NULL";
            break;
            case wal::types::RecordSerialTypes::Blob:
            {
                auto data = rec.asData();
                if (!data.empty())
                {
                    ss << "0x";
                    for (auto b : data) { ss << std::setfill('0') << std::setw(2) << std::hex << b; }
                }
            }
            break;
            case wal::types::RecordSerialTypes::String:
                ss << "\"" << rec.asString() << "\"";
            break;
            case wal::types::RecordSerialTypes::One:
            case wal::types::RecordSerialTypes::Zero:
            case wal::types::RecordSerialTypes::ByteInt:
            case wal::types::RecordSerialTypes::TwoBytesIntBE:
            case wal::types::RecordSerialTypes::ThreeBytesIntBE:
            case wal::types::RecordSerialTypes::FourBytesIntBE:
                ss << std::dec << rec.asUInt32();
            break;
            case wal::types::RecordSerialTypes::SixBytesIntBE:
            case wal::types::RecordSerialTypes::EightBytesIntBE:
            case wal::types::RecordSerialTypes::FloatBE:
                ss << std::dec << rec.asFloat64();
            break;
            default: 
            {
                std::stringstream ss;
                ss << "unexpected column type on generateOutput: " << rec.getType();
                throw CSVFormatterException(ss.str(), CSVFormatterException::ErrorCode::UnexpectedColumnType);
            }
        }
        comma = COMMA;
        ++columnIndex;
    }
    return ss.str();
}


void CSVFormatter::parseColumns()
{
    auto input = getInput()->getInputData();
    const std::string delim = ",";
    tokenizers::split(input, delim, [this](const std::string& part){
        _tableColumns.emplace_back(part);
        return true;
    });
}