/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/lex/DFA.h"
#include "src/sksl/lex/TransitionTable.h"

#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

// The number of bits to use per entry in our compact transition table. This is customizable:
// - 1-bit: reasonable in theory. Doesn't actually pack many slices.
// - 2-bit: best fit for our data. Packs extremely well.
// - 4-bit: packs all but one slice, but doesn't save as much space overall.
// - 8-bit: way too large (an 8-bit LUT plus an 8-bit data table is as big as a 16-bit table)
// Other values don't divide cleanly into a byte and do not work.
constexpr int kNumBits = 2;

// These values are derived from kNumBits and shouldn't need to change.
constexpr int kNumValues = (1 << kNumBits) - 1;
constexpr int kDataPerByte = 8 / kNumBits;

enum IndexType {
    kZero = 0,
    kFullEntry,
    kCompactEntry,
};
struct IndexEntry {
    IndexType type;
    int pos;
};
struct CompactEntry {
    std::array<int, kNumValues> v = {};
    std::vector<int> data;
};
struct FullEntry {
    std::vector<int> data;
};

using TransitionSet = std::unordered_set<int>;

static int add_compact_entry(const TransitionSet& transitionSet,
                             const std::vector<int>& data,
                             std::vector<CompactEntry>* entries) {
    // Create a compact entry with the unique values from the transition set, padded out with zeros
    // and sorted.
    CompactEntry result{};
    assert(transitionSet.size() <= result.v.size());
    std::copy(transitionSet.begin(), transitionSet.end(), result.v.begin());
    std::sort(result.v.begin(), result.v.end());

    // Create a mapping from real values to small values. (0 -> 0, v[0] -> 1, v[1] -> 2, v[2] -> 3)
    std::unordered_map<int, int> translationTable;
    for (size_t index = 0; index < result.v.size(); ++index) {
        translationTable[result.v[index]] = 1 + index;
    }
    translationTable[0] = 0;

    // Convert the real values into small values.
    for (size_t index = 0; index < data.size(); ++index) {
        int value = data[index];
        assert(translationTable.find(value) != translationTable.end());
        result.data.push_back(translationTable[value]);
    }

    // Look for an existing entry that exactly matches this one.
    for (size_t index = 0; index < entries->size(); ++index) {
        if (entries->at(index).v == result.v && entries->at(index).data == result.data) {
            return index;
        }
    }

    // Add this as a new entry.
    entries->push_back(std::move(result));
    return (int)(entries->size() - 1);
}

static int add_full_entry(const TransitionSet& transitionMap,
                          const std::vector<int>& data,
                          std::vector<FullEntry>* entries) {
    // Create a full entry with this data.
    FullEntry result{};
    result.data = std::vector<int>(data.begin(), data.end());

    // Look for an existing entry that exactly matches this one.
    for (size_t index = 0; index < entries->size(); ++index) {
        if (entries->at(index).data == result.data) {
            return index;
        }
    }

    // Add this as a new entry.
    entries->push_back(std::move(result));
    return (int)(entries->size() - 1);
}

}  // namespace

