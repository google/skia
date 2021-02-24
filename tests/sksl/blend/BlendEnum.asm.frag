OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_overlay_component "_blend_overlay_component"
OpName %blend_overlay "blend_overlay"
OpName %result "result"
OpName %_color_dodge_component "_color_dodge_component"
OpName %delta "delta"
OpName %_3_guarded_divide "_3_guarded_divide"
OpName %_4_n "_4_n"
OpName %_color_burn_component "_color_burn_component"
OpName %_5_guarded_divide "_5_guarded_divide"
OpName %_6_n "_6_n"
OpName %delta_0 "delta"
OpName %_soft_light_component "_soft_light_component"
OpName %_7_guarded_divide "_7_guarded_divide"
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_9_guarded_divide "_9_guarded_divide"
OpName %_10_n "_10_n"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %_11_blend_color_luminance "_11_blend_color_luminance"
OpName %lum "lum"
OpName %_12_blend_color_luminance "_12_blend_color_luminance"
OpName %result_0 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_13_guarded_divide "_13_guarded_divide"
OpName %_14_d "_14_d"
OpName %_15_guarded_divide "_15_guarded_divide"
OpName %_16_n "_16_n"
OpName %_17_d "_17_d"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_18_guarded_divide "_18_guarded_divide"
OpName %_19_n "_19_n"
OpName %_20_d "_20_d"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %_21_blend_color_saturation "_21_blend_color_saturation"
OpName %sat "sat"
OpName %main "main"
OpName %_0_blend "_0_blend"
OpName %_1_loop "_1_loop"
OpName %_2_blend_clear "_2_blend_clear"
OpName %_3_blend_src "_3_blend_src"
OpName %_4_blend_dst "_4_blend_dst"
OpName %_5_blend_src_over "_5_blend_src_over"
OpName %_6_blend_dst_over "_6_blend_dst_over"
OpName %_7_blend_src_in "_7_blend_src_in"
OpName %_8_blend_dst_in "_8_blend_dst_in"
OpName %_9_blend_src_in "_9_blend_src_in"
OpName %_10_blend_src_out "_10_blend_src_out"
OpName %_11_blend_dst_out "_11_blend_dst_out"
OpName %_12_blend_src_atop "_12_blend_src_atop"
OpName %_13_blend_dst_atop "_13_blend_dst_atop"
OpName %_14_blend_xor "_14_blend_xor"
OpName %_15_blend_plus "_15_blend_plus"
OpName %_16_blend_modulate "_16_blend_modulate"
OpName %_17_blend_screen "_17_blend_screen"
OpName %_18_blend_darken "_18_blend_darken"
OpName %_19_blend_src_over "_19_blend_src_over"
OpName %_20_result "_20_result"
OpName %_21_blend_lighten "_21_blend_lighten"
OpName %_22_blend_src_over "_22_blend_src_over"
OpName %_23_result "_23_result"
OpName %_24_blend_color_dodge "_24_blend_color_dodge"
OpName %_25_blend_color_burn "_25_blend_color_burn"
OpName %_26_blend_hard_light "_26_blend_hard_light"
OpName %_27_blend_soft_light "_27_blend_soft_light"
OpName %_28_blend_difference "_28_blend_difference"
OpName %_29_blend_exclusion "_29_blend_exclusion"
OpName %_30_blend_multiply "_30_blend_multiply"
OpName %_31_blend_hue "_31_blend_hue"
OpName %_32_alpha "_32_alpha"
OpName %_33_sda "_33_sda"
OpName %_34_dsa "_34_dsa"
OpName %_35_blend_saturation "_35_blend_saturation"
OpName %_36_alpha "_36_alpha"
OpName %_37_sda "_37_sda"
OpName %_38_dsa "_38_dsa"
OpName %_39_blend_color "_39_blend_color"
OpName %_40_alpha "_40_alpha"
OpName %_41_sda "_41_sda"
OpName %_42_dsa "_42_dsa"
OpName %_43_blend_luminosity "_43_blend_luminosity"
OpName %_44_alpha "_44_alpha"
OpName %_45_sda "_45_sda"
OpName %_46_dsa "_46_dsa"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %536 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %539 RelaxedPrecision
OpDecorate %540 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %571 RelaxedPrecision
OpDecorate %573 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %577 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %596 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %602 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %613 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
OpDecorate %626 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %639 RelaxedPrecision
OpDecorate %642 RelaxedPrecision
OpDecorate %646 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %654 RelaxedPrecision
OpDecorate %657 RelaxedPrecision
OpDecorate %661 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %669 RelaxedPrecision
OpDecorate %672 RelaxedPrecision
OpDecorate %676 RelaxedPrecision
OpDecorate %679 RelaxedPrecision
OpDecorate %734 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %739 RelaxedPrecision
OpDecorate %741 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %751 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %754 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %761 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %774 RelaxedPrecision
OpDecorate %777 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %781 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %795 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %799 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %804 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %807 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
OpDecorate %813 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %815 RelaxedPrecision
OpDecorate %817 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %827 RelaxedPrecision
OpDecorate %828 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %840 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %849 RelaxedPrecision
OpDecorate %852 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %856 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %861 RelaxedPrecision
OpDecorate %862 RelaxedPrecision
OpDecorate %864 RelaxedPrecision
OpDecorate %865 RelaxedPrecision
OpDecorate %867 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %877 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %882 RelaxedPrecision
OpDecorate %884 RelaxedPrecision
OpDecorate %887 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %894 RelaxedPrecision
OpDecorate %898 RelaxedPrecision
OpDecorate %901 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %909 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %913 RelaxedPrecision
OpDecorate %916 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %926 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %937 RelaxedPrecision
OpDecorate %939 RelaxedPrecision
OpDecorate %941 RelaxedPrecision
OpDecorate %942 RelaxedPrecision
OpDecorate %944 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %948 RelaxedPrecision
OpDecorate %950 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %961 RelaxedPrecision
OpDecorate %962 RelaxedPrecision
OpDecorate %965 RelaxedPrecision
OpDecorate %969 RelaxedPrecision
OpDecorate %972 RelaxedPrecision
OpDecorate %976 RelaxedPrecision
OpDecorate %979 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1001 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1020 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %1023 RelaxedPrecision
OpDecorate %1024 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1035 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1047 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1050 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1059 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1062 RelaxedPrecision
OpDecorate %1065 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1070 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %1075 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1080 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1083 RelaxedPrecision
OpDecorate %1087 RelaxedPrecision
OpDecorate %1089 RelaxedPrecision
OpDecorate %1091 RelaxedPrecision
OpDecorate %1093 RelaxedPrecision
OpDecorate %1095 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1101 RelaxedPrecision
OpDecorate %1104 RelaxedPrecision
OpDecorate %1106 RelaxedPrecision
OpDecorate %1110 RelaxedPrecision
OpDecorate %1112 RelaxedPrecision
OpDecorate %1115 RelaxedPrecision
OpDecorate %1117 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1119 RelaxedPrecision
OpDecorate %1120 RelaxedPrecision
OpDecorate %1122 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1124 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1150 RelaxedPrecision
OpDecorate %1152 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1161 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1166 RelaxedPrecision
OpDecorate %1168 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1171 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1179 RelaxedPrecision
OpDecorate %1181 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1189 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1193 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1197 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1208 RelaxedPrecision
OpDecorate %1210 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1216 RelaxedPrecision
OpDecorate %1217 RelaxedPrecision
OpDecorate %1218 RelaxedPrecision
OpDecorate %1220 RelaxedPrecision
OpDecorate %1221 RelaxedPrecision
OpDecorate %1222 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1230 RelaxedPrecision
OpDecorate %1231 RelaxedPrecision
OpDecorate %1232 RelaxedPrecision
OpDecorate %1236 RelaxedPrecision
OpDecorate %1238 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1242 RelaxedPrecision
OpDecorate %1244 RelaxedPrecision
OpDecorate %1248 RelaxedPrecision
OpDecorate %1250 RelaxedPrecision
OpDecorate %1253 RelaxedPrecision
OpDecorate %1255 RelaxedPrecision
OpDecorate %1257 RelaxedPrecision
OpDecorate %1260 RelaxedPrecision
OpDecorate %1262 RelaxedPrecision
OpDecorate %1263 RelaxedPrecision
OpDecorate %1264 RelaxedPrecision
OpDecorate %1265 RelaxedPrecision
OpDecorate %1267 RelaxedPrecision
OpDecorate %1268 RelaxedPrecision
OpDecorate %1269 RelaxedPrecision
OpDecorate %1273 RelaxedPrecision
OpDecorate %1275 RelaxedPrecision
OpDecorate %1277 RelaxedPrecision
OpDecorate %1278 RelaxedPrecision
OpDecorate %1279 RelaxedPrecision
OpDecorate %1283 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%22 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%64 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%float_0 = OpConstant %float 0
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v3float = OpTypePointer Function %v3float
%445 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%457 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%547 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%579 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%580 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%684 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_13 = OpConstant %int 13
%732 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_blend_overlay_component = OpFunction %float None %22
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%34 = OpVariable %_ptr_Function_float Function
%28 = OpLoad %v2float %25
%29 = OpCompositeExtract %float %28 0
%30 = OpFMul %float %float_2 %29
%31 = OpLoad %v2float %25
%32 = OpCompositeExtract %float %31 1
%33 = OpFOrdLessThanEqual %bool %30 %32
OpSelectionMerge %38 None
OpBranchConditional %33 %36 %37
%36 = OpLabel
%39 = OpLoad %v2float %24
%40 = OpCompositeExtract %float %39 0
%41 = OpFMul %float %float_2 %40
%42 = OpLoad %v2float %25
%43 = OpCompositeExtract %float %42 0
%44 = OpFMul %float %41 %43
OpStore %34 %44
OpBranch %38
%37 = OpLabel
%45 = OpLoad %v2float %24
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %25
%48 = OpCompositeExtract %float %47 1
%49 = OpFMul %float %46 %48
%50 = OpLoad %v2float %25
%51 = OpCompositeExtract %float %50 1
%52 = OpLoad %v2float %25
%53 = OpCompositeExtract %float %52 0
%54 = OpFSub %float %51 %53
%55 = OpFMul %float %float_2 %54
%56 = OpLoad %v2float %24
%57 = OpCompositeExtract %float %56 1
%58 = OpLoad %v2float %24
%59 = OpCompositeExtract %float %58 0
%60 = OpFSub %float %57 %59
%61 = OpFMul %float %55 %60
%62 = OpFSub %float %49 %61
OpStore %34 %62
OpBranch %38
%38 = OpLabel
%63 = OpLoad %float %34
OpReturnValue %63
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %64
%66 = OpFunctionParameter %_ptr_Function_v4float
%67 = OpFunctionParameter %_ptr_Function_v4float
%68 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%72 = OpVariable %_ptr_Function_v2float Function
%75 = OpVariable %_ptr_Function_v2float Function
%79 = OpVariable %_ptr_Function_v2float Function
%82 = OpVariable %_ptr_Function_v2float Function
%86 = OpVariable %_ptr_Function_v2float Function
%89 = OpVariable %_ptr_Function_v2float Function
%70 = OpLoad %v4float %66
%71 = OpVectorShuffle %v2float %70 %70 0 3
OpStore %72 %71
%73 = OpLoad %v4float %67
%74 = OpVectorShuffle %v2float %73 %73 0 3
OpStore %75 %74
%76 = OpFunctionCall %float %_blend_overlay_component %72 %75
%77 = OpLoad %v4float %66
%78 = OpVectorShuffle %v2float %77 %77 1 3
OpStore %79 %78
%80 = OpLoad %v4float %67
%81 = OpVectorShuffle %v2float %80 %80 1 3
OpStore %82 %81
%83 = OpFunctionCall %float %_blend_overlay_component %79 %82
%84 = OpLoad %v4float %66
%85 = OpVectorShuffle %v2float %84 %84 2 3
OpStore %86 %85
%87 = OpLoad %v4float %67
%88 = OpVectorShuffle %v2float %87 %87 2 3
OpStore %89 %88
%90 = OpFunctionCall %float %_blend_overlay_component %86 %89
%91 = OpLoad %v4float %66
%92 = OpCompositeExtract %float %91 3
%94 = OpLoad %v4float %66
%95 = OpCompositeExtract %float %94 3
%96 = OpFSub %float %float_1 %95
%97 = OpLoad %v4float %67
%98 = OpCompositeExtract %float %97 3
%99 = OpFMul %float %96 %98
%100 = OpFAdd %float %92 %99
%101 = OpCompositeConstruct %v4float %76 %83 %90 %100
OpStore %result %101
%102 = OpLoad %v4float %result
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%105 = OpLoad %v4float %67
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%107 = OpLoad %v4float %66
%108 = OpCompositeExtract %float %107 3
%109 = OpFSub %float %float_1 %108
%110 = OpVectorTimesScalar %v3float %106 %109
%111 = OpLoad %v4float %66
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%113 = OpLoad %v4float %67
%114 = OpCompositeExtract %float %113 3
%115 = OpFSub %float %float_1 %114
%116 = OpVectorTimesScalar %v3float %112 %115
%117 = OpFAdd %v3float %110 %116
%118 = OpFAdd %v3float %103 %117
%119 = OpLoad %v4float %result
%120 = OpVectorShuffle %v4float %119 %118 4 5 6 3
OpStore %result %120
%121 = OpLoad %v4float %result
OpReturnValue %121
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %22
%122 = OpFunctionParameter %_ptr_Function_v2float
%123 = OpFunctionParameter %_ptr_Function_v2float
%124 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_3_guarded_divide = OpVariable %_ptr_Function_float Function
%_4_n = OpVariable %_ptr_Function_float Function
%125 = OpLoad %v2float %123
%126 = OpCompositeExtract %float %125 0
%128 = OpFOrdEqual %bool %126 %float_0
OpSelectionMerge %131 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpLoad %v2float %122
%133 = OpCompositeExtract %float %132 0
%134 = OpLoad %v2float %123
%135 = OpCompositeExtract %float %134 1
%136 = OpFSub %float %float_1 %135
%137 = OpFMul %float %133 %136
OpReturnValue %137
%130 = OpLabel
%139 = OpLoad %v2float %122
%140 = OpCompositeExtract %float %139 1
%141 = OpLoad %v2float %122
%142 = OpCompositeExtract %float %141 0
%143 = OpFSub %float %140 %142
OpStore %delta %143
%144 = OpLoad %float %delta
%145 = OpFOrdEqual %bool %144 %float_0
OpSelectionMerge %148 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpLoad %v2float %122
%150 = OpCompositeExtract %float %149 1
%151 = OpLoad %v2float %123
%152 = OpCompositeExtract %float %151 1
%153 = OpFMul %float %150 %152
%154 = OpLoad %v2float %122
%155 = OpCompositeExtract %float %154 0
%156 = OpLoad %v2float %123
%157 = OpCompositeExtract %float %156 1
%158 = OpFSub %float %float_1 %157
%159 = OpFMul %float %155 %158
%160 = OpFAdd %float %153 %159
%161 = OpLoad %v2float %123
%162 = OpCompositeExtract %float %161 0
%163 = OpLoad %v2float %122
%164 = OpCompositeExtract %float %163 1
%165 = OpFSub %float %float_1 %164
%166 = OpFMul %float %162 %165
%167 = OpFAdd %float %160 %166
OpReturnValue %167
%147 = OpLabel
%170 = OpLoad %v2float %123
%171 = OpCompositeExtract %float %170 0
%172 = OpLoad %v2float %122
%173 = OpCompositeExtract %float %172 1
%174 = OpFMul %float %171 %173
OpStore %_4_n %174
%176 = OpLoad %v2float %123
%177 = OpCompositeExtract %float %176 1
%178 = OpLoad %float %_4_n
%179 = OpLoad %float %delta
%180 = OpFDiv %float %178 %179
%175 = OpExtInst %float %1 FMin %177 %180
OpStore %delta %175
%181 = OpLoad %float %delta
%182 = OpLoad %v2float %122
%183 = OpCompositeExtract %float %182 1
%184 = OpFMul %float %181 %183
%185 = OpLoad %v2float %122
%186 = OpCompositeExtract %float %185 0
%187 = OpLoad %v2float %123
%188 = OpCompositeExtract %float %187 1
%189 = OpFSub %float %float_1 %188
%190 = OpFMul %float %186 %189
%191 = OpFAdd %float %184 %190
%192 = OpLoad %v2float %123
%193 = OpCompositeExtract %float %192 0
%194 = OpLoad %v2float %122
%195 = OpCompositeExtract %float %194 1
%196 = OpFSub %float %float_1 %195
%197 = OpFMul %float %193 %196
%198 = OpFAdd %float %191 %197
OpReturnValue %198
%148 = OpLabel
OpBranch %131
%131 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %22
%199 = OpFunctionParameter %_ptr_Function_v2float
%200 = OpFunctionParameter %_ptr_Function_v2float
%201 = OpLabel
%_5_guarded_divide = OpVariable %_ptr_Function_float Function
%_6_n = OpVariable %_ptr_Function_float Function
%delta_0 = OpVariable %_ptr_Function_float Function
%202 = OpLoad %v2float %200
%203 = OpCompositeExtract %float %202 1
%204 = OpLoad %v2float %200
%205 = OpCompositeExtract %float %204 0
%206 = OpFOrdEqual %bool %203 %205
OpSelectionMerge %209 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%210 = OpLoad %v2float %199
%211 = OpCompositeExtract %float %210 1
%212 = OpLoad %v2float %200
%213 = OpCompositeExtract %float %212 1
%214 = OpFMul %float %211 %213
%215 = OpLoad %v2float %199
%216 = OpCompositeExtract %float %215 0
%217 = OpLoad %v2float %200
%218 = OpCompositeExtract %float %217 1
%219 = OpFSub %float %float_1 %218
%220 = OpFMul %float %216 %219
%221 = OpFAdd %float %214 %220
%222 = OpLoad %v2float %200
%223 = OpCompositeExtract %float %222 0
%224 = OpLoad %v2float %199
%225 = OpCompositeExtract %float %224 1
%226 = OpFSub %float %float_1 %225
%227 = OpFMul %float %223 %226
%228 = OpFAdd %float %221 %227
OpReturnValue %228
%208 = OpLabel
%229 = OpLoad %v2float %199
%230 = OpCompositeExtract %float %229 0
%231 = OpFOrdEqual %bool %230 %float_0
OpSelectionMerge %234 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%235 = OpLoad %v2float %200
%236 = OpCompositeExtract %float %235 0
%237 = OpLoad %v2float %199
%238 = OpCompositeExtract %float %237 1
%239 = OpFSub %float %float_1 %238
%240 = OpFMul %float %236 %239
OpReturnValue %240
%233 = OpLabel
%243 = OpLoad %v2float %200
%244 = OpCompositeExtract %float %243 1
%245 = OpLoad %v2float %200
%246 = OpCompositeExtract %float %245 0
%247 = OpFSub %float %244 %246
%248 = OpLoad %v2float %199
%249 = OpCompositeExtract %float %248 1
%250 = OpFMul %float %247 %249
OpStore %_6_n %250
%253 = OpLoad %v2float %200
%254 = OpCompositeExtract %float %253 1
%255 = OpLoad %float %_6_n
%256 = OpLoad %v2float %199
%257 = OpCompositeExtract %float %256 0
%258 = OpFDiv %float %255 %257
%259 = OpFSub %float %254 %258
%252 = OpExtInst %float %1 FMax %float_0 %259
OpStore %delta_0 %252
%260 = OpLoad %float %delta_0
%261 = OpLoad %v2float %199
%262 = OpCompositeExtract %float %261 1
%263 = OpFMul %float %260 %262
%264 = OpLoad %v2float %199
%265 = OpCompositeExtract %float %264 0
%266 = OpLoad %v2float %200
%267 = OpCompositeExtract %float %266 1
%268 = OpFSub %float %float_1 %267
%269 = OpFMul %float %265 %268
%270 = OpFAdd %float %263 %269
%271 = OpLoad %v2float %200
%272 = OpCompositeExtract %float %271 0
%273 = OpLoad %v2float %199
%274 = OpCompositeExtract %float %273 1
%275 = OpFSub %float %float_1 %274
%276 = OpFMul %float %272 %275
%277 = OpFAdd %float %270 %276
OpReturnValue %277
%234 = OpLabel
OpBranch %209
%209 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %22
%278 = OpFunctionParameter %_ptr_Function_v2float
%279 = OpFunctionParameter %_ptr_Function_v2float
%280 = OpLabel
%_7_guarded_divide = OpVariable %_ptr_Function_float Function
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_9_guarded_divide = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
%281 = OpLoad %v2float %278
%282 = OpCompositeExtract %float %281 0
%283 = OpFMul %float %float_2 %282
%284 = OpLoad %v2float %278
%285 = OpCompositeExtract %float %284 1
%286 = OpFOrdLessThanEqual %bool %283 %285
OpSelectionMerge %289 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%292 = OpLoad %v2float %279
%293 = OpCompositeExtract %float %292 0
%294 = OpLoad %v2float %279
%295 = OpCompositeExtract %float %294 0
%296 = OpFMul %float %293 %295
%297 = OpLoad %v2float %278
%298 = OpCompositeExtract %float %297 1
%299 = OpLoad %v2float %278
%300 = OpCompositeExtract %float %299 0
%301 = OpFMul %float %float_2 %300
%302 = OpFSub %float %298 %301
%303 = OpFMul %float %296 %302
OpStore %_8_n %303
%304 = OpLoad %float %_8_n
%305 = OpLoad %v2float %279
%306 = OpCompositeExtract %float %305 1
%307 = OpFDiv %float %304 %306
%308 = OpLoad %v2float %279
%309 = OpCompositeExtract %float %308 1
%310 = OpFSub %float %float_1 %309
%311 = OpLoad %v2float %278
%312 = OpCompositeExtract %float %311 0
%313 = OpFMul %float %310 %312
%314 = OpFAdd %float %307 %313
%315 = OpLoad %v2float %279
%316 = OpCompositeExtract %float %315 0
%318 = OpLoad %v2float %278
%319 = OpCompositeExtract %float %318 1
%317 = OpFNegate %float %319
%320 = OpLoad %v2float %278
%321 = OpCompositeExtract %float %320 0
%322 = OpFMul %float %float_2 %321
%323 = OpFAdd %float %317 %322
%324 = OpFAdd %float %323 %float_1
%325 = OpFMul %float %316 %324
%326 = OpFAdd %float %314 %325
OpReturnValue %326
%288 = OpLabel
%328 = OpLoad %v2float %279
%329 = OpCompositeExtract %float %328 0
%330 = OpFMul %float %float_4 %329
%331 = OpLoad %v2float %279
%332 = OpCompositeExtract %float %331 1
%333 = OpFOrdLessThanEqual %bool %330 %332
OpSelectionMerge %336 None
OpBranchConditional %333 %334 %335
%334 = OpLabel
%338 = OpLoad %v2float %279
%339 = OpCompositeExtract %float %338 0
%340 = OpLoad %v2float %279
%341 = OpCompositeExtract %float %340 0
%342 = OpFMul %float %339 %341
OpStore %DSqd %342
%344 = OpLoad %float %DSqd
%345 = OpLoad %v2float %279
%346 = OpCompositeExtract %float %345 0
%347 = OpFMul %float %344 %346
OpStore %DCub %347
%349 = OpLoad %v2float %279
%350 = OpCompositeExtract %float %349 1
%351 = OpLoad %v2float %279
%352 = OpCompositeExtract %float %351 1
%353 = OpFMul %float %350 %352
OpStore %DaSqd %353
%355 = OpLoad %float %DaSqd
%356 = OpLoad %v2float %279
%357 = OpCompositeExtract %float %356 1
%358 = OpFMul %float %355 %357
OpStore %DaCub %358
%361 = OpLoad %float %DaSqd
%362 = OpLoad %v2float %278
%363 = OpCompositeExtract %float %362 0
%364 = OpLoad %v2float %279
%365 = OpCompositeExtract %float %364 0
%367 = OpLoad %v2float %278
%368 = OpCompositeExtract %float %367 1
%369 = OpFMul %float %float_3 %368
%371 = OpLoad %v2float %278
%372 = OpCompositeExtract %float %371 0
%373 = OpFMul %float %float_6 %372
%374 = OpFSub %float %369 %373
%375 = OpFSub %float %374 %float_1
%376 = OpFMul %float %365 %375
%377 = OpFSub %float %363 %376
%378 = OpFMul %float %361 %377
%380 = OpLoad %v2float %279
%381 = OpCompositeExtract %float %380 1
%382 = OpFMul %float %float_12 %381
%383 = OpLoad %float %DSqd
%384 = OpFMul %float %382 %383
%385 = OpLoad %v2float %278
%386 = OpCompositeExtract %float %385 1
%387 = OpLoad %v2float %278
%388 = OpCompositeExtract %float %387 0
%389 = OpFMul %float %float_2 %388
%390 = OpFSub %float %386 %389
%391 = OpFMul %float %384 %390
%392 = OpFAdd %float %378 %391
%394 = OpLoad %float %DCub
%395 = OpFMul %float %float_16 %394
%396 = OpLoad %v2float %278
%397 = OpCompositeExtract %float %396 1
%398 = OpLoad %v2float %278
%399 = OpCompositeExtract %float %398 0
%400 = OpFMul %float %float_2 %399
%401 = OpFSub %float %397 %400
%402 = OpFMul %float %395 %401
%403 = OpFSub %float %392 %402
%404 = OpLoad %float %DaCub
%405 = OpLoad %v2float %278
%406 = OpCompositeExtract %float %405 0
%407 = OpFMul %float %404 %406
%408 = OpFSub %float %403 %407
OpStore %_10_n %408
%409 = OpLoad %float %_10_n
%410 = OpLoad %float %DaSqd
%411 = OpFDiv %float %409 %410
OpReturnValue %411
%335 = OpLabel
%412 = OpLoad %v2float %279
%413 = OpCompositeExtract %float %412 0
%414 = OpLoad %v2float %278
%415 = OpCompositeExtract %float %414 1
%416 = OpLoad %v2float %278
%417 = OpCompositeExtract %float %416 0
%418 = OpFMul %float %float_2 %417
%419 = OpFSub %float %415 %418
%420 = OpFAdd %float %419 %float_1
%421 = OpFMul %float %413 %420
%422 = OpLoad %v2float %278
%423 = OpCompositeExtract %float %422 0
%424 = OpFAdd %float %421 %423
%426 = OpLoad %v2float %279
%427 = OpCompositeExtract %float %426 1
%428 = OpLoad %v2float %279
%429 = OpCompositeExtract %float %428 0
%430 = OpFMul %float %427 %429
%425 = OpExtInst %float %1 Sqrt %430
%431 = OpLoad %v2float %278
%432 = OpCompositeExtract %float %431 1
%433 = OpLoad %v2float %278
%434 = OpCompositeExtract %float %433 0
%435 = OpFMul %float %float_2 %434
%436 = OpFSub %float %432 %435
%437 = OpFMul %float %425 %436
%438 = OpFSub %float %424 %437
%439 = OpLoad %v2float %279
%440 = OpCompositeExtract %float %439 1
%441 = OpLoad %v2float %278
%442 = OpCompositeExtract %float %441 0
%443 = OpFMul %float %440 %442
%444 = OpFSub %float %438 %443
OpReturnValue %444
%336 = OpLabel
OpBranch %289
%289 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %445
%447 = OpFunctionParameter %_ptr_Function_v3float
%448 = OpFunctionParameter %_ptr_Function_float
%449 = OpFunctionParameter %_ptr_Function_v3float
%450 = OpLabel
%_11_blend_color_luminance = OpVariable %_ptr_Function_float Function
%lum = OpVariable %_ptr_Function_float Function
%_12_blend_color_luminance = OpVariable %_ptr_Function_float Function
%result_0 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_13_guarded_divide = OpVariable %_ptr_Function_float Function
%_14_d = OpVariable %_ptr_Function_float Function
%_15_guarded_divide = OpVariable %_ptr_Function_v3float Function
%_16_n = OpVariable %_ptr_Function_v3float Function
%_17_d = OpVariable %_ptr_Function_float Function
%458 = OpLoad %v3float %449
%453 = OpDot %float %457 %458
OpStore %lum %453
%461 = OpLoad %float %lum
%463 = OpLoad %v3float %447
%462 = OpDot %float %457 %463
%464 = OpFSub %float %461 %462
%465 = OpLoad %v3float %447
%466 = OpCompositeConstruct %v3float %464 %464 %464
%467 = OpFAdd %v3float %466 %465
OpStore %result_0 %467
%471 = OpLoad %v3float %result_0
%472 = OpCompositeExtract %float %471 0
%473 = OpLoad %v3float %result_0
%474 = OpCompositeExtract %float %473 1
%470 = OpExtInst %float %1 FMin %472 %474
%475 = OpLoad %v3float %result_0
%476 = OpCompositeExtract %float %475 2
%469 = OpExtInst %float %1 FMin %470 %476
OpStore %minComp %469
%480 = OpLoad %v3float %result_0
%481 = OpCompositeExtract %float %480 0
%482 = OpLoad %v3float %result_0
%483 = OpCompositeExtract %float %482 1
%479 = OpExtInst %float %1 FMax %481 %483
%484 = OpLoad %v3float %result_0
%485 = OpCompositeExtract %float %484 2
%478 = OpExtInst %float %1 FMax %479 %485
OpStore %maxComp %478
%487 = OpLoad %float %minComp
%488 = OpFOrdLessThan %bool %487 %float_0
OpSelectionMerge %490 None
OpBranchConditional %488 %489 %490
%489 = OpLabel
%491 = OpLoad %float %lum
%492 = OpLoad %float %minComp
%493 = OpFOrdNotEqual %bool %491 %492
OpBranch %490
%490 = OpLabel
%494 = OpPhi %bool %false %450 %493 %489
OpSelectionMerge %496 None
OpBranchConditional %494 %495 %496
%495 = OpLabel
%499 = OpLoad %float %lum
%500 = OpLoad %float %minComp
%501 = OpFSub %float %499 %500
OpStore %_14_d %501
%502 = OpLoad %float %lum
%503 = OpLoad %v3float %result_0
%504 = OpLoad %float %lum
%505 = OpCompositeConstruct %v3float %504 %504 %504
%506 = OpFSub %v3float %503 %505
%507 = OpLoad %float %lum
%508 = OpLoad %float %_14_d
%509 = OpFDiv %float %507 %508
%510 = OpVectorTimesScalar %v3float %506 %509
%511 = OpCompositeConstruct %v3float %502 %502 %502
%512 = OpFAdd %v3float %511 %510
OpStore %result_0 %512
OpBranch %496
%496 = OpLabel
%513 = OpLoad %float %maxComp
%514 = OpLoad %float %448
%515 = OpFOrdGreaterThan %bool %513 %514
OpSelectionMerge %517 None
OpBranchConditional %515 %516 %517
%516 = OpLabel
%518 = OpLoad %float %maxComp
%519 = OpLoad %float %lum
%520 = OpFOrdNotEqual %bool %518 %519
OpBranch %517
%517 = OpLabel
%521 = OpPhi %bool %false %496 %520 %516
OpSelectionMerge %524 None
OpBranchConditional %521 %522 %523
%522 = OpLabel
%527 = OpLoad %v3float %result_0
%528 = OpLoad %float %lum
%529 = OpCompositeConstruct %v3float %528 %528 %528
%530 = OpFSub %v3float %527 %529
%531 = OpLoad %float %448
%532 = OpLoad %float %lum
%533 = OpFSub %float %531 %532
%534 = OpVectorTimesScalar %v3float %530 %533
OpStore %_16_n %534
%536 = OpLoad %float %maxComp
%537 = OpLoad %float %lum
%538 = OpFSub %float %536 %537
OpStore %_17_d %538
%539 = OpLoad %float %lum
%540 = OpLoad %v3float %_16_n
%541 = OpLoad %float %_17_d
%542 = OpFDiv %float %float_1 %541
%543 = OpVectorTimesScalar %v3float %540 %542
%544 = OpCompositeConstruct %v3float %539 %539 %539
%545 = OpFAdd %v3float %544 %543
OpReturnValue %545
%523 = OpLabel
%546 = OpLoad %v3float %result_0
OpReturnValue %546
%524 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %547
%548 = OpFunctionParameter %_ptr_Function_v3float
%549 = OpFunctionParameter %_ptr_Function_float
%550 = OpLabel
%_18_guarded_divide = OpVariable %_ptr_Function_float Function
%_19_n = OpVariable %_ptr_Function_float Function
%_20_d = OpVariable %_ptr_Function_float Function
%551 = OpLoad %v3float %548
%552 = OpCompositeExtract %float %551 0
%553 = OpLoad %v3float %548
%554 = OpCompositeExtract %float %553 2
%555 = OpFOrdLessThan %bool %552 %554
OpSelectionMerge %558 None
OpBranchConditional %555 %556 %557
%556 = OpLabel
%561 = OpLoad %float %549
%562 = OpLoad %v3float %548
%563 = OpCompositeExtract %float %562 1
%564 = OpLoad %v3float %548
%565 = OpCompositeExtract %float %564 0
%566 = OpFSub %float %563 %565
%567 = OpFMul %float %561 %566
OpStore %_19_n %567
%569 = OpLoad %v3float %548
%570 = OpCompositeExtract %float %569 2
%571 = OpLoad %v3float %548
%572 = OpCompositeExtract %float %571 0
%573 = OpFSub %float %570 %572
OpStore %_20_d %573
%574 = OpLoad %float %_19_n
%575 = OpLoad %float %_20_d
%576 = OpFDiv %float %574 %575
%577 = OpLoad %float %549
%578 = OpCompositeConstruct %v3float %float_0 %576 %577
OpReturnValue %578
%557 = OpLabel
OpReturnValue %579
%558 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %580
%581 = OpFunctionParameter %_ptr_Function_v3float
%582 = OpFunctionParameter %_ptr_Function_v3float
%583 = OpLabel
%_21_blend_color_saturation = OpVariable %_ptr_Function_float Function
%sat = OpVariable %_ptr_Function_float Function
%620 = OpVariable %_ptr_Function_v3float Function
%622 = OpVariable %_ptr_Function_float Function
%634 = OpVariable %_ptr_Function_v3float Function
%636 = OpVariable %_ptr_Function_float Function
%641 = OpVariable %_ptr_Function_v3float Function
%643 = OpVariable %_ptr_Function_float Function
%656 = OpVariable %_ptr_Function_v3float Function
%658 = OpVariable %_ptr_Function_float Function
%671 = OpVariable %_ptr_Function_v3float Function
%673 = OpVariable %_ptr_Function_float Function
%678 = OpVariable %_ptr_Function_v3float Function
%680 = OpVariable %_ptr_Function_float Function
%588 = OpLoad %v3float %582
%589 = OpCompositeExtract %float %588 0
%590 = OpLoad %v3float %582
%591 = OpCompositeExtract %float %590 1
%587 = OpExtInst %float %1 FMax %589 %591
%592 = OpLoad %v3float %582
%593 = OpCompositeExtract %float %592 2
%586 = OpExtInst %float %1 FMax %587 %593
%596 = OpLoad %v3float %582
%597 = OpCompositeExtract %float %596 0
%598 = OpLoad %v3float %582
%599 = OpCompositeExtract %float %598 1
%595 = OpExtInst %float %1 FMin %597 %599
%600 = OpLoad %v3float %582
%601 = OpCompositeExtract %float %600 2
%594 = OpExtInst %float %1 FMin %595 %601
%602 = OpFSub %float %586 %594
OpStore %sat %602
%603 = OpLoad %v3float %581
%604 = OpCompositeExtract %float %603 0
%605 = OpLoad %v3float %581
%606 = OpCompositeExtract %float %605 1
%607 = OpFOrdLessThanEqual %bool %604 %606
OpSelectionMerge %610 None
OpBranchConditional %607 %608 %609
%608 = OpLabel
%611 = OpLoad %v3float %581
%612 = OpCompositeExtract %float %611 1
%613 = OpLoad %v3float %581
%614 = OpCompositeExtract %float %613 2
%615 = OpFOrdLessThanEqual %bool %612 %614
OpSelectionMerge %618 None
OpBranchConditional %615 %616 %617
%616 = OpLabel
%619 = OpLoad %v3float %581
OpStore %620 %619
%621 = OpLoad %float %sat
OpStore %622 %621
%623 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %620 %622
OpReturnValue %623
%617 = OpLabel
%624 = OpLoad %v3float %581
%625 = OpCompositeExtract %float %624 0
%626 = OpLoad %v3float %581
%627 = OpCompositeExtract %float %626 2
%628 = OpFOrdLessThanEqual %bool %625 %627
OpSelectionMerge %631 None
OpBranchConditional %628 %629 %630
%629 = OpLabel
%632 = OpLoad %v3float %581
%633 = OpVectorShuffle %v3float %632 %632 0 2 1
OpStore %634 %633
%635 = OpLoad %float %sat
OpStore %636 %635
%637 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %634 %636
%638 = OpVectorShuffle %v3float %637 %637 0 2 1
OpReturnValue %638
%630 = OpLabel
%639 = OpLoad %v3float %581
%640 = OpVectorShuffle %v3float %639 %639 2 0 1
OpStore %641 %640
%642 = OpLoad %float %sat
OpStore %643 %642
%644 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %641 %643
%645 = OpVectorShuffle %v3float %644 %644 1 2 0
OpReturnValue %645
%631 = OpLabel
OpBranch %618
%618 = OpLabel
OpBranch %610
%609 = OpLabel
%646 = OpLoad %v3float %581
%647 = OpCompositeExtract %float %646 0
%648 = OpLoad %v3float %581
%649 = OpCompositeExtract %float %648 2
%650 = OpFOrdLessThanEqual %bool %647 %649
OpSelectionMerge %653 None
OpBranchConditional %650 %651 %652
%651 = OpLabel
%654 = OpLoad %v3float %581
%655 = OpVectorShuffle %v3float %654 %654 1 0 2
OpStore %656 %655
%657 = OpLoad %float %sat
OpStore %658 %657
%659 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %656 %658
%660 = OpVectorShuffle %v3float %659 %659 1 0 2
OpReturnValue %660
%652 = OpLabel
%661 = OpLoad %v3float %581
%662 = OpCompositeExtract %float %661 1
%663 = OpLoad %v3float %581
%664 = OpCompositeExtract %float %663 2
%665 = OpFOrdLessThanEqual %bool %662 %664
OpSelectionMerge %668 None
OpBranchConditional %665 %666 %667
%666 = OpLabel
%669 = OpLoad %v3float %581
%670 = OpVectorShuffle %v3float %669 %669 1 2 0
OpStore %671 %670
%672 = OpLoad %float %sat
OpStore %673 %672
%674 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %671 %673
%675 = OpVectorShuffle %v3float %674 %674 2 0 1
OpReturnValue %675
%667 = OpLabel
%676 = OpLoad %v3float %581
%677 = OpVectorShuffle %v3float %676 %676 2 1 0
OpStore %678 %677
%679 = OpLoad %float %sat
OpStore %680 %679
%681 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %678 %680
%682 = OpVectorShuffle %v3float %681 %681 2 1 0
OpReturnValue %682
%668 = OpLabel
OpBranch %653
%653 = OpLabel
OpBranch %610
%610 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %684
%685 = OpLabel
%_0_blend = OpVariable %_ptr_Function_v4float Function
%_1_loop = OpVariable %_ptr_Function_int Function
%_2_blend_clear = OpVariable %_ptr_Function_v4float Function
%_3_blend_src = OpVariable %_ptr_Function_v4float Function
%_4_blend_dst = OpVariable %_ptr_Function_v4float Function
%_5_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_6_blend_dst_over = OpVariable %_ptr_Function_v4float Function
%_7_blend_src_in = OpVariable %_ptr_Function_v4float Function
%_8_blend_dst_in = OpVariable %_ptr_Function_v4float Function
%_9_blend_src_in = OpVariable %_ptr_Function_v4float Function
%_10_blend_src_out = OpVariable %_ptr_Function_v4float Function
%_11_blend_dst_out = OpVariable %_ptr_Function_v4float Function
%_12_blend_src_atop = OpVariable %_ptr_Function_v4float Function
%_13_blend_dst_atop = OpVariable %_ptr_Function_v4float Function
%_14_blend_xor = OpVariable %_ptr_Function_v4float Function
%_15_blend_plus = OpVariable %_ptr_Function_v4float Function
%_16_blend_modulate = OpVariable %_ptr_Function_v4float Function
%_17_blend_screen = OpVariable %_ptr_Function_v4float Function
%829 = OpVariable %_ptr_Function_v4float Function
%831 = OpVariable %_ptr_Function_v4float Function
%_18_blend_darken = OpVariable %_ptr_Function_v4float Function
%_19_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_20_result = OpVariable %_ptr_Function_v4float Function
%_21_blend_lighten = OpVariable %_ptr_Function_v4float Function
%_22_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_23_result = OpVariable %_ptr_Function_v4float Function
%_24_blend_color_dodge = OpVariable %_ptr_Function_v4float Function
%886 = OpVariable %_ptr_Function_v2float Function
%889 = OpVariable %_ptr_Function_v2float Function
%893 = OpVariable %_ptr_Function_v2float Function
%896 = OpVariable %_ptr_Function_v2float Function
%900 = OpVariable %_ptr_Function_v2float Function
%903 = OpVariable %_ptr_Function_v2float Function
%_25_blend_color_burn = OpVariable %_ptr_Function_v4float Function
%918 = OpVariable %_ptr_Function_v2float Function
%921 = OpVariable %_ptr_Function_v2float Function
%925 = OpVariable %_ptr_Function_v2float Function
%928 = OpVariable %_ptr_Function_v2float Function
%932 = OpVariable %_ptr_Function_v2float Function
%935 = OpVariable %_ptr_Function_v2float Function
%_26_blend_hard_light = OpVariable %_ptr_Function_v4float Function
%949 = OpVariable %_ptr_Function_v4float Function
%951 = OpVariable %_ptr_Function_v4float Function
%_27_blend_soft_light = OpVariable %_ptr_Function_v4float Function
%957 = OpVariable %_ptr_Function_v4float Function
%964 = OpVariable %_ptr_Function_v2float Function
%967 = OpVariable %_ptr_Function_v2float Function
%971 = OpVariable %_ptr_Function_v2float Function
%974 = OpVariable %_ptr_Function_v2float Function
%978 = OpVariable %_ptr_Function_v2float Function
%981 = OpVariable %_ptr_Function_v2float Function
%_28_blend_difference = OpVariable %_ptr_Function_v4float Function
%_29_blend_exclusion = OpVariable %_ptr_Function_v4float Function
%_30_blend_multiply = OpVariable %_ptr_Function_v4float Function
%_31_blend_hue = OpVariable %_ptr_Function_v4float Function
%_32_alpha = OpVariable %_ptr_Function_float Function
%_33_sda = OpVariable %_ptr_Function_v3float Function
%_34_dsa = OpVariable %_ptr_Function_v3float Function
%1105 = OpVariable %_ptr_Function_v3float Function
%1107 = OpVariable %_ptr_Function_v3float Function
%1109 = OpVariable %_ptr_Function_v3float Function
%1111 = OpVariable %_ptr_Function_float Function
%1113 = OpVariable %_ptr_Function_v3float Function
%_35_blend_saturation = OpVariable %_ptr_Function_v4float Function
%_36_alpha = OpVariable %_ptr_Function_float Function
%_37_sda = OpVariable %_ptr_Function_v3float Function
%_38_dsa = OpVariable %_ptr_Function_v3float Function
%1156 = OpVariable %_ptr_Function_v3float Function
%1158 = OpVariable %_ptr_Function_v3float Function
%1160 = OpVariable %_ptr_Function_v3float Function
%1162 = OpVariable %_ptr_Function_float Function
%1164 = OpVariable %_ptr_Function_v3float Function
%_39_blend_color = OpVariable %_ptr_Function_v4float Function
%_40_alpha = OpVariable %_ptr_Function_float Function
%_41_sda = OpVariable %_ptr_Function_v3float Function
%_42_dsa = OpVariable %_ptr_Function_v3float Function
%1207 = OpVariable %_ptr_Function_v3float Function
%1209 = OpVariable %_ptr_Function_float Function
%1211 = OpVariable %_ptr_Function_v3float Function
%_43_blend_luminosity = OpVariable %_ptr_Function_v4float Function
%_44_alpha = OpVariable %_ptr_Function_float Function
%_45_sda = OpVariable %_ptr_Function_v3float Function
%_46_dsa = OpVariable %_ptr_Function_v3float Function
%1254 = OpVariable %_ptr_Function_v3float Function
%1256 = OpVariable %_ptr_Function_float Function
%1258 = OpVariable %_ptr_Function_v3float Function
OpStore %_1_loop %int_0
OpBranch %691
%691 = OpLabel
OpLoopMerge %695 %694 None
OpBranch %692
%692 = OpLabel
%696 = OpLoad %int %_1_loop
%698 = OpSLessThan %bool %696 %int_1
OpBranchConditional %698 %693 %695
%693 = OpLabel
OpSelectionMerge %700 None
OpSwitch %int_13 %730 0 %701 1 %702 2 %703 3 %704 4 %705 5 %706 6 %707 7 %708 8 %709 9 %710 10 %711 11 %712 12 %713 13 %714 14 %715 15 %716 16 %717 17 %718 18 %719 19 %720 20 %721 21 %722 22 %723 23 %724 24 %725 25 %726 26 %727 27 %728 28 %729
%701 = OpLabel
OpStore %_0_blend %732
OpBranch %694
%702 = OpLabel
%734 = OpLoad %v4float %src
OpStore %_0_blend %734
OpBranch %694
%703 = OpLabel
%736 = OpLoad %v4float %dst
OpStore %_0_blend %736
OpBranch %694
%704 = OpLabel
%738 = OpLoad %v4float %src
%739 = OpLoad %v4float %src
%740 = OpCompositeExtract %float %739 3
%741 = OpFSub %float %float_1 %740
%742 = OpLoad %v4float %dst
%743 = OpVectorTimesScalar %v4float %742 %741
%744 = OpFAdd %v4float %738 %743
OpStore %_0_blend %744
OpBranch %694
%705 = OpLabel
%746 = OpLoad %v4float %dst
%747 = OpCompositeExtract %float %746 3
%748 = OpFSub %float %float_1 %747
%749 = OpLoad %v4float %src
%750 = OpVectorTimesScalar %v4float %749 %748
%751 = OpLoad %v4float %dst
%752 = OpFAdd %v4float %750 %751
OpStore %_0_blend %752
OpBranch %694
%706 = OpLabel
%754 = OpLoad %v4float %src
%755 = OpLoad %v4float %dst
%756 = OpCompositeExtract %float %755 3
%757 = OpVectorTimesScalar %v4float %754 %756
OpStore %_0_blend %757
OpBranch %694
%707 = OpLabel
%760 = OpLoad %v4float %dst
%761 = OpLoad %v4float %src
%762 = OpCompositeExtract %float %761 3
%763 = OpVectorTimesScalar %v4float %760 %762
OpStore %_0_blend %763
OpBranch %694
%708 = OpLabel
%765 = OpLoad %v4float %dst
%766 = OpCompositeExtract %float %765 3
%767 = OpFSub %float %float_1 %766
%768 = OpLoad %v4float %src
%769 = OpVectorTimesScalar %v4float %768 %767
OpStore %_0_blend %769
OpBranch %694
%709 = OpLabel
%771 = OpLoad %v4float %src
%772 = OpCompositeExtract %float %771 3
%773 = OpFSub %float %float_1 %772
%774 = OpLoad %v4float %dst
%775 = OpVectorTimesScalar %v4float %774 %773
OpStore %_0_blend %775
OpBranch %694
%710 = OpLabel
%777 = OpLoad %v4float %dst
%778 = OpCompositeExtract %float %777 3
%779 = OpLoad %v4float %src
%780 = OpVectorTimesScalar %v4float %779 %778
%781 = OpLoad %v4float %src
%782 = OpCompositeExtract %float %781 3
%783 = OpFSub %float %float_1 %782
%784 = OpLoad %v4float %dst
%785 = OpVectorTimesScalar %v4float %784 %783
%786 = OpFAdd %v4float %780 %785
OpStore %_0_blend %786
OpBranch %694
%711 = OpLabel
%788 = OpLoad %v4float %dst
%789 = OpCompositeExtract %float %788 3
%790 = OpFSub %float %float_1 %789
%791 = OpLoad %v4float %src
%792 = OpVectorTimesScalar %v4float %791 %790
%793 = OpLoad %v4float %src
%794 = OpCompositeExtract %float %793 3
%795 = OpLoad %v4float %dst
%796 = OpVectorTimesScalar %v4float %795 %794
%797 = OpFAdd %v4float %792 %796
OpStore %_0_blend %797
OpBranch %694
%712 = OpLabel
%799 = OpLoad %v4float %dst
%800 = OpCompositeExtract %float %799 3
%801 = OpFSub %float %float_1 %800
%802 = OpLoad %v4float %src
%803 = OpVectorTimesScalar %v4float %802 %801
%804 = OpLoad %v4float %src
%805 = OpCompositeExtract %float %804 3
%806 = OpFSub %float %float_1 %805
%807 = OpLoad %v4float %dst
%808 = OpVectorTimesScalar %v4float %807 %806
%809 = OpFAdd %v4float %803 %808
OpStore %_0_blend %809
OpBranch %694
%713 = OpLabel
%812 = OpLoad %v4float %src
%813 = OpLoad %v4float %dst
%814 = OpFAdd %v4float %812 %813
%815 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%811 = OpExtInst %v4float %1 FMin %814 %815
OpStore %_0_blend %811
OpBranch %694
%714 = OpLabel
%817 = OpLoad %v4float %src
%818 = OpLoad %v4float %dst
%819 = OpFMul %v4float %817 %818
OpStore %_0_blend %819
OpBranch %694
%715 = OpLabel
%821 = OpLoad %v4float %src
%822 = OpLoad %v4float %src
%823 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%824 = OpFSub %v4float %823 %822
%825 = OpLoad %v4float %dst
%826 = OpFMul %v4float %824 %825
%827 = OpFAdd %v4float %821 %826
OpStore %_0_blend %827
OpBranch %694
%716 = OpLabel
%828 = OpLoad %v4float %src
OpStore %829 %828
%830 = OpLoad %v4float %dst
OpStore %831 %830
%832 = OpFunctionCall %v4float %blend_overlay %829 %831
OpStore %_0_blend %832
OpBranch %694
%717 = OpLabel
%836 = OpLoad %v4float %src
%837 = OpLoad %v4float %src
%838 = OpCompositeExtract %float %837 3
%839 = OpFSub %float %float_1 %838
%840 = OpLoad %v4float %dst
%841 = OpVectorTimesScalar %v4float %840 %839
%842 = OpFAdd %v4float %836 %841
OpStore %_20_result %842
%844 = OpLoad %v4float %_20_result
%845 = OpVectorShuffle %v3float %844 %844 0 1 2
%846 = OpLoad %v4float %dst
%847 = OpCompositeExtract %float %846 3
%848 = OpFSub %float %float_1 %847
%849 = OpLoad %v4float %src
%850 = OpVectorShuffle %v3float %849 %849 0 1 2
%851 = OpVectorTimesScalar %v3float %850 %848
%852 = OpLoad %v4float %dst
%853 = OpVectorShuffle %v3float %852 %852 0 1 2
%854 = OpFAdd %v3float %851 %853
%843 = OpExtInst %v3float %1 FMin %845 %854
%855 = OpLoad %v4float %_20_result
%856 = OpVectorShuffle %v4float %855 %843 4 5 6 3
OpStore %_20_result %856
%857 = OpLoad %v4float %_20_result
OpStore %_0_blend %857
OpBranch %694
%718 = OpLabel
%861 = OpLoad %v4float %src
%862 = OpLoad %v4float %src
%863 = OpCompositeExtract %float %862 3
%864 = OpFSub %float %float_1 %863
%865 = OpLoad %v4float %dst
%866 = OpVectorTimesScalar %v4float %865 %864
%867 = OpFAdd %v4float %861 %866
OpStore %_23_result %867
%869 = OpLoad %v4float %_23_result
%870 = OpVectorShuffle %v3float %869 %869 0 1 2
%871 = OpLoad %v4float %dst
%872 = OpCompositeExtract %float %871 3
%873 = OpFSub %float %float_1 %872
%874 = OpLoad %v4float %src
%875 = OpVectorShuffle %v3float %874 %874 0 1 2
%876 = OpVectorTimesScalar %v3float %875 %873
%877 = OpLoad %v4float %dst
%878 = OpVectorShuffle %v3float %877 %877 0 1 2
%879 = OpFAdd %v3float %876 %878
%868 = OpExtInst %v3float %1 FMax %870 %879
%880 = OpLoad %v4float %_23_result
%881 = OpVectorShuffle %v4float %880 %868 4 5 6 3
OpStore %_23_result %881
%882 = OpLoad %v4float %_23_result
OpStore %_0_blend %882
OpBranch %694
%719 = OpLabel
%884 = OpLoad %v4float %src
%885 = OpVectorShuffle %v2float %884 %884 0 3
OpStore %886 %885
%887 = OpLoad %v4float %dst
%888 = OpVectorShuffle %v2float %887 %887 0 3
OpStore %889 %888
%890 = OpFunctionCall %float %_color_dodge_component %886 %889
%891 = OpLoad %v4float %src
%892 = OpVectorShuffle %v2float %891 %891 1 3
OpStore %893 %892
%894 = OpLoad %v4float %dst
%895 = OpVectorShuffle %v2float %894 %894 1 3
OpStore %896 %895
%897 = OpFunctionCall %float %_color_dodge_component %893 %896
%898 = OpLoad %v4float %src
%899 = OpVectorShuffle %v2float %898 %898 2 3
OpStore %900 %899
%901 = OpLoad %v4float %dst
%902 = OpVectorShuffle %v2float %901 %901 2 3
OpStore %903 %902
%904 = OpFunctionCall %float %_color_dodge_component %900 %903
%905 = OpLoad %v4float %src
%906 = OpCompositeExtract %float %905 3
%907 = OpLoad %v4float %src
%908 = OpCompositeExtract %float %907 3
%909 = OpFSub %float %float_1 %908
%910 = OpLoad %v4float %dst
%911 = OpCompositeExtract %float %910 3
%912 = OpFMul %float %909 %911
%913 = OpFAdd %float %906 %912
%914 = OpCompositeConstruct %v4float %890 %897 %904 %913
OpStore %_0_blend %914
OpBranch %694
%720 = OpLabel
%916 = OpLoad %v4float %src
%917 = OpVectorShuffle %v2float %916 %916 0 3
OpStore %918 %917
%919 = OpLoad %v4float %dst
%920 = OpVectorShuffle %v2float %919 %919 0 3
OpStore %921 %920
%922 = OpFunctionCall %float %_color_burn_component %918 %921
%923 = OpLoad %v4float %src
%924 = OpVectorShuffle %v2float %923 %923 1 3
OpStore %925 %924
%926 = OpLoad %v4float %dst
%927 = OpVectorShuffle %v2float %926 %926 1 3
OpStore %928 %927
%929 = OpFunctionCall %float %_color_burn_component %925 %928
%930 = OpLoad %v4float %src
%931 = OpVectorShuffle %v2float %930 %930 2 3
OpStore %932 %931
%933 = OpLoad %v4float %dst
%934 = OpVectorShuffle %v2float %933 %933 2 3
OpStore %935 %934
%936 = OpFunctionCall %float %_color_burn_component %932 %935
%937 = OpLoad %v4float %src
%938 = OpCompositeExtract %float %937 3
%939 = OpLoad %v4float %src
%940 = OpCompositeExtract %float %939 3
%941 = OpFSub %float %float_1 %940
%942 = OpLoad %v4float %dst
%943 = OpCompositeExtract %float %942 3
%944 = OpFMul %float %941 %943
%945 = OpFAdd %float %938 %944
%946 = OpCompositeConstruct %v4float %922 %929 %936 %945
OpStore %_0_blend %946
OpBranch %694
%721 = OpLabel
%948 = OpLoad %v4float %dst
OpStore %949 %948
%950 = OpLoad %v4float %src
OpStore %951 %950
%952 = OpFunctionCall %v4float %blend_overlay %949 %951
OpStore %_0_blend %952
OpBranch %694
%722 = OpLabel
%954 = OpLoad %v4float %dst
%955 = OpCompositeExtract %float %954 3
%956 = OpFOrdEqual %bool %955 %float_0
OpSelectionMerge %960 None
OpBranchConditional %956 %958 %959
%958 = OpLabel
%961 = OpLoad %v4float %src
OpStore %957 %961
OpBranch %960
%959 = OpLabel
%962 = OpLoad %v4float %src
%963 = OpVectorShuffle %v2float %962 %962 0 3
OpStore %964 %963
%965 = OpLoad %v4float %dst
%966 = OpVectorShuffle %v2float %965 %965 0 3
OpStore %967 %966
%968 = OpFunctionCall %float %_soft_light_component %964 %967
%969 = OpLoad %v4float %src
%970 = OpVectorShuffle %v2float %969 %969 1 3
OpStore %971 %970
%972 = OpLoad %v4float %dst
%973 = OpVectorShuffle %v2float %972 %972 1 3
OpStore %974 %973
%975 = OpFunctionCall %float %_soft_light_component %971 %974
%976 = OpLoad %v4float %src
%977 = OpVectorShuffle %v2float %976 %976 2 3
OpStore %978 %977
%979 = OpLoad %v4float %dst
%980 = OpVectorShuffle %v2float %979 %979 2 3
OpStore %981 %980
%982 = OpFunctionCall %float %_soft_light_component %978 %981
%983 = OpLoad %v4float %src
%984 = OpCompositeExtract %float %983 3
%985 = OpLoad %v4float %src
%986 = OpCompositeExtract %float %985 3
%987 = OpFSub %float %float_1 %986
%988 = OpLoad %v4float %dst
%989 = OpCompositeExtract %float %988 3
%990 = OpFMul %float %987 %989
%991 = OpFAdd %float %984 %990
%992 = OpCompositeConstruct %v4float %968 %975 %982 %991
OpStore %957 %992
OpBranch %960
%960 = OpLabel
%993 = OpLoad %v4float %957
OpStore %_0_blend %993
OpBranch %694
%723 = OpLabel
%995 = OpLoad %v4float %src
%996 = OpVectorShuffle %v3float %995 %995 0 1 2
%997 = OpLoad %v4float %dst
%998 = OpVectorShuffle %v3float %997 %997 0 1 2
%999 = OpFAdd %v3float %996 %998
%1001 = OpLoad %v4float %src
%1002 = OpVectorShuffle %v3float %1001 %1001 0 1 2
%1003 = OpLoad %v4float %dst
%1004 = OpCompositeExtract %float %1003 3
%1005 = OpVectorTimesScalar %v3float %1002 %1004
%1006 = OpLoad %v4float %dst
%1007 = OpVectorShuffle %v3float %1006 %1006 0 1 2
%1008 = OpLoad %v4float %src
%1009 = OpCompositeExtract %float %1008 3
%1010 = OpVectorTimesScalar %v3float %1007 %1009
%1000 = OpExtInst %v3float %1 FMin %1005 %1010
%1011 = OpVectorTimesScalar %v3float %1000 %float_2
%1012 = OpFSub %v3float %999 %1011
%1013 = OpCompositeExtract %float %1012 0
%1014 = OpCompositeExtract %float %1012 1
%1015 = OpCompositeExtract %float %1012 2
%1016 = OpLoad %v4float %src
%1017 = OpCompositeExtract %float %1016 3
%1018 = OpLoad %v4float %src
%1019 = OpCompositeExtract %float %1018 3
%1020 = OpFSub %float %float_1 %1019
%1021 = OpLoad %v4float %dst
%1022 = OpCompositeExtract %float %1021 3
%1023 = OpFMul %float %1020 %1022
%1024 = OpFAdd %float %1017 %1023
%1025 = OpCompositeConstruct %v4float %1013 %1014 %1015 %1024
OpStore %_0_blend %1025
OpBranch %694
%724 = OpLabel
%1027 = OpLoad %v4float %dst
%1028 = OpVectorShuffle %v3float %1027 %1027 0 1 2
%1029 = OpLoad %v4float %src
%1030 = OpVectorShuffle %v3float %1029 %1029 0 1 2
%1031 = OpFAdd %v3float %1028 %1030
%1032 = OpLoad %v4float %dst
%1033 = OpVectorShuffle %v3float %1032 %1032 0 1 2
%1034 = OpVectorTimesScalar %v3float %1033 %float_2
%1035 = OpLoad %v4float %src
%1036 = OpVectorShuffle %v3float %1035 %1035 0 1 2
%1037 = OpFMul %v3float %1034 %1036
%1038 = OpFSub %v3float %1031 %1037
%1039 = OpCompositeExtract %float %1038 0
%1040 = OpCompositeExtract %float %1038 1
%1041 = OpCompositeExtract %float %1038 2
%1042 = OpLoad %v4float %src
%1043 = OpCompositeExtract %float %1042 3
%1044 = OpLoad %v4float %src
%1045 = OpCompositeExtract %float %1044 3
%1046 = OpFSub %float %float_1 %1045
%1047 = OpLoad %v4float %dst
%1048 = OpCompositeExtract %float %1047 3
%1049 = OpFMul %float %1046 %1048
%1050 = OpFAdd %float %1043 %1049
%1051 = OpCompositeConstruct %v4float %1039 %1040 %1041 %1050
OpStore %_0_blend %1051
OpBranch %694
%725 = OpLabel
%1053 = OpLoad %v4float %src
%1054 = OpCompositeExtract %float %1053 3
%1055 = OpFSub %float %float_1 %1054
%1056 = OpLoad %v4float %dst
%1057 = OpVectorShuffle %v3float %1056 %1056 0 1 2
%1058 = OpVectorTimesScalar %v3float %1057 %1055
%1059 = OpLoad %v4float %dst
%1060 = OpCompositeExtract %float %1059 3
%1061 = OpFSub %float %float_1 %1060
%1062 = OpLoad %v4float %src
%1063 = OpVectorShuffle %v3float %1062 %1062 0 1 2
%1064 = OpVectorTimesScalar %v3float %1063 %1061
%1065 = OpFAdd %v3float %1058 %1064
%1066 = OpLoad %v4float %src
%1067 = OpVectorShuffle %v3float %1066 %1066 0 1 2
%1068 = OpLoad %v4float %dst
%1069 = OpVectorShuffle %v3float %1068 %1068 0 1 2
%1070 = OpFMul %v3float %1067 %1069
%1071 = OpFAdd %v3float %1065 %1070
%1072 = OpCompositeExtract %float %1071 0
%1073 = OpCompositeExtract %float %1071 1
%1074 = OpCompositeExtract %float %1071 2
%1075 = OpLoad %v4float %src
%1076 = OpCompositeExtract %float %1075 3
%1077 = OpLoad %v4float %src
%1078 = OpCompositeExtract %float %1077 3
%1079 = OpFSub %float %float_1 %1078
%1080 = OpLoad %v4float %dst
%1081 = OpCompositeExtract %float %1080 3
%1082 = OpFMul %float %1079 %1081
%1083 = OpFAdd %float %1076 %1082
%1084 = OpCompositeConstruct %v4float %1072 %1073 %1074 %1083
OpStore %_0_blend %1084
OpBranch %694
%726 = OpLabel
%1087 = OpLoad %v4float %dst
%1088 = OpCompositeExtract %float %1087 3
%1089 = OpLoad %v4float %src
%1090 = OpCompositeExtract %float %1089 3
%1091 = OpFMul %float %1088 %1090
OpStore %_32_alpha %1091
%1093 = OpLoad %v4float %src
%1094 = OpVectorShuffle %v3float %1093 %1093 0 1 2
%1095 = OpLoad %v4float %dst
%1096 = OpCompositeExtract %float %1095 3
%1097 = OpVectorTimesScalar %v3float %1094 %1096
OpStore %_33_sda %1097
%1099 = OpLoad %v4float %dst
%1100 = OpVectorShuffle %v3float %1099 %1099 0 1 2
%1101 = OpLoad %v4float %src
%1102 = OpCompositeExtract %float %1101 3
%1103 = OpVectorTimesScalar %v3float %1100 %1102
OpStore %_34_dsa %1103
%1104 = OpLoad %v3float %_33_sda
OpStore %1105 %1104
%1106 = OpLoad %v3float %_34_dsa
OpStore %1107 %1106
%1108 = OpFunctionCall %v3float %_blend_set_color_saturation %1105 %1107
OpStore %1109 %1108
%1110 = OpLoad %float %_32_alpha
OpStore %1111 %1110
%1112 = OpLoad %v3float %_34_dsa
OpStore %1113 %1112
%1114 = OpFunctionCall %v3float %_blend_set_color_luminance %1109 %1111 %1113
%1115 = OpLoad %v4float %dst
%1116 = OpVectorShuffle %v3float %1115 %1115 0 1 2
%1117 = OpFAdd %v3float %1114 %1116
%1118 = OpLoad %v3float %_34_dsa
%1119 = OpFSub %v3float %1117 %1118
%1120 = OpLoad %v4float %src
%1121 = OpVectorShuffle %v3float %1120 %1120 0 1 2
%1122 = OpFAdd %v3float %1119 %1121
%1123 = OpLoad %v3float %_33_sda
%1124 = OpFSub %v3float %1122 %1123
%1125 = OpCompositeExtract %float %1124 0
%1126 = OpCompositeExtract %float %1124 1
%1127 = OpCompositeExtract %float %1124 2
%1128 = OpLoad %v4float %src
%1129 = OpCompositeExtract %float %1128 3
%1130 = OpLoad %v4float %dst
%1131 = OpCompositeExtract %float %1130 3
%1132 = OpFAdd %float %1129 %1131
%1133 = OpLoad %float %_32_alpha
%1134 = OpFSub %float %1132 %1133
%1135 = OpCompositeConstruct %v4float %1125 %1126 %1127 %1134
OpStore %_0_blend %1135
OpBranch %694
%727 = OpLabel
%1138 = OpLoad %v4float %dst
%1139 = OpCompositeExtract %float %1138 3
%1140 = OpLoad %v4float %src
%1141 = OpCompositeExtract %float %1140 3
%1142 = OpFMul %float %1139 %1141
OpStore %_36_alpha %1142
%1144 = OpLoad %v4float %src
%1145 = OpVectorShuffle %v3float %1144 %1144 0 1 2
%1146 = OpLoad %v4float %dst
%1147 = OpCompositeExtract %float %1146 3
%1148 = OpVectorTimesScalar %v3float %1145 %1147
OpStore %_37_sda %1148
%1150 = OpLoad %v4float %dst
%1151 = OpVectorShuffle %v3float %1150 %1150 0 1 2
%1152 = OpLoad %v4float %src
%1153 = OpCompositeExtract %float %1152 3
%1154 = OpVectorTimesScalar %v3float %1151 %1153
OpStore %_38_dsa %1154
%1155 = OpLoad %v3float %_38_dsa
OpStore %1156 %1155
%1157 = OpLoad %v3float %_37_sda
OpStore %1158 %1157
%1159 = OpFunctionCall %v3float %_blend_set_color_saturation %1156 %1158
OpStore %1160 %1159
%1161 = OpLoad %float %_36_alpha
OpStore %1162 %1161
%1163 = OpLoad %v3float %_38_dsa
OpStore %1164 %1163
%1165 = OpFunctionCall %v3float %_blend_set_color_luminance %1160 %1162 %1164
%1166 = OpLoad %v4float %dst
%1167 = OpVectorShuffle %v3float %1166 %1166 0 1 2
%1168 = OpFAdd %v3float %1165 %1167
%1169 = OpLoad %v3float %_38_dsa
%1170 = OpFSub %v3float %1168 %1169
%1171 = OpLoad %v4float %src
%1172 = OpVectorShuffle %v3float %1171 %1171 0 1 2
%1173 = OpFAdd %v3float %1170 %1172
%1174 = OpLoad %v3float %_37_sda
%1175 = OpFSub %v3float %1173 %1174
%1176 = OpCompositeExtract %float %1175 0
%1177 = OpCompositeExtract %float %1175 1
%1178 = OpCompositeExtract %float %1175 2
%1179 = OpLoad %v4float %src
%1180 = OpCompositeExtract %float %1179 3
%1181 = OpLoad %v4float %dst
%1182 = OpCompositeExtract %float %1181 3
%1183 = OpFAdd %float %1180 %1182
%1184 = OpLoad %float %_36_alpha
%1185 = OpFSub %float %1183 %1184
%1186 = OpCompositeConstruct %v4float %1176 %1177 %1178 %1185
OpStore %_0_blend %1186
OpBranch %694
%728 = OpLabel
%1189 = OpLoad %v4float %dst
%1190 = OpCompositeExtract %float %1189 3
%1191 = OpLoad %v4float %src
%1192 = OpCompositeExtract %float %1191 3
%1193 = OpFMul %float %1190 %1192
OpStore %_40_alpha %1193
%1195 = OpLoad %v4float %src
%1196 = OpVectorShuffle %v3float %1195 %1195 0 1 2
%1197 = OpLoad %v4float %dst
%1198 = OpCompositeExtract %float %1197 3
%1199 = OpVectorTimesScalar %v3float %1196 %1198
OpStore %_41_sda %1199
%1201 = OpLoad %v4float %dst
%1202 = OpVectorShuffle %v3float %1201 %1201 0 1 2
%1203 = OpLoad %v4float %src
%1204 = OpCompositeExtract %float %1203 3
%1205 = OpVectorTimesScalar %v3float %1202 %1204
OpStore %_42_dsa %1205
%1206 = OpLoad %v3float %_41_sda
OpStore %1207 %1206
%1208 = OpLoad %float %_40_alpha
OpStore %1209 %1208
%1210 = OpLoad %v3float %_42_dsa
OpStore %1211 %1210
%1212 = OpFunctionCall %v3float %_blend_set_color_luminance %1207 %1209 %1211
%1213 = OpLoad %v4float %dst
%1214 = OpVectorShuffle %v3float %1213 %1213 0 1 2
%1215 = OpFAdd %v3float %1212 %1214
%1216 = OpLoad %v3float %_42_dsa
%1217 = OpFSub %v3float %1215 %1216
%1218 = OpLoad %v4float %src
%1219 = OpVectorShuffle %v3float %1218 %1218 0 1 2
%1220 = OpFAdd %v3float %1217 %1219
%1221 = OpLoad %v3float %_41_sda
%1222 = OpFSub %v3float %1220 %1221
%1223 = OpCompositeExtract %float %1222 0
%1224 = OpCompositeExtract %float %1222 1
%1225 = OpCompositeExtract %float %1222 2
%1226 = OpLoad %v4float %src
%1227 = OpCompositeExtract %float %1226 3
%1228 = OpLoad %v4float %dst
%1229 = OpCompositeExtract %float %1228 3
%1230 = OpFAdd %float %1227 %1229
%1231 = OpLoad %float %_40_alpha
%1232 = OpFSub %float %1230 %1231
%1233 = OpCompositeConstruct %v4float %1223 %1224 %1225 %1232
OpStore %_0_blend %1233
OpBranch %694
%729 = OpLabel
%1236 = OpLoad %v4float %dst
%1237 = OpCompositeExtract %float %1236 3
%1238 = OpLoad %v4float %src
%1239 = OpCompositeExtract %float %1238 3
%1240 = OpFMul %float %1237 %1239
OpStore %_44_alpha %1240
%1242 = OpLoad %v4float %src
%1243 = OpVectorShuffle %v3float %1242 %1242 0 1 2
%1244 = OpLoad %v4float %dst
%1245 = OpCompositeExtract %float %1244 3
%1246 = OpVectorTimesScalar %v3float %1243 %1245
OpStore %_45_sda %1246
%1248 = OpLoad %v4float %dst
%1249 = OpVectorShuffle %v3float %1248 %1248 0 1 2
%1250 = OpLoad %v4float %src
%1251 = OpCompositeExtract %float %1250 3
%1252 = OpVectorTimesScalar %v3float %1249 %1251
OpStore %_46_dsa %1252
%1253 = OpLoad %v3float %_46_dsa
OpStore %1254 %1253
%1255 = OpLoad %float %_44_alpha
OpStore %1256 %1255
%1257 = OpLoad %v3float %_45_sda
OpStore %1258 %1257
%1259 = OpFunctionCall %v3float %_blend_set_color_luminance %1254 %1256 %1258
%1260 = OpLoad %v4float %dst
%1261 = OpVectorShuffle %v3float %1260 %1260 0 1 2
%1262 = OpFAdd %v3float %1259 %1261
%1263 = OpLoad %v3float %_46_dsa
%1264 = OpFSub %v3float %1262 %1263
%1265 = OpLoad %v4float %src
%1266 = OpVectorShuffle %v3float %1265 %1265 0 1 2
%1267 = OpFAdd %v3float %1264 %1266
%1268 = OpLoad %v3float %_45_sda
%1269 = OpFSub %v3float %1267 %1268
%1270 = OpCompositeExtract %float %1269 0
%1271 = OpCompositeExtract %float %1269 1
%1272 = OpCompositeExtract %float %1269 2
%1273 = OpLoad %v4float %src
%1274 = OpCompositeExtract %float %1273 3
%1275 = OpLoad %v4float %dst
%1276 = OpCompositeExtract %float %1275 3
%1277 = OpFAdd %float %1274 %1276
%1278 = OpLoad %float %_44_alpha
%1279 = OpFSub %float %1277 %1278
%1280 = OpCompositeConstruct %v4float %1270 %1271 %1272 %1279
OpStore %_0_blend %1280
OpBranch %694
%730 = OpLabel
OpStore %_0_blend %732
OpBranch %694
%700 = OpLabel
OpBranch %694
%694 = OpLabel
%1281 = OpLoad %int %_1_loop
%1282 = OpIAdd %int %1281 %int_1
OpStore %_1_loop %1282
OpBranch %691
%695 = OpLabel
%1283 = OpLoad %v4float %_0_blend
OpStore %sk_FragColor %1283
OpReturn
OpFunctionEnd
