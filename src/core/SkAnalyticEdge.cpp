/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnalyticEdge.h"
#include "SkFDot6.h"
#include "SkMathPriv.h"
#include "SkTo.h"
#include <utility>

static const int kInverseTableSize = 1024; // SK_FDot6One * 16

static inline SkFixed quick_inverse(SkFDot6 x) {
    SkASSERT(SkAbs32(x) < kInverseTableSize);
    static const int32_t table[kInverseTableSize * 2] = {
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
        -1048576, -1398101, -2097152, -4194304, 0, 4194304, 2097152, 1398101, 1048576,
        838860, 699050, 599186, 524288, 466033, 419430, 381300, 349525, 322638, 299593,
        279620, 262144, 246723, 233016, 220752, 209715, 199728, 190650, 182361, 174762,
        167772, 161319, 155344, 149796, 144631, 139810, 135300, 131072, 127100, 123361,
        119837, 116508, 113359, 110376, 107546, 104857, 102300, 99864, 97541, 95325,
        93206, 91180, 89240, 87381, 85598, 83886, 82241, 80659, 79137, 77672, 76260,
        74898, 73584, 72315, 71089, 69905, 68759, 67650, 66576, 65536, 64527, 63550,
        62601, 61680, 60787, 59918, 59074, 58254, 57456, 56679, 55924, 55188, 54471,
        53773, 53092, 52428, 51781, 51150, 50533, 49932, 49344, 48770, 48210, 47662,
        47127, 46603, 46091, 45590, 45100, 44620, 44150, 43690, 43240, 42799, 42366,
        41943, 41527, 41120, 40721, 40329, 39945, 39568, 39199, 38836, 38479, 38130,
        37786, 37449, 37117, 36792, 36472, 36157, 35848, 35544, 35246, 34952, 34663,
        34379, 34100, 33825, 33554, 33288, 33026, 32768, 32513, 32263, 32017, 31775,
        31536, 31300, 31068, 30840, 30615, 30393, 30174, 29959, 29746, 29537, 29330,
        29127, 28926, 28728, 28532, 28339, 28149, 27962, 27776, 27594, 27413, 27235,
        27060, 26886, 26715, 26546, 26379, 26214, 26051, 25890, 25731, 25575, 25420,
        25266, 25115, 24966, 24818, 24672, 24528, 24385, 24244, 24105, 23967, 23831,
        23696, 23563, 23431, 23301, 23172, 23045, 22919, 22795, 22671, 22550, 22429,
        22310, 22192, 22075, 21959, 21845, 21732, 21620, 21509, 21399, 21290, 21183,
        21076, 20971, 20867, 20763, 20661, 20560, 20460, 20360, 20262, 20164, 20068,
        19972, 19878, 19784, 19691, 19599, 19508, 19418, 19328, 19239, 19152, 19065,
        18978, 18893, 18808, 18724, 18641, 18558, 18477, 18396, 18315, 18236, 18157,
        18078, 18001, 17924, 17848, 17772, 17697, 17623, 17549, 17476, 17403, 17331,
        17260, 17189, 17119, 17050, 16980, 16912, 16844, 16777, 16710, 16644, 16578,
        16513, 16448, 16384, 16320, 16256, 16194, 16131, 16070, 16008, 15947, 15887,
        15827, 15768, 15709, 15650, 15592, 15534, 15477, 15420, 15363, 15307, 15252,
        15196, 15141, 15087, 15033, 14979, 14926, 14873, 14820, 14768, 14716, 14665,
        14614, 14563, 14513, 14463, 14413, 14364, 14315, 14266, 14217, 14169, 14122,
        14074, 14027, 13981, 13934, 13888, 13842, 13797, 13751, 13706, 13662, 13617,
        13573, 13530, 13486, 13443, 13400, 13357, 13315, 13273, 13231, 13189, 13148,
        13107, 13066, 13025, 12985, 12945, 12905, 12865, 12826, 12787, 12748, 12710,
        12671, 12633, 12595, 12557, 12520, 12483, 12446, 12409, 12372, 12336, 12300,
        12264, 12228, 12192, 12157, 12122, 12087, 12052, 12018, 11983, 11949, 11915,
        11881, 11848, 11814, 11781, 11748, 11715, 11683, 11650, 11618, 11586, 11554,
        11522, 11491, 11459, 11428, 11397, 11366, 11335, 11305, 11275, 11244, 11214,
        11184, 11155, 11125, 11096, 11066, 11037, 11008, 10979, 10951, 10922, 10894,
        10866, 10837, 10810, 10782, 10754, 10727, 10699, 10672, 10645, 10618, 10591,
        10564, 10538, 10512, 10485, 10459, 10433, 10407, 10381, 10356, 10330, 10305,
        10280, 10255, 10230, 10205, 10180, 10155, 10131, 10106, 10082, 10058, 10034,
        10010, 9986, 9962, 9939, 9915, 9892, 9868, 9845, 9822, 9799, 9776, 9754, 9731,
        9709, 9686, 9664, 9642, 9619, 9597, 9576, 9554, 9532, 9510, 9489, 9467, 9446,
        9425, 9404, 9383, 9362, 9341, 9320, 9300, 9279, 9258, 9238, 9218, 9198, 9177,
        9157, 9137, 9118, 9098, 9078, 9058, 9039, 9020, 9000, 8981, 8962, 8943, 8924,
        8905, 8886, 8867, 8848, 8830, 8811, 8793, 8774, 8756, 8738, 8719, 8701, 8683,
        8665, 8648, 8630, 8612, 8594, 8577, 8559, 8542, 8525, 8507, 8490, 8473, 8456,
        8439, 8422, 8405, 8388, 8371, 8355, 8338, 8322, 8305, 8289, 8272, 8256, 8240,
        8224, 8208, 8192, 8176, 8160, 8144, 8128, 8112, 8097, 8081, 8065, 8050, 8035,
        8019, 8004, 7989, 7973, 7958, 7943, 7928, 7913, 7898, 7884, 7869, 7854, 7839,
        7825, 7810, 7796, 7781, 7767, 7752, 7738, 7724, 7710, 7695, 7681, 7667, 7653,
        7639, 7626, 7612, 7598, 7584, 7570, 7557, 7543, 7530, 7516, 7503, 7489, 7476,
        7463, 7449, 7436, 7423, 7410, 7397, 7384, 7371, 7358, 7345, 7332, 7319, 7307,
        7294, 7281, 7269, 7256, 7244, 7231, 7219, 7206, 7194, 7182, 7169, 7157, 7145,
        7133, 7121, 7108, 7096, 7084, 7073, 7061, 7049, 7037, 7025, 7013, 7002, 6990,
        6978, 6967, 6955, 6944, 6932, 6921, 6909, 6898, 6887, 6875, 6864, 6853, 6842,
        6831, 6820, 6808, 6797, 6786, 6775, 6765, 6754, 6743, 6732, 6721, 6710, 6700,
        6689, 6678, 6668, 6657, 6647, 6636, 6626, 6615, 6605, 6594, 6584, 6574, 6563,
        6553, 6543, 6533, 6523, 6512, 6502, 6492, 6482, 6472, 6462, 6452, 6442, 6432,
        6423, 6413, 6403, 6393, 6384, 6374, 6364, 6355, 6345, 6335, 6326, 6316, 6307,
        6297, 6288, 6278, 6269, 6260, 6250, 6241, 6232, 6223, 6213, 6204, 6195, 6186,
        6177, 6168, 6159, 6150, 6141, 6132, 6123, 6114, 6105, 6096, 6087, 6078, 6069,
        6061, 6052, 6043, 6034, 6026, 6017, 6009, 6000, 5991, 5983, 5974, 5966, 5957,
        5949, 5940, 5932, 5924, 5915, 5907, 5899, 5890, 5882, 5874, 5866, 5857, 5849,
        5841, 5833, 5825, 5817, 5809, 5801, 5793, 5785, 5777, 5769, 5761, 5753, 5745,
        5737, 5729, 5722, 5714, 5706, 5698, 5691, 5683, 5675, 5667, 5660, 5652, 5645,
        5637, 5629, 5622, 5614, 5607, 5599, 5592, 5584, 5577, 5570, 5562, 5555, 5548,
        5540, 5533, 5526, 5518, 5511, 5504, 5497, 5489, 5482, 5475, 5468, 5461, 5454,
        5447, 5440, 5433, 5426, 5418, 5412, 5405, 5398, 5391, 5384, 5377, 5370, 5363,
        5356, 5349, 5343, 5336, 5329, 5322, 5315, 5309, 5302, 5295, 5289, 5282, 5275,
        5269, 5262, 5256, 5249, 5242, 5236, 5229, 5223, 5216, 5210, 5203, 5197, 5190,
        5184, 5178, 5171, 5165, 5159, 5152, 5146, 5140, 5133, 5127, 5121, 5115, 5108,
        5102, 5096, 5090, 5084, 5077, 5071, 5065, 5059, 5053, 5047, 5041, 5035, 5029,
        5023, 5017, 5011, 5005, 4999, 4993, 4987, 4981, 4975, 4969, 4963, 4957, 4951,
        4946, 4940, 4934, 4928, 4922, 4917, 4911, 4905, 4899, 4894, 4888, 4882, 4877,
        4871, 4865, 4860, 4854, 4848, 4843, 4837, 4832, 4826, 4821, 4815, 4809, 4804,
        4798, 4793, 4788, 4782, 4777, 4771, 4766, 4760, 4755, 4750, 4744, 4739, 4733,
        4728, 4723, 4718, 4712, 4707, 4702, 4696, 4691, 4686, 4681, 4675, 4670, 4665,
        4660, 4655, 4650, 4644, 4639, 4634, 4629, 4624, 4619, 4614, 4609, 4604, 4599,
        4593, 4588, 4583, 4578, 4573, 4568, 4563, 4559, 4554, 4549, 4544, 4539, 4534,
        4529, 4524, 4519, 4514, 4510, 4505, 4500, 4495, 4490, 4485, 4481, 4476, 4471,
        4466, 4462, 4457, 4452, 4447, 4443, 4438, 4433, 4429, 4424, 4419, 4415, 4410,
        4405, 4401, 4396, 4391, 4387, 4382, 4378, 4373, 4369, 4364, 4359, 4355, 4350,
        4346, 4341, 4337, 4332, 4328, 4324, 4319, 4315, 4310, 4306, 4301, 4297, 4293,
        4288, 4284, 4279, 4275, 4271, 4266, 4262, 4258, 4253, 4249, 4245, 4240, 4236,
        4232, 4228, 4223, 4219, 4215, 4211, 4206, 4202, 4198, 4194, 4190, 4185, 4181,
        4177, 4173, 4169, 4165, 4161, 4156, 4152, 4148, 4144, 4140, 4136, 4132, 4128,
        4124, 4120, 4116, 4112, 4108, 4104, 4100
    };
    return table[kInverseTableSize + x];
}

