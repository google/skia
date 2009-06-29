#include "SkGL.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkXfermode.h"

//#define TRACE_TEXTURE_CREATION

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_GL_HAS_COLOR4UB
static inline void gl_pmcolor(U8CPU r, U8CPU g, U8CPU b, U8CPU a) {
    glColor4ub(r, g, b, a);
}

void SkGL::SetAlpha(U8CPU alpha) {
    glColor4ub(alpha, alpha, alpha, alpha);
}
#else
static inline SkFixed byte2fixed(U8CPU value) {
    return (value + (value >> 7)) << 8;
}

static inline void gl_pmcolor(U8CPU r, U8CPU g, U8CPU b, U8CPU a) {
    glColor4x(byte2fixed(r), byte2fixed(g), byte2fixed(b), byte2fixed(a));
}

void SkGL::SetAlpha(U8CPU alpha) {
    SkFixed fa = byte2fixed(alpha);
    glColor4x(fa, fa, fa, fa);
}
#endif

void SkGL::SetColor(SkColor c) {
    SkPMColor pm = SkPreMultiplyColor(c);
    gl_pmcolor(SkGetPackedR32(pm),
               SkGetPackedG32(pm),
               SkGetPackedB32(pm),
               SkGetPackedA32(pm));
}

static const GLenum gXfermodeCoeff2Blend[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
};

