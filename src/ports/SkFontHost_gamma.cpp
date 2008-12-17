#include "SkFontHost.h"
#include <math.h>

// define this to use pre-compiled tables for gamma. This is slightly faster,
// and doesn't create any RW global memory, but means we cannot change the
// gamma at runtime.
#define USE_PREDEFINED_GAMMA_TABLES

#ifndef USE_PREDEFINED_GAMMA_TABLES
    // define this if you want to spew out the "C" code for the tables, given
    // the current values for SK_BLACK_GAMMA and SK_WHITE_GAMMA.
    #define DUMP_GAMMA_TABLESx
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_PREDEFINED_GAMMA_TABLES

#include "sk_predefined_gamma.h"

#else   // use writable globals for gamma tables

static bool gGammaIsBuilt;
static uint8_t gBlackGamma[256], gWhiteGamma[256];

#define SK_BLACK_GAMMA     (1.4f)
#define SK_WHITE_GAMMA     (1/1.4f)

static void build_power_table(uint8_t table[], float ee)
{
    //    printf("------ build_power_table %g\n", ee);
    for (int i = 0; i < 256; i++)
    {
        float x = i / 255.f;
        //   printf(" %d %g", i, x);
        x = powf(x, ee);
        //   printf(" %g", x);
        int xx = SkScalarRound(SkFloatToScalar(x * 255));
        //   printf(" %d\n", xx);
        table[i] = SkToU8(xx);
    }
}

#ifdef DUMP_GAMMA_TABLES

#include "SkString.h"

static void dump_a_table(const char name[], const uint8_t table[],
                         float gamma) {
    SkDebugf("\n");
    SkDebugf("\/\/ Gamma table for %g\n", gamma);
    SkDebugf("static const uint8_t %s[] = {\n", name);
    for (int y = 0; y < 16; y++) {
        SkString line, tmp;
        for (int x = 0; x < 16; x++) {
            tmp.printf("0x%02X, ", *table++);
            line.append(tmp);
        }
        SkDebugf("    %s\n", line.c_str());
    }
    SkDebugf("};\n");
}

#endif

#endif

///////////////////////////////////////////////////////////////////////////////

void SkFontHost::GetGammaTables(const uint8_t* tables[2])
{
#ifndef USE_PREDEFINED_GAMMA_TABLES
    if (!gGammaIsBuilt)
    {
        build_power_table(gBlackGamma, SK_BLACK_GAMMA);
        build_power_table(gWhiteGamma, SK_WHITE_GAMMA);
        gGammaIsBuilt = true;

#ifdef DUMP_GAMMA_TABLES
        dump_a_table("gBlackGamma", gBlackGamma, SK_BLACK_GAMMA);
        dump_a_table("gWhiteGamma", gWhiteGamma, SK_WHITE_GAMMA);
#endif
    }
#endif
    tables[0] = gBlackGamma;
    tables[1] = gWhiteGamma;
}

// If the luminance is <= this value, then apply the black gamma table
#define BLACK_GAMMA_THRESHOLD   0x40

// If the luminance is >= this value, then apply the white gamma table
#define WHITE_GAMMA_THRESHOLD   0xC0

int SkFontHost::ComputeGammaFlag(const SkPaint& paint)
{
    if (paint.getShader() == NULL)
    {
        SkColor c = paint.getColor();
        int r = SkColorGetR(c);
        int g = SkColorGetG(c);
        int b = SkColorGetB(c);
        int luminance = (r * 2 + g * 5 + b) >> 3;
        
        if (luminance <= BLACK_GAMMA_THRESHOLD)
        {
        //    printf("------ black gamma for [%d %d %d]\n", r, g, b);
            return SkScalerContext::kGammaForBlack_Flag;
        }
        if (luminance >= WHITE_GAMMA_THRESHOLD)
        {
        //    printf("------ white gamma for [%d %d %d]\n", r, g, b);
            return SkScalerContext::kGammaForWhite_Flag;
        }
    }
    return 0;
}

