/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTextEncoding_DEFINED
#define SkTextEncoding_DEFINED

enum class SkTextEncoding {
    kUTF8,    //!< the text parameters are UTF-8
    kUTF16,   //!< the text parameters are UTF-16
    kUTF32,   //!< the text parameters are UTF-32
    kGlyphID  //!< the text parameters are glyph indices
};

/**
 * POD struct that points to a segment of encoded unicode text or glyphIDs.
 */
struct SkText {
    const void*    fText;
    size_t         fByteLength;
    SkTextEncoding fEncoding;
};

#endif  // SkTextEncoding_DEFINED
