// Copyright 2019 Google LLC.
#ifndef StringCache_DEFINED
#define StringCache_DEFINED
#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTHash.h"

typedef struct { const char* value; } CachedString;
bool operator==(CachedString a, CachedString b);
bool operator!=(CachedString a, CachedString b);

class StringCache {
private:
    struct String {
        const char* fPtr;
        bool operator==(const String& that) const {
            return fPtr == that.fPtr || 0 == strcmp(fPtr, that.fPtr);
        }
    };
    struct Hasher {
        uint32_t operator()(String k) const {
            static constexpr char gEmpty[] = "";
            if (!k.fPtr) { k = {gEmpty}; }
            return SkOpts::hash_fn(k.fPtr, strlen(k.fPtr), 0);
         }
    };
    mutable SkMutex fMutex;
    SkTHashMap<String, SkString, Hasher> fMap;

    const SkString& maker_internal(const char* str) {
        SkAutoMutexExclusive lock(fMutex);
        String k = { str };
        const SkString* ptr = fMap.find(k);
        if (!ptr) {
            SkString newString(str);
            const char* cstr = newString.c_str();
            ptr = fMap.set(String{cstr}, std::move(newString));
            SkASSERT(ptr);
        }
        return *ptr;
    }

public:

    static StringCache gStringCache;

    SkString makerSkString(CachedString cs) {
        return SkString(this->maker_internal(cs.value));
    }

    const char* makeCString(CachedString cs) {
        const SkString& str = this->maker_internal(cs.value);
        return str.c_str();
    }

    CachedString make(const char* ch) {
        const SkString& str = this->maker_internal(ch);
        return { str.c_str() };
    }

    const char* toCString(CachedString cached) { return cached.value; }
};

#endif // StringCache_DEFINED
