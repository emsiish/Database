# FMISql - Database Management System

A relational database management system (RDBMS) implemented in C++ with SQL-like functionality.

## Key Features

### Data Types & Storage
- **Supported Types**: DOUBLE, STRING, and DATE
- **Auto-increment**: Automatic ID generation for numeric columns
- **Default Values**: Column-level default value support
- **Binary Persistence**: Data stored in binary format with checksum validation

### Indexing
- Tree-based structures using `std::map` and `std::multimap`
- Optimized SELECT and REMOVE operations
- Support for both unique and non-unique indexes
- Automatic index maintenance

### Query Processing
- **Recursive Parser**: Converts text queries into object trees
- **Expression Evaluation**: Complex logical conditions with AND, OR, NOT operators
- **Comparison Operators**: Support for `=`, `!=`, `<`, `>`, `<=`, `>=`
- **WHERE Clauses**: Advanced filtering capabilities

### Data Persistence
- **Format**: Binary file (`fmisql.db`)
- **Checksum**: File integrity validation
- **Auto-save**: Data written on exit
- **Auto-load**: Data restored on startup

## System Requirements

- C++20 compatible compiler
- CMake 3.10 or newer

## Building the Project

### Compilation Steps

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the Application

After successful compilation, start the application:

```bash
./application
```

You will see the interactive prompt:

```
FMI Database Management System v1.0
Type 'quit' to exit

FMISql> 
```

## Running Tests

The project includes automated tests using the Catch2 framework:

```bash
./test_runner
```

## Technical Details

### Architecture

**Database Class** (`Database.h/cpp`)
- Manages collection of tables
- Handles persistence (save/load)
- Checksum validation for data integrity

**Table Class** (`Table.h/cpp`)
- Column definitions and metadata
- Row storage and management
- Index maintenance

**Parser** (`Parser.h`)
- Tokenization of SQL commands
- Data type recognition
- WHERE expression parsing

**Expression Tree** (`Expression.h`)
- Recursive expression evaluation
- Support for complex logical conditions
- Type-safe comparisons

**Index** (`Index.h/cpp`)
- Multi-map based indexing
- Fast lookups for WHERE clauses
- Automatic updates on data modification