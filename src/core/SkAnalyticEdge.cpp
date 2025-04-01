/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkAnalyticEdge.h"

#include "include/core/SkPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkFDot6.h"

#include <algorithm>
#include <cstddef>
#include <iterator>

static constexpr int kInverseTableSize = 1024; // SK_FDot6One * 16

static inline SkFixed quick_inverse(SkFDot6 x) {
    static const int32_t table[] = {
        -4096, -4100, -4104, -4108, -4112, -4116, -4120, -4124, -4128, -4132, -4136,
        -4140, -4144, -4148, -4152, -4156, -4161, -4165, -4169, -4173, -4177, -4181,
        -4185, -4190, -4194, -4198, -4202, -4206, -4211, -4215, -4219, -4223, -4228,
        -4232, -4236, -4240, -4245, -4249, -4253, -4258, -4262, -4266, -4271, -4275,
        -4279, -4284, -4288, -4293, -4297, -4301, -4306, -4310, -4315, -4319, -4324,
        -4328, -4332, -4337, -4341, -4346, -4350, -4355, -4359, -4364, -4369, -4373,
        -4378, -4382, -4387, -4391, -4396, -4401, -4405, -4410, -4415, -4419, -4424,
        -4429, -4433, -4438, -4443, -4447, -4452, -4457, -4462, -4466, -4471, -4476,
        -4481, -4485, -4490, -4495, -4500, -4505, -4510, -4514, -4519, -4524, -4529,
        -4534, -4539, -4544, -4549, -4554, -4559, -4563, -4568, -4573, -4578, -4583,
        -4588, -4593, -4599, -4604, -4609, -4614, -4619, -4624, -4629, -4634, -4639,
        -4644, -4650, -4655, -4660, -4665, -4670, -4675, -4681, -4686, -4691, -4696,
        -4702, -4707, -4712, -4718, -4723, -4728, -4733, -4739, -4744, -4750, -4755,
        -4760, -4766, -4771, -4777, -4782, -4788, -4793, -4798, -4804, -4809, -4815,
        -4821, -4826, -4832, -4837, -4843, -4848, -4854, -4860, -4865, -4871, -4877,
        -4882, -4888, -4894, -4899, -4905, -4911, -4917, -4922, -4928, -4934, -4940,
        -4946, -4951, -4957, -4963, -4969, -4975, -4981, -4987, -4993, -4999, -5005,
        -5011, -5017, -5023, -5029, -5035, -5041, -5047, -5053, -5059, -5065, -5071,
        -5077, -5084, -5090, -5096, -5102, -5108, -5115, -5121, -5127, -5133, -5140,
        -5146, -5152, -5159, -5165, -5171, -5178, -5184, -5190, -5197, -5203, -5210,
        -5216, -5223, -5229, -5236, -5242, -5249, -5256, -5262, -5269, -5275, -5282,
        -5289, -5295, -5302, -5309, -5315, -5322, -5329, -5336, -5343, -5349, -5356,
        -5363, -5370, -5377, -5384, -5391, -5398, -5405, -5412, -5418, -5426, -5433,
        -5440, -5447, -5454, -5461, -5468, -5475, -5482, -5489, -5497, -5504, -5511,
        -5518, -5526, -5533, -5540, -5548, -5555, -5562, -5570, -5577, -5584, -5592,
        -5599, -5607, -5614, -5622, -5629, -5637, -5645, -5652, -5660, -5667, -5675,
        -5683, -5691, -5698, -5706, -5714, -5722, -5729, -5737, -5745, -5753, -5761,
        -5769, -5777, -5785, -5793, -5801, -5809, -5817, -5825, -5833, -5841, -5849,
        -5857, -5866, -5874, -5882, -5890, -5899, -5907, -5915, -5924, -5932, -5940,
        -5949, -5957, -5966, -5974, -5983, -5991, -6000, -6009, -6017, -6026, -6034,
        -6043, -6052, -6061, -6069, -6078, -6087, -6096, -6105, -6114, -6123, -6132,
        -6141, -6150, -6159, -6168, -6177, -6186, -6195, -6204, -6213, -6223, -6232,
        -6241, -6250, -6260, -6269, -6278, -6288, -6297, -6307, -6316, -6326, -6335,
        -6345, -6355, -6364, -6374, -6384, -6393, -6403, -6413, -6423, -6432, -6442,
        -6452, -6462, -6472, -6482, -6492, -6502, -6512, -6523, -6533, -6543, -6553,
        -6563, -6574, -6584, -6594, -6605, -6615, -6626, -6636, -6647, -6657, -6668,
        -6678, -6689, -6700, -6710, -6721, -6732, -6743, -6754, -6765, -6775, -6786,
        -6797, -6808, -6820, -6831, -6842, -6853, -6864, -6875, -6887, -6898, -6909,
        -6921, -6932, -6944, -6955, -6967, -6978, -6990, -7002, -7013, -7025, -7037,
        -7049, -7061, -7073, -7084, -7096, -7108, -7121, -7133, -7145, -7157, -7169,
        -7182, -7194, -7206, -7219, -7231, -7244, -7256, -7269, -7281, -7294, -7307,
        -7319, -7332, -7345, -7358, -7371, -7384, -7397, -7410, -7423, -7436, -7449,
        -7463, -7476, -7489, -7503, -7516, -7530, -7543, -7557, -7570, -7584, -7598,
        -7612, -7626, -7639, -7653, -7667, -7681, -7695, -7710, -7724, -7738, -7752,
        -7767, -7781, -7796, -7810, -7825, -7839, -7854, -7869, -7884, -7898, -7913,
        -7928, -7943, -7958, -7973, -7989, -8004, -8019, -8035, -8050, -8065, -8081,
        -8097, -8112, -8128, -8144, -8160, -8176, -8192, -8208, -8224, -8240, -8256,
        -8272, -8289, -8305, -8322, -8338, -8355, -8371, -8388, -8405, -8422, -8439,
        -8456, -8473, -8490, -8507, -8525, -8542, -8559, -8577, -8594, -8612, -8630,
        -8648, -8665, -8683, -8701, -8719, -8738, -8756, -8774, -8793, -8811, -8830,
        -8848, -8867, -8886, -8905, -8924, -8943, -8962, -8981, -9000, -9020, -9039,
        -9058, -9078, -9098, -9118, -9137, -9157, -9177, -9198, -9218, -9238, -9258,
        -9279, -9300, -9320, -9341, -9362, -9383, -9404, -9425, -9446, -9467, -9489,
        -9510, -9532, -9554, -9576, -9597, -9619, -9642, -9664, -9686, -9709, -9731,
        -9754, -9776, -9799, -9822, -9845, -9868, -9892, -9915, -9939, -9962, -9986,
        -10010, -10034, -10058, -10082, -10106, -10131, -10155, -10180, -10205, -10230,
        -10255, -10280, -10305, -10330, -10356, -10381, -10407, -10433, -10459, -10485,
        -10512, -10538, -10564, -10591, -10618, -10645, -10672, -10699, -10727, -10754,
        -10782, -10810, -10837, -10866, -10894, -10922, -10951, -10979, -11008, -11037,
        -11066, -11096, -11125, -11155, -11184, -11214, -11244, -11275, -11305, -11335,
        -11366, -11397, -11428, -11459, -11491, -11522, -11554, -11586, -11618, -11650,
        -11683, -11715, -11748, -11781, -11814, -11848, -11881, -11915, -11949, -11983,
        -12018, -12052, -12087, -12122, -12157, -12192, -12228, -12264, -12300, -12336,
        -12372, -12409, -12446, -12483, -12520, -12557, -12595, -12633, -12671, -12710,
        -12748, -12787, -12826, -12865, -12905, -12945, -12985, -13025, -13066, -13107,
        -13148, -13189, -13231, -13273, -13315, -13357, -13400, -13443, -13486, -13530,
        -13573, -13617, -13662, -13706, -13751, -13797, -13842, -13888, -13934, -13981,
        -14027, -14074, -14122, -14169, -14217, -14266, -14315, -14364, -14413, -14463,
        -14513, -14563, -14614, -14665, -14716, -14768, -14820, -14873, -14926, -14979,
        -15033, -15087, -15141, -15196, -15252, -15307, -15363, -15420, -15477, -15534,
        -15592, -15650, -15709, -15768, -15827, -15887, -15947, -16008, -16070, -16131,
        -16194, -16256, -16320, -16384, -16448, -16513, -16578, -16644, -16710, -16777,
        -16844, -16912, -16980, -17050, -17119, -17189, -17260, -17331, -17403, -17476,
        -17549, -17623, -17697, -17772, -17848, -17924, -18001, -18078, -18157, -18236,
        -18315, -18396, -18477, -18558, -18641, -18724, -18808, -18893, -18978, -19065,
        -19152, -19239, -19328, -19418, -19508, -19599, -19691, -19784, -19878, -19972,
        -20068, -20164, -20262, -20360, -20460, -20560, -20661, -20763, -20867, -20971,
        -21076, -21183, -21290, -21399, -21509, -21620, -21732, -21845, -21959, -22075,
        -22192, -22310, -22429, -22550, -22671, -22795, -22919, -23045, -23172, -23301,
        -23431, -23563, -23696, -23831, -23967, -24105, -24244, -24385, -24528, -24672,
        -24818, -24966, -25115, -25266, -25420, -25575, -25731, -25890, -26051, -26214,
        -26379, -26546, -26715, -26886, -27060, -27235, -27413, -27594, -27776, -27962,
        -28149, -28339, -28532, -28728, -28926, -29127, -29330, -29537, -29746, -29959,
        -30174, -30393, -30615, -30840, -31068, -31300, -31536, -31775, -32017, -32263,
        -32513, -32768, -33026, -33288, -33554, -33825, -34100, -34379, -34663, -34952,
        -35246, -35544, -35848, -36157, -36472, -36792, -37117, -37449, -37786, -38130,
        -38479, -38836, -39199, -39568, -39945, -40329, -40721, -41120, -41527, -41943,
        -42366, -42799, -43240, -43690, -44150, -44620, -45100, -45590, -46091, -46603,
        -47127, -47662, -48210, -48770, -49344, -49932, -50533, -51150, -51781, -52428,
        -53092, -53773, -54471, -55188, -55924, -56679, -57456, -58254, -59074, -59918,
        -60787, -61680, -62601, -63550, -64527, -65536, -66576, -67650, -68759, -69905,
        -71089, -72315, -73584, -74898, -76260, -77672, -79137, -80659, -82241, -83886,
        -85598, -87381, -89240, -91180, -93206, -95325, -97541, -99864, -102300,
        -104857, -107546, -110376, -113359, -116508, -119837, -123361, -127100, -131072,
        -135300, -139810, -144631, -149796, -155344, -161319, -167772, -174762, -182361,
        -190650, -199728, -209715, -220752, -233016, -246723, -262144, -279620, -299593,
        -322638, -349525, -381300, -419430, -466033, -524288, -599186, -699050, -838860,
        -1048576, -1398101, -2097152, -4194304, 0
    };

    static constexpr size_t kLastEntry = std::size(table) - 1;
    SkASSERT(SkAbs32(x) <= static_cast<int32_t>(kLastEntry));
    static_assert(kLastEntry == kInverseTableSize);

    if (x > 0) {
        return -table[kLastEntry - x];
    } else {
        return table[kLastEntry + x];
    }
}

