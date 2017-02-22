/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformPriv.h"
#include "SkEndian.h"
#include "SkFixed.h"
#include "SkICC.h"
#include "SkICCPriv.h"

const uint16_t redarray[] = {
0,
3,
6,
9,
12,
15,
18,
21,
24,
27,
31,
36,
43,
52,
62,
74,
88,
104,
121,
140,
161,
184,
209,
235,
264,
295,
327,
361,
398,
436,
477,
519,
563,
610,
658,
708,
761,
815,
872,
930,
991,
1053,
1118,
1184,
1253,
1324,
1397,
1471,
1548,
1626,
1707,
1789,
1873,
1960,
2048,
2138,
2230,
2324,
2420,
2517,
2617,
2718,
2821,
2926,
3033,
3142,
3252,
3364,
3478,
3594,
3711,
3831,
3951,
4074,
4197,
4323,
4450,
4578,
4709,
4840,
4973,
5108,
5244,
5381,
5520,
5660,
5802,
5945,
6090,
6236,
6385,
6534,
6685,
6839,
6994,
7152,
7311,
7473,
7638,
7805,
7974,
8146,
8320,
8497,
8676,
8858,
9042,
9229,
9418,
9611,
9806,
10004,
10206,
10410,
10618,
10828,
11042,
11260,
11481,
11704,
11931,
12161,
12393,
12628,
12866,
13106,
13349,
13594,
13842,
14093,
14345,
14599,
14855,
15113,
15373,
15634,
15897,
16162,
16430,
16698,
16967,
17238,
17511,
17785,
18061,
18339,
18619,
18903,
19189,
19475,
19764,
20056,
20352,
20651,
20954,
21260,
21572,
21887,
22206,
22530,
22859,
23193,
23532,
23877,
24226,
24579,
24935,
25295,
25659,
26027,
26398,
26771,
27141,
27511,
27879,
28247,
28615,
28984,
29354,
29723,
30092,
30462,
30833,
31206,
31581,
31956,
32332,
32710,
33090,
33474,
33864,
34258,
34657,
35060,
35468,
35880,
36296,
36716,
37138,
37565,
37994,
38425,
38859,
39296,
39734,
40174,
40615,
41057,
41500,
41946,
42394,
42847,
43304,
43765,
44229,
44693,
45158,
45624,
46092,
46565,
47045,
47532,
48027,
48528,
49034,
49546,
50063,
50584,
51112,
51646,
52189,
52737,
53283,
53826,
54371,
54916,
55461,
56007,
56553,
57097,
57638,
58176,
58712,
59244,
59777,
60308,
60837,
61365,
61892,
62417,
62941,
63462,
63983,
64502,
65019,
65535,
};
const uint16_t greenarray[] = {
0,
2,
5,
7,
9,
12,
15,
19,
24,
31,
38,
48,
58,
70,
84,
99,
115,
133,
153,
174,
197,
221,
247,
274,
304,
334,
367,
401,
437,
475,
514,
555,
598,
643,
690,
738,
788,
840,
894,
950,
1007,
1066,
1128,
1191,
1256,
1323,
1392,
1463,
1536,
1611,
1688,
1766,
1847,
1930,
2015,
2102,
2192,
2283,
2376,
2471,
2568,
2668,
2769,
2873,
2979,
3086,
3196,
3309,
3423,
3540,
3658,
3779,
3902,
4028,
4156,
4285,
4418,
4552,
4689,
4828,
4969,
5113,
5259,
5408,
5558,
5712,
5867,
6024,
6184,
6346,
6510,
6678,
6846,
7017,
7190,
7366,
7543,
7722,
7903,
8086,
8270,
8456,
8644,
8833,
9023,
9215,
9408,
9602,
9799,
9998,
10198,
10401,
10605,
10811,
11021,
11231,
11442,
11655,
11870,
12087,
12305,
12525,
12747,
12970,
13196,
13423,
13654,
13887,
14122,
14360,
14601,
14845,
15092,
15344,
15599,
15857,
16119,
16384,
16653,
16925,
17201,
17481,
17764,
18050,
18340,
18633,
18929,
19226,
19526,
19829,
20132,
20438,
20746,
21056,
21368,
21682,
21998,
22317,
22639,
22963,
23290,
23620,
23953,
24290,
24629,
24971,
25316,
25662,
26012,
26363,
26715,
27067,
27419,
27771,
28122,
28474,
28827,
29181,
29537,
29896,
30258,
30623,
30990,
31359,
31731,
32106,
32483,
32863,
33245,
33633,
34025,
34421,
34822,
35229,
35639,
36056,
36477,
36904,
37334,
37767,
38200,
38633,
39066,
39499,
39933,
40370,
40809,
41250,
41694,
42136,
42579,
43021,
43463,
43904,
44349,
44798,
45251,
45709,
46173,
46640,
47114,
47596,
48088,
48590,
49103,
49625,
50157,
50701,
51254,
51814,
52380,
52950,
53521,
54093,
54665,
55234,
55801,
56365,
56923,
57475,
58017,
58550,
59079,
59601,
60120,
60634,
61144,
61649,
62150,
62646,
63138,
63627,
64110,
64590,
65064,
65535,
};
const uint16_t bluearray[] = {
0,
2,
3,
5,
7,
10,
14,
19,
25,
32,
40,
49,
60,
71,
84,
99,
114,
131,
149,
169,
190,
212,
236,
262,
289,
317,
347,
378,
411,
446,
482,
520,
559,
600,
643,
688,
734,
782,
831,
882,
936,
990,
1047,
1106,
1166,
1228,
1292,
1358,
1426,
1496,
1567,
1641,
1717,
1794,
1874,
1955,
2039,
2125,
2213,
2303,
2395,
2489,
2585,
2684,
2784,
2887,
2992,
3100,
3210,
3322,
3436,
3553,
3672,
3793,
3917,
4043,
4172,
4303,
4437,
4573,
4712,
4853,
4997,
5143,
5292,
5444,
5600,
5756,
5914,
6074,
6236,
6400,
6566,
6733,
6903,
7074,
7248,
7424,
7601,
7781,
7962,
8145,
8329,
8515,
8702,
8890,
9080,
9271,
9462,
9655,
9850,
10045,
10242,
10440,
10639,
10840,
11042,
11246,
11452,
11662,
11874,
12088,
12306,
12526,
12750,
12976,
13205,
13437,
13671,
13908,
14148,
14391,
14636,
14885,
15137,
15392,
15651,
15914,
16180,
16450,
16722,
16996,
17272,
17550,
17830,
18112,
18395,
18681,
18967,
19255,
19544,
19835,
20127,
20421,
20718,
21016,
21317,
21620,
21924,
22233,
22546,
22864,
23187,
23515,
23848,
24187,
24531,
24881,
25235,
25593,
25956,
26323,
26694,
27067,
27444,
27819,
28193,
28565,
28936,
29306,
29675,
30045,
30414,
30782,
31150,
31518,
31884,
32250,
32615,
32981,
33354,
33734,
34122,
34518,
34922,
35335,
35756,
36188,
36628,
37078,
37536,
38003,
38478,
38955,
39433,
39911,
40390,
40868,
41346,
41822,
42293,
42761,
43225,
43685,
44145,
44608,
45074,
45544,
46017,
46495,
46979,
47471,
47969,
48476,
48992,
49516,
50043,
50572,
51105,
51639,
52173,
52708,
53239,
53767,
54290,
54808,
55329,
55855,
56385,
56919,
57455,
57992,
58529,
59069,
59612,
60163,
60713,
61260,
61804,
62345,
62884,
63420,
63953,
64483,
65010,
65535,
};

