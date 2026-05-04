/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelStorage_DEFINED
#define SkPixelStorage_DEFINED

#include <cstdint>

/**
 * SkPixelStorage is the class representing the storage block for pixel data. It serves as the
 * abstract source of pixel memory—the 2D array of data—that is read from and written to by
 * higher-level Skia objects like SkImage and SkSurface.
 */
class SkPixelStorage {
public:
    SkPixelStorage();
    virtual ~SkPixelStorage() = default;

    enum Type {
        kTextureProxy,
        kPixelRef,
    };

    uint32_t getPixelStorageId() const { return fID; }
    virtual Type type() const = 0;

private:
    uint32_t fID;

    static uint32_t NextId();
};

#endif
