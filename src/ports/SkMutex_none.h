/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_none_DEFINED
#define SkMutex_none_DEFINED

/** Non-mutex mutex for uniprocessor systems. */

struct SkBaseMutex {
    void acquire() { }
    void release() { }
};

class SkMutex : public SkBaseMutex {
public:
    SkMutex() { }
    ~SkMutex() { }

private:
    SkMutex(const SkMutex&);
    SkMutex& operator=(const SkMutex&);
};

// Using POD-style initialization prevents the generation of a static initializer.
#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name = { }

// Special case used when the static mutex must be available globally.
#define SK_DECLARE_GLOBAL_MUTEX(name) SkBaseMutex name = { }

#endif
