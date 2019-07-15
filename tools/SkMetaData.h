/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMetaData_DEFINED
#define SkMetaData_DEFINED

#include "include/core/SkScalar.h"

#include <memory>
#include <cstring>

class SK_API SkMetaData {
public:
    SkMetaData() = default;
    ~SkMetaData();

    template <typename T>
    bool find(const char name[], T* value = nullptr) const {
        int count = 0;
        const T* ptr = (const T*)this->find(name, GetType<T>(), &count);
        if (ptr && count == 1) {
            if (value) { *value = *ptr; }
            return true;
        }
        return false;
    }

    template <typename T>
    const T* finds(const char name[], int* count, T* values = nullptr) const {
        int n = 0;
        if (const T* ptr = (const T*)this->find(name, GetType<T>(), &n)) {
            if (count) { *count = n; }
            if (values) { std::memcpy(values, ptr, sizeof(T) * n); }
            return ptr;
        }
        return nullptr;
    }

    template <typename T>
    bool has(const char name[], T value) const {
        T v{};
        return this->find<T>(name, &v) && v == value;
    }

    template <typename T>
    void set(const char name[], T val) { (void)this->set(name, &val, sizeof(T), GetType<T>(), 1); }

    template <typename T>
    T* sets(const char name[], int count, const T values[]) {
        return (T*)this->set(name, values, sizeof(T), GetType<T>(), 1);
    }


    bool findS32(const char name[], int32_t* v = nullptr) const { return this->find(name, v); }
    bool findScalar(const char name[], SkScalar* v = nullptr) const { return this->find(name, v); }
    const SkScalar* findScalars(const char name[], int* count, SkScalar vs[] = nullptr) const {
        return this->finds(name, count, vs);
    }
    bool findPtr(const char name[], void** v = nullptr) const { return this->find(name, v); }
    bool findBool(const char name[], bool* v = nullptr) const { return this->find(name, v); }
    bool hasS32(const char name[], int32_t v) const { return this->has(name, v); }
    bool hasScalar(const char name[], SkScalar v) const { return this->has(name, v); }
    bool hasPtr(const char name[], void* v) const { return this->has(name, v); }
    bool hasBool(const char name[], bool v) const { return this->has(name, v); }
    void setS32(const char name[], int32_t v) { this->set(name, v); }
    void setScalar(const char name[], SkScalar v) { this->set(name, v); }
    SkScalar* setScalars(const char name[], int count, const SkScalar vs[]) {
        return this->sets(name, count, vs);
    }
    void setPtr(const char name[], void* ptr) { this->set(name, ptr); }
    void setBool(const char name[], bool v) { this->set(name, v); }

    enum Type {
        kS32_Type,
        kScalar_Type,
        kPtr_Type,
        kBool_Type,

        kTypeCount
    };

    struct Rec;
    class Iter {
    public:
        Iter(const SkMetaData&);

        /** Each time next is called, it returns the name of the next data element,
            or null when there are no more elements. If non-null is returned, then the
            element's type is returned (if not null), and the number of data values
            is returned in count (if not null).
        */
        const char* next(Type*, int* count);

    private:
        Rec* fRec = nullptr;
    };

public:
    struct D { void operator()(Rec*); };
    struct Rec {
        std::unique_ptr<Rec, D> fNext;
        uint16_t    fDataCount; // number of elements
        uint8_t     fDataLen;   // sizeof a single element
        uint8_t     fType;

        const void* data() const { return (this + 1); }
        void*       data() { return (this + 1); }
        const char* name() const { return (const char*)this->data() + fDataLen * fDataCount; }
        char*       name() { return (char*)this->data() + fDataLen * fDataCount; }
    };
    std::unique_ptr<Rec, D> fRec;

    friend class Iter;
    template <typename T> static Type GetType();

    const void* find(const char name[], Type type, int* count) const;
    void* set(const char name[], const void* data, size_t len, Type, int count);
    bool remove(const char name[], Type);


    SkMetaData(const SkMetaData&) = delete;
    SkMetaData& operator=(const SkMetaData&) = delete;
};

template <> inline SkMetaData::Type SkMetaData::GetType<int32_t>()  { return SkMetaData::kS32_Type;    }
template <> inline SkMetaData::Type SkMetaData::GetType<SkScalar>() { return SkMetaData::kScalar_Type; }
template <> inline SkMetaData::Type SkMetaData::GetType<void*>()    { return SkMetaData::kPtr_Type;    }
template <> inline SkMetaData::Type SkMetaData::GetType<bool>()     { return SkMetaData::kBool_Type;   }
#endif
