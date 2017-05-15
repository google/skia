/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkPath.h"
#include "SkPM4f.h"
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
    void filterSpan4f(const SkPM4f src[], int count, SkPM4f result[]) const override;

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

static inline float eval_gaussian(float x) {
    // x = 1 - x;
    // return sk_float_exp(-x * x * 4) - 0.018f;

    return 0.00030726194381713867f +
             x*(0.15489584207534790039f +
               x*(0.21345567703247070312f +
                 (2.89795351028442382812f - 2.26661229133605957031f*x)*x));
}

static void build_table() {
    SkDebugf("const uint8_t gByteExpU8Table[256] = {");
    for (int i = 0; i <= 255; ++i) {
        if (!(i % 8)) {
            SkDebugf("\n");
        }
        int v = (int)(eval_gaussian(i / 255.f) * 256);
        SkDebugf(" 0x%02X,", v);
    }
    SkDebugf("\n};\n");
}

const uint8_t gByteExpU8Table[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07,
    0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D,
    0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x11,
    0x11, 0x12, 0x12, 0x13, 0x14, 0x14, 0x15, 0x15,
    0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1A, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1E, 0x1F, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x39,
    0x3A, 0x3B, 0x3C, 0x3D, 0x3F, 0x40, 0x41, 0x42,
    0x44, 0x45, 0x46, 0x48, 0x49, 0x4A, 0x4C, 0x4D,
    0x4E, 0x50, 0x51, 0x53, 0x54, 0x55, 0x57, 0x58,
    0x5A, 0x5B, 0x5D, 0x5E, 0x60, 0x61, 0x63, 0x64,
    0x66, 0x68, 0x69, 0x6B, 0x6C, 0x6E, 0x70, 0x71,
    0x73, 0x75, 0x76, 0x78, 0x79, 0x7B, 0x7D, 0x7F,
    0x80, 0x82, 0x84, 0x85, 0x87, 0x89, 0x8A, 0x8C,
    0x8E, 0x90, 0x91, 0x93, 0x95, 0x96, 0x98, 0x9A,
    0x9C, 0x9D, 0x9F, 0xA1, 0xA2, 0xA4, 0xA6, 0xA8,
    0xA9, 0xAB, 0xAD, 0xAE, 0xB0, 0xB2, 0xB3, 0xB5,
    0xB7, 0xB8, 0xBA, 0xBC, 0xBD, 0xBF, 0xC0, 0xC2,
    0xC3, 0xC5, 0xC7, 0xC8, 0xCA, 0xCB, 0xCD, 0xCE,
    0xCF, 0xD1, 0xD2, 0xD4, 0xD5, 0xD6, 0xD8, 0xD9,
    0xDA, 0xDC, 0xDD, 0xDE, 0xDF, 0xE1, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
    0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF0, 0xF1, 0xF2,
    0xF3, 0xF3, 0xF4, 0xF5, 0xF5, 0xF6, 0xF6, 0xF7,
    0xF7, 0xF8, 0xF8, 0xF9, 0xF9, 0xF9, 0xFA, 0xFA,
    0xFA, 0xFA, 0xFA, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,
};

void SkGaussianColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    // to re-build the table, call build_table() which will dump it out using SkDebugf.
    if (false) {
        build_table();
    }
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];
        uint8_t a = gByteExpU8Table[SkGetPackedA32(c)];
        dst[i] = SkPackARGB32(a, a, a, a);
    }
}

