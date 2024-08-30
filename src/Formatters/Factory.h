#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Formatter.h"

namespace wal::formatters {
    
    class Factory
    {
        public:
            virtual ~Factory() = default;
            static Factory& instance()
            {
                static Factory factory;
                return factory;
            }
            
            Formatter* registerFormatter(int id, Formatter* formatter) 
            {
                _map.emplace( std::make_pair( id, formatter ) );
                return formatter;
            }

            Formatter* getFormatter(int id) 
            { 
                return _map.at(id).get(); 
            }
        private:
            Factory() = default;
            std::unordered_map<int, std::unique_ptr<Formatter>> _map;
    };
}