SkICC::SkICC(sk_sp<SkColorSpace> colorSpace)
    : fColorSpace(std::move(colorSpace))
{}

sk_sp<SkICC> SkICC::Make(const void* ptr, size_t len) {
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(ptr, len);
    if (!colorSpace) {
        return nullptr;
    }

    return sk_sp<SkICC>(new SkICC(std::move(colorSpace)));
}

bool SkICC::toXYZD50(SkMatrix44* toXYZD50) const {
    const SkMatrix44* m = as_CSB(fColorSpace)->toXYZD50();
    if (!m) {
        return false;
    }

    *toXYZD50 = *m;
    return true;
}

bool SkICC::isNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    return as_CSB(fColorSpace)->onIsNumericalTransferFn(coeffs);
}

static const int kDefaultTableSize = 512; // Arbitrary

void fn_to_table(float* tablePtr, const SkColorSpaceTransferFn& fn) {
    // Y = (aX + b)^g + e  for X >= d
    // Y = cX + f          otherwise
    for (int i = 0; i < kDefaultTableSize; i++) {
        float x = ((float) i) / ((float) (kDefaultTableSize - 1));
        if (x >= fn.fD) {
            tablePtr[i] = clamp_0_1(powf(fn.fA * x + fn.fB, fn.fG) + fn.fE);
        } else {
            tablePtr[i] = clamp_0_1(fn.fC * x + fn.fF);
        }
    }
}

