#ifndef PROEKT_TABLE_H
#define PROEKT_TABLE_H

#include <cstdint>
#include <utility>
#include "Index.h"

class Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::map<std::string, Index> indices; //column name -> index
    std::map<std::string, int> autoIncrementCounters;

public:
    Table() = default;
    Table(std::string  name, const std::vector<Column>& columns);

    int getColumnIndex(const std::string& name) const;
    void insertRow(Row& row);
    void removeRow(std::size_t rowIdx);
    std::vector<Column> getColumns() const;
    std::vector<Row> getRows() const;
    std::string getName() const;
    std::size_t getDataSize() const;
    std::map<std::string, int> getAutoIncrementCounters() const;
    void setAutoIncrementCounters(const std::string& colName, const int& value);
};

#endif //PROEKT_TABLE_H