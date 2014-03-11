/*
 * fontconfig/fc-lang/fclang.tmpl.h
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* total size: 911 unique leaves: 617 */

#define LEAF0       (244 * sizeof (FcLangCharSet))
#define OFF0        (LEAF0 + 617 * sizeof (FcCharLeaf))
#define NUM0        (OFF0 + 667 * sizeof (uintptr_t))
#define SET(n)      (n * sizeof (FcLangCharSet) + offsetof (FcLangCharSet, charset))
#define OFF(s,o)    (OFF0 + o * sizeof (uintptr_t) - SET(s))
#define NUM(s,n)    (NUM0 + n * sizeof (FcChar16) - SET(s))
#define LEAF(o,l)   (LEAF0 + l * sizeof (FcCharLeaf) - (OFF0 + o * sizeof (intptr_t)))
#define fcLangCharSets (fcLangData.langCharSets)
#define fcLangCharSetIndices (fcLangData.langIndices)
#define fcLangCharSetIndicesInv (fcLangData.langIndicesInv)

static const struct {
    FcLangCharSet  langCharSets[244];
    FcCharLeaf     leaves[617];
    uintptr_t      leaf_offsets[667];
    FcChar16       numbers[667];
    FcChar8        langIndices[244];
    FcChar8        langIndicesInv[244];
} fcLangData = {
{
    { "aa",  { FC_REF_CONSTANT, 1, OFF(0,0), NUM(0,0) } }, /* 0 */
    { "ab",  { FC_REF_CONSTANT, 1, OFF(1,1), NUM(1,1) } }, /* 1 */
    { "af",  { FC_REF_CONSTANT, 2, OFF(2,2), NUM(2,2) } }, /* 2 */
    { "ak",  { FC_REF_CONSTANT, 5, OFF(3,4), NUM(3,4) } }, /* 3 */
    { "am",  { FC_REF_CONSTANT, 2, OFF(4,9), NUM(4,9) } }, /* 4 */
    { "an",  { FC_REF_CONSTANT, 1, OFF(5,11), NUM(5,11) } }, /* 5 */
    { "ar",  { FC_REF_CONSTANT, 1, OFF(6,12), NUM(6,12) } }, /* 6 */
    { "as",  { FC_REF_CONSTANT, 1, OFF(7,13), NUM(7,13) } }, /* 7 */
    { "ast",  { FC_REF_CONSTANT, 2, OFF(8,14), NUM(8,14) } }, /* 8 */
    { "av",  { FC_REF_CONSTANT, 1, OFF(9,16), NUM(9,16) } }, /* 9 */
    { "ay",  { FC_REF_CONSTANT, 1, OFF(10,17), NUM(10,17) } }, /* 10 */
    { "az-az",  { FC_REF_CONSTANT, 3, OFF(11,18), NUM(11,18) } }, /* 11 */
    { "az-ir",  { FC_REF_CONSTANT, 1, OFF(12,21), NUM(12,21) } }, /* 12 */
    { "ba",  { FC_REF_CONSTANT, 1, OFF(13,22), NUM(13,22) } }, /* 13 */
    { "be",  { FC_REF_CONSTANT, 1, OFF(14,23), NUM(14,23) } }, /* 14 */
    { "ber-dz",  { FC_REF_CONSTANT, 4, OFF(15,24), NUM(15,24) } }, /* 15 */
    { "ber-ma",  { FC_REF_CONSTANT, 1, OFF(16,28), NUM(16,28) } }, /* 16 */
    { "bg",  { FC_REF_CONSTANT, 1, OFF(17,29), NUM(17,29) } }, /* 17 */
    { "bh",  { FC_REF_CONSTANT, 1, OFF(18,30), NUM(18,30) } }, /* 18 */
    { "bho",  { FC_REF_CONSTANT, 1, OFF(19,30), NUM(19,30) } }, /* 19 */
    { "bi",  { FC_REF_CONSTANT, 1, OFF(20,31), NUM(20,31) } }, /* 20 */
    { "bin",  { FC_REF_CONSTANT, 3, OFF(21,32), NUM(21,32) } }, /* 21 */
    { "bm",  { FC_REF_CONSTANT, 3, OFF(22,35), NUM(22,35) } }, /* 22 */
    { "bn",  { FC_REF_CONSTANT, 1, OFF(23,38), NUM(23,38) } }, /* 23 */
    { "bo",  { FC_REF_CONSTANT, 1, OFF(24,39), NUM(24,39) } }, /* 24 */
    { "br",  { FC_REF_CONSTANT, 1, OFF(25,40), NUM(25,40) } }, /* 25 */
    { "brx",  { FC_REF_CONSTANT, 1, OFF(26,41), NUM(26,41) } }, /* 26 */
    { "bs",  { FC_REF_CONSTANT, 2, OFF(27,42), NUM(27,42) } }, /* 27 */
    { "bua",  { FC_REF_CONSTANT, 1, OFF(28,44), NUM(28,44) } }, /* 28 */
    { "byn",  { FC_REF_CONSTANT, 2, OFF(29,45), NUM(29,45) } }, /* 29 */
    { "ca",  { FC_REF_CONSTANT, 2, OFF(30,47), NUM(30,47) } }, /* 30 */
    { "ce",  { FC_REF_CONSTANT, 1, OFF(31,16), NUM(31,16) } }, /* 31 */
    { "ch",  { FC_REF_CONSTANT, 1, OFF(32,49), NUM(32,49) } }, /* 32 */
    { "chm",  { FC_REF_CONSTANT, 1, OFF(33,50), NUM(33,50) } }, /* 33 */
    { "chr",  { FC_REF_CONSTANT, 1, OFF(34,51), NUM(34,51) } }, /* 34 */
    { "co",  { FC_REF_CONSTANT, 2, OFF(35,52), NUM(35,52) } }, /* 35 */
    { "crh",  { FC_REF_CONSTANT, 2, OFF(36,54), NUM(36,54) } }, /* 36 */
    { "cs",  { FC_REF_CONSTANT, 2, OFF(37,56), NUM(37,56) } }, /* 37 */
    { "csb",  { FC_REF_CONSTANT, 2, OFF(38,58), NUM(38,58) } }, /* 38 */
    { "cu",  { FC_REF_CONSTANT, 1, OFF(39,60), NUM(39,60) } }, /* 39 */
    { "cv",  { FC_REF_CONSTANT, 2, OFF(40,61), NUM(40,61) } }, /* 40 */
    { "cy",  { FC_REF_CONSTANT, 3, OFF(41,63), NUM(41,63) } }, /* 41 */
    { "da",  { FC_REF_CONSTANT, 1, OFF(42,66), NUM(42,66) } }, /* 42 */
    { "de",  { FC_REF_CONSTANT, 1, OFF(43,67), NUM(43,67) } }, /* 43 */
    { "doi",  { FC_REF_CONSTANT, 1, OFF(44,68), NUM(44,68) } }, /* 44 */
    { "dv",  { FC_REF_CONSTANT, 1, OFF(45,69), NUM(45,69) } }, /* 45 */
    { "dz",  { FC_REF_CONSTANT, 1, OFF(46,39), NUM(46,39) } }, /* 46 */
    { "ee",  { FC_REF_CONSTANT, 4, OFF(47,70), NUM(47,70) } }, /* 47 */
    { "el",  { FC_REF_CONSTANT, 1, OFF(48,74), NUM(48,74) } }, /* 48 */
    { "en",  { FC_REF_CONSTANT, 1, OFF(49,75), NUM(49,75) } }, /* 49 */
    { "eo",  { FC_REF_CONSTANT, 2, OFF(50,76), NUM(50,76) } }, /* 50 */
    { "es",  { FC_REF_CONSTANT, 1, OFF(51,11), NUM(51,11) } }, /* 51 */
    { "et",  { FC_REF_CONSTANT, 2, OFF(52,78), NUM(52,78) } }, /* 52 */
    { "eu",  { FC_REF_CONSTANT, 1, OFF(53,80), NUM(53,80) } }, /* 53 */
    { "fa",  { FC_REF_CONSTANT, 1, OFF(54,21), NUM(54,21) } }, /* 54 */
    { "fat",  { FC_REF_CONSTANT, 5, OFF(55,4), NUM(55,4) } }, /* 55 */
    { "ff",  { FC_REF_CONSTANT, 3, OFF(56,81), NUM(56,81) } }, /* 56 */
    { "fi",  { FC_REF_CONSTANT, 2, OFF(57,84), NUM(57,84) } }, /* 57 */
    { "fil",  { FC_REF_CONSTANT, 1, OFF(58,86), NUM(58,86) } }, /* 58 */
    { "fj",  { FC_REF_CONSTANT, 1, OFF(59,87), NUM(59,87) } }, /* 59 */
    { "fo",  { FC_REF_CONSTANT, 1, OFF(60,88), NUM(60,88) } }, /* 60 */
    { "fr",  { FC_REF_CONSTANT, 2, OFF(61,52), NUM(61,52) } }, /* 61 */
    { "fur",  { FC_REF_CONSTANT, 1, OFF(62,89), NUM(62,89) } }, /* 62 */
    { "fy",  { FC_REF_CONSTANT, 1, OFF(63,90), NUM(63,90) } }, /* 63 */
    { "ga",  { FC_REF_CONSTANT, 3, OFF(64,91), NUM(64,91) } }, /* 64 */
    { "gd",  { FC_REF_CONSTANT, 1, OFF(65,94), NUM(65,94) } }, /* 65 */
    { "gez",  { FC_REF_CONSTANT, 2, OFF(66,95), NUM(66,95) } }, /* 66 */
    { "gl",  { FC_REF_CONSTANT, 1, OFF(67,11), NUM(67,11) } }, /* 67 */
    { "gn",  { FC_REF_CONSTANT, 3, OFF(68,97), NUM(68,97) } }, /* 68 */
    { "gu",  { FC_REF_CONSTANT, 1, OFF(69,100), NUM(69,100) } }, /* 69 */
    { "gv",  { FC_REF_CONSTANT, 1, OFF(70,101), NUM(70,101) } }, /* 70 */
    { "ha",  { FC_REF_CONSTANT, 3, OFF(71,102), NUM(71,102) } }, /* 71 */
    { "haw",  { FC_REF_CONSTANT, 3, OFF(72,105), NUM(72,105) } }, /* 72 */
    { "he",  { FC_REF_CONSTANT, 1, OFF(73,108), NUM(73,108) } }, /* 73 */
    { "hi",  { FC_REF_CONSTANT, 1, OFF(74,30), NUM(74,30) } }, /* 74 */
    { "hne",  { FC_REF_CONSTANT, 1, OFF(75,30), NUM(75,30) } }, /* 75 */
    { "ho",  { FC_REF_CONSTANT, 1, OFF(76,87), NUM(76,87) } }, /* 76 */
    { "hr",  { FC_REF_CONSTANT, 2, OFF(77,42), NUM(77,42) } }, /* 77 */
    { "hsb",  { FC_REF_CONSTANT, 2, OFF(78,109), NUM(78,109) } }, /* 78 */
    { "ht",  { FC_REF_CONSTANT, 1, OFF(79,111), NUM(79,111) } }, /* 79 */
    { "hu",  { FC_REF_CONSTANT, 2, OFF(80,112), NUM(80,112) } }, /* 80 */
    { "hy",  { FC_REF_CONSTANT, 1, OFF(81,114), NUM(81,114) } }, /* 81 */
    { "hz",  { FC_REF_CONSTANT, 3, OFF(82,115), NUM(82,115) } }, /* 82 */
    { "ia",  { FC_REF_CONSTANT, 1, OFF(83,87), NUM(83,87) } }, /* 83 */
    { "id",  { FC_REF_CONSTANT, 1, OFF(84,118), NUM(84,118) } }, /* 84 */
    { "ie",  { FC_REF_CONSTANT, 1, OFF(85,87), NUM(85,87) } }, /* 85 */
    { "ig",  { FC_REF_CONSTANT, 2, OFF(86,119), NUM(86,119) } }, /* 86 */
    { "ii",  { FC_REF_CONSTANT, 5, OFF(87,121), NUM(87,121) } }, /* 87 */
    { "ik",  { FC_REF_CONSTANT, 1, OFF(88,126), NUM(88,126) } }, /* 88 */
    { "io",  { FC_REF_CONSTANT, 1, OFF(89,87), NUM(89,87) } }, /* 89 */
    { "is",  { FC_REF_CONSTANT, 1, OFF(90,127), NUM(90,127) } }, /* 90 */
    { "it",  { FC_REF_CONSTANT, 1, OFF(91,128), NUM(91,128) } }, /* 91 */
    { "iu",  { FC_REF_CONSTANT, 3, OFF(92,129), NUM(92,129) } }, /* 92 */
    { "ja",  { FC_REF_CONSTANT, 83, OFF(93,132), NUM(93,132) } }, /* 93 */
    { "jv",  { FC_REF_CONSTANT, 1, OFF(94,215), NUM(94,215) } }, /* 94 */
    { "ka",  { FC_REF_CONSTANT, 1, OFF(95,216), NUM(95,216) } }, /* 95 */
    { "kaa",  { FC_REF_CONSTANT, 1, OFF(96,217), NUM(96,217) } }, /* 96 */
    { "kab",  { FC_REF_CONSTANT, 4, OFF(97,24), NUM(97,24) } }, /* 97 */
    { "ki",  { FC_REF_CONSTANT, 2, OFF(98,218), NUM(98,218) } }, /* 98 */
    { "kj",  { FC_REF_CONSTANT, 1, OFF(99,87), NUM(99,87) } }, /* 99 */
    { "kk",  { FC_REF_CONSTANT, 1, OFF(100,220), NUM(100,220) } }, /* 100 */
    { "kl",  { FC_REF_CONSTANT, 2, OFF(101,221), NUM(101,221) } }, /* 101 */
    { "km",  { FC_REF_CONSTANT, 1, OFF(102,223), NUM(102,223) } }, /* 102 */
    { "kn",  { FC_REF_CONSTANT, 1, OFF(103,224), NUM(103,224) } }, /* 103 */
    { "ko",  { FC_REF_CONSTANT, 45, OFF(104,225), NUM(104,225) } }, /* 104 */
    { "kok",  { FC_REF_CONSTANT, 1, OFF(105,30), NUM(105,30) } }, /* 105 */
    { "kr",  { FC_REF_CONSTANT, 3, OFF(106,270), NUM(106,270) } }, /* 106 */
    { "ks",  { FC_REF_CONSTANT, 1, OFF(107,273), NUM(107,273) } }, /* 107 */
    { "ku-am",  { FC_REF_CONSTANT, 2, OFF(108,274), NUM(108,274) } }, /* 108 */
    { "ku-iq",  { FC_REF_CONSTANT, 1, OFF(109,276), NUM(109,276) } }, /* 109 */
    { "ku-ir",  { FC_REF_CONSTANT, 1, OFF(110,276), NUM(110,276) } }, /* 110 */
    { "ku-tr",  { FC_REF_CONSTANT, 2, OFF(111,277), NUM(111,277) } }, /* 111 */
    { "kum",  { FC_REF_CONSTANT, 1, OFF(112,279), NUM(112,279) } }, /* 112 */
    { "kv",  { FC_REF_CONSTANT, 1, OFF(113,280), NUM(113,280) } }, /* 113 */
    { "kw",  { FC_REF_CONSTANT, 3, OFF(114,281), NUM(114,281) } }, /* 114 */
    { "kwm",  { FC_REF_CONSTANT, 1, OFF(115,87), NUM(115,87) } }, /* 115 */
    { "ky",  { FC_REF_CONSTANT, 1, OFF(116,284), NUM(116,284) } }, /* 116 */
    { "la",  { FC_REF_CONSTANT, 2, OFF(117,285), NUM(117,285) } }, /* 117 */
    { "lah",  { FC_REF_CONSTANT, 1, OFF(118,287), NUM(118,287) } }, /* 118 */
    { "lb",  { FC_REF_CONSTANT, 1, OFF(119,288), NUM(119,288) } }, /* 119 */
    { "lez",  { FC_REF_CONSTANT, 1, OFF(120,16), NUM(120,16) } }, /* 120 */
    { "lg",  { FC_REF_CONSTANT, 2, OFF(121,289), NUM(121,289) } }, /* 121 */
    { "li",  { FC_REF_CONSTANT, 1, OFF(122,291), NUM(122,291) } }, /* 122 */
    { "ln",  { FC_REF_CONSTANT, 4, OFF(123,292), NUM(123,292) } }, /* 123 */
    { "lo",  { FC_REF_CONSTANT, 1, OFF(124,296), NUM(124,296) } }, /* 124 */
    { "lt",  { FC_REF_CONSTANT, 2, OFF(125,297), NUM(125,297) } }, /* 125 */
    { "lv",  { FC_REF_CONSTANT, 2, OFF(126,299), NUM(126,299) } }, /* 126 */
    { "mai",  { FC_REF_CONSTANT, 1, OFF(127,30), NUM(127,30) } }, /* 127 */
    { "mg",  { FC_REF_CONSTANT, 1, OFF(128,301), NUM(128,301) } }, /* 128 */
    { "mh",  { FC_REF_CONSTANT, 2, OFF(129,302), NUM(129,302) } }, /* 129 */
    { "mi",  { FC_REF_CONSTANT, 3, OFF(130,304), NUM(130,304) } }, /* 130 */
    { "mk",  { FC_REF_CONSTANT, 1, OFF(131,307), NUM(131,307) } }, /* 131 */
    { "ml",  { FC_REF_CONSTANT, 1, OFF(132,308), NUM(132,308) } }, /* 132 */
    { "mn-cn",  { FC_REF_CONSTANT, 1, OFF(133,309), NUM(133,309) } }, /* 133 */
    { "mn-mn",  { FC_REF_CONSTANT, 1, OFF(134,310), NUM(134,310) } }, /* 134 */
    { "mni",  { FC_REF_CONSTANT, 1, OFF(135,311), NUM(135,311) } }, /* 135 */
    { "mo",  { FC_REF_CONSTANT, 4, OFF(136,312), NUM(136,312) } }, /* 136 */
    { "mr",  { FC_REF_CONSTANT, 1, OFF(137,30), NUM(137,30) } }, /* 137 */
    { "ms",  { FC_REF_CONSTANT, 1, OFF(138,87), NUM(138,87) } }, /* 138 */
    { "mt",  { FC_REF_CONSTANT, 2, OFF(139,316), NUM(139,316) } }, /* 139 */
    { "my",  { FC_REF_CONSTANT, 1, OFF(140,318), NUM(140,318) } }, /* 140 */
    { "na",  { FC_REF_CONSTANT, 2, OFF(141,319), NUM(141,319) } }, /* 141 */
    { "nb",  { FC_REF_CONSTANT, 1, OFF(142,321), NUM(142,321) } }, /* 142 */
    { "nds",  { FC_REF_CONSTANT, 1, OFF(143,67), NUM(143,67) } }, /* 143 */
    { "ne",  { FC_REF_CONSTANT, 1, OFF(144,322), NUM(144,322) } }, /* 144 */
    { "ng",  { FC_REF_CONSTANT, 1, OFF(145,87), NUM(145,87) } }, /* 145 */
    { "nl",  { FC_REF_CONSTANT, 1, OFF(146,323), NUM(146,323) } }, /* 146 */
    { "nn",  { FC_REF_CONSTANT, 1, OFF(147,324), NUM(147,324) } }, /* 147 */
    { "no",  { FC_REF_CONSTANT, 1, OFF(148,321), NUM(148,321) } }, /* 148 */
    { "nqo",  { FC_REF_CONSTANT, 1, OFF(149,325), NUM(149,325) } }, /* 149 */
    { "nr",  { FC_REF_CONSTANT, 1, OFF(150,87), NUM(150,87) } }, /* 150 */
    { "nso",  { FC_REF_CONSTANT, 2, OFF(151,326), NUM(151,326) } }, /* 151 */
    { "nv",  { FC_REF_CONSTANT, 4, OFF(152,328), NUM(152,328) } }, /* 152 */
    { "ny",  { FC_REF_CONSTANT, 2, OFF(153,332), NUM(153,332) } }, /* 153 */
    { "oc",  { FC_REF_CONSTANT, 1, OFF(154,334), NUM(154,334) } }, /* 154 */
    { "om",  { FC_REF_CONSTANT, 1, OFF(155,87), NUM(155,87) } }, /* 155 */
    { "or",  { FC_REF_CONSTANT, 1, OFF(156,335), NUM(156,335) } }, /* 156 */
    { "os",  { FC_REF_CONSTANT, 1, OFF(157,279), NUM(157,279) } }, /* 157 */
    { "ota",  { FC_REF_CONSTANT, 1, OFF(158,336), NUM(158,336) } }, /* 158 */
    { "pa",  { FC_REF_CONSTANT, 1, OFF(159,337), NUM(159,337) } }, /* 159 */
    { "pa-pk",  { FC_REF_CONSTANT, 1, OFF(160,287), NUM(160,287) } }, /* 160 */
    { "pap-an",  { FC_REF_CONSTANT, 1, OFF(161,338), NUM(161,338) } }, /* 161 */
    { "pap-aw",  { FC_REF_CONSTANT, 1, OFF(162,339), NUM(162,339) } }, /* 162 */
    { "pl",  { FC_REF_CONSTANT, 2, OFF(163,340), NUM(163,340) } }, /* 163 */
    { "ps-af",  { FC_REF_CONSTANT, 1, OFF(164,342), NUM(164,342) } }, /* 164 */
    { "ps-pk",  { FC_REF_CONSTANT, 1, OFF(165,343), NUM(165,343) } }, /* 165 */
    { "pt",  { FC_REF_CONSTANT, 1, OFF(166,344), NUM(166,344) } }, /* 166 */
    { "qu",  { FC_REF_CONSTANT, 2, OFF(167,345), NUM(167,345) } }, /* 167 */
    { "quz",  { FC_REF_CONSTANT, 2, OFF(168,345), NUM(168,345) } }, /* 168 */
    { "rm",  { FC_REF_CONSTANT, 1, OFF(169,347), NUM(169,347) } }, /* 169 */
    { "rn",  { FC_REF_CONSTANT, 1, OFF(170,87), NUM(170,87) } }, /* 170 */
    { "ro",  { FC_REF_CONSTANT, 3, OFF(171,348), NUM(171,348) } }, /* 171 */
    { "ru",  { FC_REF_CONSTANT, 1, OFF(172,279), NUM(172,279) } }, /* 172 */
    { "rw",  { FC_REF_CONSTANT, 1, OFF(173,87), NUM(173,87) } }, /* 173 */
    { "sa",  { FC_REF_CONSTANT, 1, OFF(174,30), NUM(174,30) } }, /* 174 */
    { "sah",  { FC_REF_CONSTANT, 1, OFF(175,351), NUM(175,351) } }, /* 175 */
    { "sat",  { FC_REF_CONSTANT, 1, OFF(176,352), NUM(176,352) } }, /* 176 */
    { "sc",  { FC_REF_CONSTANT, 1, OFF(177,353), NUM(177,353) } }, /* 177 */
    { "sco",  { FC_REF_CONSTANT, 3, OFF(178,354), NUM(178,354) } }, /* 178 */
    { "sd",  { FC_REF_CONSTANT, 1, OFF(179,357), NUM(179,357) } }, /* 179 */
    { "se",  { FC_REF_CONSTANT, 2, OFF(180,358), NUM(180,358) } }, /* 180 */
    { "sel",  { FC_REF_CONSTANT, 1, OFF(181,279), NUM(181,279) } }, /* 181 */
    { "sg",  { FC_REF_CONSTANT, 1, OFF(182,360), NUM(182,360) } }, /* 182 */
    { "sh",  { FC_REF_CONSTANT, 3, OFF(183,361), NUM(183,361) } }, /* 183 */
    { "shs",  { FC_REF_CONSTANT, 2, OFF(184,364), NUM(184,364) } }, /* 184 */
    { "si",  { FC_REF_CONSTANT, 1, OFF(185,366), NUM(185,366) } }, /* 185 */
    { "sid",  { FC_REF_CONSTANT, 2, OFF(186,367), NUM(186,367) } }, /* 186 */
    { "sk",  { FC_REF_CONSTANT, 2, OFF(187,369), NUM(187,369) } }, /* 187 */
    { "sl",  { FC_REF_CONSTANT, 2, OFF(188,42), NUM(188,42) } }, /* 188 */
    { "sm",  { FC_REF_CONSTANT, 2, OFF(189,371), NUM(189,371) } }, /* 189 */
    { "sma",  { FC_REF_CONSTANT, 1, OFF(190,373), NUM(190,373) } }, /* 190 */
    { "smj",  { FC_REF_CONSTANT, 1, OFF(191,374), NUM(191,374) } }, /* 191 */
    { "smn",  { FC_REF_CONSTANT, 2, OFF(192,375), NUM(192,375) } }, /* 192 */
    { "sms",  { FC_REF_CONSTANT, 3, OFF(193,377), NUM(193,377) } }, /* 193 */
    { "sn",  { FC_REF_CONSTANT, 1, OFF(194,87), NUM(194,87) } }, /* 194 */
    { "so",  { FC_REF_CONSTANT, 1, OFF(195,87), NUM(195,87) } }, /* 195 */
    { "sq",  { FC_REF_CONSTANT, 1, OFF(196,380), NUM(196,380) } }, /* 196 */
    { "sr",  { FC_REF_CONSTANT, 1, OFF(197,381), NUM(197,381) } }, /* 197 */
    { "ss",  { FC_REF_CONSTANT, 1, OFF(198,87), NUM(198,87) } }, /* 198 */
    { "st",  { FC_REF_CONSTANT, 1, OFF(199,87), NUM(199,87) } }, /* 199 */
    { "su",  { FC_REF_CONSTANT, 1, OFF(200,118), NUM(200,118) } }, /* 200 */
    { "sv",  { FC_REF_CONSTANT, 1, OFF(201,382), NUM(201,382) } }, /* 201 */
    { "sw",  { FC_REF_CONSTANT, 1, OFF(202,87), NUM(202,87) } }, /* 202 */
    { "syr",  { FC_REF_CONSTANT, 1, OFF(203,383), NUM(203,383) } }, /* 203 */
    { "ta",  { FC_REF_CONSTANT, 1, OFF(204,384), NUM(204,384) } }, /* 204 */
    { "te",  { FC_REF_CONSTANT, 1, OFF(205,385), NUM(205,385) } }, /* 205 */
    { "tg",  { FC_REF_CONSTANT, 1, OFF(206,386), NUM(206,386) } }, /* 206 */
    { "th",  { FC_REF_CONSTANT, 1, OFF(207,387), NUM(207,387) } }, /* 207 */
    { "ti-er",  { FC_REF_CONSTANT, 2, OFF(208,45), NUM(208,45) } }, /* 208 */
    { "ti-et",  { FC_REF_CONSTANT, 2, OFF(209,367), NUM(209,367) } }, /* 209 */
    { "tig",  { FC_REF_CONSTANT, 2, OFF(210,388), NUM(210,388) } }, /* 210 */
    { "tk",  { FC_REF_CONSTANT, 2, OFF(211,390), NUM(211,390) } }, /* 211 */
    { "tl",  { FC_REF_CONSTANT, 1, OFF(212,86), NUM(212,86) } }, /* 212 */
    { "tn",  { FC_REF_CONSTANT, 2, OFF(213,326), NUM(213,326) } }, /* 213 */
    { "to",  { FC_REF_CONSTANT, 2, OFF(214,371), NUM(214,371) } }, /* 214 */
    { "tr",  { FC_REF_CONSTANT, 2, OFF(215,392), NUM(215,392) } }, /* 215 */
    { "ts",  { FC_REF_CONSTANT, 1, OFF(216,87), NUM(216,87) } }, /* 216 */
    { "tt",  { FC_REF_CONSTANT, 1, OFF(217,394), NUM(217,394) } }, /* 217 */
    { "tw",  { FC_REF_CONSTANT, 5, OFF(218,4), NUM(218,4) } }, /* 218 */
    { "ty",  { FC_REF_CONSTANT, 3, OFF(219,395), NUM(219,395) } }, /* 219 */
    { "tyv",  { FC_REF_CONSTANT, 1, OFF(220,284), NUM(220,284) } }, /* 220 */
    { "ug",  { FC_REF_CONSTANT, 1, OFF(221,398), NUM(221,398) } }, /* 221 */
    { "uk",  { FC_REF_CONSTANT, 1, OFF(222,399), NUM(222,399) } }, /* 222 */
    { "ur",  { FC_REF_CONSTANT, 1, OFF(223,287), NUM(223,287) } }, /* 223 */
    { "uz",  { FC_REF_CONSTANT, 1, OFF(224,87), NUM(224,87) } }, /* 224 */
    { "ve",  { FC_REF_CONSTANT, 2, OFF(225,400), NUM(225,400) } }, /* 225 */
    { "vi",  { FC_REF_CONSTANT, 4, OFF(226,402), NUM(226,402) } }, /* 226 */
    { "vo",  { FC_REF_CONSTANT, 1, OFF(227,406), NUM(227,406) } }, /* 227 */
    { "vot",  { FC_REF_CONSTANT, 2, OFF(228,407), NUM(228,407) } }, /* 228 */
    { "wa",  { FC_REF_CONSTANT, 1, OFF(229,409), NUM(229,409) } }, /* 229 */
    { "wal",  { FC_REF_CONSTANT, 2, OFF(230,367), NUM(230,367) } }, /* 230 */
    { "wen",  { FC_REF_CONSTANT, 2, OFF(231,410), NUM(231,410) } }, /* 231 */
    { "wo",  { FC_REF_CONSTANT, 2, OFF(232,412), NUM(232,412) } }, /* 232 */
    { "xh",  { FC_REF_CONSTANT, 1, OFF(233,87), NUM(233,87) } }, /* 233 */
    { "yap",  { FC_REF_CONSTANT, 1, OFF(234,414), NUM(234,414) } }, /* 234 */
    { "yi",  { FC_REF_CONSTANT, 1, OFF(235,108), NUM(235,108) } }, /* 235 */
    { "yo",  { FC_REF_CONSTANT, 4, OFF(236,415), NUM(236,415) } }, /* 236 */
    { "za",  { FC_REF_CONSTANT, 1, OFF(237,87), NUM(237,87) } }, /* 237 */
    { "zh-cn",  { FC_REF_CONSTANT, 82, OFF(238,419), NUM(238,419) } }, /* 238 */
    { "zh-hk",  { FC_REF_CONSTANT, 83, OFF(239,501), NUM(239,501) } }, /* 239 */
    { "zh-mo",  { FC_REF_CONSTANT, 83, OFF(240,501), NUM(240,501) } }, /* 240 */
    { "zh-sg",  { FC_REF_CONSTANT, 82, OFF(241,419), NUM(241,419) } }, /* 241 */
    { "zh-tw",  { FC_REF_CONSTANT, 83, OFF(242,584), NUM(242,584) } }, /* 242 */
    { "zu",  { FC_REF_CONSTANT, 1, OFF(243,87), NUM(243,87) } }, /* 243 */
},
{
    { { /* 0 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x08104404, 0x08104404,
    } },
    { { /* 1 */
    0xffff8002, 0xffffffff, 0x8002ffff, 0x00000000,
    0xc0000000, 0xf0fc33c0, 0x03000000, 0x00000003,
    } },
    { { /* 2 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0810cf00, 0x0810cf00,
    } },
    { { /* 3 */
    0x00000000, 0x00000000, 0x00000200, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 4 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00220008, 0x00220008,
    } },
    { { /* 5 */
    0x00000000, 0x00000300, 0x00000000, 0x00000300,
    0x00010040, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 6 */
    0x00000000, 0x00000000, 0x08100000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 7 */
    0x00000048, 0x00000200, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 8 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x30000000, 0x00000000, 0x03000000,
    } },
    { { /* 9 */
    0xff7fff7f, 0xff01ff7f, 0x00003d7f, 0xffff7fff,
    0xffff3d7f, 0x003d7fff, 0xff7f7f00, 0x00ff7fff,
    } },
    { { /* 10 */
    0x003d7fff, 0xffffffff, 0x007fff7f, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 11 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x140a2202, 0x140a2202,
    } },
    { { /* 12 */
    0x00000000, 0x07fffffe, 0x000007fe, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 13 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfff99fee, 0xd3c4fdff, 0xb000399f, 0x00030000,
    } },
    { { /* 14 */
    0x00000000, 0x00c00030, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 15 */
    0xffff0042, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 16 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10028010, 0x10028010,
    } },
    { { /* 17 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10400080, 0x10400080,
    } },
    { { /* 18 */
    0xc0000000, 0x00030000, 0xc0000000, 0x00000000,
    0x00008000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 19 */
    0x00000000, 0x00000000, 0x02000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 20 */
    0x00000000, 0x07ffffde, 0x001009f6, 0x40000000,
    0x01000040, 0x00008200, 0x00001000, 0x00000000,
    } },
    { { /* 21 */
    0xffff0000, 0xffffffff, 0x0000ffff, 0x00000000,
    0x030c0000, 0x0c00cc0f, 0x03000000, 0x00000300,
    } },
    { { /* 22 */
    0xffff4040, 0xffffffff, 0x4040ffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 23 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 24 */
    0x00003000, 0x00000000, 0x00000000, 0x00000000,
    0x00110000, 0x00000000, 0x00000000, 0x000000c0,
    } },
    { { /* 25 */
    0x00000000, 0x00000000, 0x08000000, 0x00000008,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 26 */
    0x00003000, 0x00000030, 0x00000000, 0x0000300c,
    0x000c0000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 27 */
    0x00000000, 0x3a8b0000, 0x9e78e6b9, 0x0000802e,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 28 */
    0xffff0000, 0xffffd7ff, 0x0000d7ff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 29 */
    0xffffffe0, 0x83ffffff, 0x00003fff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 30 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10008200, 0x10008200,
    } },
    { { /* 31 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x060c3303, 0x060c3303,
    } },
    { { /* 32 */
    0x00000003, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 33 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x03000000, 0x00003000, 0x00000000,
    } },
    { { /* 34 */
    0x00000000, 0x00000000, 0x00000c00, 0x00000000,
    0x20010040, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 35 */
    0x00000000, 0x00000000, 0x08100000, 0x00040000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 36 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfff99fee, 0xd3c5fdff, 0xb000399f, 0x00000000,
    } },
    { { /* 37 */
    0x00000000, 0x00000000, 0xfffffeff, 0x3d7e03ff,
    0xfeff0003, 0x03ffffff, 0x00000000, 0x00000000,
    } },
    { { /* 38 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x12120404, 0x12120404,
    } },
    { { /* 39 */
    0xfff99fee, 0xf3e5fdff, 0x0007399f, 0x0001ffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 40 */
    0x000330c0, 0x00000000, 0x00000000, 0x60000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 41 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x0c00c000, 0x00000000, 0x00000000,
    } },
    { { /* 42 */
    0xff7fff7f, 0xff01ff00, 0x3d7f3d7f, 0xffff7fff,
    0xffff0000, 0x003d7fff, 0xff7f7f3d, 0x00ff7fff,
    } },
    { { /* 43 */
    0x003d7fff, 0xffffffff, 0x007fff00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 44 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x140ca381, 0x140ca381,
    } },
    { { /* 45 */
    0x00000000, 0x80000000, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 46 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10020004, 0x10020004,
    } },
    { { /* 47 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x00000030, 0x000c0000, 0x030300c0,
    } },
    { { /* 48 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0x001fffff,
    } },
    { { /* 49 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x1a10cfc5, 0x9a10cfc5,
    } },
    { { /* 50 */
    0x00000000, 0x00000000, 0x000c0000, 0x01000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 51 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10420084, 0x10420084,
    } },
    { { /* 52 */
    0xc0000000, 0x00030000, 0xc0000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 53 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x24082202, 0x24082202,
    } },
    { { /* 54 */
    0x0c00f000, 0x00000000, 0x03000180, 0x6000c033,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 55 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x021c0a08, 0x021c0a08,
    } },
    { { /* 56 */
    0x00000030, 0x00000000, 0x0000001e, 0x18000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 57 */
    0xfdffa966, 0xffffdfff, 0xa965dfff, 0x03ffffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 58 */
    0x0000000c, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 59 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x00000c00, 0x00c00000, 0x000c0000,
    } },
    { { /* 60 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0010c604, 0x8010c604,
    } },
    { { /* 61 */
    0x00000000, 0x00000000, 0x00000000, 0x01f00000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 62 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0000003f, 0x00000000, 0x00000000, 0x000c0000,
    } },
    { { /* 63 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x25082262, 0x25082262,
    } },
    { { /* 64 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x90400010, 0x10400010,
    } },
    { { /* 65 */
    0xfff99fec, 0xf3e5fdff, 0xf807399f, 0x0000ffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 66 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0x0001ffff, 0x00000000, 0x00000000,
    } },
    { { /* 67 */
    0x0c000000, 0x00000000, 0x00000c00, 0x00000000,
    0x00170240, 0x00040000, 0x001fe000, 0x00000000,
    } },
    { { /* 68 */
    0x00000000, 0x00000000, 0x08500000, 0x00000008,
    0x00000800, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 69 */
    0x00001003, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 70 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffd740, 0xfffffffb, 0x00007fff, 0x00000000,
    } },
    { { /* 71 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00528f81, 0x00528f81,
    } },
    { { /* 72 */
    0x30000300, 0x00300030, 0x30000000, 0x00003000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 73 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10600010, 0x10600010,
    } },
    { { /* 74 */
    0x00000000, 0x00000000, 0x00000000, 0x60000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 75 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10020000, 0x10020000,
    } },
    { { /* 76 */
    0x00000000, 0x00000000, 0x00000c00, 0x00000000,
    0x20000402, 0x00180000, 0x00000000, 0x00000000,
    } },
    { { /* 77 */
    0x00000000, 0x00000000, 0x00880000, 0x00040000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 78 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00400030, 0x00400030,
    } },
    { { /* 79 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0e1e7707, 0x0e1e7707,
    } },
    { { /* 80 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x25092042, 0x25092042,
    } },
    { { /* 81 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x02041107, 0x02041107,
    } },
    { { /* 82 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x9c508e14, 0x1c508e14,
    } },
    { { /* 83 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x04082202, 0x04082202,
    } },
    { { /* 84 */
    0x00000c00, 0x00000003, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 85 */
    0xc0000c0c, 0x00000000, 0x00c00003, 0x00000c03,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 86 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x020c1383, 0x020c1383,
    } },
    { { /* 87 */
    0xff7fff7f, 0xff01ff7f, 0x00003d7f, 0x00ff00ff,
    0x00ff3d7f, 0x003d7fff, 0xff7f7f00, 0x00ff7f00,
    } },
    { { /* 88 */
    0x003d7f00, 0xffff01ff, 0x007fff7f, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 89 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x040a2202, 0x042a220a,
    } },
    { { /* 90 */
    0x00000000, 0x00000200, 0x00000000, 0x00000200,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 91 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x20000000, 0x00000000, 0x02000000,
    } },
    { { /* 92 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfffbafee, 0xf3edfdff, 0x00013bbf, 0x00000001,
    } },
    { { /* 93 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000080, 0x00000080,
    } },
    { { /* 94 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x03000402, 0x00180000, 0x00000000, 0x00000000,
    } },
    { { /* 95 */
    0x00000000, 0x00000000, 0x00880000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 96 */
    0x000c0003, 0x00000c00, 0x00003000, 0x00000c00,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 97 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x08000000, 0x00000000, 0x00000000,
    } },
    { { /* 98 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0x000007ff,
    } },
    { { /* 99 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00080000, 0x00080000,
    } },
    { { /* 100 */
    0x0c0030c0, 0x00000000, 0x0300001e, 0x66000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 101 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00040100, 0x00040100,
    } },
    { { /* 102 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x14482202, 0x14482202,
    } },
    { { /* 103 */
    0x00000000, 0x00000000, 0x00030000, 0x00030000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 104 */
    0x00000000, 0xfffe0000, 0x007fffff, 0xfffffffe,
    0x000000ff, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 105 */
    0x00000000, 0x00008000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 106 */
    0x000c0000, 0x00000000, 0x00000c00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 107 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000200, 0x00000200,
    } },
    { { /* 108 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00003c00, 0x00000030,
    } },
    { { /* 109 */
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    } },
    { { /* 110 */
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00001fff, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 111 */
    0xffff4002, 0xffffffff, 0x4002ffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 112 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x64092242, 0x64092242,
    } },
    { { /* 113 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x060cb301, 0x060cb301,
    } },
    { { /* 114 */
    0x00000c7e, 0x031f8000, 0x0063f200, 0x000df840,
    0x00037e08, 0x08000dfa, 0x0df901bf, 0x5437e400,
    } },
    { { /* 115 */
    0x00000025, 0x40006fc0, 0x27f91be4, 0xdee00000,
    0x007ff83f, 0x00007f7f, 0x00000000, 0x00000000,
    } },
    { { /* 116 */
    0x00000000, 0x00000000, 0x00000000, 0x007f8000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 117 */
    0x000000e7, 0x00000000, 0xfffffffe, 0xffffffff,
    0x780fffff, 0xfffffffe, 0xffffffff, 0x787fffff,
    } },
    { { /* 118 */
    0x43f36f8b, 0x9b462442, 0xe3e0e82c, 0x400a0004,
    0xdb365f65, 0x04497977, 0xe3f0ecd7, 0x08c56038,
    } },
    { { /* 119 */
    0x3403e602, 0x35518000, 0x7eabe0c8, 0x98698200,
    0x2942a948, 0x8060e803, 0xad93441c, 0x4568c03a,
    } },
    { { /* 120 */
    0x8656aa60, 0x02403f7a, 0x14618388, 0x21741020,
    0x07022021, 0x40bc3000, 0x4462a624, 0x0a2060a8,
    } },
    { { /* 121 */
    0x85740217, 0x9c840402, 0x14157bfb, 0x11e27f24,
    0x02efb665, 0x20ff1f75, 0x28403a70, 0x676326c3,
    } },
    { { /* 122 */
    0x20924dd9, 0x0fc946b0, 0x4850bc98, 0xa03f8638,
    0x88162388, 0x52323e09, 0xe3a422aa, 0xc72c00dd,
    } },
    { { /* 123 */
    0x26e1a166, 0x8f0a840b, 0x559e27eb, 0x89bbc241,
    0x85400014, 0x08496361, 0x8ad07f0c, 0x05cfff3e,
    } },
    { { /* 124 */
    0xa803ff1a, 0x7b407a41, 0x80024745, 0x38eb0500,
    0x0005d851, 0x710c9934, 0x01000397, 0x24046366,
    } },
    { { /* 125 */
    0x005180d0, 0x430ac000, 0x30c89071, 0x58000008,
    0xf7000e99, 0x00415f80, 0x941000b0, 0x62800018,
    } },
    { { /* 126 */
    0x09d00240, 0x01568200, 0x08015004, 0x05101d10,
    0x001084c1, 0x10504025, 0x4d8a410f, 0xa60d4009,
    } },
    { { /* 127 */
    0x914cab19, 0x098121c0, 0x0003c485, 0x80000652,
    0x00080b04, 0x0009041d, 0x905c4849, 0x16900009,
    } },
    { { /* 128 */
    0x22200c65, 0x24338412, 0x47960c03, 0x42250a04,
    0x90880028, 0x4f084900, 0xd3aa14a2, 0x3e87d830,
    } },
    { { /* 129 */
    0x1f618604, 0x41867ea4, 0x05b3c390, 0x211857a5,
    0x2a48241e, 0x4a041128, 0x161b0a40, 0x88400d60,
    } },
    { { /* 130 */
    0x9502020a, 0x10608221, 0x04000243, 0x80001444,
    0x0c040000, 0x70000000, 0x00c11a06, 0x0c00024a,
    } },
    { { /* 131 */
    0x00401a00, 0x40451404, 0xbdb30029, 0x052b0a78,
    0xbfa0bba9, 0x8379407c, 0xe81d12fc, 0xc5694bf6,
    } },
    { { /* 132 */
    0x044aeff6, 0xff022115, 0x402bed63, 0x0242d033,
    0x00131000, 0x59ca1b02, 0x020000a0, 0x2c41a703,
    } },
    { { /* 133 */
    0x8ff24880, 0x00000204, 0x10055800, 0x00489200,
    0x20011894, 0x34805004, 0x684c3200, 0x68be49ea,
    } },
    { { /* 134 */
    0x2e42184c, 0x21c9a820, 0x80b050b9, 0xff7c001e,
    0x14e0849a, 0x01e028c1, 0xac49870e, 0xdddb130f,
    } },
    { { /* 135 */
    0x89fbbe1a, 0x51a2a2e0, 0x32ca5502, 0x928b3e46,
    0x438f1dbf, 0x32186703, 0x33c03028, 0xa9230811,
    } },
    { { /* 136 */
    0x3a65c000, 0x04028fe3, 0x86252c4e, 0x00a1bf3d,
    0x8cd43a1a, 0x317c06c9, 0x950a00e0, 0x0edb018b,
    } },
    { { /* 137 */
    0x8c20e34b, 0xf0101182, 0xa7287d94, 0x40fbc9ac,
    0x06534484, 0x44445a90, 0x00013fc8, 0xf5d40048,
    } },
    { { /* 138 */
    0xec577701, 0x891dc442, 0x49286b83, 0xd2424109,
    0x59fe061d, 0x3a221800, 0x3b9fb7e4, 0xc0eaf003,
    } },
    { { /* 139 */
    0x82021386, 0xe4008980, 0x10a1b200, 0x0cc44b80,
    0x8944d309, 0x48341faf, 0x0c458259, 0x0450420a,
    } },
    { { /* 140 */
    0x10c8a040, 0x44503140, 0x01004004, 0x05408280,
    0x442c0108, 0x1a056a30, 0x051420a6, 0x645690cf,
    } },
    { { /* 141 */
    0x31000021, 0xcbf09c18, 0x63e2a120, 0x01b5104c,
    0x9a83538c, 0x3281b8b2, 0x0a84987a, 0x0c0233e7,
    } },
    { { /* 142 */
    0x9018d4cc, 0x9070a1a1, 0xe0048a1e, 0x0451c3d4,
    0x21c2439a, 0x53104844, 0x36400292, 0xf3bd0241,
    } },
    { { /* 143 */
    0xe8f0ab09, 0xa5d27dc0, 0xd24bc242, 0xd0afa43f,
    0x34a11aa0, 0x03d88247, 0x651bc452, 0xc83ad294,
    } },
    { { /* 144 */
    0x40c8001c, 0x33140e06, 0xb21b614f, 0xc0d00088,
    0xa898a02a, 0x166ba1c5, 0x85b42e50, 0x0604c08b,
    } },
    { { /* 145 */
    0x1e04f933, 0xa251056e, 0x76380400, 0x73b8ec07,
    0x18324406, 0xc8164081, 0x63097c8a, 0xaa042980,
    } },
    { { /* 146 */
    0xca9c1c24, 0x27604e0e, 0x83000990, 0x81040046,
    0x10816011, 0x0908540d, 0xcc0a000e, 0x0c000500,
    } },
    { { /* 147 */
    0xa0440430, 0x6784008b, 0x8a195288, 0x8b18865e,
    0x41602e59, 0x9cbe8c10, 0x891c6861, 0x00089800,
    } },
    { { /* 148 */
    0x089a8100, 0x41900018, 0xe4a14007, 0x640d0505,
    0x0e4d310e, 0xff0a4806, 0x2aa81632, 0x000b852e,
    } },
    { { /* 149 */
    0xca841800, 0x696c0e20, 0x16000032, 0x03905658,
    0x1a285120, 0x11248000, 0x432618e1, 0x0eaa5d52,
    } },
    { { /* 150 */
    0xae280fa0, 0x4500fa7b, 0x89406408, 0xc044c880,
    0xb1419005, 0x24c48424, 0x603a1a34, 0xc1949000,
    } },
    { { /* 151 */
    0x003a8246, 0xc106180d, 0x99100022, 0x1511e050,
    0x00824057, 0x020a041a, 0x8930004f, 0x444ad813,
    } },
    { { /* 152 */
    0xed228a02, 0x400510c0, 0x01021000, 0x31018808,
    0x02044600, 0x0708f000, 0xa2008900, 0x22020000,
    } },
    { { /* 153 */
    0x16100200, 0x10400042, 0x02605200, 0x200052f4,
    0x80308510, 0x42021100, 0x80b54308, 0x9a2070e1,
    } },
    { { /* 154 */
    0x08012040, 0xfc653500, 0xab0419c1, 0x62140286,
    0x00440087, 0x02449085, 0x0a85405c, 0x33803207,
    } },
    { { /* 155 */
    0xb8c00400, 0xc0d0ce20, 0x0080c030, 0x0d250508,
    0x00400a90, 0x080c0200, 0x40006505, 0x41026421,
    } },
    { { /* 156 */
    0x00000268, 0x847c0024, 0xde200002, 0x40498619,
    0x40000808, 0x20010084, 0x10108400, 0x01c742cd,
    } },
    { { /* 157 */
    0xd52a7038, 0x1d8f1968, 0x3e12be50, 0x81d92ef5,
    0x2412cec4, 0x732e0828, 0x4b3424ac, 0xd41d020c,
    } },
    { { /* 158 */
    0x80002a02, 0x08110097, 0x114411c4, 0x7d451786,
    0x064949d9, 0x87914000, 0xd8c4254c, 0x491444ba,
    } },
    { { /* 159 */
    0xc8001b92, 0x15800271, 0x0c000081, 0xc200096a,
    0x40024800, 0xba493021, 0x1c802080, 0x1008e2ac,
    } },
    { { /* 160 */
    0x00341004, 0x841400e1, 0x20000020, 0x10149800,
    0x04aa70c2, 0x54208688, 0x04130c62, 0x20109180,
    } },
    { { /* 161 */
    0x02064082, 0x54001c40, 0xe4e90383, 0x84802125,
    0x2000e433, 0xe60944c0, 0x81260a03, 0x080112da,
    } },
    { { /* 162 */
    0x97906901, 0xf8864001, 0x0081e24d, 0xa6510a0e,
    0x81ec011a, 0x8441c600, 0xb62cadb8, 0x8741a46f,
    } },
    { { /* 163 */
    0x4b028d54, 0x02681161, 0x2057bb60, 0x043350a0,
    0xb7b4a8c0, 0x01122402, 0x20009ad3, 0x00c82271,
    } },
    { { /* 164 */
    0x809e2081, 0xe1800c8a, 0x8151b009, 0x40281031,
    0x89a52a0e, 0x620e69b6, 0xd1444425, 0x4d548085,
    } },
    { { /* 165 */
    0x1fb12c75, 0x862dd807, 0x4841d87c, 0x226e414e,
    0x9e088200, 0xed37f80c, 0x75268c80, 0x08149313,
    } },
    { { /* 166 */
    0xc8040e32, 0x6ea6484e, 0x66702c4a, 0xba0126c0,
    0x185dd30c, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 167 */
    0x00000000, 0x05400000, 0x81337020, 0x03a54f81,
    0x641055ec, 0x2344c318, 0x00341462, 0x1a090a43,
    } },
    { { /* 168 */
    0x13a5187b, 0xa8480102, 0xc5440440, 0xe2dd8106,
    0x2d481af0, 0x0416b626, 0x6e405058, 0x31128032,
    } },
    { { /* 169 */
    0x0c0007e4, 0x420a8208, 0x803b4840, 0x87134860,
    0x3428850d, 0xe5290319, 0x870a2345, 0x5c1825a9,
    } },
    { { /* 170 */
    0xd9c577a6, 0x03e85e00, 0xa7000081, 0x41c6cd54,
    0xa2042800, 0x2b0ab860, 0xda9e0020, 0x0e1a08ea,
    } },
    { { /* 171 */
    0x11c0427c, 0x03768908, 0x01058621, 0x18a80000,
    0xc44846a0, 0x20220d05, 0x91485422, 0x28978a01,
    } },
    { { /* 172 */
    0x00087898, 0x31221605, 0x08804240, 0x06a2fa4e,
    0x92110814, 0x9b042002, 0x06432e52, 0x90105000,
    } },
    { { /* 173 */
    0x85ba0041, 0x20203042, 0x05a04f0b, 0x40802708,
    0x1a930591, 0x0600df50, 0x3021a202, 0x4e800630,
    } },
    { { /* 174 */
    0x04c80cc4, 0x8001a004, 0xd4316000, 0x0a020880,
    0x00281c00, 0x00418e18, 0xca106ad0, 0x4b00f210,
    } },
    { { /* 175 */
    0x1506274d, 0x88900220, 0x82a85a00, 0x81504549,
    0x80002004, 0x2c088804, 0x000508d1, 0x4ac48001,
    } },
    { { /* 176 */
    0x0062e020, 0x0a42008e, 0x6a8c3055, 0xe0a5090e,
    0x42c42906, 0x80b34814, 0xb330803e, 0x731c0102,
    } },
    { { /* 177 */
    0x600d1494, 0x09400c20, 0xc040301a, 0xc094a451,
    0x05c88dca, 0xa40c96c2, 0x34040001, 0x011000c8,
    } },
    { { /* 178 */
    0xa9c9550d, 0x1c5a2428, 0x48370142, 0x100f7a4d,
    0x452a32b4, 0x9205317b, 0x5c44b894, 0x458a68d7,
    } },
    { { /* 179 */
    0x2ed15097, 0x42081943, 0x9d40d202, 0x20979840,
    0x064d5409, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 180 */
    0x00000000, 0x84800000, 0x04215542, 0x17001c06,
    0x61107624, 0xb9ddff87, 0x5c0a659f, 0x3c00245d,
    } },
    { { /* 181 */
    0x0059adb0, 0x00000000, 0x00000000, 0x009b28d0,
    0x02000422, 0x44080108, 0xac409804, 0x90288d0a,
    } },
    { { /* 182 */
    0xe0018700, 0x00310400, 0x82211794, 0x10540019,
    0x021a2cb2, 0x40039c02, 0x88043d60, 0x7900080c,
    } },
    { { /* 183 */
    0xba3c1628, 0xcb088640, 0x90807274, 0x0000001e,
    0xd8000000, 0x9c87e188, 0x04124034, 0x2791ae64,
    } },
    { { /* 184 */
    0xe6fbe86b, 0x5366408f, 0x537feea6, 0xb5e4e32b,
    0x0002869f, 0x01228548, 0x08004402, 0x20a02116,
    } },
    { { /* 185 */
    0x02040004, 0x00052000, 0x01547e00, 0x01ac162c,
    0x10852a84, 0x05308c14, 0xb943fbc3, 0x906000ca,
    } },
    { { /* 186 */
    0x40326000, 0x80901200, 0x4c810b30, 0x40020054,
    0x1d6a0029, 0x02802000, 0x00048000, 0x150c2610,
    } },
    { { /* 187 */
    0x07018040, 0x0c24d94d, 0x18502810, 0x50205001,
    0x04d01000, 0x02017080, 0x21c30108, 0x00000132,
    } },
    { { /* 188 */
    0x07190088, 0x05600802, 0x4c0e0012, 0xf0a10405,
    0x00000002, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 189 */
    0x00000000, 0x00000000, 0x00000000, 0x00800000,
    0x035a8e8d, 0x5a0421bd, 0x11703488, 0x00000026,
    } },
    { { /* 190 */
    0x10000000, 0x8804c502, 0xf801b815, 0x25ed147c,
    0x1bb0ed60, 0x1bd70589, 0x1a627af3, 0x0ac50d0c,
    } },
    { { /* 191 */
    0x524ae5d1, 0x63050490, 0x52440354, 0x16122b57,
    0x1101a872, 0x00182949, 0x10080948, 0x886c6000,
    } },
    { { /* 192 */
    0x058f916e, 0x39903012, 0x4930f840, 0x001b8880,
    0x00000000, 0x00428500, 0x98000058, 0x7014ea04,
    } },
    { { /* 193 */
    0x611d1628, 0x60005113, 0x00a71a24, 0x00000000,
    0x03c00000, 0x10187120, 0xa9270172, 0x89066004,
    } },
    { { /* 194 */
    0x020cc022, 0x40810900, 0x8ca0202d, 0x00000e34,
    0x00000000, 0x11012100, 0xc11a8011, 0x0892ec4c,
    } },
    { { /* 195 */
    0x85000040, 0x1806c7ac, 0x0512e03e, 0x00108000,
    0x80ce4008, 0x02106d01, 0x08568641, 0x0027011e,
    } },
    { { /* 196 */
    0x083d3750, 0x4e05e032, 0x048401c0, 0x01400081,
    0x00000000, 0x00000000, 0x00000000, 0x00591aa0,
    } },
    { { /* 197 */
    0x882443c8, 0xc8001d48, 0x72030152, 0x04049013,
    0x04008280, 0x0d148a10, 0x02088056, 0x2704a040,
    } },
    { { /* 198 */
    0x4c000000, 0x00000000, 0x00000000, 0xa3200000,
    0xa0ae1902, 0xdf002660, 0x7b15f010, 0x3ad08121,
    } },
    { { /* 199 */
    0x00284180, 0x48001003, 0x8014cc00, 0x00c414cf,
    0x30202000, 0x00000001, 0x00000000, 0x00000000,
    } },
    { { /* 200 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000300, 0x00000300,
    } },
    { { /* 201 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffff0000, 0x0001ffff,
    } },
    { { /* 202 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x0c0c0000, 0x000cc00c, 0x03000000, 0x00000000,
    } },
    { { /* 203 */
    0x00000000, 0x00000300, 0x00000000, 0x00000300,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 204 */
    0xffff0000, 0xffffffff, 0x0040ffff, 0x00000000,
    0x0c0c0000, 0x0c00000c, 0x03000000, 0x00000300,
    } },
    { { /* 205 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0d10646e, 0x0d10646e,
    } },
    { { /* 206 */
    0x00000000, 0x01000300, 0x00000000, 0x00000300,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 207 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x9fffffff, 0xffcffee7, 0x0000003f, 0x00000000,
    } },
    { { /* 208 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfffddfec, 0xc3effdff, 0x40603ddf, 0x00000003,
    } },
    { { /* 209 */
    0x00000000, 0xfffe0000, 0xffffffff, 0xffffffff,
    0x00007fff, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 210 */
    0x3eff0793, 0x1303b011, 0x11102801, 0x05930000,
    0xb0111e7b, 0x3b019703, 0x00a01112, 0x306b9593,
    } },
    { { /* 211 */
    0x1102b051, 0x11303201, 0x011102b0, 0xb879300a,
    0x30011306, 0x00800010, 0x100b0113, 0x93000011,
    } },
    { { /* 212 */
    0x00102b03, 0x05930000, 0xb051746b, 0x3b011323,
    0x00001030, 0x70000000, 0x1303b011, 0x11102900,
    } },
    { { /* 213 */
    0x00012180, 0xb0153000, 0x3001030e, 0x02000030,
    0x10230111, 0x13000000, 0x10106b81, 0x01130300,
    } },
    { { /* 214 */
    0x30111013, 0x00000100, 0x22b85530, 0x30000000,
    0x9702b011, 0x113afb07, 0x011303b0, 0x00000021,
    } },
    { { /* 215 */
    0x3b0d1b00, 0x03b01138, 0x11330113, 0x13000001,
    0x111c2b05, 0x00000100, 0xb0111000, 0x2a011300,
    } },
    { { /* 216 */
    0x02b01930, 0x10100001, 0x11000000, 0x10300301,
    0x07130230, 0x0011146b, 0x2b051300, 0x8fb8f974,
    } },
    { { /* 217 */
    0x103b0113, 0x00000000, 0xd9700000, 0x01134ab0,
    0x0011103b, 0x00001103, 0x2ab15930, 0x10000111,
    } },
    { { /* 218 */
    0x11010000, 0x00100b01, 0x01130000, 0x0000102b,
    0x20000101, 0x02a01110, 0x30210111, 0x0102b059,
    } },
    { { /* 219 */
    0x19300000, 0x011307b0, 0xb011383b, 0x00000003,
    0x00000000, 0x383b0d13, 0x0103b011, 0x00001000,
    } },
    { { /* 220 */
    0x01130000, 0x00101020, 0x00000100, 0x00000110,
    0x30000000, 0x00021811, 0x00100000, 0x01110000,
    } },
    { { /* 221 */
    0x00000023, 0x0b019300, 0x00301110, 0x302b0111,
    0x13c7b011, 0x01303b01, 0x00000280, 0xb0113000,
    } },
    { { /* 222 */
    0x2b011383, 0x03b01130, 0x300a0011, 0x1102b011,
    0x00002000, 0x01110100, 0xa011102b, 0x2b011302,
    } },
    { { /* 223 */
    0x01000010, 0x30000001, 0x13029011, 0x11302b01,
    0x000066b0, 0xb0113000, 0x6b07d302, 0x07b0113a,
    } },
    { { /* 224 */
    0x00200103, 0x13000000, 0x11386b05, 0x011303b0,
    0x000010b8, 0x2b051b00, 0x03000110, 0x10000000,
    } },
    { { /* 225 */
    0x1102a011, 0x79700a01, 0x0111a2b0, 0x0000100a,
    0x00011100, 0x00901110, 0x00090111, 0x93000000,
    } },
    { { /* 226 */
    0xf9f2bb05, 0x011322b0, 0x2001323b, 0x00000000,
    0x06b05930, 0x303b0193, 0x1123a011, 0x11700000,
    } },
    { { /* 227 */
    0x001102b0, 0x00001010, 0x03011301, 0x00000110,
    0x162b0793, 0x01010010, 0x11300000, 0x01110200,
    } },
    { { /* 228 */
    0xb0113029, 0x00000000, 0x0eb05130, 0x383b0513,
    0x0303b011, 0x00000100, 0x01930000, 0x00001039,
    } },
    { { /* 229 */
    0x3b000302, 0x00000000, 0x00230113, 0x00000000,
    0x00100000, 0x00010000, 0x90113020, 0x00000002,
    } },
    { { /* 230 */
    0x00000000, 0x10000000, 0x11020000, 0x00000301,
    0x01130000, 0xb079b02b, 0x3b011323, 0x02b01130,
    } },
    { { /* 231 */
    0xf0210111, 0x1343b0d9, 0x11303b01, 0x011103b0,
    0xb0517020, 0x20011322, 0x01901110, 0x300b0111,
    } },
    { { /* 232 */
    0x9302b011, 0x0016ab01, 0x01130100, 0xb0113021,
    0x29010302, 0x02b03130, 0x30000000, 0x1b42b819,
    } },
    { { /* 233 */
    0x11383301, 0x00000330, 0x00000020, 0x33051300,
    0x00001110, 0x00000000, 0x93000000, 0x01302305,
    } },
    { { /* 234 */
    0x00010100, 0x30111010, 0x00000100, 0x02301130,
    0x10100001, 0x11000000, 0x00000000, 0x85130200,
    } },
    { { /* 235 */
    0x10111003, 0x2b011300, 0x63b87730, 0x303b0113,
    0x11a2b091, 0x7b300201, 0x011357f0, 0xf0d1702b,
    } },
    { { /* 236 */
    0x1b0111e3, 0x0ab97130, 0x303b0113, 0x13029001,
    0x11302b01, 0x071302b0, 0x3011302b, 0x23011303,
    } },
    { { /* 237 */
    0x02b01130, 0x30ab0113, 0x11feb411, 0x71300901,
    0x05d347b8, 0xb011307b, 0x21015303, 0x00001110,
    } },
    { { /* 238 */
    0x306b0513, 0x1102b011, 0x00103301, 0x05130000,
    0xa01038eb, 0x30000102, 0x02b01110, 0x30200013,
    } },
    { { /* 239 */
    0x0102b071, 0x00101000, 0x01130000, 0x1011100b,
    0x2b011300, 0x00000000, 0x366b0593, 0x1303b095,
    } },
    { { /* 240 */
    0x01103b01, 0x00000200, 0xb0113000, 0x20000103,
    0x01000010, 0x30000000, 0x030ab011, 0x00101001,
    } },
    { { /* 241 */
    0x01110100, 0x00000003, 0x23011302, 0x03000010,
    0x10000000, 0x01000000, 0x00100000, 0x00000290,
    } },
    { { /* 242 */
    0x30113000, 0x7b015386, 0x03b01130, 0x00210151,
    0x13000000, 0x11303b01, 0x001102b0, 0x00011010,
    } },
    { { /* 243 */
    0x2b011302, 0x02001110, 0x10000000, 0x0102b011,
    0x11300100, 0x000102b0, 0x00011010, 0x2b011100,
    } },
    { { /* 244 */
    0x02101110, 0x002b0113, 0x93000000, 0x11302b03,
    0x011302b0, 0x0000303b, 0x00000002, 0x03b01930,
    } },
    { { /* 245 */
    0x102b0113, 0x0103b011, 0x11300000, 0x011302b0,
    0x00001021, 0x00010102, 0x00000010, 0x102b0113,
    } },
    { { /* 246 */
    0x01020011, 0x11302000, 0x011102b0, 0x30113001,
    0x00000002, 0x02b01130, 0x303b0313, 0x0103b011,
    } },
    { { /* 247 */
    0x00002000, 0x05130000, 0xb011303b, 0x10001102,
    0x00000110, 0x142b0113, 0x01000001, 0x01100000,
    } },
    { { /* 248 */
    0x00010280, 0xb0113000, 0x10000102, 0x00000010,
    0x10230113, 0x93021011, 0x11100b05, 0x01130030,
    } },
    { { /* 249 */
    0xb051702b, 0x3b011323, 0x00000030, 0x30000000,
    0x1303b011, 0x11102b01, 0x01010330, 0xb011300a,
    } },
    { { /* 250 */
    0x20000102, 0x00000000, 0x10000011, 0x9300a011,
    0x00102b05, 0x00000200, 0x90111000, 0x29011100,
    } },
    { { /* 251 */
    0x00b01110, 0x30000000, 0x1302b011, 0x11302b21,
    0x000103b0, 0x00000020, 0x2b051300, 0x02b01130,
    } },
    { { /* 252 */
    0x103b0113, 0x13002011, 0x11322b21, 0x00130280,
    0xa0113028, 0x0a011102, 0x02921130, 0x30210111,
    } },
    { { /* 253 */
    0x13020011, 0x11302b01, 0x03d30290, 0x3011122b,
    0x2b011302, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 254 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00004000, 0x00000000, 0x20000000, 0x00000000,
    } },
    { { /* 255 */
    0x00000000, 0x00000000, 0x00003000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 256 */
    0x00000000, 0x040001df, 0x80800176, 0x420c0000,
    0x01020140, 0x44008200, 0x00041018, 0x00000000,
    } },
    { { /* 257 */
    0xffff0000, 0xffff27bf, 0x000027bf, 0x00000000,
    0x00000000, 0x0c000000, 0x03000000, 0x000000c0,
    } },
    { { /* 258 */
    0x3c000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 259 */
    0x00000000, 0x061ef5c0, 0x000001f6, 0x40000000,
    0x01040040, 0x00208210, 0x00005040, 0x00000000,
    } },
    { { /* 260 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x08004480, 0x08004480,
    } },
    { { /* 261 */
    0x00000000, 0x00000000, 0xc0000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 262 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 263 */
    0xffff0042, 0xffffffff, 0x0042ffff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x000000c0,
    } },
    { { /* 264 */
    0x00000000, 0x000c0000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 265 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x0000c00c, 0x00000000, 0x00000000,
    } },
    { { /* 266 */
    0x000c0003, 0x00003c00, 0x0000f000, 0x00003c00,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 267 */
    0x00000000, 0x040001de, 0x00000176, 0x42000000,
    0x01020140, 0x44008200, 0x00041008, 0x00000000,
    } },
    { { /* 268 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x98504f14, 0x18504f14,
    } },
    { { /* 269 */
    0x00000000, 0x00000000, 0x00000c00, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 270 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00480910, 0x00480910,
    } },
    { { /* 271 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0c186606, 0x0c186606,
    } },
    { { /* 272 */
    0x0c000000, 0x00000000, 0x00000000, 0x00000000,
    0x00010040, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 273 */
    0x00001006, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 274 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfef02596, 0x3bffecae, 0x30003f5f, 0x00000000,
    } },
    { { /* 275 */
    0x03c03030, 0x0000c000, 0x00000000, 0x600c0c03,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 276 */
    0x000c3003, 0x18c00c0c, 0x00c03060, 0x60000c03,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 277 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00100002, 0x00100002,
    } },
    { { /* 278 */
    0x00000003, 0x18000000, 0x00003060, 0x00000c00,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 279 */
    0x00000000, 0x00300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 280 */
    0xfdffb729, 0x000001ff, 0xb7290000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 281 */
    0xfffddfec, 0xc3fffdff, 0x00803dcf, 0x00000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 282 */
    0x00000000, 0xffffffff, 0xffffffff, 0x00ffffff,
    0xffffffff, 0x000003ff, 0x00000000, 0x00000000,
    } },
    { { /* 283 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00000000, 0x0000c000, 0x00000000, 0x00000300,
    } },
    { { /* 284 */
    0x00000000, 0x00000000, 0x00000000, 0x00000010,
    0xfff99fee, 0xf3c5fdff, 0xb000798f, 0x0002ffc0,
    } },
    { { /* 285 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00004004, 0x00004004,
    } },
    { { /* 286 */
    0x0f000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 287 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x02045101, 0x02045101,
    } },
    { { /* 288 */
    0x00000c00, 0x000000c3, 0x00000000, 0x18000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 289 */
    0xffffffff, 0x0007f6fb, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 290 */
    0x00000000, 0x00000000, 0x00000000, 0x00000300,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 291 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x011c0661, 0x011c0661,
    } },
    { { /* 292 */
    0xfff98fee, 0xc3e5fdff, 0x0001398f, 0x0001fff0,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 293 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x1c58af16, 0x1c58af16,
    } },
    { { /* 294 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x115c0671, 0x115c0671,
    } },
    { { /* 295 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0x07ffffff,
    } },
    { { /* 296 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00100400, 0x00100400,
    } },
    { { /* 297 */
    0x00000000, 0x00000000, 0x00000000, 0x00000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 298 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00082202, 0x00082202,
    } },
    { { /* 299 */
    0x03000030, 0x0000c000, 0x00000006, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000c00,
    } },
    { { /* 300 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x10000000, 0x00000000, 0x00000000,
    } },
    { { /* 301 */
    0x00000002, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 302 */
    0x00000000, 0x00000000, 0x00000000, 0x00300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 303 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x040c2383, 0x040c2383,
    } },
    { { /* 304 */
    0xfff99fee, 0xf3cdfdff, 0xb0c0398f, 0x00000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 305 */
    0x00000000, 0x07ffffc6, 0x000001fe, 0x40000000,
    0x01000040, 0x0000a000, 0x00001000, 0x00000000,
    } },
    { { /* 306 */
    0xfff987e0, 0xd36dfdff, 0x1e003987, 0x001f0000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 307 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x160e2302, 0x160e2302,
    } },
    { { /* 308 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00020000, 0x00020000,
    } },
    { { /* 309 */
    0x030000f0, 0x00000000, 0x0c00001e, 0x1e000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 310 */
    0x00000000, 0x07ffffde, 0x000005f6, 0x50000000,
    0x05480262, 0x10000a00, 0x00013000, 0x00000000,
    } },
    { { /* 311 */
    0x00000000, 0x07ffffde, 0x000005f6, 0x50000000,
    0x05480262, 0x10000a00, 0x00052000, 0x00000000,
    } },
    { { /* 312 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x143c278f, 0x143c278f,
    } },
    { { /* 313 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000100, 0x00000000,
    } },
    { { /* 314 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x02045301, 0x02045301,
    } },
    { { /* 315 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00300000, 0x0c00c030, 0x03000000, 0x00000000,
    } },
    { { /* 316 */
    0xfff987ee, 0xf325fdff, 0x00013987, 0x0001fff0,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 317 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x02041101, 0x02041101,
    } },
    { { /* 318 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00800000, 0x00000000, 0x00000000,
    } },
    { { /* 319 */
    0x30000000, 0x00000000, 0x00000000, 0x00000000,
    0x00040000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 320 */
    0x00000000, 0x07fffdd6, 0x000005f6, 0xec000000,
    0x0200b4d9, 0x480a8640, 0x00000000, 0x00000000,
    } },
    { { /* 321 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000002, 0x00000002,
    } },
    { { /* 322 */
    0x00033000, 0x00000000, 0x00000c00, 0x600000c3,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 323 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x1850cc14, 0x1850cc14,
    } },
    { { /* 324 */
    0xffff8f04, 0xffffffff, 0x8f04ffff, 0x00000000,
    0x030c0000, 0x0c00cc0f, 0x03000000, 0x00000300,
    } },
    { { /* 325 */
    0x00000000, 0x00800000, 0x03bffbaa, 0x03bffbaa,
    0x00000000, 0x00000000, 0x00002202, 0x00002202,
    } },
    { { /* 326 */
    0x00080000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 327 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfc7e3fec, 0x2ffbffbf, 0x7f5f847f, 0x00040000,
    } },
    { { /* 328 */
    0xff7fff7f, 0xff01ff7f, 0x3d7f3d7f, 0xffff7fff,
    0xffff3d7f, 0x003d7fff, 0xff7f7f3d, 0x00ff7fff,
    } },
    { { /* 329 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x24182212, 0x24182212,
    } },
    { { /* 330 */
    0x0000f000, 0x66000000, 0x00300180, 0x60000033,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 331 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00408030, 0x00408030,
    } },
    { { /* 332 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00020032, 0x00020032,
    } },
    { { /* 333 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000016, 0x00000016,
    } },
    { { /* 334 */
    0x00033000, 0x00000000, 0x00000c00, 0x60000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 335 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00200034, 0x00200034,
    } },
    { { /* 336 */
    0x00033000, 0x00000000, 0x00000c00, 0x60000003,
    0x00000000, 0x00800000, 0x00000000, 0x0000c3f0,
    } },
    { { /* 337 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00040000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 338 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00000880, 0x00000880,
    } },
    { { /* 339 */
    0xfdff8f04, 0xfdff01ff, 0x8f0401ff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 340 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10400a33, 0x10400a33,
    } },
    { { /* 341 */
    0xffff0000, 0xffff1fff, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 342 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xd63dc7e8, 0xc3bfc718, 0x00803dc7, 0x00000000,
    } },
    { { /* 343 */
    0xfffddfee, 0xc3effdff, 0x00603ddf, 0x00000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 344 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x0c0c0000, 0x00cc0000, 0x00000000, 0x0000c00c,
    } },
    { { /* 345 */
    0xfffffffe, 0x87ffffff, 0x00007fff, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 346 */
    0xff7fff7f, 0xff01ff00, 0x00003d7f, 0xffff7fff,
    0x00ff0000, 0x003d7f7f, 0xff7f7f00, 0x00ff7f00,
    } },
    { { /* 347 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x30400090, 0x30400090,
    } },
    { { /* 348 */
    0x00000000, 0x00000000, 0xc0000180, 0x60000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 349 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x18404084, 0x18404084,
    } },
    { { /* 350 */
    0xffff0002, 0xffffffff, 0x0002ffff, 0x00000000,
    0x00c00000, 0x0c00c00c, 0x03000000, 0x00000000,
    } },
    { { /* 351 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00008000, 0x00008000,
    } },
    { { /* 352 */
    0x00000000, 0x041ed5c0, 0x0000077e, 0x40000000,
    0x01000040, 0x4000a000, 0x002109c0, 0x00000000,
    } },
    { { /* 353 */
    0xffff00d0, 0xffffffff, 0x00d0ffff, 0x00000000,
    0x00030000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 354 */
    0x000c0000, 0x30000000, 0x00000c30, 0x00030000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 355 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x263c370f, 0x263c370f,
    } },
    { { /* 356 */
    0x0003000c, 0x00000300, 0x00000000, 0x00000300,
    0x00000000, 0x00018003, 0x00000000, 0x00000000,
    } },
    { { /* 357 */
    0x0800024f, 0x00000008, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 358 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffffffff, 0xffffffff, 0x03ffffff,
    } },
    { { /* 359 */
    0x00000000, 0x00000000, 0x077dfffe, 0x077dfffe,
    0x00000000, 0x00000000, 0x10400010, 0x10400010,
    } },
    { { /* 360 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x10400010, 0x10400010,
    } },
    { { /* 361 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x081047a4, 0x081047a4,
    } },
    { { /* 362 */
    0x0c0030c0, 0x00000000, 0x0f30001e, 0x66000003,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 363 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x000a0a09, 0x000a0a09,
    } },
    { { /* 364 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x00400810, 0x00400810,
    } },
    { { /* 365 */
    0x00000000, 0x00000000, 0x07fffffe, 0x07fffffe,
    0x00000000, 0x00000000, 0x0e3c770f, 0x0e3c770f,
    } },
    { { /* 366 */
    0x0c000000, 0x00000300, 0x00000018, 0x00000300,
    0x00000000, 0x00000000, 0x001fe000, 0x03000000,
    } },
    { { /* 367 */
    0x0000100f, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 368 */
    0x00000000, 0xc0000000, 0x00000000, 0x0000000c,
    0x00000000, 0x33000000, 0x00003000, 0x00000000,
    } },
    { { /* 369 */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000280, 0x00000000,
    } },
    { { /* 370 */
    0x7f7b7f8b, 0xef553db4, 0xf35dfba8, 0x400b0243,
    0x8d3efb40, 0x8c2c7bf7, 0xe3fa6eff, 0xa8ed1d3a,
    } },
    { { /* 371 */
    0xcf83e602, 0x35558cf5, 0xffabe048, 0xd85992b9,
    0x2892ab18, 0x8020d7e9, 0xf583c438, 0x450ae74a,
    } },
    { { /* 372 */
    0x9714b000, 0x54007762, 0x1420d188, 0xc8c01020,
    0x00002121, 0x0c0413a8, 0x04408000, 0x082870c0,
    } },
    { { /* 373 */
    0x000408c0, 0x80000002, 0x14722b7b, 0x3bfb7924,
    0x1ae43327, 0x38ef9835, 0x28029ad1, 0xbf69a813,
    } },
    { { /* 374 */
    0x2fc665cf, 0xafc96b11, 0x5053340f, 0xa00486a2,
    0xe8090106, 0xc00e3f0f, 0x81450a88, 0xc6010010,
    } },
    { { /* 375 */
    0x26e1a161, 0xce00444b, 0xd4eec7aa, 0x85bbcadf,
    0xa5203a74, 0x8840436c, 0x8bd23f06, 0x3befff79,
    } },
    { { /* 376 */
    0xe8eff75a, 0x5b36fbcb, 0x1bfd0d49, 0x39ee0154,
    0x2e75d855, 0xa91abfd8, 0xf6bff3d7, 0xb40c67e0,
    } },
    { { /* 377 */
    0x081382c2, 0xd08bd49d, 0x1061065a, 0x59e074f2,
    0xb3128f9f, 0x6aaa0080, 0xb05e3230, 0x60ac9d7a,
    } },
    { { /* 378 */
    0xc900d303, 0x8a563098, 0x13907000, 0x18421f14,
    0x0008c060, 0x10808008, 0xec900400, 0xe6332817,
    } },
    { { /* 379 */
    0x90000758, 0x4e09f708, 0xfc83f485, 0x18c8af53,
    0x080c187c, 0x01146adf, 0xa734c80c, 0x2710a011,
    } },
    { { /* 380 */
    0x422228c5, 0x00210413, 0x41123010, 0x40001820,
    0xc60c022b, 0x10000300, 0x00220022, 0x02495810,
    } },
    { { /* 381 */
    0x9670a094, 0x1792eeb0, 0x05f2cb96, 0x23580025,
    0x42cc25de, 0x4a04cf38, 0x359f0c40, 0x8a001128,
    } },
    { { /* 382 */
    0x910a13fa, 0x10560229, 0x04200641, 0x84f00484,
    0x0c040000, 0x412c0400, 0x11541206, 0x00020a4b,
    } },
    { { /* 383 */
    0x00c00200, 0x00940000, 0xbfbb0001, 0x242b167c,
    0x7fa89bbb, 0xe3790c7f, 0xe00d10f4, 0x9f014132,
    } },
    { { /* 384 */
    0x35728652, 0xff1210b4, 0x4223cf27, 0x8602c06b,
    0x1fd33106, 0xa1aa3a0c, 0x02040812, 0x08012572,
    } },
    { { /* 385 */
    0x485040cc, 0x601062d0, 0x29001c80, 0x00109a00,
    0x22000004, 0x00800000, 0x68002020, 0x609ecbe6,
    } },
    { { /* 386 */
    0x3f73916e, 0x398260c0, 0x48301034, 0xbd5c0006,
    0xd6fb8cd1, 0x43e820e1, 0x084e0600, 0xc4d00500,
    } },
    { { /* 387 */
    0x89aa8d1f, 0x1602a6e1, 0x21ed0001, 0x1a8b3656,
    0x13a51fb7, 0x30a06502, 0x23c7b278, 0xe9226c93,
    } },
    { { /* 388 */
    0x3a74e47f, 0x98208fe3, 0x2625280e, 0xbf49bf9c,
    0xac543218, 0x1916b949, 0xb5220c60, 0x0659fbc1,
    } },
    { { /* 389 */
    0x8420e343, 0x800008d9, 0x20225500, 0x00a10184,
    0x20104800, 0x40801380, 0x00160d04, 0x80200040,
    } },
    { { /* 390 */
    0x8de7fd40, 0xe0985436, 0x091e7b8b, 0xd249fec8,
    0x8dee0611, 0xba221937, 0x9fdd77f4, 0xf0daf3ec,
    } },
    { { /* 391 */
    0xec424386, 0x26048d3f, 0xc021fa6c, 0x0cc2628e,
    0x0145d785, 0x559977ad, 0x4045e250, 0xa154260b,
    } },
    { { /* 392 */
    0x58199827, 0xa4103443, 0x411405f2, 0x07002280,
    0x426600b4, 0x15a17210, 0x41856025, 0x00000054,
    } },
    { { /* 393 */
    0x01040201, 0xcb70c820, 0x6a629320, 0x0095184c,
    0x9a8b1880, 0x3201aab2, 0x00c4d87a, 0x04c3f3e5,
    } },
    { { /* 394 */
    0xa238d44d, 0x5072a1a1, 0x84fc980a, 0x44d1c152,
    0x20c21094, 0x42104180, 0x3a000000, 0xd29d0240,
    } },
    { { /* 395 */
    0xa8b12f01, 0x2432bd40, 0xd04bd34d, 0xd0ada723,
    0x75a10a92, 0x01e9adac, 0x771f801a, 0xa01b9225,
    } },
    { { /* 396 */
    0x20cadfa1, 0x738c0602, 0x003b577f, 0x00d00bff,
    0x0088806a, 0x0029a1c4, 0x05242a05, 0x16234009,
    } },
    { { /* 397 */
    0x80056822, 0xa2112011, 0x64900004, 0x13824849,
    0x193023d5, 0x08922980, 0x88115402, 0xa0042001,
    } },
    { { /* 398 */
    0x81800400, 0x60228502, 0x0b010090, 0x12020022,
    0x00834011, 0x00001a01, 0x00000000, 0x00000000,
    } },
    { { /* 399 */
    0x00000000, 0x4684009f, 0x020012c8, 0x1a0004fc,
    0x0c4c2ede, 0x80b80402, 0x0afca826, 0x22288c02,
    } },
    { { /* 400 */
    0x8f7ba0e0, 0x2135c7d6, 0xf8b106c7, 0x62550713,
    0x8a19936e, 0xfb0e6efa, 0x48f91630, 0x7debcd2f,
    } },
    { { /* 401 */
    0x4e845892, 0x7a2e4ca0, 0x561eedea, 0x1190c649,
    0xe83a5324, 0x8124cfdb, 0x634218f1, 0x1a8a5853,
    } },
    { { /* 402 */
    0x24d37420, 0x0514aa3b, 0x89586018, 0xc0004800,
    0x91018268, 0x2cd684a4, 0xc4ba8886, 0x02100377,
    } },
    { { /* 403 */
    0x00388244, 0x404aae11, 0x510028c0, 0x15146044,
    0x10007310, 0x02480082, 0x40060205, 0x0000c003,
    } },
    { { /* 404 */
    0x0c020000, 0x02200008, 0x40009000, 0xd161b800,
    0x32744621, 0x3b8af800, 0x8b00050f, 0x2280bbd0,
    } },
    { { /* 405 */
    0x07690600, 0x00438040, 0x50005420, 0x250c41d0,
    0x83108410, 0x02281101, 0x00304008, 0x020040a1,
    } },
    { { /* 406 */
    0x20000040, 0xabe31500, 0xaa443180, 0xc624c2c6,
    0x8004ac13, 0x03d1b000, 0x4285611e, 0x1d9ff303,
    } },
    { { /* 407 */
    0x78e8440a, 0xc3925e26, 0x00852000, 0x4000b001,
    0x88424a90, 0x0c8dca04, 0x4203a705, 0x000422a1,
    } },
    { { /* 408 */
    0x0c018668, 0x10795564, 0xdea00002, 0x40c12000,
    0x5001488b, 0x04000380, 0x50040000, 0x80d0c05d,
    } },
    { { /* 409 */
    0x970aa010, 0x4dafbb20, 0x1e10d921, 0x83140460,
    0xa6d68848, 0x733fd83b, 0x497427bc, 0x92130ddc,
    } },
    { { /* 410 */
    0x8ba1142b, 0xd1392e75, 0x50503009, 0x69008808,
    0x024a49d4, 0x80164010, 0x89d7e564, 0x5316c020,
    } },
    { { /* 411 */
    0x86002b92, 0x15e0a345, 0x0c03008b, 0xe200196e,
    0x80067031, 0xa82916a5, 0x18802000, 0xe1487aac,
    } },
    { { /* 412 */
    0xb5d63207, 0x5f9132e8, 0x20e550a1, 0x10807c00,
    0x9d8a7280, 0x421f00aa, 0x02310e22, 0x04941100,
    } },
    { { /* 413 */
    0x40080022, 0x5c100010, 0xfcc80343, 0x0580a1a5,
    0x04008433, 0x6e080080, 0x81262a4b, 0x2901aad8,
    } },
    { { /* 414 */
    0x4490684d, 0xba880009, 0x00820040, 0x87d10000,
    0xb1e6215b, 0x80083161, 0xc2400800, 0xa600a069,
    } },
    { { /* 415 */
    0x4a328d58, 0x550a5d71, 0x2d579aa0, 0x4aa64005,
    0x30b12021, 0x01123fc6, 0x260a10c2, 0x50824462,
    } },
    { { /* 416 */
    0x80409880, 0x810004c0, 0x00002003, 0x38180000,
    0xf1a60200, 0x720e4434, 0x92e035a2, 0x09008101,
    } },
    { { /* 417 */
    0x00000400, 0x00008885, 0x00000000, 0x00804000,
    0x00000000, 0x00004040, 0x00000000, 0x00000000,
    } },
    { { /* 418 */
    0x00000000, 0x08000000, 0x00000082, 0x00000000,
    0x88000004, 0xe7efbfff, 0xffbfffff, 0xfdffefef,
    } },
    { { /* 419 */
    0xbffefbff, 0x057fffff, 0x85b30034, 0x42164706,
    0xe4105402, 0xb3058092, 0x81305422, 0x180b4263,
    } },
    { { /* 420 */
    0x13f5387b, 0xa9ea07e5, 0x05143c4c, 0x80020600,
    0xbd481ad9, 0xf496ee37, 0x7ec0705f, 0x355fbfb2,
    } },
    { { /* 421 */
    0x455fe644, 0x41469000, 0x063b1d40, 0xfe1362a1,
    0x39028505, 0x0c080548, 0x0000144f, 0x58183488,
    } },
    { { /* 422 */
    0xd8153077, 0x4bfbbd0e, 0x85008a90, 0xe61dc100,
    0xb386ed14, 0x639bff72, 0xd9befd92, 0x0a92887b,
    } },
    { { /* 423 */
    0x1cb2d3fe, 0x177ab980, 0xdc1782c9, 0x3980fffb,
    0x590c4260, 0x37df0f01, 0xb15094a3, 0x23070623,
    } },
    { { /* 424 */
    0x3102f85a, 0x310201f0, 0x1e820040, 0x056a3a0a,
    0x12805b84, 0xa7148002, 0xa04b2612, 0x90011069,
    } },
    { { /* 425 */
    0x848a1000, 0x3f801802, 0x42400708, 0x4e140110,
    0x180080b0, 0x0281c510, 0x10298202, 0x88000210,
    } },
    { { /* 426 */
    0x00420020, 0x11000280, 0x4413e000, 0xfe025804,
    0x30283c07, 0x04739798, 0xcb13ced1, 0x431f6210,
    } },
    { { /* 427 */
    0x55ac278d, 0xc892422e, 0x02885380, 0x78514039,
    0x8088292c, 0x2428b900, 0x080e0c41, 0x42004421,
    } },
    { { /* 428 */
    0x08680408, 0x12040006, 0x02903031, 0xe0855b3e,
    0x10442936, 0x10822814, 0x83344266, 0x531b013c,
    } },
    { { /* 429 */
    0x0e0d0404, 0x00510c22, 0xc0000012, 0x88000040,
    0x0000004a, 0x00000000, 0x5447dff6, 0x00088868,
    } },
    { { /* 430 */
    0x00000081, 0x40000000, 0x00000100, 0x02000000,
    0x00080600, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 431 */
    0x00000080, 0x00000040, 0x00000000, 0x00001040,
    0x00000000, 0xf7fdefff, 0xfffeff7f, 0xfffffbff,
    } },
    { { /* 432 */
    0xbffffdff, 0x00ffffff, 0x042012c2, 0x07080c06,
    0x01101624, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 433 */
    0xe0000000, 0xfffffffe, 0x7f79ffff, 0x00f928df,
    0x80120c32, 0xd53a0008, 0xecc2d858, 0x2fa89d18,
    } },
    { { /* 434 */
    0xe0109620, 0x2622d60c, 0x02060f97, 0x9055b240,
    0x501180a2, 0x04049800, 0x00004000, 0x00000000,
    } },
    { { /* 435 */
    0x00000000, 0x00000000, 0x00000000, 0xfffffbc0,
    0xdffbeffe, 0x62430b08, 0xfb3b41b6, 0x23896f74,
    } },
    { { /* 436 */
    0xecd7ae7f, 0x5960e047, 0x098fa096, 0xa030612c,
    0x2aaa090d, 0x4f7bd44e, 0x388bc4b2, 0x6110a9c6,
    } },
    { { /* 437 */
    0x42000014, 0x0202800c, 0x6485fe48, 0xe3f7d63e,
    0x0c073aa0, 0x0430e40c, 0x1002f680, 0x00000000,
    } },
    { { /* 438 */
    0x00000000, 0x00000000, 0x00000000, 0x00100000,
    0x00004000, 0x00004000, 0x00000100, 0x00000000,
    } },
    { { /* 439 */
    0x00000000, 0x40000000, 0x00000000, 0x00000400,
    0x00008000, 0x00000000, 0x00400400, 0x00000000,
    } },
    { { /* 440 */
    0x00000000, 0x40000000, 0x00000000, 0x00000800,
    0xfebdffe0, 0xffffffff, 0xfbe77f7f, 0xf7ffffbf,
    } },
    { { /* 441 */
    0xefffffff, 0xdff7ff7e, 0xfbdff6f7, 0x804fbffe,
    0x00000000, 0x00000000, 0x00000000, 0x7fffef00,
    } },
    { { /* 442 */
    0xb6f7ff7f, 0xb87e4406, 0x88313bf5, 0x00f41796,
    0x1391a960, 0x72490080, 0x0024f2f3, 0x42c88701,
    } },
    { { /* 443 */
    0x5048e3d3, 0x43052400, 0x4a4c0000, 0x10580227,
    0x01162820, 0x0014a809, 0x00000000, 0x00683ec0,
    } },
    { { /* 444 */
    0x00000000, 0x00000000, 0x00000000, 0xffe00000,
    0xfddbb7ff, 0x000000f7, 0xc72e4000, 0x00000180,
    } },
    { { /* 445 */
    0x00012000, 0x00004000, 0x00300000, 0xb4f7ffa8,
    0x03ffadf3, 0x00000120, 0x00000000, 0x00000000,
    } },
    { { /* 446 */
    0x00000000, 0x00000000, 0x00000000, 0xfffbf000,
    0xfdcf9df7, 0x15c301bf, 0x810a1827, 0x0a00a842,
    } },
    { { /* 447 */
    0x80088108, 0x18048008, 0x0012a3be, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 448 */
    0x00000000, 0x00000000, 0x00000000, 0x90000000,
    0xdc3769e6, 0x3dff6bff, 0xf3f9fcf8, 0x00000004,
    } },
    { { /* 449 */
    0x80000000, 0xe7eebf6f, 0x5da2dffe, 0xc00b3fd8,
    0xa00c0984, 0x69100040, 0xb912e210, 0x5a0086a5,
    } },
    { { /* 450 */
    0x02896800, 0x6a809005, 0x00030010, 0x80000000,
    0x8e001ff9, 0x00000001, 0x00000000, 0x00000000,
    } },
    { { /* 451 */
    0x14000010, 0xa0c09512, 0x0c000200, 0x01000400,
    0x050002a3, 0x98800009, 0x00004000, 0x01004c00,
    } },
    { { /* 452 */
    0x04800008, 0x02840300, 0x00000824, 0x00040000,
    0x00000400, 0x20010904, 0x00001100, 0x22050030,
    } },
    { { /* 453 */
    0x00000108, 0x08104000, 0x01400000, 0x00001040,
    0x00010102, 0x10000040, 0x82012000, 0x10100002,
    } },
    { { /* 454 */
    0x00006002, 0x00000800, 0x00400000, 0x02100401,
    0x14400144, 0x013c4980, 0x880e8288, 0x50102014,
    } },
    { { /* 455 */
    0x80000824, 0x101000c1, 0x02800000, 0x02080101,
    0x04118000, 0x02200112, 0x00031020, 0x02000003,
    } },
    { { /* 456 */
    0x00000002, 0x090c1090, 0xa0004004, 0x60102290,
    0x00080000, 0x00414f45, 0x07071026, 0x40c00001,
    } },
    { { /* 457 */
    0x04580000, 0x0014800a, 0x00002800, 0x00002600,
    0x50988020, 0x02140018, 0x04013800, 0x00008008,
    } },
    { { /* 458 */
    0x41082004, 0x80000928, 0x20080280, 0x020e0a00,
    0x00010040, 0x16110200, 0x41800002, 0x08231400,
    } },
    { { /* 459 */
    0x40020020, 0x0080202f, 0x2015a008, 0x1c000002,
    0xc0040e00, 0x82028012, 0x00400000, 0x2002a004,
    } },
    { { /* 460 */
    0x20200001, 0xa0040000, 0x8890004c, 0xc4000080,
    0x10012500, 0x48100482, 0x60800110, 0x40008040,
    } },
    { { /* 461 */
    0x00040008, 0x04000044, 0x90000091, 0x000c1200,
    0x06040000, 0x08610480, 0x10010800, 0x080d0001,
    } },
    { { /* 462 */
    0x800204b4, 0x00140000, 0x00000000, 0x00200020,
    0x84100200, 0x01811000, 0x02000210, 0x03018800,
    } },
    { { /* 463 */
    0x04042804, 0x20001c92, 0x02100020, 0x4202490a,
    0x02420146, 0x00000803, 0x0008c008, 0x44050010,
    } },
    { { /* 464 */
    0x80222000, 0x00000800, 0x00008452, 0x10502140,
    0xe0410005, 0x00000400, 0x00a00008, 0x80080000,
    } },
    { { /* 465 */
    0x50180020, 0x00000009, 0x40080600, 0x00000000,
    0x56000020, 0x04000000, 0x00020006, 0x00208220,
    } },
    { { /* 466 */
    0x01210000, 0x40009000, 0x08c00140, 0x08110000,
    0x00004820, 0x02400810, 0x08800002, 0x00200000,
    } },
    { { /* 467 */
    0x00040a00, 0x00004000, 0x40000104, 0x84000000,
    0x02040048, 0x20000000, 0x00012000, 0x1b100000,
    } },
    { { /* 468 */
    0x00007000, 0x04000020, 0x10032000, 0x0804000a,
    0x00000008, 0x04020090, 0x88000014, 0x00000000,
    } },
    { { /* 469 */
    0x00000000, 0x08020008, 0x00040400, 0x40a00000,
    0x40000000, 0x00080090, 0x40800000, 0x20000388,
    } },
    { { /* 470 */
    0x02001080, 0x20010004, 0x12010004, 0x20008011,
    0x13200082, 0x02800000, 0x04098001, 0x00000004,
    } },
    { { /* 471 */
    0x00000000, 0x02801000, 0x00001000, 0x00000100,
    0x20010024, 0x00000050, 0x80200028, 0x00000020,
    } },
    { { /* 472 */
    0x01000000, 0x00a24000, 0x00000000, 0x82001010,
    0x00000800, 0x02000000, 0x40020002, 0x59000044,
    } },
    { { /* 473 */
    0x00000080, 0x0d040000, 0x04000000, 0x10020000,
    0x00022000, 0x00508000, 0x20080001, 0x000004a2,
    } },
    { { /* 474 */
    0xc0020400, 0x00310000, 0x80002000, 0x00002800,
    0x00000b60, 0x40200000, 0x00120000, 0x80000009,
    } },
    { { /* 475 */
    0x41000000, 0x00010008, 0x00880910, 0x20080888,
    0x04044020, 0x80482010, 0x00006000, 0x00020000,
    } },
    { { /* 476 */
    0x42405004, 0x00400020, 0x00000010, 0x00000886,
    0x00008000, 0x80021011, 0x00c00000, 0x42000000,
    } },
    { { /* 477 */
    0x4801201f, 0x40c00004, 0x20600480, 0x00000020,
    0x01000110, 0x22400040, 0x00000428, 0x00000000,
    } },
    { { /* 478 */
    0x0f00020f, 0x40401000, 0x00200048, 0x000c0092,
    0x81000421, 0x00040004, 0x00620001, 0x06000202,
    } },
    { { /* 479 */
    0x14001808, 0x00083800, 0x008c1028, 0x04120028,
    0x22008404, 0x40260880, 0x01100700, 0x00400000,
    } },
    { { /* 480 */
    0x20000020, 0x00200000, 0x00840000, 0x04108000,
    0x00000002, 0x00000000, 0x10000002, 0x04000402,
    } },
    { { /* 481 */
    0x10000000, 0x26a20000, 0x05000200, 0x82204000,
    0x80000000, 0x00048404, 0x80004800, 0x80000400,
    } },
    { { /* 482 */
    0x00000064, 0x00000050, 0x18804000, 0x00060000,
    0x00408002, 0x02020030, 0x00000000, 0x40000000,
    } },
    { { /* 483 */
    0x01208414, 0x00000600, 0x02018000, 0x10400000,
    0x04000840, 0x09200000, 0x2e000000, 0x04000304,
    } },
    { { /* 484 */
    0x00c01810, 0x20100010, 0x10400010, 0x02100000,
    0xa0000402, 0x48200000, 0x06080000, 0x01400000,
    } },
    { { /* 485 */
    0x40000008, 0x00001000, 0x10112800, 0xc2a09080,
    0x00008a02, 0x3a0000e9, 0x80611011, 0x40220000,
    } },
    { { /* 486 */
    0x20000020, 0x48381a00, 0x00028421, 0x54ea0800,
    0x01425100, 0x0490200c, 0x20020000, 0x00600800,
    } },
    { { /* 487 */
    0x00e0c201, 0x00004810, 0x10a10001, 0x00000040,
    0x80108084, 0x00042000, 0x00002000, 0x00000004,
    } },
    { { /* 488 */
    0x00010014, 0x03005d00, 0x00008102, 0x00120000,
    0x51009000, 0x04000480, 0x0021c200, 0x0a888056,
    } },
    { { /* 489 */
    0xd2b60004, 0x13800000, 0x204803a8, 0x04501921,
    0x0a003004, 0x02100010, 0x00091100, 0x01070080,
    } },
    { { /* 490 */
    0x42004020, 0x08300000, 0x002a2444, 0x04046081,
    0x40046008, 0x00120000, 0x10000108, 0x00000000,
    } },
    { { /* 491 */
    0x00000084, 0x08001000, 0x0012e001, 0x045880c0,
    0x00010000, 0x00800022, 0x02401000, 0x00000000,
    } },
    { { /* 492 */
    0x4000d000, 0x00000850, 0x01000009, 0x0d840000,
    0x01080000, 0x42008000, 0x20000828, 0x40100040,
    } },
    { { /* 493 */
    0x51000100, 0x32000000, 0x001a0894, 0x04000040,
    0x00002102, 0x03428000, 0x018c0080, 0x00234010,
    } },
    { { /* 494 */
    0x00000040, 0x185c4000, 0x03000000, 0x40020004,
    0xa20200c9, 0x00000220, 0x00101050, 0x00120004,
    } },
    { { /* 495 */
    0x00000040, 0x44002400, 0x00000228, 0x20000020,
    0x000a0008, 0x18010000, 0x3c08830c, 0x40000684,
    } },
    { { /* 496 */
    0x80101800, 0x02000280, 0x0020000c, 0x08009004,
    0x00040000, 0x0004000c, 0x00018000, 0x14001000,
    } },
    { { /* 497 */
    0x08240000, 0x00200000, 0x20420014, 0x58112000,
    0x10004048, 0x010050c0, 0x0408228c, 0x12282040,
    } },
    { { /* 498 */
    0x00000000, 0x00000020, 0x24002000, 0x00000000,
    0x00800a00, 0x00080910, 0x1019a000, 0x60200030,
    } },
    { { /* 499 */
    0x00000080, 0x00000080, 0x08000000, 0x800050a0,
    0x80044000, 0x04001010, 0x80008080, 0x00000000,
    } },
    { { /* 500 */
    0x00000040, 0x00800000, 0x000c4283, 0x01020000,
    0x00888000, 0x00104008, 0x20000000, 0x04000080,
    } },
    { { /* 501 */
    0x20000104, 0x1802c021, 0x08100000, 0x0000004e,
    0x80000001, 0x30c00080, 0x00000040, 0x00401200,
    } },
    { { /* 502 */
    0x04945288, 0x00940400, 0x06400104, 0x10002000,
    0x00080010, 0x00400420, 0x00000102, 0x00408010,
    } },
    { { /* 503 */
    0x05000000, 0x40002240, 0x00100000, 0x0e400024,
    0x00000080, 0x80000440, 0x01018410, 0xb1804004,
    } },
    { { /* 504 */
    0x25000800, 0x20000000, 0x00800000, 0x0000804c,
    0x10020020, 0x42001000, 0x00082000, 0x00002000,
    } },
    { { /* 505 */
    0x11500020, 0x40004053, 0x11280500, 0x80060014,
    0x004c0101, 0x60002008, 0x44000000, 0x01000036,
    } },
    { { /* 506 */
    0x00010028, 0x01180000, 0x84041804, 0x00098000,
    0x00800000, 0x00000000, 0x00400002, 0x10004001,
    } },
    { { /* 507 */
    0x0051a004, 0x00008100, 0x00000024, 0x40041000,
    0x00040000, 0x00042001, 0x00000000, 0x00008000,
    } },
    { { /* 508 */
    0x00000000, 0x00000000, 0x00000000, 0x20030000,
    0x00001840, 0x00020220, 0x04404002, 0x00204000,
    } },
    { { /* 509 */
    0x01008010, 0x00002080, 0x40008064, 0x00004031,
    0x10018090, 0x80304001, 0x000080a0, 0x80200040,
    } },
    { { /* 510 */
    0x00000001, 0x00000010, 0x00102088, 0x00800020,
    0x00120681, 0x100002a0, 0x00000042, 0x00000080,
    } },
    { { /* 511 */
    0x10000000, 0x21000a00, 0x00000200, 0x40000080,
    0x10110000, 0x00108200, 0x04000000, 0x00000400,
    } },
    { { /* 512 */
    0x80001000, 0x80002000, 0x40003008, 0x00000204,
    0x0801000a, 0x40000001, 0x00000000, 0x00000004,
    } },
    { { /* 513 */
    0x00000000, 0x00000000, 0x00020000, 0x00000000,
    0x88000000, 0x00002000, 0x08502000, 0x00840a00,
    } },
    { { /* 514 */
    0x31061808, 0x00000000, 0x00000000, 0x04000000,
    0x00000004, 0x00000240, 0x00100009, 0x00000000,
    } },
    { { /* 515 */
    0x00004002, 0x04002500, 0x00008040, 0x40a20100,
    0x00000001, 0x12412080, 0x04004008, 0x00042014,
    } },
    { { /* 516 */
    0x02000000, 0x00012000, 0x10000402, 0x000040c0,
    0x00080000, 0x5fe800a1, 0x04019402, 0x02000000,
    } },
    { { /* 517 */
    0x00040100, 0x00880000, 0x00401000, 0x00001012,
    0x00000000, 0x08004100, 0x00000010, 0x00000000,
    } },
    { { /* 518 */
    0x00000000, 0x00000000, 0x52020000, 0x10410080,
    0x00005000, 0x08400200, 0x80400010, 0x44400020,
    } },
    { { /* 519 */
    0x00084100, 0x10200d02, 0xa1200012, 0x00804804,
    0x00008212, 0xc6024000, 0x08100000, 0x205c1828,
    } },
    { { /* 520 */
    0x00000088, 0x00031000, 0x8000013f, 0x21184b44,
    0x100100f2, 0xa9002001, 0x08080840, 0x001b0001,
    } },
    { { /* 521 */
    0x28800112, 0x400020f0, 0x0910200c, 0x0a0010a0,
    0x80000020, 0x00000004, 0x1000000a, 0x00400000,
    } },
    { { /* 522 */
    0x00000000, 0x00002000, 0x00000080, 0x81000000,
    0x02c00020, 0x000004c5, 0x00000000, 0x00100100,
    } },
    { { /* 523 */
    0x20000000, 0x01080000, 0x00400022, 0x08000200,
    0x00408002, 0x20400028, 0x00000000, 0x00100000,
    } },
    { { /* 524 */
    0x08000008, 0x00420002, 0xa0a20003, 0x00022000,
    0x88000280, 0x65160000, 0x00040105, 0x00244041,
    } },
    { { /* 525 */
    0x80300000, 0x00184008, 0x00000880, 0x00201140,
    0x00000000, 0x02900000, 0x50004588, 0x00221043,
    } },
    { { /* 526 */
    0x12004000, 0x0b800000, 0x20002405, 0x0000000c,
    0x08000000, 0x11000410, 0x04000030, 0x00200043,
    } },
    { { /* 527 */
    0x80011000, 0x18008042, 0x11000000, 0x00001008,
    0x00008000, 0x24440000, 0x00800000, 0x80100005,
    } },
    { { /* 528 */
    0x00108204, 0x02102400, 0x00010001, 0x80000200,
    0xa080e80a, 0x00010000, 0x20008000, 0x80122200,
    } },
    { { /* 529 */
    0x88211404, 0x04208041, 0x20088020, 0x18040000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 530 */
    0x00401004, 0x02100002, 0x40014210, 0x50006000,
    0x00080008, 0x20000820, 0x00100211, 0x10000000,
    } },
    { { /* 531 */
    0x91005400, 0x00000000, 0x00000000, 0x08000000,
    0x41610032, 0xa0029d44, 0x000000d2, 0x41020004,
    } },
    { { /* 532 */
    0x00800104, 0x020000c0, 0x04090030, 0x80000204,
    0x82004000, 0x00000020, 0x00000000, 0x00000000,
    } },
    { { /* 533 */
    0x00000000, 0x00000000, 0x00000080, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 534 */
    0xc373ff8b, 0x1b0f6840, 0xf34ce9ac, 0xc0080200,
    0xca3e795c, 0x06487976, 0xf7f02fdf, 0xa8ff033a,
    } },
    { { /* 535 */
    0x233fef37, 0xfd59b004, 0xfffff3ca, 0xfff9de9f,
    0x7df7abff, 0x8eecc000, 0xffdbeebf, 0x45fad003,
    } },
    { { /* 536 */
    0xdffefae1, 0x10abbfef, 0xfcaaffeb, 0x24fdef3f,
    0x7f7678ad, 0xedfff00c, 0x2cfacff6, 0xeb6bf7f9,
    } },
    { { /* 537 */
    0x95bf1ffd, 0xbfbf6677, 0xfeb43bfb, 0x11e27bae,
    0x41bea681, 0x72c31435, 0x71917d70, 0x276b0003,
    } },
    { { /* 538 */
    0x70cf57cb, 0x0def4732, 0xfc747eda, 0xbdb4fe06,
    0x8bca3f9f, 0x58007e49, 0xebec228f, 0xddbb8a5c,
    } },
    { { /* 539 */
    0xb6e7ef60, 0xf293a40f, 0x549e37bb, 0x9bafd04b,
    0xf7d4c414, 0x0a1430b0, 0x88d02f08, 0x192fff7e,
    } },
    { { /* 540 */
    0xfb07ffda, 0x7beb7ff1, 0x0010c5ef, 0xfdff99ff,
    0x056779d7, 0xfdcbffe7, 0x4040c3ff, 0xbd8e6ff7,
    } },
    { { /* 541 */
    0x0497dffa, 0x5bfff4c0, 0xd0e7ed7b, 0xf8e0047e,
    0xb73eff9f, 0x882e7dfe, 0xbe7ffffd, 0xf6c483fe,
    } },
    { { /* 542 */
    0xb8fdf357, 0xef7dd680, 0x47885767, 0xc3dfff7d,
    0x37a9f0ff, 0x70fc7de0, 0xec9a3f6f, 0x86814cb3,
    } },
    { { /* 543 */
    0xdd5c3f9e, 0x4819f70d, 0x0007fea3, 0x38ffaf56,
    0xefb8980d, 0xb760403d, 0x9035d8ce, 0x3fff72bf,
    } },
    { { /* 544 */
    0x7a117ff7, 0xabfff7bb, 0x6fbeff00, 0xfe72a93c,
    0xf11bcfef, 0xf40adb6b, 0xef7ec3e6, 0xf6109b9c,
    } },
    { { /* 545 */
    0x16f4f048, 0x5182feb5, 0x15bbc7b1, 0xfbdf6e87,
    0x63cde43f, 0x7e7ec1ff, 0x7d5ffdeb, 0xfcfe777b,
    } },
    { { /* 546 */
    0xdbea960b, 0x53e86229, 0xfdef37df, 0xbd8136f5,
    0xfcbddc18, 0xffffd2e4, 0xffe03fd7, 0xabf87f6f,
    } },
    { { /* 547 */
    0x6ed99bae, 0xf115f5fb, 0xbdfb79a9, 0xadaf5a3c,
    0x1facdbba, 0x837971fc, 0xc35f7cf7, 0x0567dfff,
    } },
    { { /* 548 */
    0x8467ff9a, 0xdf8b1534, 0x3373f9f3, 0x5e1af7bd,
    0xa03fbf40, 0x01ebffff, 0xcfdddfc0, 0xabd37500,
    } },
    { { /* 549 */
    0xeed6f8c3, 0xb7ff43fd, 0x42275eaf, 0xf6869bac,
    0xf6bc27d7, 0x35b7f787, 0xe176aacd, 0xe29f49e7,
    } },
    { { /* 550 */
    0xaff2545c, 0x61d82b3f, 0xbbb8fc3b, 0x7b7dffcf,
    0x1ce0bf95, 0x43ff7dfd, 0xfffe5ff6, 0xc4ced3ef,
    } },
    { { /* 551 */
    0xadbc8db6, 0x11eb63dc, 0x23d0df59, 0xf3dbbeb4,
    0xdbc71fe7, 0xfae4ff63, 0x63f7b22b, 0xadbaed3b,
    } },
    { { /* 552 */
    0x7efffe01, 0x02bcfff7, 0xef3932ff, 0x8005fffc,
    0xbcf577fb, 0xfff7010d, 0xbf3afffb, 0xdfff0057,
    } },
    { { /* 553 */
    0xbd7def7b, 0xc8d4db88, 0xed7cfff3, 0x56ff5dee,
    0xac5f7e0d, 0xd57fff96, 0xc1403fee, 0xffe76ff9,
    } },
    { { /* 554 */
    0x8e77779b, 0xe45d6ebf, 0x5f1f6fcf, 0xfedfe07f,
    0x01fed7db, 0xfb7bff00, 0x1fdfffd4, 0xfffff800,
    } },
    { { /* 555 */
    0x007bfb8f, 0x7f5cbf00, 0x07f3ffff, 0x3de7eba0,
    0xfbd7f7bf, 0x6003ffbf, 0xbfedfffd, 0x027fefbb,
    } },
    { { /* 556 */
    0xddfdfe40, 0xe2f9fdff, 0xfb1f680b, 0xaffdfbe3,
    0xf7ed9fa4, 0xf80f7a7d, 0x0fd5eebe, 0xfd9fbb5d,
    } },
    { { /* 557 */
    0x3bf9f2db, 0xebccfe7f, 0x73fa876a, 0x9ffc95fc,
    0xfaf7109f, 0xbbcdddb7, 0xeccdf87e, 0x3c3ff366,
    } },
    { { /* 558 */
    0xb03ffffd, 0x067ee9f7, 0xfe0696ae, 0x5fd7d576,
    0xa3f33fd1, 0x6fb7cf07, 0x7f449fd1, 0xd3dd7b59,
    } },
    { { /* 559 */
    0xa9bdaf3b, 0xff3a7dcf, 0xf6ebfbe0, 0xffffb401,
    0xb7bf7afa, 0x0ffdc000, 0xff1fff7f, 0x95fffefc,
    } },
    { { /* 560 */
    0xb5dc0000, 0x3f3eef63, 0x001bfb7f, 0xfbf6e800,
    0xb8df9eef, 0x003fff9f, 0xf5ff7bd0, 0x3fffdfdb,
    } },
    { { /* 561 */
    0x00bffdf0, 0xbbbd8420, 0xffdedf37, 0x0ff3ff6d,
    0x5efb604c, 0xfafbfffb, 0x0219fe5e, 0xf9de79f4,
    } },
    { { /* 562 */
    0xebfaa7f7, 0xff3401eb, 0xef73ebd3, 0xc040afd7,
    0xdcff72bb, 0x2fd8f17f, 0xfe0bb8ec, 0x1f0bdda3,
    } },
    { { /* 563 */
    0x47cf8f1d, 0xffdeb12b, 0xda737fee, 0xcbc424ff,
    0xcbf2f75d, 0xb4edecfd, 0x4dddbff9, 0xfb8d99dd,
    } },
    { { /* 564 */
    0xaf7bbb7f, 0xc959ddfb, 0xfab5fc4f, 0x6d5fafe3,
    0x3f7dffff, 0xffdb7800, 0x7effb6ff, 0x022ffbaf,
    } },
    { { /* 565 */
    0xefc7ff9b, 0xffffffa5, 0xc7000007, 0xfff1f7ff,
    0x01bf7ffd, 0xfdbcdc00, 0xffffbff5, 0x3effff7f,
    } },
    { { /* 566 */
    0xbe000029, 0xff7ff9ff, 0xfd7e6efb, 0x039ecbff,
    0xfbdde300, 0xf6dfccff, 0x117fffff, 0xfbf6f800,
    } },
    { { /* 567 */
    0xd73ce7ef, 0xdfeffeef, 0xedbfc00b, 0xfdcdfedf,
    0x40fd7bf5, 0xb75fffff, 0xf930ffdf, 0xdc97fbdf,
    } },
    { { /* 568 */
    0xbff2fef3, 0xdfbf8fdf, 0xede6177f, 0x35530f7f,
    0x877e447c, 0x45bbfa12, 0x779eede0, 0xbfd98017,
    } },
    { { /* 569 */
    0xde897e55, 0x0447c16f, 0xf75d7ade, 0x290557ff,
    0xfe9586f7, 0xf32f97b3, 0x9f75cfff, 0xfb1771f7,
    } },
    { { /* 570 */
    0xee1934ee, 0xef6137cc, 0xef4c9fd6, 0xfbddd68f,
    0x6def7b73, 0xa431d7fe, 0x97d75e7f, 0xffd80f5b,
    } },
    { { /* 571 */
    0x7bce9d83, 0xdcff22ec, 0xef87763d, 0xfdeddfe7,
    0xa0fc4fff, 0xdbfc3b77, 0x7fdc3ded, 0xf5706fa9,
    } },
    { { /* 572 */
    0x2c403ffb, 0x847fff7f, 0xdeb7ec57, 0xf22fe69c,
    0xd5b50feb, 0xede7afeb, 0xfff08c2f, 0xe8f0537f,
    } },
    { { /* 573 */
    0xb5ffb99d, 0xe78fff66, 0xbe10d981, 0xe3c19c7c,
    0x27339cd1, 0xff6d0cbc, 0xefb7fcb7, 0xffffa0df,
    } },
    { { /* 574 */
    0xfe7bbf0b, 0x353fa3ff, 0x97cd13cc, 0xfb277637,
    0x7e6ccfd6, 0xed31ec50, 0xfc1c677c, 0x5fbff6fa,
    } },
    { { /* 575 */
    0xae2f0fba, 0x7ffea3ad, 0xde74fcf0, 0xf200ffef,
    0xfea2fbbf, 0xbcff3daf, 0x5fb9f694, 0x3f8ff3ad,
    } },
    { { /* 576 */
    0xa01ff26c, 0x01bfffef, 0x70057728, 0xda03ff35,
    0xc7fad2f9, 0x5c1d3fbf, 0xec33ff3a, 0xfe9cb7af,
    } },
    { { /* 577 */
    0x7a9f5236, 0xe722bffa, 0xfcff9ff7, 0xb61d2fbb,
    0x1dfded06, 0xefdf7dd7, 0xf166eb23, 0x0dc07ed9,
    } },
    { { /* 578 */
    0xdfbf3d3d, 0xba83c945, 0x9dd07dd1, 0xcf737b87,
    0xc3f59ff3, 0xc5fedf0d, 0x83020cb3, 0xaec0e879,
    } },
    { { /* 579 */
    0x6f0fc773, 0x093ffd7d, 0x0157fff1, 0x01ff62fb,
    0x3bf3fdb4, 0x43b2b013, 0xff305ed3, 0xeb9f0fff,
    } },
    { { /* 580 */
    0xf203feef, 0xfb893fef, 0x9e9937a9, 0xa72cdef9,
    0xc1f63733, 0xfe3e812e, 0xf2f75d20, 0x69d7d585,
    } },
    { { /* 581 */
    0xffffffff, 0xff6fdb07, 0xd97fc4ff, 0xbe0fefce,
    0xf05ef17b, 0xffb7f6cf, 0xef845ef7, 0x0edfd7cb,
    } },
    { { /* 582 */
    0xfcffff08, 0xffffee3f, 0xd7ff13ff, 0x7ffdaf0f,
    0x1ffabdc7, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 583 */
    0x00000000, 0xe7400000, 0xf933bd38, 0xfeed7feb,
    0x7c767fe8, 0xffefb3f7, 0xd8b7feaf, 0xfbbfff6f,
    } },
    { { /* 584 */
    0xdbf7f8fb, 0xe2f91752, 0x754785c8, 0xe3ef9090,
    0x3f6d9ef4, 0x0536ee2e, 0x7ff3f7bc, 0x7f3fa07b,
    } },
    { { /* 585 */
    0xeb600567, 0x6601babe, 0x583ffcd8, 0x87dfcaf7,
    0xffa0bfcd, 0xfebf5bcd, 0xefa7b6fd, 0xdf9c77ef,
    } },
    { { /* 586 */
    0xf8773fb7, 0xb7fc9d27, 0xdfefcab5, 0xf1b6fb5a,
    0xef1fec39, 0x7ffbfbbf, 0xdafe000d, 0x4e7fbdfb,
    } },
    { { /* 587 */
    0x5ac033ff, 0x9ffebff5, 0x005fffbf, 0xfdf80000,
    0x6ffdffca, 0xa001cffd, 0xfbf2dfff, 0xff7fdfbf,
    } },
    { { /* 588 */
    0x080ffeda, 0xbfffba08, 0xeed77afd, 0x67f9fbeb,
    0xff93e044, 0x9f57df97, 0x08dffef7, 0xfedfdf80,
    } },
    { { /* 589 */
    0xf7feffc5, 0x6803fffb, 0x6bfa67fb, 0x5fe27fff,
    0xff73ffff, 0xe7fb87df, 0xf7a7ebfd, 0xefc7bf7e,
    } },
    { { /* 590 */
    0xdf821ef3, 0xdf7e76ff, 0xda7d79c9, 0x1e9befbe,
    0x77fb7ce0, 0xfffb87be, 0xffdb1bff, 0x4fe03f5c,
    } },
    { { /* 591 */
    0x5f0e7fff, 0xddbf77ff, 0xfffff04f, 0x0ff8ffff,
    0xfddfa3be, 0xfffdfc1c, 0xfb9e1f7d, 0xdedcbdff,
    } },
    { { /* 592 */
    0xbafb3f6f, 0xfbefdf7f, 0x2eec7d1b, 0xf2f7af8e,
    0xcfee7b0f, 0x77c61d96, 0xfff57e07, 0x7fdfd982,
    } },
    { { /* 593 */
    0xc7ff5ee6, 0x79effeee, 0xffcf9a56, 0xde5efe5f,
    0xf9e8896e, 0xe6c4f45e, 0xbe7c0001, 0xdddf3b7f,
    } },
    { { /* 594 */
    0xe9efd59d, 0xde5334ac, 0x4bf7f573, 0x9eff7b4f,
    0x476eb8fe, 0xff450dfb, 0xfbfeabfd, 0xddffe9d7,
    } },
    { { /* 595 */
    0x7fffedf7, 0x7eebddfd, 0xb7ffcfe7, 0xef91bde9,
    0xd77c5d75, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 596 */
    0x00000000, 0xfa800000, 0xb4f1ffee, 0x2fefbf76,
    0x77bfb677, 0xfffd9fbf, 0xf6ae95bf, 0x7f3b75ff,
    } },
    { { /* 597 */
    0x0af9a7f5, 0x00000000, 0x00000000, 0x2bddfbd0,
    0x9a7ff633, 0xd6fcfdab, 0xbfebf9e6, 0xf41fdfdf,
    } },
    { { /* 598 */
    0xffffa6fd, 0xf37b4aff, 0xfef97fb7, 0x1d5cb6ff,
    0xe5ff7ff6, 0x24041f7b, 0xf99ebe05, 0xdff2dbe3,
    } },
    { { /* 599 */
    0xfdff6fef, 0xcbfcd679, 0xefffebfd, 0x0000001f,
    0x98000000, 0x8017e148, 0x00fe6a74, 0xfdf16d7f,
    } },
    { { /* 600 */
    0xfef3b87f, 0xf176e01f, 0x7b3fee96, 0xfffdeb8d,
    0xcbb3adff, 0xe17f84ef, 0xbff04daa, 0xfe3fbf3f,
    } },
    { { /* 601 */
    0xffd7ebff, 0xcf7fffdf, 0x85edfffb, 0x07bcd73f,
    0xfe0faeff, 0x76bffdaf, 0x37bbfaef, 0xa3ba7fdc,
    } },
    { { /* 602 */
    0x56f7b6ff, 0xe7df60f8, 0x4cdfff61, 0xff45b0fb,
    0x3ffa7ded, 0x18fc1fff, 0xe3afffff, 0xdf83c7d3,
    } },
    { { /* 603 */
    0xef7dfb57, 0x1378efff, 0x5ff7fec0, 0x5ee334bb,
    0xeff6f70d, 0x00bfd7fe, 0xf7f7f59d, 0xffe051de,
    } },
    { { /* 604 */
    0x037ffec9, 0xbfef5f01, 0x60a79ff1, 0xf1ffef1d,
    0x0000000f, 0x00000000, 0x00000000, 0x00000000,
    } },
    { { /* 605 */
    0x00000000, 0x00000000, 0x00000000, 0x3c800000,
    0xd91ffb4d, 0xfee37b3a, 0xdc7f3fe9, 0x0000003f,
    } },
    { { /* 606 */
    0x50000000, 0xbe07f51f, 0xf91bfc1d, 0x71ffbc1e,
    0x5bbe6ff9, 0x9b1b5796, 0xfffc7fff, 0xafe7872e,
    } },
    { { /* 607 */
    0xf34febf5, 0xe725dffd, 0x5d440bdc, 0xfddd5747,
    0x7790ed3f, 0x8ac87d7f, 0xf3f9fafa, 0xef4b202a,
    } },
    { { /* 608 */
    0x79cff5ff, 0x0ba5abd3, 0xfb8ff77a, 0x001f8ebd,
    0x00000000, 0xfd4ef300, 0x88001a57, 0x7654aeac,
    } },
    { { /* 609 */
    0xcdff17ad, 0xf42fffb2, 0xdbff5baa, 0x00000002,
    0x73c00000, 0x2e3ff9ea, 0xbbfffa8e, 0xffd376bc,
    } },
    { { /* 610 */
    0x7e72eefe, 0xe7f77ebd, 0xcefdf77f, 0x00000ff5,
    0x00000000, 0xdb9ba900, 0x917fa4c7, 0x7ecef8ca,
    } },
    { { /* 611 */
    0xc7e77d7a, 0xdcaecbbd, 0x8f76fd7e, 0x7cf391d3,
    0x4c2f01e5, 0xa360ed77, 0x5ef807db, 0x21811df7,
    } },
    { { /* 612 */
    0x309c6be0, 0xfade3b3a, 0xc3f57f53, 0x07ba61cd,
    0x00000000, 0x00000000, 0x00000000, 0xbefe26e0,
    } },
    { { /* 613 */
    0xebb503f9, 0xe9cbe36d, 0xbfde9c2f, 0xabbf9f83,
    0xffd51ff7, 0xdffeb7df, 0xffeffdae, 0xeffdfb7e,
    } },
    { { /* 614 */
    0x6ebfaaff, 0x00000000, 0x00000000, 0xb6200000,
    0xbe9e7fcd, 0x58f162b3, 0xfd7bf10d, 0xbefde9f1,
    } },
    { { /* 615 */
    0x5f6dc6c3, 0x69ffff3d, 0xfbf4ffcf, 0x4ff7dcfb,
    0x11372000, 0x00000015, 0x00000000, 0x00000000,
    } },
    { { /* 616 */
    0x00003000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    } },
},
{
    /* aa */
    LEAF(  0,  0),
    /* ab */
    LEAF(  1,  1),
    /* af */
    LEAF(  2,  2), LEAF(  2,  3),
    /* ak */
    LEAF(  4,  4), LEAF(  4,  5), LEAF(  4,  6), LEAF(  4,  7),
    LEAF(  4,  8),
    /* am */
    LEAF(  9,  9), LEAF(  9, 10),
    /* an */
    LEAF( 11, 11),
    /* ar */
    LEAF( 12, 12),
    /* as */
    LEAF( 13, 13),
    /* ast */
    LEAF( 14, 11), LEAF( 14, 14),
    /* av */
    LEAF( 16, 15),
    /* ay */
    LEAF( 17, 16),
    /* az_az */
    LEAF( 18, 17), LEAF( 18, 18), LEAF( 18, 19),
    /* az_ir */
    LEAF( 21, 20),
    /* ba */
    LEAF( 22, 21),
    /* be */
    LEAF( 23, 22),
    /* ber_dz */
    LEAF( 24, 23), LEAF( 24, 24), LEAF( 24, 25), LEAF( 24, 26),
    /* ber_ma */
    LEAF( 28, 27),
    /* bg */
    LEAF( 29, 28),
    /* bh */
    LEAF( 30, 29),
    /* bi */
    LEAF( 31, 30),
    /* bin */
    LEAF( 32, 31), LEAF( 32, 32), LEAF( 32, 33),
    /* bm */
    LEAF( 35, 23), LEAF( 35, 34), LEAF( 35, 35),
    /* bn */
    LEAF( 38, 36),
    /* bo */
    LEAF( 39, 37),
    /* br */
    LEAF( 40, 38),
    /* brx */
    LEAF( 41, 39),
    /* bs */
    LEAF( 42, 23), LEAF( 42, 40),
    /* bua */
    LEAF( 44, 41),
    /* byn */
    LEAF( 45, 42), LEAF( 45, 43),
    /* ca */
    LEAF( 47, 44), LEAF( 47, 45),
    /* ch */
    LEAF( 49, 46),
    /* chm */
    LEAF( 50, 47),
    /* chr */
    LEAF( 51, 48),
    /* co */
    LEAF( 52, 49), LEAF( 52, 50),
    /* crh */
    LEAF( 54, 51), LEAF( 54, 52),
    /* cs */
    LEAF( 56, 53), LEAF( 56, 54),
    /* csb */
    LEAF( 58, 55), LEAF( 58, 56),
    /* cu */
    LEAF( 60, 57),
    /* cv */
    LEAF( 61, 58), LEAF( 61, 59),
    /* cy */
    LEAF( 63, 60), LEAF( 63, 61), LEAF( 63, 62),
    /* da */
    LEAF( 66, 63),
    /* de */
    LEAF( 67, 64),
    /* doi */
    LEAF( 68, 65),
    /* dv */
    LEAF( 69, 66),
    /* ee */
    LEAF( 70, 31), LEAF( 70, 67), LEAF( 70, 68), LEAF( 70, 69),
    /* el */
    LEAF( 74, 70),
    /* en */
    LEAF( 75, 71),
    /* eo */
    LEAF( 76, 23), LEAF( 76, 72),
    /* et */
    LEAF( 78, 73), LEAF( 78, 74),
    /* eu */
    LEAF( 80, 75),
    /* ff */
    LEAF( 81, 23), LEAF( 81, 76), LEAF( 81, 77),
    /* fi */
    LEAF( 84, 78), LEAF( 84, 74),
    /* fil */
    LEAF( 86, 79),
    /* fj */
    LEAF( 87, 23),
    /* fo */
    LEAF( 88, 80),
    /* fur */
    LEAF( 89, 81),
    /* fy */
    LEAF( 90, 82),
    /* ga */
    LEAF( 91, 83), LEAF( 91, 84), LEAF( 91, 85),
    /* gd */
    LEAF( 94, 86),
    /* gez */
    LEAF( 95, 87), LEAF( 95, 88),
    /* gn */
    LEAF( 97, 89), LEAF( 97, 90), LEAF( 97, 91),
    /* gu */
    LEAF(100, 92),
    /* gv */
    LEAF(101, 93),
    /* ha */
    LEAF(102, 23), LEAF(102, 94), LEAF(102, 95),
    /* haw */
    LEAF(105, 23), LEAF(105, 96), LEAF(105, 97),
    /* he */
    LEAF(108, 98),
    /* hsb */
    LEAF(109, 99), LEAF(109,100),
    /* ht */
    LEAF(111,101),
    /* hu */
    LEAF(112,102), LEAF(112,103),
    /* hy */
    LEAF(114,104),
    /* hz */
    LEAF(115, 23), LEAF(115,105), LEAF(115,106),
    /* id */
    LEAF(118,107),
    /* ig */
    LEAF(119, 23), LEAF(119,108),
    /* ii */
    LEAF(121,109), LEAF(121,109), LEAF(121,109), LEAF(121,109),
    LEAF(121,110),
    /* ik */
    LEAF(126,111),
    /* is */
    LEAF(127,112),
    /* it */
    LEAF(128,113),
    /* iu */
    LEAF(129,114), LEAF(129,115), LEAF(129,116),
    /* ja */
    LEAF(132,117), LEAF(132,118), LEAF(132,119), LEAF(132,120),
    LEAF(132,121), LEAF(132,122), LEAF(132,123), LEAF(132,124),
    LEAF(132,125), LEAF(132,126), LEAF(132,127), LEAF(132,128),
    LEAF(132,129), LEAF(132,130), LEAF(132,131), LEAF(132,132),
    LEAF(132,133), LEAF(132,134), LEAF(132,135), LEAF(132,136),
    LEAF(132,137), LEAF(132,138), LEAF(132,139), LEAF(132,140),
    LEAF(132,141), LEAF(132,142), LEAF(132,143), LEAF(132,144),
    LEAF(132,145), LEAF(132,146), LEAF(132,147), LEAF(132,148),
    LEAF(132,149), LEAF(132,150), LEAF(132,151), LEAF(132,152),
    LEAF(132,153), LEAF(132,154), LEAF(132,155), LEAF(132,156),
    LEAF(132,157), LEAF(132,158), LEAF(132,159), LEAF(132,160),
    LEAF(132,161), LEAF(132,162), LEAF(132,163), LEAF(132,164),
    LEAF(132,165), LEAF(132,166), LEAF(132,167), LEAF(132,168),
    LEAF(132,169), LEAF(132,170), LEAF(132,171), LEAF(132,172),
    LEAF(132,173), LEAF(132,174), LEAF(132,175), LEAF(132,176),
    LEAF(132,177), LEAF(132,178), LEAF(132,179), LEAF(132,180),
    LEAF(132,181), LEAF(132,182), LEAF(132,183), LEAF(132,184),
    LEAF(132,185), LEAF(132,186), LEAF(132,187), LEAF(132,188),
    LEAF(132,189), LEAF(132,190), LEAF(132,191), LEAF(132,192),
    LEAF(132,193), LEAF(132,194), LEAF(132,195), LEAF(132,196),
    LEAF(132,197), LEAF(132,198), LEAF(132,199),
    /* jv */
    LEAF(215,200),
    /* ka */
    LEAF(216,201),
    /* kaa */
    LEAF(217,202),
    /* ki */
    LEAF(218, 23), LEAF(218,203),
    /* kk */
    LEAF(220,204),
    /* kl */
    LEAF(221,205), LEAF(221,206),
    /* km */
    LEAF(223,207),
    /* kn */
    LEAF(224,208),
    /* ko */
    LEAF(225,209), LEAF(225,210), LEAF(225,211), LEAF(225,212),
    LEAF(225,213), LEAF(225,214), LEAF(225,215), LEAF(225,216),
    LEAF(225,217), LEAF(225,218), LEAF(225,219), LEAF(225,220),
    LEAF(225,221), LEAF(225,222), LEAF(225,223), LEAF(225,224),
    LEAF(225,225), LEAF(225,226), LEAF(225,227), LEAF(225,228),
    LEAF(225,229), LEAF(225,230), LEAF(225,231), LEAF(225,232),
    LEAF(225,233), LEAF(225,234), LEAF(225,235), LEAF(225,236),
    LEAF(225,237), LEAF(225,238), LEAF(225,239), LEAF(225,240),
    LEAF(225,241), LEAF(225,242), LEAF(225,243), LEAF(225,244),
    LEAF(225,245), LEAF(225,246), LEAF(225,247), LEAF(225,248),
    LEAF(225,249), LEAF(225,250), LEAF(225,251), LEAF(225,252),
    LEAF(225,253),
    /* kr */
    LEAF(270, 23), LEAF(270,254), LEAF(270,255),
    /* ks */
    LEAF(273,256),
    /* ku_am */
    LEAF(274,257), LEAF(274,258),
    /* ku_iq */
    LEAF(276,259),
    /* ku_tr */
    LEAF(277,260), LEAF(277,261),
    /* kum */
    LEAF(279,262),
    /* kv */
    LEAF(280,263),
    /* kw */
    LEAF(281, 23), LEAF(281, 96), LEAF(281,264),
    /* ky */
    LEAF(284,265),
    /* la */
    LEAF(285, 23), LEAF(285,266),
    /* lah */
    LEAF(287,267),
    /* lb */
    LEAF(288,268),
    /* lg */
    LEAF(289, 23), LEAF(289,269),
    /* li */
    LEAF(291,270),
    /* ln */
    LEAF(292,271), LEAF(292,272), LEAF(292,  6), LEAF(292,273),
    /* lo */
    LEAF(296,274),
    /* lt */
    LEAF(297, 23), LEAF(297,275),
    /* lv */
    LEAF(299, 23), LEAF(299,276),
    /* mg */
    LEAF(301,277),
    /* mh */
    LEAF(302, 23), LEAF(302,278),
    /* mi */
    LEAF(304, 23), LEAF(304, 96), LEAF(304,279),
    /* mk */
    LEAF(307,280),
    /* ml */
    LEAF(308,281),
    /* mn_cn */
    LEAF(309,282),
    /* mn_mn */
    LEAF(310,283),
    /* mni */
    LEAF(311,284),
    /* mo */
    LEAF(312,285), LEAF(312, 58), LEAF(312,286), LEAF(312,262),
    /* mt */
    LEAF(316,287), LEAF(316,288),
    /* my */
    LEAF(318,289),
    /* na */
    LEAF(319,  4), LEAF(319,290),
    /* nb */
    LEAF(321,291),
    /* ne */
    LEAF(322,292),
    /* nl */
    LEAF(323,293),
    /* nn */
    LEAF(324,294),
    /* nqo */
    LEAF(325,295),
    /* nso */
    LEAF(326,296), LEAF(326,297),
    /* nv */
    LEAF(328,298), LEAF(328,299), LEAF(328,300), LEAF(328,301),
    /* ny */
    LEAF(332, 23), LEAF(332,302),
    /* oc */
    LEAF(334,303),
    /* or */
    LEAF(335,304),
    /* ota */
    LEAF(336,305),
    /* pa */
    LEAF(337,306),
    /* pap_an */
    LEAF(338,307),
    /* pap_aw */
    LEAF(339,308),
    /* pl */
    LEAF(340, 99), LEAF(340,309),
    /* ps_af */
    LEAF(342,310),
    /* ps_pk */
    LEAF(343,311),
    /* pt */
    LEAF(344,312),
    /* qu */
    LEAF(345,308), LEAF(345,313),
    /* rm */
    LEAF(347,314),
    /* ro */
    LEAF(348,285), LEAF(348, 58), LEAF(348,286),
    /* sah */
    LEAF(351,315),
    /* sat */
    LEAF(352,316),
    /* sc */
    LEAF(353,317),
    /* sco */
    LEAF(354, 23), LEAF(354,318), LEAF(354,319),
    /* sd */
    LEAF(357,320),
    /* se */
    LEAF(358,321), LEAF(358,322),
    /* sg */
    LEAF(360,323),
    /* sh */
    LEAF(361, 23), LEAF(361, 40), LEAF(361,324),
    /* shs */
    LEAF(364,325), LEAF(364,326),
    /* si */
    LEAF(366,327),
    /* sid */
    LEAF(367,328), LEAF(367, 10),
    /* sk */
    LEAF(369,329), LEAF(369,330),
    /* sm */
    LEAF(371, 23), LEAF(371, 97),
    /* sma */
    LEAF(373,331),
    /* smj */
    LEAF(374,332),
    /* smn */
    LEAF(375,333), LEAF(375,334),
    /* sms */
    LEAF(377,335), LEAF(377,336), LEAF(377,337),
    /* sq */
    LEAF(380,338),
    /* sr */
    LEAF(381,339),
    /* sv */
    LEAF(382,340),
    /* syr */
    LEAF(383,341),
    /* ta */
    LEAF(384,342),
    /* te */
    LEAF(385,343),
    /* tg */
    LEAF(386,344),
    /* th */
    LEAF(387,345),
    /* tig */
    LEAF(388,346), LEAF(388, 43),
    /* tk */
    LEAF(390,347), LEAF(390,348),
    /* tr */
    LEAF(392,349), LEAF(392, 52),
    /* tt */
    LEAF(394,350),
    /* ty */
    LEAF(395,351), LEAF(395, 96), LEAF(395,300),
    /* ug */
    LEAF(398,352),
    /* uk */
    LEAF(399,353),
    /* ve */
    LEAF(400, 23), LEAF(400,354),
    /* vi */
    LEAF(402,355), LEAF(402,356), LEAF(402,357), LEAF(402,358),
    /* vo */
    LEAF(406,359),
    /* vot */
    LEAF(407,360), LEAF(407, 74),
    /* wa */
    LEAF(409,361),
    /* wen */
    LEAF(410, 99), LEAF(410,362),
    /* wo */
    LEAF(412,363), LEAF(412,269),
    /* yap */
    LEAF(414,364),
    /* yo */
    LEAF(415,365), LEAF(415,366), LEAF(415,367), LEAF(415,368),
    /* zh_cn */
    LEAF(419,369), LEAF(419,370), LEAF(419,371), LEAF(419,372),
    LEAF(419,373), LEAF(419,374), LEAF(419,375), LEAF(419,376),
    LEAF(419,377), LEAF(419,378), LEAF(419,379), LEAF(419,380),
    LEAF(419,381), LEAF(419,382), LEAF(419,383), LEAF(419,384),
    LEAF(419,385), LEAF(419,386), LEAF(419,387), LEAF(419,388),
    LEAF(419,389), LEAF(419,390), LEAF(419,391), LEAF(419,392),
    LEAF(419,393), LEAF(419,394), LEAF(419,395), LEAF(419,396),
    LEAF(419,397), LEAF(419,398), LEAF(419,399), LEAF(419,400),
    LEAF(419,401), LEAF(419,402), LEAF(419,403), LEAF(419,404),
    LEAF(419,405), LEAF(419,406), LEAF(419,407), LEAF(419,408),
    LEAF(419,409), LEAF(419,410), LEAF(419,411), LEAF(419,412),
    LEAF(419,413), LEAF(419,414), LEAF(419,415), LEAF(419,416),
    LEAF(419,417), LEAF(419,418), LEAF(419,419), LEAF(419,420),
    LEAF(419,421), LEAF(419,422), LEAF(419,423), LEAF(419,424),
    LEAF(419,425), LEAF(419,426), LEAF(419,427), LEAF(419,428),
    LEAF(419,429), LEAF(419,430), LEAF(419,431), LEAF(419,432),
    LEAF(419,433), LEAF(419,434), LEAF(419,435), LEAF(419,436),
    LEAF(419,437), LEAF(419,438), LEAF(419,439), LEAF(419,440),
    LEAF(419,441), LEAF(419,442), LEAF(419,443), LEAF(419,444),
    LEAF(419,445), LEAF(419,446), LEAF(419,447), LEAF(419,448),
    LEAF(419,449), LEAF(419,450),
    /* zh_hk */
    LEAF(501,451), LEAF(501,452), LEAF(501,453), LEAF(501,454),
    LEAF(501,455), LEAF(501,456), LEAF(501,457), LEAF(501,458),
    LEAF(501,459), LEAF(501,460), LEAF(501,461), LEAF(501,462),
    LEAF(501,463), LEAF(501,464), LEAF(501,465), LEAF(501,466),
    LEAF(501,467), LEAF(501,468), LEAF(501,469), LEAF(501,470),
    LEAF(501,471), LEAF(501,472), LEAF(501,473), LEAF(501,474),
    LEAF(501,475), LEAF(501,476), LEAF(501,477), LEAF(501,478),
    LEAF(501,479), LEAF(501,480), LEAF(501,481), LEAF(501,482),
    LEAF(501,483), LEAF(501,484), LEAF(501,485), LEAF(501,486),
    LEAF(501,487), LEAF(501,488), LEAF(501,489), LEAF(501,490),
    LEAF(501,491), LEAF(501,492), LEAF(501,493), LEAF(501,494),
    LEAF(501,495), LEAF(501,496), LEAF(501,497), LEAF(501,498),
    LEAF(501,499), LEAF(501,500), LEAF(501,501), LEAF(501,502),
    LEAF(501,503), LEAF(501,504), LEAF(501,505), LEAF(501,506),
    LEAF(501,507), LEAF(501,508), LEAF(501,509), LEAF(501,510),
    LEAF(501,511), LEAF(501,512), LEAF(501,513), LEAF(501,514),
    LEAF(501,515), LEAF(501,516), LEAF(501,517), LEAF(501,518),
    LEAF(501,519), LEAF(501,520), LEAF(501,521), LEAF(501,522),
    LEAF(501,523), LEAF(501,524), LEAF(501,525), LEAF(501,526),
    LEAF(501,527), LEAF(501,528), LEAF(501,529), LEAF(501,530),
    LEAF(501,531), LEAF(501,532), LEAF(501,533),
    /* zh_tw */
    LEAF(584,534), LEAF(584,535), LEAF(584,536), LEAF(584,537),
    LEAF(584,538), LEAF(584,539), LEAF(584,540), LEAF(584,541),
    LEAF(584,542), LEAF(584,543), LEAF(584,544), LEAF(584,545),
    LEAF(584,546), LEAF(584,547), LEAF(584,548), LEAF(584,549),
    LEAF(584,550), LEAF(584,551), LEAF(584,552), LEAF(584,553),
    LEAF(584,554), LEAF(584,555), LEAF(584,556), LEAF(584,557),
    LEAF(584,558), LEAF(584,559), LEAF(584,560), LEAF(584,561),
    LEAF(584,562), LEAF(584,563), LEAF(584,564), LEAF(584,565),
    LEAF(584,566), LEAF(584,567), LEAF(584,568), LEAF(584,569),
    LEAF(584,570), LEAF(584,571), LEAF(584,572), LEAF(584,573),
    LEAF(584,574), LEAF(584,575), LEAF(584,576), LEAF(584,577),
    LEAF(584,578), LEAF(584,579), LEAF(584,580), LEAF(584,581),
    LEAF(584,582), LEAF(584,583), LEAF(584,584), LEAF(584,585),
    LEAF(584,586), LEAF(584,587), LEAF(584,588), LEAF(584,589),
    LEAF(584,590), LEAF(584,591), LEAF(584,592), LEAF(584,593),
    LEAF(584,594), LEAF(584,595), LEAF(584,596), LEAF(584,597),
    LEAF(584,598), LEAF(584,599), LEAF(584,600), LEAF(584,601),
    LEAF(584,602), LEAF(584,603), LEAF(584,604), LEAF(584,605),
    LEAF(584,606), LEAF(584,607), LEAF(584,608), LEAF(584,609),
    LEAF(584,610), LEAF(584,611), LEAF(584,612), LEAF(584,613),
    LEAF(584,614), LEAF(584,615), LEAF(584,616),
},
{
    /* aa */
    0x0000,
    /* ab */
    0x0004,
    /* af */
    0x0000, 0x0001,
    /* ak */
    0x0000, 0x0001, 0x0002, 0x0003, 0x001e,
    /* am */
    0x0012, 0x0013,
    /* an */
    0x0000,
    /* ar */
    0x0006,
    /* as */
    0x0009,
    /* ast */
    0x0000, 0x001e,
    /* av */
    0x0004,
    /* ay */
    0x0000,
    /* az_az */
    0x0000, 0x0001, 0x0002,
    /* az_ir */
    0x0006,
    /* ba */
    0x0004,
    /* be */
    0x0004,
    /* ber_dz */
    0x0000, 0x0001, 0x0002, 0x001e,
    /* ber_ma */
    0x002d,
    /* bg */
    0x0004,
    /* bh */
    0x0009,
    /* bi */
    0x0000,
    /* bin */
    0x0000, 0x0003, 0x001e,
    /* bm */
    0x0000, 0x0001, 0x0002,
    /* bn */
    0x0009,
    /* bo */
    0x000f,
    /* br */
    0x0000,
    /* brx */
    0x0009,
    /* bs */
    0x0000, 0x0001,
    /* bua */
    0x0004,
    /* byn */
    0x0012, 0x0013,
    /* ca */
    0x0000, 0x0001,
    /* ch */
    0x0000,
    /* chm */
    0x0004,
    /* chr */
    0x0013,
    /* co */
    0x0000, 0x0001,
    /* crh */
    0x0000, 0x0001,
    /* cs */
    0x0000, 0x0001,
    /* csb */
    0x0000, 0x0001,
    /* cu */
    0x0004,
    /* cv */
    0x0001, 0x0004,
    /* cy */
    0x0000, 0x0001, 0x001e,
    /* da */
    0x0000,
    /* de */
    0x0000,
    /* doi */
    0x0009,
    /* dv */
    0x0007,
    /* ee */
    0x0000, 0x0001, 0x0002, 0x0003,
    /* el */
    0x0003,
    /* en */
    0x0000,
    /* eo */
    0x0000, 0x0001,
    /* et */
    0x0000, 0x0001,
    /* eu */
    0x0000,
    /* ff */
    0x0000, 0x0001, 0x0002,
    /* fi */
    0x0000, 0x0001,
    /* fil */
    0x0000,
    /* fj */
    0x0000,
    /* fo */
    0x0000,
    /* fur */
    0x0000,
    /* fy */
    0x0000,
    /* ga */
    0x0000, 0x0001, 0x001e,
    /* gd */
    0x0000,
    /* gez */
    0x0012, 0x0013,
    /* gn */
    0x0000, 0x0001, 0x001e,
    /* gu */
    0x000a,
    /* gv */
    0x0000,
    /* ha */
    0x0000, 0x0001, 0x0002,
    /* haw */
    0x0000, 0x0001, 0x0002,
    /* he */
    0x0005,
    /* hsb */
    0x0000, 0x0001,
    /* ht */
    0x0000,
    /* hu */
    0x0000, 0x0001,
    /* hy */
    0x0005,
    /* hz */
    0x0000, 0x0003, 0x001e,
    /* id */
    0x0000,
    /* ig */
    0x0000, 0x001e,
    /* ii */
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4,
    /* ik */
    0x0004,
    /* is */
    0x0000,
    /* it */
    0x0000,
    /* iu */
    0x0014, 0x0015, 0x0016,
    /* ja */
    0x0030, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c,
    0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c,
    0x007d, 0x007e, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084,
    0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c,
    0x008d, 0x008e, 0x008f, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094,
    0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c,
    0x009d, 0x009e, 0x009f,
    /* jv */
    0x0000,
    /* ka */
    0x0010,
    /* kaa */
    0x0004,
    /* ki */
    0x0000, 0x0001,
    /* kk */
    0x0004,
    /* kl */
    0x0000, 0x0001,
    /* km */
    0x0017,
    /* kn */
    0x000c,
    /* ko */
    0x0031, 0x00ac, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2,
    0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba,
    0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 0x00c0, 0x00c1, 0x00c2,
    0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca,
    0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 0x00d0, 0x00d1, 0x00d2,
    0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    /* kr */
    0x0000, 0x0001, 0x0002,
    /* ks */
    0x0006,
    /* ku_am */
    0x0004, 0x0005,
    /* ku_iq */
    0x0006,
    /* ku_tr */
    0x0000, 0x0001,
    /* kum */
    0x0004,
    /* kv */
    0x0004,
    /* kw */
    0x0000, 0x0001, 0x0002,
    /* ky */
    0x0004,
    /* la */
    0x0000, 0x0001,
    /* lah */
    0x0006,
    /* lb */
    0x0000,
    /* lg */
    0x0000, 0x0001,
    /* li */
    0x0000,
    /* ln */
    0x0000, 0x0001, 0x0002, 0x0003,
    /* lo */
    0x000e,
    /* lt */
    0x0000, 0x0001,
    /* lv */
    0x0000, 0x0001,
    /* mg */
    0x0000,
    /* mh */
    0x0000, 0x0001,
    /* mi */
    0x0000, 0x0001, 0x001e,
    /* mk */
    0x0004,
    /* ml */
    0x000d,
    /* mn_cn */
    0x0018,
    /* mn_mn */
    0x0004,
    /* mni */
    0x0009,
    /* mo */
    0x0000, 0x0001, 0x0002, 0x0004,
    /* mt */
    0x0000, 0x0001,
    /* my */
    0x0010,
    /* na */
    0x0000, 0x0001,
    /* nb */
    0x0000,
    /* ne */
    0x0009,
    /* nl */
    0x0000,
    /* nn */
    0x0000,
    /* nqo */
    0x0007,
    /* nso */
    0x0000, 0x0001,
    /* nv */
    0x0000, 0x0001, 0x0002, 0x0003,
    /* ny */
    0x0000, 0x0001,
    /* oc */
    0x0000,
    /* or */
    0x000b,
    /* ota */
    0x0006,
    /* pa */
    0x000a,
    /* pap_an */
    0x0000,
    /* pap_aw */
    0x0000,
    /* pl */
    0x0000, 0x0001,
    /* ps_af */
    0x0006,
    /* ps_pk */
    0x0006,
    /* pt */
    0x0000,
    /* qu */
    0x0000, 0x0002,
    /* rm */
    0x0000,
    /* ro */
    0x0000, 0x0001, 0x0002,
    /* sah */
    0x0004,
    /* sat */
    0x0009,
    /* sc */
    0x0000,
    /* sco */
    0x0000, 0x0001, 0x0002,
    /* sd */
    0x0006,
    /* se */
    0x0000, 0x0001,
    /* sg */
    0x0000,
    /* sh */
    0x0000, 0x0001, 0x0004,
    /* shs */
    0x0000, 0x0003,
    /* si */
    0x000d,
    /* sid */
    0x0012, 0x0013,
    /* sk */
    0x0000, 0x0001,
    /* sm */
    0x0000, 0x0002,
    /* sma */
    0x0000,
    /* smj */
    0x0000,
    /* smn */
    0x0000, 0x0001,
    /* sms */
    0x0000, 0x0001, 0x0002,
    /* sq */
    0x0000,
    /* sr */
    0x0004,
    /* sv */
    0x0000,
    /* syr */
    0x0007,
    /* ta */
    0x000b,
    /* te */
    0x000c,
    /* tg */
    0x0004,
    /* th */
    0x000e,
    /* tig */
    0x0012, 0x0013,
    /* tk */
    0x0000, 0x0001,
    /* tr */
    0x0000, 0x0001,
    /* tt */
    0x0004,
    /* ty */
    0x0000, 0x0001, 0x0002,
    /* ug */
    0x0006,
    /* uk */
    0x0004,
    /* ve */
    0x0000, 0x001e,
    /* vi */
    0x0000, 0x0001, 0x0003, 0x001e,
    /* vo */
    0x0000,
    /* vot */
    0x0000, 0x0001,
    /* wa */
    0x0000,
    /* wen */
    0x0000, 0x0001,
    /* wo */
    0x0000, 0x0001,
    /* yap */
    0x0000,
    /* yo */
    0x0000, 0x0001, 0x0003, 0x001e,
    /* zh_cn */
    0x0002, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c,
    0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c,
    0x007d, 0x007e, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084,
    0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c,
    0x008d, 0x008e, 0x008f, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094,
    0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c,
    0x009e, 0x009f,
    /* zh_hk */
    0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055,
    0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d,
    0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065,
    0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075,
    0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
    0x007e, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085,
    0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d,
    0x008e, 0x008f, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095,
    0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d,
    0x009e, 0x009f, 0x0205,
    /* zh_tw */
    0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055,
    0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d,
    0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065,
    0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075,
    0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d,
    0x007e, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085,
    0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d,
    0x008e, 0x008f, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095,
    0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d,
    0x009e, 0x009f, 0x00fa,
},
{
    0, /* aa */
    1, /* ab */
    2, /* af */
    190, /* ak */
    3, /* am */
    191, /* an */
    4, /* ar */
    5, /* as */
    6, /* ast */
    7, /* av */
    8, /* ay */
    9, /* az_az */
    10, /* az_ir */
    11, /* ba */
    13, /* be */
    192, /* ber_dz */
    193, /* ber_ma */
    14, /* bg */
    15, /* bh */
    16, /* bho */
    17, /* bi */
    18, /* bin */
    12, /* bm */
    19, /* bn */
    20, /* bo */
    21, /* br */
    240, /* brx */
    22, /* bs */
    23, /* bua */
    194, /* byn */
    24, /* ca */
    25, /* ce */
    26, /* ch */
    27, /* chm */
    28, /* chr */
    29, /* co */
    195, /* crh */
    30, /* cs */
    196, /* csb */
    31, /* cu */
    32, /* cv */
    33, /* cy */
    34, /* da */
    35, /* de */
    242, /* doi */
    197, /* dv */
    36, /* dz */
    198, /* ee */
    37, /* el */
    38, /* en */
    39, /* eo */
    40, /* es */
    41, /* et */
    42, /* eu */
    43, /* fa */
    199, /* fat */
    48, /* ff */
    44, /* fi */
    200, /* fil */
    45, /* fj */
    46, /* fo */
    47, /* fr */
    49, /* fur */
    50, /* fy */
    51, /* ga */
    52, /* gd */
    53, /* gez */
    54, /* gl */
    55, /* gn */
    56, /* gu */
    57, /* gv */
    58, /* ha */
    59, /* haw */
    60, /* he */
    61, /* hi */
    201, /* hne */
    62, /* ho */
    63, /* hr */
    202, /* hsb */
    203, /* ht */
    64, /* hu */
    65, /* hy */
    204, /* hz */
    66, /* ia */
    68, /* id */
    69, /* ie */
    67, /* ig */
    205, /* ii */
    70, /* ik */
    71, /* io */
    72, /* is */
    73, /* it */
    74, /* iu */
    75, /* ja */
    206, /* jv */
    76, /* ka */
    77, /* kaa */
    207, /* kab */
    78, /* ki */
    208, /* kj */
    79, /* kk */
    80, /* kl */
    81, /* km */
    82, /* kn */
    83, /* ko */
    84, /* kok */
    209, /* kr */
    85, /* ks */
    86, /* ku_am */
    210, /* ku_iq */
    87, /* ku_ir */
    211, /* ku_tr */
    88, /* kum */
    89, /* kv */
    90, /* kw */
    212, /* kwm */
    91, /* ky */
    92, /* la */
    238, /* lah */
    93, /* lb */
    94, /* lez */
    213, /* lg */
    214, /* li */
    95, /* ln */
    96, /* lo */
    97, /* lt */
    98, /* lv */
    215, /* mai */
    99, /* mg */
    100, /* mh */
    101, /* mi */
    102, /* mk */
    103, /* ml */
    104, /* mn_cn */
    216, /* mn_mn */
    243, /* mni */
    105, /* mo */
    106, /* mr */
    217, /* ms */
    107, /* mt */
    108, /* my */
    218, /* na */
    109, /* nb */
    110, /* nds */
    111, /* ne */
    219, /* ng */
    112, /* nl */
    113, /* nn */
    114, /* no */
    239, /* nqo */
    115, /* nr */
    116, /* nso */
    220, /* nv */
    117, /* ny */
    118, /* oc */
    119, /* om */
    120, /* or */
    121, /* os */
    221, /* ota */
    122, /* pa */
    222, /* pa_pk */
    223, /* pap_an */
    224, /* pap_aw */
    123, /* pl */
    124, /* ps_af */
    125, /* ps_pk */
    126, /* pt */
    225, /* qu */
    226, /* quz */
    127, /* rm */
    227, /* rn */
    128, /* ro */
    129, /* ru */
    228, /* rw */
    130, /* sa */
    131, /* sah */
    241, /* sat */
    229, /* sc */
    132, /* sco */
    230, /* sd */
    133, /* se */
    134, /* sel */
    231, /* sg */
    135, /* sh */
    136, /* shs */
    137, /* si */
    232, /* sid */
    138, /* sk */
    139, /* sl */
    140, /* sm */
    141, /* sma */
    142, /* smj */
    143, /* smn */
    144, /* sms */
    233, /* sn */
    145, /* so */
    146, /* sq */
    147, /* sr */
    148, /* ss */
    149, /* st */
    234, /* su */
    150, /* sv */
    151, /* sw */
    152, /* syr */
    153, /* ta */
    154, /* te */
    155, /* tg */
    156, /* th */
    157, /* ti_er */
    158, /* ti_et */
    159, /* tig */
    160, /* tk */
    161, /* tl */
    162, /* tn */
    163, /* to */
    164, /* tr */
    165, /* ts */
    166, /* tt */
    167, /* tw */
    235, /* ty */
    168, /* tyv */
    169, /* ug */
    170, /* uk */
    171, /* ur */
    172, /* uz */
    173, /* ve */
    174, /* vi */
    175, /* vo */
    176, /* vot */
    177, /* wa */
    236, /* wal */
    178, /* wen */
    179, /* wo */
    180, /* xh */
    181, /* yap */
    182, /* yi */
    183, /* yo */
    237, /* za */
    184, /* zh_cn */
    185, /* zh_hk */
    186, /* zh_mo */
    187, /* zh_sg */
    188, /* zh_tw */
    189, /* zu */
},
{
    0, /* aa */
    1, /* ab */
    2, /* af */
    4, /* am */
    6, /* ar */
    7, /* as */
    8, /* ast */
    9, /* av */
    10, /* ay */
    11, /* az_az */
    12, /* az_ir */
    13, /* ba */
    22, /* bm */
    14, /* be */
    17, /* bg */
    18, /* bh */
    19, /* bho */
    20, /* bi */
    21, /* bin */
    23, /* bn */
    24, /* bo */
    25, /* br */
    27, /* bs */
    28, /* bua */
    30, /* ca */
    31, /* ce */
    32, /* ch */
    33, /* chm */
    34, /* chr */
    35, /* co */
    37, /* cs */
    39, /* cu */
    40, /* cv */
    41, /* cy */
    42, /* da */
    43, /* de */
    46, /* dz */
    48, /* el */
    49, /* en */
    50, /* eo */
    51, /* es */
    52, /* et */
    53, /* eu */
    54, /* fa */
    57, /* fi */
    59, /* fj */
    60, /* fo */
    61, /* fr */
    56, /* ff */
    62, /* fur */
    63, /* fy */
    64, /* ga */
    65, /* gd */
    66, /* gez */
    67, /* gl */
    68, /* gn */
    69, /* gu */
    70, /* gv */
    71, /* ha */
    72, /* haw */
    73, /* he */
    74, /* hi */
    76, /* ho */
    77, /* hr */
    80, /* hu */
    81, /* hy */
    83, /* ia */
    86, /* ig */
    84, /* id */
    85, /* ie */
    88, /* ik */
    89, /* io */
    90, /* is */
    91, /* it */
    92, /* iu */
    93, /* ja */
    95, /* ka */
    96, /* kaa */
    98, /* ki */
    100, /* kk */
    101, /* kl */
    102, /* km */
    103, /* kn */
    104, /* ko */
    105, /* kok */
    107, /* ks */
    108, /* ku_am */
    110, /* ku_ir */
    112, /* kum */
    113, /* kv */
    114, /* kw */
    116, /* ky */
    117, /* la */
    119, /* lb */
    120, /* lez */
    123, /* ln */
    124, /* lo */
    125, /* lt */
    126, /* lv */
    128, /* mg */
    129, /* mh */
    130, /* mi */
    131, /* mk */
    132, /* ml */
    133, /* mn_cn */
    136, /* mo */
    137, /* mr */
    139, /* mt */
    140, /* my */
    142, /* nb */
    143, /* nds */
    144, /* ne */
    146, /* nl */
    147, /* nn */
    148, /* no */
    150, /* nr */
    151, /* nso */
    153, /* ny */
    154, /* oc */
    155, /* om */
    156, /* or */
    157, /* os */
    159, /* pa */
    163, /* pl */
    164, /* ps_af */
    165, /* ps_pk */
    166, /* pt */
    169, /* rm */
    171, /* ro */
    172, /* ru */
    174, /* sa */
    175, /* sah */
    178, /* sco */
    180, /* se */
    181, /* sel */
    183, /* sh */
    184, /* shs */
    185, /* si */
    187, /* sk */
    188, /* sl */
    189, /* sm */
    190, /* sma */
    191, /* smj */
    192, /* smn */
    193, /* sms */
    195, /* so */
    196, /* sq */
    197, /* sr */
    198, /* ss */
    199, /* st */
    201, /* sv */
    202, /* sw */
    203, /* syr */
    204, /* ta */
    205, /* te */
    206, /* tg */
    207, /* th */
    208, /* ti_er */
    209, /* ti_et */
    210, /* tig */
    211, /* tk */
    212, /* tl */
    213, /* tn */
    214, /* to */
    215, /* tr */
    216, /* ts */
    217, /* tt */
    218, /* tw */
    220, /* tyv */
    221, /* ug */
    222, /* uk */
    223, /* ur */
    224, /* uz */
    225, /* ve */
    226, /* vi */
    227, /* vo */
    228, /* vot */
    229, /* wa */
    231, /* wen */
    232, /* wo */
    233, /* xh */
    234, /* yap */
    235, /* yi */
    236, /* yo */
    238, /* zh_cn */
    239, /* zh_hk */
    240, /* zh_mo */
    241, /* zh_sg */
    242, /* zh_tw */
    243, /* zu */
    3, /* ak */
    5, /* an */
    15, /* ber_dz */
    16, /* ber_ma */
    29, /* byn */
    36, /* crh */
    38, /* csb */
    45, /* dv */
    47, /* ee */
    55, /* fat */
    58, /* fil */
    75, /* hne */
    78, /* hsb */
    79, /* ht */
    82, /* hz */
    87, /* ii */
    94, /* jv */
    97, /* kab */
    99, /* kj */
    106, /* kr */
    109, /* ku_iq */
    111, /* ku_tr */
    115, /* kwm */
    121, /* lg */
    122, /* li */
    127, /* mai */
    134, /* mn_mn */
    138, /* ms */
    141, /* na */
    145, /* ng */
    152, /* nv */
    158, /* ota */
    160, /* pa_pk */
    161, /* pap_an */
    162, /* pap_aw */
    167, /* qu */
    168, /* quz */
    170, /* rn */
    173, /* rw */
    177, /* sc */
    179, /* sd */
    182, /* sg */
    186, /* sid */
    194, /* sn */
    200, /* su */
    219, /* ty */
    230, /* wal */
    237, /* za */
    118, /* lah */
    149, /* nqo */
    26, /* brx */
    176, /* sat */
    44, /* doi */
    135, /* mni */
}
};

