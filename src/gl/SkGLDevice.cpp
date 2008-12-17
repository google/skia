#include "SkGLDevice.h"
#include "SkGL.h"
#include "SkDrawProcs.h"
#include "SkRegion.h"
#include "SkThread.h"

static void TRACE_DRAW(const char func[], SkGLDevice* device,
                       const SkDraw& draw) {
    //    SkDebugf("--- <%s> %p %p\n", func, canvas, draw.fDevice);
}

struct SkGLDrawProcs : public SkDrawProcs {
public:
    void init(const SkRegion* clip, int height) {
        fCurrQuad = 0;
        fCurrTexture = 0;
        fClip = clip;
        fViewportHeight = height;

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, SK_TextGLType, 0, fTexs);
        glDisableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2, SK_TextGLType, 0, fVerts);
    }

    GLenum texture() const { return fCurrTexture; }

    void flush() {
        if (fCurrQuad && fCurrTexture) {
            this->drawQuads();
        }
        fCurrQuad = 0;
    }

    void addQuad(GLuint texture, int x, int y, const SkGlyph& glyph,
                 SkFixed left, SkFixed right, SkFixed bottom) {
        SkASSERT((size_t)fCurrQuad <= SK_ARRAY_COUNT(fVerts));
        
        if (fCurrTexture != texture || fCurrQuad == SK_ARRAY_COUNT(fVerts)) {
            if (fCurrQuad && fCurrTexture) {
                this->drawQuads();
            }
            fCurrQuad = 0;
            fCurrTexture = texture;
        }
        
        fVerts[fCurrQuad].setIRectFan(x, y,
                                      x + glyph.fWidth, y + glyph.fHeight);
        fTexs[fCurrQuad].setXRectFan(left, 0, right, bottom);
        fCurrQuad += 4;
    }
    
    void drawQuads();

private:
    enum {
        MAX_QUADS = 32
    };
    
    SkGLTextVertex fVerts[MAX_QUADS * 4];
    SkGLTextVertex fTexs[MAX_QUADS * 4];
    
    // these are initialized in setupForText
    GLuint          fCurrTexture;    
    int             fCurrQuad;
    int             fViewportHeight;
    const SkRegion* fClip;
};

///////////////////////////////////////////////////////////////////////////////

SkGLDevice::SkGLDevice(const SkBitmap& bitmap, bool offscreen)
        : SkDevice(bitmap), fClipIter(bitmap.height()) {
    fDrawProcs = NULL;
}

SkGLDevice::~SkGLDevice() {
    if (fDrawProcs) {
        SkDELETE(fDrawProcs);
    }
}

void SkGLDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& clip) {
    this->INHERITED::setMatrixClip(matrix, clip);
    
    fGLMatrix.set(matrix);
    fMatrix = matrix;
    fClip = clip;
    fDirty = true;
}

SkGLDevice::TexOrientation SkGLDevice::bindDeviceAsTexture() {
    return kNo_TexOrientation;
}

void SkGLDevice::gainFocus(SkCanvas* canvas) {
    this->INHERITED::gainFocus(canvas);

    const int w = this->width();
    const int h = this->height();
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SkGL::Ortho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    fDirty = true;    
}

SkGLClipIter* SkGLDevice::updateMatrixClip() {
    bool useIter = false;

    // first handle the clip
    if (fDirty || !fClip.isRect()) {
        fClipIter.reset(fClip);
        useIter = true;
    } else if (fDirty) {
        // no iter means caller is not respecting complex clips :(
        SkGL::Scissor(fClip.getBounds(), this->height());
    }
    // else we're just a rect, and we've already call scissor

    // now handle the matrix
    if (fDirty) {
        MAKE_GL(glLoadMatrix)(fGLMatrix.fMat);
#if 0
        SkDebugf("--- gldevice update matrix %p %p\n", this, fFBO);
        for (int y = 0; y < 4; y++) {
            SkDebugf(" [ ");
            for (int x = 0; x < 4; x++) {
                SkDebugf("%g ", fGLMatrix.fMat[y*4 + x]);
            }
            SkDebugf("]\n");
        }
#endif
        fDirty = false;
    }

    return useIter ? &fClipIter : NULL;
}

