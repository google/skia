/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTHash_DEFINED
#define SkTHash_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTemplates.h"
#include <new>

// Before trying to use SkTHashTable, look below to see if SkTHashMap or SkTHashSet works for you.
// They're easier to use, usually perform the same, and have fewer sharp edges.

// T and K are treated as ordinary copyable C++ types.
// Traits must have:
//   - static K GetKey(T)
//   - static uint32_t Hash(K)
// If the key is large and stored inside T, you may want to make K a const&.
// Similarly, if T is large you might want it to be a pointer.
template <typename T, typename K, typename Traits = T>
class SkTHashTable {
public:
    SkTHashTable() : fCount(0), fCapacity(0) {}
    SkTHashTable(SkTHashTable&& other)
        : fCount(other.fCount)
        , fCapacity(other.fCapacity)
        , fSlots(std::move(other.fSlots)) { other.fCount = other.fCapacity = 0; }

    SkTHashTable& operator=(SkTHashTable&& other) {
        if (this != &other) {
            this->~SkTHashTable();
            new (this) SkTHashTable(std::move(other));
        }
        return *this;
    }

    // Clear the table.
    void reset() { *this = SkTHashTable(); }

    // How many entries are in the table?
    int count() const { return fCount; }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fCapacity * sizeof(Slot); }

    // !!!!!!!!!!!!!!!!!                 CAUTION                   !!!!!!!!!!!!!!!!!
    // set(), find() and foreach() all allow mutable access to table entries.
    // If you change an entry so that it no longer has the same key, all hell
    // will break loose.  Do not do that!
    //
    // Please prefer to use SkTHashMap or SkTHashSet, which do not have this danger.

    // The pointers returned by set() and find() are valid only until the next call to set().
    // The pointers you receive in foreach() are only valid for its duration.

    // Copy val into the hash table, returning a pointer to the copy now in the table.
    // If there already is an entry in the table with the same key, we overwrite it.
    T* set(T val) {
        if (4 * fCount >= 3 * fCapacity) {
            this->resize(fCapacity > 0 ? fCapacity * 2 : 4);
        }
        return this->uncheckedSet(std::move(val));
    }

    // If there is an entry in the table with this key, return a pointer to it.  If not, null.
    T* find(const K& key) const {
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                return nullptr;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                return &s.val;
            }
            index = this->next(index);
        }
        SkASSERT(fCapacity == 0);
        return nullptr;
    }

    // If there is an entry in the table with this key, return it.  If not, null.
    // This only works for pointer type T, and cannot be used to find an nullptr entry.
    T findOrNull(const K& key) const {
        if (T* p = this->find(key)) {
            return *p;
        }
        return nullptr;
    }

    // Remove the value with this key from the hash table.
    void remove(const K& key) {
        SkASSERT(this->find(key));

        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            SkASSERT(!s.empty());
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                fCount--;
                break;
            }
            index = this->next(index);
        }

        // Rearrange elements to restore the invariants for linear probing.
        for (;;) {
            Slot& emptySlot = fSlots[index];
            int emptyIndex = index;
            int originalIndex;
            // Look for an element that can be moved into the empty slot.
            // If the empty slot is in between where an element landed, and its native slot, then
            // move it to the empty slot. Don't move it if its native slot is in between where
            // the element landed and the empty slot.
            // [native] <= [empty] < [candidate] == GOOD, can move candidate to empty slot
            // [empty] < [native] < [candidate] == BAD, need to leave candidate where it is
            do {
                index = this->next(index);
                Slot& s = fSlots[index];
                if (s.empty()) {
                    // We're done shuffling elements around.  Clear the last empty slot.
                    emptySlot = Slot();
                    return;
                }
                originalIndex = s.hash & (fCapacity - 1);
            } while ((index <= originalIndex && originalIndex < emptyIndex)
                     || (originalIndex < emptyIndex && emptyIndex < index)
                     || (emptyIndex < index && index <= originalIndex));
            // Move the element to the empty slot.
            Slot& moveFrom = fSlots[index];
            emptySlot = std::move(moveFrom);
        }
    }

    // Call fn on every entry in the table.  You may mutate the entries, but be very careful.
    template <typename Fn>  // f(T*)
    void foreach(Fn&& fn) {
        for (int i = 0; i < fCapacity; i++) {
            if (!fSlots[i].empty()) {
                fn(&fSlots[i].val);
            }
        }
    }

    // Call fn on every entry in the table.  You may not mutate anything.
    template <typename Fn>  // f(T) or f(const T&)
    void foreach(Fn&& fn) const {
        for (int i = 0; i < fCapacity; i++) {
            if (!fSlots[i].empty()) {
                fn(fSlots[i].val);
            }
        }
    }

