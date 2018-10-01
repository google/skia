/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCacheImpl_DEFINED
#define SkRemoteGlyphCacheImpl_DEFINED

#include "SkDescriptor.h"
#include "SkGlyphRun.h"
#include "SkGlyphRunPainter.h"
#include "SkRemoteGlyphCache.h"

class SkStrikeServer::SkGlyphCacheState : public SkGlyphCacheInterface {
public:
    // N.B. SkGlyphCacheState is not valid until ensureScalerContext is called.
    SkGlyphCacheState(const SkDescriptor& keyDescriptor,
                      const SkDescriptor& deviceDescriptor,
                      std::unique_ptr<SkScalerContext> context,
                      SkDiscardableHandleId discardableHandleId);
    ~SkGlyphCacheState() override;

    void addGlyph(SkPackedGlyphID, bool pathOnly);
    void writePendingGlyphs(Serializer* serializer);
    SkDiscardableHandleId discardableHandleId() const { return fDiscardableHandleId; }
    const SkDescriptor& getDeviceDescriptor() {
        return *fDeviceDescriptor.getDesc();
    }

    bool isSubpixel() const { return fIsSubpixel; }
    SkAxisAlignment axisAlignmentForHText() const { return fAxisAlignmentForHText; }

    const SkDescriptor& getKeyDescriptor() {
        return *fKeyDescriptor.getDesc();
    }

    const SkGlyph& findGlyph(SkPackedGlyphID);

    void setPaint(const SkPaint& paint);

    SkVector rounding() const override;

    const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) override;

private:
    bool hasPendingGlyphs() const {
        return !fPendingGlyphImages.empty() || !fPendingGlyphPaths.empty();
    }
    void writeGlyphPath(const SkPackedGlyphID& glyphID, Serializer* serializer) const;

    void ensureScalerContext();

    // The set of glyphs cached on the remote client.
    SkTHashSet<SkPackedGlyphID> fCachedGlyphImages;
    SkTHashSet<SkPackedGlyphID> fCachedGlyphPaths;

    // The set of glyphs which has not yet been serialized and sent to the
    // remote client.
    std::vector<SkPackedGlyphID> fPendingGlyphImages;
    std::vector<SkPackedGlyphID> fPendingGlyphPaths;

    // The device descriptor is used to create the scaler context. The glyphs to have the
    // correct device rendering. The key descriptor is used for communication. The GPU side will
    // create descriptors with out the device filtering, thus matching the key descriptor.
    const SkAutoDescriptor fKeyDescriptor;
    const SkAutoDescriptor fDeviceDescriptor;

    const SkDiscardableHandleId fDiscardableHandleId;

    // Values saved from the initial context.
    const bool fIsSubpixel;
    const SkAxisAlignment fAxisAlignmentForHText;

    // The context built using fDeviceDescriptor
    std::unique_ptr<SkScalerContext> fContext;

    // This field is set everytime getOrCreateCache. This allows the code to maintain the fContext
    // as lazy as possible.
    const SkPaint* fPaint{nullptr};

    // FallbackTextHelper cases require glyph metrics when analyzing a glyph run, in which case
    // we cache them here.
    SkTHashMap<SkPackedGlyphID, SkGlyph> fGlyphMap;
};

class SkTextBlobCacheDiffCanvas::TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props, SkStrikeServer* server,
                     const SkTextBlobCacheDiffCanvas::Settings& settings);

    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override;

protected:
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;

private:
    void processGlyphRun(const SkPoint& origin, const SkGlyphRun& glyphRun);

    void processGlyphRunForMask(
            const SkGlyphRun& glyphRun, const SkMatrix& runMatrix, SkPoint origin);

    void processGlyphRunForPaths(
            const SkGlyphRun& glyphRun, const SkMatrix& runMatrix, SkPoint origin);

#if SK_SUPPORT_GPU
    bool maybeProcessGlyphRunForDFT(
            const SkGlyphRun& glyphRun, const SkMatrix& runMatrix, SkPoint origin);
#endif

    SkStrikeServer* const fStrikeServer;
    const SkTextBlobCacheDiffCanvas::Settings fSettings;
    SkGlyphRunListPainter fPainter;
};

#endif // SkRemoteGlyphCacheImpl_DEFINED
