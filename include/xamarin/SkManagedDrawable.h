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
typedef void       (*draw_delegate)               (SkManagedDrawable* managedDrawable, SkCanvas* canvas);
typedef void       (*getBounds_delegate)          (SkManagedDrawable* managedDrawable, SkRect* rect);
typedef SkPicture* (*newPictureSnapshot_delegate) (SkManagedDrawable* managedDrawable);


// managed drawable
class SkManagedDrawable : public SkDrawable {
public:
    SkManagedDrawable();

    virtual ~SkManagedDrawable();

    static void setDelegates(const draw_delegate pDraw,
                             const getBounds_delegate pgetBounds,
                             const newPictureSnapshot_delegate pNewPictureSnapshot);

protected:
    void onDraw(SkCanvas*) override;
    SkRect onGetBounds() override;
    SkPicture* onNewPictureSnapshot() override;

    typedef SkDrawable INHERITED;
};


#endif
