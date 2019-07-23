// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/StringCache.h"

bool operator==(CachedString a, CachedString b) {
    return a.value == b.value;
}

bool operator!=(CachedString a, CachedString b) {
    return a.value != b.value;
}

StringCache StringCache::gStringCache;