void SkGL::SetPaint(const SkPaint& paint, bool isPremul, bool justAlpha) {
    if (justAlpha) {
        SkGL::SetAlpha(paint.getAlpha());
    } else {
        SkGL::SetColor(paint.getColor());
    }

    GLenum sm = GL_ONE;
    GLenum dm = GL_ONE_MINUS_SRC_ALPHA;

    SkXfermode* mode = paint.getXfermode();
    SkXfermode::Coeff sc, dc;
    if (mode && mode->asCoeff(&sc, &dc)) {
        sm = gXfermodeCoeff2Blend[sc];
        dm = gXfermodeCoeff2Blend[dc];
    }
    
    // hack for text, which is not-premul (afaik)
    if (!isPremul) {
        if (GL_ONE == sm) {
            sm = GL_SRC_ALPHA;
        }
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(sm, dm);
    
    if (paint.isDither()) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGL::DumpError(const char caller[]) {
    GLenum err = glGetError();
    if (err) {
        SkDebugf("---- glGetError(%s) %d\n", caller, err);
    }
}

void SkGL::SetRGBA(uint8_t rgba[], const SkColor src[], int count) {
    for (int i = 0; i < count; i++) {
        SkPMColor c = SkPreMultiplyColor(*src++);
        *rgba++ = SkGetPackedR32(c);
        *rgba++ = SkGetPackedG32(c);
        *rgba++ = SkGetPackedB32(c);
        *rgba++ = SkGetPackedA32(c);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGL::Scissor(const SkIRect& r, int viewportHeight) {
    glScissor(r.fLeft, viewportHeight - r.fBottom, r.width(), r.height());
}

///////////////////////////////////////////////////////////////////////////////

void SkGL::Ortho(float left, float right, float bottom, float top,
                 float near, float far) {
    
    float mat[16];
    
    sk_bzero(mat, sizeof(mat));
    
    mat[0] = 2 / (right - left);
    mat[5] = 2 / (top - bottom);
    mat[10] = 2 / (near - far);
    mat[15] = 1;
    
    mat[12] = (right + left) / (left - right);
    mat[13] = (top + bottom) / (bottom - top);
    mat[14] = (far + near) / (near - far);
    
    glMultMatrixf(mat);
}

///////////////////////////////////////////////////////////////////////////////

static bool canBeTexture(const SkBitmap& bm, GLenum* format, GLenum* type) {
    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            *format = GL_RGBA;
            *type = GL_UNSIGNED_BYTE;
            break;
        case SkBitmap::kRGB_565_Config:
            *format = GL_RGB;
            *type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case SkBitmap::kARGB_4444_Config:
            *format = GL_RGBA;
            *type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case SkBitmap::kIndex8_Config:
#ifdef SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
            *format = GL_PALETTE8_RGBA8_OES;
            *type = GL_UNSIGNED_BYTE;   // unused I think
#else
            // we promote index to argb32
            *format = GL_RGBA;
            *type = GL_UNSIGNED_BYTE;
#endif
            break;
        case SkBitmap::kA8_Config:
            *format = GL_ALPHA;
            *type = GL_UNSIGNED_BYTE;
            break;
        default:
            return false;
    }
    return true;
}

#define SK_GL_SIZE_OF_PALETTE   (256 * sizeof(SkPMColor))

size_t SkGL::ComputeTextureMemorySize(const SkBitmap& bitmap) {
    int shift = 0;
    size_t adder = 0;
    switch (bitmap.config()) {
        case SkBitmap::kARGB_8888_Config:
        case SkBitmap::kRGB_565_Config:
        case SkBitmap::kARGB_4444_Config:
        case SkBitmap::kA8_Config:
            // we're good as is
            break;
        case SkBitmap::kIndex8_Config:
#ifdef SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
            // account for the colortable
            adder = SK_GL_SIZE_OF_PALETTE;
#else
            // we promote index to argb32
            shift = 2;
#endif
            break;
        default:
            return 0;
    }
    return (bitmap.getSize() << shift) + adder;
}

#ifdef SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
/*  Fill out buffer with the compressed format GL expects from a colortable
    based bitmap. [palette (colortable) + indices].

    At the moment I always take the 8bit version, since that's what my data
    is. I could detect that the colortable.count is <= 16, and then repack the
    indices as nibbles to save RAM, but it would take more time (i.e. a lot
    slower than memcpy), so I'm skipping that for now.
 
    GL wants a full 256 palette entry, even though my ctable is only as big
    as the colortable.count says it is. I presume it is OK to leave any
    trailing entries uninitialized, since none of my indices should exceed
    ctable->count().
*/
static void build_compressed_data(void* buffer, const SkBitmap& bitmap) {
    SkASSERT(SkBitmap::kIndex8_Config == bitmap.config());

    SkColorTable* ctable = bitmap.getColorTable();
    uint8_t* dst = (uint8_t*)buffer;

    memcpy(dst, ctable->lockColors(), ctable->count() * sizeof(SkPMColor));
    ctable->unlockColors(false);

    // always skip a full 256 number of entries, even if we memcpy'd fewer
    dst += SK_GL_SIZE_OF_PALETTE;
    memcpy(dst, bitmap.getPixels(), bitmap.getSize());
}
#endif

/*  Return true if the bitmap cannot be supported in its current config as a
    texture, and it needs to be promoted to ARGB32.
 */
static bool needToPromoteTo32bit(const SkBitmap& bitmap) {
    if (bitmap.config() == SkBitmap::kIndex8_Config) {
#ifdef SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
        const int w = bitmap.width();
        const int h = bitmap.height();
        if (SkNextPow2(w) == w && SkNextPow2(h) == h) {
            // we can handle Indx8 if we're a POW2
            return false;
        }
#endif
        return true;    // must promote to ARGB32
    }
    return false;
}

GLuint SkGL::BindNewTexture(const SkBitmap& origBitmap, SkPoint* max) {
    SkBitmap tmpBitmap;
    const SkBitmap* bitmap = &origBitmap;

    if (needToPromoteTo32bit(origBitmap)) {
        origBitmap.copyTo(&tmpBitmap, SkBitmap::kARGB_8888_Config);
        // now bitmap points to our temp, which has been promoted to 32bits
        bitmap = &tmpBitmap;
    }
    
    GLenum format, type;
    if (!canBeTexture(*bitmap, &format, &type)) {
        return 0;
    }
    
    SkAutoLockPixels alp(*bitmap);
    if (!bitmap->readyToDraw()) {
        return 0;
    }
    
    GLuint  textureName;
    glGenTextures(1, &textureName);
    
    glBindTexture(GL_TEXTURE_2D, textureName);
    
    // express rowbytes as a number of pixels for ow
    int ow = bitmap->rowBytesAsPixels();
    int oh = bitmap->height();
    int nw = SkNextPow2(ow);
    int nh = SkNextPow2(oh);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap->bytesPerPixel());
    
    // check if we need to scale to create power-of-2 dimensions
#ifdef SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
    if (SkBitmap::kIndex8_Config == bitmap->config()) {
        size_t imagesize = bitmap->getSize() + SK_GL_SIZE_OF_PALETTE;
        SkAutoMalloc storage(imagesize);

        build_compressed_data(storage.get(), *bitmap);
        // we only support POW2 here (GLES 1.0 restriction)
        SkASSERT(ow == nw);
        SkASSERT(oh == nh);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, ow, oh, 0,
                               imagesize, storage.get());
    } else  // fall through to non-compressed logic
#endif
    {
        if (ow != nw || oh != nh) {
            glTexImage2D(GL_TEXTURE_2D, 0, format, nw, nh, 0,
                         format, type, NULL);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ow, oh,
                            format, type, bitmap->getPixels());
        } else {
            // easy case, the bitmap is already pow2
            glTexImage2D(GL_TEXTURE_2D, 0, format, ow, oh, 0,
                         format, type, bitmap->getPixels());
        }
    }
    
#ifdef TRACE_TEXTURE_CREATION
    SkDebugf("--- new texture [%d] size=(%d %d) bpp=%d\n", textureName, ow, oh,
             bitmap->bytesPerPixel());
#endif

    if (max) {
        max->fX = SkFixedToScalar(bitmap->width() << (16 - SkNextLog2(nw)));
        max->fY = SkFixedToScalar(oh << (16 - SkNextLog2(nh)));
    }
    return textureName;
}

