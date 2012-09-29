#include "SkArithmeticMode.h"
#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"

class SkArithmeticMode_scalar : public SkXfermode {
public:
    SkArithmeticMode_scalar(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4) {
        fK[0] = k1;
        fK[1] = k2;
        fK[2] = k3;
        fK[3] = k4;
    }

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) SK_OVERRIDE;

    SK_DECLARE_UNFLATTENABLE_OBJECT()

private:
    SkScalar fK[4];
};

static int pinToByte(int value) {
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    return value;
}

static int arith(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                 int src, int dst) {
    SkScalar result = SkScalarMul(k1, src * dst) +
                      SkScalarMul(k2, src) +
                      SkScalarMul(k3, dst) +
                      k4;
    int res = SkScalarRoundToInt(result);
    return pinToByte(res);
}

static int blend(int src, int dst, int scale) {
    return dst + ((src - dst) * scale >> 8);
}

static bool needsUnpremul(int alpha) {
    return 0 != alpha && 0xFF != alpha;
}

void SkArithmeticMode_scalar::xfer32(SkPMColor dst[], const SkPMColor src[],
                                     int count, const SkAlpha aaCoverage[]) {
    SkScalar k1 = fK[0] / 255;
    SkScalar k2 = fK[1];
    SkScalar k3 = fK[2];
    SkScalar k4 = fK[3] * 255;

    for (int i = 0; i < count; ++i) {
        if ((NULL == aaCoverage) || aaCoverage[i]) {
            SkPMColor sc = src[i];
            SkPMColor dc = dst[i];
            int sa = SkGetPackedA32(sc);
            int da = SkGetPackedA32(dc);

            int srcNeedsUnpremul = needsUnpremul(sa);
            int dstNeedsUnpremul = needsUnpremul(sa);

            int a, r, g, b;

            if (!srcNeedsUnpremul && !dstNeedsUnpremul) {
                a = arith(k1, k2, k3, k4, sa, sa);
                r = arith(k1, k2, k3, k4, SkGetPackedR32(sc), SkGetPackedR32(dc));
                g = arith(k1, k2, k3, k4, SkGetPackedG32(sc), SkGetPackedG32(dc));
                b = arith(k1, k2, k3, k4, SkGetPackedB32(sc), SkGetPackedB32(dc));
            } else {
                int sr = SkGetPackedR32(sc);
                int sg = SkGetPackedG32(sc);
                int sb = SkGetPackedB32(sc);
                if (srcNeedsUnpremul) {
                    SkUnPreMultiply::Scale scale = SkUnPreMultiply::GetScale(sa);
                    sr = SkUnPreMultiply::ApplyScale(scale, sr);
                    sg = SkUnPreMultiply::ApplyScale(scale, sg);
                    sb = SkUnPreMultiply::ApplyScale(scale, sb);
                }

                int dr = SkGetPackedR32(dc);
                int dg = SkGetPackedG32(dc);
                int db = SkGetPackedB32(dc);
                if (dstNeedsUnpremul) {
                    SkUnPreMultiply::Scale scale = SkUnPreMultiply::GetScale(da);
                    dr = SkUnPreMultiply::ApplyScale(scale, dr);
                    dg = SkUnPreMultiply::ApplyScale(scale, dg);
                    db = SkUnPreMultiply::ApplyScale(scale, db);
                }

                a = arith(k1, k2, k3, k4, sa, sa);
                r = arith(k1, k2, k3, k4, sr, dr);
                g = arith(k1, k2, k3, k4, sg, dg);
                b = arith(k1, k2, k3, k4, sb, db);
            }

            // apply antialias coverage if necessary
            if (aaCoverage && 0xFF != aaCoverage[i]) {
                int scale = aaCoverage[i] + (aaCoverage[i] >> 7);
                a = blend(a, SkGetPackedA32(sc), scale);
                r = blend(r, SkGetPackedR32(sc), scale);
                g = blend(g, SkGetPackedG32(sc), scale);
                b = blend(b, SkGetPackedB32(sc), scale);
            }

            // turn the result back into premul
            if (0xFF != a) {
                int scale = a + (a >> 7);
                r = SkAlphaMul(r, scale);
                g = SkAlphaMul(g, scale);
                b = SkAlphaMul(b, scale);
            }
            dst[i] = SkPackARGB32(a, r, g, b);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////

static bool fitsInBits(SkScalar x, int bits) {
#ifdef SK_SCALAR_IS_FIXED
    x = SkAbs32(x);
    x += 1 << 7;
    x >>= 8;
    return x < (1 << (bits - 1));
#else
    return SkScalarAbs(x) < (1 << (bits - 1));
#endif
}

#if 0 // UNUSED
static int32_t toDot8(SkScalar x) {
#ifdef SK_SCALAR_IS_FIXED
    x += 1 << 7;
    x >>= 8;
    return x;
#else
    return (int32_t)(x * 256);
#endif
}
#endif

SkXfermode* SkArithmeticMode::Create(SkScalar k1, SkScalar k2,
                                     SkScalar k3, SkScalar k4) {
    if (fitsInBits(k1, 8) && fitsInBits(k2, 16) &&
        fitsInBits(k2, 16) && fitsInBits(k2, 24)) {

#if 0 // UNUSED
        int32_t i1 = toDot8(k1);
        int32_t i2 = toDot8(k2);
        int32_t i3 = toDot8(k3);
        int32_t i4 = toDot8(k4);
        if (i1) {
            return SkNEW_ARGS(SkArithmeticMode_quad, (i1, i2, i3, i4));
        }
        if (0 == i2) {
            return SkNEW_ARGS(SkArithmeticMode_dst, (i3, i4));
        }
        if (0 == i3) {
            return SkNEW_ARGS(SkArithmeticMode_src, (i2, i4));
        }
        return SkNEW_ARGS(SkArithmeticMode_linear, (i2, i3, i4));
#endif
    }
    return SkNEW_ARGS(SkArithmeticMode_scalar, (k1, k2, k3, k4));
}

