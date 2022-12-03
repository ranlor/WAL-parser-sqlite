cmake_minimum_required(VERSION 3.22)
project(walParser)

set(sourceFiles
    main.cpp 
    ${CMAKE_SOURCE_DIR}/src/Utils/Log.cpp
    ${CMAKE_SOURCE_DIR}/src/Utils/FixedRuntimeArray.h
    ${CMAKE_SOURCE_DIR}/src/Readers/RecordHeaderReader.cpp
    ${CMAKE_SOURCE_DIR}/src/Readers/RecordHeaderDataType.cpp
    ${CMAKE_SOURCE_DIR}/src/Readers/BTreeReader.cpp
    ${CMAKE_SOURCE_DIR}/src/Readers/WalHeaderReader.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/Factory.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/Formatter.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/SchemaFormatter.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/SchemaFormatter.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatters/CSVFormatter.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatters/Input/InputType.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/Input/StringInput.h
    ${CMAKE_SOURCE_DIR}/src/Formatters/Input/FileInput.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatters/utils/Tokenizers.h
    ${CMAKE_SOURCE_DIR}/src/ArgParsing/ArgsParsing.h
    ${CMAKE_SOURCE_DIR}/src/ArgParsing/ArgsParsing.cpp
    )

    
add_executable(wal-parser ${sourceFiles})

set_property(TARGET wal-parser PROPERTY CXX_STANDARD 23)
target_compile_options(wal-parser PRIVATE -fsanitize=address -fno-omit-frame-pointer )
target_link_options(wal-parser PRIVATE -fsanitize=address -fno-omit-frame-pointer)

target_include_directories(wal-parser PRIVATE
    "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(tests)