///////////////////////////////////////////////////////////////////////////////

// must be in the same order as SkXfermode::Coeff in SkXfermode.h
SkGLDevice::AutoPaintShader::AutoPaintShader(SkGLDevice* device,
                                             const SkPaint& paint) {
    fDevice = device;
    fTexCache = device->setupGLPaintShader(paint);
}

SkGLDevice::AutoPaintShader::~AutoPaintShader() {
    if (fTexCache) {
        SkGLDevice::UnlockTexCache(fTexCache);
    }
}

SkGLDevice::TexCache* SkGLDevice::setupGLPaintShader(const SkPaint& paint) {
    SkGL::SetPaint(paint);
    
    SkShader* shader = paint.getShader();
    if (NULL == shader) {
        return NULL;
    }
    
    if (!shader->setContext(this->accessBitmap(false), paint, this->matrix())) {
        return NULL;
    }
    
    SkBitmap bitmap;
    SkMatrix matrix;
    SkShader::TileMode tileModes[2];
    if (!shader->asABitmap(&bitmap, &matrix, tileModes)) {
        return NULL;
    }
    
    bitmap.lockPixels();
    if (!bitmap.readyToDraw()) {
        return NULL;
    }
    
    // see if we've already cached the bitmap from the shader
    SkPoint max;
    GLuint name;
    TexCache* cache = SkGLDevice::LockTexCache(bitmap, &name, &max);
    // the lock has already called glBindTexture for us
    SkGL::SetTexParams(paint.isFilterBitmap(), tileModes[0], tileModes[1]);
    
    // since our texture coords will be in local space, we wack the texture
    // matrix to map them back into 0...1 before we load it
    SkMatrix localM;
    if (shader->getLocalMatrix(&localM)) {
        SkMatrix inverse;
        if (localM.invert(&inverse)) {
            matrix.preConcat(inverse);
        }
    }
    
    matrix.postScale(max.fX / bitmap.width(), max.fY / bitmap.height());
    glMatrixMode(GL_TEXTURE);
    SkGL::LoadMatrix(matrix);
    glMatrixMode(GL_MODELVIEW);
    
    // since we're going to use a shader/texture, we don't want the color,
    // just its alpha
    SkGL::SetAlpha(paint.getAlpha());
    // report that we have setup the texture
    return cache;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkGLDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    TRACE_DRAW("coreDrawPaint", this, draw);
    
    AutoPaintShader   shader(this, paint);
    SkGLVertex        vertex[4];
    const SkGLVertex* texs = shader.useTex() ? vertex : NULL;
    
    // set vert to be big enough to fill the space, but not super-huge, to we
    // don't overflow fixed-point implementations
    {
        SkRect r;
        r.set(this->clip().getBounds());
        SkMatrix inverse;
        if (draw.fMatrix->invert(&inverse)) {
            inverse.mapRect(&r);
        }
        vertex->setRectFan(r);
    }
    
    SkGL::DrawVertices(4, GL_TRIANGLE_FAN, vertex, texs, NULL, NULL,
                       this->updateMatrixClip());
}

static const GLenum gPointMode2GL[] = {
    GL_POINTS,
    GL_LINES,
    GL_LINE_STRIP
};

void SkGLDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode,
                            size_t count, const SkPoint pts[], const SkPaint& paint) {
    TRACE_DRAW("coreDrawPoints", this, draw);
    
    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }
    
    /*  We should really only use drawverts for hairlines, since gl and skia
     treat the thickness differently...
     */
    
    AutoPaintShader shader(this, paint);
    
    if (width <= 0) {
        width = SK_Scalar1;
    }
    
    if (SkCanvas::kPoints_PointMode == mode) {
        glPointSize(SkScalarToFloat(width));
    } else {
        glLineWidth(SkScalarToFloat(width));
    }
    
    const SkGLVertex* verts;
    
