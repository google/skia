
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
    SkImageRef_ashmem(SkStream*, SkBitmap::Config, int sampleSize = 1);
    virtual ~SkImageRef_ashmem();
    
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageRef_ashmem)

protected:
    SkImageRef_ashmem(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onDecode(SkImageDecoder* codec, SkStream* stream,
                          SkBitmap* bitmap, SkBitmap::Config config,
                          SkImageDecoder::Mode mode);
    
    virtual void* onLockPixels(SkColorTable**);
    virtual void onUnlockPixels();
    
private:
    void closeFD();

    SkColorTable* fCT;
    SkAshmemRec fRec;

    typedef SkImageRef INHERITED;
};

#endif
