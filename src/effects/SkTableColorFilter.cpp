/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkTableColorFilter.h"

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"

class SkTable_ColorFilter : public SkColorFilter {
public:
    SkTable_ColorFilter(const uint8_t tableA[], const uint8_t tableR[],
                        const uint8_t tableG[], const uint8_t tableB[]) {
        fBitmap = nullptr;
        fFlags = 0;

        uint8_t* dst = fStorage;
        if (tableA) {
            memcpy(dst, tableA, 256);
            dst += 256;
            fFlags |= kA_Flag;
        }
        if (tableR) {
            memcpy(dst, tableR, 256);
            dst += 256;
            fFlags |= kR_Flag;
        }
        if (tableG) {
            memcpy(dst, tableG, 256);
            dst += 256;
            fFlags |= kG_Flag;
        }
        if (tableB) {
            memcpy(dst, tableB, 256);
            fFlags |= kB_Flag;
        }
    }

    virtual ~SkTable_ColorFilter() { delete fBitmap; }

    bool asComponentTable(SkBitmap* table) const override;
    sk_sp<SkColorFilter> makeComposed(sk_sp<SkColorFilter> inner) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*) const override;
#endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTable_ColorFilter)

    enum {
        kA_Flag = 1 << 0,
        kR_Flag = 1 << 1,
        kG_Flag = 1 << 2,
        kB_Flag = 1 << 3,
    };

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    mutable const SkBitmap* fBitmap; // lazily allocated

    uint8_t fStorage[256 * 4];
    unsigned fFlags;

    friend class SkTableColorFilter;

    typedef SkColorFilter INHERITED;
};

static const uint8_t gIdentityTable[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

void SkTable_ColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    const uint8_t* table = fStorage;
    const uint8_t* tableA = gIdentityTable;
    const uint8_t* tableR = gIdentityTable;
    const uint8_t* tableG = gIdentityTable;
    const uint8_t* tableB = gIdentityTable;
    if (fFlags & kA_Flag) {
        tableA = table; table += 256;
    }
    if (fFlags & kR_Flag) {
        tableR = table; table += 256;
    }
    if (fFlags & kG_Flag) {
        tableG = table; table += 256;
    }
    if (fFlags & kB_Flag) {
        tableB = table;
    }

    const SkUnPreMultiply::Scale* scaleTable = SkUnPreMultiply::GetScaleTable();
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];
        unsigned a, r, g, b;
        if (0 == c) {
            a = r = g = b = 0;
        } else {
            a = SkGetPackedA32(c);
            r = SkGetPackedR32(c);
            g = SkGetPackedG32(c);
            b = SkGetPackedB32(c);

            if (a < 255) {
                SkUnPreMultiply::Scale scale = scaleTable[a];
                r = SkUnPreMultiply::ApplyScale(scale, r);
                g = SkUnPreMultiply::ApplyScale(scale, g);
                b = SkUnPreMultiply::ApplyScale(scale, b);
            }
        }
        dst[i] = SkPremultiplyARGBInline(tableA[a], tableR[r],
                                         tableG[g], tableB[b]);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkTable_ColorFilter::toString(SkString* str) const {
    const uint8_t* table = fStorage;
    const uint8_t* tableA = gIdentityTable;
    const uint8_t* tableR = gIdentityTable;
    const uint8_t* tableG = gIdentityTable;
    const uint8_t* tableB = gIdentityTable;
    if (fFlags & kA_Flag) {
        tableA = table; table += 256;
    }
    if (fFlags & kR_Flag) {
        tableR = table; table += 256;
    }
    if (fFlags & kG_Flag) {
        tableG = table; table += 256;
    }
    if (fFlags & kB_Flag) {
        tableB = table;
    }

    str->append("SkTable_ColorFilter (");

    for (int i = 0; i < 256; ++i) {
        str->appendf("%d: %d,%d,%d,%d\n",
                     i, tableR[i], tableG[i], tableB[i], tableA[i]);
    }

    str->append(")");
}
#endif

static const uint8_t gCountNibBits[] = {
    0, 1, 1, 2,
    1, 2, 2, 3,
    1, 2, 2, 3,
    2, 3, 3, 4
};

#include "SkPackBits.h"

void SkTable_ColorFilter::flatten(SkWriteBuffer& buffer) const {
    uint8_t storage[5*256];
    int count = gCountNibBits[fFlags & 0xF];
    size_t size = SkPackBits::Pack8(fStorage, count * 256, storage,
                                    sizeof(storage));

    buffer.write32(fFlags);
    buffer.writeByteArray(storage, size);
}

