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

void SkGaussianColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];

        SkScalar factor = SK_Scalar1 - SkGetPackedB32(c) / 255.f;
        factor = SkScalarExp(-factor * factor * 4) - 0.018f;

        SkScalar a = factor * SkGetPackedG32(c);
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

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& devLightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags, SkResourceCache* cache) {
    SkAutoCanvasRestore acr(canvas, true);
    SkMatrix viewMatrix = canvas->getTotalMatrix();

    // try circular fast path
    SkRect rect;
    if (viewMatrix.isSimilarity() &&
        path.isOval(&rect) && rect.width() == rect.height()) {
        SkPaint newPaint;
        newPaint.setColor(color);
        if (ambientAlpha > 0) {
            newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderHeight, ambientAlpha,
                                                                   flags));
            canvas->drawPath(path, newPaint);
        }
        if (spotAlpha > 0) {
            newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderHeight, devLightPos,
                                                                lightRadius, spotAlpha, flags));
            canvas->drawPath(path, newPaint);
        }
        return;
    }

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
