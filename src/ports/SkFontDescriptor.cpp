/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkStream.h"

enum {
    kFontFamilyName = 0x01,
    kFontStyle      = 0x02,
    kFontFileName   = 0xFD,
    kSentinel       = 0xFE,
};

SkFontDescriptor::SkFontDescriptor() {
    fFontStyle = SkTypeface::kNormal;
}

static const char* read_string(SkStream* stream) {
    SkString string;
    const uint32_t length = stream->readPackedUInt();
    if (length > 0) {
        string.resize(length);
        stream->read(string.writable_str(), length);
    }
    return string.c_str();
}

static void write_string(SkWStream* stream, SkString string, uint32_t id) {
    if (!string.isEmpty()) {
        stream->writePackedUInt(id);
        stream->writePackedUInt(string.size());
        stream->write(string.c_str(), string.size());
    }
}

SkFontDescriptor::SkFontDescriptor(SkStream* stream) {
    fFontStyle = SkTypeface::kNormal;

    uint32_t id = stream->readPackedUInt();
    while (id != kSentinel) {
        switch (id) {
            case kFontFamilyName: {
                fFontFamilyName.set(read_string(stream));
            } break;
            case kFontStyle: {
                fFontStyle = (SkTypeface::Style)stream->readPackedUInt();
            } break;
            case kFontFileName: {
                fFontFileName.set(read_string(stream));
            } break;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
        }
        int prevId = id;
        id = stream->readPackedUInt();
    }
}

void SkFontDescriptor::serialize(SkWStream* stream) {
    write_string(stream, fFontFamilyName, kFontFamilyName);
    write_string(stream, fFontFileName, kFontFileName);

    if (fFontStyle != SkTypeface::kNormal) {
        stream->writePackedUInt(kFontStyle);
        stream->writePackedUInt((uint32_t)fFontStyle);
    }

    stream->writePackedUInt(kSentinel);
}
