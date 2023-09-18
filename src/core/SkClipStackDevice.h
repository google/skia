/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClipStackDevice_DEFINED
#define SkClipStackDevice_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkDevice.h"

#include <cstdint>

class SkPath;
class SkRRect;
class SkRegion;
class SkShader;
class SkSurfaceProps;
enum class SkClipOp;
struct SkImageInfo;

class SkClipStackDevice : public SkDevice {
public:
    SkClipStackDevice(const SkImageInfo& info, const SkSurfaceProps& props)
        : SkDevice(info, props)
        , fClipStack(fStorage, sizeof(fStorage))
    {}

    SkClipStack& cs() { return fClipStack; }
    const SkClipStack& cs() const { return fClipStack; }

    void pushClipStack() override;
    void popClipStack() override;

    void clipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void clipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void clipPath(const SkPath& path, SkClipOp, bool aa) override;
    void clipRegion(const SkRegion& deviceRgn, SkClipOp) override;

    void replaceClip(const SkIRect& rect) override;

    bool isClipAntiAliased() const override;
    bool isClipWideOpen() const override;
    bool isClipEmpty() const override;
    bool isClipRect() const override;

    void android_utils_clipAsRgn(SkRegion*) const override;

    SkIRect devClipBounds() const override;

private:
    // empirically determined, adjust as needed to reduce mallocs
    static constexpr int kPreallocCount = 16;

    void onClipShader(sk_sp<SkShader>) override;

    intptr_t fStorage[kPreallocCount * sizeof(SkClipStack::Element) / sizeof(intptr_t)];
    SkClipStack fClipStack;
};

#endif