static inline SkFixed quick_div(SkFDot6 a, SkFDot6 b) {
    constexpr int kMinBits = 3;   // abs(b) should be at least (1 << kMinBits) for quick division
    constexpr int kMaxBits = 31;  // Number of bits available in signed int
    // Given abs(b) <= (1 << kMinBits), the inverse of abs(b) is at most 1 << (22 - kMinBits) in
    // SkFixed format. Hence abs(a) should be less than kMaxAbsA
    constexpr int kMaxAbsA = 1 << (kMaxBits - (22 - kMinBits));
    SkFDot6 abs_a = SkAbs32(a);
    SkFDot6 abs_b = SkAbs32(b);
    if (abs_b >= (1 << kMinBits) && abs_b < kInverseTableSize && abs_a < kMaxAbsA) {
        SkASSERT((int64_t)a * quick_inverse(b) <= SK_MaxS32
              && (int64_t)a * quick_inverse(b) >= SK_MinS32);
        SkFixed ourAnswer = (a * quick_inverse(b)) >> 6;
        SkASSERT(
            (SkFDot6Div(a,b) == 0 && ourAnswer == 0) ||
            SkFixedDiv(SkAbs32(SkFDot6Div(a,b) - ourAnswer), SkAbs32(SkFDot6Div(a,b))) <= 1 << 10
        );
        return ourAnswer;
    }
    return SkFDot6Div(a, b);
}

