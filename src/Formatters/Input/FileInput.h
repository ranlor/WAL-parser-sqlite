#pragma once
#include <string>
#include <filesystem>
#include "InputType.h"

namespace wal::formatters::inputs {

    class FileInput : public InputType
    {
        public:
            FileInput(const std::filesystem::path& filepath);
            ~FileInput() = default;

            std::string getInputData() override;
        private:
            std::filesystem::path _path;
            std::string _buffer;
    };
}
