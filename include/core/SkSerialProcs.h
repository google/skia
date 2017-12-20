/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSerialProcs_DEFINED
#define SkSerialProcs_DEFINED

#include "SkImage.h"
#include "SkPicture.h"
#include "SkTypeface.h"

/**
 *  A serial-proc is asked to serialize the specified object (e.g. picture or image).
 *  If a data object is returned, it will be used (even if it is zero-length).
 *  If null is returned, then Skia will take its default action.
 *
 *  The default action for pictures is to use Skia's internal format.
 *  The default action for images is to encode using PNG.
 *  The default action for typefaces is to use Skia's internal format.
 */

typedef sk_sp<SkData> (*SkSerialPictureProc)(SkPicture*, void* ctx);
typedef sk_sp<SkData> (*SkSerialImageProc)(SkImage*, void* ctx);
typedef sk_sp<SkData> (*SkSerialTypefaceProc)(SkTypeface*, void* ctx);

/**
 *  A deserial-proc is given the serialized form previously returned by the corresponding
 *  serial-proc, and should return the re-constituted object. In case of an error, the proc
 *  can return nullptr.
 */

typedef sk_sp<SkPicture> (*SkDeserialPictureProc)(const void* data, size_t length, void* ctx);
typedef sk_sp<SkImage> (*SkDeserialImageProc)(const void* data, size_t length, void* ctx);
typedef sk_sp<SkTypeface> (*SkDeserialTypefaceProc)(const void* data, size_t length, void* ctx);

struct SK_API SkSerialProcs {
    SkSerialPictureProc fPictureProc = nullptr;
    void*               fPictureCtx = nullptr;

    SkSerialImageProc   fImageProc = nullptr;
    void*               fImageCtx = nullptr;

    SkSerialTypefaceProc fTypefaceProc = nullptr;
    void*                fTypefaceCtx = nullptr;
};

struct SK_API SkDeserialProcs {
    SkDeserialPictureProc   fPictureProc = nullptr;
    void*                   fPictureCtx = nullptr;

    SkDeserialImageProc     fImageProc = nullptr;
    void*                   fImageCtx = nullptr;

    SkDeserialTypefaceProc  fTypefaceProc = nullptr;
    void*                   fTypefaceCtx = nullptr;
};

#endif