void WriteTransitionTable(std::ofstream& out, const DFA& dfa, size_t states) {
    int numTransitions = dfa.fTransitions.size();

    // Assemble our compact and full data tables, and an index into them.
    std::vector<CompactEntry> compactEntries;
    std::vector<FullEntry> fullEntries;
    std::vector<IndexEntry> indices;
    for (size_t s = 0; s < states; ++s) {
        // Copy all the transitions for this state into a flat array, and into a histogram (counting
        // the number of unique state-transition values). Most states only transition to a few
        // possible new states.
        TransitionSet transitionSet;
        std::vector<int> data(numTransitions);
        for (int t = 0; t < numTransitions; ++t) {
            if ((size_t) t < dfa.fTransitions.size() && s < dfa.fTransitions[t].size()) {
                int value = dfa.fTransitions[t][s];
                assert(value >= 0 && value < (int)states);
                data[t] = value;
                transitionSet.insert(value);
            }
        }

        transitionSet.erase(0);
        if (transitionSet.empty()) {
            // This transition table was completely empty (every value was zero). No data needed;
            // zero pages are handled as a special index type.
            indices.push_back(IndexEntry{kZero, 0});
        } else if (transitionSet.size() <= kNumValues) {
            // This table only contained a small number of unique nonzero values.
            // Use a compact representation that squishes each value down to a few bits.
            int index = add_compact_entry(transitionSet, data, &compactEntries);
            indices.push_back(IndexEntry{kCompactEntry, index});
        } else {
            // This table contained a large number of values. We can't compact it.
            int index = add_full_entry(transitionSet, data, &fullEntries);
            indices.push_back(IndexEntry{kFullEntry, index});
        }
    }

    // Find the largest value for each compact-entry slot.
    int maxValue[kNumValues] = {};
    for (const CompactEntry& entry : compactEntries) {
        for (int index=0; index < kNumValues; ++index) {
            maxValue[index] = std::max(maxValue[index], entry.v[index]);
        }
    }

    // Emit all the structs our transition table will use.
    out << "struct IndexEntry {\n"
        << "    uint16_t type : 2;\n"
        << "    uint16_t pos : 14;\n"
        << "};\n"
        << "struct FullEntry {\n"
        << "    State data[" << numTransitions << "];\n"
        << "};\n";

    // Emit the compact-entry structure; minimize the number of bits needed per value.
    out << "struct CompactEntry {\n";
    for (int index=0; index < kNumValues; ++index) {
        if (maxValue[index] > 0) {
            out << "    State v" << index << " : " << int(std::ceil(std::log2(maxValue[index])))
                << ";\n";
        }
    }

    out << "    uint8_t data[" << std::ceil(float(numTransitions) / float(kDataPerByte)) << "];\n"
        << "};\n";

    // Emit the full-table data.
    out << "static constexpr FullEntry kFull[] = {\n";
    for (const FullEntry& entry : fullEntries) {
        out << "    {";
        for (int value : entry.data) {
            out << value << ", ";
        }
        out << "},\n";
    }
    out << "};\n";

    // Emit the compact-table data.
    out << "static constexpr CompactEntry kCompact[] = {\n";
    for (const CompactEntry& entry : compactEntries) {
        out << "    {";
        for (int index=0; index < kNumValues; ++index) {
            if (maxValue[index] > 0) {
                out << entry.v[index] << ", ";
            }
        }
        out << "{";
        unsigned int shiftBits = 0, combinedBits = 0;
        for (int index = 0; index < numTransitions; index++) {
            combinedBits |= entry.data[index] << shiftBits;
            shiftBits += kNumBits;
            assert(shiftBits <= 8);
            if (shiftBits == 8) {
                out << combinedBits << ", ";
                shiftBits = 0;
                combinedBits = 0;
            }
        }
        if (shiftBits > 0) {
            // Flush any partial values.
            out << combinedBits;
        }
        out << "}},\n";
    }
    out << "};\n"
        << "static constexpr IndexEntry kIndices[] = {\n";
    for (const IndexEntry& entry : indices) {
        out << "    {" << entry.type << ", " << entry.pos << "},\n";
    }
    out << "};\n"
        << "State get_transition(int transition, int state) {\n"
        << "    IndexEntry index = kIndices[state];\n"
        << "    if (index.type == 0) { return 0; }\n"
        << "    if (index.type == 1) { return kFull[index.pos].data[transition]; }\n"
        << "    const CompactEntry& entry = kCompact[index.pos];\n"
        << "    int value = entry.data[transition >> " << std::log2(kDataPerByte) << "];\n"
        << "    value >>= " << kNumBits << " * (transition & " << kDataPerByte - 1 << ");\n"
        << "    value &= " << kNumValues << ";\n"
        << "    State table[] = {0";

    for (int index=0; index < kNumValues; ++index) {
        if (maxValue[index] > 0) {
            out << ", entry.v" << index;
        } else {
            out << ", 0";
        }
    }

    out << "};\n"
        << "    return table[value];\n"
        << "}\n";
}
