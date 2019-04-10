/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCacheImpl_DEFINED
#define SkRemoteGlyphCacheImpl_DEFINED

#include "SkArenaAlloc.h"
#include "SkDescriptor.h"
#include "SkGlyphRun.h"
#include "SkGlyphRunPainter.h"
#include "SkRemoteGlyphCache.h"

class SkStrikeServer::SkGlyphCacheState : public SkStrikeInterface {
public:
    // N.B. SkGlyphCacheState is not valid until ensureScalerContext is called.
    SkGlyphCacheState(const SkDescriptor& descriptor,
                      std::unique_ptr<SkScalerContext> context,
                      SkDiscardableHandleId discardableHandleId);
    ~SkGlyphCacheState() override;

    void addGlyph(SkPackedGlyphID, bool pathOnly);
    void writePendingGlyphs(Serializer* serializer);
    SkDiscardableHandleId discardableHandleId() const { return fDiscardableHandleId; }

    bool isSubpixel() const { return fIsSubpixel; }
    SkAxisAlignment axisAlignmentForHText() const { return fAxisAlignmentForHText; }

    const SkDescriptor& getDescriptor() const override {
        return *fDescriptor.getDesc();
    }

    SkStrikeSpec strikeSpec() const override {
        return SkStrikeSpec(this->getDescriptor(), *fTypeface, fEffects);
    }

    void setTypefaceAndEffects(const SkTypeface* typeface, SkScalerContextEffects effects);

    SkVector rounding() const override;

    const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) override;

    SkSpan<const SkGlyphPos> prepareForDrawing(const SkGlyphID glyphIDs[],
                                               const SkPoint positions[],
                                               size_t n,
                                               int maxDimension,
                                               SkGlyphPos results[]) override;

    bool decideCouldDrawFromPath(const SkGlyph& glyph) override;

    void onAboutToExitScope() override {}

private:
    bool hasPendingGlyphs() const {
        return !fPendingGlyphImages.empty() || !fPendingGlyphPaths.empty();
    }
    void writeGlyphPath(const SkPackedGlyphID& glyphID, Serializer* serializer) const;

    void ensureScalerContext();
    void resetScalerContext();

    // The set of glyphs cached on the remote client.
    SkTHashSet<SkPackedGlyphID> fCachedGlyphImages;
    SkTHashSet<SkPackedGlyphID> fCachedGlyphPaths;

    // The set of glyphs which has not yet been serialized and sent to the
    // remote client.
    std::vector<SkPackedGlyphID> fPendingGlyphImages;
    std::vector<SkPackedGlyphID> fPendingGlyphPaths;

    const SkAutoDescriptor fDescriptor;

    const SkDiscardableHandleId fDiscardableHandleId;

    // Values saved from the initial context.
    const bool fIsSubpixel;
    const SkAxisAlignment fAxisAlignmentForHText;

    // The context built using fDescriptor
    std::unique_ptr<SkScalerContext> fContext;

    // These fields are set everytime getOrCreateCache. This allows the code to maintain the
    // fContext as lazy as possible.
    const SkTypeface* fTypeface{nullptr};
    SkScalerContextEffects fEffects;

    class GlyphMapHashTraits {
    public:
        static SkPackedGlyphID GetKey(const SkGlyph* glyph) {
            return glyph->getPackedID();
        }
        static uint32_t Hash(SkPackedGlyphID glyphId) {
            return glyphId.hash();
        }
    };

    // FallbackTextHelper cases require glyph metrics when analyzing a glyph run, in which case
    // we cache them here.
    SkTHashTable<SkGlyph*, SkPackedGlyphID, GlyphMapHashTraits> fGlyphMap;

    SkArenaAlloc fAlloc{256};
};

class SkTextBlobCacheDiffCanvas::TrackLayerDevice : public SkNoPixelsDevice {
public:
    TrackLayerDevice(const SkIRect& bounds, const SkSurfaceProps& props, SkStrikeServer* server,
                     sk_sp<SkColorSpace> colorSpace,
                     const SkTextBlobCacheDiffCanvas::Settings& settings);

    SkBaseDevice* onCreateDevice(const CreateInfo& cinfo, const SkPaint*) override;

protected:
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override;

private:
    SkStrikeServer* const fStrikeServer;
    const SkTextBlobCacheDiffCanvas::Settings fSettings;
    SkGlyphRunListPainter fPainter;
};

#endif // SkRemoteGlyphCacheImpl_DEFINED
