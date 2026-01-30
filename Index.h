#ifndef PROEKT_INDEX_H
#define PROEKT_INDEX_H
#include <map>

#include "Data.h"

class Index {
    std::map<Value, std::size_t> uniqueIndices; // value -> single row
    std::multimap<Value, std::size_t> nonUniqueIndexes; //value -> multipleRows
    bool isUnique;

public:
    Index(bool isUnique = false) : isUnique(isUnique) {}

    void insert(const Value& val, size_t rowIdx);
    void remove(const Value& val, size_t rowIdx);
    std::vector<size_t> find(const Value& val) const;
    void clear();
    bool getIsUnique() const;
};

#endif //PROEKT_INDEX_H