bool SkAnalyticEdge::setLine(const SkPoint& p0, const SkPoint& p1) {
    // We must set X/Y using the same way (e.g., times 4, to FDot6, then to Fixed) as Quads/Cubics.
    // Otherwise the order of the edge might be wrong due to precision limit.
    constexpr int accuracy = kDefaultAccuracy;
#ifdef SK_RASTERIZE_EVEN_ROUNDING
    SkFixed x0 = SkFDot6ToFixed(SkScalarRoundToFDot6(p0.fX, accuracy)) >> accuracy;
    SkFixed y0 = SnapY(SkFDot6ToFixed(SkScalarRoundToFDot6(p0.fY, accuracy)) >> accuracy);
    SkFixed x1 = SkFDot6ToFixed(SkScalarRoundToFDot6(p1.fX, accuracy)) >> accuracy;
    SkFixed y1 = SnapY(SkFDot6ToFixed(SkScalarRoundToFDot6(p1.fY, accuracy)) >> accuracy);
#else
    constexpr int multiplier = (1 << kDefaultAccuracy);
    SkFixed x0 = SkFDot6ToFixed(SkScalarToFDot6(p0.fX * multiplier)) >> accuracy;
    SkFixed y0 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p0.fY * multiplier)) >> accuracy);
    SkFixed x1 = SkFDot6ToFixed(SkScalarToFDot6(p1.fX * multiplier)) >> accuracy;
    SkFixed y1 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p1.fY * multiplier)) >> accuracy);
