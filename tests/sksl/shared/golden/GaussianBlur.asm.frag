OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %vLocalCoord_Stage0
OpExecutionMode %main OriginUpperLeft
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "sk_RTAdjust"
OpMemberName %uniformBuffer 1 "uIncrement_Stage1_c0"
OpMemberName %uniformBuffer 2 "uKernel_Stage1_c0"
OpMemberName %uniformBuffer 3 "umatrix_Stage1_c0_c0"
OpMemberName %uniformBuffer 4 "uborder_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 5 "usubset_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 6 "unorm_Stage1_c0_c0_c0"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %uTextureSampler_0_Stage1 "uTextureSampler_0_Stage1"
OpName %vLocalCoord_Stage0 "vLocalCoord_Stage0"
OpName %main "main"
OpName %output_Stage1 "output_Stage1"
OpName %_207_GaussianConvolution_Stage1_c0 "_207_GaussianConvolution_Stage1_c0"
OpName %_208_output "_208_output"
OpName %_209_coord "_209_coord"
OpName %_210_coordSampled "_210_coordSampled"
OpName %_211_7_MatrixEffect_Stage1_c0_c0 "_211_7_MatrixEffect_Stage1_c0_c0"
OpName %_212_8_0_TextureEffect_Stage1_c0_c0_c0 "_212_8_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_213_9_1_coords "_213_9_1_coords"
OpName %_214_10_2_inCoord "_214_10_2_inCoord"
OpName %_215_11_3_subsetCoord "_215_11_3_subsetCoord"
OpName %_216_12_4_clampedCoord "_216_12_4_clampedCoord"
OpName %_217_13_5_textureColor "_217_13_5_textureColor"
OpName %_218_14_6_snappedX "_218_14_6_snappedX"
OpName %_219_15_MatrixEffect_Stage1_c0_c0 "_219_15_MatrixEffect_Stage1_c0_c0"
OpName %_220_16_0_TextureEffect_Stage1_c0_c0_c0 "_220_16_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_221_17_1_coords "_221_17_1_coords"
OpName %_222_18_2_inCoord "_222_18_2_inCoord"
OpName %_223_19_3_subsetCoord "_223_19_3_subsetCoord"
OpName %_224_20_4_clampedCoord "_224_20_4_clampedCoord"
OpName %_225_21_5_textureColor "_225_21_5_textureColor"
OpName %_226_22_6_snappedX "_226_22_6_snappedX"
OpName %_227_23_MatrixEffect_Stage1_c0_c0 "_227_23_MatrixEffect_Stage1_c0_c0"
OpName %_228_24_0_TextureEffect_Stage1_c0_c0_c0 "_228_24_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_229_25_1_coords "_229_25_1_coords"
OpName %_230_26_2_inCoord "_230_26_2_inCoord"
OpName %_231_27_3_subsetCoord "_231_27_3_subsetCoord"
OpName %_232_28_4_clampedCoord "_232_28_4_clampedCoord"
OpName %_233_29_5_textureColor "_233_29_5_textureColor"
OpName %_234_30_6_snappedX "_234_30_6_snappedX"
OpName %_235_31_MatrixEffect_Stage1_c0_c0 "_235_31_MatrixEffect_Stage1_c0_c0"
OpName %_236_32_0_TextureEffect_Stage1_c0_c0_c0 "_236_32_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_237_33_1_coords "_237_33_1_coords"
OpName %_238_34_2_inCoord "_238_34_2_inCoord"
OpName %_239_35_3_subsetCoord "_239_35_3_subsetCoord"
OpName %_240_36_4_clampedCoord "_240_36_4_clampedCoord"
OpName %_241_37_5_textureColor "_241_37_5_textureColor"
OpName %_242_38_6_snappedX "_242_38_6_snappedX"
OpName %_243_39_MatrixEffect_Stage1_c0_c0 "_243_39_MatrixEffect_Stage1_c0_c0"
OpName %_244_40_0_TextureEffect_Stage1_c0_c0_c0 "_244_40_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_245_41_1_coords "_245_41_1_coords"
OpName %_246_42_2_inCoord "_246_42_2_inCoord"
OpName %_247_43_3_subsetCoord "_247_43_3_subsetCoord"
OpName %_248_44_4_clampedCoord "_248_44_4_clampedCoord"
OpName %_249_45_5_textureColor "_249_45_5_textureColor"
OpName %_250_46_6_snappedX "_250_46_6_snappedX"
OpName %_251_47_MatrixEffect_Stage1_c0_c0 "_251_47_MatrixEffect_Stage1_c0_c0"
OpName %_252_48_0_TextureEffect_Stage1_c0_c0_c0 "_252_48_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_253_49_1_coords "_253_49_1_coords"
OpName %_254_50_2_inCoord "_254_50_2_inCoord"
OpName %_255_51_3_subsetCoord "_255_51_3_subsetCoord"
OpName %_256_52_4_clampedCoord "_256_52_4_clampedCoord"
OpName %_257_53_5_textureColor "_257_53_5_textureColor"
OpName %_258_54_6_snappedX "_258_54_6_snappedX"
OpName %_259_55_MatrixEffect_Stage1_c0_c0 "_259_55_MatrixEffect_Stage1_c0_c0"
OpName %_260_56_0_TextureEffect_Stage1_c0_c0_c0 "_260_56_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_261_57_1_coords "_261_57_1_coords"
OpName %_262_58_2_inCoord "_262_58_2_inCoord"
OpName %_263_59_3_subsetCoord "_263_59_3_subsetCoord"
OpName %_264_60_4_clampedCoord "_264_60_4_clampedCoord"
OpName %_265_61_5_textureColor "_265_61_5_textureColor"
OpName %_266_62_6_snappedX "_266_62_6_snappedX"
OpName %_267_63_MatrixEffect_Stage1_c0_c0 "_267_63_MatrixEffect_Stage1_c0_c0"
OpName %_268_64_0_TextureEffect_Stage1_c0_c0_c0 "_268_64_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_269_65_1_coords "_269_65_1_coords"
OpName %_270_66_2_inCoord "_270_66_2_inCoord"
OpName %_271_67_3_subsetCoord "_271_67_3_subsetCoord"
OpName %_272_68_4_clampedCoord "_272_68_4_clampedCoord"
OpName %_273_69_5_textureColor "_273_69_5_textureColor"
OpName %_274_70_6_snappedX "_274_70_6_snappedX"
OpName %_275_71_MatrixEffect_Stage1_c0_c0 "_275_71_MatrixEffect_Stage1_c0_c0"
OpName %_276_72_0_TextureEffect_Stage1_c0_c0_c0 "_276_72_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_277_73_1_coords "_277_73_1_coords"
OpName %_278_74_2_inCoord "_278_74_2_inCoord"
OpName %_279_75_3_subsetCoord "_279_75_3_subsetCoord"
OpName %_280_76_4_clampedCoord "_280_76_4_clampedCoord"
OpName %_281_77_5_textureColor "_281_77_5_textureColor"
OpName %_282_78_6_snappedX "_282_78_6_snappedX"
OpName %_283_79_MatrixEffect_Stage1_c0_c0 "_283_79_MatrixEffect_Stage1_c0_c0"
OpName %_284_80_0_TextureEffect_Stage1_c0_c0_c0 "_284_80_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_285_81_1_coords "_285_81_1_coords"
OpName %_286_82_2_inCoord "_286_82_2_inCoord"
OpName %_287_83_3_subsetCoord "_287_83_3_subsetCoord"
OpName %_288_84_4_clampedCoord "_288_84_4_clampedCoord"
OpName %_289_85_5_textureColor "_289_85_5_textureColor"
OpName %_290_86_6_snappedX "_290_86_6_snappedX"
OpName %_291_87_MatrixEffect_Stage1_c0_c0 "_291_87_MatrixEffect_Stage1_c0_c0"
OpName %_292_88_0_TextureEffect_Stage1_c0_c0_c0 "_292_88_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_293_89_1_coords "_293_89_1_coords"
OpName %_294_90_2_inCoord "_294_90_2_inCoord"
OpName %_295_91_3_subsetCoord "_295_91_3_subsetCoord"
OpName %_296_92_4_clampedCoord "_296_92_4_clampedCoord"
OpName %_297_93_5_textureColor "_297_93_5_textureColor"
OpName %_298_94_6_snappedX "_298_94_6_snappedX"
OpName %_299_95_MatrixEffect_Stage1_c0_c0 "_299_95_MatrixEffect_Stage1_c0_c0"
OpName %_300_96_0_TextureEffect_Stage1_c0_c0_c0 "_300_96_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_301_97_1_coords "_301_97_1_coords"
OpName %_302_98_2_inCoord "_302_98_2_inCoord"
OpName %_303_99_3_subsetCoord "_303_99_3_subsetCoord"
OpName %_304_100_4_clampedCoord "_304_100_4_clampedCoord"
OpName %_305_101_5_textureColor "_305_101_5_textureColor"
OpName %_306_102_6_snappedX "_306_102_6_snappedX"
OpName %_307_103_MatrixEffect_Stage1_c0_c0 "_307_103_MatrixEffect_Stage1_c0_c0"
OpName %_308_104_0_TextureEffect_Stage1_c0_c0_c0 "_308_104_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_309_105_1_coords "_309_105_1_coords"
OpName %_310_106_2_inCoord "_310_106_2_inCoord"
OpName %_311_107_3_subsetCoord "_311_107_3_subsetCoord"
OpName %_312_108_4_clampedCoord "_312_108_4_clampedCoord"
OpName %_313_109_5_textureColor "_313_109_5_textureColor"
OpName %_314_110_6_snappedX "_314_110_6_snappedX"
OpName %_315_111_MatrixEffect_Stage1_c0_c0 "_315_111_MatrixEffect_Stage1_c0_c0"
OpName %_316_112_0_TextureEffect_Stage1_c0_c0_c0 "_316_112_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_317_113_1_coords "_317_113_1_coords"
OpName %_318_114_2_inCoord "_318_114_2_inCoord"
OpName %_319_115_3_subsetCoord "_319_115_3_subsetCoord"
OpName %_320_116_4_clampedCoord "_320_116_4_clampedCoord"
OpName %_321_117_5_textureColor "_321_117_5_textureColor"
OpName %_322_118_6_snappedX "_322_118_6_snappedX"
OpName %_323_119_MatrixEffect_Stage1_c0_c0 "_323_119_MatrixEffect_Stage1_c0_c0"
OpName %_324_120_0_TextureEffect_Stage1_c0_c0_c0 "_324_120_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_325_121_1_coords "_325_121_1_coords"
OpName %_326_122_2_inCoord "_326_122_2_inCoord"
OpName %_327_123_3_subsetCoord "_327_123_3_subsetCoord"
OpName %_328_124_4_clampedCoord "_328_124_4_clampedCoord"
OpName %_329_125_5_textureColor "_329_125_5_textureColor"
OpName %_330_126_6_snappedX "_330_126_6_snappedX"
OpName %_331_127_MatrixEffect_Stage1_c0_c0 "_331_127_MatrixEffect_Stage1_c0_c0"
OpName %_332_128_0_TextureEffect_Stage1_c0_c0_c0 "_332_128_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_333_129_1_coords "_333_129_1_coords"
OpName %_334_130_2_inCoord "_334_130_2_inCoord"
OpName %_335_131_3_subsetCoord "_335_131_3_subsetCoord"
OpName %_336_132_4_clampedCoord "_336_132_4_clampedCoord"
OpName %_337_133_5_textureColor "_337_133_5_textureColor"
OpName %_338_134_6_snappedX "_338_134_6_snappedX"
OpName %_339_135_MatrixEffect_Stage1_c0_c0 "_339_135_MatrixEffect_Stage1_c0_c0"
OpName %_340_136_0_TextureEffect_Stage1_c0_c0_c0 "_340_136_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_341_137_1_coords "_341_137_1_coords"
OpName %_342_138_2_inCoord "_342_138_2_inCoord"
OpName %_343_139_3_subsetCoord "_343_139_3_subsetCoord"
OpName %_344_140_4_clampedCoord "_344_140_4_clampedCoord"
OpName %_345_141_5_textureColor "_345_141_5_textureColor"
OpName %_346_142_6_snappedX "_346_142_6_snappedX"
OpName %_347_143_MatrixEffect_Stage1_c0_c0 "_347_143_MatrixEffect_Stage1_c0_c0"
OpName %_348_144_0_TextureEffect_Stage1_c0_c0_c0 "_348_144_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_349_145_1_coords "_349_145_1_coords"
OpName %_350_146_2_inCoord "_350_146_2_inCoord"
OpName %_351_147_3_subsetCoord "_351_147_3_subsetCoord"
OpName %_352_148_4_clampedCoord "_352_148_4_clampedCoord"
OpName %_353_149_5_textureColor "_353_149_5_textureColor"
OpName %_354_150_6_snappedX "_354_150_6_snappedX"
OpName %_355_151_MatrixEffect_Stage1_c0_c0 "_355_151_MatrixEffect_Stage1_c0_c0"
OpName %_356_152_0_TextureEffect_Stage1_c0_c0_c0 "_356_152_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_357_153_1_coords "_357_153_1_coords"
OpName %_358_154_2_inCoord "_358_154_2_inCoord"
OpName %_359_155_3_subsetCoord "_359_155_3_subsetCoord"
OpName %_360_156_4_clampedCoord "_360_156_4_clampedCoord"
OpName %_361_157_5_textureColor "_361_157_5_textureColor"
OpName %_362_158_6_snappedX "_362_158_6_snappedX"
OpName %_363_159_MatrixEffect_Stage1_c0_c0 "_363_159_MatrixEffect_Stage1_c0_c0"
OpName %_364_160_0_TextureEffect_Stage1_c0_c0_c0 "_364_160_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_365_161_1_coords "_365_161_1_coords"
OpName %_366_162_2_inCoord "_366_162_2_inCoord"
OpName %_367_163_3_subsetCoord "_367_163_3_subsetCoord"
OpName %_368_164_4_clampedCoord "_368_164_4_clampedCoord"
OpName %_369_165_5_textureColor "_369_165_5_textureColor"
OpName %_370_166_6_snappedX "_370_166_6_snappedX"
OpName %_371_167_MatrixEffect_Stage1_c0_c0 "_371_167_MatrixEffect_Stage1_c0_c0"
OpName %_372_168_0_TextureEffect_Stage1_c0_c0_c0 "_372_168_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_373_169_1_coords "_373_169_1_coords"
OpName %_374_170_2_inCoord "_374_170_2_inCoord"
OpName %_375_171_3_subsetCoord "_375_171_3_subsetCoord"
OpName %_376_172_4_clampedCoord "_376_172_4_clampedCoord"
OpName %_377_173_5_textureColor "_377_173_5_textureColor"
OpName %_378_174_6_snappedX "_378_174_6_snappedX"
OpName %_379_175_MatrixEffect_Stage1_c0_c0 "_379_175_MatrixEffect_Stage1_c0_c0"
OpName %_380_176_0_TextureEffect_Stage1_c0_c0_c0 "_380_176_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_381_177_1_coords "_381_177_1_coords"
OpName %_382_178_2_inCoord "_382_178_2_inCoord"
OpName %_383_179_3_subsetCoord "_383_179_3_subsetCoord"
OpName %_384_180_4_clampedCoord "_384_180_4_clampedCoord"
OpName %_385_181_5_textureColor "_385_181_5_textureColor"
OpName %_386_182_6_snappedX "_386_182_6_snappedX"
OpName %_387_183_MatrixEffect_Stage1_c0_c0 "_387_183_MatrixEffect_Stage1_c0_c0"
OpName %_388_184_0_TextureEffect_Stage1_c0_c0_c0 "_388_184_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_389_185_1_coords "_389_185_1_coords"
OpName %_390_186_2_inCoord "_390_186_2_inCoord"
OpName %_391_187_3_subsetCoord "_391_187_3_subsetCoord"
OpName %_392_188_4_clampedCoord "_392_188_4_clampedCoord"
OpName %_393_189_5_textureColor "_393_189_5_textureColor"
OpName %_394_190_6_snappedX "_394_190_6_snappedX"
OpName %_395_191_MatrixEffect_Stage1_c0_c0 "_395_191_MatrixEffect_Stage1_c0_c0"
OpName %_396_192_0_TextureEffect_Stage1_c0_c0_c0 "_396_192_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_397_193_1_coords "_397_193_1_coords"
OpName %_398_194_2_inCoord "_398_194_2_inCoord"
OpName %_399_195_3_subsetCoord "_399_195_3_subsetCoord"
OpName %_400_196_4_clampedCoord "_400_196_4_clampedCoord"
OpName %_401_197_5_textureColor "_401_197_5_textureColor"
OpName %_402_198_6_snappedX "_402_198_6_snappedX"
OpName %_403_199_MatrixEffect_Stage1_c0_c0 "_403_199_MatrixEffect_Stage1_c0_c0"
OpName %_404_200_0_TextureEffect_Stage1_c0_c0_c0 "_404_200_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_405_201_1_coords "_405_201_1_coords"
OpName %_406_202_2_inCoord "_406_202_2_inCoord"
OpName %_407_203_3_subsetCoord "_407_203_3_subsetCoord"
OpName %_408_204_4_clampedCoord "_408_204_4_clampedCoord"
OpName %_409_205_5_textureColor "_409_205_5_textureColor"
OpName %_410_206_6_snappedX "_410_206_6_snappedX"
OpDecorate %_arr_v4float_int_7 ArrayStride 16
OpMemberDecorate %uniformBuffer 0 Offset 0
OpMemberDecorate %uniformBuffer 1 Offset 16
OpMemberDecorate %uniformBuffer 1 RelaxedPrecision
OpMemberDecorate %uniformBuffer 2 Offset 32
OpMemberDecorate %uniformBuffer 2 RelaxedPrecision
OpMemberDecorate %uniformBuffer 3 Offset 144
OpMemberDecorate %uniformBuffer 3 ColMajor
OpMemberDecorate %uniformBuffer 3 MatrixStride 16
OpMemberDecorate %uniformBuffer 4 Offset 192
OpMemberDecorate %uniformBuffer 4 RelaxedPrecision
OpMemberDecorate %uniformBuffer 5 Offset 208
OpMemberDecorate %uniformBuffer 6 Offset 224
OpDecorate %uniformBuffer Block
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %41 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %549 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
OpDecorate %625 RelaxedPrecision
OpDecorate %627 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %665 RelaxedPrecision
OpDecorate %693 RelaxedPrecision
OpDecorate %694 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %696 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %699 RelaxedPrecision
OpDecorate %702 RelaxedPrecision
OpDecorate %705 RelaxedPrecision
OpDecorate %737 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %769 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %774 RelaxedPrecision
OpDecorate %777 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %838 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %840 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
OpDecorate %849 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %909 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %911 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %913 RelaxedPrecision
OpDecorate %915 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %921 RelaxedPrecision
OpDecorate %953 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %984 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1057 RelaxedPrecision
OpDecorate %1059 RelaxedPrecision
OpDecorate %1062 RelaxedPrecision
OpDecorate %1065 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1125 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1197 RelaxedPrecision
OpDecorate %1198 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1200 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1241 RelaxedPrecision
OpDecorate %1269 RelaxedPrecision
OpDecorate %1270 RelaxedPrecision
OpDecorate %1271 RelaxedPrecision
OpDecorate %1272 RelaxedPrecision
OpDecorate %1273 RelaxedPrecision
OpDecorate %1275 RelaxedPrecision
OpDecorate %1278 RelaxedPrecision
OpDecorate %1281 RelaxedPrecision
OpDecorate %1313 RelaxedPrecision
OpDecorate %1341 RelaxedPrecision
OpDecorate %1342 RelaxedPrecision
OpDecorate %1343 RelaxedPrecision
OpDecorate %1344 RelaxedPrecision
OpDecorate %1345 RelaxedPrecision
OpDecorate %1347 RelaxedPrecision
OpDecorate %1350 RelaxedPrecision
OpDecorate %1353 RelaxedPrecision
OpDecorate %1385 RelaxedPrecision
OpDecorate %1413 RelaxedPrecision
OpDecorate %1414 RelaxedPrecision
OpDecorate %1415 RelaxedPrecision
OpDecorate %1416 RelaxedPrecision
OpDecorate %1417 RelaxedPrecision
OpDecorate %1419 RelaxedPrecision
OpDecorate %1422 RelaxedPrecision
OpDecorate %1425 RelaxedPrecision
OpDecorate %1457 RelaxedPrecision
OpDecorate %1485 RelaxedPrecision
OpDecorate %1486 RelaxedPrecision
OpDecorate %1487 RelaxedPrecision
OpDecorate %1488 RelaxedPrecision
OpDecorate %1489 RelaxedPrecision
OpDecorate %1491 RelaxedPrecision
OpDecorate %1494 RelaxedPrecision
OpDecorate %1497 RelaxedPrecision
OpDecorate %1529 RelaxedPrecision
OpDecorate %1557 RelaxedPrecision
OpDecorate %1558 RelaxedPrecision
OpDecorate %1559 RelaxedPrecision
OpDecorate %1560 RelaxedPrecision
OpDecorate %1561 RelaxedPrecision
OpDecorate %1563 RelaxedPrecision
OpDecorate %1566 RelaxedPrecision
OpDecorate %1569 RelaxedPrecision
OpDecorate %1601 RelaxedPrecision
OpDecorate %1629 RelaxedPrecision
OpDecorate %1630 RelaxedPrecision
OpDecorate %1631 RelaxedPrecision
OpDecorate %1632 RelaxedPrecision
OpDecorate %1633 RelaxedPrecision
OpDecorate %1635 RelaxedPrecision
OpDecorate %1638 RelaxedPrecision
OpDecorate %1641 RelaxedPrecision
OpDecorate %1673 RelaxedPrecision
OpDecorate %1701 RelaxedPrecision
OpDecorate %1702 RelaxedPrecision
OpDecorate %1703 RelaxedPrecision
OpDecorate %1704 RelaxedPrecision
OpDecorate %1705 RelaxedPrecision
OpDecorate %1707 RelaxedPrecision
OpDecorate %1710 RelaxedPrecision
OpDecorate %1713 RelaxedPrecision
OpDecorate %1745 RelaxedPrecision
OpDecorate %1773 RelaxedPrecision
OpDecorate %1774 RelaxedPrecision
OpDecorate %1775 RelaxedPrecision
OpDecorate %1776 RelaxedPrecision
OpDecorate %1777 RelaxedPrecision
OpDecorate %1779 RelaxedPrecision
OpDecorate %1782 RelaxedPrecision
OpDecorate %1785 RelaxedPrecision
OpDecorate %1817 RelaxedPrecision
OpDecorate %1845 RelaxedPrecision
OpDecorate %1846 RelaxedPrecision
OpDecorate %1847 RelaxedPrecision
OpDecorate %1848 RelaxedPrecision
OpDecorate %1849 RelaxedPrecision
OpDecorate %1851 RelaxedPrecision
OpDecorate %1854 RelaxedPrecision
OpDecorate %1857 RelaxedPrecision
OpDecorate %1859 RelaxedPrecision
OpDecorate %1860 RelaxedPrecision
OpDecorate %1861 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_7 = OpConstant %int 7
%_arr_v4float_int_7 = OpTypeArray %v4float %int_7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%uniformBuffer = OpTypeStruct %v4float %v2float %_arr_v4float_int_7 %mat3v3float %v4float %v4float %v4float
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
%3 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%22 = OpTypeImage %float 2D 0 0 0 1 Unknown
%21 = OpTypeSampledImage %22
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_21 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%void = OpTypeVoid
%26 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%32 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_12 = OpConstant %float 12
%int_1 = OpConstant %int 1
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%45 = OpConstantComposite %v2float %float_0 %float_0
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%float_1 = OpConstant %float 1
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0_5 = OpConstant %float 0.5
%true = OpConstantTrue %bool
%int_5 = OpConstant %int 5
%int_4 = OpConstant %int 4
%int_2 = OpConstant %int 2
%main = OpFunction %void None %26
%27 = OpLabel
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_207_GaussianConvolution_Stage1_c0 = OpVariable %_ptr_Function_v4float Function
%_208_output = OpVariable %_ptr_Function_v4float Function
%_209_coord = OpVariable %_ptr_Function_v2float Function
%_210_coordSampled = OpVariable %_ptr_Function_v2float Function
%_211_7_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_212_8_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_213_9_1_coords = OpVariable %_ptr_Function_v2float Function
%_214_10_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_215_11_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_216_12_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_217_13_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_218_14_6_snappedX = OpVariable %_ptr_Function_float Function
%_219_15_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_220_16_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_221_17_1_coords = OpVariable %_ptr_Function_v2float Function
%_222_18_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_223_19_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_224_20_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_225_21_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_226_22_6_snappedX = OpVariable %_ptr_Function_float Function
%_227_23_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_228_24_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_229_25_1_coords = OpVariable %_ptr_Function_v2float Function
%_230_26_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_231_27_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_232_28_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_233_29_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_234_30_6_snappedX = OpVariable %_ptr_Function_float Function
%_235_31_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_236_32_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_237_33_1_coords = OpVariable %_ptr_Function_v2float Function
%_238_34_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_239_35_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_240_36_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_241_37_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_242_38_6_snappedX = OpVariable %_ptr_Function_float Function
%_243_39_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_244_40_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_245_41_1_coords = OpVariable %_ptr_Function_v2float Function
%_246_42_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_247_43_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_248_44_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_249_45_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_250_46_6_snappedX = OpVariable %_ptr_Function_float Function
%_251_47_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_252_48_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_253_49_1_coords = OpVariable %_ptr_Function_v2float Function
%_254_50_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_255_51_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_256_52_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_257_53_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_258_54_6_snappedX = OpVariable %_ptr_Function_float Function
%_259_55_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_260_56_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_261_57_1_coords = OpVariable %_ptr_Function_v2float Function
%_262_58_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_263_59_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_264_60_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_265_61_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_266_62_6_snappedX = OpVariable %_ptr_Function_float Function
%_267_63_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_268_64_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_269_65_1_coords = OpVariable %_ptr_Function_v2float Function
%_270_66_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_271_67_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_272_68_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_273_69_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_274_70_6_snappedX = OpVariable %_ptr_Function_float Function
%_275_71_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_276_72_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_277_73_1_coords = OpVariable %_ptr_Function_v2float Function
%_278_74_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_279_75_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_280_76_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_281_77_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_282_78_6_snappedX = OpVariable %_ptr_Function_float Function
%_283_79_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_284_80_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_285_81_1_coords = OpVariable %_ptr_Function_v2float Function
%_286_82_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_287_83_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_288_84_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_289_85_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_290_86_6_snappedX = OpVariable %_ptr_Function_float Function
%_291_87_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_292_88_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_293_89_1_coords = OpVariable %_ptr_Function_v2float Function
%_294_90_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_295_91_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_296_92_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_297_93_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_298_94_6_snappedX = OpVariable %_ptr_Function_float Function
%_299_95_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_300_96_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_301_97_1_coords = OpVariable %_ptr_Function_v2float Function
%_302_98_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_303_99_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_304_100_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_305_101_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_306_102_6_snappedX = OpVariable %_ptr_Function_float Function
%_307_103_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_308_104_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_309_105_1_coords = OpVariable %_ptr_Function_v2float Function
%_310_106_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_311_107_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_312_108_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_313_109_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_314_110_6_snappedX = OpVariable %_ptr_Function_float Function
%_315_111_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_316_112_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_317_113_1_coords = OpVariable %_ptr_Function_v2float Function
%_318_114_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_319_115_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_320_116_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_321_117_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_322_118_6_snappedX = OpVariable %_ptr_Function_float Function
%_323_119_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_324_120_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_325_121_1_coords = OpVariable %_ptr_Function_v2float Function
%_326_122_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_327_123_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_328_124_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_329_125_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_330_126_6_snappedX = OpVariable %_ptr_Function_float Function
%_331_127_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_332_128_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_333_129_1_coords = OpVariable %_ptr_Function_v2float Function
%_334_130_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_335_131_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_336_132_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_337_133_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_338_134_6_snappedX = OpVariable %_ptr_Function_float Function
%_339_135_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_340_136_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_341_137_1_coords = OpVariable %_ptr_Function_v2float Function
%_342_138_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_343_139_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_344_140_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_345_141_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_346_142_6_snappedX = OpVariable %_ptr_Function_float Function
%_347_143_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_348_144_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_349_145_1_coords = OpVariable %_ptr_Function_v2float Function
%_350_146_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_351_147_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_352_148_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_353_149_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_354_150_6_snappedX = OpVariable %_ptr_Function_float Function
%_355_151_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_356_152_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_357_153_1_coords = OpVariable %_ptr_Function_v2float Function
%_358_154_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_359_155_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_360_156_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_361_157_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_362_158_6_snappedX = OpVariable %_ptr_Function_float Function
%_363_159_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_364_160_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_365_161_1_coords = OpVariable %_ptr_Function_v2float Function
%_366_162_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_367_163_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_368_164_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_369_165_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_370_166_6_snappedX = OpVariable %_ptr_Function_float Function
%_371_167_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_372_168_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_373_169_1_coords = OpVariable %_ptr_Function_v2float Function
%_374_170_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_375_171_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_376_172_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_377_173_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_378_174_6_snappedX = OpVariable %_ptr_Function_float Function
%_379_175_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_380_176_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_381_177_1_coords = OpVariable %_ptr_Function_v2float Function
%_382_178_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_383_179_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_384_180_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_385_181_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_386_182_6_snappedX = OpVariable %_ptr_Function_float Function
%_387_183_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_388_184_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_389_185_1_coords = OpVariable %_ptr_Function_v2float Function
%_390_186_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_391_187_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_392_188_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_393_189_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_394_190_6_snappedX = OpVariable %_ptr_Function_float Function
%_395_191_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_396_192_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_397_193_1_coords = OpVariable %_ptr_Function_v2float Function
%_398_194_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_399_195_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_400_196_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_401_197_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_402_198_6_snappedX = OpVariable %_ptr_Function_float Function
%_403_199_MatrixEffect_Stage1_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_404_200_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_405_201_1_coords = OpVariable %_ptr_Function_v2float Function
%_406_202_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_407_203_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_408_204_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_409_205_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_410_206_6_snappedX = OpVariable %_ptr_Function_float Function
OpStore %_208_output %32
%36 = OpLoad %v2float %vLocalCoord_Stage0
%39 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%41 = OpLoad %v2float %39
%42 = OpVectorTimesScalar %v2float %41 %float_12
%43 = OpFSub %v2float %36 %42
OpStore %_209_coord %43
OpStore %_210_coordSampled %45
%46 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %46
%51 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%53 = OpLoad %mat3v3float %51
%54 = OpLoad %v2float %_210_coordSampled
%55 = OpCompositeExtract %float %54 0
%56 = OpCompositeExtract %float %54 1
%58 = OpCompositeConstruct %v3float %55 %56 %float_1
%59 = OpMatrixTimesVector %v3float %53 %58
%60 = OpVectorShuffle %v2float %59 %59 0 1
OpStore %_213_9_1_coords %60
%62 = OpLoad %v2float %_213_9_1_coords
OpStore %_214_10_2_inCoord %62
%63 = OpLoad %v2float %_214_10_2_inCoord
%65 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%67 = OpLoad %v4float %65
%68 = OpVectorShuffle %v2float %67 %67 0 1
%69 = OpFMul %v2float %63 %68
OpStore %_214_10_2_inCoord %69
%71 = OpLoad %v2float %_214_10_2_inCoord
%72 = OpCompositeExtract %float %71 0
%73 = OpAccessChain %_ptr_Function_float %_215_11_3_subsetCoord %int_0
OpStore %73 %72
%76 = OpLoad %v2float %_214_10_2_inCoord
%77 = OpCompositeExtract %float %76 1
%78 = OpAccessChain %_ptr_Function_float %_215_11_3_subsetCoord %int_1
OpStore %78 %77
%80 = OpLoad %v2float %_215_11_3_subsetCoord
OpStore %_216_12_4_clampedCoord %80
%83 = OpLoad %21 %uTextureSampler_0_Stage1
%84 = OpLoad %v2float %_216_12_4_clampedCoord
%85 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%86 = OpLoad %v4float %85
%87 = OpVectorShuffle %v2float %86 %86 2 3
%88 = OpFMul %v2float %84 %87
%82 = OpImageSampleImplicitLod %v4float %83 %88
OpStore %_217_13_5_textureColor %82
%91 = OpLoad %v2float %_214_10_2_inCoord
%92 = OpCompositeExtract %float %91 0
%94 = OpFAdd %float %92 %float_0_00100000005
%90 = OpExtInst %float %1 Floor %94
%96 = OpFAdd %float %90 %float_0_5
OpStore %_218_14_6_snappedX %96
%98 = OpLoad %float %_218_14_6_snappedX
%100 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%101 = OpLoad %v4float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpFOrdLessThan %bool %98 %102
OpSelectionMerge %105 None
OpBranchConditional %103 %105 %104
%104 = OpLabel
%106 = OpLoad %float %_218_14_6_snappedX
%107 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 2
%110 = OpFOrdGreaterThan %bool %106 %109
OpBranch %105
%105 = OpLabel
%111 = OpPhi %bool %true %27 %110 %104
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%116 = OpLoad %v4float %115
OpStore %_217_13_5_textureColor %116
OpBranch %113
%113 = OpLabel
%117 = OpLoad %v4float %_217_13_5_textureColor
OpStore %_212_8_0_TextureEffect_Stage1_c0_c0_c0 %117
%118 = OpLoad %v4float %_212_8_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_211_7_MatrixEffect_Stage1_c0_c0 %118
%119 = OpLoad %v4float %_208_output
%120 = OpLoad %v4float %_211_7_MatrixEffect_Stage1_c0_c0
%122 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_0
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 0
%125 = OpVectorTimesScalar %v4float %120 %124
%126 = OpFAdd %v4float %119 %125
OpStore %_208_output %126
%127 = OpLoad %v2float %_209_coord
%128 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%129 = OpLoad %v2float %128
%130 = OpFAdd %v2float %127 %129
OpStore %_209_coord %130
%131 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %131
%135 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%136 = OpLoad %mat3v3float %135
%137 = OpLoad %v2float %_210_coordSampled
%138 = OpCompositeExtract %float %137 0
%139 = OpCompositeExtract %float %137 1
%140 = OpCompositeConstruct %v3float %138 %139 %float_1
%141 = OpMatrixTimesVector %v3float %136 %140
%142 = OpVectorShuffle %v2float %141 %141 0 1
OpStore %_221_17_1_coords %142
%144 = OpLoad %v2float %_221_17_1_coords
OpStore %_222_18_2_inCoord %144
%145 = OpLoad %v2float %_222_18_2_inCoord
%146 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%147 = OpLoad %v4float %146
%148 = OpVectorShuffle %v2float %147 %147 0 1
%149 = OpFMul %v2float %145 %148
OpStore %_222_18_2_inCoord %149
%151 = OpLoad %v2float %_222_18_2_inCoord
%152 = OpCompositeExtract %float %151 0
%153 = OpAccessChain %_ptr_Function_float %_223_19_3_subsetCoord %int_0
OpStore %153 %152
%154 = OpLoad %v2float %_222_18_2_inCoord
%155 = OpCompositeExtract %float %154 1
%156 = OpAccessChain %_ptr_Function_float %_223_19_3_subsetCoord %int_1
OpStore %156 %155
%158 = OpLoad %v2float %_223_19_3_subsetCoord
OpStore %_224_20_4_clampedCoord %158
%161 = OpLoad %21 %uTextureSampler_0_Stage1
%162 = OpLoad %v2float %_224_20_4_clampedCoord
%163 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%164 = OpLoad %v4float %163
%165 = OpVectorShuffle %v2float %164 %164 2 3
%166 = OpFMul %v2float %162 %165
%160 = OpImageSampleImplicitLod %v4float %161 %166
OpStore %_225_21_5_textureColor %160
%169 = OpLoad %v2float %_222_18_2_inCoord
%170 = OpCompositeExtract %float %169 0
%171 = OpFAdd %float %170 %float_0_00100000005
%168 = OpExtInst %float %1 Floor %171
%172 = OpFAdd %float %168 %float_0_5
OpStore %_226_22_6_snappedX %172
%173 = OpLoad %float %_226_22_6_snappedX
%174 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 0
%177 = OpFOrdLessThan %bool %173 %176
OpSelectionMerge %179 None
OpBranchConditional %177 %179 %178
%178 = OpLabel
%180 = OpLoad %float %_226_22_6_snappedX
%181 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%182 = OpLoad %v4float %181
%183 = OpCompositeExtract %float %182 2
%184 = OpFOrdGreaterThan %bool %180 %183
OpBranch %179
%179 = OpLabel
%185 = OpPhi %bool %true %113 %184 %178
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%189 = OpLoad %v4float %188
OpStore %_225_21_5_textureColor %189
OpBranch %187
%187 = OpLabel
%190 = OpLoad %v4float %_225_21_5_textureColor
OpStore %_220_16_0_TextureEffect_Stage1_c0_c0_c0 %190
%191 = OpLoad %v4float %_220_16_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_219_15_MatrixEffect_Stage1_c0_c0 %191
%192 = OpLoad %v4float %_208_output
%193 = OpLoad %v4float %_219_15_MatrixEffect_Stage1_c0_c0
%194 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_0
%195 = OpLoad %v4float %194
%196 = OpCompositeExtract %float %195 1
%197 = OpVectorTimesScalar %v4float %193 %196
%198 = OpFAdd %v4float %192 %197
OpStore %_208_output %198
%199 = OpLoad %v2float %_209_coord
%200 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%201 = OpLoad %v2float %200
%202 = OpFAdd %v2float %199 %201
OpStore %_209_coord %202
%203 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %203
%207 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%208 = OpLoad %mat3v3float %207
%209 = OpLoad %v2float %_210_coordSampled
%210 = OpCompositeExtract %float %209 0
%211 = OpCompositeExtract %float %209 1
%212 = OpCompositeConstruct %v3float %210 %211 %float_1
%213 = OpMatrixTimesVector %v3float %208 %212
%214 = OpVectorShuffle %v2float %213 %213 0 1
OpStore %_229_25_1_coords %214
%216 = OpLoad %v2float %_229_25_1_coords
OpStore %_230_26_2_inCoord %216
%217 = OpLoad %v2float %_230_26_2_inCoord
%218 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%219 = OpLoad %v4float %218
%220 = OpVectorShuffle %v2float %219 %219 0 1
%221 = OpFMul %v2float %217 %220
OpStore %_230_26_2_inCoord %221
%223 = OpLoad %v2float %_230_26_2_inCoord
%224 = OpCompositeExtract %float %223 0
%225 = OpAccessChain %_ptr_Function_float %_231_27_3_subsetCoord %int_0
OpStore %225 %224
%226 = OpLoad %v2float %_230_26_2_inCoord
%227 = OpCompositeExtract %float %226 1
%228 = OpAccessChain %_ptr_Function_float %_231_27_3_subsetCoord %int_1
OpStore %228 %227
%230 = OpLoad %v2float %_231_27_3_subsetCoord
OpStore %_232_28_4_clampedCoord %230
%233 = OpLoad %21 %uTextureSampler_0_Stage1
%234 = OpLoad %v2float %_232_28_4_clampedCoord
%235 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%236 = OpLoad %v4float %235
%237 = OpVectorShuffle %v2float %236 %236 2 3
%238 = OpFMul %v2float %234 %237
%232 = OpImageSampleImplicitLod %v4float %233 %238
OpStore %_233_29_5_textureColor %232
%241 = OpLoad %v2float %_230_26_2_inCoord
%242 = OpCompositeExtract %float %241 0
%243 = OpFAdd %float %242 %float_0_00100000005
%240 = OpExtInst %float %1 Floor %243
%244 = OpFAdd %float %240 %float_0_5
OpStore %_234_30_6_snappedX %244
%245 = OpLoad %float %_234_30_6_snappedX
%246 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 0
%249 = OpFOrdLessThan %bool %245 %248
OpSelectionMerge %251 None
OpBranchConditional %249 %251 %250
%250 = OpLabel
%252 = OpLoad %float %_234_30_6_snappedX
%253 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%254 = OpLoad %v4float %253
%255 = OpCompositeExtract %float %254 2
%256 = OpFOrdGreaterThan %bool %252 %255
OpBranch %251
%251 = OpLabel
%257 = OpPhi %bool %true %187 %256 %250
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%261 = OpLoad %v4float %260
OpStore %_233_29_5_textureColor %261
OpBranch %259
%259 = OpLabel
%262 = OpLoad %v4float %_233_29_5_textureColor
OpStore %_228_24_0_TextureEffect_Stage1_c0_c0_c0 %262
%263 = OpLoad %v4float %_228_24_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_227_23_MatrixEffect_Stage1_c0_c0 %263
%264 = OpLoad %v4float %_208_output
%265 = OpLoad %v4float %_227_23_MatrixEffect_Stage1_c0_c0
%266 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_0
%267 = OpLoad %v4float %266
%268 = OpCompositeExtract %float %267 2
%269 = OpVectorTimesScalar %v4float %265 %268
%270 = OpFAdd %v4float %264 %269
OpStore %_208_output %270
%271 = OpLoad %v2float %_209_coord
%272 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%273 = OpLoad %v2float %272
%274 = OpFAdd %v2float %271 %273
OpStore %_209_coord %274
%275 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %275
%279 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%280 = OpLoad %mat3v3float %279
%281 = OpLoad %v2float %_210_coordSampled
%282 = OpCompositeExtract %float %281 0
%283 = OpCompositeExtract %float %281 1
%284 = OpCompositeConstruct %v3float %282 %283 %float_1
%285 = OpMatrixTimesVector %v3float %280 %284
%286 = OpVectorShuffle %v2float %285 %285 0 1
OpStore %_237_33_1_coords %286
%288 = OpLoad %v2float %_237_33_1_coords
OpStore %_238_34_2_inCoord %288
%289 = OpLoad %v2float %_238_34_2_inCoord
%290 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%291 = OpLoad %v4float %290
%292 = OpVectorShuffle %v2float %291 %291 0 1
%293 = OpFMul %v2float %289 %292
OpStore %_238_34_2_inCoord %293
%295 = OpLoad %v2float %_238_34_2_inCoord
%296 = OpCompositeExtract %float %295 0
%297 = OpAccessChain %_ptr_Function_float %_239_35_3_subsetCoord %int_0
OpStore %297 %296
%298 = OpLoad %v2float %_238_34_2_inCoord
%299 = OpCompositeExtract %float %298 1
%300 = OpAccessChain %_ptr_Function_float %_239_35_3_subsetCoord %int_1
OpStore %300 %299
%302 = OpLoad %v2float %_239_35_3_subsetCoord
OpStore %_240_36_4_clampedCoord %302
%305 = OpLoad %21 %uTextureSampler_0_Stage1
%306 = OpLoad %v2float %_240_36_4_clampedCoord
%307 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%308 = OpLoad %v4float %307
%309 = OpVectorShuffle %v2float %308 %308 2 3
%310 = OpFMul %v2float %306 %309
%304 = OpImageSampleImplicitLod %v4float %305 %310
OpStore %_241_37_5_textureColor %304
%313 = OpLoad %v2float %_238_34_2_inCoord
%314 = OpCompositeExtract %float %313 0
%315 = OpFAdd %float %314 %float_0_00100000005
%312 = OpExtInst %float %1 Floor %315
%316 = OpFAdd %float %312 %float_0_5
OpStore %_242_38_6_snappedX %316
%317 = OpLoad %float %_242_38_6_snappedX
%318 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%319 = OpLoad %v4float %318
%320 = OpCompositeExtract %float %319 0
%321 = OpFOrdLessThan %bool %317 %320
OpSelectionMerge %323 None
OpBranchConditional %321 %323 %322
%322 = OpLabel
%324 = OpLoad %float %_242_38_6_snappedX
%325 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%326 = OpLoad %v4float %325
%327 = OpCompositeExtract %float %326 2
%328 = OpFOrdGreaterThan %bool %324 %327
OpBranch %323
%323 = OpLabel
%329 = OpPhi %bool %true %259 %328 %322
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%333 = OpLoad %v4float %332
OpStore %_241_37_5_textureColor %333
OpBranch %331
%331 = OpLabel
%334 = OpLoad %v4float %_241_37_5_textureColor
OpStore %_236_32_0_TextureEffect_Stage1_c0_c0_c0 %334
%335 = OpLoad %v4float %_236_32_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_235_31_MatrixEffect_Stage1_c0_c0 %335
%336 = OpLoad %v4float %_208_output
%337 = OpLoad %v4float %_235_31_MatrixEffect_Stage1_c0_c0
%338 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_0
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 3
%341 = OpVectorTimesScalar %v4float %337 %340
%342 = OpFAdd %v4float %336 %341
OpStore %_208_output %342
%343 = OpLoad %v2float %_209_coord
%344 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%345 = OpLoad %v2float %344
%346 = OpFAdd %v2float %343 %345
OpStore %_209_coord %346
%347 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %347
%351 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%352 = OpLoad %mat3v3float %351
%353 = OpLoad %v2float %_210_coordSampled
%354 = OpCompositeExtract %float %353 0
%355 = OpCompositeExtract %float %353 1
%356 = OpCompositeConstruct %v3float %354 %355 %float_1
%357 = OpMatrixTimesVector %v3float %352 %356
%358 = OpVectorShuffle %v2float %357 %357 0 1
OpStore %_245_41_1_coords %358
%360 = OpLoad %v2float %_245_41_1_coords
OpStore %_246_42_2_inCoord %360
%361 = OpLoad %v2float %_246_42_2_inCoord
%362 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%363 = OpLoad %v4float %362
%364 = OpVectorShuffle %v2float %363 %363 0 1
%365 = OpFMul %v2float %361 %364
OpStore %_246_42_2_inCoord %365
%367 = OpLoad %v2float %_246_42_2_inCoord
%368 = OpCompositeExtract %float %367 0
%369 = OpAccessChain %_ptr_Function_float %_247_43_3_subsetCoord %int_0
OpStore %369 %368
%370 = OpLoad %v2float %_246_42_2_inCoord
%371 = OpCompositeExtract %float %370 1
%372 = OpAccessChain %_ptr_Function_float %_247_43_3_subsetCoord %int_1
OpStore %372 %371
%374 = OpLoad %v2float %_247_43_3_subsetCoord
OpStore %_248_44_4_clampedCoord %374
%377 = OpLoad %21 %uTextureSampler_0_Stage1
%378 = OpLoad %v2float %_248_44_4_clampedCoord
%379 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%380 = OpLoad %v4float %379
%381 = OpVectorShuffle %v2float %380 %380 2 3
%382 = OpFMul %v2float %378 %381
%376 = OpImageSampleImplicitLod %v4float %377 %382
OpStore %_249_45_5_textureColor %376
%385 = OpLoad %v2float %_246_42_2_inCoord
%386 = OpCompositeExtract %float %385 0
%387 = OpFAdd %float %386 %float_0_00100000005
%384 = OpExtInst %float %1 Floor %387
%388 = OpFAdd %float %384 %float_0_5
OpStore %_250_46_6_snappedX %388
%389 = OpLoad %float %_250_46_6_snappedX
%390 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%391 = OpLoad %v4float %390
%392 = OpCompositeExtract %float %391 0
%393 = OpFOrdLessThan %bool %389 %392
OpSelectionMerge %395 None
OpBranchConditional %393 %395 %394
%394 = OpLabel
%396 = OpLoad %float %_250_46_6_snappedX
%397 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%398 = OpLoad %v4float %397
%399 = OpCompositeExtract %float %398 2
%400 = OpFOrdGreaterThan %bool %396 %399
OpBranch %395
%395 = OpLabel
%401 = OpPhi %bool %true %331 %400 %394
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%405 = OpLoad %v4float %404
OpStore %_249_45_5_textureColor %405
OpBranch %403
%403 = OpLabel
%406 = OpLoad %v4float %_249_45_5_textureColor
OpStore %_244_40_0_TextureEffect_Stage1_c0_c0_c0 %406
%407 = OpLoad %v4float %_244_40_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_243_39_MatrixEffect_Stage1_c0_c0 %407
%408 = OpLoad %v4float %_208_output
%409 = OpLoad %v4float %_243_39_MatrixEffect_Stage1_c0_c0
%410 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_1
%411 = OpLoad %v4float %410
%412 = OpCompositeExtract %float %411 0
%413 = OpVectorTimesScalar %v4float %409 %412
%414 = OpFAdd %v4float %408 %413
OpStore %_208_output %414
%415 = OpLoad %v2float %_209_coord
%416 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%417 = OpLoad %v2float %416
%418 = OpFAdd %v2float %415 %417
OpStore %_209_coord %418
%419 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %419
%423 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%424 = OpLoad %mat3v3float %423
%425 = OpLoad %v2float %_210_coordSampled
%426 = OpCompositeExtract %float %425 0
%427 = OpCompositeExtract %float %425 1
%428 = OpCompositeConstruct %v3float %426 %427 %float_1
%429 = OpMatrixTimesVector %v3float %424 %428
%430 = OpVectorShuffle %v2float %429 %429 0 1
OpStore %_253_49_1_coords %430
%432 = OpLoad %v2float %_253_49_1_coords
OpStore %_254_50_2_inCoord %432
%433 = OpLoad %v2float %_254_50_2_inCoord
%434 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%435 = OpLoad %v4float %434
%436 = OpVectorShuffle %v2float %435 %435 0 1
%437 = OpFMul %v2float %433 %436
OpStore %_254_50_2_inCoord %437
%439 = OpLoad %v2float %_254_50_2_inCoord
%440 = OpCompositeExtract %float %439 0
%441 = OpAccessChain %_ptr_Function_float %_255_51_3_subsetCoord %int_0
OpStore %441 %440
%442 = OpLoad %v2float %_254_50_2_inCoord
%443 = OpCompositeExtract %float %442 1
%444 = OpAccessChain %_ptr_Function_float %_255_51_3_subsetCoord %int_1
OpStore %444 %443
%446 = OpLoad %v2float %_255_51_3_subsetCoord
OpStore %_256_52_4_clampedCoord %446
%449 = OpLoad %21 %uTextureSampler_0_Stage1
%450 = OpLoad %v2float %_256_52_4_clampedCoord
%451 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%452 = OpLoad %v4float %451
%453 = OpVectorShuffle %v2float %452 %452 2 3
%454 = OpFMul %v2float %450 %453
%448 = OpImageSampleImplicitLod %v4float %449 %454
OpStore %_257_53_5_textureColor %448
%457 = OpLoad %v2float %_254_50_2_inCoord
%458 = OpCompositeExtract %float %457 0
%459 = OpFAdd %float %458 %float_0_00100000005
%456 = OpExtInst %float %1 Floor %459
%460 = OpFAdd %float %456 %float_0_5
OpStore %_258_54_6_snappedX %460
%461 = OpLoad %float %_258_54_6_snappedX
%462 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%463 = OpLoad %v4float %462
%464 = OpCompositeExtract %float %463 0
%465 = OpFOrdLessThan %bool %461 %464
OpSelectionMerge %467 None
OpBranchConditional %465 %467 %466
%466 = OpLabel
%468 = OpLoad %float %_258_54_6_snappedX
%469 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%470 = OpLoad %v4float %469
%471 = OpCompositeExtract %float %470 2
%472 = OpFOrdGreaterThan %bool %468 %471
OpBranch %467
%467 = OpLabel
%473 = OpPhi %bool %true %403 %472 %466
OpSelectionMerge %475 None
OpBranchConditional %473 %474 %475
%474 = OpLabel
%476 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%477 = OpLoad %v4float %476
OpStore %_257_53_5_textureColor %477
OpBranch %475
%475 = OpLabel
%478 = OpLoad %v4float %_257_53_5_textureColor
OpStore %_252_48_0_TextureEffect_Stage1_c0_c0_c0 %478
%479 = OpLoad %v4float %_252_48_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_251_47_MatrixEffect_Stage1_c0_c0 %479
%480 = OpLoad %v4float %_208_output
%481 = OpLoad %v4float %_251_47_MatrixEffect_Stage1_c0_c0
%482 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_1
%483 = OpLoad %v4float %482
%484 = OpCompositeExtract %float %483 1
%485 = OpVectorTimesScalar %v4float %481 %484
%486 = OpFAdd %v4float %480 %485
OpStore %_208_output %486
%487 = OpLoad %v2float %_209_coord
%488 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%489 = OpLoad %v2float %488
%490 = OpFAdd %v2float %487 %489
OpStore %_209_coord %490
%491 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %491
%495 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%496 = OpLoad %mat3v3float %495
%497 = OpLoad %v2float %_210_coordSampled
%498 = OpCompositeExtract %float %497 0
%499 = OpCompositeExtract %float %497 1
%500 = OpCompositeConstruct %v3float %498 %499 %float_1
%501 = OpMatrixTimesVector %v3float %496 %500
%502 = OpVectorShuffle %v2float %501 %501 0 1
OpStore %_261_57_1_coords %502
%504 = OpLoad %v2float %_261_57_1_coords
OpStore %_262_58_2_inCoord %504
%505 = OpLoad %v2float %_262_58_2_inCoord
%506 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%507 = OpLoad %v4float %506
%508 = OpVectorShuffle %v2float %507 %507 0 1
%509 = OpFMul %v2float %505 %508
OpStore %_262_58_2_inCoord %509
%511 = OpLoad %v2float %_262_58_2_inCoord
%512 = OpCompositeExtract %float %511 0
%513 = OpAccessChain %_ptr_Function_float %_263_59_3_subsetCoord %int_0
OpStore %513 %512
%514 = OpLoad %v2float %_262_58_2_inCoord
%515 = OpCompositeExtract %float %514 1
%516 = OpAccessChain %_ptr_Function_float %_263_59_3_subsetCoord %int_1
OpStore %516 %515
%518 = OpLoad %v2float %_263_59_3_subsetCoord
OpStore %_264_60_4_clampedCoord %518
%521 = OpLoad %21 %uTextureSampler_0_Stage1
%522 = OpLoad %v2float %_264_60_4_clampedCoord
%523 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%524 = OpLoad %v4float %523
%525 = OpVectorShuffle %v2float %524 %524 2 3
%526 = OpFMul %v2float %522 %525
%520 = OpImageSampleImplicitLod %v4float %521 %526
OpStore %_265_61_5_textureColor %520
%529 = OpLoad %v2float %_262_58_2_inCoord
%530 = OpCompositeExtract %float %529 0
%531 = OpFAdd %float %530 %float_0_00100000005
%528 = OpExtInst %float %1 Floor %531
%532 = OpFAdd %float %528 %float_0_5
OpStore %_266_62_6_snappedX %532
%533 = OpLoad %float %_266_62_6_snappedX
%534 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%535 = OpLoad %v4float %534
%536 = OpCompositeExtract %float %535 0
%537 = OpFOrdLessThan %bool %533 %536
OpSelectionMerge %539 None
OpBranchConditional %537 %539 %538
%538 = OpLabel
%540 = OpLoad %float %_266_62_6_snappedX
%541 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%542 = OpLoad %v4float %541
%543 = OpCompositeExtract %float %542 2
%544 = OpFOrdGreaterThan %bool %540 %543
OpBranch %539
%539 = OpLabel
%545 = OpPhi %bool %true %475 %544 %538
OpSelectionMerge %547 None
OpBranchConditional %545 %546 %547
%546 = OpLabel
%548 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%549 = OpLoad %v4float %548
OpStore %_265_61_5_textureColor %549
OpBranch %547
%547 = OpLabel
%550 = OpLoad %v4float %_265_61_5_textureColor
OpStore %_260_56_0_TextureEffect_Stage1_c0_c0_c0 %550
%551 = OpLoad %v4float %_260_56_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_259_55_MatrixEffect_Stage1_c0_c0 %551
%552 = OpLoad %v4float %_208_output
%553 = OpLoad %v4float %_259_55_MatrixEffect_Stage1_c0_c0
%554 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_1
%555 = OpLoad %v4float %554
%556 = OpCompositeExtract %float %555 2
%557 = OpVectorTimesScalar %v4float %553 %556
%558 = OpFAdd %v4float %552 %557
OpStore %_208_output %558
%559 = OpLoad %v2float %_209_coord
%560 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%561 = OpLoad %v2float %560
%562 = OpFAdd %v2float %559 %561
OpStore %_209_coord %562
%563 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %563
%567 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%568 = OpLoad %mat3v3float %567
%569 = OpLoad %v2float %_210_coordSampled
%570 = OpCompositeExtract %float %569 0
%571 = OpCompositeExtract %float %569 1
%572 = OpCompositeConstruct %v3float %570 %571 %float_1
%573 = OpMatrixTimesVector %v3float %568 %572
%574 = OpVectorShuffle %v2float %573 %573 0 1
OpStore %_269_65_1_coords %574
%576 = OpLoad %v2float %_269_65_1_coords
OpStore %_270_66_2_inCoord %576
%577 = OpLoad %v2float %_270_66_2_inCoord
%578 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%579 = OpLoad %v4float %578
%580 = OpVectorShuffle %v2float %579 %579 0 1
%581 = OpFMul %v2float %577 %580
OpStore %_270_66_2_inCoord %581
%583 = OpLoad %v2float %_270_66_2_inCoord
%584 = OpCompositeExtract %float %583 0
%585 = OpAccessChain %_ptr_Function_float %_271_67_3_subsetCoord %int_0
OpStore %585 %584
%586 = OpLoad %v2float %_270_66_2_inCoord
%587 = OpCompositeExtract %float %586 1
%588 = OpAccessChain %_ptr_Function_float %_271_67_3_subsetCoord %int_1
OpStore %588 %587
%590 = OpLoad %v2float %_271_67_3_subsetCoord
OpStore %_272_68_4_clampedCoord %590
%593 = OpLoad %21 %uTextureSampler_0_Stage1
%594 = OpLoad %v2float %_272_68_4_clampedCoord
%595 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%596 = OpLoad %v4float %595
%597 = OpVectorShuffle %v2float %596 %596 2 3
%598 = OpFMul %v2float %594 %597
%592 = OpImageSampleImplicitLod %v4float %593 %598
OpStore %_273_69_5_textureColor %592
%601 = OpLoad %v2float %_270_66_2_inCoord
%602 = OpCompositeExtract %float %601 0
%603 = OpFAdd %float %602 %float_0_00100000005
%600 = OpExtInst %float %1 Floor %603
%604 = OpFAdd %float %600 %float_0_5
OpStore %_274_70_6_snappedX %604
%605 = OpLoad %float %_274_70_6_snappedX
%606 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%607 = OpLoad %v4float %606
%608 = OpCompositeExtract %float %607 0
%609 = OpFOrdLessThan %bool %605 %608
OpSelectionMerge %611 None
OpBranchConditional %609 %611 %610
%610 = OpLabel
%612 = OpLoad %float %_274_70_6_snappedX
%613 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%614 = OpLoad %v4float %613
%615 = OpCompositeExtract %float %614 2
%616 = OpFOrdGreaterThan %bool %612 %615
OpBranch %611
%611 = OpLabel
%617 = OpPhi %bool %true %547 %616 %610
OpSelectionMerge %619 None
OpBranchConditional %617 %618 %619
%618 = OpLabel
%620 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%621 = OpLoad %v4float %620
OpStore %_273_69_5_textureColor %621
OpBranch %619
%619 = OpLabel
%622 = OpLoad %v4float %_273_69_5_textureColor
OpStore %_268_64_0_TextureEffect_Stage1_c0_c0_c0 %622
%623 = OpLoad %v4float %_268_64_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_267_63_MatrixEffect_Stage1_c0_c0 %623
%624 = OpLoad %v4float %_208_output
%625 = OpLoad %v4float %_267_63_MatrixEffect_Stage1_c0_c0
%626 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_1
%627 = OpLoad %v4float %626
%628 = OpCompositeExtract %float %627 3
%629 = OpVectorTimesScalar %v4float %625 %628
%630 = OpFAdd %v4float %624 %629
OpStore %_208_output %630
%631 = OpLoad %v2float %_209_coord
%632 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%633 = OpLoad %v2float %632
%634 = OpFAdd %v2float %631 %633
OpStore %_209_coord %634
%635 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %635
%639 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%640 = OpLoad %mat3v3float %639
%641 = OpLoad %v2float %_210_coordSampled
%642 = OpCompositeExtract %float %641 0
%643 = OpCompositeExtract %float %641 1
%644 = OpCompositeConstruct %v3float %642 %643 %float_1
%645 = OpMatrixTimesVector %v3float %640 %644
%646 = OpVectorShuffle %v2float %645 %645 0 1
OpStore %_277_73_1_coords %646
%648 = OpLoad %v2float %_277_73_1_coords
OpStore %_278_74_2_inCoord %648
%649 = OpLoad %v2float %_278_74_2_inCoord
%650 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%651 = OpLoad %v4float %650
%652 = OpVectorShuffle %v2float %651 %651 0 1
%653 = OpFMul %v2float %649 %652
OpStore %_278_74_2_inCoord %653
%655 = OpLoad %v2float %_278_74_2_inCoord
%656 = OpCompositeExtract %float %655 0
%657 = OpAccessChain %_ptr_Function_float %_279_75_3_subsetCoord %int_0
OpStore %657 %656
%658 = OpLoad %v2float %_278_74_2_inCoord
%659 = OpCompositeExtract %float %658 1
%660 = OpAccessChain %_ptr_Function_float %_279_75_3_subsetCoord %int_1
OpStore %660 %659
%662 = OpLoad %v2float %_279_75_3_subsetCoord
OpStore %_280_76_4_clampedCoord %662
%665 = OpLoad %21 %uTextureSampler_0_Stage1
%666 = OpLoad %v2float %_280_76_4_clampedCoord
%667 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%668 = OpLoad %v4float %667
%669 = OpVectorShuffle %v2float %668 %668 2 3
%670 = OpFMul %v2float %666 %669
%664 = OpImageSampleImplicitLod %v4float %665 %670
OpStore %_281_77_5_textureColor %664
%673 = OpLoad %v2float %_278_74_2_inCoord
%674 = OpCompositeExtract %float %673 0
%675 = OpFAdd %float %674 %float_0_00100000005
%672 = OpExtInst %float %1 Floor %675
%676 = OpFAdd %float %672 %float_0_5
OpStore %_282_78_6_snappedX %676
%677 = OpLoad %float %_282_78_6_snappedX
%678 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%679 = OpLoad %v4float %678
%680 = OpCompositeExtract %float %679 0
%681 = OpFOrdLessThan %bool %677 %680
OpSelectionMerge %683 None
OpBranchConditional %681 %683 %682
%682 = OpLabel
%684 = OpLoad %float %_282_78_6_snappedX
%685 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%686 = OpLoad %v4float %685
%687 = OpCompositeExtract %float %686 2
%688 = OpFOrdGreaterThan %bool %684 %687
OpBranch %683
%683 = OpLabel
%689 = OpPhi %bool %true %619 %688 %682
OpSelectionMerge %691 None
OpBranchConditional %689 %690 %691
%690 = OpLabel
%692 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%693 = OpLoad %v4float %692
OpStore %_281_77_5_textureColor %693
OpBranch %691
%691 = OpLabel
%694 = OpLoad %v4float %_281_77_5_textureColor
OpStore %_276_72_0_TextureEffect_Stage1_c0_c0_c0 %694
%695 = OpLoad %v4float %_276_72_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_275_71_MatrixEffect_Stage1_c0_c0 %695
%696 = OpLoad %v4float %_208_output
%697 = OpLoad %v4float %_275_71_MatrixEffect_Stage1_c0_c0
%698 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_2
%699 = OpLoad %v4float %698
%700 = OpCompositeExtract %float %699 0
%701 = OpVectorTimesScalar %v4float %697 %700
%702 = OpFAdd %v4float %696 %701
OpStore %_208_output %702
%703 = OpLoad %v2float %_209_coord
%704 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%705 = OpLoad %v2float %704
%706 = OpFAdd %v2float %703 %705
OpStore %_209_coord %706
%707 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %707
%711 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%712 = OpLoad %mat3v3float %711
%713 = OpLoad %v2float %_210_coordSampled
%714 = OpCompositeExtract %float %713 0
%715 = OpCompositeExtract %float %713 1
%716 = OpCompositeConstruct %v3float %714 %715 %float_1
%717 = OpMatrixTimesVector %v3float %712 %716
%718 = OpVectorShuffle %v2float %717 %717 0 1
OpStore %_285_81_1_coords %718
%720 = OpLoad %v2float %_285_81_1_coords
OpStore %_286_82_2_inCoord %720
%721 = OpLoad %v2float %_286_82_2_inCoord
%722 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%723 = OpLoad %v4float %722
%724 = OpVectorShuffle %v2float %723 %723 0 1
%725 = OpFMul %v2float %721 %724
OpStore %_286_82_2_inCoord %725
%727 = OpLoad %v2float %_286_82_2_inCoord
%728 = OpCompositeExtract %float %727 0
%729 = OpAccessChain %_ptr_Function_float %_287_83_3_subsetCoord %int_0
OpStore %729 %728
%730 = OpLoad %v2float %_286_82_2_inCoord
%731 = OpCompositeExtract %float %730 1
%732 = OpAccessChain %_ptr_Function_float %_287_83_3_subsetCoord %int_1
OpStore %732 %731
%734 = OpLoad %v2float %_287_83_3_subsetCoord
OpStore %_288_84_4_clampedCoord %734
%737 = OpLoad %21 %uTextureSampler_0_Stage1
%738 = OpLoad %v2float %_288_84_4_clampedCoord
%739 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%740 = OpLoad %v4float %739
%741 = OpVectorShuffle %v2float %740 %740 2 3
%742 = OpFMul %v2float %738 %741
%736 = OpImageSampleImplicitLod %v4float %737 %742
OpStore %_289_85_5_textureColor %736
%745 = OpLoad %v2float %_286_82_2_inCoord
%746 = OpCompositeExtract %float %745 0
%747 = OpFAdd %float %746 %float_0_00100000005
%744 = OpExtInst %float %1 Floor %747
%748 = OpFAdd %float %744 %float_0_5
OpStore %_290_86_6_snappedX %748
%749 = OpLoad %float %_290_86_6_snappedX
%750 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%751 = OpLoad %v4float %750
%752 = OpCompositeExtract %float %751 0
%753 = OpFOrdLessThan %bool %749 %752
OpSelectionMerge %755 None
OpBranchConditional %753 %755 %754
%754 = OpLabel
%756 = OpLoad %float %_290_86_6_snappedX
%757 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%758 = OpLoad %v4float %757
%759 = OpCompositeExtract %float %758 2
%760 = OpFOrdGreaterThan %bool %756 %759
OpBranch %755
%755 = OpLabel
%761 = OpPhi %bool %true %691 %760 %754
OpSelectionMerge %763 None
OpBranchConditional %761 %762 %763
%762 = OpLabel
%764 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%765 = OpLoad %v4float %764
OpStore %_289_85_5_textureColor %765
OpBranch %763
%763 = OpLabel
%766 = OpLoad %v4float %_289_85_5_textureColor
OpStore %_284_80_0_TextureEffect_Stage1_c0_c0_c0 %766
%767 = OpLoad %v4float %_284_80_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_283_79_MatrixEffect_Stage1_c0_c0 %767
%768 = OpLoad %v4float %_208_output
%769 = OpLoad %v4float %_283_79_MatrixEffect_Stage1_c0_c0
%770 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_2
%771 = OpLoad %v4float %770
%772 = OpCompositeExtract %float %771 1
%773 = OpVectorTimesScalar %v4float %769 %772
%774 = OpFAdd %v4float %768 %773
OpStore %_208_output %774
%775 = OpLoad %v2float %_209_coord
%776 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%777 = OpLoad %v2float %776
%778 = OpFAdd %v2float %775 %777
OpStore %_209_coord %778
%779 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %779
%783 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%784 = OpLoad %mat3v3float %783
%785 = OpLoad %v2float %_210_coordSampled
%786 = OpCompositeExtract %float %785 0
%787 = OpCompositeExtract %float %785 1
%788 = OpCompositeConstruct %v3float %786 %787 %float_1
%789 = OpMatrixTimesVector %v3float %784 %788
%790 = OpVectorShuffle %v2float %789 %789 0 1
OpStore %_293_89_1_coords %790
%792 = OpLoad %v2float %_293_89_1_coords
OpStore %_294_90_2_inCoord %792
%793 = OpLoad %v2float %_294_90_2_inCoord
%794 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%795 = OpLoad %v4float %794
%796 = OpVectorShuffle %v2float %795 %795 0 1
%797 = OpFMul %v2float %793 %796
OpStore %_294_90_2_inCoord %797
%799 = OpLoad %v2float %_294_90_2_inCoord
%800 = OpCompositeExtract %float %799 0
%801 = OpAccessChain %_ptr_Function_float %_295_91_3_subsetCoord %int_0
OpStore %801 %800
%802 = OpLoad %v2float %_294_90_2_inCoord
%803 = OpCompositeExtract %float %802 1
%804 = OpAccessChain %_ptr_Function_float %_295_91_3_subsetCoord %int_1
OpStore %804 %803
%806 = OpLoad %v2float %_295_91_3_subsetCoord
OpStore %_296_92_4_clampedCoord %806
%809 = OpLoad %21 %uTextureSampler_0_Stage1
%810 = OpLoad %v2float %_296_92_4_clampedCoord
%811 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%812 = OpLoad %v4float %811
%813 = OpVectorShuffle %v2float %812 %812 2 3
%814 = OpFMul %v2float %810 %813
%808 = OpImageSampleImplicitLod %v4float %809 %814
OpStore %_297_93_5_textureColor %808
%817 = OpLoad %v2float %_294_90_2_inCoord
%818 = OpCompositeExtract %float %817 0
%819 = OpFAdd %float %818 %float_0_00100000005
%816 = OpExtInst %float %1 Floor %819
%820 = OpFAdd %float %816 %float_0_5
OpStore %_298_94_6_snappedX %820
%821 = OpLoad %float %_298_94_6_snappedX
%822 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%823 = OpLoad %v4float %822
%824 = OpCompositeExtract %float %823 0
%825 = OpFOrdLessThan %bool %821 %824
OpSelectionMerge %827 None
OpBranchConditional %825 %827 %826
%826 = OpLabel
%828 = OpLoad %float %_298_94_6_snappedX
%829 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%830 = OpLoad %v4float %829
%831 = OpCompositeExtract %float %830 2
%832 = OpFOrdGreaterThan %bool %828 %831
OpBranch %827
%827 = OpLabel
%833 = OpPhi %bool %true %763 %832 %826
OpSelectionMerge %835 None
OpBranchConditional %833 %834 %835
%834 = OpLabel
%836 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%837 = OpLoad %v4float %836
OpStore %_297_93_5_textureColor %837
OpBranch %835
%835 = OpLabel
%838 = OpLoad %v4float %_297_93_5_textureColor
OpStore %_292_88_0_TextureEffect_Stage1_c0_c0_c0 %838
%839 = OpLoad %v4float %_292_88_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_291_87_MatrixEffect_Stage1_c0_c0 %839
%840 = OpLoad %v4float %_208_output
%841 = OpLoad %v4float %_291_87_MatrixEffect_Stage1_c0_c0
%842 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_2
%843 = OpLoad %v4float %842
%844 = OpCompositeExtract %float %843 2
%845 = OpVectorTimesScalar %v4float %841 %844
%846 = OpFAdd %v4float %840 %845
OpStore %_208_output %846
%847 = OpLoad %v2float %_209_coord
%848 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%849 = OpLoad %v2float %848
%850 = OpFAdd %v2float %847 %849
OpStore %_209_coord %850
%851 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %851
%855 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%856 = OpLoad %mat3v3float %855
%857 = OpLoad %v2float %_210_coordSampled
%858 = OpCompositeExtract %float %857 0
%859 = OpCompositeExtract %float %857 1
%860 = OpCompositeConstruct %v3float %858 %859 %float_1
%861 = OpMatrixTimesVector %v3float %856 %860
%862 = OpVectorShuffle %v2float %861 %861 0 1
OpStore %_301_97_1_coords %862
%864 = OpLoad %v2float %_301_97_1_coords
OpStore %_302_98_2_inCoord %864
%865 = OpLoad %v2float %_302_98_2_inCoord
%866 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%867 = OpLoad %v4float %866
%868 = OpVectorShuffle %v2float %867 %867 0 1
%869 = OpFMul %v2float %865 %868
OpStore %_302_98_2_inCoord %869
%871 = OpLoad %v2float %_302_98_2_inCoord
%872 = OpCompositeExtract %float %871 0
%873 = OpAccessChain %_ptr_Function_float %_303_99_3_subsetCoord %int_0
OpStore %873 %872
%874 = OpLoad %v2float %_302_98_2_inCoord
%875 = OpCompositeExtract %float %874 1
%876 = OpAccessChain %_ptr_Function_float %_303_99_3_subsetCoord %int_1
OpStore %876 %875
%878 = OpLoad %v2float %_303_99_3_subsetCoord
OpStore %_304_100_4_clampedCoord %878
%881 = OpLoad %21 %uTextureSampler_0_Stage1
%882 = OpLoad %v2float %_304_100_4_clampedCoord
%883 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%884 = OpLoad %v4float %883
%885 = OpVectorShuffle %v2float %884 %884 2 3
%886 = OpFMul %v2float %882 %885
%880 = OpImageSampleImplicitLod %v4float %881 %886
OpStore %_305_101_5_textureColor %880
%889 = OpLoad %v2float %_302_98_2_inCoord
%890 = OpCompositeExtract %float %889 0
%891 = OpFAdd %float %890 %float_0_00100000005
%888 = OpExtInst %float %1 Floor %891
%892 = OpFAdd %float %888 %float_0_5
OpStore %_306_102_6_snappedX %892
%893 = OpLoad %float %_306_102_6_snappedX
%894 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%895 = OpLoad %v4float %894
%896 = OpCompositeExtract %float %895 0
%897 = OpFOrdLessThan %bool %893 %896
OpSelectionMerge %899 None
OpBranchConditional %897 %899 %898
%898 = OpLabel
%900 = OpLoad %float %_306_102_6_snappedX
%901 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%902 = OpLoad %v4float %901
%903 = OpCompositeExtract %float %902 2
%904 = OpFOrdGreaterThan %bool %900 %903
OpBranch %899
%899 = OpLabel
%905 = OpPhi %bool %true %835 %904 %898
OpSelectionMerge %907 None
OpBranchConditional %905 %906 %907
%906 = OpLabel
%908 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%909 = OpLoad %v4float %908
OpStore %_305_101_5_textureColor %909
OpBranch %907
%907 = OpLabel
%910 = OpLoad %v4float %_305_101_5_textureColor
OpStore %_300_96_0_TextureEffect_Stage1_c0_c0_c0 %910
%911 = OpLoad %v4float %_300_96_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_299_95_MatrixEffect_Stage1_c0_c0 %911
%912 = OpLoad %v4float %_208_output
%913 = OpLoad %v4float %_299_95_MatrixEffect_Stage1_c0_c0
%914 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_2
%915 = OpLoad %v4float %914
%916 = OpCompositeExtract %float %915 3
%917 = OpVectorTimesScalar %v4float %913 %916
%918 = OpFAdd %v4float %912 %917
OpStore %_208_output %918
%919 = OpLoad %v2float %_209_coord
%920 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%921 = OpLoad %v2float %920
%922 = OpFAdd %v2float %919 %921
OpStore %_209_coord %922
%923 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %923
%927 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%928 = OpLoad %mat3v3float %927
%929 = OpLoad %v2float %_210_coordSampled
%930 = OpCompositeExtract %float %929 0
%931 = OpCompositeExtract %float %929 1
%932 = OpCompositeConstruct %v3float %930 %931 %float_1
%933 = OpMatrixTimesVector %v3float %928 %932
%934 = OpVectorShuffle %v2float %933 %933 0 1
OpStore %_309_105_1_coords %934
%936 = OpLoad %v2float %_309_105_1_coords
OpStore %_310_106_2_inCoord %936
%937 = OpLoad %v2float %_310_106_2_inCoord
%938 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%939 = OpLoad %v4float %938
%940 = OpVectorShuffle %v2float %939 %939 0 1
%941 = OpFMul %v2float %937 %940
OpStore %_310_106_2_inCoord %941
%943 = OpLoad %v2float %_310_106_2_inCoord
%944 = OpCompositeExtract %float %943 0
%945 = OpAccessChain %_ptr_Function_float %_311_107_3_subsetCoord %int_0
OpStore %945 %944
%946 = OpLoad %v2float %_310_106_2_inCoord
%947 = OpCompositeExtract %float %946 1
%948 = OpAccessChain %_ptr_Function_float %_311_107_3_subsetCoord %int_1
OpStore %948 %947
%950 = OpLoad %v2float %_311_107_3_subsetCoord
OpStore %_312_108_4_clampedCoord %950
%953 = OpLoad %21 %uTextureSampler_0_Stage1
%954 = OpLoad %v2float %_312_108_4_clampedCoord
%955 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%956 = OpLoad %v4float %955
%957 = OpVectorShuffle %v2float %956 %956 2 3
%958 = OpFMul %v2float %954 %957
%952 = OpImageSampleImplicitLod %v4float %953 %958
OpStore %_313_109_5_textureColor %952
%961 = OpLoad %v2float %_310_106_2_inCoord
%962 = OpCompositeExtract %float %961 0
%963 = OpFAdd %float %962 %float_0_00100000005
%960 = OpExtInst %float %1 Floor %963
%964 = OpFAdd %float %960 %float_0_5
OpStore %_314_110_6_snappedX %964
%965 = OpLoad %float %_314_110_6_snappedX
%966 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%967 = OpLoad %v4float %966
%968 = OpCompositeExtract %float %967 0
%969 = OpFOrdLessThan %bool %965 %968
OpSelectionMerge %971 None
OpBranchConditional %969 %971 %970
%970 = OpLabel
%972 = OpLoad %float %_314_110_6_snappedX
%973 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%974 = OpLoad %v4float %973
%975 = OpCompositeExtract %float %974 2
%976 = OpFOrdGreaterThan %bool %972 %975
OpBranch %971
%971 = OpLabel
%977 = OpPhi %bool %true %907 %976 %970
OpSelectionMerge %979 None
OpBranchConditional %977 %978 %979
%978 = OpLabel
%980 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%981 = OpLoad %v4float %980
OpStore %_313_109_5_textureColor %981
OpBranch %979
%979 = OpLabel
%982 = OpLoad %v4float %_313_109_5_textureColor
OpStore %_308_104_0_TextureEffect_Stage1_c0_c0_c0 %982
%983 = OpLoad %v4float %_308_104_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_307_103_MatrixEffect_Stage1_c0_c0 %983
%984 = OpLoad %v4float %_208_output
%985 = OpLoad %v4float %_307_103_MatrixEffect_Stage1_c0_c0
%986 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_3
%987 = OpLoad %v4float %986
%988 = OpCompositeExtract %float %987 0
%989 = OpVectorTimesScalar %v4float %985 %988
%990 = OpFAdd %v4float %984 %989
OpStore %_208_output %990
%991 = OpLoad %v2float %_209_coord
%992 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%993 = OpLoad %v2float %992
%994 = OpFAdd %v2float %991 %993
OpStore %_209_coord %994
%995 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %995
%999 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1000 = OpLoad %mat3v3float %999
%1001 = OpLoad %v2float %_210_coordSampled
%1002 = OpCompositeExtract %float %1001 0
%1003 = OpCompositeExtract %float %1001 1
%1004 = OpCompositeConstruct %v3float %1002 %1003 %float_1
%1005 = OpMatrixTimesVector %v3float %1000 %1004
%1006 = OpVectorShuffle %v2float %1005 %1005 0 1
OpStore %_317_113_1_coords %1006
%1008 = OpLoad %v2float %_317_113_1_coords
OpStore %_318_114_2_inCoord %1008
%1009 = OpLoad %v2float %_318_114_2_inCoord
%1010 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1011 = OpLoad %v4float %1010
%1012 = OpVectorShuffle %v2float %1011 %1011 0 1
%1013 = OpFMul %v2float %1009 %1012
OpStore %_318_114_2_inCoord %1013
%1015 = OpLoad %v2float %_318_114_2_inCoord
%1016 = OpCompositeExtract %float %1015 0
%1017 = OpAccessChain %_ptr_Function_float %_319_115_3_subsetCoord %int_0
OpStore %1017 %1016
%1018 = OpLoad %v2float %_318_114_2_inCoord
%1019 = OpCompositeExtract %float %1018 1
%1020 = OpAccessChain %_ptr_Function_float %_319_115_3_subsetCoord %int_1
OpStore %1020 %1019
%1022 = OpLoad %v2float %_319_115_3_subsetCoord
OpStore %_320_116_4_clampedCoord %1022
%1025 = OpLoad %21 %uTextureSampler_0_Stage1
%1026 = OpLoad %v2float %_320_116_4_clampedCoord
%1027 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1028 = OpLoad %v4float %1027
%1029 = OpVectorShuffle %v2float %1028 %1028 2 3
%1030 = OpFMul %v2float %1026 %1029
%1024 = OpImageSampleImplicitLod %v4float %1025 %1030
OpStore %_321_117_5_textureColor %1024
%1033 = OpLoad %v2float %_318_114_2_inCoord
%1034 = OpCompositeExtract %float %1033 0
%1035 = OpFAdd %float %1034 %float_0_00100000005
%1032 = OpExtInst %float %1 Floor %1035
%1036 = OpFAdd %float %1032 %float_0_5
OpStore %_322_118_6_snappedX %1036
%1037 = OpLoad %float %_322_118_6_snappedX
%1038 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1039 = OpLoad %v4float %1038
%1040 = OpCompositeExtract %float %1039 0
%1041 = OpFOrdLessThan %bool %1037 %1040
OpSelectionMerge %1043 None
OpBranchConditional %1041 %1043 %1042
%1042 = OpLabel
%1044 = OpLoad %float %_322_118_6_snappedX
%1045 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1046 = OpLoad %v4float %1045
%1047 = OpCompositeExtract %float %1046 2
%1048 = OpFOrdGreaterThan %bool %1044 %1047
OpBranch %1043
%1043 = OpLabel
%1049 = OpPhi %bool %true %979 %1048 %1042
OpSelectionMerge %1051 None
OpBranchConditional %1049 %1050 %1051
%1050 = OpLabel
%1052 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1053 = OpLoad %v4float %1052
OpStore %_321_117_5_textureColor %1053
OpBranch %1051
%1051 = OpLabel
%1054 = OpLoad %v4float %_321_117_5_textureColor
OpStore %_316_112_0_TextureEffect_Stage1_c0_c0_c0 %1054
%1055 = OpLoad %v4float %_316_112_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_315_111_MatrixEffect_Stage1_c0_c0 %1055
%1056 = OpLoad %v4float %_208_output
%1057 = OpLoad %v4float %_315_111_MatrixEffect_Stage1_c0_c0
%1058 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_3
%1059 = OpLoad %v4float %1058
%1060 = OpCompositeExtract %float %1059 1
%1061 = OpVectorTimesScalar %v4float %1057 %1060
%1062 = OpFAdd %v4float %1056 %1061
OpStore %_208_output %1062
%1063 = OpLoad %v2float %_209_coord
%1064 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1065 = OpLoad %v2float %1064
%1066 = OpFAdd %v2float %1063 %1065
OpStore %_209_coord %1066
%1067 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1067
%1071 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1072 = OpLoad %mat3v3float %1071
%1073 = OpLoad %v2float %_210_coordSampled
%1074 = OpCompositeExtract %float %1073 0
%1075 = OpCompositeExtract %float %1073 1
%1076 = OpCompositeConstruct %v3float %1074 %1075 %float_1
%1077 = OpMatrixTimesVector %v3float %1072 %1076
%1078 = OpVectorShuffle %v2float %1077 %1077 0 1
OpStore %_325_121_1_coords %1078
%1080 = OpLoad %v2float %_325_121_1_coords
OpStore %_326_122_2_inCoord %1080
%1081 = OpLoad %v2float %_326_122_2_inCoord
%1082 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1083 = OpLoad %v4float %1082
%1084 = OpVectorShuffle %v2float %1083 %1083 0 1
%1085 = OpFMul %v2float %1081 %1084
OpStore %_326_122_2_inCoord %1085
%1087 = OpLoad %v2float %_326_122_2_inCoord
%1088 = OpCompositeExtract %float %1087 0
%1089 = OpAccessChain %_ptr_Function_float %_327_123_3_subsetCoord %int_0
OpStore %1089 %1088
%1090 = OpLoad %v2float %_326_122_2_inCoord
%1091 = OpCompositeExtract %float %1090 1
%1092 = OpAccessChain %_ptr_Function_float %_327_123_3_subsetCoord %int_1
OpStore %1092 %1091
%1094 = OpLoad %v2float %_327_123_3_subsetCoord
OpStore %_328_124_4_clampedCoord %1094
%1097 = OpLoad %21 %uTextureSampler_0_Stage1
%1098 = OpLoad %v2float %_328_124_4_clampedCoord
%1099 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1100 = OpLoad %v4float %1099
%1101 = OpVectorShuffle %v2float %1100 %1100 2 3
%1102 = OpFMul %v2float %1098 %1101
%1096 = OpImageSampleImplicitLod %v4float %1097 %1102
OpStore %_329_125_5_textureColor %1096
%1105 = OpLoad %v2float %_326_122_2_inCoord
%1106 = OpCompositeExtract %float %1105 0
%1107 = OpFAdd %float %1106 %float_0_00100000005
%1104 = OpExtInst %float %1 Floor %1107
%1108 = OpFAdd %float %1104 %float_0_5
OpStore %_330_126_6_snappedX %1108
%1109 = OpLoad %float %_330_126_6_snappedX
%1110 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1111 = OpLoad %v4float %1110
%1112 = OpCompositeExtract %float %1111 0
%1113 = OpFOrdLessThan %bool %1109 %1112
OpSelectionMerge %1115 None
OpBranchConditional %1113 %1115 %1114
%1114 = OpLabel
%1116 = OpLoad %float %_330_126_6_snappedX
%1117 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1118 = OpLoad %v4float %1117
%1119 = OpCompositeExtract %float %1118 2
%1120 = OpFOrdGreaterThan %bool %1116 %1119
OpBranch %1115
%1115 = OpLabel
%1121 = OpPhi %bool %true %1051 %1120 %1114
OpSelectionMerge %1123 None
OpBranchConditional %1121 %1122 %1123
%1122 = OpLabel
%1124 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1125 = OpLoad %v4float %1124
OpStore %_329_125_5_textureColor %1125
OpBranch %1123
%1123 = OpLabel
%1126 = OpLoad %v4float %_329_125_5_textureColor
OpStore %_324_120_0_TextureEffect_Stage1_c0_c0_c0 %1126
%1127 = OpLoad %v4float %_324_120_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_323_119_MatrixEffect_Stage1_c0_c0 %1127
%1128 = OpLoad %v4float %_208_output
%1129 = OpLoad %v4float %_323_119_MatrixEffect_Stage1_c0_c0
%1130 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_3
%1131 = OpLoad %v4float %1130
%1132 = OpCompositeExtract %float %1131 2
%1133 = OpVectorTimesScalar %v4float %1129 %1132
%1134 = OpFAdd %v4float %1128 %1133
OpStore %_208_output %1134
%1135 = OpLoad %v2float %_209_coord
%1136 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1137 = OpLoad %v2float %1136
%1138 = OpFAdd %v2float %1135 %1137
OpStore %_209_coord %1138
%1139 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1139
%1143 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1144 = OpLoad %mat3v3float %1143
%1145 = OpLoad %v2float %_210_coordSampled
%1146 = OpCompositeExtract %float %1145 0
%1147 = OpCompositeExtract %float %1145 1
%1148 = OpCompositeConstruct %v3float %1146 %1147 %float_1
%1149 = OpMatrixTimesVector %v3float %1144 %1148
%1150 = OpVectorShuffle %v2float %1149 %1149 0 1
OpStore %_333_129_1_coords %1150
%1152 = OpLoad %v2float %_333_129_1_coords
OpStore %_334_130_2_inCoord %1152
%1153 = OpLoad %v2float %_334_130_2_inCoord
%1154 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1155 = OpLoad %v4float %1154
%1156 = OpVectorShuffle %v2float %1155 %1155 0 1
%1157 = OpFMul %v2float %1153 %1156
OpStore %_334_130_2_inCoord %1157
%1159 = OpLoad %v2float %_334_130_2_inCoord
%1160 = OpCompositeExtract %float %1159 0
%1161 = OpAccessChain %_ptr_Function_float %_335_131_3_subsetCoord %int_0
OpStore %1161 %1160
%1162 = OpLoad %v2float %_334_130_2_inCoord
%1163 = OpCompositeExtract %float %1162 1
%1164 = OpAccessChain %_ptr_Function_float %_335_131_3_subsetCoord %int_1
OpStore %1164 %1163
%1166 = OpLoad %v2float %_335_131_3_subsetCoord
OpStore %_336_132_4_clampedCoord %1166
%1169 = OpLoad %21 %uTextureSampler_0_Stage1
%1170 = OpLoad %v2float %_336_132_4_clampedCoord
%1171 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1172 = OpLoad %v4float %1171
%1173 = OpVectorShuffle %v2float %1172 %1172 2 3
%1174 = OpFMul %v2float %1170 %1173
%1168 = OpImageSampleImplicitLod %v4float %1169 %1174
OpStore %_337_133_5_textureColor %1168
%1177 = OpLoad %v2float %_334_130_2_inCoord
%1178 = OpCompositeExtract %float %1177 0
%1179 = OpFAdd %float %1178 %float_0_00100000005
%1176 = OpExtInst %float %1 Floor %1179
%1180 = OpFAdd %float %1176 %float_0_5
OpStore %_338_134_6_snappedX %1180
%1181 = OpLoad %float %_338_134_6_snappedX
%1182 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1183 = OpLoad %v4float %1182
%1184 = OpCompositeExtract %float %1183 0
%1185 = OpFOrdLessThan %bool %1181 %1184
OpSelectionMerge %1187 None
OpBranchConditional %1185 %1187 %1186
%1186 = OpLabel
%1188 = OpLoad %float %_338_134_6_snappedX
%1189 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1190 = OpLoad %v4float %1189
%1191 = OpCompositeExtract %float %1190 2
%1192 = OpFOrdGreaterThan %bool %1188 %1191
OpBranch %1187
%1187 = OpLabel
%1193 = OpPhi %bool %true %1123 %1192 %1186
OpSelectionMerge %1195 None
OpBranchConditional %1193 %1194 %1195
%1194 = OpLabel
%1196 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1197 = OpLoad %v4float %1196
OpStore %_337_133_5_textureColor %1197
OpBranch %1195
%1195 = OpLabel
%1198 = OpLoad %v4float %_337_133_5_textureColor
OpStore %_332_128_0_TextureEffect_Stage1_c0_c0_c0 %1198
%1199 = OpLoad %v4float %_332_128_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_331_127_MatrixEffect_Stage1_c0_c0 %1199
%1200 = OpLoad %v4float %_208_output
%1201 = OpLoad %v4float %_331_127_MatrixEffect_Stage1_c0_c0
%1202 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_3
%1203 = OpLoad %v4float %1202
%1204 = OpCompositeExtract %float %1203 3
%1205 = OpVectorTimesScalar %v4float %1201 %1204
%1206 = OpFAdd %v4float %1200 %1205
OpStore %_208_output %1206
%1207 = OpLoad %v2float %_209_coord
%1208 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1209 = OpLoad %v2float %1208
%1210 = OpFAdd %v2float %1207 %1209
OpStore %_209_coord %1210
%1211 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1211
%1215 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1216 = OpLoad %mat3v3float %1215
%1217 = OpLoad %v2float %_210_coordSampled
%1218 = OpCompositeExtract %float %1217 0
%1219 = OpCompositeExtract %float %1217 1
%1220 = OpCompositeConstruct %v3float %1218 %1219 %float_1
%1221 = OpMatrixTimesVector %v3float %1216 %1220
%1222 = OpVectorShuffle %v2float %1221 %1221 0 1
OpStore %_341_137_1_coords %1222
%1224 = OpLoad %v2float %_341_137_1_coords
OpStore %_342_138_2_inCoord %1224
%1225 = OpLoad %v2float %_342_138_2_inCoord
%1226 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1227 = OpLoad %v4float %1226
%1228 = OpVectorShuffle %v2float %1227 %1227 0 1
%1229 = OpFMul %v2float %1225 %1228
OpStore %_342_138_2_inCoord %1229
%1231 = OpLoad %v2float %_342_138_2_inCoord
%1232 = OpCompositeExtract %float %1231 0
%1233 = OpAccessChain %_ptr_Function_float %_343_139_3_subsetCoord %int_0
OpStore %1233 %1232
%1234 = OpLoad %v2float %_342_138_2_inCoord
%1235 = OpCompositeExtract %float %1234 1
%1236 = OpAccessChain %_ptr_Function_float %_343_139_3_subsetCoord %int_1
OpStore %1236 %1235
%1238 = OpLoad %v2float %_343_139_3_subsetCoord
OpStore %_344_140_4_clampedCoord %1238
%1241 = OpLoad %21 %uTextureSampler_0_Stage1
%1242 = OpLoad %v2float %_344_140_4_clampedCoord
%1243 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1244 = OpLoad %v4float %1243
%1245 = OpVectorShuffle %v2float %1244 %1244 2 3
%1246 = OpFMul %v2float %1242 %1245
%1240 = OpImageSampleImplicitLod %v4float %1241 %1246
OpStore %_345_141_5_textureColor %1240
%1249 = OpLoad %v2float %_342_138_2_inCoord
%1250 = OpCompositeExtract %float %1249 0
%1251 = OpFAdd %float %1250 %float_0_00100000005
%1248 = OpExtInst %float %1 Floor %1251
%1252 = OpFAdd %float %1248 %float_0_5
OpStore %_346_142_6_snappedX %1252
%1253 = OpLoad %float %_346_142_6_snappedX
%1254 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1255 = OpLoad %v4float %1254
%1256 = OpCompositeExtract %float %1255 0
%1257 = OpFOrdLessThan %bool %1253 %1256
OpSelectionMerge %1259 None
OpBranchConditional %1257 %1259 %1258
%1258 = OpLabel
%1260 = OpLoad %float %_346_142_6_snappedX
%1261 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1262 = OpLoad %v4float %1261
%1263 = OpCompositeExtract %float %1262 2
%1264 = OpFOrdGreaterThan %bool %1260 %1263
OpBranch %1259
%1259 = OpLabel
%1265 = OpPhi %bool %true %1195 %1264 %1258
OpSelectionMerge %1267 None
OpBranchConditional %1265 %1266 %1267
%1266 = OpLabel
%1268 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1269 = OpLoad %v4float %1268
OpStore %_345_141_5_textureColor %1269
OpBranch %1267
%1267 = OpLabel
%1270 = OpLoad %v4float %_345_141_5_textureColor
OpStore %_340_136_0_TextureEffect_Stage1_c0_c0_c0 %1270
%1271 = OpLoad %v4float %_340_136_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_339_135_MatrixEffect_Stage1_c0_c0 %1271
%1272 = OpLoad %v4float %_208_output
%1273 = OpLoad %v4float %_339_135_MatrixEffect_Stage1_c0_c0
%1274 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_4
%1275 = OpLoad %v4float %1274
%1276 = OpCompositeExtract %float %1275 0
%1277 = OpVectorTimesScalar %v4float %1273 %1276
%1278 = OpFAdd %v4float %1272 %1277
OpStore %_208_output %1278
%1279 = OpLoad %v2float %_209_coord
%1280 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1281 = OpLoad %v2float %1280
%1282 = OpFAdd %v2float %1279 %1281
OpStore %_209_coord %1282
%1283 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1283
%1287 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1288 = OpLoad %mat3v3float %1287
%1289 = OpLoad %v2float %_210_coordSampled
%1290 = OpCompositeExtract %float %1289 0
%1291 = OpCompositeExtract %float %1289 1
%1292 = OpCompositeConstruct %v3float %1290 %1291 %float_1
%1293 = OpMatrixTimesVector %v3float %1288 %1292
%1294 = OpVectorShuffle %v2float %1293 %1293 0 1
OpStore %_349_145_1_coords %1294
%1296 = OpLoad %v2float %_349_145_1_coords
OpStore %_350_146_2_inCoord %1296
%1297 = OpLoad %v2float %_350_146_2_inCoord
%1298 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1299 = OpLoad %v4float %1298
%1300 = OpVectorShuffle %v2float %1299 %1299 0 1
%1301 = OpFMul %v2float %1297 %1300
OpStore %_350_146_2_inCoord %1301
%1303 = OpLoad %v2float %_350_146_2_inCoord
%1304 = OpCompositeExtract %float %1303 0
%1305 = OpAccessChain %_ptr_Function_float %_351_147_3_subsetCoord %int_0
OpStore %1305 %1304
%1306 = OpLoad %v2float %_350_146_2_inCoord
%1307 = OpCompositeExtract %float %1306 1
%1308 = OpAccessChain %_ptr_Function_float %_351_147_3_subsetCoord %int_1
OpStore %1308 %1307
%1310 = OpLoad %v2float %_351_147_3_subsetCoord
OpStore %_352_148_4_clampedCoord %1310
%1313 = OpLoad %21 %uTextureSampler_0_Stage1
%1314 = OpLoad %v2float %_352_148_4_clampedCoord
%1315 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1316 = OpLoad %v4float %1315
%1317 = OpVectorShuffle %v2float %1316 %1316 2 3
%1318 = OpFMul %v2float %1314 %1317
%1312 = OpImageSampleImplicitLod %v4float %1313 %1318
OpStore %_353_149_5_textureColor %1312
%1321 = OpLoad %v2float %_350_146_2_inCoord
%1322 = OpCompositeExtract %float %1321 0
%1323 = OpFAdd %float %1322 %float_0_00100000005
%1320 = OpExtInst %float %1 Floor %1323
%1324 = OpFAdd %float %1320 %float_0_5
OpStore %_354_150_6_snappedX %1324
%1325 = OpLoad %float %_354_150_6_snappedX
%1326 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1327 = OpLoad %v4float %1326
%1328 = OpCompositeExtract %float %1327 0
%1329 = OpFOrdLessThan %bool %1325 %1328
OpSelectionMerge %1331 None
OpBranchConditional %1329 %1331 %1330
%1330 = OpLabel
%1332 = OpLoad %float %_354_150_6_snappedX
%1333 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1334 = OpLoad %v4float %1333
%1335 = OpCompositeExtract %float %1334 2
%1336 = OpFOrdGreaterThan %bool %1332 %1335
OpBranch %1331
%1331 = OpLabel
%1337 = OpPhi %bool %true %1267 %1336 %1330
OpSelectionMerge %1339 None
OpBranchConditional %1337 %1338 %1339
%1338 = OpLabel
%1340 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1341 = OpLoad %v4float %1340
OpStore %_353_149_5_textureColor %1341
OpBranch %1339
%1339 = OpLabel
%1342 = OpLoad %v4float %_353_149_5_textureColor
OpStore %_348_144_0_TextureEffect_Stage1_c0_c0_c0 %1342
%1343 = OpLoad %v4float %_348_144_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_347_143_MatrixEffect_Stage1_c0_c0 %1343
%1344 = OpLoad %v4float %_208_output
%1345 = OpLoad %v4float %_347_143_MatrixEffect_Stage1_c0_c0
%1346 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_4
%1347 = OpLoad %v4float %1346
%1348 = OpCompositeExtract %float %1347 1
%1349 = OpVectorTimesScalar %v4float %1345 %1348
%1350 = OpFAdd %v4float %1344 %1349
OpStore %_208_output %1350
%1351 = OpLoad %v2float %_209_coord
%1352 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1353 = OpLoad %v2float %1352
%1354 = OpFAdd %v2float %1351 %1353
OpStore %_209_coord %1354
%1355 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1355
%1359 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1360 = OpLoad %mat3v3float %1359
%1361 = OpLoad %v2float %_210_coordSampled
%1362 = OpCompositeExtract %float %1361 0
%1363 = OpCompositeExtract %float %1361 1
%1364 = OpCompositeConstruct %v3float %1362 %1363 %float_1
%1365 = OpMatrixTimesVector %v3float %1360 %1364
%1366 = OpVectorShuffle %v2float %1365 %1365 0 1
OpStore %_357_153_1_coords %1366
%1368 = OpLoad %v2float %_357_153_1_coords
OpStore %_358_154_2_inCoord %1368
%1369 = OpLoad %v2float %_358_154_2_inCoord
%1370 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1371 = OpLoad %v4float %1370
%1372 = OpVectorShuffle %v2float %1371 %1371 0 1
%1373 = OpFMul %v2float %1369 %1372
OpStore %_358_154_2_inCoord %1373
%1375 = OpLoad %v2float %_358_154_2_inCoord
%1376 = OpCompositeExtract %float %1375 0
%1377 = OpAccessChain %_ptr_Function_float %_359_155_3_subsetCoord %int_0
OpStore %1377 %1376
%1378 = OpLoad %v2float %_358_154_2_inCoord
%1379 = OpCompositeExtract %float %1378 1
%1380 = OpAccessChain %_ptr_Function_float %_359_155_3_subsetCoord %int_1
OpStore %1380 %1379
%1382 = OpLoad %v2float %_359_155_3_subsetCoord
OpStore %_360_156_4_clampedCoord %1382
%1385 = OpLoad %21 %uTextureSampler_0_Stage1
%1386 = OpLoad %v2float %_360_156_4_clampedCoord
%1387 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1388 = OpLoad %v4float %1387
%1389 = OpVectorShuffle %v2float %1388 %1388 2 3
%1390 = OpFMul %v2float %1386 %1389
%1384 = OpImageSampleImplicitLod %v4float %1385 %1390
OpStore %_361_157_5_textureColor %1384
%1393 = OpLoad %v2float %_358_154_2_inCoord
%1394 = OpCompositeExtract %float %1393 0
%1395 = OpFAdd %float %1394 %float_0_00100000005
%1392 = OpExtInst %float %1 Floor %1395
%1396 = OpFAdd %float %1392 %float_0_5
OpStore %_362_158_6_snappedX %1396
%1397 = OpLoad %float %_362_158_6_snappedX
%1398 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1399 = OpLoad %v4float %1398
%1400 = OpCompositeExtract %float %1399 0
%1401 = OpFOrdLessThan %bool %1397 %1400
OpSelectionMerge %1403 None
OpBranchConditional %1401 %1403 %1402
%1402 = OpLabel
%1404 = OpLoad %float %_362_158_6_snappedX
%1405 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1406 = OpLoad %v4float %1405
%1407 = OpCompositeExtract %float %1406 2
%1408 = OpFOrdGreaterThan %bool %1404 %1407
OpBranch %1403
%1403 = OpLabel
%1409 = OpPhi %bool %true %1339 %1408 %1402
OpSelectionMerge %1411 None
OpBranchConditional %1409 %1410 %1411
%1410 = OpLabel
%1412 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1413 = OpLoad %v4float %1412
OpStore %_361_157_5_textureColor %1413
OpBranch %1411
%1411 = OpLabel
%1414 = OpLoad %v4float %_361_157_5_textureColor
OpStore %_356_152_0_TextureEffect_Stage1_c0_c0_c0 %1414
%1415 = OpLoad %v4float %_356_152_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_355_151_MatrixEffect_Stage1_c0_c0 %1415
%1416 = OpLoad %v4float %_208_output
%1417 = OpLoad %v4float %_355_151_MatrixEffect_Stage1_c0_c0
%1418 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_4
%1419 = OpLoad %v4float %1418
%1420 = OpCompositeExtract %float %1419 2
%1421 = OpVectorTimesScalar %v4float %1417 %1420
%1422 = OpFAdd %v4float %1416 %1421
OpStore %_208_output %1422
%1423 = OpLoad %v2float %_209_coord
%1424 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1425 = OpLoad %v2float %1424
%1426 = OpFAdd %v2float %1423 %1425
OpStore %_209_coord %1426
%1427 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1427
%1431 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1432 = OpLoad %mat3v3float %1431
%1433 = OpLoad %v2float %_210_coordSampled
%1434 = OpCompositeExtract %float %1433 0
%1435 = OpCompositeExtract %float %1433 1
%1436 = OpCompositeConstruct %v3float %1434 %1435 %float_1
%1437 = OpMatrixTimesVector %v3float %1432 %1436
%1438 = OpVectorShuffle %v2float %1437 %1437 0 1
OpStore %_365_161_1_coords %1438
%1440 = OpLoad %v2float %_365_161_1_coords
OpStore %_366_162_2_inCoord %1440
%1441 = OpLoad %v2float %_366_162_2_inCoord
%1442 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1443 = OpLoad %v4float %1442
%1444 = OpVectorShuffle %v2float %1443 %1443 0 1
%1445 = OpFMul %v2float %1441 %1444
OpStore %_366_162_2_inCoord %1445
%1447 = OpLoad %v2float %_366_162_2_inCoord
%1448 = OpCompositeExtract %float %1447 0
%1449 = OpAccessChain %_ptr_Function_float %_367_163_3_subsetCoord %int_0
OpStore %1449 %1448
%1450 = OpLoad %v2float %_366_162_2_inCoord
%1451 = OpCompositeExtract %float %1450 1
%1452 = OpAccessChain %_ptr_Function_float %_367_163_3_subsetCoord %int_1
OpStore %1452 %1451
%1454 = OpLoad %v2float %_367_163_3_subsetCoord
OpStore %_368_164_4_clampedCoord %1454
%1457 = OpLoad %21 %uTextureSampler_0_Stage1
%1458 = OpLoad %v2float %_368_164_4_clampedCoord
%1459 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1460 = OpLoad %v4float %1459
%1461 = OpVectorShuffle %v2float %1460 %1460 2 3
%1462 = OpFMul %v2float %1458 %1461
%1456 = OpImageSampleImplicitLod %v4float %1457 %1462
OpStore %_369_165_5_textureColor %1456
%1465 = OpLoad %v2float %_366_162_2_inCoord
%1466 = OpCompositeExtract %float %1465 0
%1467 = OpFAdd %float %1466 %float_0_00100000005
%1464 = OpExtInst %float %1 Floor %1467
%1468 = OpFAdd %float %1464 %float_0_5
OpStore %_370_166_6_snappedX %1468
%1469 = OpLoad %float %_370_166_6_snappedX
%1470 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1471 = OpLoad %v4float %1470
%1472 = OpCompositeExtract %float %1471 0
%1473 = OpFOrdLessThan %bool %1469 %1472
OpSelectionMerge %1475 None
OpBranchConditional %1473 %1475 %1474
%1474 = OpLabel
%1476 = OpLoad %float %_370_166_6_snappedX
%1477 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1478 = OpLoad %v4float %1477
%1479 = OpCompositeExtract %float %1478 2
%1480 = OpFOrdGreaterThan %bool %1476 %1479
OpBranch %1475
%1475 = OpLabel
%1481 = OpPhi %bool %true %1411 %1480 %1474
OpSelectionMerge %1483 None
OpBranchConditional %1481 %1482 %1483
%1482 = OpLabel
%1484 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1485 = OpLoad %v4float %1484
OpStore %_369_165_5_textureColor %1485
OpBranch %1483
%1483 = OpLabel
%1486 = OpLoad %v4float %_369_165_5_textureColor
OpStore %_364_160_0_TextureEffect_Stage1_c0_c0_c0 %1486
%1487 = OpLoad %v4float %_364_160_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_363_159_MatrixEffect_Stage1_c0_c0 %1487
%1488 = OpLoad %v4float %_208_output
%1489 = OpLoad %v4float %_363_159_MatrixEffect_Stage1_c0_c0
%1490 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_4
%1491 = OpLoad %v4float %1490
%1492 = OpCompositeExtract %float %1491 3
%1493 = OpVectorTimesScalar %v4float %1489 %1492
%1494 = OpFAdd %v4float %1488 %1493
OpStore %_208_output %1494
%1495 = OpLoad %v2float %_209_coord
%1496 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1497 = OpLoad %v2float %1496
%1498 = OpFAdd %v2float %1495 %1497
OpStore %_209_coord %1498
%1499 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1499
%1503 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1504 = OpLoad %mat3v3float %1503
%1505 = OpLoad %v2float %_210_coordSampled
%1506 = OpCompositeExtract %float %1505 0
%1507 = OpCompositeExtract %float %1505 1
%1508 = OpCompositeConstruct %v3float %1506 %1507 %float_1
%1509 = OpMatrixTimesVector %v3float %1504 %1508
%1510 = OpVectorShuffle %v2float %1509 %1509 0 1
OpStore %_373_169_1_coords %1510
%1512 = OpLoad %v2float %_373_169_1_coords
OpStore %_374_170_2_inCoord %1512
%1513 = OpLoad %v2float %_374_170_2_inCoord
%1514 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1515 = OpLoad %v4float %1514
%1516 = OpVectorShuffle %v2float %1515 %1515 0 1
%1517 = OpFMul %v2float %1513 %1516
OpStore %_374_170_2_inCoord %1517
%1519 = OpLoad %v2float %_374_170_2_inCoord
%1520 = OpCompositeExtract %float %1519 0
%1521 = OpAccessChain %_ptr_Function_float %_375_171_3_subsetCoord %int_0
OpStore %1521 %1520
%1522 = OpLoad %v2float %_374_170_2_inCoord
%1523 = OpCompositeExtract %float %1522 1
%1524 = OpAccessChain %_ptr_Function_float %_375_171_3_subsetCoord %int_1
OpStore %1524 %1523
%1526 = OpLoad %v2float %_375_171_3_subsetCoord
OpStore %_376_172_4_clampedCoord %1526
%1529 = OpLoad %21 %uTextureSampler_0_Stage1
%1530 = OpLoad %v2float %_376_172_4_clampedCoord
%1531 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1532 = OpLoad %v4float %1531
%1533 = OpVectorShuffle %v2float %1532 %1532 2 3
%1534 = OpFMul %v2float %1530 %1533
%1528 = OpImageSampleImplicitLod %v4float %1529 %1534
OpStore %_377_173_5_textureColor %1528
%1537 = OpLoad %v2float %_374_170_2_inCoord
%1538 = OpCompositeExtract %float %1537 0
%1539 = OpFAdd %float %1538 %float_0_00100000005
%1536 = OpExtInst %float %1 Floor %1539
%1540 = OpFAdd %float %1536 %float_0_5
OpStore %_378_174_6_snappedX %1540
%1541 = OpLoad %float %_378_174_6_snappedX
%1542 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1543 = OpLoad %v4float %1542
%1544 = OpCompositeExtract %float %1543 0
%1545 = OpFOrdLessThan %bool %1541 %1544
OpSelectionMerge %1547 None
OpBranchConditional %1545 %1547 %1546
%1546 = OpLabel
%1548 = OpLoad %float %_378_174_6_snappedX
%1549 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1550 = OpLoad %v4float %1549
%1551 = OpCompositeExtract %float %1550 2
%1552 = OpFOrdGreaterThan %bool %1548 %1551
OpBranch %1547
%1547 = OpLabel
%1553 = OpPhi %bool %true %1483 %1552 %1546
OpSelectionMerge %1555 None
OpBranchConditional %1553 %1554 %1555
%1554 = OpLabel
%1556 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1557 = OpLoad %v4float %1556
OpStore %_377_173_5_textureColor %1557
OpBranch %1555
%1555 = OpLabel
%1558 = OpLoad %v4float %_377_173_5_textureColor
OpStore %_372_168_0_TextureEffect_Stage1_c0_c0_c0 %1558
%1559 = OpLoad %v4float %_372_168_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_371_167_MatrixEffect_Stage1_c0_c0 %1559
%1560 = OpLoad %v4float %_208_output
%1561 = OpLoad %v4float %_371_167_MatrixEffect_Stage1_c0_c0
%1562 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_5
%1563 = OpLoad %v4float %1562
%1564 = OpCompositeExtract %float %1563 0
%1565 = OpVectorTimesScalar %v4float %1561 %1564
%1566 = OpFAdd %v4float %1560 %1565
OpStore %_208_output %1566
%1567 = OpLoad %v2float %_209_coord
%1568 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1569 = OpLoad %v2float %1568
%1570 = OpFAdd %v2float %1567 %1569
OpStore %_209_coord %1570
%1571 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1571
%1575 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1576 = OpLoad %mat3v3float %1575
%1577 = OpLoad %v2float %_210_coordSampled
%1578 = OpCompositeExtract %float %1577 0
%1579 = OpCompositeExtract %float %1577 1
%1580 = OpCompositeConstruct %v3float %1578 %1579 %float_1
%1581 = OpMatrixTimesVector %v3float %1576 %1580
%1582 = OpVectorShuffle %v2float %1581 %1581 0 1
OpStore %_381_177_1_coords %1582
%1584 = OpLoad %v2float %_381_177_1_coords
OpStore %_382_178_2_inCoord %1584
%1585 = OpLoad %v2float %_382_178_2_inCoord
%1586 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1587 = OpLoad %v4float %1586
%1588 = OpVectorShuffle %v2float %1587 %1587 0 1
%1589 = OpFMul %v2float %1585 %1588
OpStore %_382_178_2_inCoord %1589
%1591 = OpLoad %v2float %_382_178_2_inCoord
%1592 = OpCompositeExtract %float %1591 0
%1593 = OpAccessChain %_ptr_Function_float %_383_179_3_subsetCoord %int_0
OpStore %1593 %1592
%1594 = OpLoad %v2float %_382_178_2_inCoord
%1595 = OpCompositeExtract %float %1594 1
%1596 = OpAccessChain %_ptr_Function_float %_383_179_3_subsetCoord %int_1
OpStore %1596 %1595
%1598 = OpLoad %v2float %_383_179_3_subsetCoord
OpStore %_384_180_4_clampedCoord %1598
%1601 = OpLoad %21 %uTextureSampler_0_Stage1
%1602 = OpLoad %v2float %_384_180_4_clampedCoord
%1603 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1604 = OpLoad %v4float %1603
%1605 = OpVectorShuffle %v2float %1604 %1604 2 3
%1606 = OpFMul %v2float %1602 %1605
%1600 = OpImageSampleImplicitLod %v4float %1601 %1606
OpStore %_385_181_5_textureColor %1600
%1609 = OpLoad %v2float %_382_178_2_inCoord
%1610 = OpCompositeExtract %float %1609 0
%1611 = OpFAdd %float %1610 %float_0_00100000005
%1608 = OpExtInst %float %1 Floor %1611
%1612 = OpFAdd %float %1608 %float_0_5
OpStore %_386_182_6_snappedX %1612
%1613 = OpLoad %float %_386_182_6_snappedX
%1614 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1615 = OpLoad %v4float %1614
%1616 = OpCompositeExtract %float %1615 0
%1617 = OpFOrdLessThan %bool %1613 %1616
OpSelectionMerge %1619 None
OpBranchConditional %1617 %1619 %1618
%1618 = OpLabel
%1620 = OpLoad %float %_386_182_6_snappedX
%1621 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1622 = OpLoad %v4float %1621
%1623 = OpCompositeExtract %float %1622 2
%1624 = OpFOrdGreaterThan %bool %1620 %1623
OpBranch %1619
%1619 = OpLabel
%1625 = OpPhi %bool %true %1555 %1624 %1618
OpSelectionMerge %1627 None
OpBranchConditional %1625 %1626 %1627
%1626 = OpLabel
%1628 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1629 = OpLoad %v4float %1628
OpStore %_385_181_5_textureColor %1629
OpBranch %1627
%1627 = OpLabel
%1630 = OpLoad %v4float %_385_181_5_textureColor
OpStore %_380_176_0_TextureEffect_Stage1_c0_c0_c0 %1630
%1631 = OpLoad %v4float %_380_176_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_379_175_MatrixEffect_Stage1_c0_c0 %1631
%1632 = OpLoad %v4float %_208_output
%1633 = OpLoad %v4float %_379_175_MatrixEffect_Stage1_c0_c0
%1634 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_5
%1635 = OpLoad %v4float %1634
%1636 = OpCompositeExtract %float %1635 1
%1637 = OpVectorTimesScalar %v4float %1633 %1636
%1638 = OpFAdd %v4float %1632 %1637
OpStore %_208_output %1638
%1639 = OpLoad %v2float %_209_coord
%1640 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1641 = OpLoad %v2float %1640
%1642 = OpFAdd %v2float %1639 %1641
OpStore %_209_coord %1642
%1643 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1643
%1647 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1648 = OpLoad %mat3v3float %1647
%1649 = OpLoad %v2float %_210_coordSampled
%1650 = OpCompositeExtract %float %1649 0
%1651 = OpCompositeExtract %float %1649 1
%1652 = OpCompositeConstruct %v3float %1650 %1651 %float_1
%1653 = OpMatrixTimesVector %v3float %1648 %1652
%1654 = OpVectorShuffle %v2float %1653 %1653 0 1
OpStore %_389_185_1_coords %1654
%1656 = OpLoad %v2float %_389_185_1_coords
OpStore %_390_186_2_inCoord %1656
%1657 = OpLoad %v2float %_390_186_2_inCoord
%1658 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1659 = OpLoad %v4float %1658
%1660 = OpVectorShuffle %v2float %1659 %1659 0 1
%1661 = OpFMul %v2float %1657 %1660
OpStore %_390_186_2_inCoord %1661
%1663 = OpLoad %v2float %_390_186_2_inCoord
%1664 = OpCompositeExtract %float %1663 0
%1665 = OpAccessChain %_ptr_Function_float %_391_187_3_subsetCoord %int_0
OpStore %1665 %1664
%1666 = OpLoad %v2float %_390_186_2_inCoord
%1667 = OpCompositeExtract %float %1666 1
%1668 = OpAccessChain %_ptr_Function_float %_391_187_3_subsetCoord %int_1
OpStore %1668 %1667
%1670 = OpLoad %v2float %_391_187_3_subsetCoord
OpStore %_392_188_4_clampedCoord %1670
%1673 = OpLoad %21 %uTextureSampler_0_Stage1
%1674 = OpLoad %v2float %_392_188_4_clampedCoord
%1675 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1676 = OpLoad %v4float %1675
%1677 = OpVectorShuffle %v2float %1676 %1676 2 3
%1678 = OpFMul %v2float %1674 %1677
%1672 = OpImageSampleImplicitLod %v4float %1673 %1678
OpStore %_393_189_5_textureColor %1672
%1681 = OpLoad %v2float %_390_186_2_inCoord
%1682 = OpCompositeExtract %float %1681 0
%1683 = OpFAdd %float %1682 %float_0_00100000005
%1680 = OpExtInst %float %1 Floor %1683
%1684 = OpFAdd %float %1680 %float_0_5
OpStore %_394_190_6_snappedX %1684
%1685 = OpLoad %float %_394_190_6_snappedX
%1686 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1687 = OpLoad %v4float %1686
%1688 = OpCompositeExtract %float %1687 0
%1689 = OpFOrdLessThan %bool %1685 %1688
OpSelectionMerge %1691 None
OpBranchConditional %1689 %1691 %1690
%1690 = OpLabel
%1692 = OpLoad %float %_394_190_6_snappedX
%1693 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1694 = OpLoad %v4float %1693
%1695 = OpCompositeExtract %float %1694 2
%1696 = OpFOrdGreaterThan %bool %1692 %1695
OpBranch %1691
%1691 = OpLabel
%1697 = OpPhi %bool %true %1627 %1696 %1690
OpSelectionMerge %1699 None
OpBranchConditional %1697 %1698 %1699
%1698 = OpLabel
%1700 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1701 = OpLoad %v4float %1700
OpStore %_393_189_5_textureColor %1701
OpBranch %1699
%1699 = OpLabel
%1702 = OpLoad %v4float %_393_189_5_textureColor
OpStore %_388_184_0_TextureEffect_Stage1_c0_c0_c0 %1702
%1703 = OpLoad %v4float %_388_184_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_387_183_MatrixEffect_Stage1_c0_c0 %1703
%1704 = OpLoad %v4float %_208_output
%1705 = OpLoad %v4float %_387_183_MatrixEffect_Stage1_c0_c0
%1706 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_5
%1707 = OpLoad %v4float %1706
%1708 = OpCompositeExtract %float %1707 2
%1709 = OpVectorTimesScalar %v4float %1705 %1708
%1710 = OpFAdd %v4float %1704 %1709
OpStore %_208_output %1710
%1711 = OpLoad %v2float %_209_coord
%1712 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1713 = OpLoad %v2float %1712
%1714 = OpFAdd %v2float %1711 %1713
OpStore %_209_coord %1714
%1715 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1715
%1719 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1720 = OpLoad %mat3v3float %1719
%1721 = OpLoad %v2float %_210_coordSampled
%1722 = OpCompositeExtract %float %1721 0
%1723 = OpCompositeExtract %float %1721 1
%1724 = OpCompositeConstruct %v3float %1722 %1723 %float_1
%1725 = OpMatrixTimesVector %v3float %1720 %1724
%1726 = OpVectorShuffle %v2float %1725 %1725 0 1
OpStore %_397_193_1_coords %1726
%1728 = OpLoad %v2float %_397_193_1_coords
OpStore %_398_194_2_inCoord %1728
%1729 = OpLoad %v2float %_398_194_2_inCoord
%1730 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1731 = OpLoad %v4float %1730
%1732 = OpVectorShuffle %v2float %1731 %1731 0 1
%1733 = OpFMul %v2float %1729 %1732
OpStore %_398_194_2_inCoord %1733
%1735 = OpLoad %v2float %_398_194_2_inCoord
%1736 = OpCompositeExtract %float %1735 0
%1737 = OpAccessChain %_ptr_Function_float %_399_195_3_subsetCoord %int_0
OpStore %1737 %1736
%1738 = OpLoad %v2float %_398_194_2_inCoord
%1739 = OpCompositeExtract %float %1738 1
%1740 = OpAccessChain %_ptr_Function_float %_399_195_3_subsetCoord %int_1
OpStore %1740 %1739
%1742 = OpLoad %v2float %_399_195_3_subsetCoord
OpStore %_400_196_4_clampedCoord %1742
%1745 = OpLoad %21 %uTextureSampler_0_Stage1
%1746 = OpLoad %v2float %_400_196_4_clampedCoord
%1747 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1748 = OpLoad %v4float %1747
%1749 = OpVectorShuffle %v2float %1748 %1748 2 3
%1750 = OpFMul %v2float %1746 %1749
%1744 = OpImageSampleImplicitLod %v4float %1745 %1750
OpStore %_401_197_5_textureColor %1744
%1753 = OpLoad %v2float %_398_194_2_inCoord
%1754 = OpCompositeExtract %float %1753 0
%1755 = OpFAdd %float %1754 %float_0_00100000005
%1752 = OpExtInst %float %1 Floor %1755
%1756 = OpFAdd %float %1752 %float_0_5
OpStore %_402_198_6_snappedX %1756
%1757 = OpLoad %float %_402_198_6_snappedX
%1758 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1759 = OpLoad %v4float %1758
%1760 = OpCompositeExtract %float %1759 0
%1761 = OpFOrdLessThan %bool %1757 %1760
OpSelectionMerge %1763 None
OpBranchConditional %1761 %1763 %1762
%1762 = OpLabel
%1764 = OpLoad %float %_402_198_6_snappedX
%1765 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1766 = OpLoad %v4float %1765
%1767 = OpCompositeExtract %float %1766 2
%1768 = OpFOrdGreaterThan %bool %1764 %1767
OpBranch %1763
%1763 = OpLabel
%1769 = OpPhi %bool %true %1699 %1768 %1762
OpSelectionMerge %1771 None
OpBranchConditional %1769 %1770 %1771
%1770 = OpLabel
%1772 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1773 = OpLoad %v4float %1772
OpStore %_401_197_5_textureColor %1773
OpBranch %1771
%1771 = OpLabel
%1774 = OpLoad %v4float %_401_197_5_textureColor
OpStore %_396_192_0_TextureEffect_Stage1_c0_c0_c0 %1774
%1775 = OpLoad %v4float %_396_192_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_395_191_MatrixEffect_Stage1_c0_c0 %1775
%1776 = OpLoad %v4float %_208_output
%1777 = OpLoad %v4float %_395_191_MatrixEffect_Stage1_c0_c0
%1778 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_5
%1779 = OpLoad %v4float %1778
%1780 = OpCompositeExtract %float %1779 3
%1781 = OpVectorTimesScalar %v4float %1777 %1780
%1782 = OpFAdd %v4float %1776 %1781
OpStore %_208_output %1782
%1783 = OpLoad %v2float %_209_coord
%1784 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1785 = OpLoad %v2float %1784
%1786 = OpFAdd %v2float %1783 %1785
OpStore %_209_coord %1786
%1787 = OpLoad %v2float %_209_coord
OpStore %_210_coordSampled %1787
%1791 = OpAccessChain %_ptr_Uniform_mat3v3float %3 %int_3
%1792 = OpLoad %mat3v3float %1791
%1793 = OpLoad %v2float %_210_coordSampled
%1794 = OpCompositeExtract %float %1793 0
%1795 = OpCompositeExtract %float %1793 1
%1796 = OpCompositeConstruct %v3float %1794 %1795 %float_1
%1797 = OpMatrixTimesVector %v3float %1792 %1796
%1798 = OpVectorShuffle %v2float %1797 %1797 0 1
OpStore %_405_201_1_coords %1798
%1800 = OpLoad %v2float %_405_201_1_coords
OpStore %_406_202_2_inCoord %1800
%1801 = OpLoad %v2float %_406_202_2_inCoord
%1802 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1803 = OpLoad %v4float %1802
%1804 = OpVectorShuffle %v2float %1803 %1803 0 1
%1805 = OpFMul %v2float %1801 %1804
OpStore %_406_202_2_inCoord %1805
%1807 = OpLoad %v2float %_406_202_2_inCoord
%1808 = OpCompositeExtract %float %1807 0
%1809 = OpAccessChain %_ptr_Function_float %_407_203_3_subsetCoord %int_0
OpStore %1809 %1808
%1810 = OpLoad %v2float %_406_202_2_inCoord
%1811 = OpCompositeExtract %float %1810 1
%1812 = OpAccessChain %_ptr_Function_float %_407_203_3_subsetCoord %int_1
OpStore %1812 %1811
%1814 = OpLoad %v2float %_407_203_3_subsetCoord
OpStore %_408_204_4_clampedCoord %1814
%1817 = OpLoad %21 %uTextureSampler_0_Stage1
%1818 = OpLoad %v2float %_408_204_4_clampedCoord
%1819 = OpAccessChain %_ptr_Uniform_v4float %3 %int_6
%1820 = OpLoad %v4float %1819
%1821 = OpVectorShuffle %v2float %1820 %1820 2 3
%1822 = OpFMul %v2float %1818 %1821
%1816 = OpImageSampleImplicitLod %v4float %1817 %1822
OpStore %_409_205_5_textureColor %1816
%1825 = OpLoad %v2float %_406_202_2_inCoord
%1826 = OpCompositeExtract %float %1825 0
%1827 = OpFAdd %float %1826 %float_0_00100000005
%1824 = OpExtInst %float %1 Floor %1827
%1828 = OpFAdd %float %1824 %float_0_5
OpStore %_410_206_6_snappedX %1828
%1829 = OpLoad %float %_410_206_6_snappedX
%1830 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1831 = OpLoad %v4float %1830
%1832 = OpCompositeExtract %float %1831 0
%1833 = OpFOrdLessThan %bool %1829 %1832
OpSelectionMerge %1835 None
OpBranchConditional %1833 %1835 %1834
%1834 = OpLabel
%1836 = OpLoad %float %_410_206_6_snappedX
%1837 = OpAccessChain %_ptr_Uniform_v4float %3 %int_5
%1838 = OpLoad %v4float %1837
%1839 = OpCompositeExtract %float %1838 2
%1840 = OpFOrdGreaterThan %bool %1836 %1839
OpBranch %1835
%1835 = OpLabel
%1841 = OpPhi %bool %true %1771 %1840 %1834
OpSelectionMerge %1843 None
OpBranchConditional %1841 %1842 %1843
%1842 = OpLabel
%1844 = OpAccessChain %_ptr_Uniform_v4float %3 %int_4
%1845 = OpLoad %v4float %1844
OpStore %_409_205_5_textureColor %1845
OpBranch %1843
%1843 = OpLabel
%1846 = OpLoad %v4float %_409_205_5_textureColor
OpStore %_404_200_0_TextureEffect_Stage1_c0_c0_c0 %1846
%1847 = OpLoad %v4float %_404_200_0_TextureEffect_Stage1_c0_c0_c0
OpStore %_403_199_MatrixEffect_Stage1_c0_c0 %1847
%1848 = OpLoad %v4float %_208_output
%1849 = OpLoad %v4float %_403_199_MatrixEffect_Stage1_c0_c0
%1850 = OpAccessChain %_ptr_Uniform_v4float %3 %int_2 %int_6
%1851 = OpLoad %v4float %1850
%1852 = OpCompositeExtract %float %1851 0
%1853 = OpVectorTimesScalar %v4float %1849 %1852
%1854 = OpFAdd %v4float %1848 %1853
OpStore %_208_output %1854
%1855 = OpLoad %v2float %_209_coord
%1856 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1
%1857 = OpLoad %v2float %1856
%1858 = OpFAdd %v2float %1855 %1857
OpStore %_209_coord %1858
%1859 = OpLoad %v4float %_208_output
OpStore %_207_GaussianConvolution_Stage1_c0 %1859
%1860 = OpLoad %v4float %_207_GaussianConvolution_Stage1_c0
OpStore %output_Stage1 %1860
%1861 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %1861
OpReturn
OpFunctionEnd
