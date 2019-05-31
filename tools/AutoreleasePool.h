/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAutoreleasePool_DEFINED
#define SkAutoreleasePool_DEFINED

/*
 * Helper class for managing an autorelease pool on MacOS and iOS. On other platforms this will
 * do nothing so there's no need to #ifdef it out.
 */
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
class AutoreleasePool {
public:
    AutoreleasePool();
    ~AutoreleasePool();

    void drain();

private:
    void* fPool;
};
#else
class AutoreleasePool {
public:
    AutoreleasePool() {}
    ~AutoreleasePool() = default;

    void drain() {}
};
#endif

#endif