static inline SkFixed quick_div(SkFDot6 a, SkFDot6 b) {
    const int kMinBits = 3;  // abs(b) should be at least (1 << kMinBits) for quick division
    const int kMaxBits = 31; // Number of bits available in signed int
    // Given abs(b) <= (1 << kMinBits), the inverse of abs(b) is at most 1 << (22 - kMinBits) in
    // SkFixed format. Hence abs(a) should be less than kMaxAbsA
    const int kMaxAbsA = 1 << (kMaxBits - (22 - kMinBits));
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
    fRiteE = nullptr;

    // We must set X/Y using the same way (e.g., times 4, to FDot6, then to Fixed) as Quads/Cubics.
    // Otherwise the order of the edge might be wrong due to precision limit.
    const int accuracy = kDefaultAccuracy;
#ifdef SK_RASTERIZE_EVEN_ROUNDING
    SkFixed x0 = SkFDot6ToFixed(SkScalarRoundToFDot6(p0.fX, accuracy)) >> accuracy;
    SkFixed y0 = SnapY(SkFDot6ToFixed(SkScalarRoundToFDot6(p0.fY, accuracy)) >> accuracy);
    SkFixed x1 = SkFDot6ToFixed(SkScalarRoundToFDot6(p1.fX, accuracy)) >> accuracy;
    SkFixed y1 = SnapY(SkFDot6ToFixed(SkScalarRoundToFDot6(p1.fY, accuracy)) >> accuracy);
#else
    const int multiplier = (1 << kDefaultAccuracy);
    SkFixed x0 = SkFDot6ToFixed(SkScalarToFDot6(p0.fX * multiplier)) >> accuracy;
    SkFixed y0 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p0.fY * multiplier)) >> accuracy);
    SkFixed x1 = SkFDot6ToFixed(SkScalarToFDot6(p1.fX * multiplier)) >> accuracy;
    SkFixed y1 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p1.fY * multiplier)) >> accuracy);
