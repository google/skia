/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMetaData_DEFINED
#define SkMetaData_DEFINED

#include "include/private/SkTHash.h"

#include <cstdint>
#include <vector>

typedef float SkScalar;

class SkMetaData {
public:
    SkMetaData() = default;
    ~SkMetaData();

    #define SK_METADATA_TYPES(M) \
        M(int32_t,  kS32)     \
        M(uint32_t, kU32)     \
        M(bool,     kBool)    \
        M(float,    kFloat)   \
        M(char,     kChar)    \
        M(void*,    kPtr)

    #define M(T, E) E,
    enum class Type : unsigned {
        SK_METADATA_TYPES(M)
    };
    #undef M

    template <typename T>
    T* set(const SkString& name, const T* src, size_t count) {
        return (T*)this->set(name, src, sizeof(T), GetType<T>(), count);
    }

    template <typename T>
    const T* get(const SkString& name, size_t* count) const {
        return (const T*)this->get(name, GetType<T>(), count);
    }

    template <typename T>
    bool find(const SkString& name, size_t* count, T* value = nullptr) const {
        return this->find(name, sizeof(T), GetType<T>(), count, value);
    }

    template <typename T>
    bool findOne(const SkString& name, T* value = nullptr) const {
        return this->findOne(name, sizeof(T), GetType<T>(), value);
    }

    class Iter {
    public:
        Iter(const SkMetaData&);
        const char* next(Type*, int* count);

    private:
        struct Entry { const char* name; Type type; unsigned count; };
        std::vector<Entry> fEntries;
        size_t fIndex = 0;
    };

    // shims for backwards compatibility
    static constexpr Type kScalar_Type = Type::kFloat;
    static constexpr Type kBool_Type = Type::kBool;
    static constexpr Type kS32_Type = Type::kS32;
    static constexpr Type kPtr_Type = Type::kPtr;

    void setBool(const char name[], bool value) { (void)this->set(SkString(name), &value, 1); }
    void setScalar(const char name[], SkScalar v) { (void)this->set<float>(SkString(name), &v, 1); }
    void setS32(const char name[], int32_t value) { (void)this->set(SkString(name), &value, 1); }
    void setPtr(const char name[], void* value) { (void)this->set(SkString(name), &value, 1); }
    bool findBool(const char name[], bool* value = nullptr) const {
        return this->findOne<bool>(SkString(name), value);
    }
    bool hasBool(const char name[], bool value) const {
        bool v;
        return this->findBool(name, &v) && v == value;
    }
    SkScalar* setScalars(const char name[], int count, const SkScalar values[]) {
        return this->set<float>(SkString(name), values, count);
    }
    bool findScalar(const char name[], SkScalar* value = nullptr) const {
        return this->findOne<float>(SkString(name), value);
    }
    bool findScalars(const char name[], int* count, SkScalar values[] = nullptr) const {
        size_t n = 0;
        bool r = this->find<float>(SkString(name), &n, values);
        if (count) { *count = SkToInt(n); }
        return r;
    }
    bool findS32(const char name[], int32_t* value = nullptr) const {
        return this->findOne<int32_t>(SkString(name), value);
    }
    bool findPtr(const char name[], void** value = nullptr) const {
        return this->findOne<void*>(SkString(name), value);
    }

private:
    struct Rec {
        unsigned fDataCount; // number of elements
        unsigned fType;
    };
    struct D { void operator()(Rec*); };
    using RecHolder = std::unique_ptr<Rec, D>;
    SkTHashMap<SkString, RecHolder> fMap;
    friend class Iter;

    template<typename T> static Type GetType();
    void* set(const SkString&, const void*, size_t, Type, size_t);
    const void* get(const SkString&, Type, size_t*) const;
    bool find(const SkString&, size_t, SkMetaData::Type, size_t*, void*) const;
    bool findOne(const SkString&, size_t, SkMetaData::Type, void*) const;

    SkMetaData(const SkMetaData&) = delete;
    SkMetaData& operator=(const SkMetaData&) = delete;
};

#define M(T, E) \
    static_assert(std::is_trivially_copyable<T>::value, ""); \
    template<> inline SkMetaData::Type SkMetaData::GetType<T>() { \
        return SkMetaData::Type::E; \
    }
SK_METADATA_TYPES(M)
#undef M

#endif
