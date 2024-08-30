cmake_minimum_required(VERSION 3.22)
project(walParserTests)

set(sourceFiles
    BigEndianConvertTest.cpp
    FixedRuntimeArrayTests.cpp
    RecordHeaderReaderTests.cpp
    SchemaFormatterTests.cpp
    TestBase.h
    TestBase.cpp
    ${CMAKE_SOURCE_DIR}/src/Utils/Log.cpp
    ${CMAKE_SOURCE_DIR}/src/Readers/RecordHeaderReader.cpp
    ${CMAKE_SOURCE_DIR}/src/Readers/RecordHeaderDataType.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatters/SchemaFormatter.cpp
    ${CMAKE_SOURCE_DIR}/src/Formatters/CSVFormatter.cpp
    )


add_executable(wal-parser-tests ${sourceFiles})

set_property(TARGET wal-parser-tests PROPERTY CXX_STANDARD 23)
target_compile_options(wal-parser-tests PRIVATE -fsanitize=address -fno-omit-frame-pointer )
target_link_options(wal-parser-tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)

target_include_directories(wal-parser-tests PRIVATE
    "${CMAKE_SOURCE_DIR}/src")
