/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamHelpers_DEFINED
#define SkStreamHelpers_DEFINED

class SkAutoMalloc;
class SkStream;
class SkData;

/**
 *  Copy the provided stream to memory allocated by storage.
 *  Used by SkImageDecoder_libbmp and SkImageDecoder_libico.
 *  @param storage Allocator to hold the memory. Will be reset to be large
 *      enough to hold the entire stream. Upon successful return,
 *      storage->get() will point to data holding the SkStream's entire
 *      contents.
 *  @param stream SkStream to be copied into storage.
 *  @return size_t Total number of bytes in the SkStream, which is also the
 *      number of bytes pointed to by storage->get(). Returns 0 on failure.
 */
size_t CopyStreamToStorage(SkAutoMalloc* storage, SkStream* stream);

/**
 *  Copy the provided stream to an SkData variable. Used by SkImageDecoder_libktx.
 *  @param stream SkStream to be copied into data.
 *  @return SkData* The resulting SkData after the copy. This data will have a
 *      ref count of one upon return and belongs to the caller. Returns NULL on failure.
 */
SkData *CopyStreamToData(SkStream* stream);

#endif // SkStreamHelpers_DEFINED
