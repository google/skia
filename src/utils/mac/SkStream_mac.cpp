/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCGUtils.h"
#include "SkStream.h"

// This is used by CGDataProviderCreateWithData

static void unref_data_proc(void* info, const void* addr, size_t size) {
    SkASSERT(info);
    ((SkRefCnt*)info)->unref();
}

// These are used by CGDataProviderSequentialCallbacks

size_t get_bytes_proc(void* info, void* buffer, size_t bytes) {
    SkASSERT(info);
    return ((SkStream*)info)->read(buffer, bytes);
}

static off_t skip_forward_proc(void* info, off_t bytes) {
    return ((SkStream*)info)->skip(bytes);
}

static void rewind_proc(void* info) {
    SkASSERT(info);
    ((SkStream*)info)->rewind();
}

static void release_info_proc(void* info) {
    SkASSERT(info);
    ((SkStream*)info)->unref();
}

CGDataProviderRef SkCreateDataProviderFromStream(SkStream* stream) {
    stream->ref();  // unref will be called when the provider is deleted

    const void* addr = stream->getMemoryBase();
    if (addr) {
        // special-case when the stream is just a block of ram
        return CGDataProviderCreateWithData(stream, addr, stream->getLength(),
                                            unref_data_proc);
    }

    CGDataProviderSequentialCallbacks rec;
    sk_bzero(&rec, sizeof(rec));
    rec.version = 0;
    rec.getBytes = get_bytes_proc;
    rec.skipForward = skip_forward_proc;
    rec.rewind = rewind_proc;
    rec.releaseInfo = release_info_proc;
    return CGDataProviderCreateSequential(stream, &rec);
}