static const GLenum gTileMode2GLWrap[] = {
    GL_CLAMP_TO_EDGE,
    GL_REPEAT,
#if GL_VERSION_ES_CM_1_0
    GL_REPEAT       // GLES doesn't support MIRROR
#else
    GL_MIRRORED_REPEAT
#endif
};

void SkGL::SetTexParams(bool doFilter,
                        SkShader::TileMode tx, SkShader::TileMode ty) {
    SkASSERT((unsigned)tx < SK_ARRAY_COUNT(gTileMode2GLWrap));
    SkASSERT((unsigned)ty < SK_ARRAY_COUNT(gTileMode2GLWrap));
    
    GLenum filter = doFilter ? GL_LINEAR : GL_NEAREST;

    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gTileMode2GLWrap[tx]);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gTileMode2GLWrap[ty]);
}

void SkGL::SetTexParamsClamp(bool doFilter) {
    GLenum filter = doFilter ? GL_LINEAR : GL_NEAREST;

    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

///////////////////////////////////////////////////////////////////////////////

void SkGL::DrawVertices(int count, GLenum mode,
                        const SkGLVertex* SK_RESTRICT vertex,
                        const SkGLVertex* SK_RESTRICT texCoords,
                        const uint8_t* SK_RESTRICT colorArray,
                        const uint16_t* SK_RESTRICT indexArray,
                        SkGLClipIter* iter) {
    SkASSERT(NULL != vertex);
    
    if (NULL != texCoords) {
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, SK_GLType, 0, texCoords);
    } else {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if (NULL != colorArray) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);
        glShadeModel(GL_SMOOTH); 
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
        glShadeModel(GL_FLAT); 
    }
    
    glVertexPointer(2, SK_GLType, 0, vertex);

    if (NULL != indexArray) {
        if (iter) {
            while (!iter->done()) {
                iter->scissor();
                glDrawElements(mode, count, GL_UNSIGNED_SHORT, indexArray);
                iter->next();
            }
        } else {
            glDrawElements(mode, count, GL_UNSIGNED_SHORT, indexArray);
        }
    } else {
        if (iter) {
            while (!iter->done()) {
                iter->scissor();
                glDrawArrays(mode, 0, count);
                iter->next();
            }
        } else {
            glDrawArrays(mode, 0, count);
        }
    }
}