#if GLSCALAR_IS_SCALAR
    verts = (const SkGLVertex*)pts;
#else
    SkAutoSTMalloc<32, SkGLVertex> storage(count);
    SkGLVertex* v = storage.get();
    
    v->setPoints(pts, count);
    verts = v;
#endif
    
    const SkGLVertex* texs = shader.useTex() ? verts : NULL;
    
    SkGL::DrawVertices(count, gPointMode2GL[mode], verts, texs, NULL, NULL,
                       this->updateMatrixClip());
}

void SkGLDevice::drawRect(const SkDraw& draw, const SkRect& rect,
                          const SkPaint& paint) {
    TRACE_DRAW("coreDrawRect", this, draw);
    
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        return;
    }
    
    if (paint.getStrokeJoin() != SkPaint::kMiter_Join) {
        SkPath  path;
        path.addRect(rect);
        this->drawPath(draw, path, paint);
        return;
    }
    
    AutoPaintShader shader(this, paint);
    
    SkGLVertex vertex[4];
    vertex->setRectFan(rect);
    const SkGLVertex* texs = shader.useTex() ? vertex : NULL;
    
    SkGL::DrawVertices(4, GL_TRIANGLE_FAN, vertex, texs, NULL, NULL,
                       this->updateMatrixClip());
}

void SkGLDevice::drawPath(const SkDraw& draw, const SkPath& path,
                          const SkPaint& paint) {
    TRACE_DRAW("coreDrawPath", this, draw);
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        return;
    }
    
    AutoPaintShader shader(this, paint);
    
    SkGL::FillPath(path, paint, shader.useTex(), this->updateMatrixClip());
}

void SkGLDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                            const SkMatrix& m, const SkPaint& paint) {
    TRACE_DRAW("coreDrawBitmap", this, draw);
    
    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        return;
    }
    
    SkGLClipIter* iter = this->updateMatrixClip();
    
    SkPoint max;
    GLenum name;
    SkAutoLockTexCache(bitmap, &name, &max);
    // the lock has already called glBindTexture for us
    SkGL::SetTexParamsClamp(paint.isFilterBitmap());
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    SkGL::MultMatrix(m);
    
    SkGLVertex  pts[4], tex[4];
    
    pts->setIRectFan(0, 0, bitmap.width(), bitmap.height());
    tex->setRectFan(0, 0, max.fX, max.fY);
    
    // now draw the mesh
    SkGL::SetPaintAlpha(paint);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    SkGL::DrawVertices(4, GL_TRIANGLE_FAN, pts, tex, NULL, NULL, iter);
    
    glPopMatrix();    
}

// move this guy into SkGL, so we can call it from SkGLDevice
static void gl_drawSprite(int x, int y, int w, int h, const SkPoint& max,
                          const SkPaint& paint, SkGLClipIter* iter) {
    SkGL::SetTexParamsClamp(false);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    SkGLVertex  pts[4], tex[4];
    
    // if h < 0, then the texture is bottom-to-top, but since our projection
    // matrix always inverts Y, we have to re-invert our texture coord here
    if (h < 0) {
        h = -h;
        tex->setRectFan(0, max.fY, max.fX, 0);
    } else {
        tex->setRectFan(0, 0, max.fX, max.fY);
    }
    pts->setIRectFan(x, y, x + w, y + h);
    
    SkGL::SetPaintAlpha(paint);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    // should look to use glDrawTexi() has we do for text...
    SkGL::DrawVertices(4, GL_TRIANGLE_FAN, pts, tex, NULL, NULL, iter);
    
    glPopMatrix();
}

void SkGLDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                            int left, int top, const SkPaint& paint) {
    TRACE_DRAW("coreDrawSprite", this, draw);
    
    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        return;
    }
    
    SkGLClipIter* iter = this->updateMatrixClip();
    
    SkPoint max;
    GLuint name;
    SkAutoLockTexCache(bitmap, &name, &max);    
    
    gl_drawSprite(left, top, bitmap.width(), bitmap.height(), max, paint, iter);
}

