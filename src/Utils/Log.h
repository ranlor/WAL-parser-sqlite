#include <iostream>
#pragma once

namespace wal {

    class OsstreamBubbleWrap
    {
        public:
            OsstreamBubbleWrap(std::ostream& ostream, std::string_view prefix):
                _ss(ostream.rdbuf()), 
                _pf(prefix) 
            {}
            
            ~OsstreamBubbleWrap() { _ss << std::endl; }

            template<typename T>
            std::ostream& operator<<(const T& obj)
            {
                return _ss << _pf << obj ;
            }

        private:
            std::ostream _ss;
            std::string _pf;
    };
    
    class Log 
    {
        public:
            enum class ReportLevel
            {
                None=0,
                Error=1,
                Info=2,
                Debug=3
            };

            static Log& get()
            {
                static Log instance;
                return instance;
            }

            void setLogLevel(const ReportLevel& level);

            OsstreamBubbleWrap debug();
            OsstreamBubbleWrap info();
            OsstreamBubbleWrap err();

        private:
            Log();
            ~Log() = default;

        std::ostream _streamOut;
        std::ostream _streamErr;
        std::ostream _streamNull;
        ReportLevel _level;
    };
}