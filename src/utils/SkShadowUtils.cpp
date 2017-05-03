/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkResourceCache.h"
#include "SkShadowTessellator.h"
#include "SkString.h"
#include "SkTLazy.h"
#include "SkVertices.h"
#if SK_SUPPORT_GPU
#include "GrShape.h"
#include "effects/GrBlurredEdgeFragmentProcessor.h"
#endif
#include "../../src/effects/shadows/SkAmbientShadowMaskFilter.h"
#include "../../src/effects/shadows/SkSpotShadowMaskFilter.h"

/**
*  Gaussian color filter -- produces a Gaussian ramp based on the color's B value,
*                           then blends with the color's G value.
*                           Final result is black with alpha of Gaussian(B)*G.
*                           The assumption is that the original color's alpha is 1.
*/
class SK_API SkGaussianColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make() {
        return sk_sp<SkColorFilter>(new SkGaussianColorFilter);
    }

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGaussianColorFilter)

protected:
    void flatten(SkWriteBuffer&) const override {}

private:
    SkGaussianColorFilter() : INHERITED() {}

    typedef SkColorFilter INHERITED;
};

static void build_table() {
    SkDebugf("const uint16_t gByteExpU16Table[256] = {");
    for (int i = 0; i <= 255; ++i) {
        if (!(i % 8)) {
            SkDebugf("\n");
        }
        SkScalar factor = SK_Scalar1 - i / 255.f;
        factor = SkScalarExp(-factor * factor * 4) - 0.018f;
        int v = (int)(factor * 65536);
        SkDebugf(" 0x%04X,", v);
    }
    SkDebugf("\n};\n");
}

const uint16_t gByteExpU16Table[256] = {
    0x0014, 0x003A, 0x0062, 0x008A, 0x00B3, 0x00DE, 0x010A, 0x0136,
    0x0165, 0x0194, 0x01C4, 0x01F6, 0x0229, 0x025E, 0x0294, 0x02CB,
    0x0304, 0x033E, 0x0379, 0x03B7, 0x03F5, 0x0435, 0x0477, 0x04BB,
    0x0500, 0x0546, 0x058F, 0x05D9, 0x0625, 0x0673, 0x06C3, 0x0714,
    0x0768, 0x07BD, 0x0814, 0x086E, 0x08C9, 0x0926, 0x0986, 0x09E8,
    0x0A4B, 0x0AB1, 0x0B1A, 0x0B84, 0x0BF1, 0x0C60, 0x0CD2, 0x0D46,
    0x0DBC, 0x0E35, 0x0EB0, 0x0F2E, 0x0FAF, 0x1032, 0x10B7, 0x1140,
    0x11CB, 0x1258, 0x12E9, 0x137C, 0x1412, 0x14AB, 0x1547, 0x15E6,
    0x1688, 0x172D, 0x17D5, 0x187F, 0x192D, 0x19DE, 0x1A92, 0x1B4A,
    0x1C04, 0x1CC2, 0x1D83, 0x1E47, 0x1F0E, 0x1FD9, 0x20A7, 0x2178,
    0x224D, 0x2325, 0x2401, 0x24E0, 0x25C2, 0x26A8, 0x2792, 0x287F,
    0x296F, 0x2A63, 0x2B5A, 0x2C56, 0x2D54, 0x2E56, 0x2F5C, 0x3065,
    0x3172, 0x3283, 0x3397, 0x34AE, 0x35CA, 0x36E9, 0x380B, 0x3931,
    0x3A5B, 0x3B88, 0x3CB9, 0x3DED, 0x3F25, 0x4061, 0x41A0, 0x42E2,
    0x4428, 0x4572, 0x46BF, 0x480F, 0x4963, 0x4ABA, 0x4C14, 0x4D72,
    0x4ED3, 0x5038, 0x519F, 0x530A, 0x5478, 0x55E9, 0x575D, 0x58D4,
    0x5A4F, 0x5BCC, 0x5D4C, 0x5ECF, 0x6054, 0x61DD, 0x6368, 0x64F6,
    0x6686, 0x6819, 0x69AE, 0x6B45, 0x6CDF, 0x6E7B, 0x701A, 0x71BA,
    0x735D, 0x7501, 0x76A7, 0x784F, 0x79F9, 0x7BA4, 0x7D51, 0x7F00,
    0x80AF, 0x8260, 0x8413, 0x85C6, 0x877A, 0x8930, 0x8AE6, 0x8C9C,
    0x8E54, 0x900C, 0x91C4, 0x937D, 0x9535, 0x96EE, 0x98A7, 0x9A60,
    0x9C18, 0x9DD1, 0x9F88, 0xA13F, 0xA2F6, 0xA4AB, 0xA660, 0xA814,
    0xA9C6, 0xAB78, 0xAD27, 0xAED6, 0xB082, 0xB22D, 0xB3D6, 0xB57D,
    0xB722, 0xB8C5, 0xBA65, 0xBC03, 0xBD9E, 0xBF37, 0xC0CD, 0xC25F,
    0xC3EF, 0xC57B, 0xC704, 0xC889, 0xCA0B, 0xCB89, 0xCD04, 0xCE7A,
    0xCFEC, 0xD15A, 0xD2C4, 0xD429, 0xD58A, 0xD6E6, 0xD83D, 0xD990,
    0xDADD, 0xDC25, 0xDD68, 0xDEA6, 0xDFDE, 0xE111, 0xE23E, 0xE365,
    0xE486, 0xE5A2, 0xE6B7, 0xE7C6, 0xE8CF, 0xE9D1, 0xEACD, 0xEBC3,
    0xECB2, 0xED9A, 0xEE7C, 0xEF56, 0xF02A, 0xF0F6, 0xF1BC, 0xF27A,
    0xF332, 0xF3E1, 0xF48A, 0xF52B, 0xF5C5, 0xF657, 0xF6E1, 0xF764,
    0xF7DF, 0xF852, 0xF8BE, 0xF922, 0xF97E, 0xF9D2, 0xFA1E, 0xFA62,
    0xFA9F, 0xFAD3, 0xFAFF, 0xFB23, 0xFB40, 0xFB54, 0xFB60, 0xFB64,
};

void SkGaussianColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    // to re-build the table, call build_table() which will dump it out using SkDebugf.
    if (false) {
        build_table();
    }
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];
        uint8_t a = gByteExpU16Table[SkGetPackedB32(c)] * SkGetPackedG32(c) >> 16;
        dst[i] = SkPackARGB32(a, a, a, a);
    }
}

sk_sp<SkFlattenable> SkGaussianColorFilter::CreateProc(SkReadBuffer&) {
    return Make();
}

#ifndef SK_IGNORE_TO_STRING
void SkGaussianColorFilter::toString(SkString* str) const {
    str->append("SkGaussianColorFilter ");
}
#endif

#if SK_SUPPORT_GPU

sk_sp<GrFragmentProcessor> SkGaussianColorFilter::asFragmentProcessor(GrContext*,
                                                                      SkColorSpace*) const {
    return GrBlurredEdgeFP::Make(GrBlurredEdgeFP::kGaussian_Mode);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

uint64_t resource_cache_shared_id() {
    return 0x2020776f64616873llu;  // 'shadow  '
}

/** Factory for an ambient shadow mesh with particular shadow properties. */
struct AmbientVerticesFactory {
    SkScalar fOccluderHeight = SK_ScalarNaN;  // NaN so that isCompatible will fail until init'ed.
    SkScalar fAmbientAlpha;
    bool fTransparent;

    bool isCompatible(const AmbientVerticesFactory& that, SkVector* translate) const {
        if (fOccluderHeight != that.fOccluderHeight || fAmbientAlpha != that.fAmbientAlpha ||
            fTransparent != that.fTransparent) {
            return false;
        }
        translate->set(0, 0);
        return true;
    }

    sk_sp<SkVertices> makeVertices(const SkPath& path, const SkMatrix& ctm) const {
        SkScalar z = fOccluderHeight;
        return SkShadowTessellator::MakeAmbient(path, ctm,
                                                [z](SkScalar, SkScalar) { return z; },
                                                fAmbientAlpha, fTransparent);
    }
};

/** Factory for an spot shadow mesh with particular shadow properties. */
struct SpotVerticesFactory {
    enum class OccluderType {
        // The umbra cannot be dropped out because the occluder is not opaque.
        kTransparent,
        // The umbra can be dropped where it is occluded.
        kOpaque,
        // It is known that the entire umbra is occluded.
        kOpaqueCoversUmbra
    };

    SkVector fOffset;
    SkScalar fOccluderHeight = SK_ScalarNaN; // NaN so that isCompatible will fail until init'ed.
    SkPoint3 fDevLightPos;
    SkScalar fLightRadius;
    SkScalar fSpotAlpha;
    OccluderType fOccluderType;

    bool isCompatible(const SpotVerticesFactory& that, SkVector* translate) const {
        if (fOccluderHeight != that.fOccluderHeight || fDevLightPos.fZ != that.fDevLightPos.fZ ||
            fLightRadius != that.fLightRadius || fSpotAlpha != that.fSpotAlpha ||
            fOccluderType != that.fOccluderType) {
            return false;
        }
        switch (fOccluderType) {
            case OccluderType::kTransparent:
            case OccluderType::kOpaqueCoversUmbra:
                // 'this' and 'that' will either both have no umbra removed or both have all the
                // umbra removed.
                *translate = that.fOffset - fOffset;
                return true;
            case OccluderType::kOpaque:
                // In this case we partially remove the umbra differently for 'this' and 'that'
                // if the offsets don't match.
                if (fOffset == that.fOffset) {
                    translate->set(0, 0);
                    return true;
                }
                return false;
        }
        SkFAIL("Uninitialized occluder type?");
        return false;
    }

    sk_sp<SkVertices> makeVertices(const SkPath& path, const SkMatrix& ctm) const {
        bool transparent = OccluderType::kTransparent == fOccluderType;
        SkScalar z = fOccluderHeight;
        return SkShadowTessellator::MakeSpot(path, ctm,
                                             [z](SkScalar, SkScalar) -> SkScalar { return z; },
                                             fDevLightPos, fLightRadius,
                                             fSpotAlpha, transparent);
    }
};

/**
 * This manages a set of tessellations for a given shape in the cache. Because SkResourceCache
 * records are immutable this is not itself a Rec. When we need to update it we return this on
 * the FindVisitor and let the cache destory the Rec. We'll update the tessellations and then add
 * a new Rec with an adjusted size for any deletions/additions.
 */
class CachedTessellations : public SkRefCnt {
public:
    size_t size() const { return fAmbientSet.size() + fSpotSet.size(); }

    sk_sp<SkVertices> find(const AmbientVerticesFactory& ambient, const SkMatrix& matrix,
                           SkVector* translate) const {
        return fAmbientSet.find(ambient, matrix, translate);
    }

    sk_sp<SkVertices> add(const SkPath& devPath, const AmbientVerticesFactory& ambient,
                          const SkMatrix& matrix) {
        return fAmbientSet.add(devPath, ambient, matrix);
    }

    sk_sp<SkVertices> find(const SpotVerticesFactory& spot, const SkMatrix& matrix,
                           SkVector* translate) const {
        return fSpotSet.find(spot, matrix, translate);
    }

    sk_sp<SkVertices> add(const SkPath& devPath, const SpotVerticesFactory& spot,
                          const SkMatrix& matrix) {
        return fSpotSet.add(devPath, spot, matrix);
    }

private:
    template <typename FACTORY, int MAX_ENTRIES>
    class Set {
    public:
        size_t size() const { return fSize; }

        sk_sp<SkVertices> find(const FACTORY& factory, const SkMatrix& matrix,
                               SkVector* translate) const {
            for (int i = 0; i < MAX_ENTRIES; ++i) {
                if (fEntries[i].fFactory.isCompatible(factory, translate)) {
                    const SkMatrix& m = fEntries[i].fMatrix;
                    if (matrix.hasPerspective() || m.hasPerspective()) {
                        if (matrix != fEntries[i].fMatrix) {
                            continue;
                        }
                    } else if (matrix.getScaleX() != m.getScaleX() ||
                               matrix.getSkewX() != m.getSkewX() ||
                               matrix.getScaleY() != m.getScaleY() ||
                               matrix.getSkewY() != m.getSkewY()) {
                        continue;
                    }
                    *translate += SkVector{matrix.getTranslateX() - m.getTranslateX(),
                                           matrix.getTranslateY() - m.getTranslateY()};
                    return fEntries[i].fVertices;
                }
            }
            return nullptr;
        }

        sk_sp<SkVertices> add(const SkPath& path, const FACTORY& factory, const SkMatrix& matrix) {
            sk_sp<SkVertices> vertices = factory.makeVertices(path, matrix);
            if (!vertices) {
                return nullptr;
            }
            int i;
            if (fCount < MAX_ENTRIES) {
                i = fCount++;
            } else {
                i = gRandom.nextULessThan(MAX_ENTRIES);
                fSize -= fEntries[i].fVertices->approximateSize();
            }
            fEntries[i].fFactory = factory;
            fEntries[i].fVertices = vertices;
            fEntries[i].fMatrix = matrix;
            fSize += vertices->approximateSize();
            return vertices;
        }

    private:
        struct Entry {
            FACTORY fFactory;
            sk_sp<SkVertices> fVertices;
            SkMatrix fMatrix;
        };
        Entry fEntries[MAX_ENTRIES];
        int fCount = 0;
        size_t fSize = 0;
    };

    Set<AmbientVerticesFactory, 4> fAmbientSet;
    Set<SpotVerticesFactory, 4> fSpotSet;

    static SkRandom gRandom;
};

SkRandom CachedTessellations::gRandom;

/**
 * A record of shadow vertices stored in SkResourceCache of CachedTessellations for a particular
 * path. The key represents the path's geometry and not any shadow params.
 */
class CachedTessellationsRec : public SkResourceCache::Rec {
public:
    CachedTessellationsRec(const SkResourceCache::Key& key,
                           sk_sp<CachedTessellations> tessellations)
            : fTessellations(std::move(tessellations)) {
        fKey.reset(new uint8_t[key.size()]);
        memcpy(fKey.get(), &key, key.size());
    }

    const Key& getKey() const override {
        return *reinterpret_cast<SkResourceCache::Key*>(fKey.get());
    }

    size_t bytesUsed() const override { return fTessellations->size(); }

    const char* getCategory() const override { return "tessellated shadow masks"; }

    sk_sp<CachedTessellations> refTessellations() const { return fTessellations; }

    template <typename FACTORY>
    sk_sp<SkVertices> find(const FACTORY& factory, const SkMatrix& matrix,
                           SkVector* translate) const {
        return fTessellations->find(factory, matrix, translate);
    }

private:
    std::unique_ptr<uint8_t[]> fKey;
    sk_sp<CachedTessellations> fTessellations;
};

/**
 * Used by FindVisitor to determine whether a cache entry can be reused and if so returns the
 * vertices and a translation vector. If the CachedTessellations does not contain a suitable
 * mesh then we inform SkResourceCache to destroy the Rec and we return the CachedTessellations
 * to the caller. The caller will update it and reinsert it back into the cache.
 */
template <typename FACTORY>
struct FindContext {
    FindContext(const SkMatrix* viewMatrix, const FACTORY* factory)
            : fViewMatrix(viewMatrix), fFactory(factory) {}
    const SkMatrix* const fViewMatrix;
    // If this is valid after Find is called then we found the vertices and they should be drawn
    // with fTranslate applied.
    sk_sp<SkVertices> fVertices;
    SkVector fTranslate = {0, 0};

    // If this is valid after Find then the caller should add the vertices to the tessellation set
    // and create a new CachedTessellationsRec and insert it into SkResourceCache.
    sk_sp<CachedTessellations> fTessellationsOnFailure;

    const FACTORY* fFactory;
};

/**
 * Function called by SkResourceCache when a matching cache key is found. The FACTORY and matrix of
 * the FindContext are used to determine if the vertices are reusable. If so the vertices and
 * necessary translation vector are set on the FindContext.
 */
template <typename FACTORY>
bool FindVisitor(const SkResourceCache::Rec& baseRec, void* ctx) {
    FindContext<FACTORY>* findContext = (FindContext<FACTORY>*)ctx;
    const CachedTessellationsRec& rec = static_cast<const CachedTessellationsRec&>(baseRec);
    findContext->fVertices =
            rec.find(*findContext->fFactory, *findContext->fViewMatrix, &findContext->fTranslate);
    if (findContext->fVertices) {
        return true;
    }
    // We ref the tessellations and let the cache destroy the Rec. Once the tessellations have been
    // manipulated we will add a new Rec.
    findContext->fTessellationsOnFailure = rec.refTessellations();
    return false;
}

class ShadowedPath {
public:
    ShadowedPath(const SkPath* path, const SkMatrix* viewMatrix)
            : fPath(path)
            , fViewMatrix(viewMatrix)
#if SK_SUPPORT_GPU
            , fShapeForKey(*path, GrStyle::SimpleFill())
#endif
    {}

    const SkPath& path() const { return *fPath; }
    const SkMatrix& viewMatrix() const { return *fViewMatrix; }
#if SK_SUPPORT_GPU
    /** Negative means the vertices should not be cached for this path. */
    int keyBytes() const { return fShapeForKey.unstyledKeySize() * sizeof(uint32_t); }
    void writeKey(void* key) const {
        fShapeForKey.writeUnstyledKey(reinterpret_cast<uint32_t*>(key));
    }
    bool isRRect(SkRRect* rrect) { return fShapeForKey.asRRect(rrect, nullptr, nullptr, nullptr); }
#else
    int keyBytes() const { return -1; }
    void writeKey(void* key) const { SkFAIL("Should never be called"); }
    bool isRRect(SkRRect* rrect) { return false; }
#endif

private:
    const SkPath* fPath;
    const SkMatrix* fViewMatrix;
#if SK_SUPPORT_GPU
    GrShape fShapeForKey;
#endif
};

// This creates a domain of keys in SkResourceCache used by this file.
static void* kNamespace;

/**
 * Draws a shadow to 'canvas'. The vertices used to draw the shadow are created by 'factory' unless
 * they are first found in SkResourceCache.
 */
template <typename FACTORY>
void draw_shadow(const FACTORY& factory, SkCanvas* canvas, ShadowedPath& path, SkColor color,
                 SkResourceCache* cache) {
    FindContext<FACTORY> context(&path.viewMatrix(), &factory);

    SkResourceCache::Key* key = nullptr;
    SkAutoSTArray<32 * 4, uint8_t> keyStorage;
    int keyDataBytes = path.keyBytes();
    if (keyDataBytes >= 0) {
        keyStorage.reset(keyDataBytes + sizeof(SkResourceCache::Key));
        key = new (keyStorage.begin()) SkResourceCache::Key();
        path.writeKey((uint32_t*)(keyStorage.begin() + sizeof(*key)));
        key->init(&kNamespace, resource_cache_shared_id(), keyDataBytes);
        if (cache) {
            cache->find(*key, FindVisitor<FACTORY>, &context);
        } else {
            SkResourceCache::Find(*key, FindVisitor<FACTORY>, &context);
        }
    }

    sk_sp<SkVertices> vertices;
    const SkVector* translate;
    static constexpr SkVector kZeroTranslate = {0, 0};
    bool foundInCache = SkToBool(context.fVertices);
    if (foundInCache) {
        vertices = std::move(context.fVertices);
        translate = &context.fTranslate;
    } else {
        // TODO: handle transforming the path as part of the tessellator
        if (key) {
            // Update or initialize a tessellation set and add it to the cache.
            sk_sp<CachedTessellations> tessellations;
            if (context.fTessellationsOnFailure) {
                tessellations = std::move(context.fTessellationsOnFailure);
            } else {
                tessellations.reset(new CachedTessellations());
            }
            vertices = tessellations->add(path.path(), factory, path.viewMatrix());
            if (!vertices) {
                return;
            }
            auto rec = new CachedTessellationsRec(*key, std::move(tessellations));
            if (cache) {
                cache->add(rec);
            } else {
                SkResourceCache::Add(rec);
            }
        } else {
            vertices = factory.makeVertices(path.path(), path.viewMatrix());
            if (!vertices) {
                return;
            }
        }
        translate = &kZeroTranslate;
    }

    SkPaint paint;
    // Run the vertex color through a GaussianColorFilter and then modulate the grayscale result of
    // that against our 'color' param.
    paint.setColorFilter(SkColorFilter::MakeComposeFilter(
            SkColorFilter::MakeModeFilter(color, SkBlendMode::kModulate),
            SkGaussianColorFilter::Make()));
    if (translate->fX || translate->fY) {
        canvas->save();
        canvas->translate(translate->fX, translate->fY);
    }
    canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
    if (translate->fX || translate->fY) {
        canvas->restore();
    }
}
}

