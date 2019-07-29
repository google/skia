/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkManagedDrawable_h
#define SkManagedDrawable_h

#include "SkTypes.h"
#include "SkDrawable.h"

class SkCanvas;
class SkPicture;
struct SkRect;

class SK_API SkManagedDrawable;

// delegate declarations


// managed drawable
class SkManagedDrawable : public SkDrawable {
public:
    SkManagedDrawable(void* context);

    virtual ~SkManagedDrawable();

public:
    typedef void       (*DrawProc)               (SkManagedDrawable* d, void* context, SkCanvas* canvas);
    typedef void       (*GetBoundsProc)          (SkManagedDrawable* d, void* context, SkRect* rect);
    typedef SkPicture* (*NewPictureSnapshotProc) (SkManagedDrawable* d, void* context);
    typedef void       (*DestroyProc)            (SkManagedDrawable* d, void* context);

    struct Procs {
        DrawProc fDraw = nullptr;
        GetBoundsProc fGetBounds = nullptr;
        NewPictureSnapshotProc fNewPictureSnapshot = nullptr;
        DestroyProc fDestroy = nullptr;
    };

    static void setProcs(Procs procs);

protected:
    void onDraw(SkCanvas*) override;
    SkRect onGetBounds() override;
    SkPicture* onNewPictureSnapshot() override;

private:
    void* fContext;
    static Procs fProcs;

    typedef SkDrawable INHERITED;
};


#endif
