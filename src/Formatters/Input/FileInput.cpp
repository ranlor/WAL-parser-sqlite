#include "FileInput.h"
#include <fstream>
#include <streambuf>

using namespace wal::formatters::inputs;

namespace {

    void readFile(const std::filesystem::path& pathToSchema,std::string& buffer)
    {
        if (!std::filesystem::exists(pathToSchema)) { throw std::runtime_error("Failed to find schema file at path"); }

        std::ifstream file(pathToSchema);

        if (file.bad()) { throw std::runtime_error("Failed to read schema file"); }

        file.seekg( 0 , std::ios::end );

        auto filesize = file.tellg();

        file.seekg(0, std::ios::beg );

        buffer.clear();

        buffer.reserve(filesize);

        buffer.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );

        file.close();
    }
}


FileInput::FileInput(const std::filesystem::path& filepath):
    _path(filepath),
    _buffer({})
{}


std::string FileInput::getInputData()
{
    if (_buffer.empty()) {readFile(_path, _buffer /*out*/);}
    return _buffer;
}
