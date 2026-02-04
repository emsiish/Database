#include "Table.h"

Table::Table(std::string  name, const std::vector<Column>& columns) : name(std::move(name)), columns(columns) {
    for (const auto& col : columns) {
        if (col.indexed) {
            indices[col.name] = Index(col.uniqueIndex);
        }
        if (col.autoIncrement) {
            autoIncrementCounters[col.name] = 1;
        }
    }
}

int Table::getColumnIndex(const std::string &name) const {
    for (std::size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == name) {
            return i;
        }
    }
    return -1;
}


void Table::insertRow(Row &row) {
    Row finalRow;
    for (size_t i = 0; i < columns.size(); ++i) {
        const Column& col = columns[i];

        bool valueProvided = i < row.values.size();
        bool useAutoValue = !valueProvided ||
                            row.values[i].strValue == "__INTERNAL_DEFAULT__" ||
                            (col.autoIncrement && row.values[i].type == DataType::DOUBLE && row.values[i].numValue == 0);

        if (useAutoValue) {
            if (col.autoIncrement) {
                finalRow.values.emplace_back(static_cast<double>(autoIncrementCounters[col.name]++));
            } else if (col.hasDefault) {
                finalRow.values.push_back(col.defaultValue);
            } else {
                finalRow.values.push_back(col.type == DataType::DOUBLE ? Value(0.0) : Value("", col.type));
            }
        } else {
            finalRow.values.push_back(row.values[i]);
            if (col.autoIncrement && row.values[i].type == DataType::DOUBLE) {
                if (row.values[i].numValue >= autoIncrementCounters[col.name]) {
                    autoIncrementCounters[col.name] = row.values[i].numValue + 1;
                }
            }
        }
    }

    rows.push_back(finalRow);
    const std::size_t rowIdx = rows.size() - 1;

    for (std::size_t i = 0; i < columns.size(); i++) {
        if (columns[i].indexed) {
            indices[columns[i].name].insert(finalRow.values[i], rowIdx);
        }
    }
}

void Table::removeRow(std::size_t rowIdx) {
    if (rowIdx >= rows.size()) return;

    rows.erase(rows.begin() + rowIdx);

    for (auto& [colName, index] : indices) {
        index.clear();

        int colIdx = getColumnIndex(colName);
        if (colIdx == -1) continue;

        for (std::size_t i = 0; i < rows.size(); i++) {
            index.insert(rows[i].values[colIdx], i);
        }
    }
}

std::vector<Column> Table::getColumns() const {
    return columns;
}

std::vector<Row> Table::getRows() const {
    return rows;
}

std::string Table::getName() const {
    return name;
}

std::size_t Table::getDataSize() const {
    size_t size = 0;
    for (const auto& row : rows) {
        for (const auto& val : row.values) {
            if (val.type == DataType::DOUBLE) size += sizeof(double);
            else size += val.strValue.size();
        }
    }
    return size;
}

std::map<std::string, int> Table::getAutoIncrementCounters() const {
    return autoIncrementCounters;
}

void Table::setAutoIncrementCounters(const std::string &colName, const int &value) {
    autoIncrementCounters[colName] = value;
}