#pragma once
#include "Formatter.h"
#include <filesystem>
#include <vector>

namespace wal::formatters {

    class SchemaFormatter: public Formatter
    {
        public:
            template<typename T>
            struct Range {
                T::const_iterator start;
                T::const_iterator stop;
            };

            class SchemaFormatterException : public FormatterException
            {
                public:
                    enum class ErrorCode: short
                    {
                        //unexpected column type on generateOutput:
                        UnexpectedColumnType = 1,
                        //"malformed table name"
                        MalformedTableName,
                        //"malformed parentheses in table parameters"
                        MalformedParentheses,
                        //malformed colname found"
                        MalformedColumnDefinition,
                        //"failed to extract primary key column name"
                        FailedToGetColumnName,
                        //"primary key position is not null value, malformed"
                        InvalidPrimaryKeyPosition,
                        //failed to extract primary key column name"
                        MalformedPrimaryKey
                    };

                    SchemaFormatterException(std::string_view message, ErrorCode errorCode):FormatterException(message, std::to_underlying(errorCode)) {}
            };

            SchemaFormatter() = default;
            ~SchemaFormatter() = default;

            std::string generateOutput(const readers::RecordHeaderReader::RecordData& record) override;

            static constexpr int id = 10; // the id in the factory when self registering

        private:
            void reset();
            void parseSchema();
            // return the index in columnNames where the column is defined as integer primary key
            size_t findPrimaryColumnIndex(const Range<const std::string>& range) const;
            std::string::const_iterator parseParams(const Range<const std::string>& range);
            std::string::const_iterator parseName(const Range<const std::string>& range);
            bool isTableConstraint(const std::string& tableColumn) const;

            static Formatter* _ref; 
            
            std::string _buffer;
            std::vector<std::string> _columnNames;
            std::string _tableName;
            size_t _primaryKeyIndex = noPrimaryKeyIndex;
            static constexpr size_t noPrimaryKeyIndex = -1;
    };
}