void copy_to_table(float* tablePtr, const SkGammas* gammas, int index) {
    SkASSERT(gammas->isTable(index));
    const float* ptr = gammas->table(index);
    const size_t bytes = gammas->tableSize(index) * sizeof(float);
    memcpy(tablePtr, ptr, bytes);
}

bool SkICC::rawTransferFnData(Tables* tables) const {
    if (SkColorSpace_Base::Type::kA2B == as_CSB(fColorSpace)->type()) {
        return false;
    }
    SkColorSpace_XYZ* colorSpace = (SkColorSpace_XYZ*) fColorSpace.get();

    SkColorSpaceTransferFn fn;
    if (this->isNumericalTransferFn(&fn)) {
        tables->fStorage = SkData::MakeUninitialized(kDefaultTableSize * sizeof(float));
        fn_to_table((float*) tables->fStorage->writable_data(), fn);
        tables->fRed.fOffset = tables->fGreen.fOffset = tables->fBlue.fOffset = 0;
        tables->fRed.fCount = tables->fGreen.fCount = tables->fBlue.fCount = kDefaultTableSize;
        return true;
    }

    const SkGammas* gammas = colorSpace->gammas();
    SkASSERT(gammas);
    if (gammas->data(0) == gammas->data(1) && gammas->data(0) == gammas->data(2)) {
        SkASSERT(gammas->isTable(0));
        tables->fStorage = SkData::MakeUninitialized(gammas->tableSize(0) * sizeof(float));
        copy_to_table((float*) tables->fStorage->writable_data(), gammas, 0);
        tables->fRed.fOffset = tables->fGreen.fOffset = tables->fBlue.fOffset = 0;
        tables->fRed.fCount = tables->fGreen.fCount = tables->fBlue.fCount = gammas->tableSize(0);
        return true;
    }

    // Determine the storage size.
    size_t storageSize = 0;
    for (int i = 0; i < 3; i++) {
        if (gammas->isTable(i)) {
            storageSize += gammas->tableSize(i) * sizeof(float);
        } else {
            storageSize += kDefaultTableSize * sizeof(float);
        }
    }

    // Fill in the tables.
    tables->fStorage = SkData::MakeUninitialized(storageSize);
    float* ptr = (float*) tables->fStorage->writable_data();
    size_t offset = 0;
    Channel rgb[3];
    for (int i = 0; i < 3; i++) {
        if (gammas->isTable(i)) {
            copy_to_table(ptr, gammas, i);
            rgb[i].fOffset = offset;
            rgb[i].fCount = gammas->tableSize(i);
            offset += rgb[i].fCount * sizeof(float);
            ptr += rgb[i].fCount;
            continue;
        }

        if (gammas->isNamed(i)) {
            SkAssertResult(named_to_parametric(&fn, gammas->data(i).fNamed));
        } else if (gammas->isValue(i)) {
            value_to_parametric(&fn, gammas->data(i).fValue);
        } else {
            SkASSERT(gammas->isParametric(i));
            fn = gammas->params(i);
        }

        fn_to_table(ptr, fn);
        rgb[i].fOffset = offset;
        rgb[i].fCount = kDefaultTableSize;
        offset += kDefaultTableSize * sizeof(float);
        ptr += kDefaultTableSize;
    }

    tables->fRed = rgb[0];
    tables->fGreen = rgb[1];
    tables->fBlue = rgb[2];
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Google Skia (UTF-16)
static constexpr uint8_t kDescriptionTagBody[] = {
        0x00, 0x47, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x20, 0x00,
        0x53, 0x00, 0x6b, 0x00, 0x69, 0x00, 0x61, 0x00, 0x20,
    };
static_assert(SkIsAlign4(sizeof(kDescriptionTagBody)), "Description must be aligned to 4-bytes.");
static constexpr uint32_t kDescriptionTagHeader[7] {
    SkEndian_SwapBE32(kTAG_TextType),                        // Type signature
    0,                                                       // Reserved
    SkEndian_SwapBE32(1),                                    // Number of records
    SkEndian_SwapBE32(12),                                   // Record size (must be 12)
    SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')), // English USA
    SkEndian_SwapBE32(sizeof(kDescriptionTagBody)),          // Length of string
    SkEndian_SwapBE32(28),                                   // Offset of string
};

static constexpr uint32_t kWhitePointTag[5] {
    SkEndian_SwapBE32(kXYZ_PCSSpace),
    0,
    SkEndian_SwapBE32(0x0000f6d6), // X = 0.96420 (D50)
    SkEndian_SwapBE32(0x00010000), // Y = 1.00000 (D50)
    SkEndian_SwapBE32(0x0000d32d), // Z = 0.82491 (D50)
};

// Google Inc. 2016 (UTF-16)
static constexpr uint8_t kCopyrightTagBody[] = {
        0x00, 0x47, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x20, 0x00,
        0x49, 0x00, 0x6e, 0x00, 0x63, 0x00, 0x2e, 0x00, 0x20, 0x00, 0x32, 0x00, 0x30, 0x00, 0x31,
        0x00, 0x36,
};
static_assert(SkIsAlign4(sizeof(kCopyrightTagBody)), "Copyright must be aligned to 4-bytes.");
static constexpr uint32_t kCopyrightTagHeader[7] {
    SkEndian_SwapBE32(kTAG_TextType),                        // Type signature
    0,                                                       // Reserved
    SkEndian_SwapBE32(1),                                    // Number of records
    SkEndian_SwapBE32(12),                                   // Record size (must be 12)
    SkEndian_SwapBE32(SkSetFourByteTag('e', 'n', 'U', 'S')), // English USA
    SkEndian_SwapBE32(sizeof(kCopyrightTagBody)),            // Length of string
    SkEndian_SwapBE32(28),                                   // Offset of string
};

// We will write a profile with the minimum nine required tags.
static constexpr uint32_t kICCNumEntries = 9;

static constexpr uint32_t kTAG_desc = SkSetFourByteTag('d', 'e', 's', 'c');
static constexpr uint32_t kTAG_desc_Bytes = sizeof(kDescriptionTagHeader) +
                                            sizeof(kDescriptionTagBody);
static constexpr uint32_t kTAG_desc_Offset = kICCHeaderSize +
                                             kICCNumEntries * kICCTagTableEntrySize;

static constexpr uint32_t kTAG_XYZ_Bytes = 20;
static constexpr uint32_t kTAG_rXYZ_Offset = kTAG_desc_Offset + kTAG_desc_Bytes;
static constexpr uint32_t kTAG_gXYZ_Offset = kTAG_rXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_bXYZ_Offset = kTAG_gXYZ_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kTAG_TRC_Bytes = 256 * 2 + 12;
static constexpr uint32_t kTAG_rTRC_Offset = kTAG_bXYZ_Offset + kTAG_XYZ_Bytes;
static constexpr uint32_t kTAG_gTRC_Offset = kTAG_rTRC_Offset + kTAG_TRC_Bytes;
static constexpr uint32_t kTAG_bTRC_Offset = kTAG_gTRC_Offset + kTAG_TRC_Bytes;

static constexpr uint32_t kTAG_wtpt = SkSetFourByteTag('w', 't', 'p', 't');
static constexpr uint32_t kTAG_wtpt_Offset = kTAG_bTRC_Offset + kTAG_TRC_Bytes;

static constexpr uint32_t kTAG_cprt = SkSetFourByteTag('c', 'p', 'r', 't');
static constexpr uint32_t kTAG_cprt_Bytes = sizeof(kCopyrightTagHeader) +
                                            sizeof(kCopyrightTagBody);
static constexpr uint32_t kTAG_cprt_Offset = kTAG_wtpt_Offset + kTAG_XYZ_Bytes;

static constexpr uint32_t kICCProfileSize = kTAG_cprt_Offset + kTAG_cprt_Bytes;

static constexpr uint32_t kICCHeader[kICCHeaderSize / 4] {
    SkEndian_SwapBE32(kICCProfileSize),  // Size of the profile
    0,                                   // Preferred CMM type (ignored)
    SkEndian_SwapBE32(0x02100000),       // Version 2.1
    SkEndian_SwapBE32(kDisplay_Profile), // Display device profile
    SkEndian_SwapBE32(kRGB_ColorSpace),  // RGB input color space
    SkEndian_SwapBE32(kXYZ_PCSSpace),    // XYZ profile connection space
    0, 0, 0,                             // Date and time (ignored)
    SkEndian_SwapBE32(kACSP_Signature),  // Profile signature
    0,                                   // Platform target (ignored)
    0x00000000,                          // Flags: not embedded, can be used independently
    0,                                   // Device manufacturer (ignored)
    0,                                   // Device model (ignored)
    0, 0,                                // Device attributes (ignored)
    SkEndian_SwapBE32(1),                // Relative colorimetric rendering intent
    SkEndian_SwapBE32(0x0000f6d6),       // D50 standard illuminant (X)
    SkEndian_SwapBE32(0x00010000),       // D50 standard illuminant (Y)
    SkEndian_SwapBE32(0x0000d32d),       // D50 standard illuminant (Z)
    0,                                   // Profile creator (ignored)
    0, 0, 0, 0,                          // Profile id checksum (ignored)
    0, 0, 0, 0, 0, 0, 0,                 // Reserved (ignored)
    SkEndian_SwapBE32(kICCNumEntries),   // Number of tags
};

static constexpr uint32_t kICCTagTable[3 * kICCNumEntries] {
    // Profile description
    SkEndian_SwapBE32(kTAG_desc),
    SkEndian_SwapBE32(kTAG_desc_Offset),
    SkEndian_SwapBE32(kTAG_desc_Bytes),

    // rXYZ
    SkEndian_SwapBE32(kTAG_rXYZ),
    SkEndian_SwapBE32(kTAG_rXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // gXYZ
    SkEndian_SwapBE32(kTAG_gXYZ),
    SkEndian_SwapBE32(kTAG_gXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // bXYZ
    SkEndian_SwapBE32(kTAG_bXYZ),
    SkEndian_SwapBE32(kTAG_bXYZ_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // rTRC
    SkEndian_SwapBE32(kTAG_rTRC),
    SkEndian_SwapBE32(kTAG_rTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // gTRC
    SkEndian_SwapBE32(kTAG_gTRC),
    SkEndian_SwapBE32(kTAG_gTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // bTRC
    SkEndian_SwapBE32(kTAG_bTRC),
    SkEndian_SwapBE32(kTAG_bTRC_Offset),
    SkEndian_SwapBE32(kTAG_TRC_Bytes),

    // White point
    SkEndian_SwapBE32(kTAG_wtpt),
    SkEndian_SwapBE32(kTAG_wtpt_Offset),
    SkEndian_SwapBE32(kTAG_XYZ_Bytes),

    // Copyright
    SkEndian_SwapBE32(kTAG_cprt),
    SkEndian_SwapBE32(kTAG_cprt_Offset),
    SkEndian_SwapBE32(kTAG_cprt_Bytes),
};

static void write_xyz_tag(uint32_t* ptr, const SkMatrix44& toXYZ, int col) {
    ptr[0] = SkEndian_SwapBE32(kXYZ_PCSSpace);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(0, col)));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(1, col)));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(toXYZ.getFloat(2, col)));
}

static void write_trc_tag(uint32_t* ptr, int color) {
    /*ptr[0] = SkEndian_SwapBE32(kTAG_ParaCurveType);
    ptr[1] = 0;
    ptr[2] = (uint32_t) (SkEndian_SwapBE16(kGABCDEF_ParaCurveType));
    ptr[3] = SkEndian_SwapBE32(SkFloatToFixed(fn.fG));
    ptr[4] = SkEndian_SwapBE32(SkFloatToFixed(fn.fA));
    ptr[5] = SkEndian_SwapBE32(SkFloatToFixed(fn.fB));
    ptr[6] = SkEndian_SwapBE32(SkFloatToFixed(fn.fC));
    ptr[7] = SkEndian_SwapBE32(SkFloatToFixed(fn.fD));
    ptr[8] = SkEndian_SwapBE32(SkFloatToFixed(fn.fE));
    ptr[9] = SkEndian_SwapBE32(SkFloatToFixed(fn.fF));*/

    ptr[0] = SkEndian_SwapBE32(kTAG_CurveType);
    ptr[1] = 0;
    ptr[2] = SkEndian_SwapBE32(256);
    ptr += 3;

    auto ptr16 = (uint16_t*) ptr;
    for (int i = 0; i < 256; i++) {
        if (0 == color) {
            ptr16[i] = SkEndian_SwapBE16(redarray[i]);
        } else if (1 == color) {
            ptr16[i] = SkEndian_SwapBE16(greenarray[i]);
        } else {
            ptr16[i] = SkEndian_SwapBE16(bluearray[i]);
        }
    }
}

static bool is_3x3(const SkMatrix44& toXYZD50) {
    return 0.0f == toXYZD50.get(3, 0) && 0.0f == toXYZD50.get(3, 1) && 0.0f == toXYZD50.get(3, 2) &&
           0.0f == toXYZD50.get(0, 3) && 0.0f == toXYZD50.get(1, 3) && 0.0f == toXYZD50.get(2, 3) &&
           1.0f == toXYZD50.get(3, 3);
}

sk_sp<SkData> SkICC::WriteToICC(const SkColorSpaceTransferFn& fn, const SkMatrix44& toXYZD50) {
    if (false && (!is_3x3(toXYZD50) || !is_valid_transfer_fn(fn))) {
        return nullptr;
    }

    SkAutoMalloc profile(kICCProfileSize);
    uint8_t* ptr = (uint8_t*) profile.get();

    // Write profile header
    memcpy(ptr, kICCHeader, sizeof(kICCHeader));
    ptr += sizeof(kICCHeader);

    // Write tag table
    memcpy(ptr, kICCTagTable, sizeof(kICCTagTable));
    ptr += sizeof(kICCTagTable);

    // Write profile description tag
    memcpy(ptr, kDescriptionTagHeader, sizeof(kDescriptionTagHeader));
    ptr += sizeof(kDescriptionTagHeader);
    memcpy(ptr, kDescriptionTagBody, sizeof(kDescriptionTagBody));
    ptr += sizeof(kDescriptionTagBody);

    // Write XYZ tags
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 0);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 1);
    ptr += kTAG_XYZ_Bytes;
    write_xyz_tag((uint32_t*) ptr, toXYZD50, 2);
    ptr += kTAG_XYZ_Bytes;

    // Write TRC tag
    write_trc_tag((uint32_t*) ptr, 0);
    ptr += kTAG_TRC_Bytes;
    write_trc_tag((uint32_t*) ptr, 1);
    ptr += kTAG_TRC_Bytes;
    write_trc_tag((uint32_t*) ptr, 2);
    ptr += kTAG_TRC_Bytes;

    // Write white point tag (must be D50)
    memcpy(ptr, kWhitePointTag, sizeof(kWhitePointTag));
    ptr += sizeof(kWhitePointTag);

    // Write copyright tag
    memcpy(ptr, kCopyrightTagHeader, sizeof(kCopyrightTagHeader));
    ptr += sizeof(kCopyrightTagHeader);
    memcpy(ptr, kCopyrightTagBody, sizeof(kCopyrightTagBody));
    ptr += sizeof(kCopyrightTagBody);

    SkASSERT(kICCProfileSize == ptr - (uint8_t*) profile.get());
    return SkData::MakeFromMalloc(profile.release(), kICCProfileSize);
}
