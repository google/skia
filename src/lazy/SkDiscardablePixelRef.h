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

class SkDiscardablePixelRef : public SkPixelRef {
public:
    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    ~SkDiscardablePixelRef();
    virtual void* onLockPixels(SkColorTable**) SK_OVERRIDE;
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

    /* Takes ownership of SkImageGenerator. */
    SkDiscardablePixelRef(const SkImageInfo&, SkImageGenerator*,
                          size_t rowBytes,
                          SkDiscardableMemory::Factory* factory);
    friend bool SkInstallDiscardablePixelRef(SkImageGenerator*,
                                             SkBitmap*,
                                             SkDiscardableMemory::Factory*);
    typedef SkPixelRef INHERITED;
};
#endif  // SkDiscardablePixelRef_DEFINED