void SkGLDevice::drawDevice(const SkDraw& draw, SkDevice* dev,
                            int x, int y, const SkPaint& paint) {
    TRACE_DRAW("coreDrawDevice", this, draw);
    
    SkGLDevice::TexOrientation to = ((SkGLDevice*)dev)->bindDeviceAsTexture();
    if (SkGLDevice::kNo_TexOrientation != to) {
        SkGLClipIter* iter = this->updateMatrixClip();
        
        const SkBitmap& bm = dev->accessBitmap(false);
        int w = bm.width();
        int h = bm.height();
        SkPoint max;
        
        max.set(SkFixedToScalar(w << (16 - SkNextLog2(bm.rowBytesAsPixels()))),
                SkFixedToScalar(h << (16 - SkNextLog2(h))));
        
        if (SkGLDevice::kBottomToTop_TexOrientation == to) {
            h = -h;
        }
        gl_drawSprite(x, y, w, h, max, paint, iter);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const GLenum gVertexModeToGL[] = {
    GL_TRIANGLES,       // kTriangles_VertexMode,
    GL_TRIANGLE_STRIP,  // kTriangleStrip_VertexMode,
    GL_TRIANGLE_FAN     // kTriangleFan_VertexMode
};

#include "SkShader.h"

void SkGLDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {

    if (false) {
        SkRect bounds;
        SkIRect ibounds;
        
        bounds.set(vertices, vertexCount);
        bounds.round(&ibounds);
        
        SkDebugf("---- drawverts: %d pts, texs=%d colors=%d indices=%d bounds [%d %d]\n",
                 vertexCount, texs!=0, colors!=0, indexCount, ibounds.width(), ibounds.height());
    }
    
    SkGLClipIter* iter = this->updateMatrixClip();
    
    SkGL::SetPaint(paint);
    
    const SkGLVertex* glVerts;
    const SkGLVertex* glTexs = NULL;
    
#if GLSCALAR_IS_SCALAR
    glVerts = (const SkGLVertex*)vertices;
#else
    SkAutoSTMalloc<32, SkGLVertex> storage(vertexCount);
    storage.get()->setPoints(vertices, vertexCount);
    glVerts = storage.get();
#endif
    
    uint8_t* colorArray = NULL;
    if (colors) {
        colorArray = (uint8_t*)sk_malloc_throw(vertexCount*4);
        SkGL::SetRGBA(colorArray, colors, vertexCount);
    }
    SkAutoFree afca(colorArray);
    
    SkGLVertex* texArray = NULL;
    TexCache* cache = NULL;

    if (texs && paint.getShader()) {
        SkShader* shader = paint.getShader();
        
        //        if (!shader->setContext(this->accessBitmap(), paint, *draw.fMatrix)) {
        if (!shader->setContext(*draw.fBitmap, paint, *draw.fMatrix)) {
            goto DONE;
        }
        
        SkBitmap bitmap;
        SkMatrix matrix;
        SkShader::TileMode tileModes[2];
        if (shader->asABitmap(&bitmap, &matrix, tileModes)) {
            SkPoint max;
            GLuint name;
            cache = SkGLDevice::LockTexCache(bitmap, &name, &max);
            if (NULL == cache) {
                return;
            }

            matrix.postScale(max.fX / bitmap.width(), max.fY / bitmap.height());
            glMatrixMode(GL_TEXTURE);
            SkGL::LoadMatrix(matrix);
            glMatrixMode(GL_MODELVIEW);
            
#if GLSCALAR_IS_SCALAR
            glTexs = (const SkGLVertex*)texs;
#else
            texArray = (SkGLVertex*)sk_malloc_throw(vertexCount * sizeof(SkGLVertex));
            texArray->setPoints(texs, vertexCount);
            glTexs = texArray;
#endif
            
            SkGL::SetPaintAlpha(paint);
            SkGL::SetTexParams(paint.isFilterBitmap(),
                               tileModes[0], tileModes[1]);
        }
    }
DONE:
    SkAutoFree aftex(texArray);
    
    SkGL::DrawVertices(indices ? indexCount : vertexCount,
                       gVertexModeToGL[vmode],
                       glVerts, glTexs, colorArray, indices, iter);
    
    if (cache) {
        SkGLDevice::UnlockTexCache(cache);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkGLTextCache.h"

void SkGLDevice::GlyphCacheAuxProc(void* data) {
    SkDebugf("-------------- delete text texture cache\n");
    SkDELETE((SkGLTextCache*)data);
}

#ifdef SK_SCALAR_IS_FIXED
#define SkDiv16ToScalar(numer, denom)    (SkIntToFixed(numer) / (denom))
#else
#define SkDiv16ToScalar(numer, denom)    SkScalarDiv(numer, denom)
#endif

// stolen from SkDraw.cpp - D1G_NoBounder_RectClip
static void SkGL_Draw1Glyph(const SkDraw1Glyph& state, const SkGlyph& glyph,
                            int x, int y) {
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

    SkGLDrawProcs* procs = (SkGLDrawProcs*)state.fDraw->fProcs;
    
    x += glyph.fLeft;
    y  += glyph.fTop;
    
    // check if we're clipped out (nothing to draw)
	SkIRect bounds;
	bounds.set(x, y, x + glyph.fWidth, y + glyph.fHeight);
    if (!SkIRect::Intersects(state.fClip->getBounds(), bounds)) {
        return;
    }
    
    // now dig up our texture cache
    
    SkGlyphCache* gcache = state.fCache;
    void* auxData;
    SkGLTextCache* textCache = NULL;
    
    if (gcache->getAuxProcData(SkGLDevice::GlyphCacheAuxProc, &auxData)) {
        textCache = (SkGLTextCache*)auxData;            
    }
    if (NULL == textCache) {
        // need to create one
        textCache = SkNEW(SkGLTextCache);
        gcache->setAuxProc(SkGLDevice::GlyphCacheAuxProc, textCache);
    }
    
    int offset;
    SkGLTextCache::Strike* strike = textCache->findGlyph(glyph, &offset);
    if (NULL == strike) {
        // make sure the glyph has an image
        uint8_t* aa = (uint8_t*)glyph.fImage;               
        if (NULL == aa) {
            aa = (uint8_t*)gcache->findImage(glyph);
            if (NULL == aa) {
                return; // can't rasterize glyph
            }
        }
        strike = textCache->addGlyphAndBind(glyph, aa, &offset);
        if (NULL == strike) {
            // too big to cache, need to draw as is...
            return;
        }
    }
    
    const int shiftW = strike->widthShift();
    const int shiftH = strike->heightShift();
    
    SkFixed left = offset << (16 - shiftW);
    SkFixed right = (offset + glyph.fWidth) << (16 - shiftW);
    SkFixed bottom = glyph.fHeight << (16 - shiftH);

    procs->addQuad(strike->texture(), x, y, glyph, left, right, bottom);
}

#if 1
// matches the orientation used in SkGL::setRectFan. Too bad we can't rely on
// QUADS in android's GL
static const uint8_t gQuadIndices[] = {
    0,   1,   2,   0,   2,   3,
    4,   5,   6,   4,   6,   7,
    8,   9,  10,   8,  10,  11,
    12,  13,  14,  12,  14,  15,
    16,  17,  18,  16,  18,  19,
    20,  21,  22,  20,  22,  23,
    24,  25,  26,  24,  26,  27,
    28,  29,  30,  28,  30,  31,
    32,  33,  34,  32,  34,  35,
    36,  37,  38,  36,  38,  39,
    40,  41,  42,  40,  42,  43,
    44,  45,  46,  44,  46,  47,
    48,  49,  50,  48,  50,  51,
    52,  53,  54,  52,  54,  55,
    56,  57,  58,  56,  58,  59,
    60,  61,  62,  60,  62,  63,
    64,  65,  66,  64,  66,  67,
    68,  69,  70,  68,  70,  71,
    72,  73,  74,  72,  74,  75,
    76,  77,  78,  76,  78,  79,
    80,  81,  82,  80,  82,  83,
    84,  85,  86,  84,  86,  87,
    88,  89,  90,  88,  90,  91,
    92,  93,  94,  92,  94,  95,
    96,  97,  98,  96,  98,  99,
    100, 101, 102, 100, 102, 103,
    104, 105, 106, 104, 106, 107,
    108, 109, 110, 108, 110, 111,
    112, 113, 114, 112, 114, 115,
    116, 117, 118, 116, 118, 119,
    120, 121, 122, 120, 122, 123,
    124, 125, 126, 124, 126, 127
};
#else
static void generateQuadIndices(int n) {
    int index = 0;
    for (int i = 0; i < n; i++) {
        SkDebugf("    %3d, %3d, %3d, %3d, %3d, %3d,\n",
                 index, index + 1, index + 2, index, index + 2, index + 3);
        index += 4;
    }
}
#endif

void SkGLDrawProcs::drawQuads() {
    SkASSERT(SK_ARRAY_COUNT(gQuadIndices) == MAX_QUADS * 6);

    glBindTexture(GL_TEXTURE_2D, fCurrTexture);

#if 0
    static bool gOnce;
    if (!gOnce) {
        generateQuadIndices(MAX_QUADS);
        gOnce = true;
    }
#endif

    // convert from quad vertex count to triangle vertex count
    // 6/4 * n == n + (n >> 1) since n is always a multiple of 4
    SkASSERT((fCurrQuad & 3) == 0);
    int count = fCurrQuad + (fCurrQuad >> 1);

    if (fClip->isComplex()) {
        SkGLClipIter iter(fViewportHeight);
        iter.reset(*fClip);
        while (!iter.done()) {
            iter.scissor();
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_BYTE, gQuadIndices);
            iter.next();
        }
    } else {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_BYTE, gQuadIndices);
    }
}

void SkGLDevice::setupForText(SkDraw* draw, const SkPaint& paint) {
    // we handle complex clips in the SkDraw common code, so we don't check
    // for it here
    this->updateMatrixClip();
    
    SkGL::SetPaint(paint, false);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // deferred allocation
    if (NULL == fDrawProcs) {
        fDrawProcs = SkNEW(SkGLDrawProcs);
        fDrawProcs->fD1GProc = SkGL_Draw1Glyph;
    }

    // init our (and GL's) state
    fDrawProcs->init(draw->fClip, this->height());
    // assign to the caller's SkDraw
    draw->fProcs = fDrawProcs;

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glShadeModel(GL_FLAT); 
}

void SkGLDevice::drawText(const SkDraw& draw, const void* text,
                          size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    /*  Currently, perspective text is draw via paths, invoked directly by
     SkDraw. This can't work for us, since the bitmap that our draw points
     to has no pixels, so we just abort if we're in perspective.
     
     Better fix would be to...
     - have a callback inside draw to handle path drawing
     - option to have draw call the font cache, which we could patch (?)
     */
    if (draw.fMatrix->getType() & SkMatrix::kPerspective_Mask) {
        return;
    }
    
    SkDraw myDraw(draw);
    this->setupForText(&myDraw, paint);
    this->INHERITED::drawText(myDraw, text, byteLength, x, y, paint);
    fDrawProcs->flush();
    glPopMatrix();  // GL_MODELVIEW
}

void SkGLDevice::drawPosText(const SkDraw& draw, const void* text,
                             size_t byteLength, const SkScalar pos[],
                             SkScalar constY, int scalarsPerPos,
                             const SkPaint& paint) {
    if (draw.fMatrix->getType() & SkMatrix::kPerspective_Mask) {
        return;
    }
    
    SkDraw myDraw(draw);
    this->setupForText(&myDraw, paint);
    this->INHERITED::drawPosText(myDraw, text, byteLength, pos, constY,
                                 scalarsPerPos, paint);
    fDrawProcs->flush();
    glPopMatrix();  // GL_MODELVIEW
}

void SkGLDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                size_t byteLength, const SkPath& path,
                                const SkMatrix* m, const SkPaint& paint) {
    // not supported yet
}

