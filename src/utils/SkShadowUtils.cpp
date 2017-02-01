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
#include "SkResourceCache.h"
#include "SkShadowTessellator.h"
#include "SkTLazy.h"
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

        dst[i] = SkPackARGB32(factor*SkGetPackedG32(c), 0, 0, 0);
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

struct AmbientVerticesFactory {
    SkScalar fRadius;
    SkColor fUmbraColor;
    SkColor fPenumbraColor;
    bool fTransparent;

    bool operator==(const AmbientVerticesFactory& that) const {
        return fRadius == that.fRadius && fUmbraColor == that.fUmbraColor &&
               fPenumbraColor == that.fPenumbraColor && fTransparent == that.fTransparent;
    }
    bool operator!=(const AmbientVerticesFactory& that) const { return !(*this == that); }

    sk_sp<SkShadowVertices> makeVertices(const SkPath& devPath) const {
        return SkShadowVertices::MakeAmbient(devPath, fRadius, fUmbraColor, fPenumbraColor,
                                             fTransparent);
    }
};

struct SpotVerticesFactory {
    SkScalar fRadius;
    SkColor fUmbraColor;
    SkColor fPenumbraColor;
    SkScalar fScale;
    SkVector fOffset;
    bool fTransparent;

    bool operator==(const SpotVerticesFactory& that) const {
        return fRadius == that.fRadius && fUmbraColor == that.fUmbraColor &&
               fPenumbraColor == that.fPenumbraColor && fTransparent == that.fTransparent &&
               fScale == that.fScale && fOffset == that.fOffset;
    }
    bool operator!=(const SpotVerticesFactory& that) const { return !(*this == that); }

    sk_sp<SkShadowVertices> makeVertices(const SkPath& devPath) const {
        return SkShadowVertices::MakeSpot(devPath, fScale, fOffset, fRadius, fUmbraColor,
                                          fPenumbraColor, fTransparent);
    }
};

/**
 * A record of shadow vertices stored in SkResourceCache. Each shape may have one record for a given
 * FACTORY type.
 */
template <typename FACTORY>
class TessPathRec : public SkResourceCache::Rec {
public:
    TessPathRec(const SkResourceCache::Key& key, const SkMatrix& viewMatrix, const FACTORY& factory,
                sk_sp<SkShadowVertices> vertices)
            : fVertices(std::move(vertices)), fFactory(factory), fOriginalMatrix(viewMatrix) {
        fKey.reset(new uint8_t[key.size()]);
        memcpy(fKey.get(), &key, key.size());
    }

    const Key& getKey() const override {
        return *reinterpret_cast<SkResourceCache::Key*>(fKey.get());
    }
    size_t bytesUsed() const override { return fVertices->size(); }

    const char* getCategory() const override { return "tessellated shadow mask"; }

    sk_sp<SkShadowVertices> refVertices() const { return fVertices; }

    const FACTORY& factory() const { return fFactory; }

    const SkMatrix& originalViewMatrix() const { return fOriginalMatrix; }

private:
    std::unique_ptr<uint8_t[]> fKey;
    sk_sp<SkShadowVertices> fVertices;
    FACTORY fFactory;
    SkMatrix fOriginalMatrix;
};

/**
 * Used by FindVisitor to determine whether a cache entry can be reused and if so returns the
 * vertices and translation vector.
 */
template <typename FACTORY>
struct FindContext {
    FindContext(const SkMatrix* viewMatrix, const FACTORY* factory)
            : fViewMatrix(viewMatrix), fFactory(factory) {}
    const SkMatrix* fViewMatrix;
    SkVector fTranslate = {0, 0};
    sk_sp<SkShadowVertices> fVertices;
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
    const TessPathRec<FACTORY>& rec = static_cast<const TessPathRec<FACTORY>&>(baseRec);

    const SkMatrix& viewMatrix = *findContext->fViewMatrix;
    const SkMatrix& recMatrix = rec.originalViewMatrix();
    if (findContext->fViewMatrix->hasPerspective() || recMatrix.hasPerspective()) {
        if (recMatrix != viewMatrix) {
            return false;
        }
    } else if (recMatrix.getScaleX() != viewMatrix.getScaleX() ||
               recMatrix.getSkewX() != viewMatrix.getSkewX() ||
               recMatrix.getScaleY() != viewMatrix.getScaleY() ||
               recMatrix.getSkewY() != viewMatrix.getSkewY()) {
        return false;
    }
    if (*findContext->fFactory != rec.factory()) {
        return false;
    }
    findContext->fTranslate.fX = viewMatrix.getTranslateX() - recMatrix.getTranslateX();
    findContext->fTranslate.fY = viewMatrix.getTranslateY() - recMatrix.getTranslateY();
    findContext->fVertices = rec.refVertices();
    return true;
}

class ShadowedPath {
public:
    ShadowedPath(const SkPath* path, const SkMatrix* viewMatrix)
            : fOriginalPath(path)
            , fViewMatrix(viewMatrix)
#if SK_SUPPORT_GPU
            , fShapeForKey(*path, GrStyle::SimpleFill())
#endif
    {}

