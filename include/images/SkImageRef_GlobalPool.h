
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkImageRef_GlobalPool_DEFINED
#define SkImageRef_GlobalPool_DEFINED

#include "SkImageRef.h"

class SkImageRef_GlobalPool : public SkImageRef {
public:
    // if pool is null, use the global pool
    SkImageRef_GlobalPool(const SkImageInfo&, SkStreamRewindable*,
                          int sampleSize = 1);
    virtual ~SkImageRef_GlobalPool();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageRef_GlobalPool)

    // API to control the global pool

    /** Return the amount specified as the budget for the cache (in bytes).
     */
    static size_t GetRAMBudget();

    /** Set a new budget value for the cache.
     */
    static void SetRAMBudget(size_t);

    /** Return how much ram is currently in use by the global cache.
     */
    static size_t GetRAMUsed();

    /** Free up (approximately) enough such that the amount used by the cache
     is <= the specified amount. Since some images may be "in use", the
     amount actually freed may not always result in a ram usage value <=
     to the requested amount. In addition, because of the
     chunky nature of the cache, the resulting usage may be < the requested
     amount.
     */
    static void SetRAMUsed(size_t usageInBytes);

    static void DumpPool();

protected:
    virtual bool onDecode(SkImageDecoder* codec, SkStreamRewindable* stream,
                          SkBitmap* bitmap, SkBitmap::Config config,
                          SkImageDecoder::Mode mode);

    virtual void onUnlockPixels();

    SkImageRef_GlobalPool(SkReadBuffer&);

private:
    typedef SkImageRef INHERITED;
};

#endif