#define NUM_LANG_CHAR_SET	244
#define NUM_LANG_SET_MAP	8

static const FcChar32 fcLangCountrySets[][NUM_LANG_SET_MAP] = {
    { 0x00000600, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, }, /* az */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000003, 0x00000000, }, /* ber */
    { 0x00000000, 0x00000000, 0x00c00000, 0x00000000, 0x00000000, 0x00000000, 0x000c0000, 0x00000000, }, /* ku */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000100, 0x00000000, 0x00000000, 0x01000000, 0x00000000, }, /* mn */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40000000, 0x00000000, }, /* pa */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x00000001, }, /* pap */
    { 0x00000000, 0x00000000, 0x00000000, 0x30000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, }, /* ps */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x60000000, 0x00000000, 0x00000000, 0x00000000, }, /* ti */
    { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1f000000, 0x00000000, 0x00000000, }, /* zh */
};

#define NUM_COUNTRY_SET 9

static const FcLangCharSetRange  fcLangCharSetRanges[] = {

    { 0, 12 }, /* a */
    { 13, 29 }, /* b */
    { 30, 41 }, /* c */
    { 42, 46 }, /* d */
    { 47, 53 }, /* e */
    { 54, 63 }, /* f */
    { 64, 70 }, /* g */
    { 71, 82 }, /* h */
    { 83, 92 }, /* i */
    { 93, 94 }, /* j */
    { 95, 116 }, /* k */
    { 117, 126 }, /* l */
    { 127, 140 }, /* m */
    { 141, 153 }, /* n */
    { 154, 158 }, /* o */
    { 159, 166 }, /* p */
    { 167, 168 }, /* q */
    { 169, 173 }, /* r */
    { 174, 203 }, /* s */
    { 204, 220 }, /* t */
    { 221, 224 }, /* u */
    { 225, 228 }, /* v */
    { 229, 232 }, /* w */
    { 233, 233 }, /* x */
    { 234, 236 }, /* y */
    { 237, 243 }, /* z */
};

