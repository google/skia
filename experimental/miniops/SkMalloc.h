#ifndef SkMalloc_DEFINED
#define SkMalloc_DEFINED

#include <cstring>

static inline void sk_bzero(void* buffer, size_t size) {
    // Please c.f. sk_careful_memcpy.  It's undefined behavior to call memset(null, 0, 0).
    if (size) {
        memset(buffer, 0, size);
    }
}

#endif  // SkMalloc_DEFINED
