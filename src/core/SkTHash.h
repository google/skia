#ifndef SkTHash_DEFINED
#define SkTHash_DEFINED

#include "SkTypes.h"
#include "SkTemplates.h"

// Before trying to use SkTHashTable, look below to see if SkTHashMap or SkTHashSet works for you.
// They're easier to use, usually perform the same, and have fewer sharp edges.

// T and K are treated as ordinary copyable C++ types.
// Traits must have:
//   - static K GetKey(T)
//   - static uint32_t Hash(K)
// If the key is large and stored inside T, you may want to make K a const&.
// Similarly, if T is large you might want it to be a pointer.
template <typename T, typename K, typename Traits = T>
class SkTHashTable : SkNoncopyable {
public:
    SkTHashTable() : fCount(0), fCapacity(0) {}

    // Clear the table.
    void reset() {
        this->~SkTHashTable();
        SkNEW_PLACEMENT(this, SkTHashTable);
    }

    // How many entries are in the table?
    int count() const { return fCount; }

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
    T* set(const T& val) {
        if (4 * fCount >= 3 * fCapacity) {
            this->resize(fCapacity > 0 ? fCapacity * 2 : 4);
        }
        return this->uncheckedSet(val);
    }

    // If there is an entry in the table with this key, return a pointer to it.  If not, NULL.
    T* find(const K& key) const {
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                return NULL;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                return &s.val;
            }
            index = this->next(index, n);
        }
        SkASSERT(fCapacity == 0);
        return NULL;
    }

    // Call fn on every entry in the table.  You may mutate the entries, but be very careful.
    template <typename Arg>
    void foreach(void(*fn)(T*, Arg), Arg arg) {
        for (int i = 0; i < fCapacity; i++) {
            Slot& s = fSlots[i];
            if (!s.empty()) {
                fn(&s.val, arg);
            }
        }
    }

private:
    T* uncheckedSet(const T& val) {
        const K& key = Traits::GetKey(val);
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                // New entry.
                s.val  = val;
                s.hash = hash;
                fCount++;
                return &s.val;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                // Overwrite previous entry.
                // Note: this triggers extra copies when adding the same value repeatedly.
                s.val = val;
                return &s.val;
            }
            index = this->next(index, n);
        }
        SkASSERT(false);
        return NULL;
    }

    void resize(int capacity) {
        int oldCapacity = fCapacity;
        SkDEBUGCODE(int oldCount = fCount);

        fCount = 0;
        fCapacity = capacity;
        SkAutoTArray<Slot> oldSlots(capacity);
        oldSlots.swap(fSlots);

        for (int i = 0; i < oldCapacity; i++) {
            const Slot& s = oldSlots[i];
            if (!s.empty()) {
                this->uncheckedSet(s.val);
            }
        }
        SkASSERT(fCount == oldCount);
    }

    int next(int index, int n) const {
        // A valid strategy explores all slots in [0, fCapacity) as n walks from 0 to fCapacity-1.
        // Both of these strategies are valid:
        //return (index + 0 + 1) & (fCapacity-1);      // Linear probing.
        return (index + n + 1) & (fCapacity-1);        // Quadratic probing.
    }

    static uint32_t Hash(const K& key) {
        uint32_t hash = Traits::Hash(key);
        return hash == 0 ? 1 : hash;  // We reserve hash == 0 to mark empty slots.
    }

    struct Slot {
        Slot() : hash(0) {}
        bool empty() const { return hash == 0; }

        T val;
        uint32_t hash;
    };

    int fCount, fCapacity;
    SkAutoTArray<Slot> fSlots;
};

// Maps K->V.  A more user-friendly wrapper around SkTHashTable, suitable for most use cases.
// K and V are treated as ordinary copyable C++ types, with no assumed relationship between the two.
template <typename K, typename V, uint32_t(*HashK)(const K&)>
class SkTHashMap : SkNoncopyable {
public:
    SkTHashMap() {}

    // Clear the map.
    void reset() { fTable.reset(); }

    // How many key/value pairs are in the table?
    int count() const { return fTable.count(); }

    // N.B. The pointers returned by set() and find() are valid only until the next call to set().

    // Set key to val in the table, replacing any previous value with the same key.
    // We copy both key and val, and return a pointer to the value copy now in the table.
    V* set(const K& key, const V& val) {
        Pair in = { key, val };
        Pair* out = fTable.set(in);
        return &out->val;
    }

    // If there is key/value entry in the table with this key, return a pointer to the value.
    // If not, return NULL.
    V* find(const K& key) const {
        if (Pair* p = fTable.find(key)) {
            return &p->val;
        }
        return NULL;
    }

    // Call fn on every key/value pair in the table.  You may mutate the value but not the key.
    void foreach(void(*fn)(K, V*)) { fTable.foreach(ForEach, fn); }

private:
    struct Pair {
        K key;
        V val;
        static const K& GetKey(const Pair& p) { return p.key; }
        static uint32_t Hash(const K& key) { return HashK(key); }
    };
    static void ForEach(Pair* p, void (*fn)(K, V*)) { fn(p->key, &p->val); }

    SkTHashTable<Pair, K> fTable;
};

// A set of T.  T is treated as an ordiary copyable C++ type.
template <typename T, uint32_t(*HashT)(const T&)>
class SkTHashSet : SkNoncopyable {
public:
    SkTHashSet() {}

    // Clear the set.
    void reset() { fTable.reset(); }

    // How many items are in the set?
    int count() const { return fTable.count(); }

    // Copy an item into the set.
    void add(const T& item) { fTable.set(item); }

    // Is this item in the set?
    bool contains(const T& item) const { return SkToBool(fTable.find(item)); }

private:
    struct Traits {
        static const T& GetKey(const T& item) { return item; }
        static uint32_t Hash(const T& item) { return HashT(item); }
    };
    SkTHashTable<T, T, Traits> fTable;
};

#endif//SkTHash_DEFINED
