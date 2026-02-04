#include "Index.h"

void Index::insert(const Value &val, size_t rowIdx) {
    if (isUnique) {
        if (uniqueIndices.contains(val)) {
            throw std::logic_error("Unique index already exists");
        }
        uniqueIndices[val] = rowIdx;
    } else {
        nonUniqueIndexes.insert({val, rowIdx});
    }
}

void Index::remove(const Value &val, size_t rowIdx) {
    if (isUnique) {
        uniqueIndices.erase(val);
    } else {
        auto range = nonUniqueIndexes.equal_range(val);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == rowIdx) {
                nonUniqueIndexes.erase(it);
                break;
            }
        }
    }
}

std::vector<size_t> Index::find(const Value &val) const {
    std::vector<size_t> result;

    if (isUnique) {
        auto it = uniqueIndices.find(val);
        if (it != uniqueIndices.end()) {
            result.push_back(it->second);
        }
    } else {
        auto range = nonUniqueIndexes.equal_range(val);
        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(it->second);
        }
    }
    return result;
}

void Index::clear() {
    uniqueIndices.clear();
    nonUniqueIndexes.clear();
}

bool Index::getIsUnique() const {
    return isUnique;
}