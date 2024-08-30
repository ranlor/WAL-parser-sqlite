#include "Log.h"

using namespace wal;


Log::Log():
_streamOut(std::cout.rdbuf()),
_streamErr(std::cerr.rdbuf()),
_streamNull(nullptr),
_level(ReportLevel::Info)
{}

void Log::setLogLevel(const ReportLevel& level)
{
    _level = level;
}

OsstreamBubbleWrap Log::debug()
{
    if ( _level < ReportLevel::Debug) { return {_streamNull, ""}; }
    return {_streamOut, "[-D]"};
}

OsstreamBubbleWrap Log::info()
{
    if ( _level < ReportLevel::Info) { return {_streamNull, ""}; }
    return {_streamOut, "[-I]"};
}

OsstreamBubbleWrap Log::err()
{
    if ( _level < ReportLevel::Error) { return {_streamNull, ""}; }
    return {_streamErr, "[-E]"};
}
