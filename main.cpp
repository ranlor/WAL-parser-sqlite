#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>
#include <set>
#include <optional>
#include "Converters/Endian.h"
#include "Utils/Log.h"
#include "Utils/FixedRuntimeArray.h"
#include "Readers/RecordHeaderReader.h"
#include "Readers/BTreeReader.h"
#include "Readers/WalHeaderReader.h"
#include "Readers/FrameHeader.h"
#include "Formatters/Factory.h"
#include "Formatters/SchemaFormatter.h"
#include "Formatters/CSVFormatter.h"
#include "Formatters/Input/FileInput.h"
#include "Formatters/Input/StringInput.h"
#include "ArgParsing/ArgsParsing.h"

#define EXIT_OK 0
#define ARG_ERR 1
#define PATH_ERR 2
#define READ_ERR 3
#define HEAD_READ_ERR 4
#define MISSING_OPT_ERR 5

inline bool isFrameWeakValid(const wal::readers::WalHeaderReader& header,
                             const wal::readers::FrameHeader& frameHeader)
{
    return header.salt1() == frameHeader.salt1() &&
           header.salt2() == frameHeader.salt2();
}

inline void printWalHeader(const wal::readers::WalHeaderReader& header)
{
    wal::Log::get().debug() << "header: " << header.header();
    wal::Log::get().debug() << "version: " << header.version();
    wal::Log::get().debug() << "page size: " << header.page_size();
    wal::Log::get().debug() << "seq: " << header.sequence();
    wal::Log::get().debug() << "salt1: " << header.salt1();
    wal::Log::get().debug() << "salt2: " << header.salt2();
}

inline void printFrameHeader(const wal::readers::FrameHeader& header)
{
    wal::Log::get().debug() << "frame page number " << header.pageNumber();
    wal::Log::get().debug() << "frame page size " << header.sizeInPage();
}

enum class VerboseLevels
{
    Info=1,
    Debug=2
};

