#pragma once
#include "Formatter.h"
#include <vector>

namespace wal::formatters {

    class CSVFormatter: public Formatter
    {
        public:
            class CSVFormatterException : public FormatterException
            {
                public:
                    enum class ErrorCode: short
                    {
                        UnexpectedColumnType
                    };

                    CSVFormatterException(std::string_view message, ErrorCode errorCode):FormatterException(message, std::to_underlying(errorCode)) {}
            };

            CSVFormatter() = default;
            ~CSVFormatter() = default;

            std::string generateOutput(const readers::RecordHeaderReader::RecordData& record) override;

            static constexpr int id = 20; // the id in the factory when self registering
        private:

            static Formatter* _ref; 

            std::vector<std::string> _tableColumns;

            void parseColumns();
    };
}
