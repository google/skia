/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkStream.h"
#include "SkData.h"

enum {
    // these must match the sfnt 'name' enums
    kFontFamilyName = 0x01,
    kFullName       = 0x04,
    kPostscriptName = 0x06,

    // These count backwards from 0xFF, so as not to collide with the SFNT
    // defines for names in its 'name' table.
    kFontIndex      = 0xFD,
    kFontFileName   = 0xFE,  // Remove when MIN_PICTURE_VERSION > 41
    kSentinel       = 0xFF,
};

SkFontDescriptor::SkFontDescriptor(SkTypeface::Style style) : fFontIndex(0), fStyle(style) { }

static void read_string(SkStream* stream, SkString* string) {
    const uint32_t length = SkToU32(stream->readPackedUInt());
    if (length > 0) {
        string->resize(length);
        stream->read(string->writable_str(), length);
    }
}

// Remove when MIN_PICTURE_VERSION > 41
static void skip_string(SkStream* stream) {
    const uint32_t length = SkToU32(stream->readPackedUInt());
    if (length > 0) {
        stream->skip(length);
    }
}

static void write_string(SkWStream* stream, const SkString& string,
                         uint32_t id) {
    if (!string.isEmpty()) {
        stream->writePackedUInt(id);
        stream->writePackedUInt(string.size());
        stream->write(string.c_str(), string.size());
    }
}

static size_t read_uint(SkStream* stream) {
    return stream->readPackedUInt();
}

static void write_uint(SkWStream* stream, size_t n, uint32_t id) {
    stream->writePackedUInt(id);
    stream->writePackedUInt(n);
}

SkFontDescriptor::SkFontDescriptor(SkStream* stream) : fFontIndex(0) {
    fStyle = (SkTypeface::Style)stream->readPackedUInt();

    for (size_t id; (id = stream->readPackedUInt()) != kSentinel;) {
        switch (id) {
            case kFontFamilyName:
                read_string(stream, &fFamilyName);
                break;
            case kFullName:
                read_string(stream, &fFullName);
                break;
            case kPostscriptName:
                read_string(stream, &fPostscriptName);
                break;
            case kFontIndex:
                fFontIndex = read_uint(stream);
                break;
            case kFontFileName:  // Remove when MIN_PICTURE_VERSION > 41
                skip_string(stream);
                break;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
                return;
        }
    }

    size_t length = stream->readPackedUInt();
    if (length > 0) {
        SkAutoTUnref<SkData> data(SkData::NewUninitialized(length));
        if (stream->read(data->writable_data(), length) == length) {
            fFontData.reset(SkNEW_ARGS(SkMemoryStream, (data)));
        }
    }
}

void SkFontDescriptor::serialize(SkWStream* stream) {
    stream->writePackedUInt(fStyle);

    write_string(stream, fFamilyName, kFontFamilyName);
    write_string(stream, fFullName, kFullName);
    write_string(stream, fPostscriptName, kPostscriptName);
    if (fFontIndex) {
        write_uint(stream, fFontIndex, kFontIndex);
    }

    stream->writePackedUInt(kSentinel);

    if (fFontData) {
        size_t length = fFontData->getLength();
        stream->writePackedUInt(length);
        stream->writeStream(fFontData, length);
    } else {
        stream->writePackedUInt(0);
    }
}
