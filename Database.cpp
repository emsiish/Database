#include "Database.h"

#include <fstream>

void Database::createTable(const std::string &tableName, const std::vector<Column> &columnNames) {
    if (tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " already exists");
    }
    tables[tableName] = Table(tableName, columnNames);
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

void Database::insert(const std::string &tableName, const std::vector<Row> &rows) {
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

void Database::select(const std::string& tableName, const std::vector<std::string>& columnNames, const std::unique_ptr<Expression>& whereExpression, const std::string& orderByColumn) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    const Table &table = tables[tableName];
    std::vector<Row> results;

    for (const Row &row : table.getRows()) {
        if (!whereExpression || whereExpression->evaluate(row, table)) {
            results.push_back(row);
        }
    }

    if (!orderByColumn.empty()) {
        int colIdx = table.getColumnIndex(orderByColumn);
        if (colIdx != -1) {
            std::ranges::sort(results, [colIdx](const Row& a, const Row& b) {
                         return a.values[colIdx] < b.values[colIdx];
                     });
        }
    }

    std::vector<int> columnsToDisplay;
    if (columnNames.size() == 1 && columnNames[0] == "*") {
        for (std::size_t i = 0; i < table.getColumns().size(); i++) {
            columnsToDisplay.push_back(i);
        }
    } else {
        for (const auto& name : columnNames) {
            int idx = table.getColumnIndex(name);
            if (idx != -1) columnsToDisplay.push_back(idx);
        }
    }

    for (size_t i = 0; i < columnsToDisplay.size(); i++) {
        std::cout << "|" << table.getColumns()[columnsToDisplay[i]].name;
    }
    std::cout << "|" << std::endl;

    for (size_t i = 0; i < columnsToDisplay.size(); i++) {
        for (size_t j = 0; j < table.getColumns()[columnsToDisplay[i]].name.length() + 1; j++) {
            std::cout << "-";
        }
    }
    std::cout << std::endl;

    for (const auto& row : results) {
        for (size_t i = 0; i < columnsToDisplay.size(); i++) {
            std::cout << "|" << std::setw(table.getColumns()[columnsToDisplay[i]].name.length())
                 << row.values[columnsToDisplay[i]].toString();
        }
        std::cout << "|" << std::endl;
    }

    std::cout << "Total " << results.size() << " row"
         << (results.size() == 1 ? "" : "s") << " selected" << std::endl;
}

void Database::saveToDisk() const {
    std::ofstream file(dbPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Could not open file for writing");
    }

    uint32_t tableCount = tables.size();
    file.write(reinterpret_cast<char*>(&tableCount), sizeof(tableCount));

    for (const auto& t : tables) {
        const Table& table = t.second;

        uint32_t nameLen = table.getName().length();
        file.write(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        file.write(table.getName().c_str(), nameLen);

        uint32_t colCount = table.getColumns().size();
        file.write(reinterpret_cast<char*>(&colCount), sizeof(colCount));

        for (const auto& col : table.getColumns()) {
            uint32_t colNameLen = col.name.length();
            file.write(reinterpret_cast<char*>(&colNameLen), sizeof(colNameLen));
            file.write(col.name.c_str(), colNameLen);

            uint8_t type = static_cast<uint8_t>(col.type);
            file.write(reinterpret_cast<char*>(&type), sizeof(type));

            uint8_t flags = (col.indexed ? 1 : 0) | (col.autoIncrement ? 2 : 0);
            file.write(reinterpret_cast<char*>(&flags), sizeof(flags));
        }

        uint32_t rowCount = table.getRows().size();
        file.write(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));

        for (const auto& row : table.getRows()) {
            for (size_t i = 0; i < row.values.size(); i++) {
                const Value& val = row.values[i];
                if (table.getColumns()[i].type == DataType::INT) {
                    file.write(reinterpret_cast<const char*>(&val.intValue), sizeof(val.intValue));
                } else {
                    uint32_t strLen = val.strValue.length();
                    file.write(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    file.write(val.strValue.c_str(), strLen);
                }
            }
        }
    }

    file.close();
}

void Database::loadFromDisk() {
    std::ifstream file(dbPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Could not open file for reading");
    }
    uint32_t tableCount;
    file.read(reinterpret_cast<char*>(&tableCount), sizeof(tableCount));

    for (uint32_t t = 0; t < tableCount; t++) {
        uint32_t nameLen;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        std::string tableName(nameLen, ' ');
        file.read(&tableName[0], nameLen);

        uint32_t colCount;
        file.read(reinterpret_cast<char*>(&colCount), sizeof(colCount));

        std::vector<Column> columns;
        for (uint32_t c = 0; c < colCount; c++) {
            uint32_t colNameLen;
            file.read(reinterpret_cast<char*>(&colNameLen), sizeof(colNameLen));
            std::string colName(colNameLen, ' ');
            file.read(&colName[0], colNameLen);

            uint8_t type;
            file.read(reinterpret_cast<char*>(&type), sizeof(type));

            uint8_t flags;
            file.read(reinterpret_cast<char*>(&flags), sizeof(flags));

            Column col(colName, static_cast<DataType>(type));
            col.indexed = (flags & 1) != 0;
            col.autoIncrement = (flags & 2) != 0;
            columns.push_back(col);
        }

        tables[tableName] = Table(tableName, columns);
        Table& table = tables[tableName];

        uint32_t rowCount;
        file.read(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));

        for (uint32_t r = 0; r < rowCount; r++) {
            Row row;
            for (const auto& col : columns) {
                if (col.type == DataType::INT) {
                    double val;
                    file.read(reinterpret_cast<char*>(&val), sizeof(val));
                    row.values.emplace_back(val);
                } else {
                    uint32_t strLen;
                    file.read(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    std::string str(strLen, ' ');
                    file.read(&str[0], strLen);
                    row.values.emplace_back(str);
                }
            }
            table.insertRow(row);
        }
    }
    file.close();
}





