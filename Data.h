#ifndef PROEKT_DATA_H
#define PROEKT_DATA_H
#include <string>
#include <utility>
#include <vector>

enum class DataType { DOUBLE, STRING, DATE };

inline std::string dataTypeToString(const DataType type) {
    switch(type) {
        case DataType::DOUBLE: return "Double";
        case DataType::STRING: return "String";
        case DataType::DATE: return "Date";
    }
    return "Unknown";
}

struct Value {
    DataType type;
    double numValue;
    std::string strValue;

    static constexpr double epsilon = 1e-5;

    Value() : type(DataType::DOUBLE), numValue(0) {}
    Value(const double value) : type(DataType::DOUBLE), numValue(value) {}
    Value(std::string value, const DataType type = DataType::STRING) : type(type), numValue(0), strValue(std::move(value)) {}

    std::string toString() const {
        if (type == DataType::DOUBLE) {
            std::string s = std::to_string(numValue);
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') s.pop_back();
            return s;
        }
        return "\"" + strValue + "\"";
    }

    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        return (type == DataType::DOUBLE) ? std::abs(numValue - other.numValue) < epsilon : (strValue == other.strValue);
    }

    bool operator<(const Value& other) const {
        if (type != other.type) {
            if (!((type == DataType::STRING || type == DataType::DATE) &&
                  (other.type == DataType::STRING || other.type == DataType::DATE))) {
                return false;
                  }
        }
        return (type == DataType::DOUBLE) ? (numValue < other.numValue) : (strValue < other.strValue);
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

    Column() : type(DataType::DOUBLE), hasDefault(false), autoIncrement(false), indexed(false), uniqueIndex(false) {}
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