sk_sp<SkFlattenable> SkTable_ColorFilter::CreateProc(SkReadBuffer& buffer) {
    const int flags = buffer.read32();
    const size_t count = gCountNibBits[flags & 0xF];
    SkASSERT(count <= 4);

    uint8_t packedStorage[5*256];
    size_t packedSize = buffer.getArrayCount();
    if (!buffer.validate(packedSize <= sizeof(packedStorage))) {
        return nullptr;
    }
    if (!buffer.readByteArray(packedStorage, packedSize)) {
        return nullptr;
    }

    uint8_t unpackedStorage[4*256];
    size_t unpackedSize = SkPackBits::Unpack8(packedStorage, packedSize,
                              unpackedStorage, sizeof(unpackedStorage));
    // now check that we got the size we expected
    if (!buffer.validate(unpackedSize == count*256)) {
        return nullptr;
    }

    const uint8_t* a = nullptr;
    const uint8_t* r = nullptr;
    const uint8_t* g = nullptr;
    const uint8_t* b = nullptr;
    const uint8_t* ptr = unpackedStorage;

    if (flags & kA_Flag) {
        a = ptr;
        ptr += 256;
    }
    if (flags & kR_Flag) {
        r = ptr;
        ptr += 256;
    }
    if (flags & kG_Flag) {
        g = ptr;
        ptr += 256;
    }
    if (flags & kB_Flag) {
        b = ptr;
        ptr += 256;
    }
    return SkTableColorFilter::MakeARGB(a, r, g, b);
}

bool SkTable_ColorFilter::asComponentTable(SkBitmap* table) const {
    if (table) {
        if (nullptr == fBitmap) {
            SkBitmap* bmp = new SkBitmap;
            bmp->allocPixels(SkImageInfo::MakeA8(256, 4));
            uint8_t* bitmapPixels = bmp->getAddr8(0, 0);
            int offset = 0;
            static const unsigned kFlags[] = { kA_Flag, kR_Flag, kG_Flag, kB_Flag };

            for (int x = 0; x < 4; ++x) {
                if (!(fFlags & kFlags[x])) {
                    memcpy(bitmapPixels, gIdentityTable, sizeof(gIdentityTable));
                } else {
                    memcpy(bitmapPixels, fStorage + offset, 256);
                    offset += 256;
                }
                bitmapPixels += 256;
            }
            fBitmap = bmp;
        }
        *table = *fBitmap;
    }
    return true;
}

// Combines the two lookup tables so that making a lookup using res[] has
// the same effect as making a lookup through inner[] then outer[].
static void combine_tables(uint8_t res[256], const uint8_t outer[256], const uint8_t inner[256]) {
    for (int i = 0; i < 256; i++) {
        res[i] = outer[inner[i]];
    }
}

sk_sp<SkColorFilter> SkTable_ColorFilter::makeComposed(sk_sp<SkColorFilter> innerFilter) const {
    SkBitmap innerBM;
    if (!innerFilter->asComponentTable(&innerBM)) {
        return nullptr;
    }

    innerBM.lockPixels();
    if (nullptr == innerBM.getPixels()) {
        return nullptr;
    }

    const uint8_t* table = fStorage;
    const uint8_t* tableA = gIdentityTable;
    const uint8_t* tableR = gIdentityTable;
    const uint8_t* tableG = gIdentityTable;
    const uint8_t* tableB = gIdentityTable;
    if (fFlags & kA_Flag) {
        tableA = table; table += 256;
    }
    if (fFlags & kR_Flag) {
        tableR = table; table += 256;
    }
    if (fFlags & kG_Flag) {
        tableG = table; table += 256;
    }
    if (fFlags & kB_Flag) {
        tableB = table;
    }

    uint8_t concatA[256];
    uint8_t concatR[256];
    uint8_t concatG[256];
    uint8_t concatB[256];

    combine_tables(concatA, tableA, innerBM.getAddr8(0, 0));
    combine_tables(concatR, tableR, innerBM.getAddr8(0, 1));
    combine_tables(concatG, tableG, innerBM.getAddr8(0, 2));
    combine_tables(concatB, tableB, innerBM.getAddr8(0, 3));

    return SkTableColorFilter::MakeARGB(concatA, concatR, concatG, concatB);
}

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "SkGr.h"
#include "effects/GrTextureStripAtlas.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class ColorTableEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrContext* context, SkBitmap bitmap, unsigned flags);

    virtual ~ColorTableEffect();

    const char* name() const override { return "ColorTable"; }

    const GrTextureStripAtlas* atlas() const { return fAtlas; }
    int atlasRow() const { return fRow; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    ColorTableEffect(GrTexture* texture, GrTextureStripAtlas* atlas, int row, unsigned flags);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    GrTextureAccess         fTextureAccess;

    // currently not used in shader code, just to assist onComputeInvariantOutput().
    unsigned                fFlags;

    GrTextureStripAtlas*    fAtlas;
    int                     fRow;

    typedef GrFragmentProcessor INHERITED;
};

class GLColorTableEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    UniformHandle fRGBAYValuesUni;
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLColorTableEffect::onSetData(const GrGLSLProgramDataManager& pdm, const GrProcessor& proc) {
    // The textures are organized in a strip where the rows are ordered a, r, g, b.
    float rgbaYValues[4];
    const ColorTableEffect& cte = proc.cast<ColorTableEffect>();
    if (cte.atlas()) {
        SkScalar yDelta = cte.atlas()->getNormalizedTexelHeight();
        rgbaYValues[3] = cte.atlas()->getYOffset(cte.atlasRow()) + SK_ScalarHalf * yDelta;
        rgbaYValues[0] = rgbaYValues[3] + yDelta;
        rgbaYValues[1] = rgbaYValues[0] + yDelta;
        rgbaYValues[2] = rgbaYValues[1] + yDelta;
    } else {
        rgbaYValues[3] = 0.125;
        rgbaYValues[0] = 0.375;
        rgbaYValues[1] = 0.625;
        rgbaYValues[2] = 0.875;
    }
    pdm.set4fv(fRGBAYValuesUni, 1, rgbaYValues);
}

void GLColorTableEffect::emitCode(EmitArgs& args) {
    const char* yoffsets;
    fRGBAYValuesUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                       "yoffsets", &yoffsets);
    static const float kColorScaleFactor = 255.0f / 256.0f;
    static const float kColorOffsetFactor = 1.0f / 512.0f;
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    if (nullptr == args.fInputColor) {
        // the input color is solid white (all ones).
        static const float kMaxValue = kColorScaleFactor + kColorOffsetFactor;
        fragBuilder->codeAppendf("\t\tvec4 coord = vec4(%f, %f, %f, %f);\n",
                                 kMaxValue, kMaxValue, kMaxValue, kMaxValue);

    } else {
        fragBuilder->codeAppendf("\t\tfloat nonZeroAlpha = max(%s.a, .0001);\n", args.fInputColor);
        fragBuilder->codeAppendf("\t\tvec4 coord = vec4(%s.rgb / nonZeroAlpha, nonZeroAlpha);\n",
                                 args.fInputColor);
        fragBuilder->codeAppendf("\t\tcoord = coord * %f + vec4(%f, %f, %f, %f);\n",
                                 kColorScaleFactor,
                                 kColorOffsetFactor, kColorOffsetFactor,
                                 kColorOffsetFactor, kColorOffsetFactor);
    }

    SkString coord;

    fragBuilder->codeAppendf("\t\t%s.a = ", args.fOutputColor);
    coord.printf("vec2(coord.a, %s.a)", yoffsets);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], coord.c_str());
    fragBuilder->codeAppend(".a;\n");

    fragBuilder->codeAppendf("\t\t%s.r = ", args.fOutputColor);
    coord.printf("vec2(coord.r, %s.r)", yoffsets);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], coord.c_str());
    fragBuilder->codeAppend(".a;\n");

    fragBuilder->codeAppendf("\t\t%s.g = ", args.fOutputColor);
    coord.printf("vec2(coord.g, %s.g)", yoffsets);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], coord.c_str());
    fragBuilder->codeAppend(".a;\n");

    fragBuilder->codeAppendf("\t\t%s.b = ", args.fOutputColor);
    coord.printf("vec2(coord.b, %s.b)", yoffsets);
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], coord.c_str());
    fragBuilder->codeAppend(".a;\n");

    fragBuilder->codeAppendf("\t\t%s.rgb *= %s.a;\n", args.fOutputColor, args.fOutputColor);
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<GrFragmentProcessor> ColorTableEffect::Make(GrContext* context, SkBitmap bitmap,
                                                  unsigned flags) {

    GrTextureStripAtlas::Desc desc;
    desc.fWidth  = bitmap.width();
    desc.fHeight = 128;
    desc.fRowHeight = bitmap.height();
    desc.fContext = context;
    desc.fConfig = SkImageInfo2GrPixelConfig(bitmap.info(), *context->caps());
    GrTextureStripAtlas* atlas = GrTextureStripAtlas::GetAtlas(desc);
    int row = atlas->lockRow(bitmap);
    SkAutoTUnref<GrTexture> texture;
    if (-1 == row) {
        atlas = nullptr;
        texture.reset(GrRefCachedBitmapTexture(context, bitmap, GrTextureParams::ClampNoFilter(),
                                               SkSourceGammaTreatment::kRespect));
    } else {
        texture.reset(SkRef(atlas->getTexture()));
    }

    return sk_sp<GrFragmentProcessor>(new ColorTableEffect(texture, atlas, row, flags));
}

