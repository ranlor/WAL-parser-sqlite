#pragma once
#include <string>

////////////////////// to string overloading ///////////////////////////////////
namespace TestUtils {

    template<typename type>
    inline std::string toString(type t) { return std::to_string(t); }

    template<>
    inline std::string toString<std::string>(std::string t) { return t; }

    template<>
    inline std::string toString<const std::string&>(const std::string& t) { return t; }

    template<>
    inline std::string toString<const char*>(const char* t) { return std::string{t}; }
}