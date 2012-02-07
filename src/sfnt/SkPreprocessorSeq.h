/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPreprocessorSeq_DEFINED
#define SkPreprocessorSeq_DEFINED

#define SK_CONCAT(a, b) SK_CONCAT_I(a, b) // allow for macro expansion
#define SK_CONCAT_I(a, b) a ## b

//A preprocessor pair is of the form '(a, b)'
#define SK_PAIR_FIRST(pair) SK_PAIR_FIRST_I pair
#define SK_PAIR_FIRST_I(a, b) a
#define SK_PAIR_SECOND(pair) SK_PAIR_SECOND_I pair
#define SK_PAIR_SECOND_I(a, b) b

//A preprocessor sequence is of the form (a)(b)(c)SK_SEQ_END
#if 0
//The following is what we logically want to do.
//However, MSVC expands macros at the wrong time, causing an error on use of
//SK_SEQ_IGNORE_SECOND_I because it formally takes two parameters, but we only
//pass "one" parameter in SK_SEQ_IGNORE_SECOND.

#define SK_SEQ_HEAD(seq) SK_SEQ_IGNORE_SECOND(SK_SEQ_FIRST seq)
#define SK_SEQ_IGNORE_SECOND(x) SK_SEQ_IGNORE_SECOND_I(x) // expand x
#define SK_SEQ_IGNORE_SECOND_I(x, _) x
#define SK_SEQ_FIRST(x) x, SK_NIL

#else

//This is less obvious, but works on GCC, clang, and MSVC.
#define SK_SEQ_HEAD(seq) SK_SEQ_HEAD_II((SK_SEQ_FIRST seq))
#define SK_SEQ_HEAD_II(x) SK_SEQ_IGNORE_SECOND_I x
#define SK_SEQ_IGNORE_SECOND_I(x, _) x
#define SK_SEQ_FIRST(x) x, SK_NIL

#endif

#define SK_SEQ_TAIL(seq) SK_SEQ_TAIL_I seq
#define SK_SEQ_TAIL_I(x)

#define SK_SEQ_SIZE(seq) SK_CONCAT(SK_SEQ_SIZE_, SK_SEQ_SIZE_TERMINATOR seq)

