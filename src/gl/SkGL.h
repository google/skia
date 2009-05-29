#ifndef SkGL_DEFINED
#define SkGL_DEFINED

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_SDL)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #include <AGL/agl.h>
    // use FBOs for devices
    #define SK_GL_DEVICE_FBO
#elif defined(ANDROID)
    #include <GLES/gl.h>
    #include <EGL/egl.h>
#endif

#include "SkColor.h"
#include "SkMatrix.h"
#include "SkShader.h"

class SkPaint;
class SkPath;

class SkGLClipIter;

//#define TRACE_TEXTURE_CREATE

///////////////////////////////////////////////////////////////////////////////

#if GL_OES_compressed_paletted_texture
    #define SK_GL_SUPPORT_COMPRESSEDTEXIMAGE2D
#endif

#if GL_OES_fixed_point && defined(SK_SCALAR_IS_FIXED)
    #define SK_GLType   GL_FIXED
#else
    #define SK_GLType   GL_FLOAT
#endif

#if SK_GLType == GL_FIXED
    typedef SkFixed SkGLScalar;

    #define SkIntToGL(n)        SkIntToFixed(n)
    #define SkScalarToGL(x)     SkScalarToFixed(x)
    #define SK_GLScalar1        SK_Fixed1
    #define SkGLScalarMul(a, b) SkFixedMul(a, b)
    #define MAKE_GL(name)       name ## x

    #ifdef SK_SCALAR_IS_FIXED
        #define GLSCALAR_IS_SCALAR  1
        #define SkPerspToGL(x)      SkFractToFixed(x)
    #else
        #define GLSCALAR_IS_SCALAR  0
        #define SkPerspToGL(x)      SkFractToFloat(x)
    #endif
#else
    typedef float SkGLScalar;

    #define SkIntToGL(n)        (n)
    #define SkScalarToGL(x)     SkScalarToFloat(x)
    #define SK_GLScalar1        (1.f)
    #define SkGLScalarMul(a, b) ((a) * (b))
    #define MAKE_GL(name)       name ## f

    #ifdef SK_SCALAR_IS_FLOAT
        #define GLSCALAR_IS_SCALAR  1
        #define SkPerspToGL(x)      (x)
    #else
        #define GLSCALAR_IS_SCALAR  0
        #define SkPerspToGL(x)      SkFractToFloat(x)
    #endif
#endif

#if GL_OES_fixed_point
    typedef SkFixed SkGLTextScalar;
    #define SK_TextGLType       GL_FIXED

    #define SkIntToTextGL(n)    SkIntToFixed(n)
    #define SkFixedToTextGL(x)  (x)

    #define SK_glTexParameteri(target, pname, param) \
                glTexParameterx(target, pname, param)
#else
    typedef float SkGLTextScalar;
    #define SK_TextGLType       SK_GLType
    #define SK_GL_HAS_COLOR4UB

    #define SkIntToTextGL(n)    SkIntToGL(n)
    #define SkFixedToTextGL(x)  SkFixedToFloat(x)


    #define SK_glTexParameteri(target, pname, param) \
                glTexParameteri(target, pname, param)
#endif

///////////////////////////////////////////////////////////////////////////////

// text has its own vertex class, since it may want to be in fixed point (given)
// that it starts with all integers) even when the default vertices are floats
struct SkGLTextVertex {
    SkGLTextScalar fX;
    SkGLTextScalar fY;
    
    void setI(int x, int y) {
        fX = SkIntToTextGL(x);
        fY = SkIntToTextGL(y);
    }
    
    void setX(SkFixed x, SkFixed y) {
        fX = SkFixedToTextGL(x);
        fY = SkFixedToTextGL(y);
    }
    
    // counter-clockwise fan
    void setIRectFan(int l, int t, int r, int b) {
        SkGLTextVertex* SK_RESTRICT v = this;
        v[0].setI(l, t);
        v[1].setI(l, b);
        v[2].setI(r, b);
        v[3].setI(r, t);
    }
    
    // counter-clockwise fan
    void setXRectFan(SkFixed l, SkFixed t, SkFixed r, SkFixed b) {
        SkGLTextVertex* SK_RESTRICT v = this;
        v[0].setX(l, t);
        v[1].setX(l, b);
        v[2].setX(r, b);
        v[3].setX(r, t);
    }
};

struct SkGLVertex {
    SkGLScalar  fX;
    SkGLScalar  fY;
    
    void setGL(SkGLScalar x, SkGLScalar y) {
        fX = x;
        fY = y;
    }

    void setScalars(SkScalar x, SkScalar y) {
        fX = SkScalarToGL(x);
        fY = SkScalarToGL(y);
    }
    
    void setPoint(const SkPoint& pt) {
        fX = SkScalarToGL(pt.fX);
        fY = SkScalarToGL(pt.fY);
    }
    
    void setPoints(const SkPoint* SK_RESTRICT pts, int count) {
        const SkScalar* SK_RESTRICT src = (const SkScalar*)pts;
        SkGLScalar* SK_RESTRICT dst = (SkGLScalar*)this;
        for (int i = 0; i < count; i++) {
            *dst++ = SkScalarToGL(*src++);
            *dst++ = SkScalarToGL(*src++);
        }
    }
    
