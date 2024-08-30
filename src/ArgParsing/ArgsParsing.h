#include <any>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <unordered_map>
#include <optional>
#include <string_view>
#include "ArgValues.h"
#pragma once

namespace wal::arg_parsing
{
    class ArgsParsing
    {
        public:
            ArgsParsing(std::string_view programName,
                        std::string_view description,
                        int argc,
                        char* argv[]);

            ~ArgsParsing() = default;

            struct ArgName
            {
                std::string name;
                std::string shortName;
            };

            template<typename... ERR_C>
            void error(std::string_view error, ERR_C... components)
            {
                std::vector<std::string_view> strComponents{components...};
                _errors.emplace_back(error);
                for (const auto& e : strComponents)
                {
                    _errors.back() += e;
                }
            }

            template<typename T>
            void addArg(const ArgName& name, std::string_view desc = "", bool optional = false,  const ArgValues<T>& args = {})
            {
                if (name.name.empty()) { throw std::runtime_error("argument name is mandatory"); }

                createUsage(name, desc, optional, (args.size() > 0 ? args.toString() : "") );

                auto argInfo = isInArguments(name);
                if ( !argInfo.isFound )
                {
                    if (optional) { return; }
                    error("missing non-optional argument: ", name.name);
                    return;
                }

                if ( args.size() > 0 )
                {
                    auto argIt = ++argInfo.pos;
                    if (std::distance(argIt ,_args.cend()) <= 0)
                    {
                        error("missing argument-value for argument: ", name.name);
                        return;
                    }
                    if ( !args.isValid(*argIt) )
                    {
                        error("invalid argument-value for argument ", name.name, ": ", *argIt);
                        return;
                    }
                    _tokens.emplace(name.name, args.toValue(*argIt));
                    return;
                }

                _tokens.emplace(name.name,0);
            }

            template<typename RET>
            std::optional<RET> getArgValue(const std::string& name)
            {
                if (argExists(name))
                {
                    return std::any_cast<RET>(_tokens[name]);
                }
                return std::nullopt;
            }

            void addArg(const ArgName& name, std::string_view desc = "", bool optional = false, bool anyInput = false);

            bool argExists(const std::string& name);
            bool anyErrors();
            std::string usage();
            std::list<std::string> getErrorList();

        private:

            std::vector<std::string> _args;
            std::unordered_map<std::string, std::any> _tokens;
            std::vector<std::string> _usage;
            std::list<std::string> _errors;

            struct ArgFound
            {
                bool isFound;
                std::vector<std::string>::const_iterator pos;
            };

            ArgFound isInArguments(const ArgName& name);
            void createUsage(const ArgName& name, std::string_view desc = "", bool optional = false, std::string_view args = "");

    };
}
