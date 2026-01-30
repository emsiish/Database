#ifndef PROEKT_EXPRESSION_H
#define PROEKT_EXPRESSION_H

#include "Table.h"

class Expression {
public:
    virtual ~Expression() {}
    virtual bool evaluate(const Row& row, const Table& table) const = 0;
};

class ComparisonExpression : public Expression {
    std::string colName;
    std::string op;
    Value value;

public:
    ComparisonExpression(std::string& colName, const std::string& op, const Value& value) : colName(colName), op(op), value(value) {}
    bool evaluate(const Row& row, const Table& table) const override {
        const int colIndex = table.getColumnIndex(colName);
        if (colIndex == -1) {
            return false;
        }
        const Value& rowValue = row.values[colIndex];

        if (op == "=") return rowValue == value;
        if (op == "!=") return rowValue != value;
        if (op == ">=") return rowValue >= value;
        if (op == ">") return rowValue > value;
        if (op == "<") return rowValue < value;
        if (op == "<=") return rowValue <= value;

        return false;
    }
};

class LogicalExpression : public Expression {
    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

public:
    LogicalExpression(const std::string& op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right = nullptr) : op(op), left(std::move(left)), right(std::move(right)) {}
    bool evaluate(const Row& row, const Table& table) const override {
        if (op == "NOT") return !left->evaluate(row, table);
        if (op == "AND") return left->evaluate(row, table) && right->evaluate(row, table);
        if (op == "OR") return left->evaluate(row, table) || right->evaluate(row, table);
        return false;
    }
};

#endif //PROEKT_EXPRESSION_H