/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include "google_font_file_buffering.h"

#include "SkFontConfigInterface_direct_google3.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTypes.h"

bool SkFontConfigInterfaceDirectGoogle3::isAccessible(const char* filename) {
    // Check if this font has been pre-loaded into memory.
    const char* unused;
    if (GoogleFreeType::GoogleFt2ReadFontFromMemory(filename, &unused) >= 0) {
        return true;
    }
    return this->INHERITED::isAccessible(filename);
}

SkStreamAsset* SkFontConfigInterfaceDirectGoogle3::openStream(const FontIdentity& identity) {
    const char* c_filename = identity.fString.c_str();
    // Read the system fonts from the fonts we've pre-loaded into memory.
    const char* buffer;
    int length = GoogleFreeType::GoogleFt2ReadFontFromMemory(
        c_filename, &buffer);
    if (length >= 0) return new SkMemoryStream(buffer, length);
    return this->INHERITED::openStream(identity);
}
