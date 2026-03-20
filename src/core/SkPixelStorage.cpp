/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPixelStorage.h"
#include <atomic>

uint32_t SkPixelStorage::NextId() {
    static std::atomic<uint32_t> gNextID{1};
    uint32_t id;
    do {
        id = gNextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == 0);
    return id;
}

SkPixelStorage::SkPixelStorage() : fID(NextId()) {}