void SkGL::PrepareForFillPath(SkPaint* paint) {
    if (paint->getStrokeWidth() <= 0) {
        paint->setStrokeWidth(SK_Scalar1);
    }
}

void SkGL::FillPath(const SkPath& path, const SkPaint& paint, bool useTex,
                    SkGLClipIter* iter) {
    SkPaint p(paint);
    SkPath  fillPath;
    
    SkGL::PrepareForFillPath(&p);
    p.getFillPath(path, &fillPath);
    SkGL::DrawPath(fillPath, useTex, iter);
}

// should return max of all contours, rather than the sum (to save temp RAM)
static int worst_case_edge_count(const SkPath& path) {
    int edgeCount = 0;
    
    SkPath::Iter    iter(path, true);
    SkPath::Verb    verb;
    
    while ((verb = iter.next(NULL)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                edgeCount += 1;
                break;
            case SkPath::kQuad_Verb:
                edgeCount += 8;
                break;
            case SkPath::kCubic_Verb:
                edgeCount += 16;
                break;
            default:
                break;
        }
    }
    return edgeCount;
}

void SkGL::DrawPath(const SkPath& path, bool useTex, SkGLClipIter* clipIter) {
    const SkRect& bounds = path.getBounds();
    if (bounds.isEmpty()) {
        return;
    }
    
    int maxPts = worst_case_edge_count(path);
    // add 1 for center of fan, and 1 for closing edge
    SkAutoSTMalloc<32, SkGLVertex>  storage(maxPts + 2);
    SkGLVertex* base = storage.get();
    SkGLVertex* vert = base;
    SkGLVertex* texs = useTex ? base : NULL;

    SkPath::Iter    pathIter(path, true);
    SkPoint         pts[4];
    
    bool needEnd = false;
    
    for (;;) {
        switch (pathIter.next(pts)) {
            case SkPath::kMove_Verb:
                if (needEnd) {
                    SkGL::DrawVertices(vert - base, GL_TRIANGLE_FAN,
                                       base, texs, NULL, NULL, clipIter);
                    clipIter->safeRewind();
                    vert = base;
                }
                needEnd = true;
                // center of the FAN
                vert->setScalars(bounds.centerX(), bounds.centerY());
                vert++;
                // add first edge point
                vert->setPoint(pts[0]);
                vert++;
                break;
                case SkPath::kLine_Verb:
                vert->setPoint(pts[1]);
                vert++;
                break;
                case SkPath::kQuad_Verb: {
                    const int n = 8;
                    const SkScalar dt = SK_Scalar1 / n;
                    SkScalar t = dt;
                    for (int i = 1; i < n; i++) {
                        SkPoint loc;
                        SkEvalQuadAt(pts, t, &loc, NULL);
                        t += dt;
                        vert->setPoint(loc);
                        vert++;
                    }
                    vert->setPoint(pts[2]);
                    vert++;
                    break;
                }
                case SkPath::kCubic_Verb: {
                    const int n = 16;
                    const SkScalar dt = SK_Scalar1 / n;
                    SkScalar t = dt;
                    for (int i = 1; i < n; i++) {
                        SkPoint loc;
                        SkEvalCubicAt(pts, t, &loc, NULL, NULL);
                        t += dt;
                        vert->setPoint(loc);
                        vert++;
                    }
                    vert->setPoint(pts[3]);
                    vert++;
                    break;
                }
                case SkPath::kClose_Verb:
                break;
                case SkPath::kDone_Verb:
                goto FINISHED;
        }
    }
FINISHED:
    if (needEnd) {
        SkGL::DrawVertices(vert - base, GL_TRIANGLE_FAN, base, texs,
                           NULL, NULL, clipIter);
    }
}

