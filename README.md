# WAL-parser-sqlite
Write Ahead Log parser with multiple ways to output data

While accidentally deleting a sqlite database file i was able to recover some of the files while looking at the *.sqlite-wal file i salvaged.
So i wrote this parser to be able to extract all the data from it to recover some of the data.

## Usage

```
wal-parser
    parse binary WAL (Write-Ahead-Log) sqlite file & has options to output it
    
    --input|-i: filepath input of wal sql binary file. Valid values: [string input]
    --help|-h: (Optional) show this usage.
    --verbose|-v: (Optional) verbose levels. Valid values: [debug,info]
    --csv|-csv: (Optional) [default] will output csv format with defined columns i.e. -csv 'col1,col2'. Valid values: [string input]
    --sql|-s: (Optional) will output sql insert statements with the given schema file (only supports single create table schema) i.e. -s '../schema.sql' . Valid values: [string input]
    --strict|-c: (Optional) if using --sql arg will output only insert statement that are valid with the schema.
    --lenient|-l: (Optional) [default] if using --sql arg will output any insert statement, even if the column don't match.
    --valid-frames|-f: (Optional) prase only btree frames that have valid checksum.
    --index|-x: (Optional) parse index btree data instead of table data, will only print the indices as is.
    --quiet|-q: (Optional) don't output any logs, not even error

```

## Dependencies

- a compiler that has some support for cpp++23 standard. (Small changes can be made to conform to cpp++20 standard.)
- cmake 3.22

## Structure

The project has zero dependencies, all testing and argument parsing is done with small simple code that was written as a learning experience and to make this build very simple

Testing is done with similar way to how gtest works just with much more simple and streamlined approach (with no mocking)

Argument parsing is done with logic that checks the validity of the arguments as their being added in the code

Logs are written with a simple singleton anti-patteren with none/error/info/debug levels

FixedRuntimeArray - an "array" that has a size that can be determined in run time but has no ability to grow or shrink and has only one allocation (a mix between std::array and std::vector)

## Building

```
./build.sh <Debug|Release>
```

Should work on Windows when using cmake to generate Visual Studio project

## Testing

```
./build/tests/wal-parser-tests
```

## Examples

Parse file with csv output to file
```
./wal-parser -i /path/to/database.sql-wal --csv "col1,col2,col3" > output.csv
```
Parse file with csv output to file only with data that fits to the columns
```
./wal-parser -i /path/to/database.sql-wal --strict --csv "col1,col2,col3" > output.csv
```
Parse file with reading a sql schema file and  output insert directive to file
the schema file should have a valid `CREATE TABLE` directive with all the columns
```
./wal-parser -i /path/to/database.sql-wal --sql /path/to/schema.sql > output.sql
```

Parse file with reading a sql schema file and output insert directive to file, while
only process data that fits the schema and suppress errors
the schema file should have a valid `CREATE TABLE` directive with all the columns
```
./wal-parser -i /path/to/database.sql-wal --strict -q --sql /path/to/schema.sql > output.sql
```

Parse file with csv output , but only output index data, to file
```
./wal-parser -i /path/to/database.sql-wal --index --csv "col1,col2,col3" > output.csv
```

Parse file with csv output with maximum verbosity and output to file
```
./wal-parser -i /path/to/database.sql-wal -v debug --csv "col1,col2,col3" > output.csv
```

## Extensions

New formatters can be added that take a file/string input and a processes record from the WAL file and output data, new formatters have a self-registering logic.
Any extension needs to inherit `Formatter.h` and include:
- `static constexpr int id` in the definition , this acts as unique id to use in `Factory.h` to create the formatter
- and a line to build an instance in the factory i.e. `Formatter* AFormatter::_ref = Factory::instance().registerFormatter(AFormatter::id, new AFormatter());`

with these 2 requirements the formatter is part of the factory


## Resources

- https://sqlite.org/src/file/src/wal.c
- https://sqlite.org/fileformat2.html
- https://sqlite.org/src4/doc/trunk/www/varint.wiki


## TODO
- add more tests for ArgParse and Formatters
- add output to file instead of using redirection