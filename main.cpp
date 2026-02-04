#include "Parser.h"
#include "Database.h"

void printPrompt() {
    std::cout << "FMISql> ";
}

bool isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    bool hasDot = false;
    while (it != s.end()) {
        if (*it == '.') {
            if (hasDot) {
                return false;
            }
            hasDot = true;
        } else if (!isdigit(*it)) {
            return false;
        }

        ++it;
    }
    return !s.empty() && it == s.end();
}

void processCommand(Database& db, const std::string& command) {
    auto tokens = Parser::tokenize(command);
    if (tokens.empty()) return;

    std::string cmd = tokens[0];
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    try {
        if (cmd == "CREATETABLE") {
            const std::string& tableName = tokens[1];
            std::vector<Column> columns;

            size_t i = 3;
            while (i < tokens.size() && tokens[i] != ")") {
                const std::string& colName = tokens[i++];
                if (tokens[i++] != ":") throw std::runtime_error("Expected ':'");

                const std::string& typeStr = tokens[i++];
                DataType type = Parser::parseDataType(typeStr);

                Column col(colName, type);
                if (i < tokens.size() && tokens[i] != "," && tokens[i] != ")") {
                    std::string attr = tokens[i];
                    transform(attr.begin(), attr.end(), attr.begin(), ::toupper);

                    if (attr == "DEFAULT") {
                        i++;
                        col.hasDefault = true;
                        col.defaultValue = Parser::parseValue(tokens[i++], col.type);
                    }

                    if (attr == "AUTOINCREMENT") {
                        col.autoIncrement = true;
                        i++;
                    }
                }
                columns.push_back(col);

                if (i < tokens.size() && tokens[i] == ",") i++;
            }

            i++;
            if (i < tokens.size()) {
                std::string indexToken = tokens[i];
                transform(indexToken.begin(), indexToken.end(), indexToken.begin(), ::toupper);
                if (indexToken == "INDEX") {
                    i += 2;
                    const std::string& indexCol = tokens[i];
                    for (auto& col : columns) {
                        if (col.name == indexCol) col.indexed = true;
                    }
                }
            }

        db.createTable(tableName, columns);

        } else if (cmd == "DROPTABLE") {
            db.dropTable(tokens[1]);

        } else if (cmd == "LISTTABLES") {
            db.listTables();

        } else if (cmd == "TABLEINFO") {
            db.tableInfo(tokens[1]);

        } else if (cmd == "INSERT") {
            std::vector<Row> rows;
            size_t i = 4;
            while (i < tokens.size() && tokens[i] != "}") {
                if (tokens[i] == "(") {
                    Row row;
                    i++;
                    while (tokens[i] != ")") {
                        if (tokens[i] == ",") i++;

                        const std::string& valToken = tokens[i++];
                        std::string upperToken = valToken;
                        transform(upperToken.begin(), upperToken.end(), upperToken.begin(), ::toupper);

                        if (upperToken == "DEFAULT") {
                            Value defaultMarker;
                            defaultMarker.strValue = "__INTERNAL_DEFAULT__";
                            row.values.push_back(defaultMarker);
                        } else if (isNumber(valToken)) {
                            row.values.emplace_back(std::stod(valToken));
                        } else {
                            std::string clean = valToken.substr(1, valToken.size() - 2);
                            row.values.emplace_back(clean);
                        }
                    }

                    rows.emplace_back(row);
                }
                i++;
            }
            db.insert(tokens[2], rows);

        } else if (cmd == "REMOVE") {
            size_t i = 1;

            if (i < tokens.size()) {
                std::string upper = tokens[i];
                std::ranges::transform(upper, upper.begin(), ::toupper);
                if (upper == "FROM") i++;
            }

            if (i >= tokens.size()) throw std::runtime_error("Expected table name after REMOVE");
            const std::string& tableName = tokens[i++];

            std::unique_ptr<Expression> whereExpr = nullptr;
            Table& table = db.getTable(tableName);

            if (i < tokens.size()) {
                std::string upper = tokens[i];
                std::ranges::transform(upper, upper.begin(), ::toupper);

                if (upper == "WHERE") {
                    i++;
                    whereExpr = Parser::parseWhereExpression(tokens, i, table);
                }
            }

            db.remove(tableName, std::move(whereExpr));

        } else if (cmd == "SELECT") {
            bool isDistinct = false;
            std::vector<std::string> colNames;
            size_t i = 1;

            if (i < tokens.size()) {
                std::string firstToken = tokens[i];
                transform(firstToken.begin(), firstToken.end(), firstToken.begin(), ::toupper);
                if (firstToken == "DISTINCT") {
                    isDistinct = true;
                    i++;
                }
            }

            while (i < tokens.size() && tokens[i] != "FROM") {
                std::string upper = tokens[i];
                transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                if (upper != "FROM" && tokens[i] != ",") {
                    colNames.push_back(tokens[i]);
                }
                i++;
            }

            i++;
            const std::string& tableName = tokens[i++];

            std::unique_ptr<Expression> whereExpr = nullptr;
            std::string orderByCol;
            Table& table = db.getTable(tableName);
            while (i < tokens.size()) {
                std::string upper = tokens[i];
                transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

                if (upper == "WHERE") {
                    i++;
                    whereExpr = Parser::parseWhereExpression(tokens, i, table);
                } else if (upper == "ORDER") {
                    i += 2;
                    orderByCol = tokens[i++];
                } else if (upper == "DISTINCT") {
                    i++;
                }
            }

            db.select(tableName, colNames, std::move(whereExpr), orderByCol, isDistinct);

        } else if (cmd == "QUIT" || cmd == "EXIT") {
            std::cout << "Goodbye" << std::endl;
            exit(0);

        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    Database db;
    std::string command;

    std::cout << "FMI Database Management System v1.0" << std::endl;
    std::cout << "Type 'quit' to exit" << std::endl << std::endl;

    while (true) {
        printPrompt();
        getline(std::cin, command);

        if (!command.empty()) {
            processCommand(db, command);
        }
    }
    return 0;
}