#endif

    Winding winding = Winding::kCW;

    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        winding = Winding::kCCW;
    }

    // are we a zero-height line?
    SkFDot6 dy = SkFixedToFDot6(y1 - y0);
    if (dy == 0) {
        return false;
    }
    SkFDot6 dx = SkFixedToFDot6(x1 - x0);
    SkFixed slope = quick_div(dx, dy);
    SkFixed absSlope = SkAbs32(slope);

    fX          = x0;
    fDX         = slope;
    fUpperX     = x0;
    fY          = y0;
    fUpperY     = y0;
    fLowerY     = y1;
    fDY         = dx == 0 || slope == 0 ? SK_MaxS32 : absSlope < kInverseTableSize
                                                    ? quick_inverse(absSlope)
                                                    : SkAbs32(quick_div(dy, dx));
    fEdgeType   = Type::kLine;
    fCurveCount = 0;
    fWinding    = winding;
    fCurveShift = 0;

    return true;
}

static SkAnalyticEdge::Winding swap_winding(SkAnalyticEdge::Winding w) {
    return static_cast<SkAnalyticEdge::Winding>(static_cast<int8_t>(w) * -1);
}

// This will become a bottleneck for small ovals rendering if we call SkFixedDiv twice here.
// Therefore, we'll let the outter function compute the slope once and send in the value.
// Moreover, we'll compute fDY by quickly lookup the inverse table (if possible).
bool SkAnalyticEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1, SkFixed slope) {
    // Since we send in the slope, we can no longer snap y inside this function.
    // If we don't send in the slope, or we do some more sophisticated snapping, this function
    // could be a performance bottleneck.
    SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    SkASSERT(fCurveCount != 0);

    // We don't chop at y extrema for cubics so the y is not guaranteed to be increasing for them.
    // In that case, we have to swap x/y and negate the winding.
    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        fWinding = swap_winding(fWinding);
    }

    SkASSERT(y0 <= y1);

    SkFDot6 dx = SkFixedToFDot6(x1 - x0);
    SkFDot6 dy = SkFixedToFDot6(y1 - y0);

    // are we a zero-height line?
    if (dy == 0) {
        return false;
    }

    SkASSERT(slope < SK_MaxS32);

    SkFDot6     absSlope = SkAbs32(SkFixedToFDot6(slope));
    fX          = x0;
    fDX         = slope;
    fUpperX     = x0;
    fY          = y0;
    fUpperY     = y0;
    fLowerY     = y1;
    fDY         = (dx == 0 || slope == 0)
                  ? SK_MaxS32
                  : absSlope < kInverseTableSize
                    ? quick_inverse(absSlope)
                    : SkAbs32(quick_div(dy, dx));

    return true;
}

