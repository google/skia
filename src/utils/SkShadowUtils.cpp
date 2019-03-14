/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorData.h"
#include "SkDevice.h"
#include "SkDrawShadowInfo.h"
#include "SkEffectPriv.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRasterPipeline.h"
#include "SkResourceCache.h"
#include "SkShadowTessellator.h"
#include "SkString.h"
#include "SkTLazy.h"
#include "SkVertices.h"
#include <new>
#if SK_SUPPORT_GPU
#include "GrShape.h"
#include "effects/GrBlurredEdgeFragmentProcessor.h"
#endif

/**
*  Gaussian color filter -- produces a Gaussian ramp based on the color's B value,
*                           then blends with the color's G value.
*                           Final result is black with alpha of Gaussian(B)*G.
*                           The assumption is that the original color's alpha is 1.
*/
class SkGaussianColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make() {
        return sk_sp<SkColorFilter>(new SkGaussianColorFilter);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override {}
    void onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        rec.fPipeline->append(SkRasterPipeline::gauss_a_to_rgba);
    }
private:
    SK_FLATTENABLE_HOOKS(SkGaussianColorFilter)

    SkGaussianColorFilter() : INHERITED() {}

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkGaussianColorFilter::CreateProc(SkReadBuffer&) {
    return Make();
}

#if SK_SUPPORT_GPU

std::unique_ptr<GrFragmentProcessor> SkGaussianColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return GrBlurredEdgeFragmentProcessor::Make(GrBlurredEdgeFragmentProcessor::Mode::kGaussian);
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
    SkVector fOffset;

    bool isCompatible(const AmbientVerticesFactory& that, SkVector* translate) const {
        if (fOccluderHeight != that.fOccluderHeight || fTransparent != that.fTransparent) {
            return false;
        }
        *translate = that.fOffset;
        return true;
    }

    sk_sp<SkVertices> makeVertices(const SkPath& path, const SkMatrix& ctm,
                                   SkVector* translate) const {
        SkPoint3 zParams = SkPoint3::Make(0, 0, fOccluderHeight);
        // pick a canonical place to generate shadow
        SkMatrix noTrans(ctm);
        if (!ctm.hasPerspective()) {
            noTrans[SkMatrix::kMTransX] = 0;
            noTrans[SkMatrix::kMTransY] = 0;
        }
        *translate = fOffset;
        return SkShadowTessellator::MakeAmbient(path, noTrans, zParams, fTransparent);
    }
};

/** Factory for an spot shadow mesh with particular shadow properties. */
struct SpotVerticesFactory {
    enum class OccluderType {
        // The umbra cannot be dropped out because either the occluder is not opaque,
        // or the center of the umbra is visible.
        kTransparent,
        // The umbra can be dropped where it is occluded.
        kOpaquePartialUmbra,
        // It is known that the entire umbra is occluded.
        kOpaqueNoUmbra
    };

    SkVector fOffset;
    SkPoint  fLocalCenter;
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
            case OccluderType::kOpaqueNoUmbra:
                // 'this' and 'that' will either both have no umbra removed or both have all the
                // umbra removed.
                *translate = that.fOffset;
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
        SK_ABORT("Uninitialized occluder type?");
        return false;
    }

    sk_sp<SkVertices> makeVertices(const SkPath& path, const SkMatrix& ctm,
                                   SkVector* translate) const {
        bool transparent = OccluderType::kTransparent == fOccluderType;
        SkPoint3 zParams = SkPoint3::Make(0, 0, fOccluderHeight);
        if (ctm.hasPerspective() || OccluderType::kOpaquePartialUmbra == fOccluderType) {
            translate->set(0, 0);
            return SkShadowTessellator::MakeSpot(path, ctm, zParams,
                                                 fDevLightPos, fLightRadius, transparent);
        } else {
            // pick a canonical place to generate shadow, with light centered over path
            SkMatrix noTrans(ctm);
            noTrans[SkMatrix::kMTransX] = 0;
            noTrans[SkMatrix::kMTransY] = 0;
            SkPoint devCenter(fLocalCenter);
            noTrans.mapPoints(&devCenter, 1);
            SkPoint3 centerLightPos = SkPoint3::Make(devCenter.fX, devCenter.fY, fDevLightPos.fZ);
            *translate = fOffset;
            return SkShadowTessellator::MakeSpot(path, noTrans, zParams,
                                                 centerLightPos, fLightRadius, transparent);
        }
    }
};

