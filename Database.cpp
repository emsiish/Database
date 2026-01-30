#include "Database.h"

#include <iostream>
#include <ostream>

void Database::createTable(const std::string &tableName, const std::vector<Column> &columns) {
    if (tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " already exists");
    }
    tables[tableName] = Table(tableName, columns);
    saveToDisk();
    std::cout << "Table " << tableName << " created" << std::endl;
}

void Database::dropTable(const std::string &tableName) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    tables.erase(tableName);
    saveToDisk();
    std::cout << "Table " << tableName << " deleted" << std::endl;
}

void Database::listTables() const {
    if (tables.empty()) {
        std::cout << "No tables in the database" << std::endl;
        return;
    }
    std::cout << "There " << (tables.size() == 1 ? "is" : "are") << " "
              << tables.size() << " table" << (tables.size() == 1 ? "" : "s")
              << " in the database:" << std::endl;
    for (const auto &table : tables) {
        std::cout << table.first << std::endl;
    }
}

void Database::tableInfo(const std::string &tableName) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    const auto &table = tables[tableName];
    std::cout << "Table " << tableName << " : (";

    std::vector<Column> columns = table.getColumns();
    for (std::size_t i = 0; i < table.getColumns().size(); i++) {
        std::cout << table.getColumns()[i].name << ":" << dataTypeToString(table.getColumns()[i].type);
        if (table.getColumns()[i].indexed) {
            std::cout << ", " << (table.getColumns()[i].uniqueIndex ? "Unique " : "") << "Indexed";
        }
        if (i < table.getColumns().size() - 1) std::cout << "; ";
    }
    std::cout << ")" << std::endl;
}

void Database::insert(const std::string &tableName, std::vector<Row> &rows) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    Table &table = tables[tableName];
    for (const auto &row : rows) {
        if (row.values.size() != table.getColumns().size()) {
            throw std::runtime_error("Column size mismatch");
        }
        table.insertRow(row);
    }
    saveToDisk();
    std::cout << (rows.size() == 1 ? "One row" : std::to_string(rows.size()) + " rows")
         << " inserted." << std::endl;
}

void Database::remove(const std::string &tableName, std::unique_ptr<Expression> whereExpr) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    Table &table = tables[tableName];
    int removedRows = 0;
    for (int i = table.getRows().size() - 1; i >= 0; i--) {
        if (whereExpr->evaluate(table.getRows()[i], table)) {
            table.removeRow(i);
            ++removedRows;
        }
    }
    saveToDisk();
    std::cout << removedRows << " row" << (removedRows == 1 ? "" : "s") << " removed." << std::endl;
}