    const SkPath& transformedPath() {
        if (!fTransformedPath.isValid()) {
            fOriginalPath->transform(*fViewMatrix, fTransformedPath.init());
        }
        return *fTransformedPath.get();
    }

    const SkMatrix& viewMatrix() const { return *fViewMatrix; }
#if SK_SUPPORT_GPU
    /** Negative means the vertices should not be cached for this path. */
    int keyBytes() const { return fShapeForKey.unstyledKeySize() * sizeof(uint32_t); }
    void writeKey(void* key) const {
        fShapeForKey.writeUnstyledKey(reinterpret_cast<uint32_t*>(key));
    }
#else
    int keyBytes() const { return -1; }
    void writeKey(void* key) const { SkFAIL("Should never be called"); }
#endif

private:
    const SkPath* fOriginalPath;
    const SkMatrix* fViewMatrix;
#if SK_SUPPORT_GPU
    GrShape fShapeForKey;
#endif
    SkTLazy<SkPath> fTransformedPath;
};

/**
 * Draws a shadow to 'canvas'. The vertices used to draw the shadow are created by 'factory' unless
 * they are first found in SkResourceCache.
 */
template <typename FACTORY>
void draw_shadow(const FACTORY& factory, SkCanvas* canvas, ShadowedPath& path, SkColor color) {
    FindContext<FACTORY> context(&path.viewMatrix(), &factory);
    static void* kNamespace;

    SkResourceCache::Key* key = nullptr;
    SkAutoSTArray<32 * 4, uint8_t> keyStorage;
    int keyDataBytes = path.keyBytes();
    if (keyDataBytes >= 0) {
        keyStorage.reset(keyDataBytes + sizeof(SkResourceCache::Key));
        key = new (keyStorage.begin()) SkResourceCache::Key();
        path.writeKey((uint32_t*)(keyStorage.begin() + sizeof(*key)));
        key->init(&kNamespace, 0, keyDataBytes);
        SkResourceCache::Find(*key, FindVisitor<FACTORY>, &context);
    }

    sk_sp<SkShadowVertices> vertices;
    const SkVector* translate;
    static constexpr SkVector kZeroTranslate = {0, 0};
    bool foundInCache = SkToBool(context.fVertices);
    if (foundInCache) {
        vertices = std::move(context.fVertices);
        translate = &context.fTranslate;
    } else {
        // TODO: handle transforming the path as part of the tessellator
        vertices = factory.makeVertices(path.transformedPath());
        translate = &kZeroTranslate;
    }

    SkPaint paint;
    paint.setColor(color);
    paint.setColorFilter(SkGaussianColorFilter::Make());
    if (translate->fX || translate->fY) {
        canvas->save();
        canvas->translate(translate->fX, translate->fY);
    }
    canvas->drawVertices(SkCanvas::kTriangles_VertexMode, vertices->vertexCount(),
                         vertices->positions(), nullptr, vertices->colors(), vertices->indices(),
                         vertices->indexCount(), paint);
    if (translate->fX || translate->fY) {
        canvas->restore();
    }
    if (!foundInCache && key) {
        SkResourceCache::Add(
                new TessPathRec<FACTORY>(*key, path.viewMatrix(), factory, std::move(vertices)));
    }
}
}

static const float kHeightFactor = 1.0f / 128.0f;
static const float kGeomFactor = 64.0f;

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& lightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {
    SkMatrix viewMatrix = canvas->getTotalMatrix();

    canvas->save();
    canvas->resetMatrix();

    ShadowedPath shadowedPath(&path, &viewMatrix);

    bool transparent = SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (ambientAlpha > 0) {
        AmbientVerticesFactory factory;
        factory.fRadius = occluderHeight * kHeightFactor * kGeomFactor;
        SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(occluderHeight*kHeightFactor, 0.0f)));
        // umbraColor is the interior value, penumbraColor the exterior value.
        // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
        // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
        // the final alpha.
        factory.fUmbraColor =
                SkColorSetARGB(255, 0, ambientAlpha * 255.9999f, umbraAlpha * 255.9999f);
        factory.fPenumbraColor = SkColorSetARGB(255, 0, ambientAlpha * 255.9999f, 0);
        factory.fTransparent = transparent;

        draw_shadow(factory, canvas, shadowedPath, color);
    }

    if (spotAlpha > 0) {
        SpotVerticesFactory factory;
        float zRatio = SkTPin(occluderHeight / (lightPos.fZ - occluderHeight), 0.0f, 0.95f);
        factory.fRadius = lightRadius * zRatio;

        // Compute the scale and translation for the spot shadow.
        factory.fScale = lightPos.fZ / (lightPos.fZ - occluderHeight);

        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        factory.fOffset = SkVector::Make(zRatio * (center.fX - lightPos.fX),
                                         zRatio * (center.fY - lightPos.fY));
        factory.fUmbraColor = SkColorSetARGB(255, 0, spotAlpha * 255.9999f, 255);
        factory.fPenumbraColor = SkColorSetARGB(255, 0, spotAlpha * 255.9999f, 0);
        factory.fTransparent = transparent;

        draw_shadow(factory, canvas, shadowedPath, color);
    }

    canvas->restore();
}
