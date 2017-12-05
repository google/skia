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
 *  A serial-proc is asked to serialize the specified object (e.g. picture or image), by writing
 *  its serialized form into the specified stream. If the proc does this, it returns true.
 *
 *  If the proc chooses to have Skia perform its default action, it ignores the stream parameter
 *  and just returns false.
 */

typedef bool (*SkSerialPictureProc)(SkPicture*, SkWStream*, void* ctx);
typedef bool (*SkSerialImageProc)(SkImage*, SkWStream*, void* ctx);
typedef bool (*SkSerialTypefaceProc)(SkTypeface*, SkWStream*, void* ctx);

/**
 *  A deserial-proc is given the serialized form previously returned by the corresponding
 *  serial-proc, and should return the re-constituted object. In case of an error, the proc
 *  can return nullptr.
 */

typedef sk_sp<SkPicture> (*SkDeserialPictureProc)(const void* data, size_t length, void* ctx);
typedef sk_sp<SkImage> (*SkDeserialImageProc)(const void* data, size_t length, void* ctx);
typedef sk_sp<SkTypeface> (*SkDeserialTypefaceProc)(const void* data, size_t length, void* ctx);

struct SkSerialProcs {
    SkSerialPictureProc fPictureProc = nullptr;
    void*               fPictureCtx = nullptr;

    SkSerialImageProc   fImageProc = nullptr;
    void*               fImageCtx = nullptr;

    SkSerialTypefaceProc fTypefaceProc = nullptr;
    void*                fTypefaceCtx = nullptr;
};

struct SkDeserialProcs {
    SkDeserialPictureProc   fPictureProc = nullptr;
    void*                   fPictureCtx = nullptr;

    SkDeserialImageProc     fImageProc = nullptr;
    void*                   fImageCtx = nullptr;

    SkDeserialTypefaceProc  fTypefaceProc = nullptr;
    void*                   fTypefaceCtx = nullptr;
};

#endif

