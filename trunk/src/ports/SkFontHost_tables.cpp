
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkEndian.h"
#include "SkFontHost.h"
#include "SkStream.h"

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

/** Return the number of tables, or if this is a TTC (collection), return the
    number of tables in the first element of the collection. In either case,
    if offsetToDir is not-null, set it to the offset to the beginning of the
    table headers (SkSFNTDirEntry), relative to the start of the stream.

    On an error, return 0 for number of tables, and ignore offsetToDir
 */
static int count_tables(SkStream* stream, size_t* offsetToDir = NULL) {
    SkSharedTTHeader shared;
    if (stream->read(&shared, sizeof(shared)) != sizeof(shared)) {
        return 0;
    }

    // by default, SkSFNTHeader is at the start of the stream
    size_t offset = 0;

    // if we're really a collection, the first 4-bytes will be 'ttcf'
    uint32_t tag = SkEndian_SwapBE32(shared.fCollection.fTag);
    if (SkSetFourByteTag('t', 't', 'c', 'f') == tag) {
        if (shared.fCollection.fNumOffsets == 0) {
            return 0;
        }
        // this is the offset to the first local SkSFNTHeader
        offset = SkEndian_SwapBE32(shared.fCollection.fOffset0);
        stream->rewind();
        if (stream->skip(offset) != offset) {
            return 0;
        }
        if (stream->read(&shared, sizeof(shared)) != sizeof(shared)) {
            return 0;
        }
    }

    if (offsetToDir) {
        // add the size of the header, so we will point to the DirEntries
        *offsetToDir = offset + sizeof(SkSFNTHeader);
    }
    return SkEndian_SwapBE16(shared.fSingle.fNumTables);
}

///////////////////////////////////////////////////////////////////////////////

struct SfntHeader {
    SfntHeader() : fCount(0), fDir(NULL) {}
    ~SfntHeader() { sk_free(fDir); }

    /** If it returns true, then fCount and fDir are properly initialized.
        Note: fDir will point to the raw array of SkSFNTDirEntry values,
        meaning they will still be in the file's native endianness (BE).

        fDir will be automatically freed when this object is destroyed
     */
    bool init(SkStream* stream) {
        size_t offsetToDir;
        fCount = count_tables(stream, &offsetToDir);
        if (0 == fCount) {
            return false;
        }

        stream->rewind();
        if (stream->skip(offsetToDir) != offsetToDir) {
            return false;
        }

        size_t size = fCount * sizeof(SkSFNTDirEntry);
        fDir = reinterpret_cast<SkSFNTDirEntry*>(sk_malloc_throw(size));
        return stream->read(fDir, size) == size;
    }

    int             fCount;
    SkSFNTDirEntry* fDir;
};

///////////////////////////////////////////////////////////////////////////////

int SkFontHost::CountTables(SkFontID fontID) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    return count_tables(stream);
}

int SkFontHost::GetTableTags(SkFontID fontID, SkFontTableTag tags[]) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    SfntHeader  header;
    if (!header.init(stream)) {
        return 0;
    }

    for (int i = 0; i < header.fCount; i++) {
        tags[i] = SkEndian_SwapBE32(header.fDir[i].fTag);
    }
    return header.fCount;
}

size_t SkFontHost::GetTableSize(SkFontID fontID, SkFontTableTag tag) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    SfntHeader  header;
    if (!header.init(stream)) {
        return 0;
    }

    for (int i = 0; i < header.fCount; i++) {
        if (SkEndian_SwapBE32(header.fDir[i].fTag) == tag) {
            return SkEndian_SwapBE32(header.fDir[i].fLength);
        }
    }
    return 0;
}

size_t SkFontHost::GetTableData(SkFontID fontID, SkFontTableTag tag,
                                size_t offset, size_t length, void* data) {
    SkStream* stream = SkFontHost::OpenStream(fontID);
    if (NULL == stream) {
        return 0;
    }

    SkAutoUnref au(stream);
    SfntHeader  header;
    if (!header.init(stream)) {
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
            if (offset + length > realLength) {
                length = realLength - offset;
            }
            // skip the stream to the part of the table we want to copy from
            stream->rewind();
            size_t bytesToSkip = realOffset + offset;
            if (stream->skip(bytesToSkip) != bytesToSkip) {
                return 0;
            }
            if (stream->read(data, length) != length) {
                return 0;
            }
            return length;
        }
    }
    return 0;
}