int main(int argc, char* argv[])
{
    wal::arg_parsing::ArgsParsing args{
        "wal-parser",
        "parse binary WAL (Write-Ahead-Log) sqlite file & has options to output it",
        argc, argv};

    args.addArg({"--input", "-i"}, "filepath input of wal sql binary file", false /*optional*/, true /*get any input*/);
    args.addArg({"--help", "-h"}, "show this usage", true /*optional*/);
    args.addArg<VerboseLevels>({"--verbose", "-v"}, "verbose levels", true /*optional*/ , {{"info",VerboseLevels::Info},{"debug",VerboseLevels::Debug}});
    args.addArg({"--csv", "-csv"}, "[default] will output csv format with defined columns i.e. -csv 'col1,col2'", true /*optional*/, true /*get any input*/);
    args.addArg({"--sql", "-s"}, "will output sql insert statements with the given schema file (only supports single create table schema) i.e. -s '../schema.sql' ", true /*optional*/, true /*get any input*/);
    args.addArg({"--strict", "-c"}, "if using --sql arg will output only insert statement that are valid with the schema", true /*optional*/);
    args.addArg({"--lenient", "-l"}, "[default] if using --sql arg will output any insert statement, even if the column don't match", true /*optional*/);
    args.addArg({"--valid-frames", "-f"}, "prase only btree frames that have valid checksum", true /*optional*/);
    args.addArg({"--index", "-x"}, "parse index btree data instead of table data, will only print the indices as is", true /*optional*/);
    args.addArg({"--quiet", "-q"}, "don't output any logs, not even errors", true /*optional*/);


    if ( args.argExists("--help") )
    {
        std::cout << args.usage() << std::endl;
        return EXIT_OK;
    }

    if ( args.anyErrors() )
    {
        std::cout << args.usage() << std::endl;
        for(auto& error : args.getErrorList())
        {
            std::cerr << error << std::endl;
        }
        return ARG_ERR;
    }

    wal::Log::get().setLogLevel(wal::Log::ReportLevel::Error);

    bool outputIndexes = args.argExists("--index");

    auto verboseVal = args.getArgValue<VerboseLevels>("--verbose");

    if (verboseVal)
    {
        switch (verboseVal.value())
        {
            case VerboseLevels::Debug:
                wal::Log::get().setLogLevel(wal::Log::ReportLevel::Debug);
            break;
            case VerboseLevels::Info:
                wal::Log::get().setLogLevel(wal::Log::ReportLevel::Info);
            break;
        }
    }

    if (args.argExists("--quiet"))
    {
        wal::Log::get().setLogLevel(wal::Log::ReportLevel::None);
    }

    // readfile
    auto pathStr = args.getArgValue<std::string>("--input").value_or("");
    if (pathStr.empty() || !std::filesystem::exists(pathStr))
    {
        wal::Log::get().err() << "Failed to find file at path " << pathStr;
        return PATH_ERR;
    }

    std::filesystem::path path{pathStr};
    auto file = std::ifstream(path, std::ios::binary);

    if (file.bad())
    {
        wal::Log::get().err() << "Failed to read file at path " << path;
        return READ_ERR;
    }

    wal::readers::WalHeaderReader header;
    file.read(header, header.sizeOf());

    if (!file)
    {
        wal::Log::get().err() << "Failed to read the header of the file (file may be too small)";
        file.close();
        return HEAD_READ_ERR;
    }

    bool skipInvalidFrames = args.argExists("--valid-frames");

    if ( verboseVal && verboseVal.value() == VerboseLevels::Debug )
    {
        printWalHeader(header);
    }

    int formatterId = wal::formatters::CSVFormatter::id;
    std::unique_ptr<wal::formatters::inputs::InputType> formatterInput = nullptr;

    if ( args.argExists("--csv") )
    {
        formatterId = wal::formatters::CSVFormatter::id;
        auto csvCols = args.getArgValue<std::string>("--csv");
        if (csvCols)
        {
            formatterInput = std::make_unique<wal::formatters::inputs::StringInput>(csvCols.value());
        }
    }
    else if ( args.argExists("--sql") )
    {
        formatterId = wal::formatters::SchemaFormatter::id;
        auto schemaFile = args.getArgValue<std::string>("--sql");
        if (schemaFile)
        {
            formatterInput = std::make_unique<wal::formatters::inputs::FileInput>(schemaFile.value());
        }
    }

    auto formatter = wal::formatters::Factory::instance().getFormatter(formatterId);

    formatter->lenientMode();
    if ( args.argExists("--strict") ) { formatter->strictMode(); }

    if ( nullptr == formatterInput )
    {
        wal::Log::get().err() << "Didn't get any option: --csv, --sql";
        return MISSING_OPT_ERR;
    }
    formatter->setInput(std::move(formatterInput));

    std::set<std::string> outputData;
    while(file)
    {
        // - read frame header
        wal::readers::FrameHeader frameHeader;
        file.read( frameHeader, frameHeader.sizeOf() );
        if (!file)
        {
            if (file.eof()) { wal::Log::get().info() << "reach EOF." ; break; }
            else            { wal::Log::get().err()  << "Failed to read Frame Header, file may be incomplete. existing" ; break; }
        }

        if ( verboseVal && verboseVal.value() == VerboseLevels::Debug ) { printFrameHeader(frameHeader); }

        wal::FixedRuntimeArray<uint8_t> frameData(header.page_size());
        file.read((char*)frameData.data(), header.page_size());
        if (!file)
        {
            if (file.eof()) { wal::Log::get().err() << "reach EOF unexpectedly while reading Frame Chunk. exisintg" ; break; }
            else            { wal::Log::get().err()  << "Failed to read Frame Chunk, file may be incomplete. existing" ; break; }
        }

        if ( !isFrameWeakValid(header, frameHeader) && skipInvalidFrames )
        {
            wal::Log::get().info() <<  "Frame is invalid skipping";
            wal::Log::get().info() <<  "mismatch salt1 " << header.salt1() << " != " << frameHeader.salt1();
            wal::Log::get().info() <<  "mismatch salt2 " << header.salt2() << " != " << frameHeader.salt2();
            continue;
        }

        auto it = frameData.begin();
        wal::readers::BTreeReader bTreeReader;

        bTreeReader.readHeader(it);

        if (bTreeReader.isInteriorType())
        {
            wal::Log::get().info() <<  "Found interior Btree Node, skipping";
            continue;
        }

        if (bTreeReader.getBTreeNodeType() == wal::types::BTreeNodePageType::leafIndex && !outputIndexes)
        {
            wal::Log::get().info() <<  "Found "<< bTreeReader.getBTreeNodeType() << ", skipping";
            continue;
        }

        if (bTreeReader.getBTreeNodeType() == wal::types::BTreeNodePageType::leafTable && outputIndexes)
        {
            wal::Log::get().info() <<  "Found "<< bTreeReader.getBTreeNodeType() << " and we got --index arg, skipping";
            continue;
        }

        bTreeReader.readPointerArray(it);

        wal::readers::RecordHeaderReader recordReader(bTreeReader.getBTreeNodeType());

        for(auto& ptr : bTreeReader.getPointerArray())
        {
            auto ptrPos = frameData.begin() + ptr;

            recordReader.read(ptrPos);
            recordReader.printOut();

            std::string output;
            try
            {
                output = formatter->generateOutput(recordReader.headerData());
            }
            catch(const wal::formatters::Formatter::FormatterException& e)
            {
                wal::Log::get().err() << "Failed to generate output due to: " << e.what();
            }

            if (!output.empty()) { outputData.emplace(output); }
        }

    }

    for (auto rit = outputData.rbegin(); rit != outputData.rend(); ++rit)
    {
        std::cout << *rit << std::endl;
    }

    return EXIT_OK;
}
