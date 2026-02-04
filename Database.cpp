#include "Database.h"

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
    std::cout << "Total " << table.getRows().size() << " rows ("
             << table.getDataSize() / 1024.0 << " KB data) in the table" << std::endl;
}

void Database::insert(const std::string &tableName, std::vector<Row> &rows) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    Table &table = tables[tableName];
    for (auto &row : rows) {
        if (row.values.size() > table.getColumns().size()) {
            throw std::runtime_error("Column size mismatch");
        }
        table.insertRow(row);
    }
    saveToDisk();
    std::cout << (rows.size() == 1 ? "1 row" : std::to_string(rows.size()) + " rows")
         << " inserted." << std::endl;
}

void Database::remove(const std::string &tableName, std::unique_ptr<Expression> whereExpr) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    Table &table = tables[tableName];
    int removedRows = 0;
    for (int i = table.getRows().size() - 1; i >= 0; i--) {
        if (!whereExpr || whereExpr->evaluate(table.getRows()[i], table)) {
            table.removeRow(i);
            ++removedRows;
        }
    }
    saveToDisk();
    std::cout << removedRows << " row" << (removedRows == 1 ? "" : "s") << " removed." << std::endl;
}

void Database::select(const std::string& tableName, const std::vector<std::string>& columnNames,
                      const std::unique_ptr<Expression>& whereExpression, const std::string& orderByColumn,
                      bool isDistinct) {
    if (!tables.contains(tableName)) {
        throw std::runtime_error("Table " + tableName + " does not exists");
    }
    const Table &table = tables[tableName];

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

    std::vector<Row> filteredRows;
    for (const Row &row : table.getRows()) {
        if (!whereExpression || whereExpression->evaluate(row, table)) {
            filteredRows.push_back(row);
        }
    }

    if (!orderByColumn.empty()) {
        int sortColIdx = table.getColumnIndex(orderByColumn);
        if (sortColIdx != -1) {
            std::ranges::sort(filteredRows, [sortColIdx](const Row& a, const Row& b) {
                return a.values[sortColIdx] < b.values[sortColIdx];
            });
        }
    }

    std::vector<Row> results;
    std::set<Row> seenRows;
    for (const auto& row : filteredRows) {
        Row projection;
        for (int colIdx : columnsToDisplay) {
            projection.values.push_back(row.values[colIdx]);
        }

        if (isDistinct) {
            if (seenRows.insert(projection).second) {
                results.push_back(projection);
            }
        } else {
            results.push_back(projection);
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
        for (size_t i = 0; i < row.values.size(); i++) {
            std::cout << "|" << std::setw(table.getColumns()[columnsToDisplay[i]].name.length())
                 << row.values[i].toString();
        }
        std::cout << "|" << std::endl;
    }

    std::cout << "Total " << results.size() << " row"
         << (results.size() == 1 ? "" : "s") << " selected" << std::endl;
}

uint64_t calculateChecksum(const std::vector<char>& data) {
    uint64_t checksum = 0xFDDB0123456789AB;
    for (char c : data) {
        checksum = (checksum ^ static_cast<uint8_t>(c)) * 0xBF58476D1CE4E5B9;
    }
    return checksum;
}

void Database::saveToDisk() const {
    std::ostringstream buffer(std::ios::binary);

    uint32_t tableCount = tables.size();
    buffer.write(reinterpret_cast<char*>(&tableCount), sizeof(tableCount));

    for (const auto& t : tables) {
        const Table& table = t.second;

        uint32_t nameLen = table.getName().length();
        buffer.write(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        buffer.write(table.getName().c_str(), nameLen);

        uint32_t colCount = table.getColumns().size();
        buffer.write(reinterpret_cast<char*>(&colCount), sizeof(colCount));

        for (const auto& col : table.getColumns()) {
            uint32_t colNameLen = col.name.length();
            buffer.write(reinterpret_cast<char*>(&colNameLen), sizeof(colNameLen));
            buffer.write(col.name.c_str(), colNameLen);

            uint8_t type = static_cast<uint8_t>(col.type);
            buffer.write(reinterpret_cast<char*>(&type), sizeof(type));

            uint8_t flags = (col.indexed ? 1 : 0) |
                               (col.autoIncrement ? 2 : 0) |
                               (col.uniqueIndex ? 4 : 0) |
                               (col.hasDefault ? 8 : 0);
            buffer.write(reinterpret_cast<char*>(&flags), sizeof(flags));

            if (col.autoIncrement) {
                uint32_t currentCounter = table.getAutoIncrementCounters()[col.name];
                buffer.write(reinterpret_cast<char*>(&currentCounter), sizeof(currentCounter));
            }

            if (col.hasDefault) {
                if (col.type == DataType::DOUBLE) {
                    buffer.write(reinterpret_cast<const char*>(&col.defaultValue.numValue), sizeof(double));
                } else {
                    uint32_t strLen = col.defaultValue.strValue.length();
                    buffer.write(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    buffer.write(col.defaultValue.strValue.c_str(), strLen);
                }
            }
        }

        uint32_t rowCount = table.getRows().size();
        buffer.write(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));

        for (const auto& row : table.getRows()) {
            for (size_t i = 0; i < row.values.size(); i++) {
                const Value& val = row.values[i];
                if (table.getColumns()[i].type == DataType::DOUBLE) {
                    buffer.write(reinterpret_cast<const char*>(&val.numValue), sizeof(val.numValue));
                } else {
                    uint32_t strLen = val.strValue.length();
                    buffer.write(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    buffer.write(val.strValue.c_str(), strLen);
                }
            }
        }
    }

    std::string serializedData = buffer.str();
    std::vector dataVec(serializedData.begin(), serializedData.end());
    uint64_t checksum = calculateChecksum(dataVec);

    std::ofstream file(dbPath, std::ios::binary);
    if (!file.is_open()) throw std::invalid_argument("Could not open file");

    file.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));
    file.write(dataVec.data(), dataVec.size());
    file.close();
}

void Database::loadFromDisk() {
    std::ifstream file(dbPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return;
    }

    std::streamsize size = file.tellg();
    if (size < static_cast<std::streamsize>(sizeof(uint64_t))) return; // Празен или твърде малък файл

    file.seekg(0, std::ios::beg);

    uint64_t storedChecksum;
    file.read(reinterpret_cast<char*>(&storedChecksum), sizeof(storedChecksum));

    std::vector<char> data(static_cast<size_t>(size) - sizeof(uint64_t));
    file.read(data.data(), data.size());
    file.close();

    if (calculateChecksum(data) != storedChecksum) {
        throw std::runtime_error("Database file is corrupted or invalid (Checksum mismatch)!");
    }

    std::string content(data.begin(), data.end());
    std::istringstream buffer(content, std::ios::binary);

    uint32_t tableCount;
    buffer.read(reinterpret_cast<char*>(&tableCount), sizeof(tableCount));

    for (uint32_t t = 0; t < tableCount; t++) {
        uint32_t nameLen;
        buffer.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        std::string tableName(nameLen, ' ');
        buffer.read(&tableName[0], nameLen);

        uint32_t colCount;
        buffer.read(reinterpret_cast<char*>(&colCount), sizeof(colCount));

        std::vector<Column> columns;
        std::map<std::string, uint32_t> loadedCounters;
        for (uint32_t c = 0; c < colCount; c++) {
            uint32_t colNameLen;
            buffer.read(reinterpret_cast<char*>(&colNameLen), sizeof(colNameLen));
            std::string colName(colNameLen, ' ');
            buffer.read(&colName[0], colNameLen);

            uint8_t type;
            buffer.read(reinterpret_cast<char*>(&type), sizeof(type));

            uint8_t flags;
            buffer.read(reinterpret_cast<char*>(&flags), sizeof(flags));

            Column col(colName, static_cast<DataType>(type));
            col.indexed = (flags & 1) != 0;
            col.autoIncrement = (flags & 2) != 0;
            col.uniqueIndex = (flags & 4) != 0;
            col.hasDefault = (flags & 8) != 0;

            if (col.autoIncrement) {
                uint32_t savedCounter;
                buffer.read(reinterpret_cast<char*>(&savedCounter), sizeof(savedCounter));
                loadedCounters[colName] = savedCounter;
            }

            if (col.hasDefault) {
                if (col.type == DataType::DOUBLE) {
                    buffer.read(reinterpret_cast<char*>(&col.defaultValue.numValue),
                            sizeof(col.defaultValue.numValue));
                    col.defaultValue.type = DataType::DOUBLE;
                } else {
                    uint32_t strLen;
                    buffer.read(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    std::string str(strLen, ' ');
                    buffer.read(&str[0], strLen);
                    col.defaultValue = Value(str, col.type);
                }
            }
            columns.push_back(col);
        }

        tables[tableName] = Table(tableName, columns);
        Table& table = tables[tableName];

        for (auto& [colName, val] : loadedCounters) {
            table.setAutoIncrementCounters(colName, val);
        }

        uint32_t rowCount;
        buffer.read(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));

        for (uint32_t r = 0; r < rowCount; r++) {
            Row row;
            for (const auto& col : columns) {
                if (col.type == DataType::DOUBLE) {
                    double val;
                    buffer.read(reinterpret_cast<char*>(&val), sizeof(val));
                    row.values.emplace_back(val);
                } else {
                    uint32_t strLen;
                    buffer.read(reinterpret_cast<char*>(&strLen), sizeof(strLen));
                    std::string str(strLen, ' ');
                    buffer.read(&str[0], strLen);
                    row.values.emplace_back(str, col.type);
                }
            }
            table.insertRow(row);
        }
    }
}

Table &Database::getTable(const std::string &tableName) {
    return tables[tableName];
}