/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteTypeface_DEFINED
#define SkRemoteTypeface_DEFINED

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkScalerContext.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

class SkArenaAlloc;
class SkDescriptor;
class SkDrawable;
class SkFontDescriptor;
class SkGlyph;
class SkPath;
class SkReadBuffer;
class SkStreamAsset;
class SkTypefaceProxy;
class SkWriteBuffer;
struct SkAdvancedTypefaceMetrics;
struct SkFontMetrics;

class SkScalerContextProxy : public SkScalerContext {
public:
    SkScalerContextProxy(SkTypeface& tf,
                         const SkScalerContextEffects& effects,
                         const SkDescriptor* desc,
                         sk_sp<SkStrikeClient::DiscardableHandleManager> manager);

protected:
    GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override;
    void generateImage(const SkGlyph&, void*) override;
    bool generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph&) override;
    void generateFontMetrics(SkFontMetrics* metrics) override;
    SkTypefaceProxy* getProxyTypeface() const;

private:
    sk_sp<SkStrikeClient::DiscardableHandleManager> fDiscardableManager;
    using INHERITED = SkScalerContext;
};

// SkTypefaceProxyPrototype is the serialization format for SkTypefaceProxy.
class SkTypefaceProxyPrototype {
public:
    static std::optional<SkTypefaceProxyPrototype> MakeFromBuffer(SkReadBuffer& buffer);
    explicit SkTypefaceProxyPrototype(const SkTypeface& typeface);
    SkTypefaceProxyPrototype(SkTypefaceID typefaceID,
                             int glyphCount,
                             int32_t styleValue,
                             bool isFixedPitch,
                             bool glyphMaskNeedsCurrentColor);

    void flatten(SkWriteBuffer&buffer) const;
    SkTypefaceID serverTypefaceID() const { return fServerTypefaceID; }

private:
    friend class SkTypefaceProxy;
    SkFontStyle style() const {
        SkFontStyle style;
        style.fValue = fStyleValue;
        return style;
    }
    const SkTypefaceID fServerTypefaceID;
    const int fGlyphCount;
    const int32_t fStyleValue;
    const bool fIsFixedPitch;
    // Used for COLRv0 or COLRv1 fonts that may need the 0xFFFF special palette
    // index to represent foreground color. This information needs to be on here
    // to determine how this typeface can be cached.
    const bool fGlyphMaskNeedsCurrentColor;
};

class SkTypefaceProxy : public SkTypeface {
public:
    SkTypefaceProxy(const SkTypefaceProxyPrototype& prototype,
                    sk_sp<SkStrikeClient::DiscardableHandleManager> manager,
                    bool isLogging = true);

    SkTypefaceProxy(SkTypefaceID typefaceID,
                    int glyphCount,
                    const SkFontStyle& style,
                    bool isFixedPitch,
                    bool glyphMaskNeedsCurrentColor,
                    sk_sp<SkStrikeClient::DiscardableHandleManager> manager,
                    bool isLogging = true);

    SkTypefaceID remoteTypefaceID() const {return fTypefaceID;}

    int glyphCount() const {return fGlyphCount;}

    bool isLogging() const {return fIsLogging;}

protected:
    int onGetUPEM() const override { SK_ABORT("Should never be called."); }
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        SK_ABORT("Should never be called.");
    }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        SK_ABORT("Should never be called.");
    }
    bool onGlyphMaskNeedsCurrentColor() const override {
        return fGlyphMaskNeedsCurrentColor;
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        SK_ABORT("Should never be called.");
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override {
        SK_ABORT("Should never be called.");
    }
    void onGetFamilyName(SkString* familyName) const override {
        // Used by SkStrikeCache::DumpMemoryStatistics.
        *familyName = "";
    }
    bool onGetPostScriptName(SkString*) const override {
        SK_ABORT("Should never be called.");
    }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        SK_ABORT("Should never be called.");
    }
    int onGetTableTags(SkFontTableTag tags[]) const override {
        SK_ABORT("Should never be called.");
    }
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override {
        SK_ABORT("Should never be called.");
    }
    std::unique_ptr<SkScalerContext> onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const override
    {
        return std::make_unique<SkScalerContextProxy>(
                *const_cast<SkTypefaceProxy*>(this), effects, desc, fDiscardableManager);
    }
    void onFilterRec(SkScalerContextRec* rec) const override {
        // The rec filtering is already applied by the server when generating
        // the glyphs.
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {
        SK_ABORT("Should never be called.");
    }
    void getGlyphToUnicodeMap(SkUnichar*) const override {
        SK_ABORT("Should never be called.");
    }

    void getPostScriptGlyphNames(SkString*) const override {
        SK_ABORT("Should never be called.");
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        SK_ABORT("Should never be called.");
    }
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        SK_ABORT("Should never be called.");
    }
    int onCountGlyphs() const override {
        return this->glyphCount();
    }

    void* onGetCTFontRef() const override {
        SK_ABORT("Should never be called.");
    }

private:
    const SkTypefaceID                              fTypefaceID;
    const int                                       fGlyphCount;
    const bool                                      fIsLogging;
    const bool                                      fGlyphMaskNeedsCurrentColor;
    sk_sp<SkStrikeClient::DiscardableHandleManager> fDiscardableManager;
};

#endif  // SkRemoteTypeface_DEFINED
