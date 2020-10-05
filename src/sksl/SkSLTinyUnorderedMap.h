/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TINYUNORDEREDMAP
#define SKSL_TINYUNORDEREDMAP

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace SkSL {

/** The TinyUnorderedMap presents a minimal, unordered_map-like interface, but is backed by a normal
    vector. No attempt is made to be clever during lookups or insertions. This is intended to be
    used in cases where we have tiny amount of data (e.g. less than 50 elements); CPUs are really
    good at scanning contiguous cachelines. */
template <typename K, typename V>
class TinyUnorderedMap {
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    iterator begin() { return fData.begin(); }
    const_iterator begin() const { return fData.begin(); }

    iterator end() { return fData.end(); }
    const_iterator end() const { return fData.end(); }

    void clear() { fData.clear(); }
    bool empty() { return fData.empty(); }
    size_t size() { return fData.size(); }
    void reserve(size_t amount) { fData.reserve(amount); }

    iterator find(const K& key) {
        return std::find_if(fData.begin(), fData.end(),
                            [&](const value_type& a) { return a.first == key; });
    }
    const_iterator find(const K& key) const {
        return std::find_if(fData.begin(), fData.end(),
                            [&](const value_type& a) { return a.first == key; });
    }

    V& operator[](const K& key) {
        auto iter = this->find(key);
        if (iter != fData.end()) {
            return iter->second;
        }
        fData.push_back(value_type{key, V{}});
        return fData.back().second;
    }

private:
    std::vector<value_type> fData;
};

}  // namespace SkSL

#endif