#endif

    int winding = 1;

    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        winding = -1;
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
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;

    return true;
}

// This will become a bottleneck for small ovals rendering if we call SkFixedDiv twice here.
// Therefore, we'll let the outter function compute the slope once and send in the value.
// Moreover, we'll compute fDY by quickly lookup the inverse table (if possible).
bool SkAnalyticEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1, SkFixed slope) {
    // Since we send in the slope, we can no longer snap y inside this function.
    // If we don't send in the slope, or we do some more sophisticated snapping, this function
    // could be a performance bottleneck.
    SkASSERT(fWinding == 1 || fWinding == -1);
    SkASSERT(fCurveCount != 0);

    // We don't chop at y extrema for cubics so the y is not guaranteed to be increasing for them.
    // In that case, we have to swap x/y and negate the winding.
    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        fWinding = -fWinding;
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

bool SkAnalyticEdge::update(SkFixed last_y, bool sortY) {
    SkASSERT(last_y >= fLowerY); // we shouldn't update edge if last_y < fLowerY
    if (fCurveCount < 0) {
        return static_cast<SkAnalyticCubicEdge*>(this)->updateCubic(sortY);
    } else if (fCurveCount > 0) {
        return static_cast<SkAnalyticQuadraticEdge*>(this)->updateQuadratic();
    }
    return false;
}

bool SkAnalyticQuadraticEdge::setQuadratic(const SkPoint pts[3]) {
    fRiteE = nullptr;

    if (!fQEdge.setQuadraticWithoutUpdate(pts, kDefaultAccuracy)) {
        return false;
    }
    fQEdge.fQx >>= kDefaultAccuracy;
    fQEdge.fQy >>= kDefaultAccuracy;
    fQEdge.fQDx >>= kDefaultAccuracy;
    fQEdge.fQDy >>= kDefaultAccuracy;
    fQEdge.fQDDx >>= kDefaultAccuracy;
    fQEdge.fQDDy >>= kDefaultAccuracy;
    fQEdge.fQLastX >>= kDefaultAccuracy;
    fQEdge.fQLastY >>= kDefaultAccuracy;
    fQEdge.fQy = SnapY(fQEdge.fQy);
    fQEdge.fQLastY = SnapY(fQEdge.fQLastY);

    fWinding = fQEdge.fWinding;
    fCurveCount = fQEdge.fCurveCount;
    fCurveShift = fQEdge.fCurveShift;

    fSnappedX = fQEdge.fQx;
    fSnappedY = fQEdge.fQy;

    return this->updateQuadratic();
}

bool SkAnalyticQuadraticEdge::updateQuadratic() {
    int     success = 0; // initialize to fail!
    int     count = fCurveCount;
    SkFixed oldx = fQEdge.fQx;
    SkFixed oldy = fQEdge.fQy;
    SkFixed dx = fQEdge.fQDx;
    SkFixed dy = fQEdge.fQDy;
    SkFixed newx, newy, newSnappedX, newSnappedY;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        SkFixed slope;
        if (--count > 0)
        {
            newx    = oldx + (dx >> shift);
            newy    = oldy + (dy >> shift);
            if (SkAbs32(dy >> shift) >= SK_Fixed1 * 2) { // only snap when dy is large enough
                SkFDot6 diffY = SkFixedToFDot6(newy - fSnappedY);
                slope = diffY ? quick_div(SkFixedToFDot6(newx - fSnappedX), diffY)
                              : SK_MaxS32;
                newSnappedY = SkTMin<SkFixed>(fQEdge.fQLastY, SkFixedRoundToFixed(newy));
                newSnappedX = newx - SkFixedMul(slope, newy - newSnappedY);
            } else {
                newSnappedY = SkTMin(fQEdge.fQLastY, SnapY(newy));
                newSnappedX = newx;
                SkFDot6 diffY = SkFixedToFDot6(newSnappedY - fSnappedY);
                slope = diffY ? quick_div(SkFixedToFDot6(newx - fSnappedX), diffY)
                              : SK_MaxS32;
            }
            dx += fQEdge.fQDDx;
            dy += fQEdge.fQDDy;
        }
        else    // last segment
        {
            newx    = fQEdge.fQLastX;
            newy    = fQEdge.fQLastY;
            newSnappedY = newy;
            newSnappedX = newx;
            SkFDot6 diffY = (newy - fSnappedY) >> 10;
            slope = diffY ? quick_div((newx - fSnappedX) >> 10, diffY) : SK_MaxS32;
        }
        if (slope < SK_MaxS32) {
            success = this->updateLine(fSnappedX, fSnappedY, newSnappedX, newSnappedY, slope);
        }
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    SkASSERT(newSnappedY <= fQEdge.fQLastY);

    fQEdge.fQx  = newx;
    fQEdge.fQy  = newy;
    fQEdge.fQDx = dx;
    fQEdge.fQDy = dy;
    fSnappedX   = newSnappedX;
    fSnappedY   = newSnappedY;
    fCurveCount = SkToS8(count);
    return success;
}

bool SkAnalyticCubicEdge::setCubic(const SkPoint pts[4], bool sortY) {
    fRiteE = nullptr;

    if (!fCEdge.setCubicWithoutUpdate(pts, kDefaultAccuracy, sortY)) {
        return false;
    }

    fCEdge.fCx >>= kDefaultAccuracy;
    fCEdge.fCy >>= kDefaultAccuracy;
    fCEdge.fCDx >>= kDefaultAccuracy;
    fCEdge.fCDy >>= kDefaultAccuracy;
    fCEdge.fCDDx >>= kDefaultAccuracy;
    fCEdge.fCDDy >>= kDefaultAccuracy;
    fCEdge.fCDDDx >>= kDefaultAccuracy;
    fCEdge.fCDDDy >>= kDefaultAccuracy;
    fCEdge.fCLastX >>= kDefaultAccuracy;
    fCEdge.fCLastY >>= kDefaultAccuracy;
    fCEdge.fCy = SnapY(fCEdge.fCy);
    fCEdge.fCLastY = SnapY(fCEdge.fCLastY);

    fWinding = fCEdge.fWinding;
    fCurveCount = fCEdge.fCurveCount;
    fCurveShift = fCEdge.fCurveShift;
    fCubicDShift = fCEdge.fCubicDShift;

    fSnappedY = fCEdge.fCy;

    return this->updateCubic(sortY);
}

bool SkAnalyticCubicEdge::updateCubic(bool sortY) {
    int     success;
    int     count = fCurveCount;
    SkFixed oldx = fCEdge.fCx;
    SkFixed oldy = fCEdge.fCy;
    SkFixed newx, newy;
    const int ddshift = fCurveShift;
    const int dshift = fCubicDShift;

    SkASSERT(count < 0);

    do {
        if (++count < 0) {
            newx    = oldx + (fCEdge.fCDx >> dshift);
            fCEdge.fCDx    += fCEdge.fCDDx >> ddshift;
            fCEdge.fCDDx   += fCEdge.fCDDDx;

            newy    = oldy + (fCEdge.fCDy >> dshift);
            fCEdge.fCDy    += fCEdge.fCDDy >> ddshift;
            fCEdge.fCDDy   += fCEdge.fCDDDy;
        }
        else {    // last segment
            newx    = fCEdge.fCLastX;
            newy    = fCEdge.fCLastY;
        }

        // we want to say SkASSERT(oldy <= newy), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (sortY && newy < oldy) {
            newy = oldy;
        }

        SkFixed newSnappedY = SnapY(newy);
        // we want to SkASSERT(snappedNewY <= fCEdge.fCLastY), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (sortY && fCEdge.fCLastY < newSnappedY) {
            newSnappedY = fCEdge.fCLastY;
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

    fCEdge.fCx  = newx;
    fCEdge.fCy  = newy;
    fCurveCount = SkToS8(count);
    return success;
}