bool SkAnalyticEdge::update(SkFixed last_y) {
    SkASSERT(last_y >= fLowerY); // we shouldn't update edge if last_y < fLowerY
    if (fCurveCount < 0) {
        return static_cast<SkAnalyticCubicEdge*>(this)->updateCubic();
    } else if (fCurveCount > 0) {
        return static_cast<SkAnalyticQuadraticEdge*>(this)->updateQuadratic();
    }
    return false;
}

/*  We store 1<<shift in a (signed) byte, so its maximum value is 1<<6 == 64.
    Note that this limits the number of lines we use to approximate a curve.
    If we need to increase this, we need to store fCurveCount in something
    larger than int8_t.
*/
#define MAX_COEFF_SHIFT     6

static inline SkFDot6 cheap_distance(SkFDot6 dx, SkFDot6 dy)
{
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    // return max + min/2
    if (dx > dy){
        dx += dy >> 1;
    } else {
        dx = dy + (dx >> 1);
    }
    return dx;
}

static inline int diff_to_shift(SkFDot6 dx, SkFDot6 dy, int shiftAA) {
    // cheap calc of distance from center of p0-p2 to the center of the curve
    SkFDot6 dist = cheap_distance(dx, dy);

    // shift down dist (it is currently in dot6)
    // down by 3 should give us 1/8 pixel accuracy (assuming our dist is accurate...)
    // this is chosen by heuristic: make it as big as possible (to minimize segments)
    // ... but small enough so that our curves still look smooth
    // When shift > 0, we're using AA and everything is scaled up so we can
    // lower the accuracy.
    dist = (dist + (1 << (2 + shiftAA))) >> (3 + shiftAA);

    // each subdivision (shift value) cuts this dist (error) by 1/4
    return (32 - SkCLZ(dist)) >> 1;
}

/*
    In setQuadraticWithoutUpdate, setCubicWithoutUpdate, the first thing we do is to convert
    the points into FDot6. This is modulated by the shift parameter, which
    will be something like 2 for antialiasing.

    In the float case, we want to turn the float into .6 by saying pt * 64,
    or pt * 256 for antialiasing. This is implemented as 1 << (shift + 6).

    In the fixed case, we want to turn the fixed into .6 by saying pt >> 10,
    or pt >> 8 for antialiasing. This is implemented as pt >> (10 - shift).
*/

static inline SkFixed SkFDot6ToFixedDiv2(SkFDot6 value) {
    // we want to return SkFDot6ToFixed(value >> 1), but we don't want to throw
    // away data in value, so just perform a modify up-shift
    return SkLeftShift(value, 16 - 6 - 1);
}

