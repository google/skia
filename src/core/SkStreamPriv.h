/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamPriv_DEFINED
#define SkStreamPriv_DEFINED

class SkAutoMalloc;
class SkStream;
class SkStreamRewindable;
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
size_t SkCopyStreamToStorage(SkAutoMalloc* storage, SkStream* stream);

/**
 *  Copy the provided stream to an SkData variable.
 *  @param stream SkStream to be copied into data.
 *  @return SkData* The resulting SkData after the copy. This data
 *      will have a ref count of one upon return and belongs to the
 *      caller. Returns nullptr on failure.
 */
SkData *SkCopyStreamToData(SkStream* stream);

/**
 *  Attempt to convert this stream to a StreamRewindable in the
 *  cheapest possible manner (calling duplicate() if possible, and
 *  otherwise allocating memory for a copy).  The position of the
 *  input stream is left in an indeterminate state.
 */
SkStreamRewindable* SkStreamRewindableFromSkStream(SkStream* stream);

/**
 *  Copies the input stream from the current position to the end.
 *  Does not rewind the input stream.
 */
bool SkStreamCopy(SkWStream* out, SkStream* input);

#endif  // SkStreamPriv_DEFINED
