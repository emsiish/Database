#include "Table.h"

Table::Table(const std::string& name, const std::vector<Column>& columns) : name(name), columns(columns) {
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
    for (std::size_t i = 0; i < columns.size(); i++) {
        if (columns[i].autoIncrement) {
            row.values[i] = Value(autoIncrementCounters[columns[i].name]++);
        }
    }
    rows.push_back(row);
    const std::size_t rowIdx = rows.size() - 1;

    for (std::size_t i = 0; i < columns.size(); i++) {
        if (columns[i].indexed) {
            indices[columns[i].name].insert(row.values[i], rowIdx);
        }
    }
}

void Table::removeRow(std::size_t rowIdx) {
    for (std::size_t i = 0; i < rows.size(); i++) {
        if (columns[i].indexed) {
            indices[columns[i].name].remove(rows[rowIdx].values[i], rowIdx);
        }
    }
    rows.erase(rows.begin() + rowIdx);

    for (auto& idx : indices) {
        const int colIdx = getColumnIndex(idx.first);
        idx.second.clear();
        for (std::size_t j = 0; j < rows.size(); j++) {
            idx.second.insert(rows[j].values[colIdx], j);
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
            if (val.type == DataType::INT) size += sizeof(double);
            else size += val.strValue.size();
        }
    }
    return size;
}