bool SkAnalyticQuadraticEdge::setQuadraticWithoutUpdate(const SkPoint pts[3], int shift) {
    SkFDot6 x0, y0, x1, y1, x2, y2;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(pts[0].fX, shift);
        y0 = SkScalarRoundToFDot6(pts[0].fY, shift);
        x1 = SkScalarRoundToFDot6(pts[1].fX, shift);
        y1 = SkScalarRoundToFDot6(pts[1].fY, shift);
        x2 = SkScalarRoundToFDot6(pts[2].fX, shift);
        y2 = SkScalarRoundToFDot6(pts[2].fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
#endif
    }

    Winding winding = Winding::kCW;
    if (y0 > y2)
    {
        using std::swap;
        swap(x0, x2);
        swap(y0, y2);
        winding = Winding::kCCW;
    }
    SkASSERT(y0 <= y1 && y1 <= y2);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y2);

    // are we a zero-height quad (line)?
    if (top == bot) {
        return 0;
    }

    // compute number of steps needed (1 << shift)
    {
        SkFDot6 dx = (SkLeftShift(x1, 1) - x0 - x2) >> 2;
        SkFDot6 dy = (SkLeftShift(y1, 1) - y0 - y2) >> 2;
        // This is a little confusing:
        // before this line, shift is the scale up factor for AA;
        // after this line, shift is the fCurveShift.
        shift = diff_to_shift(dx, dy, shift);
        SkASSERT(shift >= 0);
    }
    // need at least 1 subdivision for our bias trick
    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    fWinding = winding;
    //fCubicDShift only set for cubics
    fEdgeType = Type::kQuad;
    fCurveCount = SkToS8(1 << shift);

    /*
     *  We want to reformulate into polynomial form, to make it clear how we
     *  should forward-difference.
     *
     *  p0 (1 - t)^2 + p1 t(1 - t) + p2 t^2 ==> At^2 + Bt + C
     *
     *  A = p0 - 2p1 + p2
     *  B = 2(p1 - p0)
     *  C = p0
     *
     *  Our caller must have constrained our inputs (p0..p2) to all fit into
     *  16.16. However, as seen above, we sometimes compute values that can be
     *  larger (e.g. B = 2*(p1 - p0)). To guard against overflow, we will store
     *  A and B at 1/2 of their actual value, and just apply a 2x scale during
     *  application in updateQuadratic(). Hence we store (shift - 1) in
     *  fCurveShift.
     */

    fCurveShift = SkToU8(shift - 1);

    SkFixed A = SkFDot6ToFixedDiv2(x0 - x1 - x1 + x2);  // 1/2 the real value
    SkFixed B = SkFDot6ToFixed(x1 - x0);                // 1/2 the real value

    fQx     = SkFDot6ToFixed(x0);
    fQDx    = B + (A >> shift);     // biased by shift
    fQDDx   = A >> (shift - 1);     // biased by shift

    A = SkFDot6ToFixedDiv2(y0 - y1 - y1 + y2);  // 1/2 the real value
    B = SkFDot6ToFixed(y1 - y0);                // 1/2 the real value

    fQy     = SkFDot6ToFixed(y0);
    fQDy    = B + (A >> shift);     // biased by shift
    fQDDy   = A >> (shift - 1);     // biased by shift

    fQLastX = SkFDot6ToFixed(x2);
    fQLastY = SkFDot6ToFixed(y2);

    return true;
}

bool SkAnalyticQuadraticEdge::setQuadratic(const SkPoint pts[3]) {
    if (!setQuadraticWithoutUpdate(pts, kDefaultAccuracy)) {
        return false;
    }
    fQx >>= kDefaultAccuracy;
    fQy >>= kDefaultAccuracy;
    fQDx >>= kDefaultAccuracy;
    fQDy >>= kDefaultAccuracy;
    fQDDx >>= kDefaultAccuracy;
    fQDDy >>= kDefaultAccuracy;
    fQLastX >>= kDefaultAccuracy;
    fQLastY >>= kDefaultAccuracy;
    fQy = SnapY(fQy);
    fQLastY = SnapY(fQLastY);

    fEdgeType = Type::kQuad;

    fSnappedX = fQx;
    fSnappedY = fQy;

    return this->updateQuadratic();
}