ColorTableEffect::ColorTableEffect(GrTexture* texture, GrTextureStripAtlas* atlas, int row,
                                   unsigned flags)
    : fTextureAccess(texture)
    , fFlags(flags)
    , fAtlas(atlas)
    , fRow(row) {
    this->initClassID<ColorTableEffect>();
    this->addTextureAccess(&fTextureAccess);
}

ColorTableEffect::~ColorTableEffect() {
    if (fAtlas) {
        fAtlas->unlockRow(fRow);
    }
}

void ColorTableEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    GLColorTableEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* ColorTableEffect::onCreateGLSLInstance() const {
    return new GLColorTableEffect;
}

bool ColorTableEffect::onIsEqual(const GrFragmentProcessor& other) const {
    // For non-atlased instances, the texture (compared by base class) is sufficient to
    // differentiate different tables. For atlased instances we ensure they are using the
    // same row.
    const ColorTableEffect& that = other.cast<ColorTableEffect>();
    SkASSERT(SkToBool(fAtlas) == SkToBool(that.fAtlas));
    // Ok to always do this comparison since both would be -1 if non-atlased.
    return fRow == that.fRow;
}

void ColorTableEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    // If we kept the table in the effect then we could actually run known inputs through the
    // table.
    GrColorComponentFlags invalidateFlags = kNone_GrColorComponentFlags;
    if (fFlags & SkTable_ColorFilter::kR_Flag) {
        invalidateFlags |= kR_GrColorComponentFlag;
    }
    if (fFlags & SkTable_ColorFilter::kG_Flag) {
        invalidateFlags |= kG_GrColorComponentFlag;
    }
    if (fFlags & SkTable_ColorFilter::kB_Flag) {
        invalidateFlags |= kB_GrColorComponentFlag;
    }
    if (fFlags & SkTable_ColorFilter::kA_Flag) {
        invalidateFlags |= kA_GrColorComponentFlag;
    }
    inout->invalidateComponents(invalidateFlags, GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorTableEffect);

sk_sp<GrFragmentProcessor> ColorTableEffect::TestCreate(GrProcessorTestData* d) {
    int flags = 0;
    uint8_t luts[256][4];
    do {
        for (int i = 0; i < 4; ++i) {
            flags |= d->fRandom->nextBool() ? (1  << i): 0;
        }
    } while (!flags);
    for (int i = 0; i < 4; ++i) {
        if (flags & (1 << i)) {
            for (int j = 0; j < 256; ++j) {
                luts[j][i] = SkToU8(d->fRandom->nextBits(8));
            }
        }
    }
    auto filter(SkTableColorFilter::MakeARGB(
        (flags & (1 << 0)) ? luts[0] : nullptr,
        (flags & (1 << 1)) ? luts[1] : nullptr,
        (flags & (1 << 2)) ? luts[2] : nullptr,
        (flags & (1 << 3)) ? luts[3] : nullptr
    ));

    sk_sp<GrFragmentProcessor> fp = filter->asFragmentProcessor(d->fContext);
    SkASSERT(fp);
    return fp;
}

sk_sp<GrFragmentProcessor> SkTable_ColorFilter::asFragmentProcessor(GrContext* context) const {
    SkBitmap bitmap;
    this->asComponentTable(&bitmap);

    return ColorTableEffect::Make(context, bitmap, fFlags);
}

#endif // SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_CPU_BENDIAN
#else
    #define SK_A32_INDEX    (3 - (SK_A32_SHIFT >> 3))
    #define SK_R32_INDEX    (3 - (SK_R32_SHIFT >> 3))
    #define SK_G32_INDEX    (3 - (SK_G32_SHIFT >> 3))
    #define SK_B32_INDEX    (3 - (SK_B32_SHIFT >> 3))
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkTableColorFilter::Make(const uint8_t table[256]) {
    return sk_make_sp<SkTable_ColorFilter>(table, table, table, table);
}

sk_sp<SkColorFilter> SkTableColorFilter::MakeARGB(const uint8_t tableA[256],
                                                  const uint8_t tableR[256],
                                                  const uint8_t tableG[256],
                                                  const uint8_t tableB[256]) {
    return sk_make_sp<SkTable_ColorFilter>(tableA, tableR, tableG, tableB);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkTableColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTable_ColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