#define SK_SEQ_SIZE_TERMINATOR(_) SK_SEQ_SIZE_0
#define SK_SEQ_SIZE_0(_) SK_SEQ_SIZE_1
#define SK_SEQ_SIZE_1(_) SK_SEQ_SIZE_2
#define SK_SEQ_SIZE_2(_) SK_SEQ_SIZE_3
#define SK_SEQ_SIZE_3(_) SK_SEQ_SIZE_4
#define SK_SEQ_SIZE_4(_) SK_SEQ_SIZE_5
#define SK_SEQ_SIZE_5(_) SK_SEQ_SIZE_6
#define SK_SEQ_SIZE_6(_) SK_SEQ_SIZE_7
#define SK_SEQ_SIZE_7(_) SK_SEQ_SIZE_8
#define SK_SEQ_SIZE_8(_) SK_SEQ_SIZE_9
#define SK_SEQ_SIZE_9(_) SK_SEQ_SIZE_10
#define SK_SEQ_SIZE_10(_) SK_SEQ_SIZE_11
#define SK_SEQ_SIZE_11(_) SK_SEQ_SIZE_12
#define SK_SEQ_SIZE_12(_) SK_SEQ_SIZE_13
#define SK_SEQ_SIZE_13(_) SK_SEQ_SIZE_14
#define SK_SEQ_SIZE_14(_) SK_SEQ_SIZE_15
#define SK_SEQ_SIZE_15(_) SK_SEQ_SIZE_16
#define SK_SEQ_SIZE_16(_) SK_SEQ_SIZE_17
#define SK_SEQ_SIZE_17(_) SK_SEQ_SIZE_18
#define SK_SEQ_SIZE_18(_) SK_SEQ_SIZE_19
#define SK_SEQ_SIZE_19(_) SK_SEQ_SIZE_20
#define SK_SEQ_SIZE_20(_) SK_SEQ_SIZE_21
#define SK_SEQ_SIZE_21(_) SK_SEQ_SIZE_22
#define SK_SEQ_SIZE_22(_) SK_SEQ_SIZE_23
#define SK_SEQ_SIZE_23(_) SK_SEQ_SIZE_24
#define SK_SEQ_SIZE_24(_) SK_SEQ_SIZE_25
#define SK_SEQ_SIZE_25(_) SK_SEQ_SIZE_26
#define SK_SEQ_SIZE_26(_) SK_SEQ_SIZE_27
#define SK_SEQ_SIZE_27(_) SK_SEQ_SIZE_28
#define SK_SEQ_SIZE_28(_) SK_SEQ_SIZE_29
#define SK_SEQ_SIZE_29(_) SK_SEQ_SIZE_30
#define SK_SEQ_SIZE_30(_) SK_SEQ_SIZE_31
#define SK_SEQ_SIZE_31(_) SK_SEQ_SIZE_32
#define SK_SEQ_SIZE_32(_) SK_SEQ_SIZE_33
#define SK_SEQ_SIZE_33(_) SK_SEQ_SIZE_34
#define SK_SEQ_SIZE_34(_) SK_SEQ_SIZE_35
#define SK_SEQ_SIZE_35(_) SK_SEQ_SIZE_36
#define SK_SEQ_SIZE_36(_) SK_SEQ_SIZE_37
#define SK_SEQ_SIZE_37(_) SK_SEQ_SIZE_38
#define SK_SEQ_SIZE_38(_) SK_SEQ_SIZE_39
#define SK_SEQ_SIZE_39(_) SK_SEQ_SIZE_40
#define SK_SEQ_SIZE_40(_) SK_SEQ_SIZE_41
#define SK_SEQ_SIZE_41(_) SK_SEQ_SIZE_42
#define SK_SEQ_SIZE_42(_) SK_SEQ_SIZE_43
#define SK_SEQ_SIZE_43(_) SK_SEQ_SIZE_44
#define SK_SEQ_SIZE_44(_) SK_SEQ_SIZE_45
#define SK_SEQ_SIZE_45(_) SK_SEQ_SIZE_46
#define SK_SEQ_SIZE_46(_) SK_SEQ_SIZE_47
#define SK_SEQ_SIZE_47(_) SK_SEQ_SIZE_48
#define SK_SEQ_SIZE_48(_) SK_SEQ_SIZE_49
#define SK_SEQ_SIZE_49(_) SK_SEQ_SIZE_50
#define SK_SEQ_SIZE_50(_) SK_SEQ_SIZE_51
#define SK_SEQ_SIZE_51(_) SK_SEQ_SIZE_52
#define SK_SEQ_SIZE_52(_) SK_SEQ_SIZE_53
#define SK_SEQ_SIZE_53(_) SK_SEQ_SIZE_54
#define SK_SEQ_SIZE_54(_) SK_SEQ_SIZE_55
#define SK_SEQ_SIZE_55(_) SK_SEQ_SIZE_56
#define SK_SEQ_SIZE_56(_) SK_SEQ_SIZE_57
#define SK_SEQ_SIZE_57(_) SK_SEQ_SIZE_58
#define SK_SEQ_SIZE_58(_) SK_SEQ_SIZE_59
#define SK_SEQ_SIZE_59(_) SK_SEQ_SIZE_60
#define SK_SEQ_SIZE_60(_) SK_SEQ_SIZE_61
#define SK_SEQ_SIZE_61(_) SK_SEQ_SIZE_62
#define SK_SEQ_SIZE_62(_) SK_SEQ_SIZE_63
#define SK_SEQ_SIZE_63(_) SK_SEQ_SIZE_64
#define SK_SEQ_SIZE_64(_) SK_SEQ_SIZE_65
#define SK_SEQ_SIZE_65(_) SK_SEQ_SIZE_66
#define SK_SEQ_SIZE_66(_) SK_SEQ_SIZE_67
#define SK_SEQ_SIZE_67(_) SK_SEQ_SIZE_68
#define SK_SEQ_SIZE_68(_) SK_SEQ_SIZE_69
#define SK_SEQ_SIZE_69(_) SK_SEQ_SIZE_70
#define SK_SEQ_SIZE_70(_) SK_SEQ_SIZE_71
#define SK_SEQ_SIZE_71(_) SK_SEQ_SIZE_72
#define SK_SEQ_SIZE_72(_) SK_SEQ_SIZE_73
#define SK_SEQ_SIZE_73(_) SK_SEQ_SIZE_74
#define SK_SEQ_SIZE_74(_) SK_SEQ_SIZE_75
#define SK_SEQ_SIZE_75(_) SK_SEQ_SIZE_76
#define SK_SEQ_SIZE_76(_) SK_SEQ_SIZE_77
#define SK_SEQ_SIZE_77(_) SK_SEQ_SIZE_78
#define SK_SEQ_SIZE_78(_) SK_SEQ_SIZE_79
#define SK_SEQ_SIZE_79(_) SK_SEQ_SIZE_80
#define SK_SEQ_SIZE_80(_) SK_SEQ_SIZE_81
#define SK_SEQ_SIZE_81(_) SK_SEQ_SIZE_82
#define SK_SEQ_SIZE_82(_) SK_SEQ_SIZE_83
#define SK_SEQ_SIZE_83(_) SK_SEQ_SIZE_84
#define SK_SEQ_SIZE_84(_) SK_SEQ_SIZE_85
#define SK_SEQ_SIZE_85(_) SK_SEQ_SIZE_86
#define SK_SEQ_SIZE_86(_) SK_SEQ_SIZE_87
#define SK_SEQ_SIZE_87(_) SK_SEQ_SIZE_88
#define SK_SEQ_SIZE_88(_) SK_SEQ_SIZE_89
#define SK_SEQ_SIZE_89(_) SK_SEQ_SIZE_90
#define SK_SEQ_SIZE_90(_) SK_SEQ_SIZE_91
#define SK_SEQ_SIZE_91(_) SK_SEQ_SIZE_92
#define SK_SEQ_SIZE_92(_) SK_SEQ_SIZE_93
#define SK_SEQ_SIZE_93(_) SK_SEQ_SIZE_94
#define SK_SEQ_SIZE_94(_) SK_SEQ_SIZE_95
#define SK_SEQ_SIZE_95(_) SK_SEQ_SIZE_96
#define SK_SEQ_SIZE_96(_) SK_SEQ_SIZE_97
#define SK_SEQ_SIZE_97(_) SK_SEQ_SIZE_98
#define SK_SEQ_SIZE_98(_) SK_SEQ_SIZE_99
#define SK_SEQ_SIZE_99(_) SK_SEQ_SIZE_100
#define SK_SEQ_SIZE_100(_) SK_SEQ_SIZE_101
#define SK_SEQ_SIZE_101(_) SK_SEQ_SIZE_102
#define SK_SEQ_SIZE_102(_) SK_SEQ_SIZE_103
#define SK_SEQ_SIZE_103(_) SK_SEQ_SIZE_104
#define SK_SEQ_SIZE_104(_) SK_SEQ_SIZE_105
#define SK_SEQ_SIZE_105(_) SK_SEQ_SIZE_106
#define SK_SEQ_SIZE_106(_) SK_SEQ_SIZE_107
#define SK_SEQ_SIZE_107(_) SK_SEQ_SIZE_108
#define SK_SEQ_SIZE_108(_) SK_SEQ_SIZE_109
#define SK_SEQ_SIZE_109(_) SK_SEQ_SIZE_110
#define SK_SEQ_SIZE_110(_) SK_SEQ_SIZE_111
#define SK_SEQ_SIZE_111(_) SK_SEQ_SIZE_112
#define SK_SEQ_SIZE_112(_) SK_SEQ_SIZE_113
#define SK_SEQ_SIZE_113(_) SK_SEQ_SIZE_114
#define SK_SEQ_SIZE_114(_) SK_SEQ_SIZE_115
#define SK_SEQ_SIZE_115(_) SK_SEQ_SIZE_116
#define SK_SEQ_SIZE_116(_) SK_SEQ_SIZE_117
#define SK_SEQ_SIZE_117(_) SK_SEQ_SIZE_118
#define SK_SEQ_SIZE_118(_) SK_SEQ_SIZE_119
#define SK_SEQ_SIZE_119(_) SK_SEQ_SIZE_120
#define SK_SEQ_SIZE_120(_) SK_SEQ_SIZE_121
#define SK_SEQ_SIZE_121(_) SK_SEQ_SIZE_122
#define SK_SEQ_SIZE_122(_) SK_SEQ_SIZE_123
#define SK_SEQ_SIZE_123(_) SK_SEQ_SIZE_124
#define SK_SEQ_SIZE_124(_) SK_SEQ_SIZE_125
#define SK_SEQ_SIZE_125(_) SK_SEQ_SIZE_126
#define SK_SEQ_SIZE_126(_) SK_SEQ_SIZE_127
#define SK_SEQ_SIZE_127(_) SK_SEQ_SIZE_128
#define SK_SEQ_SIZE_128(_) SK_SEQ_SIZE_129
#define SK_SEQ_SIZE_129(_) SK_SEQ_SIZE_130
#define SK_SEQ_SIZE_130(_) SK_SEQ_SIZE_131
#define SK_SEQ_SIZE_131(_) SK_SEQ_SIZE_132
#define SK_SEQ_SIZE_132(_) SK_SEQ_SIZE_133
#define SK_SEQ_SIZE_133(_) SK_SEQ_SIZE_134
#define SK_SEQ_SIZE_134(_) SK_SEQ_SIZE_135
#define SK_SEQ_SIZE_135(_) SK_SEQ_SIZE_136
#define SK_SEQ_SIZE_136(_) SK_SEQ_SIZE_137
#define SK_SEQ_SIZE_137(_) SK_SEQ_SIZE_138
#define SK_SEQ_SIZE_138(_) SK_SEQ_SIZE_139
#define SK_SEQ_SIZE_139(_) SK_SEQ_SIZE_140
#define SK_SEQ_SIZE_140(_) SK_SEQ_SIZE_141
#define SK_SEQ_SIZE_141(_) SK_SEQ_SIZE_142
#define SK_SEQ_SIZE_142(_) SK_SEQ_SIZE_143
#define SK_SEQ_SIZE_143(_) SK_SEQ_SIZE_144
#define SK_SEQ_SIZE_144(_) SK_SEQ_SIZE_145
#define SK_SEQ_SIZE_145(_) SK_SEQ_SIZE_146
#define SK_SEQ_SIZE_146(_) SK_SEQ_SIZE_147
#define SK_SEQ_SIZE_147(_) SK_SEQ_SIZE_148
#define SK_SEQ_SIZE_148(_) SK_SEQ_SIZE_149
#define SK_SEQ_SIZE_149(_) SK_SEQ_SIZE_150
#define SK_SEQ_SIZE_150(_) SK_SEQ_SIZE_151
#define SK_SEQ_SIZE_151(_) SK_SEQ_SIZE_152
#define SK_SEQ_SIZE_152(_) SK_SEQ_SIZE_153
#define SK_SEQ_SIZE_153(_) SK_SEQ_SIZE_154
#define SK_SEQ_SIZE_154(_) SK_SEQ_SIZE_155
#define SK_SEQ_SIZE_155(_) SK_SEQ_SIZE_156
#define SK_SEQ_SIZE_156(_) SK_SEQ_SIZE_157
#define SK_SEQ_SIZE_157(_) SK_SEQ_SIZE_158
#define SK_SEQ_SIZE_158(_) SK_SEQ_SIZE_159
#define SK_SEQ_SIZE_159(_) SK_SEQ_SIZE_160
#define SK_SEQ_SIZE_160(_) SK_SEQ_SIZE_161
#define SK_SEQ_SIZE_161(_) SK_SEQ_SIZE_162
#define SK_SEQ_SIZE_162(_) SK_SEQ_SIZE_163
#define SK_SEQ_SIZE_163(_) SK_SEQ_SIZE_164
#define SK_SEQ_SIZE_164(_) SK_SEQ_SIZE_165
#define SK_SEQ_SIZE_165(_) SK_SEQ_SIZE_166
#define SK_SEQ_SIZE_166(_) SK_SEQ_SIZE_167
#define SK_SEQ_SIZE_167(_) SK_SEQ_SIZE_168
#define SK_SEQ_SIZE_168(_) SK_SEQ_SIZE_169
#define SK_SEQ_SIZE_169(_) SK_SEQ_SIZE_170
#define SK_SEQ_SIZE_170(_) SK_SEQ_SIZE_171
#define SK_SEQ_SIZE_171(_) SK_SEQ_SIZE_172
#define SK_SEQ_SIZE_172(_) SK_SEQ_SIZE_173
#define SK_SEQ_SIZE_173(_) SK_SEQ_SIZE_174
#define SK_SEQ_SIZE_174(_) SK_SEQ_SIZE_175
#define SK_SEQ_SIZE_175(_) SK_SEQ_SIZE_176
#define SK_SEQ_SIZE_176(_) SK_SEQ_SIZE_177
#define SK_SEQ_SIZE_177(_) SK_SEQ_SIZE_178
#define SK_SEQ_SIZE_178(_) SK_SEQ_SIZE_179
#define SK_SEQ_SIZE_179(_) SK_SEQ_SIZE_180
#define SK_SEQ_SIZE_180(_) SK_SEQ_SIZE_181
#define SK_SEQ_SIZE_181(_) SK_SEQ_SIZE_182
#define SK_SEQ_SIZE_182(_) SK_SEQ_SIZE_183
#define SK_SEQ_SIZE_183(_) SK_SEQ_SIZE_184
#define SK_SEQ_SIZE_184(_) SK_SEQ_SIZE_185
#define SK_SEQ_SIZE_185(_) SK_SEQ_SIZE_186
#define SK_SEQ_SIZE_186(_) SK_SEQ_SIZE_187
#define SK_SEQ_SIZE_187(_) SK_SEQ_SIZE_188
#define SK_SEQ_SIZE_188(_) SK_SEQ_SIZE_189
#define SK_SEQ_SIZE_189(_) SK_SEQ_SIZE_190
#define SK_SEQ_SIZE_190(_) SK_SEQ_SIZE_191
#define SK_SEQ_SIZE_191(_) SK_SEQ_SIZE_192
#define SK_SEQ_SIZE_192(_) SK_SEQ_SIZE_193
#define SK_SEQ_SIZE_193(_) SK_SEQ_SIZE_194
#define SK_SEQ_SIZE_194(_) SK_SEQ_SIZE_195
#define SK_SEQ_SIZE_195(_) SK_SEQ_SIZE_196
#define SK_SEQ_SIZE_196(_) SK_SEQ_SIZE_197
#define SK_SEQ_SIZE_197(_) SK_SEQ_SIZE_198
#define SK_SEQ_SIZE_198(_) SK_SEQ_SIZE_199
#define SK_SEQ_SIZE_199(_) SK_SEQ_SIZE_200
#define SK_SEQ_SIZE_200(_) SK_SEQ_SIZE_201
#define SK_SEQ_SIZE_201(_) SK_SEQ_SIZE_202
#define SK_SEQ_SIZE_202(_) SK_SEQ_SIZE_203
#define SK_SEQ_SIZE_203(_) SK_SEQ_SIZE_204
#define SK_SEQ_SIZE_204(_) SK_SEQ_SIZE_205
#define SK_SEQ_SIZE_205(_) SK_SEQ_SIZE_206
#define SK_SEQ_SIZE_206(_) SK_SEQ_SIZE_207
#define SK_SEQ_SIZE_207(_) SK_SEQ_SIZE_208
#define SK_SEQ_SIZE_208(_) SK_SEQ_SIZE_209
#define SK_SEQ_SIZE_209(_) SK_SEQ_SIZE_210
#define SK_SEQ_SIZE_210(_) SK_SEQ_SIZE_211
#define SK_SEQ_SIZE_211(_) SK_SEQ_SIZE_212
#define SK_SEQ_SIZE_212(_) SK_SEQ_SIZE_213
#define SK_SEQ_SIZE_213(_) SK_SEQ_SIZE_214
#define SK_SEQ_SIZE_214(_) SK_SEQ_SIZE_215
#define SK_SEQ_SIZE_215(_) SK_SEQ_SIZE_216
#define SK_SEQ_SIZE_216(_) SK_SEQ_SIZE_217
#define SK_SEQ_SIZE_217(_) SK_SEQ_SIZE_218
#define SK_SEQ_SIZE_218(_) SK_SEQ_SIZE_219
#define SK_SEQ_SIZE_219(_) SK_SEQ_SIZE_220
#define SK_SEQ_SIZE_220(_) SK_SEQ_SIZE_221
#define SK_SEQ_SIZE_221(_) SK_SEQ_SIZE_222
#define SK_SEQ_SIZE_222(_) SK_SEQ_SIZE_223
#define SK_SEQ_SIZE_223(_) SK_SEQ_SIZE_224
#define SK_SEQ_SIZE_224(_) SK_SEQ_SIZE_225
#define SK_SEQ_SIZE_225(_) SK_SEQ_SIZE_226
#define SK_SEQ_SIZE_226(_) SK_SEQ_SIZE_227
#define SK_SEQ_SIZE_227(_) SK_SEQ_SIZE_228
#define SK_SEQ_SIZE_228(_) SK_SEQ_SIZE_229
#define SK_SEQ_SIZE_229(_) SK_SEQ_SIZE_230
#define SK_SEQ_SIZE_230(_) SK_SEQ_SIZE_231
#define SK_SEQ_SIZE_231(_) SK_SEQ_SIZE_232
#define SK_SEQ_SIZE_232(_) SK_SEQ_SIZE_233
#define SK_SEQ_SIZE_233(_) SK_SEQ_SIZE_234
#define SK_SEQ_SIZE_234(_) SK_SEQ_SIZE_235
#define SK_SEQ_SIZE_235(_) SK_SEQ_SIZE_236
#define SK_SEQ_SIZE_236(_) SK_SEQ_SIZE_237
#define SK_SEQ_SIZE_237(_) SK_SEQ_SIZE_238
#define SK_SEQ_SIZE_238(_) SK_SEQ_SIZE_239
#define SK_SEQ_SIZE_239(_) SK_SEQ_SIZE_240
#define SK_SEQ_SIZE_240(_) SK_SEQ_SIZE_241
#define SK_SEQ_SIZE_241(_) SK_SEQ_SIZE_242
#define SK_SEQ_SIZE_242(_) SK_SEQ_SIZE_243
#define SK_SEQ_SIZE_243(_) SK_SEQ_SIZE_244
#define SK_SEQ_SIZE_244(_) SK_SEQ_SIZE_245
#define SK_SEQ_SIZE_245(_) SK_SEQ_SIZE_246
#define SK_SEQ_SIZE_246(_) SK_SEQ_SIZE_247
#define SK_SEQ_SIZE_247(_) SK_SEQ_SIZE_248
#define SK_SEQ_SIZE_248(_) SK_SEQ_SIZE_249
#define SK_SEQ_SIZE_249(_) SK_SEQ_SIZE_250
#define SK_SEQ_SIZE_250(_) SK_SEQ_SIZE_251
#define SK_SEQ_SIZE_251(_) SK_SEQ_SIZE_252
#define SK_SEQ_SIZE_252(_) SK_SEQ_SIZE_253
#define SK_SEQ_SIZE_253(_) SK_SEQ_SIZE_254
#define SK_SEQ_SIZE_254(_) SK_SEQ_SIZE_255
#define SK_SEQ_SIZE_255(_) SK_SEQ_SIZE_256