bool SkAnalyticQuadraticEdge::updateQuadratic() {
    int     success = 0; // initialize to fail!
    int     count = fCurveCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed dx = fQDx;
    SkFixed dy = fQDy;
    SkFixed newx, newy, newSnappedX, newSnappedY;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        SkFixed slope;
        if (--count > 0)
        {
            newx    = oldx + (dx >> shift);
            newy    = oldy + (dy >> shift);
            // only snap when dy is large enough and dx/dy isn't too large
            if (SkAbs32(dy >> shift) >= SK_Fixed1 * 2 &&
                SkLeftShift((int64_t) SkAbs32(dy), 6) > SkAbs32(dx)) {
                SkFDot6 diffY = SkFixedToFDot6(newy - fSnappedY);
                slope = diffY ? quick_div(SkFixedToFDot6(newx - fSnappedX), diffY)
                              : SK_MaxS32;
                newSnappedY = std::min<SkFixed>(fQLastY, SkFixedRoundToFixed(newy));
                newSnappedX = newx - SkFixedMul(slope, newy - newSnappedY);
            } else {
                newSnappedY = std::min(fQLastY, SnapY(newy));
                newSnappedX = newx;
                SkFDot6 diffY = SkFixedToFDot6(newSnappedY - fSnappedY);
                slope = diffY ? quick_div(SkFixedToFDot6(newx - fSnappedX), diffY)
                              : SK_MaxS32;
            }
            dx += fQDDx;
            dy += fQDDy;
        }
        else    // last segment
        {
            newx    = fQLastX;
            newy    = fQLastY;
            newSnappedY = newy;
            newSnappedX = newx;
            SkFDot6 diffY = SkFixedToFDot6(newy - fSnappedY);
            slope = diffY ? quick_div(SkFixedToFDot6(newx - fSnappedX), diffY) : SK_MaxS32;
        }
        if (slope < SK_MaxS32) {
            success = this->updateLine(fSnappedX, fSnappedY, newSnappedX, newSnappedY, slope);
        }
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    SkASSERT(newSnappedY <= fQLastY);

    fQx  = newx;
    fQy  = newy;
    fQDx = dx;
    fQDy = dy;
    fSnappedX   = newSnappedX;
    fSnappedY   = newSnappedY;
    fCurveCount = SkToS8(count);
    return success;
}

bool SkAnalyticCubicEdge::setCubic(const SkPoint pts[4]) {
    if (!setCubicWithoutUpdate(pts, kDefaultAccuracy)) {
        return false;
    }

    fCx >>= kDefaultAccuracy;
    fCy >>= kDefaultAccuracy;
    fCDx >>= kDefaultAccuracy;
    fCDy >>= kDefaultAccuracy;
    fCDDx >>= kDefaultAccuracy;
    fCDDy >>= kDefaultAccuracy;
    fCDDDx >>= kDefaultAccuracy;
    fCDDDy >>= kDefaultAccuracy;
    fCLastX >>= kDefaultAccuracy;
    fCLastY >>= kDefaultAccuracy;
    fCy = SnapY(fCy);
    fSnappedY = fCy;
    fCLastY = SnapY(fCLastY);

    fEdgeType = Type::kCubic;

    return this->updateCubic();
}

static inline int SkFDot6UpShift(SkFDot6 x, int upShift) {
    SkASSERT((SkLeftShift(x, upShift) >> upShift) == x);
    return SkLeftShift(x, upShift);
}

/*  f(1/3) = (8a + 12b + 6c + d) / 27
    f(2/3) = (a + 6b + 12c + 8d) / 27

    f(1/3)-b = (8a - 15b + 6c + d) / 27
    f(2/3)-c = (a + 6b - 15c + 8d) / 27

    use 16/512 to approximate 1/27
*/
static SkFDot6 cubic_delta_from_line(SkFDot6 a, SkFDot6 b, SkFDot6 c, SkFDot6 d)
{
    // since our parameters may be negative, we don't use << to avoid ASAN warnings
    SkFDot6 oneThird = (a*8 - b*15 + 6*c + d) * 19 >> 9;
    SkFDot6 twoThird = (a + 6*b - c*15 + d*8) * 19 >> 9;

    return std::max(SkAbs32(oneThird), SkAbs32(twoThird));
}

