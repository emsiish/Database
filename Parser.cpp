#include "Database.h"

class Parser {
public:
    static std::vector<std::string> tokenize(const std::string& str) {
        std::vector<std::string> tokens;
        std::string current;
        bool inQuotes = false;

        for (size_t i = 0; i < str.length(); i++) {
            char c = str[i];

            if (c == '"') {
                inQuotes = !inQuotes;
                current += c;
            } else if (inQuotes) {
                current += c;
            } else if (isspace(c) || c == ',' || c == '(' || c == ')' || c == '{' || c == '}' || c == ':') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                if (c == ',' || c == '(' || c == ')' || c == '{' || c == '}' || c == ':') {
                    tokens.emplace_back(1, c);
                }
            } else {
                current += c;
            }
        }

        if (!current.empty()) tokens.push_back(current);
        return tokens;
    }

    static DataType parseDataType(const std::string& str) {
        std::string lower = str;
        std::ranges::transform(lower, lower.begin(), ::tolower);
        if (lower == "int") return DataType::INT;
        if (lower == "string") return DataType::STRING;
        if (lower == "date") return DataType::DATE;
        throw std::runtime_error("Unknown data type: " + str);
    }

    static Value parseValue(const std::string& str, DataType type) {
        if (type == DataType::INT) {
            return Value(stoi(str));
        }
        std::string cleaned = str;
        if (cleaned.front() == '"') cleaned = cleaned.substr(1);
        if (cleaned.back() == '"') cleaned.pop_back();
        return Value(cleaned, type);
    }

    static std::unique_ptr<Expression> parseWhereClause(const std::vector<std::string>& tokens, size_t& pos, const Table& table) {
        if (pos >= tokens.size()) return nullptr;

        std::string token = tokens[pos];
        std::ranges::transform(token, token.begin(), ::toupper);

        if (token == "NOT") {
            pos++;
            auto expr = parseWhereClause(tokens, pos, table);
            return make_unique<LogicalExpression>("NOT", move(expr));
        }

        std::string colName = tokens[pos++];
        if (pos >= tokens.size()) return nullptr;

        std::string op = tokens[pos++];
        if (pos >= tokens.size()) return nullptr;

        int colIdx = table.getColumnIndex(colName);
        if (colIdx == -1) throw std::runtime_error("Unknown column: " + colName);

        Value val = parseValue(tokens[pos++], table.getColumns()[colIdx].type);
        auto left = make_unique<ComparisonExpression>(colName, op, val);

        if (pos < tokens.size()) {
            std::string logOp = tokens[pos];
            std::ranges::transform(logOp, logOp.begin(), ::toupper);

            if (logOp == "AND" || logOp == "OR") {
                pos++;
                auto right = parseWhereClause(tokens, pos, table);
                return make_unique<LogicalExpression>(logOp, move(left), move(right));
            }
        }

        return left;
    }
};