#define SK_SEQ_SIZE_SK_SEQ_SIZE_0 0
#define SK_SEQ_SIZE_SK_SEQ_SIZE_1 1
#define SK_SEQ_SIZE_SK_SEQ_SIZE_2 2
#define SK_SEQ_SIZE_SK_SEQ_SIZE_3 3
#define SK_SEQ_SIZE_SK_SEQ_SIZE_4 4
#define SK_SEQ_SIZE_SK_SEQ_SIZE_5 5
#define SK_SEQ_SIZE_SK_SEQ_SIZE_6 6
#define SK_SEQ_SIZE_SK_SEQ_SIZE_7 7
#define SK_SEQ_SIZE_SK_SEQ_SIZE_8 8
#define SK_SEQ_SIZE_SK_SEQ_SIZE_9 9
#define SK_SEQ_SIZE_SK_SEQ_SIZE_10 10
#define SK_SEQ_SIZE_SK_SEQ_SIZE_11 11
#define SK_SEQ_SIZE_SK_SEQ_SIZE_12 12
#define SK_SEQ_SIZE_SK_SEQ_SIZE_13 13
#define SK_SEQ_SIZE_SK_SEQ_SIZE_14 14
#define SK_SEQ_SIZE_SK_SEQ_SIZE_15 15
#define SK_SEQ_SIZE_SK_SEQ_SIZE_16 16
#define SK_SEQ_SIZE_SK_SEQ_SIZE_17 17
#define SK_SEQ_SIZE_SK_SEQ_SIZE_18 18
#define SK_SEQ_SIZE_SK_SEQ_SIZE_19 19
#define SK_SEQ_SIZE_SK_SEQ_SIZE_20 20
#define SK_SEQ_SIZE_SK_SEQ_SIZE_21 21
#define SK_SEQ_SIZE_SK_SEQ_SIZE_22 22
#define SK_SEQ_SIZE_SK_SEQ_SIZE_23 23
#define SK_SEQ_SIZE_SK_SEQ_SIZE_24 24
#define SK_SEQ_SIZE_SK_SEQ_SIZE_25 25
#define SK_SEQ_SIZE_SK_SEQ_SIZE_26 26
#define SK_SEQ_SIZE_SK_SEQ_SIZE_27 27
#define SK_SEQ_SIZE_SK_SEQ_SIZE_28 28
#define SK_SEQ_SIZE_SK_SEQ_SIZE_29 29
#define SK_SEQ_SIZE_SK_SEQ_SIZE_30 30
#define SK_SEQ_SIZE_SK_SEQ_SIZE_31 31
#define SK_SEQ_SIZE_SK_SEQ_SIZE_32 32
#define SK_SEQ_SIZE_SK_SEQ_SIZE_33 33
#define SK_SEQ_SIZE_SK_SEQ_SIZE_34 34
#define SK_SEQ_SIZE_SK_SEQ_SIZE_35 35
#define SK_SEQ_SIZE_SK_SEQ_SIZE_36 36
#define SK_SEQ_SIZE_SK_SEQ_SIZE_37 37
#define SK_SEQ_SIZE_SK_SEQ_SIZE_38 38
#define SK_SEQ_SIZE_SK_SEQ_SIZE_39 39
#define SK_SEQ_SIZE_SK_SEQ_SIZE_40 40
#define SK_SEQ_SIZE_SK_SEQ_SIZE_41 41
#define SK_SEQ_SIZE_SK_SEQ_SIZE_42 42
#define SK_SEQ_SIZE_SK_SEQ_SIZE_43 43
#define SK_SEQ_SIZE_SK_SEQ_SIZE_44 44
#define SK_SEQ_SIZE_SK_SEQ_SIZE_45 45
#define SK_SEQ_SIZE_SK_SEQ_SIZE_46 46
#define SK_SEQ_SIZE_SK_SEQ_SIZE_47 47
#define SK_SEQ_SIZE_SK_SEQ_SIZE_48 48
#define SK_SEQ_SIZE_SK_SEQ_SIZE_49 49
#define SK_SEQ_SIZE_SK_SEQ_SIZE_50 50
#define SK_SEQ_SIZE_SK_SEQ_SIZE_51 51
#define SK_SEQ_SIZE_SK_SEQ_SIZE_52 52
#define SK_SEQ_SIZE_SK_SEQ_SIZE_53 53
#define SK_SEQ_SIZE_SK_SEQ_SIZE_54 54
#define SK_SEQ_SIZE_SK_SEQ_SIZE_55 55
#define SK_SEQ_SIZE_SK_SEQ_SIZE_56 56
#define SK_SEQ_SIZE_SK_SEQ_SIZE_57 57
#define SK_SEQ_SIZE_SK_SEQ_SIZE_58 58
#define SK_SEQ_SIZE_SK_SEQ_SIZE_59 59
#define SK_SEQ_SIZE_SK_SEQ_SIZE_60 60
#define SK_SEQ_SIZE_SK_SEQ_SIZE_61 61
#define SK_SEQ_SIZE_SK_SEQ_SIZE_62 62
#define SK_SEQ_SIZE_SK_SEQ_SIZE_63 63
#define SK_SEQ_SIZE_SK_SEQ_SIZE_64 64
#define SK_SEQ_SIZE_SK_SEQ_SIZE_65 65
#define SK_SEQ_SIZE_SK_SEQ_SIZE_66 66
#define SK_SEQ_SIZE_SK_SEQ_SIZE_67 67
#define SK_SEQ_SIZE_SK_SEQ_SIZE_68 68
#define SK_SEQ_SIZE_SK_SEQ_SIZE_69 69
#define SK_SEQ_SIZE_SK_SEQ_SIZE_70 70
#define SK_SEQ_SIZE_SK_SEQ_SIZE_71 71
#define SK_SEQ_SIZE_SK_SEQ_SIZE_72 72
#define SK_SEQ_SIZE_SK_SEQ_SIZE_73 73
#define SK_SEQ_SIZE_SK_SEQ_SIZE_74 74
#define SK_SEQ_SIZE_SK_SEQ_SIZE_75 75
#define SK_SEQ_SIZE_SK_SEQ_SIZE_76 76
#define SK_SEQ_SIZE_SK_SEQ_SIZE_77 77
#define SK_SEQ_SIZE_SK_SEQ_SIZE_78 78
#define SK_SEQ_SIZE_SK_SEQ_SIZE_79 79
#define SK_SEQ_SIZE_SK_SEQ_SIZE_80 80
#define SK_SEQ_SIZE_SK_SEQ_SIZE_81 81
#define SK_SEQ_SIZE_SK_SEQ_SIZE_82 82
#define SK_SEQ_SIZE_SK_SEQ_SIZE_83 83
#define SK_SEQ_SIZE_SK_SEQ_SIZE_84 84
#define SK_SEQ_SIZE_SK_SEQ_SIZE_85 85
#define SK_SEQ_SIZE_SK_SEQ_SIZE_86 86
#define SK_SEQ_SIZE_SK_SEQ_SIZE_87 87
#define SK_SEQ_SIZE_SK_SEQ_SIZE_88 88
#define SK_SEQ_SIZE_SK_SEQ_SIZE_89 89
#define SK_SEQ_SIZE_SK_SEQ_SIZE_90 90
#define SK_SEQ_SIZE_SK_SEQ_SIZE_91 91
#define SK_SEQ_SIZE_SK_SEQ_SIZE_92 92
#define SK_SEQ_SIZE_SK_SEQ_SIZE_93 93
#define SK_SEQ_SIZE_SK_SEQ_SIZE_94 94
#define SK_SEQ_SIZE_SK_SEQ_SIZE_95 95
#define SK_SEQ_SIZE_SK_SEQ_SIZE_96 96
#define SK_SEQ_SIZE_SK_SEQ_SIZE_97 97
#define SK_SEQ_SIZE_SK_SEQ_SIZE_98 98
#define SK_SEQ_SIZE_SK_SEQ_SIZE_99 99
#define SK_SEQ_SIZE_SK_SEQ_SIZE_100 100
#define SK_SEQ_SIZE_SK_SEQ_SIZE_101 101
#define SK_SEQ_SIZE_SK_SEQ_SIZE_102 102
#define SK_SEQ_SIZE_SK_SEQ_SIZE_103 103
#define SK_SEQ_SIZE_SK_SEQ_SIZE_104 104
#define SK_SEQ_SIZE_SK_SEQ_SIZE_105 105
#define SK_SEQ_SIZE_SK_SEQ_SIZE_106 106
#define SK_SEQ_SIZE_SK_SEQ_SIZE_107 107
#define SK_SEQ_SIZE_SK_SEQ_SIZE_108 108
#define SK_SEQ_SIZE_SK_SEQ_SIZE_109 109
#define SK_SEQ_SIZE_SK_SEQ_SIZE_110 110
#define SK_SEQ_SIZE_SK_SEQ_SIZE_111 111
#define SK_SEQ_SIZE_SK_SEQ_SIZE_112 112
#define SK_SEQ_SIZE_SK_SEQ_SIZE_113 113
#define SK_SEQ_SIZE_SK_SEQ_SIZE_114 114
#define SK_SEQ_SIZE_SK_SEQ_SIZE_115 115
#define SK_SEQ_SIZE_SK_SEQ_SIZE_116 116
#define SK_SEQ_SIZE_SK_SEQ_SIZE_117 117
#define SK_SEQ_SIZE_SK_SEQ_SIZE_118 118
#define SK_SEQ_SIZE_SK_SEQ_SIZE_119 119
#define SK_SEQ_SIZE_SK_SEQ_SIZE_120 120
#define SK_SEQ_SIZE_SK_SEQ_SIZE_121 121
#define SK_SEQ_SIZE_SK_SEQ_SIZE_122 122
#define SK_SEQ_SIZE_SK_SEQ_SIZE_123 123
#define SK_SEQ_SIZE_SK_SEQ_SIZE_124 124
#define SK_SEQ_SIZE_SK_SEQ_SIZE_125 125
#define SK_SEQ_SIZE_SK_SEQ_SIZE_126 126
#define SK_SEQ_SIZE_SK_SEQ_SIZE_127 127
#define SK_SEQ_SIZE_SK_SEQ_SIZE_128 128
#define SK_SEQ_SIZE_SK_SEQ_SIZE_129 129
#define SK_SEQ_SIZE_SK_SEQ_SIZE_130 130
#define SK_SEQ_SIZE_SK_SEQ_SIZE_131 131
#define SK_SEQ_SIZE_SK_SEQ_SIZE_132 132
#define SK_SEQ_SIZE_SK_SEQ_SIZE_133 133
#define SK_SEQ_SIZE_SK_SEQ_SIZE_134 134
#define SK_SEQ_SIZE_SK_SEQ_SIZE_135 135
#define SK_SEQ_SIZE_SK_SEQ_SIZE_136 136
#define SK_SEQ_SIZE_SK_SEQ_SIZE_137 137
#define SK_SEQ_SIZE_SK_SEQ_SIZE_138 138
#define SK_SEQ_SIZE_SK_SEQ_SIZE_139 139
#define SK_SEQ_SIZE_SK_SEQ_SIZE_140 140
#define SK_SEQ_SIZE_SK_SEQ_SIZE_141 141
#define SK_SEQ_SIZE_SK_SEQ_SIZE_142 142
#define SK_SEQ_SIZE_SK_SEQ_SIZE_143 143
#define SK_SEQ_SIZE_SK_SEQ_SIZE_144 144
#define SK_SEQ_SIZE_SK_SEQ_SIZE_145 145
#define SK_SEQ_SIZE_SK_SEQ_SIZE_146 146
#define SK_SEQ_SIZE_SK_SEQ_SIZE_147 147
#define SK_SEQ_SIZE_SK_SEQ_SIZE_148 148
#define SK_SEQ_SIZE_SK_SEQ_SIZE_149 149
#define SK_SEQ_SIZE_SK_SEQ_SIZE_150 150
#define SK_SEQ_SIZE_SK_SEQ_SIZE_151 151
#define SK_SEQ_SIZE_SK_SEQ_SIZE_152 152
#define SK_SEQ_SIZE_SK_SEQ_SIZE_153 153
#define SK_SEQ_SIZE_SK_SEQ_SIZE_154 154
#define SK_SEQ_SIZE_SK_SEQ_SIZE_155 155
#define SK_SEQ_SIZE_SK_SEQ_SIZE_156 156
#define SK_SEQ_SIZE_SK_SEQ_SIZE_157 157
#define SK_SEQ_SIZE_SK_SEQ_SIZE_158 158
#define SK_SEQ_SIZE_SK_SEQ_SIZE_159 159
#define SK_SEQ_SIZE_SK_SEQ_SIZE_160 160
#define SK_SEQ_SIZE_SK_SEQ_SIZE_161 161
#define SK_SEQ_SIZE_SK_SEQ_SIZE_162 162
#define SK_SEQ_SIZE_SK_SEQ_SIZE_163 163
#define SK_SEQ_SIZE_SK_SEQ_SIZE_164 164
#define SK_SEQ_SIZE_SK_SEQ_SIZE_165 165
#define SK_SEQ_SIZE_SK_SEQ_SIZE_166 166
#define SK_SEQ_SIZE_SK_SEQ_SIZE_167 167
#define SK_SEQ_SIZE_SK_SEQ_SIZE_168 168
#define SK_SEQ_SIZE_SK_SEQ_SIZE_169 169
#define SK_SEQ_SIZE_SK_SEQ_SIZE_170 170
#define SK_SEQ_SIZE_SK_SEQ_SIZE_171 171
#define SK_SEQ_SIZE_SK_SEQ_SIZE_172 172
#define SK_SEQ_SIZE_SK_SEQ_SIZE_173 173
#define SK_SEQ_SIZE_SK_SEQ_SIZE_174 174
#define SK_SEQ_SIZE_SK_SEQ_SIZE_175 175
#define SK_SEQ_SIZE_SK_SEQ_SIZE_176 176
#define SK_SEQ_SIZE_SK_SEQ_SIZE_177 177
#define SK_SEQ_SIZE_SK_SEQ_SIZE_178 178
#define SK_SEQ_SIZE_SK_SEQ_SIZE_179 179
#define SK_SEQ_SIZE_SK_SEQ_SIZE_180 180
#define SK_SEQ_SIZE_SK_SEQ_SIZE_181 181
#define SK_SEQ_SIZE_SK_SEQ_SIZE_182 182
#define SK_SEQ_SIZE_SK_SEQ_SIZE_183 183
#define SK_SEQ_SIZE_SK_SEQ_SIZE_184 184
#define SK_SEQ_SIZE_SK_SEQ_SIZE_185 185
#define SK_SEQ_SIZE_SK_SEQ_SIZE_186 186
#define SK_SEQ_SIZE_SK_SEQ_SIZE_187 187
#define SK_SEQ_SIZE_SK_SEQ_SIZE_188 188
#define SK_SEQ_SIZE_SK_SEQ_SIZE_189 189
#define SK_SEQ_SIZE_SK_SEQ_SIZE_190 190
#define SK_SEQ_SIZE_SK_SEQ_SIZE_191 191
#define SK_SEQ_SIZE_SK_SEQ_SIZE_192 192
#define SK_SEQ_SIZE_SK_SEQ_SIZE_193 193
#define SK_SEQ_SIZE_SK_SEQ_SIZE_194 194
#define SK_SEQ_SIZE_SK_SEQ_SIZE_195 195
#define SK_SEQ_SIZE_SK_SEQ_SIZE_196 196
#define SK_SEQ_SIZE_SK_SEQ_SIZE_197 197
#define SK_SEQ_SIZE_SK_SEQ_SIZE_198 198
#define SK_SEQ_SIZE_SK_SEQ_SIZE_199 199
#define SK_SEQ_SIZE_SK_SEQ_SIZE_200 200
#define SK_SEQ_SIZE_SK_SEQ_SIZE_201 201
#define SK_SEQ_SIZE_SK_SEQ_SIZE_202 202
#define SK_SEQ_SIZE_SK_SEQ_SIZE_203 203
#define SK_SEQ_SIZE_SK_SEQ_SIZE_204 204
#define SK_SEQ_SIZE_SK_SEQ_SIZE_205 205
#define SK_SEQ_SIZE_SK_SEQ_SIZE_206 206
#define SK_SEQ_SIZE_SK_SEQ_SIZE_207 207
#define SK_SEQ_SIZE_SK_SEQ_SIZE_208 208
#define SK_SEQ_SIZE_SK_SEQ_SIZE_209 209
#define SK_SEQ_SIZE_SK_SEQ_SIZE_210 210
#define SK_SEQ_SIZE_SK_SEQ_SIZE_211 211
#define SK_SEQ_SIZE_SK_SEQ_SIZE_212 212
#define SK_SEQ_SIZE_SK_SEQ_SIZE_213 213
#define SK_SEQ_SIZE_SK_SEQ_SIZE_214 214
#define SK_SEQ_SIZE_SK_SEQ_SIZE_215 215
#define SK_SEQ_SIZE_SK_SEQ_SIZE_216 216
#define SK_SEQ_SIZE_SK_SEQ_SIZE_217 217
#define SK_SEQ_SIZE_SK_SEQ_SIZE_218 218
#define SK_SEQ_SIZE_SK_SEQ_SIZE_219 219
#define SK_SEQ_SIZE_SK_SEQ_SIZE_220 220
#define SK_SEQ_SIZE_SK_SEQ_SIZE_221 221
#define SK_SEQ_SIZE_SK_SEQ_SIZE_222 222
#define SK_SEQ_SIZE_SK_SEQ_SIZE_223 223
#define SK_SEQ_SIZE_SK_SEQ_SIZE_224 224
#define SK_SEQ_SIZE_SK_SEQ_SIZE_225 225
#define SK_SEQ_SIZE_SK_SEQ_SIZE_226 226
#define SK_SEQ_SIZE_SK_SEQ_SIZE_227 227
#define SK_SEQ_SIZE_SK_SEQ_SIZE_228 228
#define SK_SEQ_SIZE_SK_SEQ_SIZE_229 229
#define SK_SEQ_SIZE_SK_SEQ_SIZE_230 230
#define SK_SEQ_SIZE_SK_SEQ_SIZE_231 231
#define SK_SEQ_SIZE_SK_SEQ_SIZE_232 232
#define SK_SEQ_SIZE_SK_SEQ_SIZE_233 233
#define SK_SEQ_SIZE_SK_SEQ_SIZE_234 234
#define SK_SEQ_SIZE_SK_SEQ_SIZE_235 235
#define SK_SEQ_SIZE_SK_SEQ_SIZE_236 236
#define SK_SEQ_SIZE_SK_SEQ_SIZE_237 237
#define SK_SEQ_SIZE_SK_SEQ_SIZE_238 238
#define SK_SEQ_SIZE_SK_SEQ_SIZE_239 239
#define SK_SEQ_SIZE_SK_SEQ_SIZE_240 240
#define SK_SEQ_SIZE_SK_SEQ_SIZE_241 241
#define SK_SEQ_SIZE_SK_SEQ_SIZE_242 242
#define SK_SEQ_SIZE_SK_SEQ_SIZE_243 243
#define SK_SEQ_SIZE_SK_SEQ_SIZE_244 244
#define SK_SEQ_SIZE_SK_SEQ_SIZE_245 245
#define SK_SEQ_SIZE_SK_SEQ_SIZE_246 246
#define SK_SEQ_SIZE_SK_SEQ_SIZE_247 247
#define SK_SEQ_SIZE_SK_SEQ_SIZE_248 248
#define SK_SEQ_SIZE_SK_SEQ_SIZE_249 249
#define SK_SEQ_SIZE_SK_SEQ_SIZE_250 250
#define SK_SEQ_SIZE_SK_SEQ_SIZE_251 251
#define SK_SEQ_SIZE_SK_SEQ_SIZE_252 252
#define SK_SEQ_SIZE_SK_SEQ_SIZE_253 253
#define SK_SEQ_SIZE_SK_SEQ_SIZE_254 254
#define SK_SEQ_SIZE_SK_SEQ_SIZE_255 255


