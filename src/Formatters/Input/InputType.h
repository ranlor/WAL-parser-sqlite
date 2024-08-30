#pragma once
#include <string>

namespace wal::formatters::inputs {

    class InputType
    {
        public:
            InputType() = default;
            virtual ~InputType() = default;

            virtual std::string getInputData() = 0;
    };
}
