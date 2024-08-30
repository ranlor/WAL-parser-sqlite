#include "ArgsParsing.h"

using namespace wal::arg_parsing;

ArgsParsing::ArgsParsing(std::string_view programName,
                         std::string_view description,
                         int argc,
                         char* argv[]):_args({})
{
    for (int i=1; i<argc; ++i)
    {
        _args.emplace_back(argv[i]);
    }
    _usage.emplace_back(programName);
    _usage.emplace_back(description);
    _usage.emplace_back("");
}


void ArgsParsing::createUsage(const ArgName& name, std::string_view desc, bool optional, std::string_view args)
{
    std::stringstream ss;
    ss << name.name;
    if ( !name.shortName.empty() ) { ss << "|" << name.shortName; }
    ss << ": ";
    ss << (optional ? "(Optional) " : "" );
    ss << desc;
    if ( !desc.ends_with(".") ) { ss << "."; }
    if ( !args.empty() ) { ss << " Valid values: " << args; }
    _usage.emplace_back(ss.str());
}

void ArgsParsing::addArg(const ArgName& name, std::string_view desc, bool optional, bool anyInput)
{
    if (anyInput)
    {
        auto anyVal = ArgValues<std::string>::ANY_VAL;
        addArg<std::string>(name, desc, optional, { {anyVal,anyVal} });
        return;
    }
    addArg<int>(name, desc, optional, {});
}

bool ArgsParsing::argExists(const std::string& name)
{
    return _tokens.contains(name);
}

std::string ArgsParsing::usage()
{
    std::stringstream ss;
    auto programName = _usage.begin();
    ss << *programName << std::endl;
    for (auto it = _usage.begin()+1; it != _usage.end(); ++it )
    {
        ss << "    " << *it << std::endl;
    }
    return ss.str();
}

bool ArgsParsing::anyErrors() { return _errors.size() > 0; }

std::list<std::string> ArgsParsing::getErrorList() { return _errors; }

ArgsParsing::ArgFound ArgsParsing::isInArguments(const ArgName& name)
{
    auto pos = std::find(_args.crbegin(),_args.crend(), name.name);
    if (_args.crend() == pos && !name.shortName.empty())
    {
        pos = std::find(_args.crbegin(),_args.crend(), name.shortName);
    }
    // ++pos since calling base will increment the iterator
    return {_args.crend() != pos, (++pos).base()};
}