bool SkAnalyticCubicEdge::setCubicWithoutUpdate(const SkPoint pts[4], int shift) {
    SkFDot6 x0, y0, x1, y1, x2, y2, x3, y3;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(pts[0].fX, shift);
        y0 = SkScalarRoundToFDot6(pts[0].fY, shift);
        x1 = SkScalarRoundToFDot6(pts[1].fX, shift);
        y1 = SkScalarRoundToFDot6(pts[1].fY, shift);
        x2 = SkScalarRoundToFDot6(pts[2].fX, shift);
        y2 = SkScalarRoundToFDot6(pts[2].fY, shift);
        x3 = SkScalarRoundToFDot6(pts[3].fX, shift);
        y3 = SkScalarRoundToFDot6(pts[3].fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(pts[0].fX * scale);
        y0 = int(pts[0].fY * scale);
        x1 = int(pts[1].fX * scale);
        y1 = int(pts[1].fY * scale);
        x2 = int(pts[2].fX * scale);
        y2 = int(pts[2].fY * scale);
        x3 = int(pts[3].fX * scale);
        y3 = int(pts[3].fY * scale);
#endif
    }

    Winding winding = Winding::kCW;
    if (y0 > y3)
    {
        using std::swap;
        swap(x0, x3);
        swap(x1, x2);
        swap(y0, y3);
        swap(y1, y2);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y3);

    // are we a zero-height cubic (line)?
    if (top == bot)
        return 0;

    // compute number of steps needed (1 << shift)
    {
        // Can't use (center of curve - center of baseline), since center-of-curve
        // need not be the max delta from the baseline (it could even be coincident)
        // so we try just looking at the two off-curve points
        SkFDot6 dx = cubic_delta_from_line(x0, x1, x2, x3);
        SkFDot6 dy = cubic_delta_from_line(y0, y1, y2, y3);
        // add 1 (by observation)
        shift = diff_to_shift(dx, dy, 2) + 1;
    }
    // need at least 1 subdivision for our bias trick
    SkASSERT(shift > 0);
    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    /*  Since our in coming data is initially shifted down by 10 (or 8 in
        antialias). That means the most we can shift up is 8. However, we
        compute coefficients with a 3*, so the safest upshift is really 6
    */
    int upShift = 6;    // largest safe value
    int downShift = shift + upShift - 10;
    if (downShift < 0) {
        downShift = 0;
        upShift = 10 - shift;
    }

    fWinding = winding;
    fEdgeType = Type::kCubic;
    fCurveCount = SkToS8(SkLeftShift(-1, shift));
    fCurveShift = SkToU8(shift);
    fCubicDShift = SkToU8(downShift);

    SkFixed B = SkFDot6UpShift(3 * (x1 - x0), upShift);
    SkFixed C = SkFDot6UpShift(3 * (x0 - x1 - x1 + x2), upShift);
    SkFixed D = SkFDot6UpShift(x3 + 3 * (x1 - x2) - x0, upShift);

    fCx     = SkFDot6ToFixed(x0);
    fCDx    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDx   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDx  = 3*D >> (shift - 1);                   // biased by 2*shift

    B = SkFDot6UpShift(3 * (y1 - y0), upShift);
    C = SkFDot6UpShift(3 * (y0 - y1 - y1 + y2), upShift);
    D = SkFDot6UpShift(y3 + 3 * (y1 - y2) - y0, upShift);

    fCy     = SkFDot6ToFixed(y0);
    fCDy    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDy   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDy  = 3*D >> (shift - 1);                   // biased by 2*shift

    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    return true;
}

bool SkAnalyticCubicEdge::updateCubic() {
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fCx;
    SkFixed oldy = fCy;
    SkFixed newx, newy;
    const int ddshift = fCurveShift;
    const int dshift = fCubicDShift;

    SkASSERT(count < 0);

    do {
        if (++count < 0) {
            newx    = oldx + (fCDx >> dshift);
            fCDx    += fCDDx >> ddshift;
            fCDDx   += fCDDDx;

            newy    = oldy + (fCDy >> dshift);
            fCDy    += fCDDy >> ddshift;
            fCDDy   += fCDDDy;
        }
        else {    // last segment
            newx    = fCLastX;
            newy    = fCLastY;
        }

        // we want to say SkASSERT(oldy <= newy), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (newy < oldy) {
            newy = oldy;
        }

        SkFixed newSnappedY = SnapY(newy);
        // we want to SkASSERT(snappedNewY <= fCLastY), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (fCLastY < newSnappedY) {
            newSnappedY = fCLastY;
            count = 0;
        }

        SkFixed slope = SkFixedToFDot6(newSnappedY - fSnappedY) == 0
                        ? SK_MaxS32
                        : SkFDot6Div(SkFixedToFDot6(newx - oldx),
                                     SkFixedToFDot6(newSnappedY - fSnappedY));

        success = this->updateLine(oldx, fSnappedY, newx, newSnappedY, slope);

        oldx = newx;
        oldy = newy;
        fSnappedY = newSnappedY;
    } while (count < 0 && !success);

    fCx  = newx;
    fCy  = newy;
    fCurveCount = SkToS8(count);
    return success;
}
