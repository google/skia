#include "SkColorPriv.h"
#include "SkTableColorFilter.h"
#include "SkUnPreMultiply.h"

class SkTable_ColorFilter : public SkColorFilter {
public:
    SkTable_ColorFilter(const uint8_t tableA[], const uint8_t tableR[],
                        const uint8_t tableG[], const uint8_t tableB[]) {
        fBitmap = NULL;
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

    virtual bool asComponentTable(SkBitmap* table) SK_OVERRIDE;

    virtual void filterSpan(const SkPMColor src[], int count,
                            SkPMColor dst[]) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTable_ColorFilter)

protected:
    SkTable_ColorFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkBitmap* fBitmap;

    enum {
        kA_Flag = 1 << 0,
        kR_Flag = 1 << 1,
        kG_Flag = 1 << 2,
        kB_Flag = 1 << 3,
    };
    uint8_t fStorage[256 * 4];
    unsigned fFlags;

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

void SkTable_ColorFilter::filterSpan(const SkPMColor src[], int count,
                                     SkPMColor dst[]) {
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

static const uint8_t gCountNibBits[] = {
    0, 1, 1, 2,
    1, 2, 2, 3,
    1, 2, 2, 3,
    2, 3, 3, 4
};

#include "SkPackBits.h"

void SkTable_ColorFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    uint8_t storage[5*256];
    int count = gCountNibBits[fFlags & 0xF];
    size_t size = SkPackBits::Pack8(fStorage, count * 256, storage);
    SkASSERT(size <= sizeof(storage));

//    SkDebugf("raw %d packed %d\n", count * 256, size);
    
    buffer.writeInt(fFlags);
    buffer.writeInt(size);
    buffer.write(storage, size);
}

SkTable_ColorFilter::SkTable_ColorFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fBitmap = NULL;

    uint8_t storage[5*256];

    fFlags = buffer.readInt();
    size_t size = buffer.readInt();
    buffer.read(storage, size);

    size_t raw = SkPackBits::Unpack8(storage, size, fStorage);

    SkASSERT(raw <= sizeof(fStorage));
    size_t count = gCountNibBits[fFlags & 0xF];
    SkASSERT(raw == count * 256);
}

bool SkTable_ColorFilter::asComponentTable(SkBitmap* table) {
    if (table) {
        if (NULL == fBitmap) {
            fBitmap = new SkBitmap;
            fBitmap->setConfig(SkBitmap::kA8_Config, 256, 4, 256);
            fBitmap->allocPixels();
            memcpy(fBitmap->getAddr8(0, 0), fStorage, 256 * 4);
        }
        *table = *fBitmap;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_CPU_BENDIAN
#else
    #define SK_A32_INDEX    (3 - (SK_A32_SHIFT >> 3))
    #define SK_R32_INDEX    (3 - (SK_R32_SHIFT >> 3))
    #define SK_G32_INDEX    (3 - (SK_G32_SHIFT >> 3))
    #define SK_B32_INDEX    (3 - (SK_B32_SHIFT >> 3))
#endif

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkTableColorFilter::Create(const uint8_t table[256]) {
    return SkNEW_ARGS(SkTable_ColorFilter, (table, table, table, table));
}

SkColorFilter* SkTableColorFilter::CreateARGB(const uint8_t tableA[256],
                                              const uint8_t tableR[256],
                                              const uint8_t tableG[256],
                                              const uint8_t tableB[256]) {
    return SkNEW_ARGS(SkTable_ColorFilter, (tableA, tableR, tableG, tableB));
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkTableColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTable_ColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
