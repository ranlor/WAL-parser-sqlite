#include <unordered_map>
#include <string>
#include <sstream>
#pragma once

namespace wal::arg_parsing
{
   template<typename T>
    class ArgValues
    {
        public:
            static constexpr auto ANY_VAL = "ANYVAL";

            ArgValues(std::initializer_list<std::pair<std::string,T>> list)
            {
                for (auto& pair : list)
                {
                    validArgs.emplace(pair);
                }
            }

            ~ArgValues() = default;

            bool isValid(const std::string& arg) const
            {
                if ( validArgs.contains(ANY_VAL) ) { return true; }
                return validArgs.contains(arg);
            }

            T toValue(const std::string& arg) const
            {
                if constexpr (std::is_same_v<std::string,T>)
                {
                    if ( validArgs.contains(ANY_VAL) ) { return arg; }
                }
                return validArgs.at(arg);
            }

            size_t size() const { return validArgs.size(); }

            std::string toString() const
            {
                std::stringstream ss;
                char comma = '\0';
                ss << "[";
                for (const auto& pair : validArgs)
                {
                    if ( ANY_VAL == pair.first ) { ss << comma << "string input"; }
                    else { ss << comma << pair.first; }
                    comma = ',';
                }
                ss << "]";
                return ss.str();
            }

        private:
            std::unordered_map<std::string,T> validArgs;
    };
}
