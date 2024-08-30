#include "SchemaFormatter.h"
#include <fstream>
#include <streambuf>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <algorithm>
#include <ranges>
#include "Factory.h"
#include "Types.h"
#include "Utils/Log.h"
#include "utils/Tokenizers.h"

using namespace wal::formatters;
using errorCode = SchemaFormatter::SchemaFormatterException::ErrorCode;
namespace {


    void removeWhitespace(std::string& text)
    {
        std::erase_if(text, [](auto c){ return std::isspace(c) && !(c == ' ');});
    }

    SchemaFormatter::Range<std::string> getParenthesesRange(const SchemaFormatter::Range<const std::string>& range)
    {
        auto start = std::find(range.start, range.stop, '(');
        if (start == range.stop) { return {range.stop, range.stop}; }
        auto stop = range.stop;
        int counter = 1;
        auto it = start;
        it++;
        for (auto i=it; i != range.stop; ++i)
        {
            if (*i == ')') { counter--; }
            if (*i == '(') { counter++; }
            if (counter == 0)
            {
                stop = i;
                break;
            }
        }
        return {start, stop};
    }

    std::pair<std::string,std::string::const_iterator>
    extractColnameFrom(const SchemaFormatter::Range<const std::string>& range)
    {
        static auto isNameChar = [](char c) -> bool { return std::isalnum(c) || c == '_' || c == '-' || c == '.'; };
        bool endToken = std::isspace(*range.start);
        bool isReverse = std::distance(range.start, range.stop) < 0;
        std::string name;
        auto cIt = range.start;
        for( ; cIt != range.stop; (isReverse ? --cIt : ++cIt) )
        {
            if ( std::isspace(*cIt) && endToken ) { continue; }
            if ( isNameChar(*cIt) )
            {
                endToken = false;
                name+=*cIt;
                continue;
            }
            if (!std::isspace(*cIt)) { name.clear(); }
            break;
        }

        if (name.empty()) { return {name, cIt}; }

        if (isReverse)
        {
            std::reverse(name.begin(), name.end());
            cIt-=2;
        }

        return {name, ++cIt};
    }

    /**
     * @brief get the starting position of 'token' after 'offset'
     * while only ignoring white space, this means that if this gets a non whitespace character
     * and it's not the token it will return as not found
     * @param token combination of chars to look for
     * @param range range to look for the token in
     * @return std::string::const_iterator position in the end of the token or the end of the string if not found
     */
    std::string::const_iterator tokenPos(std::string_view token,const SchemaFormatter::Range<const std::string>& range)
    {
        const auto dist = std::distance(range.start,range.stop);
        if (dist >= 0)
        {
            return tokenizers::tokenPos(token.begin(), token.end(), range.start, range.stop);
        }
        else
        {
            auto rstart = std::make_reverse_iterator(range.start);
            auto rstop = std::make_reverse_iterator(range.stop);
            return tokenizers::tokenPos(token.rbegin(), token.rend(), rstart, rstop).base();
        }
    }

    std::string::const_iterator tokensPos(const std::vector<std::string>& tokens, const SchemaFormatter::Range<const std::string>& range)
    {
        auto it = range.start;
        for (auto& token : tokens)
        {
            it = tokenPos(token, {it, range.stop});
            if (std::distance(it, range.stop) <= 0) { return range.stop; }
        }
        return it;
    }

    void clearAllParentheses(std::string& buffer)
    {
        int parenthesesCount = 0;
        auto i=buffer.begin();
        while( i!=buffer.end() )
        {
            if (*i == '(') { parenthesesCount++; }
            if (parenthesesCount == 0) {++i;}
            else
            {
                if (*i == ')')  {parenthesesCount--; }
                i = buffer.erase(i);
            }
        }
    }


}

// self register this formatterto the factory - using the static intialization order
Formatter* SchemaFormatter::_ref = Factory::instance().registerFormatter(SchemaFormatter::id, new SchemaFormatter() );

void SchemaFormatter::reset()
{
    _primaryKeyIndex = noPrimaryKeyIndex;
    _tableName = "";
    _columnNames.clear();
    _buffer.clear();
}

std::string SchemaFormatter::generateOutput(const readers::RecordHeaderReader::RecordData& record)
{
    // parse content:
    if ( didInputChange() )
    {
        reset();
        _buffer = getInput()->getInputData();
        parseSchema();
    }

    if (record.headerData.size() != _columnNames.size() && _strict)
    {
        Log::get().err() << "mismatch between given schema and data on file: expected " << _columnNames.size() << " tuples got: " << record.headerData.size() << ". skipping...";
        return {};
    }
    // output data:
    constexpr auto COMMA = ", ";
    size_t columnIndex = 0;
    std::stringstream ss;
    ss << "INSERT INTO " << _tableName << " (" ;
    std::string comma = "";
    for (auto& colname : _columnNames)
    {
        ss << comma << colname;
        comma = COMMA;
    }
    ss << ") VALUES (";
    comma = "";

    // verify that the primary key position is a null value
    if ( _primaryKeyIndex != noPrimaryKeyIndex &&
         record.headerData[_primaryKeyIndex].getType() != wal::types::RecordSerialTypes::Null )
    {
        throw SchemaFormatterException("Primary key invalid position", errorCode::InvalidPrimaryKeyPosition );
    }

    for (auto& rec : record.headerData)
    {
        ss << comma ;
        switch (rec.getType())
        {
            case wal::types::RecordSerialTypes::Null:
                if (columnIndex == _primaryKeyIndex) { ss << record.rowid; }
                else { ss << "NULL"; }
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
                throw SchemaFormatterException(ss.str(), errorCode::UnexpectedColumnType);
            }
        }
        comma = COMMA;
        ++columnIndex;
    }
    ss << ");";
    return ss.str();
}

