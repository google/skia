
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImageRef_ashmem_DEFINED
#define SkImageRef_ashmem_DEFINED

#include "SkImageRef.h"

struct SkAshmemRec {
    int     fFD;
    void*   fAddr;
    size_t  fSize;
    bool    fPinned;
};

class SkImageRef_ashmem : public SkImageRef {
public:
    SkImageRef_ashmem(const SkImageInfo&, SkStreamRewindable*, int sampleSize = 1);
    virtual ~SkImageRef_ashmem();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageRef_ashmem)

protected:
    SkImageRef_ashmem(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onDecode(SkImageDecoder* codec, SkStreamRewindable* stream,
                          SkBitmap* bitmap, SkBitmap::Config config,
                          SkImageDecoder::Mode mode);

    virtual bool onNewLockPixels(LockRec*) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;

private:
    void closeFD();

    SkColorTable* fCT;
    SkAshmemRec fRec;

    typedef SkImageRef INHERITED;
};

#endif