static bool draw_analytic_shadows(SkCanvas* canvas, const SkPath& path, SkScalar occluderZ,
                                  const SkPoint3& devLightPos, SkScalar lightRadius,
                                  SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                                  uint32_t flags) {
    // only supported in GPU code
    if (!canvas->getGrContext()) {
        return false;
    }

    SkRect rect;
    SkRRect rrect;
    if (canvas->getTotalMatrix().isSimilarity()) {
        if (path.isRect(&rect)) {
            SkPaint newPaint;
            newPaint.setColor(color);
            if (ambientAlpha > 0) {
                newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderZ,
                                                                       ambientAlpha, flags));
                canvas->drawRect(rect, newPaint);
            }
            if (spotAlpha > 0) {
                newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderZ, devLightPos,
                                                                    lightRadius, spotAlpha,
                                                                    flags));
                canvas->drawRect(rect, newPaint);
            }
            return true;
        } else if (path.isRRect(&rrect) && rrect.isSimpleCircular() &&
                   rrect.radii(SkRRect::kUpperLeft_Corner).fX > SK_ScalarNearlyZero) {
            SkPaint newPaint;
            newPaint.setColor(color);
            if (ambientAlpha > 0) {
                newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderZ,
                                                                       ambientAlpha, flags));
                canvas->drawRRect(rrect, newPaint);
            }
            if (spotAlpha > 0) {
                newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderZ, devLightPos,
                                                                    lightRadius, spotAlpha,
                                                                    flags));
                canvas->drawRRect(rrect, newPaint);
            }
            return true;
        } else if (path.isOval(&rect) && SkScalarNearlyEqual(rect.width(), rect.height()) &&
                   rect.width() > SK_ScalarNearlyZero) {
            SkPaint newPaint;
            newPaint.setColor(color);
            if (ambientAlpha > 0) {
                newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderZ,
                                                                       ambientAlpha, flags));
                canvas->drawOval(rect, newPaint);
            }
            if (spotAlpha > 0) {
                newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderZ, devLightPos,
                                                                    lightRadius, spotAlpha,
                                                                    flags));
                canvas->drawOval(rect, newPaint);
            }
            return true;
        }
    }

    return false;
}

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& devLightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags, SkResourceCache* cache) {
    // try fast paths
    if (draw_analytic_shadows(canvas, path, occluderHeight, devLightPos, lightRadius,
                              ambientAlpha, spotAlpha, color, flags)) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    SkMatrix viewMatrix = canvas->getTotalMatrix();
    canvas->resetMatrix();

    ShadowedPath shadowedPath(&path, &viewMatrix);

    bool transparent = SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (ambientAlpha > 0) {
        ambientAlpha = SkTMin(ambientAlpha, 1.f);
        AmbientVerticesFactory factory;
        factory.fOccluderHeight = occluderHeight;
        factory.fAmbientAlpha = ambientAlpha;
        factory.fTransparent = transparent;

        draw_shadow(factory, canvas, shadowedPath, color, cache);
    }

    if (spotAlpha > 0) {
        spotAlpha = SkTMin(spotAlpha, 1.f);
        SpotVerticesFactory factory;
        float zRatio = SkTPin(occluderHeight / (devLightPos.fZ - occluderHeight), 0.0f, 0.95f);
        SkScalar radius = lightRadius * zRatio;

        // Compute the scale and translation for the spot shadow.
        SkScalar scale = devLightPos.fZ / (devLightPos.fZ - occluderHeight);

        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        viewMatrix.mapPoints(&center, 1);
        factory.fOffset = SkVector::Make(zRatio * (center.fX - devLightPos.fX),
                                         zRatio * (center.fY - devLightPos.fY));
        factory.fOccluderHeight = occluderHeight;
        factory.fDevLightPos = devLightPos;
        factory.fLightRadius = lightRadius;
        factory.fSpotAlpha = spotAlpha;

        SkRRect rrect;
        if (transparent) {
            factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
        } else {
            factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaque;
            if (shadowedPath.isRRect(&rrect)) {
                SkRRect devRRect;
                if (rrect.transform(viewMatrix, &devRRect)) {
                    SkScalar s = 1.f - scale;
                    SkScalar w = devRRect.width();
                    SkScalar h = devRRect.height();
                    SkScalar hw = w / 2.f;
                    SkScalar hh = h / 2.f;
                    SkScalar umbraInsetX = s * hw + radius;
                    SkScalar umbraInsetY = s * hh + radius;
                    // The umbra is inset by radius along the diagonal, so adjust for that.
                    SkScalar d = 1.f / SkScalarSqrt(hw * hw + hh * hh);
                    umbraInsetX *= hw * d;
                    umbraInsetY *= hh * d;
                    if (umbraInsetX > hw || umbraInsetY > hh) {
                        // There is no umbra to occlude.
                        factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
                    } else if (fabsf(factory.fOffset.fX) < umbraInsetX &&
                               fabsf(factory.fOffset.fY) < umbraInsetY) {
                        factory.fOccluderType =
                                SpotVerticesFactory::OccluderType::kOpaqueCoversUmbra;
                    } else if (factory.fOffset.fX > w - umbraInsetX ||
                               factory.fOffset.fY > h - umbraInsetY) {
                        // There umbra is fully exposed, there is nothing to omit.
                        factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
                    }
                }
            }
        }
        if (factory.fOccluderType == SpotVerticesFactory::OccluderType::kOpaque) {
            factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
        }
        draw_shadow(factory, canvas, shadowedPath, color, cache);
    }
}

