/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSynchronizedResourceCache.h"

SkSynchronizedResourceCache::SkSynchronizedResourceCache(DiscardableFactory fact)
 : SkResourceCache(fact) {}

SkSynchronizedResourceCache::SkSynchronizedResourceCache(size_t byteLimit)
 : SkResourceCache(byteLimit) {}

SkSynchronizedResourceCache::~SkSynchronizedResourceCache() = default;

size_t SkSynchronizedResourceCache::getTotalBytesUsed() const {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::getTotalBytesUsed();
}

size_t SkSynchronizedResourceCache::getTotalByteLimit() const {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::getTotalByteLimit();
}

size_t SkSynchronizedResourceCache::setTotalByteLimit(size_t newLimit) {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::setTotalByteLimit(newLimit);
}

SkResourceCache::DiscardableFactory SkSynchronizedResourceCache::discardableFactory() const {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::discardableFactory();
}

SkCachedData* SkSynchronizedResourceCache::newCachedData(size_t bytes) {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::newCachedData(bytes);
}

void SkSynchronizedResourceCache::dump() const {
    SkAutoMutexExclusive am(fMutex);
    SkResourceCache::dump();
}

size_t SkSynchronizedResourceCache::setSingleAllocationByteLimit(size_t size) {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::setSingleAllocationByteLimit(size);
}

size_t SkSynchronizedResourceCache::getSingleAllocationByteLimit() const {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::getSingleAllocationByteLimit();
}

size_t SkSynchronizedResourceCache::getEffectiveSingleAllocationByteLimit() const {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::getEffectiveSingleAllocationByteLimit();
}

void SkSynchronizedResourceCache::purgeAll() {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::purgeAll();
}

bool SkSynchronizedResourceCache::find(const Key& key, FindVisitor visitor, void* context) {
    SkAutoMutexExclusive am(fMutex);
    return SkResourceCache::find(key, visitor, context);
}

void SkSynchronizedResourceCache::add(Rec* rec, void* payload) {
    SkAutoMutexExclusive am(fMutex);
    SkResourceCache::add(rec, payload);
}

void SkSynchronizedResourceCache::visitAll(Visitor visitor, void* context) {
    SkAutoMutexExclusive am(fMutex);
    SkResourceCache::visitAll(visitor, context);
}
