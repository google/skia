/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscardablePixelRef_DEFINED
#define SkDiscardablePixelRef_DEFINED

#include "SkDiscardableMemory.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"
#include "SkPixelRef.h"

/**
 *  A PixelRef backed by SkDiscardableMemory, with the ability to
 *  re-generate the pixels (via a SkImageGenerator) if the DM is
 *  purged.
 */
class SkDiscardablePixelRef : public SkPixelRef {
public:
    SK_DECLARE_INST_COUNT(SkDiscardablePixelRef)

protected:
    ~SkDiscardablePixelRef();

    virtual bool onNewLockPixels(LockRec*) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual bool onLockPixelsAreWritable() const SK_OVERRIDE { return false; }

    virtual SkData* onRefEncodedData() SK_OVERRIDE {
        return fGenerator->refEncodedData();
    }

private:
    SkImageGenerator* const fGenerator;
    SkDiscardableMemory::Factory* const fDMFactory;
    const size_t fRowBytes;
    // These const members should not change over the life of the
    // PixelRef, since the SkBitmap doesn't expect them to change.

    SkDiscardableMemory* fDiscardableMemory;
    SkAutoTUnref<SkColorTable> fCTable;

    /* Takes ownership of SkImageGenerator. */
    SkDiscardablePixelRef(const SkImageInfo&, SkImageGenerator*,
                          size_t rowBytes,
                          SkDiscardableMemory::Factory* factory);

    virtual bool onGetYUV8Planes(SkISize sizes[3],
                                 void* planes[3],
                                 size_t rowBytes[3]) SK_OVERRIDE {
        return fGenerator->getYUV8Planes(sizes, planes, rowBytes);
    }

    friend bool SkInstallDiscardablePixelRef(SkImageGenerator*, SkBitmap*,
                                             SkDiscardableMemory::Factory*);

    typedef SkPixelRef INHERITED;
};

#endif  // SkDiscardablePixelRef_DEFINED
