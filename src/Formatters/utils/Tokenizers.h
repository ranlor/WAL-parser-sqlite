#pragma once
#include <string>
#include <string_view>
#include <ranges>
#include <functional>

namespace wal::formatters::tokenizers {

    /**
     * @brief 
     * 
     * @tparam IT 
     * @param token 
     * @param start 
     * @param stop 
     * @return std::string::iterator 
     */
    template<typename TIT, typename IT>
    IT tokenPos(const TIT& tokenStart, const TIT& tokenStop, const IT& start, const IT& stop)
    {
        auto tokenSize = std::distance(tokenStart, tokenStop);
        if ( tokenSize < 1 ) { return stop; }
        if ( std::distance(start,stop) < tokenSize ) { return stop; }
        auto tokenIt = tokenStart;
        for (auto i = start; i != stop; ++i)
        {
            auto c = *i;
            if (std::isspace(c)) 
            { 
                continue;
            }

            if ( c != *tokenIt )
            {
                return stop;
            }

            ++tokenIt;

            if ( tokenStop == tokenIt)
            {
                return i+1;
            }
        }

        return stop;
    }

    inline void split(std::string_view input, 
                      std::string_view delim, 
                      const std::function<bool(const std::string&)>& callback,
                      bool skipEmpty = true)
    {
        for (const auto part : std::views::split(input,delim))
        {
            if (part.empty() && skipEmpty) { continue; }
            // if compiler supports ranges fully, this can be changed to "std::string_view part{part.begin(), part.end()};"
            // since i don't have full ranges support in the current compiler, i use another ctor of string_view
            std::string partStr{&*part.begin(), static_cast<size_t>(std::ranges::distance(part)) };
            if ( !callback(partStr) ) { break; }
        }
    }
}