// Draw an offset spot shadow and outlining ambient shadow for the given path,
// without caching and using a function based on local position to compute the height.
void SkShadowUtils::DrawUncachedShadow(SkCanvas* canvas, const SkPath& path,
                                       std::function<SkScalar(SkScalar, SkScalar)> heightFunc,
                                       const SkPoint3& lightPos, SkScalar lightRadius,
                                       SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                                       uint32_t flags) {
    // try fast paths
    if (draw_analytic_shadows(canvas, path, heightFunc(0, 0), lightPos, lightRadius,
                              ambientAlpha, spotAlpha, color, flags)) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    SkMatrix viewMatrix = canvas->getTotalMatrix();
    canvas->resetMatrix();

    bool transparent = SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (ambientAlpha > 0) {
        ambientAlpha = SkTMin(ambientAlpha, 1.f);
        sk_sp<SkVertices> vertices = SkShadowTessellator::MakeAmbient(path, viewMatrix,
                                                                      heightFunc, ambientAlpha,
                                                                      transparent);
        SkPaint paint;
        // Run the vertex color through a GaussianColorFilter and then modulate the grayscale
        // result of that against our 'color' param.
        paint.setColorFilter(SkColorFilter::MakeComposeFilter(
            SkColorFilter::MakeModeFilter(color, SkBlendMode::kModulate),
            SkGaussianColorFilter::Make()));
        canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
    }

    if (spotAlpha > 0) {
        spotAlpha = SkTMin(spotAlpha, 1.f);
        sk_sp<SkVertices> vertices = SkShadowTessellator::MakeSpot(path, viewMatrix, heightFunc,
                                                                   lightPos, lightRadius,
                                                                   spotAlpha, transparent);
        SkPaint paint;
        // Run the vertex color through a GaussianColorFilter and then modulate the grayscale
        // result of that against our 'color' param.
        paint.setColorFilter(SkColorFilter::MakeComposeFilter(
            SkColorFilter::MakeModeFilter(color, SkBlendMode::kModulate),
            SkGaussianColorFilter::Make()));
        canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
    }
}
