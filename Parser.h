#ifndef PROEKT_PARSER_H
#define PROEKT_PARSER_H

#include <chrono>
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
        if (lower == "double") return DataType::DOUBLE;
        if (lower == "string") return DataType::STRING;
        if (lower == "date") return DataType::DATE;
        throw std::runtime_error("Unknown data type: " + str);
    }

    static Value parseValue(const std::string& str, DataType type) {
        if (type == DataType::DOUBLE) {
            return Value(stod(str));
        }
        std::string cleaned = str;
        if (cleaned.front() == '"') cleaned = cleaned.substr(1);
        if (cleaned.back() == '"') cleaned.pop_back();
        return Value(cleaned, type);
    }

    static std::unique_ptr<Expression> parseWhereExpression(const std::vector<std::string>& tokens, size_t& pos, const Table& table) {
        return parseOr(tokens, pos, table);
    }

    static std::unique_ptr<Expression> parseOr(const std::vector<std::string>& tokens, size_t& pos, const Table& table) {
        auto left = parseAnd(tokens, pos, table);

        while (pos < tokens.size()) {
            std::string op = tokens[pos];
            std::ranges::transform(op, op.begin(), ::toupper);

            if (op != "OR") break;

            pos++;
            auto right = parseAnd(tokens, pos, table);
            left = std::make_unique<LogicalExpression>("OR", std::move(left), std::move(right));
        }
        return left;
    }

    static std::unique_ptr<Expression> parseAnd(const std::vector<std::string>& tokens, size_t& pos, const Table& table) {
        auto left = parsePrimary(tokens, pos, table);

        while (pos < tokens.size()) {
            std::string op = tokens[pos];
            std::ranges::transform(op, op.begin(), ::toupper);

            if (op != "AND") break;

            pos++;
            auto right = parsePrimary(tokens, pos, table);
            left = std::make_unique<LogicalExpression>("AND", std::move(left), std::move(right));
        }
        return left;
    }

    static std::unique_ptr<Expression> parsePrimary(const std::vector<std::string>& tokens, size_t& pos, const Table& table) {
        if (pos >= tokens.size()) return nullptr;

        std::string token = tokens[pos];
        std::string upperToken = token;
        std::ranges::transform(upperToken, upperToken.begin(), ::toupper);

        if (upperToken == "NOT") {
            pos++;
            auto expr = parsePrimary(tokens, pos, table);
            return std::make_unique<LogicalExpression>("NOT", std::move(expr));
        }

        if (token == "(") {
            pos++;
            auto expr = parseOr(tokens, pos, table);
            if (pos < tokens.size() && tokens[pos] == ")") pos++;
            return expr;
        }

        std::string colName = tokens[pos++];
        std::string op = tokens[pos++];

        int colIdx = table.getColumnIndex(colName);
        if (colIdx == -1) throw std::runtime_error("Unknown column: " + colName);

        Value val = parseValue(tokens[pos++], table.getColumns()[colIdx].type);
        return std::make_unique<ComparisonExpression>(colName, op, val);
    }
};
#endif //PROEKT_PARSER_H