#include "SkMatrix.h"

#ifdef SK_SCALAR_IS_FIXED
    typedef int64_t SkDScalar;

    static SkScalar SkDScalar_toScalar(SkDScalar value) {
        SkDScalar result = (value + (1 << 15)) >> 16;
        int top = result >> 31;
        SkASSERT(top == 0 || top == -1);
        return (SkScalar)result;
    }
    static SkScalar div(SkDScalar numer, SkDScalar denom) {
        denom >>= 16;
        return numer / denom;
    }
#else
    typedef double SkDScalar;

    static SkScalar SkDScalar_toScalar(SkDScalar value) {
        return static_cast<float>(value);
    }
    static SkScalar div(SkDScalar numer, SkDScalar denom) {
        return static_cast<float>(numer / denom);
    }
#endif

static SkDScalar SkDScalar_setMul(SkScalar a, SkScalar b) {
    return (SkDScalar)a * b;
}

static void computeOuterProduct(SkScalar op[4],
                                const SkPoint pts0[3], const SkPoint& ave0,
                                const SkPoint pts1[3], const SkPoint& ave1) {
    bzero(op, 4 * sizeof(op[0]));
    for (int i = 0; i < 3; i++) {
        SkScalar x0 = pts0[i].fX - ave0.fX;
        SkScalar y0 = pts0[i].fY - ave0.fY;
        SkScalar x1 = pts1[i].fX - ave1.fX;
        SkScalar y1 = pts1[i].fY - ave1.fY;
        op[0] += SkScalarMul(x0, x1);
        op[1] += SkScalarMul(x0, y1);
        op[2] += SkScalarMul(y0, x1);
        op[3] += SkScalarMul(y0, y1);
    }
}

static SkDScalar ddot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return SkDScalar_setMul(ax, bx) + SkDScalar_setMul(ay, by);
}

static SkScalar dot(SkScalar ax, SkScalar ay, SkScalar bx, SkScalar by) {
    return SkDScalar_toScalar(ddot(ax, ay, bx, by));
}

bool SkSetPoly3To3(SkMatrix* matrix, const SkPoint src[3], const SkPoint dst[3]) {
    const SkPoint& srcAve = src[0];
    const SkPoint& dstAve = dst[0];
    
    SkScalar srcOP[4], dstOP[4];
    
    computeOuterProduct(srcOP, src, srcAve, src, srcAve);
    computeOuterProduct(dstOP, src, srcAve, dst, dstAve);

    SkDScalar det = SkDScalar_setMul(srcOP[0], srcOP[3]) -
                    SkDScalar_setMul(srcOP[1], srcOP[2]);

    SkDScalar M[4];
    
    const SkScalar srcOP0 = srcOP[3];
    const SkScalar srcOP1 = -srcOP[1];
    const SkScalar srcOP2 = -srcOP[2];
    const SkScalar srcOP3 = srcOP[0];
    
    M[0] = ddot(srcOP0, srcOP1, dstOP[0], dstOP[2]);
    M[1] = ddot(srcOP2, srcOP3, dstOP[0], dstOP[2]);
    M[2] = ddot(srcOP0, srcOP1, dstOP[1], dstOP[3]);
    M[3] = ddot(srcOP2, srcOP3, dstOP[1], dstOP[3]);
    
    matrix->reset();
    matrix->setScaleX(div(M[0], det));
    matrix->setSkewX( div(M[1], det));
    matrix->setSkewY (div(M[2], det));
    matrix->setScaleY(div(M[3], det));
    matrix->setTranslateX(dstAve.fX - dot(srcAve.fX, srcAve.fY,
                                    matrix->getScaleX(), matrix->getSkewX()));
    matrix->setTranslateY(dstAve.fY - dot(srcAve.fX, srcAve.fY,
                                    matrix->getSkewY(), matrix->getScaleY()));
    return true;
}

