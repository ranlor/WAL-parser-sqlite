#pragma once
#include <string>
#include <vector>
#include "Readers/RecordHeaderReader.h"
#include "Input/InputType.h"


namespace wal::formatters {

    class Formatter
    {
        public:
            class FormatterException: public  std::runtime_error
            {
                public:
                    FormatterException(std::string_view message, short errorCode):std::runtime_error(message.data()),_errorCode(errorCode) {}
                    short code() { return _errorCode; }
                protected:
                    short _errorCode;
            };

            Formatter() = default;
            virtual ~Formatter() = default;

            void setInput(std::unique_ptr<inputs::InputType>&& inputType) 
            { 
                _input = std::move(inputType); 
                _inputChanged = true;
            }
            void strictMode() { _strict = true; }
            void lenientMode() { _strict = false; }
            
            virtual std::string generateOutput(const wal::readers::RecordHeaderReader::RecordData& record) = 0;
        protected:
            inputs::InputType* getInput() 
            {
                if (nullptr == _input) { throw std::runtime_error("taking input before it was set, call setInput"); }
                _inputChanged = false;
                return _input.get();
            }

            bool didInputChange() { return _inputChanged; }
            
            bool _strict = true;
        private:
            bool _inputChanged = false;
            std::unique_ptr<inputs::InputType> _input = nullptr;
    };
}