/**
 * This manages a set of tessellations for a given shape in the cache. Because SkResourceCache
 * records are immutable this is not itself a Rec. When we need to update it we return this on
 * the FindVisitor and let the cache destroy the Rec. We'll update the tessellations and then add
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
                          const SkMatrix& matrix, SkVector* translate) {
        return fAmbientSet.add(devPath, ambient, matrix, translate);
    }

    sk_sp<SkVertices> find(const SpotVerticesFactory& spot, const SkMatrix& matrix,
                           SkVector* translate) const {
        return fSpotSet.find(spot, matrix, translate);
    }

    sk_sp<SkVertices> add(const SkPath& devPath, const SpotVerticesFactory& spot,
                          const SkMatrix& matrix, SkVector* translate) {
        return fSpotSet.add(devPath, spot, matrix, translate);
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
                    return fEntries[i].fVertices;
                }
            }
            return nullptr;
        }

        sk_sp<SkVertices> add(const SkPath& path, const FACTORY& factory, const SkMatrix& matrix,
                              SkVector* translate) {
            sk_sp<SkVertices> vertices = factory.makeVertices(path, matrix, translate);
            if (!vertices) {
                return nullptr;
            }
            int i;
            if (fCount < MAX_ENTRIES) {
                i = fCount++;
            } else {
                i = fRandom.nextULessThan(MAX_ENTRIES);
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
        SkRandom fRandom;
    };

    Set<AmbientVerticesFactory, 4> fAmbientSet;
    Set<SpotVerticesFactory, 4> fSpotSet;
};

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
    void writeKey(void* key) const { SK_ABORT("Should never be called"); }
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
bool draw_shadow(const FACTORY& factory,
                 std::function<void(const SkVertices*, SkBlendMode, const SkPaint&,
                 SkScalar tx, SkScalar ty)> drawProc, ShadowedPath& path, SkColor color) {
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
    bool foundInCache = SkToBool(context.fVertices);
    if (foundInCache) {
        vertices = std::move(context.fVertices);
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
            vertices = tessellations->add(path.path(), factory, path.viewMatrix(),
                                          &context.fTranslate);
            if (!vertices) {
                return false;
            }
            auto rec = new CachedTessellationsRec(*key, std::move(tessellations));
            SkResourceCache::Add(rec);
        } else {
            vertices = factory.makeVertices(path.path(), path.viewMatrix(),
                                            &context.fTranslate);
            if (!vertices) {
                return false;
            }
        }
    }

    SkPaint paint;
    // Run the vertex color through a GaussianColorFilter and then modulate the grayscale result of
    // that against our 'color' param.
    paint.setColorFilter(
         SkColorFilter::MakeModeFilter(color, SkBlendMode::kModulate)->makeComposed(
                                                                    SkGaussianColorFilter::Make()));

    drawProc(vertices.get(), SkBlendMode::kModulate, paint,
             context.fTranslate.fX, context.fTranslate.fY);

    return true;
}
}

static bool tilted(const SkPoint3& zPlaneParams) {
    return !SkScalarNearlyZero(zPlaneParams.fX) || !SkScalarNearlyZero(zPlaneParams.fY);
}

static SkPoint3 map(const SkMatrix& m, const SkPoint3& pt) {
    SkPoint3 result;
    m.mapXY(pt.fX, pt.fY, (SkPoint*)&result.fX);
    result.fZ = pt.fZ;
    return result;
}

void SkShadowUtils::ComputeTonalColors(SkColor inAmbientColor, SkColor inSpotColor,
                                       SkColor* outAmbientColor, SkColor* outSpotColor) {
    // For tonal color we only compute color values for the spot shadow.
    // The ambient shadow is greyscale only.

    // Ambient
    *outAmbientColor = SkColorSetARGB(SkColorGetA(inAmbientColor), 0, 0, 0);

    // Spot
    int spotR = SkColorGetR(inSpotColor);
    int spotG = SkColorGetG(inSpotColor);
    int spotB = SkColorGetB(inSpotColor);
    int max = SkTMax(SkTMax(spotR, spotG), spotB);
    int min = SkTMin(SkTMin(spotR, spotG), spotB);
    SkScalar luminance = 0.5f*(max + min)/255.f;
    SkScalar origA = SkColorGetA(inSpotColor)/255.f;

    // We compute a color alpha value based on the luminance of the color, scaled by an
    // adjusted alpha value. We want the following properties to match the UX examples
    // (assuming a = 0.25) and to ensure that we have reasonable results when the color
    // is black and/or the alpha is 0:
    //     f(0, a) = 0
    //     f(luminance, 0) = 0
    //     f(1, 0.25) = .5
    //     f(0.5, 0.25) = .4
    //     f(1, 1) = 1
    // The following functions match this as closely as possible.
    SkScalar alphaAdjust = (2.6f + (-2.66667f + 1.06667f*origA)*origA)*origA;
    SkScalar colorAlpha = (3.544762f + (-4.891428f + 2.3466f*luminance)*luminance)*luminance;
    colorAlpha = SkTPin(alphaAdjust*colorAlpha, 0.0f, 1.0f);

    // Similarly, we set the greyscale alpha based on luminance and alpha so that
    //     f(0, a) = a
    //     f(luminance, 0) = 0
    //     f(1, 0.25) = 0.15
    SkScalar greyscaleAlpha = SkTPin(origA*(1 - 0.4f*luminance), 0.0f, 1.0f);

    // The final color we want to emulate is generated by rendering a color shadow (C_rgb) using an
    // alpha computed from the color's luminance (C_a), and then a black shadow with alpha (S_a)
    // which is an adjusted value of 'a'.  Assuming SrcOver, a background color of B_rgb, and
    // ignoring edge falloff, this becomes
    //
    //      (C_a - S_a*C_a)*C_rgb + (1 - (S_a + C_a - S_a*C_a))*B_rgb
    //
    // Assuming premultiplied alpha, this means we scale the color by (C_a - S_a*C_a) and
    // set the alpha to (S_a + C_a - S_a*C_a).
    SkScalar colorScale = colorAlpha*(SK_Scalar1 - greyscaleAlpha);
    SkScalar tonalAlpha = colorScale + greyscaleAlpha;
    SkScalar unPremulScale = colorScale / tonalAlpha;
    *outSpotColor = SkColorSetARGB(tonalAlpha*255.999f,
                                   unPremulScale*spotR,
                                   unPremulScale*spotG,
                                   unPremulScale*spotB);
}

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, const SkPoint3& zPlaneParams,
                               const SkPoint3& devLightPos, SkScalar lightRadius,
                               SkColor ambientColor, SkColor spotColor,
                               uint32_t flags) {
    SkMatrix inverse;
    if (!canvas->getTotalMatrix().invert(&inverse)) {
        return;
    }
    SkPoint pt = inverse.mapXY(devLightPos.fX, devLightPos.fY);

    SkDrawShadowRec rec;
    rec.fZPlaneParams   = zPlaneParams;
    rec.fLightPos       = { pt.fX, pt.fY, devLightPos.fZ };
    rec.fLightRadius    = lightRadius;
    rec.fAmbientColor   = ambientColor;
    rec.fSpotColor      = spotColor;
    rec.fFlags          = flags;

    canvas->private_draw_shadow_rec(path, rec);
}

static bool validate_rec(const SkDrawShadowRec& rec) {
    return rec.fLightPos.isFinite() && rec.fZPlaneParams.isFinite() &&
           SkScalarIsFinite(rec.fLightRadius);
}

void SkBaseDevice::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {
    auto drawVertsProc = [this](const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint,
                                SkScalar tx, SkScalar ty) {
        if (vertices->vertexCount()) {
            SkAutoDeviceCTMRestore adr(this, SkMatrix::Concat(this->ctm(),
                                                              SkMatrix::MakeTrans(tx, ty)));
            this->drawVertices(vertices, nullptr, 0, mode, paint);
        }
    };

    if (!validate_rec(rec)) {
        return;
    }

    SkMatrix viewMatrix = this->ctm();
    SkAutoDeviceCTMRestore adr(this, SkMatrix::I());

    ShadowedPath shadowedPath(&path, &viewMatrix);

    bool tiltZPlane = tilted(rec.fZPlaneParams);
    bool transparent = SkToBool(rec.fFlags & SkShadowFlags::kTransparentOccluder_ShadowFlag);
    bool uncached = tiltZPlane || path.isVolatile();

    SkPoint3 zPlaneParams = rec.fZPlaneParams;
    SkPoint3 devLightPos = map(viewMatrix, rec.fLightPos);
    float lightRadius = rec.fLightRadius;

    if (SkColorGetA(rec.fAmbientColor) > 0) {
        bool success = false;
        if (uncached) {
            sk_sp<SkVertices> vertices = SkShadowTessellator::MakeAmbient(path, viewMatrix,
                                                                          zPlaneParams,
                                                                          transparent);
            if (vertices) {
                SkPaint paint;
                // Run the vertex color through a GaussianColorFilter and then modulate the
                // grayscale result of that against our 'color' param.
                paint.setColorFilter(
                    SkColorFilter::MakeModeFilter(rec.fAmbientColor,
                                                  SkBlendMode::kModulate)->makeComposed(
                                                                   SkGaussianColorFilter::Make()));
                this->drawVertices(vertices.get(), nullptr, 0, SkBlendMode::kModulate, paint);
                success = true;
            }
        }

        if (!success) {
            AmbientVerticesFactory factory;
            factory.fOccluderHeight = zPlaneParams.fZ;
            factory.fTransparent = transparent;
            if (viewMatrix.hasPerspective()) {
                factory.fOffset.set(0, 0);
            } else {
                factory.fOffset.fX = viewMatrix.getTranslateX();
                factory.fOffset.fY = viewMatrix.getTranslateY();
            }

            if (!draw_shadow(factory, drawVertsProc, shadowedPath, rec.fAmbientColor)) {
                // Pretransform the path to avoid transforming the stroke, below.
                SkPath devSpacePath;
                path.transform(viewMatrix, &devSpacePath);

                // The tesselator outsets by AmbientBlurRadius (or 'r') to get the outer ring of
                // the tesselation, and sets the alpha on the path to 1/AmbientRecipAlpha (or 'a').
                //
                // We want to emulate this with a blur. The full blur width (2*blurRadius or 'f')
                // can be calculated by interpolating:
                //
                //            original edge        outer edge
                //         |       |<---------- r ------>|
                //         |<------|--- f -------------->|
                //         |       |                     |
                //    alpha = 1  alpha = a          alpha = 0
                //
                // Taking ratios, f/1 = r/a, so f = r/a and blurRadius = f/2.
                //
                // We now need to outset the path to place the new edge in the center of the
                // blur region:
                //
                //             original   new
                //         |       |<------|--- r ------>|
                //         |<------|--- f -|------------>|
                //         |       |<- o ->|<--- f/2 --->|
                //
                //     r = o + f/2, so o = r - f/2
                //
                // We outset by using the stroker, so the strokeWidth is o/2.
                //
                SkScalar devSpaceOutset = SkDrawShadowMetrics::AmbientBlurRadius(zPlaneParams.fZ);
                SkScalar oneOverA = SkDrawShadowMetrics::AmbientRecipAlpha(zPlaneParams.fZ);
                SkScalar blurRadius = 0.5f*devSpaceOutset*oneOverA;
                SkScalar strokeWidth = 0.5f*(devSpaceOutset - blurRadius);

                // Now draw with blur
                SkPaint paint;
                paint.setColor(rec.fAmbientColor);
                paint.setStrokeWidth(strokeWidth);
                paint.setStyle(SkPaint::kStrokeAndFill_Style);
                SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(blurRadius);
                bool respectCTM = false;
                paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, respectCTM));
                this->drawPath(devSpacePath, paint);
            }
        }
    }

    if (SkColorGetA(rec.fSpotColor) > 0) {
        bool success = false;
        if (uncached) {
            sk_sp<SkVertices> vertices = SkShadowTessellator::MakeSpot(path, viewMatrix,
                                                                       zPlaneParams,
                                                                       devLightPos, lightRadius,
                                                                       transparent);
            if (vertices) {
                SkPaint paint;
                // Run the vertex color through a GaussianColorFilter and then modulate the
                // grayscale result of that against our 'color' param.
                paint.setColorFilter(
                    SkColorFilter::MakeModeFilter(rec.fSpotColor,
                                                  SkBlendMode::kModulate)->makeComposed(
                                                      SkGaussianColorFilter::Make()));
                this->drawVertices(vertices.get(), nullptr, 0, SkBlendMode::kModulate, paint);
                success = true;
            }
        }

        if (!success) {
            SpotVerticesFactory factory;
            factory.fOccluderHeight = zPlaneParams.fZ;
            factory.fDevLightPos = devLightPos;
            factory.fLightRadius = lightRadius;

            SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
            factory.fLocalCenter = center;
            viewMatrix.mapPoints(&center, 1);
            SkScalar radius, scale;
            SkDrawShadowMetrics::GetSpotParams(zPlaneParams.fZ, devLightPos.fX - center.fX,
                                               devLightPos.fY - center.fY, devLightPos.fZ,
                                               lightRadius, &radius, &scale, &factory.fOffset);
            SkRect devBounds;
            viewMatrix.mapRect(&devBounds, path.getBounds());
            if (transparent ||
                SkTAbs(factory.fOffset.fX) > 0.5f*devBounds.width() ||
                SkTAbs(factory.fOffset.fY) > 0.5f*devBounds.height()) {
                // if the translation of the shadow is big enough we're going to end up
                // filling the entire umbra, so we can treat these as all the same
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
            } else if (factory.fOffset.length()*scale + scale < radius) {
                // if we don't translate more than the blur distance, can assume umbra is covered
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaqueNoUmbra;
            } else if (path.isConvex()) {
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kOpaquePartialUmbra;
            } else {
                factory.fOccluderType = SpotVerticesFactory::OccluderType::kTransparent;
            }
            // need to add this after we classify the shadow
            factory.fOffset.fX += viewMatrix.getTranslateX();
            factory.fOffset.fY += viewMatrix.getTranslateY();

            SkColor color = rec.fSpotColor;
#ifdef DEBUG_SHADOW_CHECKS
            switch (factory.fOccluderType) {
                case SpotVerticesFactory::OccluderType::kTransparent:
                    color = 0xFFD2B48C;  // tan for transparent
                    break;
                case SpotVerticesFactory::OccluderType::kOpaquePartialUmbra:
                    color = 0xFFFFA500;   // orange for opaque
                    break;
                case SpotVerticesFactory::OccluderType::kOpaqueNoUmbra:
                    color = 0xFFE5E500;  // corn yellow for covered
                    break;
            }
#endif
            if (!draw_shadow(factory, drawVertsProc, shadowedPath, color)) {
                // draw with blur
                SkMatrix shadowMatrix;
                if (!SkDrawShadowMetrics::GetSpotShadowTransform(devLightPos, lightRadius,
                                                                 viewMatrix, zPlaneParams,
                                                                 path.getBounds(),
                                                                 &shadowMatrix, &radius)) {
                    return;
                }
                SkAutoDeviceCTMRestore adr(this, shadowMatrix);

                SkPaint paint;
                paint.setColor(rec.fSpotColor);
                SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(radius);
                bool respectCTM = false;
                paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, respectCTM));
                this->drawPath(path, paint);
            }
        }
    }
}