void SchemaFormatter::parseSchema()
{
    removeWhitespace(_buffer);
    const std::string PARAMS = "[PARAMS]";
    const std::string NAME = "[NAME]";
    const std::string STATEMENT_DELIM = ";";
    std::vector<std::string> pattern = {"CREATE","TABLE", NAME, PARAMS};

    for (const auto statementView : std::views::split(_buffer, STATEMENT_DELIM))
    {
        if (statementView.empty()) { continue; }
        std::string statement{&*statementView.begin(), static_cast<size_t>(std::ranges::distance(statementView)) };

        auto it = statement.cbegin();
        for (auto& token : pattern)
        {
            if ( token == NAME )
            {
                it = parseName( {it, statement.cend()} );
                continue;
            }
            if ( token == PARAMS)
            {
                it = parseParams( {it, statement.cend()} );
                continue;
            }
            it = tokenPos(token, { it, statement.cend()});
        }
        break; // TODO: support multiple statements with indexes and type matching
    }
}


std::string::const_iterator SchemaFormatter::parseName(const Range<const std::string>& range)
{
    std::string optionalEndToken = "EXISTS"; // the ending of IF NOT EXISTS
    auto pos = tokenPos(optionalEndToken,range);
    if ( range.stop == pos ) { pos = range.start; }
    auto tableNameInfo = extractColnameFrom({pos,range.stop});
    if (tableNameInfo.first.empty()) { throw SchemaFormatterException("malformed table name", errorCode::MalformedTableName); }
    _tableName = tableNameInfo.first;
    return tableNameInfo.second;
}


std::string::const_iterator SchemaFormatter::parseParams(const Range<const std::string>& range)
{
    auto enclusingParentheses = getParenthesesRange(range);
    constexpr auto minDist = 5; // minimum table column can be: (a INT)
    if (std::distance(enclusingParentheses.start, enclusingParentheses.stop) < minDist) { throw SchemaFormatterException("malformed parentheses in table parameters", errorCode::MalformedParentheses); }
    // clear all parentheses in range since token in parentheses can have commas
    std::string localCopy{enclusingParentheses.start+1, enclusingParentheses.stop};
    clearAllParentheses( localCopy );
    // per entrance take column name
    constexpr const std::string_view delim = ",";
    tokenizers::split(localCopy, delim, [this](const std::string& part){
        if (!isTableConstraint(part))
        {
            auto columnNameInfo = extractColnameFrom({part.begin(),part.end()});
            if (columnNameInfo.first.empty()) { throw SchemaFormatterException("malformed colname found", errorCode::MalformedColumnDefinition); }
            _columnNames.emplace_back( columnNameInfo.first );
        }
        return true;
    });
    _primaryKeyIndex = findPrimaryColumnIndex(range);
    return enclusingParentheses.stop+1;
}

size_t SchemaFormatter::findPrimaryColumnIndex(const Range<const std::string>& range) const
{
    constexpr std::string_view initialToken = "PRIMARY";
    std::vector<std::string> pattern = { "KEY" };
    auto it = std::search(range.start,range.stop, initialToken.begin(), initialToken.end());
    if ( it == range.stop ) { return noPrimaryKeyIndex; }

    std::string name{};

    // find pattern CREATE TABLE t(x INTEGER, y, z, PRIMARY KEY(x ASC))
    auto pos = tokensPos( { "KEY" , "("}, { it+initialToken.size(), range.stop } );
    if ( range.stop != pos )
    {
        auto colNameInfo = extractColnameFrom(Range<const std::string>{pos,range.stop});
        name = colNameInfo.first;
    }
    else
    {
        // find pattern CREATE TABLE t(x INTEGER PRIMARY KEY ASC, y, z);
        pos = tokenPos( "INTEGER", { it, range.start } );
        auto posForward = tokenPos( "KEY", { it+initialToken.size(), range.stop } );
        if ( range.stop != pos && range.stop != posForward)
        {
            auto colNameInfo = extractColnameFrom(Range<const std::string>{pos-1,range.start});
            name = colNameInfo.first;
        }
    }

    // if we got here and name is empty it means we found the primary keyword but none of the patteren worked, so this is malformed
    if ( name.empty() ) { throw SchemaFormatterException("failed to extract primary key column name", errorCode::MalformedPrimaryKey); }

    auto namePos = std::find(_columnNames.begin(), _columnNames.end(), name);
    if (namePos == _columnNames.end()) { throw SchemaFormatterException("failed to extract primary key column name", errorCode::MalformedPrimaryKey);}
    return static_cast<size_t>(std::distance(_columnNames.begin(), namePos));
}

bool SchemaFormatter::isTableConstraint(const std::string& column) const
{
    static const std::vector<std::string> constraints = {
        "CONSTRAINT",
        "PRIMARY",
        "UNIQUE",
        "CHECK",
        "FOREIGN"
    };

    for (const auto& constraint: constraints)
    {
        auto pos = tokenPos( constraint, { column.begin(), column.end() } );
        if (pos != column.end() ) { return true; }

    }

    return false;
}
