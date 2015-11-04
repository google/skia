/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemote_protocol_DEFINED
#define SkRemote_protocol_DEFINED

// ATTENTION!  Changes to this file can break protocol compatibility.  Tread carefully.

namespace SkRemote {

    // It is safe to append to this enum without breaking protocol compatibility.
    // Resorting, deleting, or inserting anywhere but the end will break compatibility.
    enum class Type : uint8_t {
        kMatrix,
        kMisc,
        kPath,
        kStroke,
        kTextBlob,
        kPathEffect,
        kShader,
        kXfermode,
        kMaskFilter,
        kColorFilter,
        kRasterizer,
        kDrawLooper,
        kImageFilter,
        kAnnotation,
    };

}  // namespace SkRemote

#endif//SkRemote_protocol_DEFINED
