#ifndef PROEKT_DATABASE_H
#define PROEKT_DATABASE_H

#include "Expression.h"

class Database {
    std::map<std::string, Table> tables;
    std::string dbPath;

    void saveToDisk();
    void loadFromDisk();

public:
    Database(const std::string& dbPath = "fmisql.db") : dbPath(dbPath) {
        loadFromDisk();
    }
    ~Database() {
        saveToDisk();
    }

    void createTable(const std::string& tableName, const std::vector<Column>& columnNames);
    void dropTable(const std::string& tableName);
    void listTables() const;
    void tableInfo(const std::string& tableName);
    void select(const std::string& tableName, std::vector<std::string>& columnNames);
    void insert(const std::string& tableName, std::vector<Row>& rows);
    void remove(const std::string& tableName, std::unique_ptr<Expression> whereExpr); //TODO: add where expression
};

#endif //PROEKT_DATABASE_H