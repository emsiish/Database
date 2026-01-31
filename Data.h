#ifndef PROEKT_DATA_H
#define PROEKT_DATA_H
#include <string>
#include <utility>
#include <vector>

enum class DataType { INT, STRING, DATE };

inline std::string dataTypeToString(const DataType type) {
    switch(type) {
        case DataType::INT: return "Int";
        case DataType::STRING: return "String";
        case DataType::DATE: return "Date";
    }
    return "Unknown";
}

struct Value {
    //TODO: convert int to double
    DataType type;
    double intValue;
    std::string strValue;

    static constexpr double epsilon = 1e-5;

    Value() : type(DataType::INT), intValue(0) {}
    Value(const double value) : type(DataType::INT), intValue(value) {}
    Value(std::string value, const DataType type = DataType::STRING) : type(type), intValue(0), strValue(std::move(value)) {}

    std::string toString() const {
        if (type == DataType::INT) return std::to_string(intValue);
        return "\"" + strValue + "\"";
    }

    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        return (type == DataType::INT) ? std::abs(intValue - other.intValue) < epsilon : (strValue == other.strValue);
    }

    bool operator<(const Value& other) const {
        if (type != other.type) return false;
        return (type == DataType::INT) ? (intValue < other.intValue) : (strValue < other.strValue);
    }

    bool operator!=(const Value& other) const { return !(*this == other); }
    bool operator>(const Value& other) const { return other < *this; }
    bool operator<=(const Value& other) const { return !(other < *this); }
    bool operator>=(const Value& other) const { return !(*this < other); }
};

struct Column {
    std::string name;
    DataType type;
    bool hasDefault;
    Value defaultValue;
    bool autoIncrement;
    bool indexed;
    bool uniqueIndex;

    Column() : type(DataType::INT), hasDefault(false), autoIncrement(false), indexed(false), uniqueIndex(false) {}
    Column(std::string  name, const DataType type, const bool indexed = false, const bool uniqueIndex = false)
        : name(std::move(name)), type(type), hasDefault(false), autoIncrement(false), indexed(indexed), uniqueIndex(uniqueIndex) {}
};

struct Row {
    std::vector<Value> values;

    Row() = default;
    Row(const std::vector<Value>& values) : values(values) {}

    bool operator==(const Row& other) const {
        return values == other.values;
    }

    bool operator<(const Row& other) const {
        return values < other.values;
    }

    bool operator!=(const Row& other) const { return !(*this == other); }
    bool operator>(const Row& other) const { return other < *this; }
    bool operator<=(const Row& other) const { return !(other < *this); }
    bool operator>=(const Row& other) const { return !(*this < other); }
};

#endif //PROEKT_DATA_H