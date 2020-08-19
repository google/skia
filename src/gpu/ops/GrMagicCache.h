/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMagicCache_DEFINED
#define GrMagicCache_DEFINED

// A magical cache - full of magic
class GrMagicCache {
public:
    GrMagicCache() {}

    void freeAll() {}

    void purgeStale() {}

    size_t usedBytes() const { return 0; }

private:
    bool fMagic = true;
};

#endif // GrMagicCache_DEFINED