void SkGaussianColorFilter::filterSpan4f(const SkPM4f src[], int count, SkPM4f dst[]) const {
    for (int i = 0; i < count; ++i) {
        float v = eval_gaussian(src[i].a());
        dst[i] = SkPM4f::FromPremulRGBA(v, v, v, v);
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
    bool fTransparent;

    bool isCompatible(const AmbientVerticesFactory& that, SkVector* translate) const {
        if (fOccluderHeight != that.fOccluderHeight || fTransparent != that.fTransparent) {
            return false;
        }
        translate->set(0, 0);
        return true;
    }

    sk_sp<SkVertices> makeVertices(const SkPath& path, const SkMatrix& ctm) const {
        SkPoint3 zParams = SkPoint3::Make(0, 0, fOccluderHeight);
        return SkShadowTessellator::MakeAmbient(path, ctm, zParams, fTransparent);
    }
};

/** Factory for an spot shadow mesh with particular shadow properties. */
struct SpotVerticesFactory {
    enum class OccluderType {
        // The umbra cannot be dropped out because the occluder is not opaque.
        kTransparent,
        // The occluder is opaque and the umbra is fully visible
        kOpaqueFullUmbra,
        // The umbra can be dropped where it is occluded.
        kOpaquePartialUmbra,
        // It is known that the entire umbra is occluded.
        kOpaqueNoUmbra
    };

    SkVector fOffset;
    SkScalar fOccluderHeight = SK_ScalarNaN; // NaN so that isCompatible will fail until init'ed.
    SkPoint3 fDevLightPos;
    SkScalar fLightRadius;
    OccluderType fOccluderType;

    bool isCompatible(const SpotVerticesFactory& that, SkVector* translate) const {
        if (fOccluderHeight != that.fOccluderHeight || fDevLightPos.fZ != that.fDevLightPos.fZ ||
            fLightRadius != that.fLightRadius || fOccluderType != that.fOccluderType) {
            return false;
        }
        switch (fOccluderType) {
            case OccluderType::kTransparent:
            case OccluderType::kOpaqueFullUmbra:
            case OccluderType::kOpaqueNoUmbra:
                // 'this' and 'that' will either both have no umbra removed or both have all the
                // umbra removed.
                *translate = that.fOffset - fOffset;
                return true;
            case OccluderType::kOpaquePartialUmbra:
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
        SkPoint3 zParams = SkPoint3::Make(0, 0, fOccluderHeight);
        return SkShadowTessellator::MakeSpot(path, ctm, zParams,
                                             fDevLightPos, fLightRadius, transparent);
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
void draw_shadow(const FACTORY& factory, SkCanvas* canvas, ShadowedPath& path, SkColor color) {
    FindContext<FACTORY> context(&path.viewMatrix(), &factory);

    SkResourceCache::Key* key = nullptr;
    SkAutoSTArray<32 * 4, uint8_t> keyStorage;
    int keyDataBytes = path.keyBytes();
    if (keyDataBytes >= 0) {
        keyStorage.reset(keyDataBytes + sizeof(SkResourceCache::Key));
        key = new (keyStorage.begin()) SkResourceCache::Key();
        path.writeKey((uint32_t*)(keyStorage.begin() + sizeof(*key)));
        key->init(&kNamespace, resource_cache_shared_id(), keyDataBytes);
        SkResourceCache::Find(*key, FindVisitor<FACTORY>, &context);
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
            SkResourceCache::Add(rec);
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
    const SkMatrix& ctm = canvas->getTotalMatrix();
    if (ctm.rectStaysRect() && ctm.isSimilarity()) {
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

static SkColor compute_render_color(SkColor color, float alpha) {
    return SkColorSetARGB(alpha*SkColorGetA(color), SkColorGetR(color),
                          SkColorGetG(color), SkColorGetB(color));
}

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, const SkPoint3& zPlaneParams,
                               const SkPoint3& devLightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {
    // check z plane
    bool tiltZPlane = !SkScalarNearlyZero(zPlaneParams.fX) || !SkScalarNearlyZero(zPlaneParams.fY);

    // try fast paths
    bool skipAnalytic = SkToBool(flags & SkShadowFlags::kGeometricOnly_ShadowFlag) || tiltZPlane;
    if (!skipAnalytic && draw_analytic_shadows(canvas, path, zPlaneParams.fZ, devLightPos,
                                               lightRadius, ambientAlpha, spotAlpha, color,
                                               flags)) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    SkMatrix viewMatrix = canvas->getTotalMatrix();
    canvas->resetMatrix();

    ShadowedPath shadowedPath(&path, &viewMatrix);

    bool transparent = SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag);
    bool uncached = tiltZPlane || path.isVolatile();

    if (ambientAlpha > 0) {
        ambientAlpha = SkTMin(ambientAlpha, 1.f);
        if (uncached) {
            sk_sp<SkVertices> vertices = SkShadowTessellator::MakeAmbient(path, viewMatrix,
                                                                          zPlaneParams,
                                                                          transparent);
            SkColor renderColor = compute_render_color(color, ambientAlpha);
            SkPaint paint;
            // Run the vertex color through a GaussianColorFilter and then modulate the grayscale
            // result of that against our 'color' param.
            paint.setColorFilter(SkColorFilter::MakeComposeFilter(
                SkColorFilter::MakeModeFilter(renderColor, SkBlendMode::kModulate),
                SkGaussianColorFilter::Make()));
            canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
        } else {
            AmbientVerticesFactory factory;
            factory.fOccluderHeight = zPlaneParams.fZ;
            factory.fTransparent = transparent;

            SkColor renderColor = compute_render_color(color, ambientAlpha);
            draw_shadow(factory, canvas, shadowedPath, renderColor);
        }
    }

    if (spotAlpha > 0) {
        spotAlpha = SkTMin(spotAlpha, 1.f);
        if (uncached) {
            sk_sp<SkVertices> vertices = SkShadowTessellator::MakeSpot(path, viewMatrix,
                                                                       zPlaneParams,
                                                                       devLightPos, lightRadius,
                                                                       transparent);
            SkColor renderColor = compute_render_color(color, spotAlpha);
            SkPaint paint;
            // Run the vertex color through a GaussianColorFilter and then modulate the grayscale
            // result of that against our 'color' param.
            paint.setColorFilter(SkColorFilter::MakeComposeFilter(
                SkColorFilter::MakeModeFilter(renderColor, SkBlendMode::kModulate),
                SkGaussianColorFilter::Make()));
            canvas->drawVertices(vertices, SkBlendMode::kModulate, paint);
        } else {
            SpotVerticesFactory factory;
            SkScalar occluderHeight = zPlaneParams.fZ;
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
            SkRect devBounds;
            viewMatrix.mapRect(&devBounds, path.getBounds());
            if (transparent) {
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
            } else if (SkTAbs(factory.fOffset.fX) > 0.5f*devBounds.width() ||
                       SkTAbs(factory.fOffset.fY) > 0.5f*devBounds.height()) {
                // if the translation of the shadow is big enough we're going to end up
                // filling the entire umbra, so we can treat these as all the same
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaqueFullUmbra;
            } else if (factory.fOffset.length()*scale + scale < radius) {
                // if we don't translate more than the blur distance, can assume umbra is covered
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaqueNoUmbra;
            } else {
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaquePartialUmbra;
            }

#ifdef DEBUG_SHADOW_CHECKS
            switch (factory.fOccluderType) {
                case SpotVerticesFactory::OccluderType::kTransparent:
                    color = 0xFFD2B48C;  // tan for transparent
                    break;
                case SpotVerticesFactory::OccluderType::kOpaqueFullUmbra:
                    color = 0xFF614126;   // brown for umBra
                    break;
                case SpotVerticesFactory::OccluderType::kOpaquePartialUmbra:
                    color = 0xFFFFA500;   // orange for opaque
                    break;
                case SpotVerticesFactory::OccluderType::kOpaqueNoUmbra:
                    color = 0xFFE5E500;  // corn yellow for covered
                    break;
            }
#endif
            SkColor renderColor = compute_render_color(color, spotAlpha);
            draw_shadow(factory, canvas, shadowedPath, renderColor);
        }
    }
}
