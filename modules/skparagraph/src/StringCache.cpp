// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/StringCache.h"

bool operator==(CachedString a, CachedString b) {
    return a.value == b.value;
}

bool operator!=(CachedString a, CachedString b) {
    return a.value != b.value;
}

bool StringCache::String::operator==(const String& that) const {
    return fPtr == that.fPtr || 0 == strcmp(fPtr, that.fPtr);
}

uint32_t StringCache::Hasher::operator()(String k) const {
    static constexpr char gEmpty[] = "";
    if (!k.fPtr) { k = {gEmpty}; }
    return SkOpts::hash_fn(k.fPtr, strlen(k.fPtr), 0);
 }

const SkString& StringCache::maker_internal(const char* str) {
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

StringCache StringCache::gStringCache;
