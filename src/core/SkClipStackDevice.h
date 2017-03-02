/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipStackDevice_DEFINED
#define SkClipStackDevice_DEFINED

#include "SkClipStack.h"
#include "SkDevice.h"

class SK_API SkClipStackDevice : public SkBaseDevice {
public:
    SkClipStackDevice(const SkImageInfo& info, const SkSurfaceProps& props)
        : SkBaseDevice(info, props)
    {}

    const SkClipStack& cs() const { return fClipStack; }

    SkIRect devClipBounds() const;

protected:
    void onSave() override;
    void onRestore() override;
    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override;
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override;
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override;
    bool onClipIsAA() const override;
    void onAsRgnClip(SkRegion*) const override;

private:
    SkClipStack fClipStack;

    typedef SkBaseDevice INHERITED;
};

#endif
