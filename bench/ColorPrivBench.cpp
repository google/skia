#include "SkBenchmark.h"
#include "SkColorPriv.h"
#include "SkRandom.h"
#include "SkString.h"

template <bool kFast, bool kScale>
class FourByteInterpBench : public SkBenchmark {
public:
    FourByteInterpBench() {
        fName.set("four_byte_interp");
        fName.append(kFast ? "_fast" : "_slow");
        fName.append(kScale ? "_255" : "_256");

        // We'll exhaustively test all scales instead of using random numbers.
        for (int i = 0; i <= 256; i++) {
            fScales[i] = i;
        }
        if (kScale) fScales[256] = 255;  // We'll just do 255 twice if we're limited to [0,255].
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual const char* onGetName() SK_OVERRIDE { return fName.c_str(); }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        const SkPMColor src = 0xAB998877, dst = 0x66334455;
        volatile SkPMColor junk = 0;
        for (int i = 0; i < 10*loops; ++i) {
            for (size_t j = 0; j <= 256; j++) {
                const unsigned scale = fScales[j];
                if (kFast && kScale) {
                    junk ^= SkFastFourByteInterp(src, dst, scale);
                } else if (kFast) {
                    junk ^= SkFastFourByteInterp256(src, dst, scale);
                } else if (kScale) {
                    junk ^= SkFourByteInterp(src, dst, scale);
                } else {
                    junk ^= SkFourByteInterp256(src, dst, scale);
                }
            }
        }
    }

private:
    SkString fName;
    unsigned fScales[257];  // We need space for [0, 256].
};

#define COMMA ,
DEF_BENCH( return SkNEW(FourByteInterpBench<true COMMA true>); )
DEF_BENCH( return SkNEW(FourByteInterpBench<true COMMA false>); )
DEF_BENCH( return SkNEW(FourByteInterpBench<false COMMA true>); )
DEF_BENCH( return SkNEW(FourByteInterpBench<false COMMA false>); )
#undef COMMA
