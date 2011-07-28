
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkThread_platform_DEFINED
#define SkThread_platform_DEFINED

#if defined(ANDROID) && !defined(SK_BUILD_FOR_ANDROID_NDK)

#include <utils/threads.h>
#include <utils/Atomic.h>

#define sk_atomic_inc(addr)     android_atomic_inc(addr)
#define sk_atomic_dec(addr)     android_atomic_dec(addr)

class SkMutex : android::Mutex {
public:
    // if isGlobal is true, then ignore any errors in the platform-specific
    // destructor
    SkMutex(bool isGlobal = true) {}
    ~SkMutex() {}

    void    acquire() { this->lock(); }
    void    release() { this->unlock(); }
};

#else

/** Implemented by the porting layer, this function adds 1 to the int specified
    by the address (in a thread-safe manner), and returns the previous value.
*/
SK_API int32_t sk_atomic_inc(int32_t* addr);
/** Implemented by the porting layer, this function subtracts 1 to the int
    specified by the address (in a thread-safe manner), and returns the previous
    value.
*/
SK_API int32_t sk_atomic_dec(int32_t* addr);

class SkMutex {
public:
    // if isGlobal is true, then ignore any errors in the platform-specific
    // destructor
    SkMutex(bool isGlobal = true);
    ~SkMutex();

    void    acquire();
    void    release();

private:
    bool fIsGlobal;
    enum {
        kStorageIntCount = 64
    };
    uint32_t    fStorage[kStorageIntCount];
};

#endif

#endif