private:
    T* uncheckedSet(T&& val) {
        const K& key = Traits::GetKey(val);
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                // New entry.
                s.val  = std::move(val);
                s.hash = hash;
                fCount++;
                return &s.val;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                // Overwrite previous entry.
                // Note: this triggers extra copies when adding the same value repeatedly.
                s.val = std::move(val);
                return &s.val;
            }

            index = this->next(index);
        }
        SkASSERT(false);
        return nullptr;
    }

    void resize(int capacity) {
        int oldCapacity = fCapacity;
        SkDEBUGCODE(int oldCount = fCount);

        fCount = 0;
        fCapacity = capacity;
        SkAutoTArray<Slot> oldSlots = std::move(fSlots);
        fSlots = SkAutoTArray<Slot>(capacity);

        for (int i = 0; i < oldCapacity; i++) {
            Slot& s = oldSlots[i];
            if (!s.empty()) {
                this->uncheckedSet(std::move(s.val));
            }
        }
        SkASSERT(fCount == oldCount);
    }

    int next(int index) const {
        index--;
        if (index < 0) { index += fCapacity; }
        return index;
    }

    static uint32_t Hash(const K& key) {
        uint32_t hash = Traits::Hash(key) & 0xffffffff;
        return hash ? hash : 1;  // We reserve hash 0 to mark empty.
    }

    struct Slot {
        Slot() : val{}, hash(0) {}
        Slot(T&& v, uint32_t h) : val(std::move(v)), hash(h) {}
        Slot(Slot&& o) { *this = std::move(o); }
        Slot& operator=(Slot&& o) {
            val  = std::move(o.val);
            hash = o.hash;
            return *this;
        }

        bool empty() const { return this->hash == 0; }

        T        val;
        uint32_t hash;
    };

    int fCount, fCapacity;
    SkAutoTArray<Slot> fSlots;

    SkTHashTable(const SkTHashTable&) = delete;
    SkTHashTable& operator=(const SkTHashTable&) = delete;
};

// Maps K->V.  A more user-friendly wrapper around SkTHashTable, suitable for most use cases.
// K and V are treated as ordinary copyable C++ types, with no assumed relationship between the two.
template <typename K, typename V, typename HashK = SkGoodHash>
class SkTHashMap {
public:
    SkTHashMap() {}
    SkTHashMap(SkTHashMap&&) = default;
    SkTHashMap& operator=(SkTHashMap&&) = default;

    // Clear the map.
    void reset() { fTable.reset(); }

    // How many key/value pairs are in the table?
    int count() const { return fTable.count(); }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // N.B. The pointers returned by set() and find() are valid only until the next call to set().

    // Set key to val in the table, replacing any previous value with the same key.
    // We copy both key and val, and return a pointer to the value copy now in the table.
    V* set(K key, V val) {
        Pair* out = fTable.set({std::move(key), std::move(val)});
        return &out->val;
    }

    // If there is key/value entry in the table with this key, return a pointer to the value.
    // If not, return null.
    V* find(const K& key) const {
        if (Pair* p = fTable.find(key)) {
            return &p->val;
        }
        return nullptr;
    }

    // Remove the key/value entry in the table with this key.
    void remove(const K& key) {
        SkASSERT(this->find(key));
        fTable.remove(key);
    }

    // Call fn on every key/value pair in the table.  You may mutate the value but not the key.
    template <typename Fn>  // f(K, V*) or f(const K&, V*)
    void foreach(Fn&& fn) {
        fTable.foreach([&fn](Pair* p){ fn(p->key, &p->val); });
    }

    // Call fn on every key/value pair in the table.  You may not mutate anything.
    template <typename Fn>  // f(K, V), f(const K&, V), f(K, const V&) or f(const K&, const V&).
    void foreach(Fn&& fn) const {
        fTable.foreach([&fn](const Pair& p){ fn(p.key, p.val); });
    }

private:
    struct Pair {
        K key;
        V val;
        static const K& GetKey(const Pair& p) { return p.key; }
        static auto Hash(const K& key) { return HashK()(key); }
    };

    SkTHashTable<Pair, K> fTable;

    SkTHashMap(const SkTHashMap&) = delete;
    SkTHashMap& operator=(const SkTHashMap&) = delete;
};

// A set of T.  T is treated as an ordinary copyable C++ type.
template <typename T, typename HashT = SkGoodHash>
class SkTHashSet {
public:
    SkTHashSet() {}
    SkTHashSet(SkTHashSet&&) = default;
    SkTHashSet& operator=(SkTHashSet&&) = default;

    // Clear the set.
    void reset() { fTable.reset(); }

    // How many items are in the set?
    int count() const { return fTable.count(); }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // Copy an item into the set.
    void add(T item) { fTable.set(std::move(item)); }

    // Is this item in the set?
    bool contains(const T& item) const { return SkToBool(this->find(item)); }

    // If an item equal to this is in the set, return a pointer to it, otherwise null.
    // This pointer remains valid until the next call to add().
    const T* find(const T& item) const { return fTable.find(item); }

    // Remove the item in the set equal to this.
    void remove(const T& item) {
        SkASSERT(this->contains(item));
        fTable.remove(item);
    }

    // Call fn on every item in the set.  You may not mutate anything.
    template <typename Fn>  // f(T), f(const T&)
    void foreach (Fn&& fn) const {
        fTable.foreach(fn);
    }

private:
    struct Traits {
        static const T& GetKey(const T& item) { return item; }
        static auto Hash(const T& item) { return HashT()(item); }
    };
    SkTHashTable<T, T, Traits> fTable;

    SkTHashSet(const SkTHashSet&) = delete;
    SkTHashSet& operator=(const SkTHashSet&) = delete;
};

#endif//SkTHash_DEFINED
