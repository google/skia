/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasStack_DEFINED
#define SkCanvasStack_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTypeTraits.h"
#include "include/utils/SkNWayCanvas.h"

#include <memory>
#include <type_traits>

class SkPath;
class SkRRect;
class SkShader;
enum class SkClipOp;
struct SkRect;

/**
 *  Like NWayCanvas, in that it forwards all canvas methods to each sub-canvas that is "pushed".
 *
 *  Unlike NWayCanvas, this takes ownership of each subcanvas, and deletes them when this canvas
 *  is deleted.
 */
class SkCanvasStack : public SkNWayCanvas {
public:
    SkCanvasStack(int width, int height);
    ~SkCanvasStack() override;

    void pushCanvas(std::unique_ptr<SkCanvas>, const SkIPoint& origin);
    void removeAll() override;

    /*
     * The following add/remove canvas methods are overrides from SkNWayCanvas
     * that do not make sense in the context of our CanvasStack, but since we
     * can share most of the other implementation of NWay we override those
     * methods to be no-ops.
     */
    void addCanvas(SkCanvas*) override { SkDEBUGFAIL("Invalid Op"); }
    void removeCanvas(SkCanvas*) override { SkDEBUGFAIL("Invalid Op"); }

protected:
    void didSetM44(const SkM44&) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

private:
    void clipToZOrderedBounds();

    struct CanvasData {
        SkIPoint origin;
        SkRegion requiredClip;
        std::unique_ptr<SkCanvas> ownedCanvas;

        static_assert(::sk_is_trivially_relocatable<decltype(origin)>::value);
        static_assert(::sk_is_trivially_relocatable<decltype(requiredClip)>::value);
        static_assert(::sk_is_trivially_relocatable<decltype(ownedCanvas)>::value);

        using sk_is_trivially_relocatable = std::true_type;
    };

    skia_private::TArray<CanvasData> fCanvasData;

    using INHERITED = SkNWayCanvas;
};

#endif