    // counter-clockwise fan
    void setRectFan(SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
        SkGLVertex* v = this;
        v[0].setScalars(l, t);
        v[1].setScalars(l, b);
        v[2].setScalars(r, b);
        v[3].setScalars(r, t);
    }
    
    // counter-clockwise fan
    void setIRectFan(int l, int t, int r, int b) {
        SkGLVertex* v = this;
        v[0].setGL(SkIntToGL(l), SkIntToGL(t));
        v[1].setGL(SkIntToGL(l), SkIntToGL(b));
        v[2].setGL(SkIntToGL(r), SkIntToGL(b));
        v[3].setGL(SkIntToGL(r), SkIntToGL(t));
    }
    
    // counter-clockwise fan
    void setRectFan(const SkRect& r) {
        this->setRectFan(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }
    
    // counter-clockwise fan
    void setIRectFan(const SkIRect& r) {
        this->setIRectFan(r.fLeft, r.fTop, r.fRight, r.fBottom);
    }
};

struct SkGLMatrix {
    SkGLScalar fMat[16];
    
    void reset() {
        bzero(fMat, sizeof(fMat));
        fMat[0] = fMat[5] = fMat[10] = fMat[15] = SK_GLScalar1;
    }
    
    void set(const SkMatrix& m) {
        bzero(fMat, sizeof(fMat));
        fMat[0] = SkScalarToGL(m[SkMatrix::kMScaleX]);
        fMat[4] = SkScalarToGL(m[SkMatrix::kMSkewX]);
        fMat[12] = SkScalarToGL(m[SkMatrix::kMTransX]);
        
        fMat[1] = SkScalarToGL(m[SkMatrix::kMSkewY]);
        fMat[5] = SkScalarToGL(m[SkMatrix::kMScaleY]);
        fMat[13] = SkScalarToGL(m[SkMatrix::kMTransY]);
                
        fMat[3] = SkPerspToGL(m[SkMatrix::kMPersp0]);
        fMat[7] = SkPerspToGL(m[SkMatrix::kMPersp1]);
        fMat[15] = SkPerspToGL(m[SkMatrix::kMPersp2]);

        fMat[10] = SK_GLScalar1;    // z-scale
    }
};

class SkGL {
public:
    static void SetColor(SkColor c);
    static void SetAlpha(U8CPU alpha);
    static void SetPaint(const SkPaint&, bool isPremul = true,
                         bool justAlpha = false);
    static void SetPaintAlpha(const SkPaint& paint, bool isPremul = true) {
        SetPaint(paint, isPremul, true);
    }

    static void SetRGBA(uint8_t rgba[], const SkColor src[], int count);
    static void DumpError(const char caller[]);
    
    static void Ortho(float left, float right, float bottom, float top,
                      float near, float far);

    static inline void Translate(SkScalar dx, SkScalar dy) {
        MAKE_GL(glTranslate)(SkScalarToGL(dx), SkScalarToGL(dy), 0);
    }
    
    static inline void Scale(SkScalar sx, SkScalar sy) {
        MAKE_GL(glScale)(SkScalarToGL(sx), SkScalarToGL(sy), SK_GLScalar1);
    }

    static inline void Rotate(SkScalar angle) {
        MAKE_GL(glRotate)(SkScalarToGL(angle), 0, 0, SK_GLScalar1);
    }

    static inline void MultMatrix(const SkMatrix& m) {
        SkGLMatrix glm;
        glm.set(m);
        MAKE_GL(glMultMatrix)(glm.fMat);
    }
    
    static inline void LoadMatrix(const SkMatrix& m) {
        SkGLMatrix glm;
        glm.set(m);
        MAKE_GL(glLoadMatrix)(glm.fMat);
    }
    
    static void Scissor(const SkIRect&, int viewportHeight);
    
    // return the byte size for the associated texture memory. This doesn't
    // always == bitmap.getSize(), since on a given port we may have to change
    // the format when the bitmap's pixels are copied over to GL
    static size_t ComputeTextureMemorySize(const SkBitmap&);
    // return 0 on failure
    static GLuint BindNewTexture(const SkBitmap&, SkPoint* dimension);

    static void SetTexParams(bool filter,
                             SkShader::TileMode tx, SkShader::TileMode ty);
    static void SetTexParamsClamp(bool filter);

    static void DrawVertices(int count, GLenum mode,
                             const SkGLVertex* SK_RESTRICT vertex,
                             const SkGLVertex* SK_RESTRICT texCoords,
                             const uint8_t* SK_RESTRICT colorArray,
                             const uint16_t* SK_RESTRICT indexArray,
                             SkGLClipIter*);
    
    static void PrepareForFillPath(SkPaint* paint);
    static void FillPath(const SkPath& path, const SkPaint& paint, bool useTex,
                         SkGLClipIter*);
    static void DrawPath(const SkPath& path, bool useTex, SkGLClipIter*);
};

#include "SkRegion.h"

class SkGLClipIter : public SkRegion::Iterator {
public:
    SkGLClipIter(int viewportHeight) : fViewportHeight(viewportHeight) {}
    
    // call rewind only if this is non-null
    void safeRewind() {
        if (this) {
            this->rewind();
        }
    }

    void scissor() {
        SkASSERT(!this->done());
        SkGL::Scissor(this->rect(), fViewportHeight);
    }
    
private:
    const int fViewportHeight;
};

#endif

