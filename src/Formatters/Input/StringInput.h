#pragma once
#include <string>
#include "InputType.h"

namespace wal::formatters::inputs {

    class StringInput : public InputType
    {
        public:
            StringInput(std::string_view buffer):_buffer(buffer) {}
            ~StringInput() = default;

            std::string getInputData() override { return _buffer; }
        private:
            std::string _buffer;
    };
}