#define SK_SEQ_FOREACH(op, data, seq) SK_SEQ_FOREACH_L(op, op, data, seq)
#define SK_SEQ_FOREACH_L(op, lop, data, seq) SK_CONCAT(SK_SEQ_FOREACH_, SK_SEQ_SIZE(seq)) (op, lop, data, SK_SEQ_HEAD(seq), SK_SEQ_TAIL(seq))

#define SK_SEQ_FOREACH_0(op,lop,d,x,t)
#define SK_SEQ_FOREACH_1(op,lop,d,x,t) lop(d,x)
#define SK_SEQ_FOREACH_2(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_1(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_3(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_2(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_4(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_3(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_5(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_4(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_6(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_5(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_7(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_6(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_8(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_7(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_9(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_8(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_10(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_9(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_11(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_10(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_12(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_11(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_13(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_12(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_14(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_13(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_15(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_14(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_16(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_15(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_17(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_16(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_18(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_17(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_19(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_18(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_20(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_19(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_21(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_20(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_22(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_21(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_23(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_22(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_24(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_23(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_25(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_24(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_26(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_25(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_27(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_26(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_28(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_27(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_29(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_28(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_30(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_29(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_31(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_30(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_32(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_31(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_33(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_32(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_34(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_33(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_35(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_34(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_36(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_35(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_37(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_36(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_38(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_37(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_39(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_38(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_40(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_39(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_41(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_40(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_42(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_41(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_43(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_42(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_44(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_43(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_45(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_44(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_46(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_45(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_47(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_46(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_48(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_47(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_49(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_48(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_50(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_49(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_51(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_50(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_52(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_51(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_53(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_52(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_54(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_53(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_55(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_54(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_56(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_55(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_57(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_56(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_58(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_57(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_59(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_58(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_60(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_59(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_61(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_60(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_62(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_61(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_63(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_62(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_64(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_63(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_65(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_64(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_66(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_65(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_67(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_66(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_68(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_67(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_69(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_68(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_70(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_69(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_71(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_70(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_72(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_71(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_73(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_72(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_74(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_73(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_75(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_74(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_76(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_75(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_77(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_76(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_78(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_77(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_79(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_78(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_80(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_79(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_81(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_80(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_82(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_81(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_83(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_82(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_84(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_83(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_85(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_84(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_86(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_85(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_87(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_86(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_88(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_87(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_89(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_88(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_90(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_89(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_91(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_90(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_92(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_91(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_93(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_92(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_94(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_93(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_95(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_94(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_96(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_95(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_97(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_96(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_98(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_97(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_99(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_98(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_100(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_99(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_101(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_100(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_102(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_101(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_103(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_102(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_104(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_103(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_105(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_104(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_106(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_105(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_107(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_106(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_108(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_107(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_109(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_108(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_110(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_109(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_111(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_110(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_112(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_111(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_113(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_112(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_114(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_113(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_115(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_114(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_116(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_115(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_117(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_116(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_118(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_117(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_119(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_118(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_120(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_119(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_121(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_120(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_122(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_121(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_123(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_122(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_124(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_123(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_125(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_124(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_126(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_125(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_127(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_126(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_128(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_127(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_129(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_128(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_130(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_129(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_131(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_130(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_132(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_131(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_133(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_132(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_134(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_133(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_135(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_134(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_136(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_135(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_137(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_136(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_138(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_137(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_139(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_138(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_140(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_139(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_141(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_140(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_142(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_141(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_143(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_142(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_144(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_143(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_145(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_144(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_146(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_145(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_147(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_146(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_148(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_147(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_149(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_148(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_150(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_149(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_151(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_150(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_152(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_151(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_153(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_152(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_154(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_153(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_155(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_154(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_156(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_155(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_157(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_156(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_158(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_157(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_159(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_158(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_160(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_159(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_161(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_160(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_162(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_161(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_163(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_162(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_164(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_163(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_165(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_164(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_166(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_165(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_167(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_166(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_168(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_167(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_169(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_168(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_170(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_169(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_171(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_170(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_172(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_171(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_173(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_172(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_174(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_173(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_175(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_174(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_176(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_175(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_177(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_176(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_178(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_177(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_179(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_178(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_180(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_179(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_181(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_180(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_182(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_181(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_183(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_182(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_184(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_183(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_185(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_184(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_186(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_185(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_187(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_186(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_188(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_187(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_189(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_188(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_190(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_189(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_191(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_190(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_192(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_191(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_193(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_192(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_194(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_193(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_195(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_194(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_196(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_195(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_197(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_196(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_198(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_197(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_199(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_198(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_200(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_199(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_201(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_200(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_202(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_201(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_203(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_202(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_204(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_203(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_205(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_204(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_206(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_205(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_207(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_206(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_208(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_207(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_209(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_208(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_210(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_209(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_211(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_210(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_212(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_211(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_213(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_212(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_214(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_213(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_215(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_214(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_216(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_215(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_217(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_216(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_218(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_217(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_219(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_218(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_220(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_219(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_221(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_220(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_222(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_221(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_223(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_222(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_224(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_223(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_225(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_224(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_226(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_225(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_227(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_226(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_228(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_227(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_229(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_228(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_230(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_229(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_231(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_230(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_232(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_231(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_233(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_232(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_234(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_233(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_235(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_234(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_236(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_235(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_237(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_236(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_238(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_237(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_239(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_238(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_240(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_239(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_241(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_240(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_242(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_241(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_243(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_242(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_244(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_243(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_245(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_244(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_246(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_245(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_247(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_246(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_248(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_247(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_249(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_248(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_250(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_249(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_251(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_250(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_252(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_251(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_253(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_252(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_254(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_253(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))
#define SK_SEQ_FOREACH_255(op,lop,d,x,t) op(d,x) SK_SEQ_FOREACH_254(op, lop, d, SK_SEQ_HEAD(t), SK_SEQ_TAIL(t))

#define SK_SEQ_END (SK_NIL)

#endif
