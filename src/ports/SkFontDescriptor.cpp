/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontDescriptor.h"
#include "SkStream.h"

enum {
    // these must match the sfnt 'name' enums
    kFontFamilyName = 0x01,
    kFullName       = 0x04,
    kPostscriptName = 0x06,

    // These count backwards from 0xFF, so as not to collide with the SFNT
    // defines for names in its 'name' table.
    kFontFileName   = 0xFE,
    kSentinel       = 0xFF,
};

SkFontDescriptor::SkFontDescriptor(SkTypeface::Style style) {
    fStyle = style;
}

static void read_string(SkStream* stream, SkString* string) {
    const uint32_t length = stream->readPackedUInt();
    if (length > 0) {
        string->resize(length);
        stream->read(string->writable_str(), length);
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

SkFontDescriptor::SkFontDescriptor(SkStream* stream) {
    fStyle = (SkTypeface::Style)stream->readPackedUInt();

    for (;;) {
        switch (stream->readPackedUInt()) {
            case kFontFamilyName:
                read_string(stream, &fFamilyName);
                break;
            case kFullName:
                read_string(stream, &fFullName);
                break;
            case kPostscriptName:
                read_string(stream, &fPostscriptName);
                break;
            case kFontFileName:
                read_string(stream, &fFontFileName);
                break;
            case kSentinel:
                return;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
                return;
        }
    }
}

void SkFontDescriptor::serialize(SkWStream* stream) {
    stream->writePackedUInt(fStyle);

    write_string(stream, fFamilyName, kFontFamilyName);
    write_string(stream, fFullName, kFullName);
    write_string(stream, fPostscriptName, kPostscriptName);
    write_string(stream, fFontFileName, kFontFileName);

    stream->writePackedUInt(kSentinel);
}
