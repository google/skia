/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontStream.h"

struct SkSFNTHeader {
    uint32_t    fVersion;
    uint16_t    fNumTables;
    uint16_t    fSearchRange;
    uint16_t    fEntrySelector;
    uint16_t    fRangeShift;
};

struct SkTTCFHeader {
    uint32_t    fTag;
    uint32_t    fVersion;
    uint32_t    fNumOffsets;
    uint32_t    fOffset0;   // the first of N (fNumOffsets)
};

union SkSharedTTHeader {
    SkSFNTHeader    fSingle;
    SkTTCFHeader    fCollection;
};

struct SkSFNTDirEntry {
    uint32_t    fTag;
    uint32_t    fChecksum;
    uint32_t    fOffset;
    uint32_t    fLength;
};

static bool read(SkStream* stream, void* buffer, size_t amount) {
    return stream->read(buffer, amount) == amount;
}

static bool skip(SkStream* stream, size_t amount) {
    return stream->skip(amount) == amount;
}

/** Return the number of tables, or if this is a TTC (collection), return the
    number of tables in the first element of the collection. In either case,
    if offsetToDir is not-null, set it to the offset to the beginning of the
    table headers (SkSFNTDirEntry), relative to the start of the stream.

    On an error, return 0 for number of tables, and ignore offsetToDir
 */
static int count_tables(SkStream* stream, int ttcIndex, size_t* offsetToDir) {
    SkASSERT(ttcIndex >= 0);

    SkAutoSMalloc<1024> storage(sizeof(SkSharedTTHeader));
    SkSharedTTHeader* header = (SkSharedTTHeader*)storage.get();

    if (!read(stream, header, sizeof(SkSharedTTHeader))) {
        return 0;
    }

    // by default, SkSFNTHeader is at the start of the stream
    size_t offset = 0;

    // if we're really a collection, the first 4-bytes will be 'ttcf'
    uint32_t tag = SkEndian_SwapBE32(header->fCollection.fTag);
    if (SkSetFourByteTag('t', 't', 'c', 'f') == tag) {
        unsigned count = SkEndian_SwapBE32(header->fCollection.fNumOffsets);
        if ((unsigned)ttcIndex >= count) {
            return 0;
        }

        if (ttcIndex > 0) { // need to read more of the shared header
            stream->rewind();
            size_t amount = sizeof(SkSharedTTHeader) + ttcIndex * sizeof(uint32_t);
            header = (SkSharedTTHeader*)storage.reset(amount);
            if (!read(stream, header, amount)) {
                return 0;
            }
        }
        // this is the offset to the local SkSFNTHeader
        offset = SkEndian_SwapBE32((&header->fCollection.fOffset0)[ttcIndex]);
        stream->rewind();
        if (!skip(stream, offset)) {
            return 0;
        }
        if (!read(stream, header, sizeof(SkSFNTHeader))) {
            return 0;
        }
    }

    if (offsetToDir) {
        // add the size of the header, so we will point to the DirEntries
        *offsetToDir = offset + sizeof(SkSFNTHeader);
    }
    return SkEndian_SwapBE16(header->fSingle.fNumTables);
}

///////////////////////////////////////////////////////////////////////////////

struct SfntHeader {
    SfntHeader() : fCount(0), fDir(nullptr) {}
    ~SfntHeader() { sk_free(fDir); }

    /** If it returns true, then fCount and fDir are properly initialized.
        Note: fDir will point to the raw array of SkSFNTDirEntry values,
        meaning they will still be in the file's native endianness (BE).

        fDir will be automatically freed when this object is destroyed
     */
    bool init(SkStream* stream, int ttcIndex) {
        stream->rewind();

        size_t offsetToDir;
        fCount = count_tables(stream, ttcIndex, &offsetToDir);
        if (0 == fCount) {
            return false;
        }

        stream->rewind();
        if (!skip(stream, offsetToDir)) {
            return false;
        }

        size_t size = fCount * sizeof(SkSFNTDirEntry);
        fDir = reinterpret_cast<SkSFNTDirEntry*>(sk_malloc_throw(size));
        return read(stream, fDir, size);
    }

    int             fCount;
    SkSFNTDirEntry* fDir;
};

///////////////////////////////////////////////////////////////////////////////

int SkFontStream::CountTTCEntries(SkStream* stream) {
    stream->rewind();

    SkSharedTTHeader shared;
    if (!read(stream, &shared, sizeof(shared))) {
        return 0;
    }

    // if we're really a collection, the first 4-bytes will be 'ttcf'
    uint32_t tag = SkEndian_SwapBE32(shared.fCollection.fTag);
    if (SkSetFourByteTag('t', 't', 'c', 'f') == tag) {
        return SkEndian_SwapBE32(shared.fCollection.fNumOffsets);
    } else {
        return 1;   // normal 'sfnt' has 1 dir entry
    }
}

int SkFontStream::GetTableTags(SkStream* stream, int ttcIndex,
                               SkFontTableTag tags[]) {
    SfntHeader  header;
    if (!header.init(stream, ttcIndex)) {
        return 0;
    }

    if (tags) {
        for (int i = 0; i < header.fCount; i++) {
            tags[i] = SkEndian_SwapBE32(header.fDir[i].fTag);
        }
    }
    return header.fCount;
}

size_t SkFontStream::GetTableData(SkStream* stream, int ttcIndex,
                                  SkFontTableTag tag,
                                  size_t offset, size_t length, void* data) {
    SfntHeader  header;
    if (!header.init(stream, ttcIndex)) {
        return 0;
    }

    for (int i = 0; i < header.fCount; i++) {
        if (SkEndian_SwapBE32(header.fDir[i].fTag) == tag) {
            size_t realOffset = SkEndian_SwapBE32(header.fDir[i].fOffset);
            size_t realLength = SkEndian_SwapBE32(header.fDir[i].fLength);
            // now sanity check the caller's offset/length
            if (offset >= realLength) {
                return 0;
            }
            // if the caller is trusting the length from the file, then a
            // hostile file might choose a value which would overflow offset +
            // length.
            if (offset + length < offset) {
                return 0;
            }
            if (length > realLength - offset) {
                length = realLength - offset;
            }
            if (data) {
                // skip the stream to the part of the table we want to copy from
                stream->rewind();
                size_t bytesToSkip = realOffset + offset;
                if (!skip(stream, bytesToSkip)) {
                    return 0;
                }
                if (!read(stream, data, length)) {
                    return 0;
                }
            }
            return length;
        }
    }
    return 0;
}
