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
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %_13_blend_color_saturation "_13_blend_color_saturation"
OpName %sat "sat"
OpName %blend "blend"
OpName %_15_blend_src "_15_blend_src"
OpName %_16_blend_dst "_16_blend_dst"
OpName %_17_blend_src_over "_17_blend_src_over"
OpName %_18_blend_dst_over "_18_blend_dst_over"
OpName %_19_blend_src_in "_19_blend_src_in"
OpName %_20_blend_dst_in "_20_blend_dst_in"
OpName %_21_blend_src_in "_21_blend_src_in"
OpName %_22_blend_src_out "_22_blend_src_out"
OpName %_23_blend_dst_out "_23_blend_dst_out"
OpName %_24_blend_src_atop "_24_blend_src_atop"
OpName %_25_blend_dst_atop "_25_blend_dst_atop"
OpName %_26_blend_xor "_26_blend_xor"
OpName %_27_blend_plus "_27_blend_plus"
OpName %_28_blend_modulate "_28_blend_modulate"
OpName %_29_blend_screen "_29_blend_screen"
OpName %_30_blend_darken "_30_blend_darken"
OpName %_31_blend_src_over "_31_blend_src_over"
OpName %_32_result "_32_result"
OpName %_33_blend_lighten "_33_blend_lighten"
OpName %_34_blend_src_over "_34_blend_src_over"
OpName %_35_result "_35_result"
OpName %_36_blend_color_dodge "_36_blend_color_dodge"
OpName %_37_blend_color_burn "_37_blend_color_burn"
OpName %_38_blend_hard_light "_38_blend_hard_light"
OpName %_39_blend_soft_light "_39_blend_soft_light"
OpName %_40_blend_difference "_40_blend_difference"
OpName %_41_blend_exclusion "_41_blend_exclusion"
OpName %_42_blend_multiply "_42_blend_multiply"
OpName %_43_blend_hue "_43_blend_hue"
OpName %_44_alpha "_44_alpha"
OpName %_45_sda "_45_sda"
OpName %_46_dsa "_46_dsa"
OpName %_47_blend_saturation "_47_blend_saturation"
OpName %_48_alpha "_48_alpha"
OpName %_49_sda "_49_sda"
OpName %_50_dsa "_50_dsa"
OpName %_51_blend_color "_51_blend_color"
OpName %_52_alpha "_52_alpha"
OpName %_53_sda "_53_sda"
OpName %_54_dsa "_54_dsa"
OpName %_55_blend_luminosity "_55_blend_luminosity"
OpName %_56_alpha "_56_alpha"
OpName %_57_sda "_57_sda"
OpName %_58_dsa "_58_dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %510 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %534 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %539 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %542 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %548 RelaxedPrecision
OpDecorate %549 RelaxedPrecision
OpDecorate %554 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %570 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %579 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %597 RelaxedPrecision
OpDecorate %599 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %625 RelaxedPrecision
OpDecorate %627 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %636 RelaxedPrecision
OpDecorate %640 RelaxedPrecision
OpDecorate %643 RelaxedPrecision
OpDecorate %647 RelaxedPrecision
OpDecorate %649 RelaxedPrecision
OpDecorate %655 RelaxedPrecision
OpDecorate %658 RelaxedPrecision
OpDecorate %662 RelaxedPrecision
OpDecorate %664 RelaxedPrecision
OpDecorate %670 RelaxedPrecision
OpDecorate %673 RelaxedPrecision
OpDecorate %677 RelaxedPrecision
OpDecorate %680 RelaxedPrecision
OpDecorate %691 RelaxedPrecision
OpDecorate %724 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %727 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %731 RelaxedPrecision
OpDecorate %733 RelaxedPrecision
OpDecorate %734 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %737 RelaxedPrecision
OpDecorate %739 RelaxedPrecision
OpDecorate %741 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %745 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %756 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %764 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %769 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %772 RelaxedPrecision
OpDecorate %774 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %785 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %795 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %798 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %805 RelaxedPrecision
OpDecorate %807 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %810 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %815 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %817 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %823 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %829 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %833 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %840 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %850 RelaxedPrecision
OpDecorate %852 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %855 RelaxedPrecision
OpDecorate %858 RelaxedPrecision
OpDecorate %860 RelaxedPrecision
OpDecorate %862 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %864 RelaxedPrecision
OpDecorate %867 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %875 RelaxedPrecision
OpDecorate %877 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %882 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %887 RelaxedPrecision
OpDecorate %889 RelaxedPrecision
OpDecorate %890 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %893 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %900 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %914 RelaxedPrecision
OpDecorate %916 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %921 RelaxedPrecision
OpDecorate %922 RelaxedPrecision
OpDecorate %924 RelaxedPrecision
OpDecorate %926 RelaxedPrecision
OpDecorate %929 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
OpDecorate %940 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %947 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %952 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %955 RelaxedPrecision
OpDecorate %957 RelaxedPrecision
OpDecorate %959 RelaxedPrecision
OpDecorate %961 RelaxedPrecision
OpDecorate %964 RelaxedPrecision
OpDecorate %966 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %974 RelaxedPrecision
OpDecorate %977 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %984 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1039 RelaxedPrecision
OpDecorate %1041 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1045 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1051 RelaxedPrecision
OpDecorate %1052 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1060 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1064 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1070 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1076 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1080 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1083 RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1094 RelaxedPrecision
OpDecorate %1095 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1098 RelaxedPrecision
OpDecorate %1100 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1107 RelaxedPrecision
OpDecorate %1109 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1115 RelaxedPrecision
OpDecorate %1117 RelaxedPrecision
OpDecorate %1120 RelaxedPrecision
OpDecorate %1122 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1135 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1139 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1148 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1150 RelaxedPrecision
OpDecorate %1152 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1159 RelaxedPrecision
OpDecorate %1161 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1187 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1192 RelaxedPrecision
OpDecorate %1196 RelaxedPrecision
OpDecorate %1198 RelaxedPrecision
OpDecorate %1200 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1202 RelaxedPrecision
OpDecorate %1204 RelaxedPrecision
OpDecorate %1207 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1211 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1221 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1231 RelaxedPrecision
OpDecorate %1233 RelaxedPrecision
OpDecorate %1234 RelaxedPrecision
OpDecorate %1235 RelaxedPrecision
OpDecorate %1236 RelaxedPrecision
OpDecorate %1238 RelaxedPrecision
OpDecorate %1239 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1244 RelaxedPrecision
OpDecorate %1246 RelaxedPrecision
OpDecorate %1248 RelaxedPrecision
OpDecorate %1249 RelaxedPrecision
OpDecorate %1250 RelaxedPrecision
OpDecorate %1252 RelaxedPrecision
OpDecorate %1255 RelaxedPrecision
OpDecorate %1257 RelaxedPrecision
OpDecorate %1259 RelaxedPrecision
OpDecorate %1261 RelaxedPrecision
OpDecorate %1263 RelaxedPrecision
OpDecorate %1267 RelaxedPrecision
OpDecorate %1269 RelaxedPrecision
OpDecorate %1272 RelaxedPrecision
OpDecorate %1274 RelaxedPrecision
OpDecorate %1276 RelaxedPrecision
OpDecorate %1279 RelaxedPrecision
OpDecorate %1281 RelaxedPrecision
OpDecorate %1282 RelaxedPrecision
OpDecorate %1283 RelaxedPrecision
OpDecorate %1284 RelaxedPrecision
OpDecorate %1286 RelaxedPrecision
OpDecorate %1287 RelaxedPrecision
OpDecorate %1288 RelaxedPrecision
OpDecorate %1292 RelaxedPrecision
OpDecorate %1294 RelaxedPrecision
OpDecorate %1296 RelaxedPrecision
OpDecorate %1297 RelaxedPrecision
OpDecorate %1298 RelaxedPrecision
OpDecorate %1300 RelaxedPrecision
OpDecorate %1307 RelaxedPrecision
OpDecorate %1309 RelaxedPrecision
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
%23 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%65 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%float_0 = OpConstant %float 0
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v3float = OpTypePointer Function %v3float
%450 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%458 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%467 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%550 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%578 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%580 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%685 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%722 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%1301 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1303 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%_blend_overlay_component = OpFunction %float None %23
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%35 = OpVariable %_ptr_Function_float Function
%29 = OpLoad %v2float %26
%30 = OpCompositeExtract %float %29 0
%31 = OpFMul %float %float_2 %30
%32 = OpLoad %v2float %26
%33 = OpCompositeExtract %float %32 1
%34 = OpFOrdLessThanEqual %bool %31 %33
OpSelectionMerge %39 None
OpBranchConditional %34 %37 %38
%37 = OpLabel
%40 = OpLoad %v2float %25
%41 = OpCompositeExtract %float %40 0
%42 = OpFMul %float %float_2 %41
%43 = OpLoad %v2float %26
%44 = OpCompositeExtract %float %43 0
%45 = OpFMul %float %42 %44
OpStore %35 %45
OpBranch %39
%38 = OpLabel
%46 = OpLoad %v2float %25
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %26
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v2float %26
%52 = OpCompositeExtract %float %51 1
%53 = OpLoad %v2float %26
%54 = OpCompositeExtract %float %53 0
%55 = OpFSub %float %52 %54
%56 = OpFMul %float %float_2 %55
%57 = OpLoad %v2float %25
%58 = OpCompositeExtract %float %57 1
%59 = OpLoad %v2float %25
%60 = OpCompositeExtract %float %59 0
%61 = OpFSub %float %58 %60
%62 = OpFMul %float %56 %61
%63 = OpFSub %float %50 %62
OpStore %35 %63
OpBranch %39
%39 = OpLabel
%64 = OpLoad %float %35
OpReturnValue %64
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %65
%67 = OpFunctionParameter %_ptr_Function_v4float
%68 = OpFunctionParameter %_ptr_Function_v4float
%69 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%73 = OpVariable %_ptr_Function_v2float Function
%76 = OpVariable %_ptr_Function_v2float Function
%80 = OpVariable %_ptr_Function_v2float Function
%83 = OpVariable %_ptr_Function_v2float Function
%87 = OpVariable %_ptr_Function_v2float Function
%90 = OpVariable %_ptr_Function_v2float Function
%71 = OpLoad %v4float %67
%72 = OpVectorShuffle %v2float %71 %71 0 3
OpStore %73 %72
%74 = OpLoad %v4float %68
%75 = OpVectorShuffle %v2float %74 %74 0 3
OpStore %76 %75
%77 = OpFunctionCall %float %_blend_overlay_component %73 %76
%78 = OpLoad %v4float %67
%79 = OpVectorShuffle %v2float %78 %78 1 3
OpStore %80 %79
%81 = OpLoad %v4float %68
%82 = OpVectorShuffle %v2float %81 %81 1 3
OpStore %83 %82
%84 = OpFunctionCall %float %_blend_overlay_component %80 %83
%85 = OpLoad %v4float %67
%86 = OpVectorShuffle %v2float %85 %85 2 3
OpStore %87 %86
%88 = OpLoad %v4float %68
%89 = OpVectorShuffle %v2float %88 %88 2 3
OpStore %90 %89
%91 = OpFunctionCall %float %_blend_overlay_component %87 %90
%92 = OpLoad %v4float %67
%93 = OpCompositeExtract %float %92 3
%95 = OpLoad %v4float %67
%96 = OpCompositeExtract %float %95 3
%97 = OpFSub %float %float_1 %96
%98 = OpLoad %v4float %68
%99 = OpCompositeExtract %float %98 3
%100 = OpFMul %float %97 %99
%101 = OpFAdd %float %93 %100
%102 = OpCompositeConstruct %v4float %77 %84 %91 %101
OpStore %result %102
%103 = OpLoad %v4float %result
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%106 = OpLoad %v4float %68
%107 = OpVectorShuffle %v3float %106 %106 0 1 2
%108 = OpLoad %v4float %67
%109 = OpCompositeExtract %float %108 3
%110 = OpFSub %float %float_1 %109
%111 = OpVectorTimesScalar %v3float %107 %110
%112 = OpLoad %v4float %67
%113 = OpVectorShuffle %v3float %112 %112 0 1 2
%114 = OpLoad %v4float %68
%115 = OpCompositeExtract %float %114 3
%116 = OpFSub %float %float_1 %115
%117 = OpVectorTimesScalar %v3float %113 %116
%118 = OpFAdd %v3float %111 %117
%119 = OpFAdd %v3float %104 %118
%120 = OpLoad %v4float %result
%121 = OpVectorShuffle %v4float %120 %119 4 5 6 3
OpStore %result %121
%122 = OpLoad %v4float %result
OpReturnValue %122
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %23
%123 = OpFunctionParameter %_ptr_Function_v2float
%124 = OpFunctionParameter %_ptr_Function_v2float
%125 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_3_guarded_divide = OpVariable %_ptr_Function_float Function
%_4_n = OpVariable %_ptr_Function_float Function
%126 = OpLoad %v2float %124
%127 = OpCompositeExtract %float %126 0
%129 = OpFOrdEqual %bool %127 %float_0
OpSelectionMerge %132 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpLoad %v2float %123
%134 = OpCompositeExtract %float %133 0
%135 = OpLoad %v2float %124
%136 = OpCompositeExtract %float %135 1
%137 = OpFSub %float %float_1 %136
%138 = OpFMul %float %134 %137
OpReturnValue %138
%131 = OpLabel
%140 = OpLoad %v2float %123
%141 = OpCompositeExtract %float %140 1
%142 = OpLoad %v2float %123
%143 = OpCompositeExtract %float %142 0
%144 = OpFSub %float %141 %143
OpStore %delta %144
%145 = OpLoad %float %delta
%146 = OpFOrdEqual %bool %145 %float_0
OpSelectionMerge %149 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%150 = OpLoad %v2float %123
%151 = OpCompositeExtract %float %150 1
%152 = OpLoad %v2float %124
%153 = OpCompositeExtract %float %152 1
%154 = OpFMul %float %151 %153
%155 = OpLoad %v2float %123
%156 = OpCompositeExtract %float %155 0
%157 = OpLoad %v2float %124
%158 = OpCompositeExtract %float %157 1
%159 = OpFSub %float %float_1 %158
%160 = OpFMul %float %156 %159
%161 = OpFAdd %float %154 %160
%162 = OpLoad %v2float %124
%163 = OpCompositeExtract %float %162 0
%164 = OpLoad %v2float %123
%165 = OpCompositeExtract %float %164 1
%166 = OpFSub %float %float_1 %165
%167 = OpFMul %float %163 %166
%168 = OpFAdd %float %161 %167
OpReturnValue %168
%148 = OpLabel
%171 = OpLoad %v2float %124
%172 = OpCompositeExtract %float %171 0
%173 = OpLoad %v2float %123
%174 = OpCompositeExtract %float %173 1
%175 = OpFMul %float %172 %174
OpStore %_4_n %175
%176 = OpLoad %float %_4_n
%177 = OpLoad %float %delta
%178 = OpFDiv %float %176 %177
OpStore %_3_guarded_divide %178
%180 = OpLoad %v2float %124
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %float %_3_guarded_divide
%179 = OpExtInst %float %1 FMin %181 %182
OpStore %delta %179
%183 = OpLoad %float %delta
%184 = OpLoad %v2float %123
%185 = OpCompositeExtract %float %184 1
%186 = OpFMul %float %183 %185
%187 = OpLoad %v2float %123
%188 = OpCompositeExtract %float %187 0
%189 = OpLoad %v2float %124
%190 = OpCompositeExtract %float %189 1
%191 = OpFSub %float %float_1 %190
%192 = OpFMul %float %188 %191
%193 = OpFAdd %float %186 %192
%194 = OpLoad %v2float %124
%195 = OpCompositeExtract %float %194 0
%196 = OpLoad %v2float %123
%197 = OpCompositeExtract %float %196 1
%198 = OpFSub %float %float_1 %197
%199 = OpFMul %float %195 %198
%200 = OpFAdd %float %193 %199
OpReturnValue %200
%149 = OpLabel
OpBranch %132
%132 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %23
%201 = OpFunctionParameter %_ptr_Function_v2float
%202 = OpFunctionParameter %_ptr_Function_v2float
%203 = OpLabel
%_5_guarded_divide = OpVariable %_ptr_Function_float Function
%_6_n = OpVariable %_ptr_Function_float Function
%delta_0 = OpVariable %_ptr_Function_float Function
%204 = OpLoad %v2float %202
%205 = OpCompositeExtract %float %204 1
%206 = OpLoad %v2float %202
%207 = OpCompositeExtract %float %206 0
%208 = OpFOrdEqual %bool %205 %207
OpSelectionMerge %211 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%212 = OpLoad %v2float %201
%213 = OpCompositeExtract %float %212 1
%214 = OpLoad %v2float %202
%215 = OpCompositeExtract %float %214 1
%216 = OpFMul %float %213 %215
%217 = OpLoad %v2float %201
%218 = OpCompositeExtract %float %217 0
%219 = OpLoad %v2float %202
%220 = OpCompositeExtract %float %219 1
%221 = OpFSub %float %float_1 %220
%222 = OpFMul %float %218 %221
%223 = OpFAdd %float %216 %222
%224 = OpLoad %v2float %202
%225 = OpCompositeExtract %float %224 0
%226 = OpLoad %v2float %201
%227 = OpCompositeExtract %float %226 1
%228 = OpFSub %float %float_1 %227
%229 = OpFMul %float %225 %228
%230 = OpFAdd %float %223 %229
OpReturnValue %230
%210 = OpLabel
%231 = OpLoad %v2float %201
%232 = OpCompositeExtract %float %231 0
%233 = OpFOrdEqual %bool %232 %float_0
OpSelectionMerge %236 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%237 = OpLoad %v2float %202
%238 = OpCompositeExtract %float %237 0
%239 = OpLoad %v2float %201
%240 = OpCompositeExtract %float %239 1
%241 = OpFSub %float %float_1 %240
%242 = OpFMul %float %238 %241
OpReturnValue %242
%235 = OpLabel
%245 = OpLoad %v2float %202
%246 = OpCompositeExtract %float %245 1
%247 = OpLoad %v2float %202
%248 = OpCompositeExtract %float %247 0
%249 = OpFSub %float %246 %248
%250 = OpLoad %v2float %201
%251 = OpCompositeExtract %float %250 1
%252 = OpFMul %float %249 %251
OpStore %_6_n %252
%253 = OpLoad %float %_6_n
%254 = OpLoad %v2float %201
%255 = OpCompositeExtract %float %254 0
%256 = OpFDiv %float %253 %255
OpStore %_5_guarded_divide %256
%259 = OpLoad %v2float %202
%260 = OpCompositeExtract %float %259 1
%261 = OpLoad %float %_5_guarded_divide
%262 = OpFSub %float %260 %261
%258 = OpExtInst %float %1 FMax %float_0 %262
OpStore %delta_0 %258
%263 = OpLoad %float %delta_0
%264 = OpLoad %v2float %201
%265 = OpCompositeExtract %float %264 1
%266 = OpFMul %float %263 %265
%267 = OpLoad %v2float %201
%268 = OpCompositeExtract %float %267 0
%269 = OpLoad %v2float %202
%270 = OpCompositeExtract %float %269 1
%271 = OpFSub %float %float_1 %270
%272 = OpFMul %float %268 %271
%273 = OpFAdd %float %266 %272
%274 = OpLoad %v2float %202
%275 = OpCompositeExtract %float %274 0
%276 = OpLoad %v2float %201
%277 = OpCompositeExtract %float %276 1
%278 = OpFSub %float %float_1 %277
%279 = OpFMul %float %275 %278
%280 = OpFAdd %float %273 %279
OpReturnValue %280
%236 = OpLabel
OpBranch %211
%211 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %23
%281 = OpFunctionParameter %_ptr_Function_v2float
%282 = OpFunctionParameter %_ptr_Function_v2float
%283 = OpLabel
%_7_guarded_divide = OpVariable %_ptr_Function_float Function
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_9_guarded_divide = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
%284 = OpLoad %v2float %281
%285 = OpCompositeExtract %float %284 0
%286 = OpFMul %float %float_2 %285
%287 = OpLoad %v2float %281
%288 = OpCompositeExtract %float %287 1
%289 = OpFOrdLessThanEqual %bool %286 %288
OpSelectionMerge %292 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%295 = OpLoad %v2float %282
%296 = OpCompositeExtract %float %295 0
%297 = OpLoad %v2float %282
%298 = OpCompositeExtract %float %297 0
%299 = OpFMul %float %296 %298
%300 = OpLoad %v2float %281
%301 = OpCompositeExtract %float %300 1
%302 = OpLoad %v2float %281
%303 = OpCompositeExtract %float %302 0
%304 = OpFMul %float %float_2 %303
%305 = OpFSub %float %301 %304
%306 = OpFMul %float %299 %305
OpStore %_8_n %306
%307 = OpLoad %float %_8_n
%308 = OpLoad %v2float %282
%309 = OpCompositeExtract %float %308 1
%310 = OpFDiv %float %307 %309
OpStore %_7_guarded_divide %310
%311 = OpLoad %float %_7_guarded_divide
%312 = OpLoad %v2float %282
%313 = OpCompositeExtract %float %312 1
%314 = OpFSub %float %float_1 %313
%315 = OpLoad %v2float %281
%316 = OpCompositeExtract %float %315 0
%317 = OpFMul %float %314 %316
%318 = OpFAdd %float %311 %317
%319 = OpLoad %v2float %282
%320 = OpCompositeExtract %float %319 0
%322 = OpLoad %v2float %281
%323 = OpCompositeExtract %float %322 1
%321 = OpFNegate %float %323
%324 = OpLoad %v2float %281
%325 = OpCompositeExtract %float %324 0
%326 = OpFMul %float %float_2 %325
%327 = OpFAdd %float %321 %326
%328 = OpFAdd %float %327 %float_1
%329 = OpFMul %float %320 %328
%330 = OpFAdd %float %318 %329
OpReturnValue %330
%291 = OpLabel
%332 = OpLoad %v2float %282
%333 = OpCompositeExtract %float %332 0
%334 = OpFMul %float %float_4 %333
%335 = OpLoad %v2float %282
%336 = OpCompositeExtract %float %335 1
%337 = OpFOrdLessThanEqual %bool %334 %336
OpSelectionMerge %340 None
OpBranchConditional %337 %338 %339
%338 = OpLabel
%342 = OpLoad %v2float %282
%343 = OpCompositeExtract %float %342 0
%344 = OpLoad %v2float %282
%345 = OpCompositeExtract %float %344 0
%346 = OpFMul %float %343 %345
OpStore %DSqd %346
%348 = OpLoad %float %DSqd
%349 = OpLoad %v2float %282
%350 = OpCompositeExtract %float %349 0
%351 = OpFMul %float %348 %350
OpStore %DCub %351
%353 = OpLoad %v2float %282
%354 = OpCompositeExtract %float %353 1
%355 = OpLoad %v2float %282
%356 = OpCompositeExtract %float %355 1
%357 = OpFMul %float %354 %356
OpStore %DaSqd %357
%359 = OpLoad %float %DaSqd
%360 = OpLoad %v2float %282
%361 = OpCompositeExtract %float %360 1
%362 = OpFMul %float %359 %361
OpStore %DaCub %362
%365 = OpLoad %float %DaSqd
%366 = OpLoad %v2float %281
%367 = OpCompositeExtract %float %366 0
%368 = OpLoad %v2float %282
%369 = OpCompositeExtract %float %368 0
%371 = OpLoad %v2float %281
%372 = OpCompositeExtract %float %371 1
%373 = OpFMul %float %float_3 %372
%375 = OpLoad %v2float %281
%376 = OpCompositeExtract %float %375 0
%377 = OpFMul %float %float_6 %376
%378 = OpFSub %float %373 %377
%379 = OpFSub %float %378 %float_1
%380 = OpFMul %float %369 %379
%381 = OpFSub %float %367 %380
%382 = OpFMul %float %365 %381
%384 = OpLoad %v2float %282
%385 = OpCompositeExtract %float %384 1
%386 = OpFMul %float %float_12 %385
%387 = OpLoad %float %DSqd
%388 = OpFMul %float %386 %387
%389 = OpLoad %v2float %281
%390 = OpCompositeExtract %float %389 1
%391 = OpLoad %v2float %281
%392 = OpCompositeExtract %float %391 0
%393 = OpFMul %float %float_2 %392
%394 = OpFSub %float %390 %393
%395 = OpFMul %float %388 %394
%396 = OpFAdd %float %382 %395
%398 = OpLoad %float %DCub
%399 = OpFMul %float %float_16 %398
%400 = OpLoad %v2float %281
%401 = OpCompositeExtract %float %400 1
%402 = OpLoad %v2float %281
%403 = OpCompositeExtract %float %402 0
%404 = OpFMul %float %float_2 %403
%405 = OpFSub %float %401 %404
%406 = OpFMul %float %399 %405
%407 = OpFSub %float %396 %406
%408 = OpLoad %float %DaCub
%409 = OpLoad %v2float %281
%410 = OpCompositeExtract %float %409 0
%411 = OpFMul %float %408 %410
%412 = OpFSub %float %407 %411
OpStore %_10_n %412
%413 = OpLoad %float %_10_n
%414 = OpLoad %float %DaSqd
%415 = OpFDiv %float %413 %414
OpStore %_9_guarded_divide %415
%416 = OpLoad %float %_9_guarded_divide
OpReturnValue %416
%339 = OpLabel
%417 = OpLoad %v2float %282
%418 = OpCompositeExtract %float %417 0
%419 = OpLoad %v2float %281
%420 = OpCompositeExtract %float %419 1
%421 = OpLoad %v2float %281
%422 = OpCompositeExtract %float %421 0
%423 = OpFMul %float %float_2 %422
%424 = OpFSub %float %420 %423
%425 = OpFAdd %float %424 %float_1
%426 = OpFMul %float %418 %425
%427 = OpLoad %v2float %281
%428 = OpCompositeExtract %float %427 0
%429 = OpFAdd %float %426 %428
%431 = OpLoad %v2float %282
%432 = OpCompositeExtract %float %431 1
%433 = OpLoad %v2float %282
%434 = OpCompositeExtract %float %433 0
%435 = OpFMul %float %432 %434
%430 = OpExtInst %float %1 Sqrt %435
%436 = OpLoad %v2float %281
%437 = OpCompositeExtract %float %436 1
%438 = OpLoad %v2float %281
%439 = OpCompositeExtract %float %438 0
%440 = OpFMul %float %float_2 %439
%441 = OpFSub %float %437 %440
%442 = OpFMul %float %430 %441
%443 = OpFSub %float %429 %442
%444 = OpLoad %v2float %282
%445 = OpCompositeExtract %float %444 1
%446 = OpLoad %v2float %281
%447 = OpCompositeExtract %float %446 0
%448 = OpFMul %float %445 %447
%449 = OpFSub %float %443 %448
OpReturnValue %449
%340 = OpLabel
OpBranch %292
%292 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %450
%452 = OpFunctionParameter %_ptr_Function_v3float
%453 = OpFunctionParameter %_ptr_Function_float
%454 = OpFunctionParameter %_ptr_Function_v3float
%455 = OpLabel
%_11_blend_color_luminance = OpVariable %_ptr_Function_float Function
%lum = OpVariable %_ptr_Function_float Function
%_12_blend_color_luminance = OpVariable %_ptr_Function_float Function
%result_0 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%528 = OpVariable %_ptr_Function_v3float Function
%462 = OpLoad %v3float %454
%457 = OpDot %float %458 %462
OpStore %_11_blend_color_luminance %457
%464 = OpLoad %float %_11_blend_color_luminance
OpStore %lum %464
%468 = OpLoad %v3float %452
%466 = OpDot %float %467 %468
OpStore %_12_blend_color_luminance %466
%470 = OpLoad %float %lum
%471 = OpLoad %float %_12_blend_color_luminance
%472 = OpFSub %float %470 %471
%473 = OpLoad %v3float %452
%474 = OpCompositeConstruct %v3float %472 %472 %472
%475 = OpFAdd %v3float %474 %473
OpStore %result_0 %475
%479 = OpLoad %v3float %result_0
%480 = OpCompositeExtract %float %479 0
%481 = OpLoad %v3float %result_0
%482 = OpCompositeExtract %float %481 1
%478 = OpExtInst %float %1 FMin %480 %482
%483 = OpLoad %v3float %result_0
%484 = OpCompositeExtract %float %483 2
%477 = OpExtInst %float %1 FMin %478 %484
OpStore %minComp %477
%488 = OpLoad %v3float %result_0
%489 = OpCompositeExtract %float %488 0
%490 = OpLoad %v3float %result_0
%491 = OpCompositeExtract %float %490 1
%487 = OpExtInst %float %1 FMax %489 %491
%492 = OpLoad %v3float %result_0
%493 = OpCompositeExtract %float %492 2
%486 = OpExtInst %float %1 FMax %487 %493
OpStore %maxComp %486
%495 = OpLoad %float %minComp
%496 = OpFOrdLessThan %bool %495 %float_0
OpSelectionMerge %498 None
OpBranchConditional %496 %497 %498
%497 = OpLabel
%499 = OpLoad %float %lum
%500 = OpLoad %float %minComp
%501 = OpFOrdNotEqual %bool %499 %500
OpBranch %498
%498 = OpLabel
%502 = OpPhi %bool %false %455 %501 %497
OpSelectionMerge %504 None
OpBranchConditional %502 %503 %504
%503 = OpLabel
%505 = OpLoad %float %lum
%506 = OpLoad %v3float %result_0
%507 = OpLoad %float %lum
%508 = OpCompositeConstruct %v3float %507 %507 %507
%509 = OpFSub %v3float %506 %508
%510 = OpLoad %float %lum
%511 = OpVectorTimesScalar %v3float %509 %510
%512 = OpLoad %float %lum
%513 = OpLoad %float %minComp
%514 = OpFSub %float %512 %513
%515 = OpFDiv %float %float_1 %514
%516 = OpVectorTimesScalar %v3float %511 %515
%517 = OpCompositeConstruct %v3float %505 %505 %505
%518 = OpFAdd %v3float %517 %516
OpStore %result_0 %518
OpBranch %504
%504 = OpLabel
%519 = OpLoad %float %maxComp
%520 = OpLoad %float %453
%521 = OpFOrdGreaterThan %bool %519 %520
OpSelectionMerge %523 None
OpBranchConditional %521 %522 %523
%522 = OpLabel
%524 = OpLoad %float %maxComp
%525 = OpLoad %float %lum
%526 = OpFOrdNotEqual %bool %524 %525
OpBranch %523
%523 = OpLabel
%527 = OpPhi %bool %false %504 %526 %522
OpSelectionMerge %531 None
OpBranchConditional %527 %529 %530
%529 = OpLabel
%532 = OpLoad %float %lum
%533 = OpLoad %v3float %result_0
%534 = OpLoad %float %lum
%535 = OpCompositeConstruct %v3float %534 %534 %534
%536 = OpFSub %v3float %533 %535
%537 = OpLoad %float %453
%538 = OpLoad %float %lum
%539 = OpFSub %float %537 %538
%540 = OpVectorTimesScalar %v3float %536 %539
%541 = OpLoad %float %maxComp
%542 = OpLoad %float %lum
%543 = OpFSub %float %541 %542
%544 = OpFDiv %float %float_1 %543
%545 = OpVectorTimesScalar %v3float %540 %544
%546 = OpCompositeConstruct %v3float %532 %532 %532
%547 = OpFAdd %v3float %546 %545
OpStore %528 %547
OpBranch %531
%530 = OpLabel
%548 = OpLoad %v3float %result_0
OpStore %528 %548
OpBranch %531
%531 = OpLabel
%549 = OpLoad %v3float %528
OpReturnValue %549
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %550
%551 = OpFunctionParameter %_ptr_Function_v3float
%552 = OpFunctionParameter %_ptr_Function_float
%553 = OpLabel
%559 = OpVariable %_ptr_Function_v3float Function
%554 = OpLoad %v3float %551
%555 = OpCompositeExtract %float %554 0
%556 = OpLoad %v3float %551
%557 = OpCompositeExtract %float %556 2
%558 = OpFOrdLessThan %bool %555 %557
OpSelectionMerge %562 None
OpBranchConditional %558 %560 %561
%560 = OpLabel
%563 = OpLoad %float %552
%564 = OpLoad %v3float %551
%565 = OpCompositeExtract %float %564 1
%566 = OpLoad %v3float %551
%567 = OpCompositeExtract %float %566 0
%568 = OpFSub %float %565 %567
%569 = OpFMul %float %563 %568
%570 = OpLoad %v3float %551
%571 = OpCompositeExtract %float %570 2
%572 = OpLoad %v3float %551
%573 = OpCompositeExtract %float %572 0
%574 = OpFSub %float %571 %573
%575 = OpFDiv %float %569 %574
%576 = OpLoad %float %552
%577 = OpCompositeConstruct %v3float %float_0 %575 %576
OpStore %559 %577
OpBranch %562
%561 = OpLabel
OpStore %559 %578
OpBranch %562
%562 = OpLabel
%579 = OpLoad %v3float %559
OpReturnValue %579
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %580
%581 = OpFunctionParameter %_ptr_Function_v3float
%582 = OpFunctionParameter %_ptr_Function_v3float
%583 = OpLabel
%_13_blend_color_saturation = OpVariable %_ptr_Function_float Function
%sat = OpVariable %_ptr_Function_float Function
%621 = OpVariable %_ptr_Function_v3float Function
%623 = OpVariable %_ptr_Function_float Function
%635 = OpVariable %_ptr_Function_v3float Function
%637 = OpVariable %_ptr_Function_float Function
%642 = OpVariable %_ptr_Function_v3float Function
%644 = OpVariable %_ptr_Function_float Function
%657 = OpVariable %_ptr_Function_v3float Function
%659 = OpVariable %_ptr_Function_float Function
%672 = OpVariable %_ptr_Function_v3float Function
%674 = OpVariable %_ptr_Function_float Function
%679 = OpVariable %_ptr_Function_v3float Function
%681 = OpVariable %_ptr_Function_float Function
%587 = OpLoad %v3float %582
%588 = OpCompositeExtract %float %587 0
%589 = OpLoad %v3float %582
%590 = OpCompositeExtract %float %589 1
%586 = OpExtInst %float %1 FMax %588 %590
%591 = OpLoad %v3float %582
%592 = OpCompositeExtract %float %591 2
%585 = OpExtInst %float %1 FMax %586 %592
%595 = OpLoad %v3float %582
%596 = OpCompositeExtract %float %595 0
%597 = OpLoad %v3float %582
%598 = OpCompositeExtract %float %597 1
%594 = OpExtInst %float %1 FMin %596 %598
%599 = OpLoad %v3float %582
%600 = OpCompositeExtract %float %599 2
%593 = OpExtInst %float %1 FMin %594 %600
%601 = OpFSub %float %585 %593
OpStore %_13_blend_color_saturation %601
%603 = OpLoad %float %_13_blend_color_saturation
OpStore %sat %603
%604 = OpLoad %v3float %581
%605 = OpCompositeExtract %float %604 0
%606 = OpLoad %v3float %581
%607 = OpCompositeExtract %float %606 1
%608 = OpFOrdLessThanEqual %bool %605 %607
OpSelectionMerge %611 None
OpBranchConditional %608 %609 %610
%609 = OpLabel
%612 = OpLoad %v3float %581
%613 = OpCompositeExtract %float %612 1
%614 = OpLoad %v3float %581
%615 = OpCompositeExtract %float %614 2
%616 = OpFOrdLessThanEqual %bool %613 %615
OpSelectionMerge %619 None
OpBranchConditional %616 %617 %618
%617 = OpLabel
%620 = OpLoad %v3float %581
OpStore %621 %620
%622 = OpLoad %float %sat
OpStore %623 %622
%624 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %621 %623
OpReturnValue %624
%618 = OpLabel
%625 = OpLoad %v3float %581
%626 = OpCompositeExtract %float %625 0
%627 = OpLoad %v3float %581
%628 = OpCompositeExtract %float %627 2
%629 = OpFOrdLessThanEqual %bool %626 %628
OpSelectionMerge %632 None
OpBranchConditional %629 %630 %631
%630 = OpLabel
%633 = OpLoad %v3float %581
%634 = OpVectorShuffle %v3float %633 %633 0 2 1
OpStore %635 %634
%636 = OpLoad %float %sat
OpStore %637 %636
%638 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %635 %637
%639 = OpVectorShuffle %v3float %638 %638 0 2 1
OpReturnValue %639
%631 = OpLabel
%640 = OpLoad %v3float %581
%641 = OpVectorShuffle %v3float %640 %640 2 0 1
OpStore %642 %641
%643 = OpLoad %float %sat
OpStore %644 %643
%645 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %642 %644
%646 = OpVectorShuffle %v3float %645 %645 1 2 0
OpReturnValue %646
%632 = OpLabel
OpBranch %619
%619 = OpLabel
OpBranch %611
%610 = OpLabel
%647 = OpLoad %v3float %581
%648 = OpCompositeExtract %float %647 0
%649 = OpLoad %v3float %581
%650 = OpCompositeExtract %float %649 2
%651 = OpFOrdLessThanEqual %bool %648 %650
OpSelectionMerge %654 None
OpBranchConditional %651 %652 %653
%652 = OpLabel
%655 = OpLoad %v3float %581
%656 = OpVectorShuffle %v3float %655 %655 1 0 2
OpStore %657 %656
%658 = OpLoad %float %sat
OpStore %659 %658
%660 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %657 %659
%661 = OpVectorShuffle %v3float %660 %660 1 0 2
OpReturnValue %661
%653 = OpLabel
%662 = OpLoad %v3float %581
%663 = OpCompositeExtract %float %662 1
%664 = OpLoad %v3float %581
%665 = OpCompositeExtract %float %664 2
%666 = OpFOrdLessThanEqual %bool %663 %665
OpSelectionMerge %669 None
OpBranchConditional %666 %667 %668
%667 = OpLabel
%670 = OpLoad %v3float %581
%671 = OpVectorShuffle %v3float %670 %670 1 2 0
OpStore %672 %671
%673 = OpLoad %float %sat
OpStore %674 %673
%675 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %672 %674
%676 = OpVectorShuffle %v3float %675 %675 2 0 1
OpReturnValue %676
%668 = OpLabel
%677 = OpLoad %v3float %581
%678 = OpVectorShuffle %v3float %677 %677 2 1 0
OpStore %679 %678
%680 = OpLoad %float %sat
OpStore %681 %680
%682 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %679 %681
%683 = OpVectorShuffle %v3float %682 %682 2 1 0
OpReturnValue %683
%669 = OpLabel
OpBranch %654
%654 = OpLabel
OpBranch %611
%611 = OpLabel
OpUnreachable
OpFunctionEnd
%blend = OpFunction %v4float None %685
%687 = OpFunctionParameter %_ptr_Function_int
%688 = OpFunctionParameter %_ptr_Function_v4float
%689 = OpFunctionParameter %_ptr_Function_v4float
%690 = OpLabel
%_15_blend_src = OpVariable %_ptr_Function_v4float Function
%_16_blend_dst = OpVariable %_ptr_Function_v4float Function
%_17_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_18_blend_dst_over = OpVariable %_ptr_Function_v4float Function
%_19_blend_src_in = OpVariable %_ptr_Function_v4float Function
%_20_blend_dst_in = OpVariable %_ptr_Function_v4float Function
%_21_blend_src_in = OpVariable %_ptr_Function_v4float Function
%_22_blend_src_out = OpVariable %_ptr_Function_v4float Function
%_23_blend_dst_out = OpVariable %_ptr_Function_v4float Function
%_24_blend_src_atop = OpVariable %_ptr_Function_v4float Function
%_25_blend_dst_atop = OpVariable %_ptr_Function_v4float Function
%_26_blend_xor = OpVariable %_ptr_Function_v4float Function
%_27_blend_plus = OpVariable %_ptr_Function_v4float Function
%_28_blend_modulate = OpVariable %_ptr_Function_v4float Function
%_29_blend_screen = OpVariable %_ptr_Function_v4float Function
%834 = OpVariable %_ptr_Function_v4float Function
%836 = OpVariable %_ptr_Function_v4float Function
%_30_blend_darken = OpVariable %_ptr_Function_v4float Function
%_31_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_32_result = OpVariable %_ptr_Function_v4float Function
%_33_blend_lighten = OpVariable %_ptr_Function_v4float Function
%_34_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_35_result = OpVariable %_ptr_Function_v4float Function
%_36_blend_color_dodge = OpVariable %_ptr_Function_v4float Function
%895 = OpVariable %_ptr_Function_v2float Function
%898 = OpVariable %_ptr_Function_v2float Function
%902 = OpVariable %_ptr_Function_v2float Function
%905 = OpVariable %_ptr_Function_v2float Function
%909 = OpVariable %_ptr_Function_v2float Function
%912 = OpVariable %_ptr_Function_v2float Function
%_37_blend_color_burn = OpVariable %_ptr_Function_v4float Function
%928 = OpVariable %_ptr_Function_v2float Function
%931 = OpVariable %_ptr_Function_v2float Function
%935 = OpVariable %_ptr_Function_v2float Function
%938 = OpVariable %_ptr_Function_v2float Function
%942 = OpVariable %_ptr_Function_v2float Function
%945 = OpVariable %_ptr_Function_v2float Function
%_38_blend_hard_light = OpVariable %_ptr_Function_v4float Function
%960 = OpVariable %_ptr_Function_v4float Function
%962 = OpVariable %_ptr_Function_v4float Function
%_39_blend_soft_light = OpVariable %_ptr_Function_v4float Function
%969 = OpVariable %_ptr_Function_v4float Function
%976 = OpVariable %_ptr_Function_v2float Function
%979 = OpVariable %_ptr_Function_v2float Function
%983 = OpVariable %_ptr_Function_v2float Function
%986 = OpVariable %_ptr_Function_v2float Function
%990 = OpVariable %_ptr_Function_v2float Function
%993 = OpVariable %_ptr_Function_v2float Function
%_40_blend_difference = OpVariable %_ptr_Function_v4float Function
%_41_blend_exclusion = OpVariable %_ptr_Function_v4float Function
%_42_blend_multiply = OpVariable %_ptr_Function_v4float Function
%_43_blend_hue = OpVariable %_ptr_Function_v4float Function
%_44_alpha = OpVariable %_ptr_Function_float Function
%_45_sda = OpVariable %_ptr_Function_v3float Function
%_46_dsa = OpVariable %_ptr_Function_v3float Function
%1121 = OpVariable %_ptr_Function_v3float Function
%1123 = OpVariable %_ptr_Function_v3float Function
%1125 = OpVariable %_ptr_Function_v3float Function
%1127 = OpVariable %_ptr_Function_float Function
%1129 = OpVariable %_ptr_Function_v3float Function
%_47_blend_saturation = OpVariable %_ptr_Function_v4float Function
%_48_alpha = OpVariable %_ptr_Function_float Function
%_49_sda = OpVariable %_ptr_Function_v3float Function
%_50_dsa = OpVariable %_ptr_Function_v3float Function
%1173 = OpVariable %_ptr_Function_v3float Function
%1175 = OpVariable %_ptr_Function_v3float Function
%1177 = OpVariable %_ptr_Function_v3float Function
%1179 = OpVariable %_ptr_Function_float Function
%1181 = OpVariable %_ptr_Function_v3float Function
%_51_blend_color = OpVariable %_ptr_Function_v4float Function
%_52_alpha = OpVariable %_ptr_Function_float Function
%_53_sda = OpVariable %_ptr_Function_v3float Function
%_54_dsa = OpVariable %_ptr_Function_v3float Function
%1225 = OpVariable %_ptr_Function_v3float Function
%1227 = OpVariable %_ptr_Function_float Function
%1229 = OpVariable %_ptr_Function_v3float Function
%_55_blend_luminosity = OpVariable %_ptr_Function_v4float Function
%_56_alpha = OpVariable %_ptr_Function_float Function
%_57_sda = OpVariable %_ptr_Function_v3float Function
%_58_dsa = OpVariable %_ptr_Function_v3float Function
%1273 = OpVariable %_ptr_Function_v3float Function
%1275 = OpVariable %_ptr_Function_float Function
%1277 = OpVariable %_ptr_Function_v3float Function
%691 = OpLoad %int %687
OpSelectionMerge %692 None
OpSwitch %691 %692 0 %693 1 %694 2 %695 3 %696 4 %697 5 %698 6 %699 7 %700 8 %701 9 %702 10 %703 11 %704 12 %705 13 %706 14 %707 15 %708 16 %709 17 %710 18 %711 19 %712 20 %713 21 %714 22 %715 23 %716 24 %717 25 %718 26 %719 27 %720 28 %721
%693 = OpLabel
OpReturnValue %722
%694 = OpLabel
%724 = OpLoad %v4float %688
OpStore %_15_blend_src %724
%725 = OpLoad %v4float %_15_blend_src
OpReturnValue %725
%695 = OpLabel
%727 = OpLoad %v4float %689
OpStore %_16_blend_dst %727
%728 = OpLoad %v4float %_16_blend_dst
OpReturnValue %728
%696 = OpLabel
%730 = OpLoad %v4float %688
%731 = OpLoad %v4float %688
%732 = OpCompositeExtract %float %731 3
%733 = OpFSub %float %float_1 %732
%734 = OpLoad %v4float %689
%735 = OpVectorTimesScalar %v4float %734 %733
%736 = OpFAdd %v4float %730 %735
OpStore %_17_blend_src_over %736
%737 = OpLoad %v4float %_17_blend_src_over
OpReturnValue %737
%697 = OpLabel
%739 = OpLoad %v4float %689
%740 = OpCompositeExtract %float %739 3
%741 = OpFSub %float %float_1 %740
%742 = OpLoad %v4float %688
%743 = OpVectorTimesScalar %v4float %742 %741
%744 = OpLoad %v4float %689
%745 = OpFAdd %v4float %743 %744
OpStore %_18_blend_dst_over %745
%746 = OpLoad %v4float %_18_blend_dst_over
OpReturnValue %746
%698 = OpLabel
%748 = OpLoad %v4float %688
%749 = OpLoad %v4float %689
%750 = OpCompositeExtract %float %749 3
%751 = OpVectorTimesScalar %v4float %748 %750
OpStore %_19_blend_src_in %751
%752 = OpLoad %v4float %_19_blend_src_in
OpReturnValue %752
%699 = OpLabel
%755 = OpLoad %v4float %689
%756 = OpLoad %v4float %688
%757 = OpCompositeExtract %float %756 3
%758 = OpVectorTimesScalar %v4float %755 %757
OpStore %_21_blend_src_in %758
%759 = OpLoad %v4float %_21_blend_src_in
OpStore %_20_blend_dst_in %759
%760 = OpLoad %v4float %_20_blend_dst_in
OpReturnValue %760
%700 = OpLabel
%762 = OpLoad %v4float %689
%763 = OpCompositeExtract %float %762 3
%764 = OpFSub %float %float_1 %763
%765 = OpLoad %v4float %688
%766 = OpVectorTimesScalar %v4float %765 %764
OpStore %_22_blend_src_out %766
%767 = OpLoad %v4float %_22_blend_src_out
OpReturnValue %767
%701 = OpLabel
%769 = OpLoad %v4float %688
%770 = OpCompositeExtract %float %769 3
%771 = OpFSub %float %float_1 %770
%772 = OpLoad %v4float %689
%773 = OpVectorTimesScalar %v4float %772 %771
OpStore %_23_blend_dst_out %773
%774 = OpLoad %v4float %_23_blend_dst_out
OpReturnValue %774
%702 = OpLabel
%776 = OpLoad %v4float %689
%777 = OpCompositeExtract %float %776 3
%778 = OpLoad %v4float %688
%779 = OpVectorTimesScalar %v4float %778 %777
%780 = OpLoad %v4float %688
%781 = OpCompositeExtract %float %780 3
%782 = OpFSub %float %float_1 %781
%783 = OpLoad %v4float %689
%784 = OpVectorTimesScalar %v4float %783 %782
%785 = OpFAdd %v4float %779 %784
OpStore %_24_blend_src_atop %785
%786 = OpLoad %v4float %_24_blend_src_atop
OpReturnValue %786
%703 = OpLabel
%788 = OpLoad %v4float %689
%789 = OpCompositeExtract %float %788 3
%790 = OpFSub %float %float_1 %789
%791 = OpLoad %v4float %688
%792 = OpVectorTimesScalar %v4float %791 %790
%793 = OpLoad %v4float %688
%794 = OpCompositeExtract %float %793 3
%795 = OpLoad %v4float %689
%796 = OpVectorTimesScalar %v4float %795 %794
%797 = OpFAdd %v4float %792 %796
OpStore %_25_blend_dst_atop %797
%798 = OpLoad %v4float %_25_blend_dst_atop
OpReturnValue %798
%704 = OpLabel
%800 = OpLoad %v4float %689
%801 = OpCompositeExtract %float %800 3
%802 = OpFSub %float %float_1 %801
%803 = OpLoad %v4float %688
%804 = OpVectorTimesScalar %v4float %803 %802
%805 = OpLoad %v4float %688
%806 = OpCompositeExtract %float %805 3
%807 = OpFSub %float %float_1 %806
%808 = OpLoad %v4float %689
%809 = OpVectorTimesScalar %v4float %808 %807
%810 = OpFAdd %v4float %804 %809
OpStore %_26_blend_xor %810
%811 = OpLoad %v4float %_26_blend_xor
OpReturnValue %811
%705 = OpLabel
%814 = OpLoad %v4float %688
%815 = OpLoad %v4float %689
%816 = OpFAdd %v4float %814 %815
%817 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%813 = OpExtInst %v4float %1 FMin %816 %817
OpStore %_27_blend_plus %813
%818 = OpLoad %v4float %_27_blend_plus
OpReturnValue %818
%706 = OpLabel
%820 = OpLoad %v4float %688
%821 = OpLoad %v4float %689
%822 = OpFMul %v4float %820 %821
OpStore %_28_blend_modulate %822
%823 = OpLoad %v4float %_28_blend_modulate
OpReturnValue %823
%707 = OpLabel
%825 = OpLoad %v4float %688
%826 = OpLoad %v4float %688
%827 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%828 = OpFSub %v4float %827 %826
%829 = OpLoad %v4float %689
%830 = OpFMul %v4float %828 %829
%831 = OpFAdd %v4float %825 %830
OpStore %_29_blend_screen %831
%832 = OpLoad %v4float %_29_blend_screen
OpReturnValue %832
%708 = OpLabel
%833 = OpLoad %v4float %688
OpStore %834 %833
%835 = OpLoad %v4float %689
OpStore %836 %835
%837 = OpFunctionCall %v4float %blend_overlay %834 %836
OpReturnValue %837
%709 = OpLabel
%840 = OpLoad %v4float %688
%841 = OpLoad %v4float %688
%842 = OpCompositeExtract %float %841 3
%843 = OpFSub %float %float_1 %842
%844 = OpLoad %v4float %689
%845 = OpVectorTimesScalar %v4float %844 %843
%846 = OpFAdd %v4float %840 %845
OpStore %_31_blend_src_over %846
%848 = OpLoad %v4float %_31_blend_src_over
OpStore %_32_result %848
%850 = OpLoad %v4float %_32_result
%851 = OpVectorShuffle %v3float %850 %850 0 1 2
%852 = OpLoad %v4float %689
%853 = OpCompositeExtract %float %852 3
%854 = OpFSub %float %float_1 %853
%855 = OpLoad %v4float %688
%856 = OpVectorShuffle %v3float %855 %855 0 1 2
%857 = OpVectorTimesScalar %v3float %856 %854
%858 = OpLoad %v4float %689
%859 = OpVectorShuffle %v3float %858 %858 0 1 2
%860 = OpFAdd %v3float %857 %859
%849 = OpExtInst %v3float %1 FMin %851 %860
%861 = OpLoad %v4float %_32_result
%862 = OpVectorShuffle %v4float %861 %849 4 5 6 3
OpStore %_32_result %862
%863 = OpLoad %v4float %_32_result
OpStore %_30_blend_darken %863
%864 = OpLoad %v4float %_30_blend_darken
OpReturnValue %864
%710 = OpLabel
%867 = OpLoad %v4float %688
%868 = OpLoad %v4float %688
%869 = OpCompositeExtract %float %868 3
%870 = OpFSub %float %float_1 %869
%871 = OpLoad %v4float %689
%872 = OpVectorTimesScalar %v4float %871 %870
%873 = OpFAdd %v4float %867 %872
OpStore %_34_blend_src_over %873
%875 = OpLoad %v4float %_34_blend_src_over
OpStore %_35_result %875
%877 = OpLoad %v4float %_35_result
%878 = OpVectorShuffle %v3float %877 %877 0 1 2
%879 = OpLoad %v4float %689
%880 = OpCompositeExtract %float %879 3
%881 = OpFSub %float %float_1 %880
%882 = OpLoad %v4float %688
%883 = OpVectorShuffle %v3float %882 %882 0 1 2
%884 = OpVectorTimesScalar %v3float %883 %881
%885 = OpLoad %v4float %689
%886 = OpVectorShuffle %v3float %885 %885 0 1 2
%887 = OpFAdd %v3float %884 %886
%876 = OpExtInst %v3float %1 FMax %878 %887
%888 = OpLoad %v4float %_35_result
%889 = OpVectorShuffle %v4float %888 %876 4 5 6 3
OpStore %_35_result %889
%890 = OpLoad %v4float %_35_result
OpStore %_33_blend_lighten %890
%891 = OpLoad %v4float %_33_blend_lighten
OpReturnValue %891
%711 = OpLabel
%893 = OpLoad %v4float %688
%894 = OpVectorShuffle %v2float %893 %893 0 3
OpStore %895 %894
%896 = OpLoad %v4float %689
%897 = OpVectorShuffle %v2float %896 %896 0 3
OpStore %898 %897
%899 = OpFunctionCall %float %_color_dodge_component %895 %898
%900 = OpLoad %v4float %688
%901 = OpVectorShuffle %v2float %900 %900 1 3
OpStore %902 %901
%903 = OpLoad %v4float %689
%904 = OpVectorShuffle %v2float %903 %903 1 3
OpStore %905 %904
%906 = OpFunctionCall %float %_color_dodge_component %902 %905
%907 = OpLoad %v4float %688
%908 = OpVectorShuffle %v2float %907 %907 2 3
OpStore %909 %908
%910 = OpLoad %v4float %689
%911 = OpVectorShuffle %v2float %910 %910 2 3
OpStore %912 %911
%913 = OpFunctionCall %float %_color_dodge_component %909 %912
%914 = OpLoad %v4float %688
%915 = OpCompositeExtract %float %914 3
%916 = OpLoad %v4float %688
%917 = OpCompositeExtract %float %916 3
%918 = OpFSub %float %float_1 %917
%919 = OpLoad %v4float %689
%920 = OpCompositeExtract %float %919 3
%921 = OpFMul %float %918 %920
%922 = OpFAdd %float %915 %921
%923 = OpCompositeConstruct %v4float %899 %906 %913 %922
OpStore %_36_blend_color_dodge %923
%924 = OpLoad %v4float %_36_blend_color_dodge
OpReturnValue %924
%712 = OpLabel
%926 = OpLoad %v4float %688
%927 = OpVectorShuffle %v2float %926 %926 0 3
OpStore %928 %927
%929 = OpLoad %v4float %689
%930 = OpVectorShuffle %v2float %929 %929 0 3
OpStore %931 %930
%932 = OpFunctionCall %float %_color_burn_component %928 %931
%933 = OpLoad %v4float %688
%934 = OpVectorShuffle %v2float %933 %933 1 3
OpStore %935 %934
%936 = OpLoad %v4float %689
%937 = OpVectorShuffle %v2float %936 %936 1 3
OpStore %938 %937
%939 = OpFunctionCall %float %_color_burn_component %935 %938
%940 = OpLoad %v4float %688
%941 = OpVectorShuffle %v2float %940 %940 2 3
OpStore %942 %941
%943 = OpLoad %v4float %689
%944 = OpVectorShuffle %v2float %943 %943 2 3
OpStore %945 %944
%946 = OpFunctionCall %float %_color_burn_component %942 %945
%947 = OpLoad %v4float %688
%948 = OpCompositeExtract %float %947 3
%949 = OpLoad %v4float %688
%950 = OpCompositeExtract %float %949 3
%951 = OpFSub %float %float_1 %950
%952 = OpLoad %v4float %689
%953 = OpCompositeExtract %float %952 3
%954 = OpFMul %float %951 %953
%955 = OpFAdd %float %948 %954
%956 = OpCompositeConstruct %v4float %932 %939 %946 %955
OpStore %_37_blend_color_burn %956
%957 = OpLoad %v4float %_37_blend_color_burn
OpReturnValue %957
%713 = OpLabel
%959 = OpLoad %v4float %689
OpStore %960 %959
%961 = OpLoad %v4float %688
OpStore %962 %961
%963 = OpFunctionCall %v4float %blend_overlay %960 %962
OpStore %_38_blend_hard_light %963
%964 = OpLoad %v4float %_38_blend_hard_light
OpReturnValue %964
%714 = OpLabel
%966 = OpLoad %v4float %689
%967 = OpCompositeExtract %float %966 3
%968 = OpFOrdEqual %bool %967 %float_0
OpSelectionMerge %972 None
OpBranchConditional %968 %970 %971
%970 = OpLabel
%973 = OpLoad %v4float %688
OpStore %969 %973
OpBranch %972
%971 = OpLabel
%974 = OpLoad %v4float %688
%975 = OpVectorShuffle %v2float %974 %974 0 3
OpStore %976 %975
%977 = OpLoad %v4float %689
%978 = OpVectorShuffle %v2float %977 %977 0 3
OpStore %979 %978
%980 = OpFunctionCall %float %_soft_light_component %976 %979
%981 = OpLoad %v4float %688
%982 = OpVectorShuffle %v2float %981 %981 1 3
OpStore %983 %982
%984 = OpLoad %v4float %689
%985 = OpVectorShuffle %v2float %984 %984 1 3
OpStore %986 %985
%987 = OpFunctionCall %float %_soft_light_component %983 %986
%988 = OpLoad %v4float %688
%989 = OpVectorShuffle %v2float %988 %988 2 3
OpStore %990 %989
%991 = OpLoad %v4float %689
%992 = OpVectorShuffle %v2float %991 %991 2 3
OpStore %993 %992
%994 = OpFunctionCall %float %_soft_light_component %990 %993
%995 = OpLoad %v4float %688
%996 = OpCompositeExtract %float %995 3
%997 = OpLoad %v4float %688
%998 = OpCompositeExtract %float %997 3
%999 = OpFSub %float %float_1 %998
%1000 = OpLoad %v4float %689
%1001 = OpCompositeExtract %float %1000 3
%1002 = OpFMul %float %999 %1001
%1003 = OpFAdd %float %996 %1002
%1004 = OpCompositeConstruct %v4float %980 %987 %994 %1003
OpStore %969 %1004
OpBranch %972
%972 = OpLabel
%1005 = OpLoad %v4float %969
OpStore %_39_blend_soft_light %1005
%1006 = OpLoad %v4float %_39_blend_soft_light
OpReturnValue %1006
%715 = OpLabel
%1008 = OpLoad %v4float %688
%1009 = OpVectorShuffle %v3float %1008 %1008 0 1 2
%1010 = OpLoad %v4float %689
%1011 = OpVectorShuffle %v3float %1010 %1010 0 1 2
%1012 = OpFAdd %v3float %1009 %1011
%1014 = OpLoad %v4float %688
%1015 = OpVectorShuffle %v3float %1014 %1014 0 1 2
%1016 = OpLoad %v4float %689
%1017 = OpCompositeExtract %float %1016 3
%1018 = OpVectorTimesScalar %v3float %1015 %1017
%1019 = OpLoad %v4float %689
%1020 = OpVectorShuffle %v3float %1019 %1019 0 1 2
%1021 = OpLoad %v4float %688
%1022 = OpCompositeExtract %float %1021 3
%1023 = OpVectorTimesScalar %v3float %1020 %1022
%1013 = OpExtInst %v3float %1 FMin %1018 %1023
%1024 = OpVectorTimesScalar %v3float %1013 %float_2
%1025 = OpFSub %v3float %1012 %1024
%1026 = OpCompositeExtract %float %1025 0
%1027 = OpCompositeExtract %float %1025 1
%1028 = OpCompositeExtract %float %1025 2
%1029 = OpLoad %v4float %688
%1030 = OpCompositeExtract %float %1029 3
%1031 = OpLoad %v4float %688
%1032 = OpCompositeExtract %float %1031 3
%1033 = OpFSub %float %float_1 %1032
%1034 = OpLoad %v4float %689
%1035 = OpCompositeExtract %float %1034 3
%1036 = OpFMul %float %1033 %1035
%1037 = OpFAdd %float %1030 %1036
%1038 = OpCompositeConstruct %v4float %1026 %1027 %1028 %1037
OpStore %_40_blend_difference %1038
%1039 = OpLoad %v4float %_40_blend_difference
OpReturnValue %1039
%716 = OpLabel
%1041 = OpLoad %v4float %689
%1042 = OpVectorShuffle %v3float %1041 %1041 0 1 2
%1043 = OpLoad %v4float %688
%1044 = OpVectorShuffle %v3float %1043 %1043 0 1 2
%1045 = OpFAdd %v3float %1042 %1044
%1046 = OpLoad %v4float %689
%1047 = OpVectorShuffle %v3float %1046 %1046 0 1 2
%1048 = OpVectorTimesScalar %v3float %1047 %float_2
%1049 = OpLoad %v4float %688
%1050 = OpVectorShuffle %v3float %1049 %1049 0 1 2
%1051 = OpFMul %v3float %1048 %1050
%1052 = OpFSub %v3float %1045 %1051
%1053 = OpCompositeExtract %float %1052 0
%1054 = OpCompositeExtract %float %1052 1
%1055 = OpCompositeExtract %float %1052 2
%1056 = OpLoad %v4float %688
%1057 = OpCompositeExtract %float %1056 3
%1058 = OpLoad %v4float %688
%1059 = OpCompositeExtract %float %1058 3
%1060 = OpFSub %float %float_1 %1059
%1061 = OpLoad %v4float %689
%1062 = OpCompositeExtract %float %1061 3
%1063 = OpFMul %float %1060 %1062
%1064 = OpFAdd %float %1057 %1063
%1065 = OpCompositeConstruct %v4float %1053 %1054 %1055 %1064
OpStore %_41_blend_exclusion %1065
%1066 = OpLoad %v4float %_41_blend_exclusion
OpReturnValue %1066
%717 = OpLabel
%1068 = OpLoad %v4float %688
%1069 = OpCompositeExtract %float %1068 3
%1070 = OpFSub %float %float_1 %1069
%1071 = OpLoad %v4float %689
%1072 = OpVectorShuffle %v3float %1071 %1071 0 1 2
%1073 = OpVectorTimesScalar %v3float %1072 %1070
%1074 = OpLoad %v4float %689
%1075 = OpCompositeExtract %float %1074 3
%1076 = OpFSub %float %float_1 %1075
%1077 = OpLoad %v4float %688
%1078 = OpVectorShuffle %v3float %1077 %1077 0 1 2
%1079 = OpVectorTimesScalar %v3float %1078 %1076
%1080 = OpFAdd %v3float %1073 %1079
%1081 = OpLoad %v4float %688
%1082 = OpVectorShuffle %v3float %1081 %1081 0 1 2
%1083 = OpLoad %v4float %689
%1084 = OpVectorShuffle %v3float %1083 %1083 0 1 2
%1085 = OpFMul %v3float %1082 %1084
%1086 = OpFAdd %v3float %1080 %1085
%1087 = OpCompositeExtract %float %1086 0
%1088 = OpCompositeExtract %float %1086 1
%1089 = OpCompositeExtract %float %1086 2
%1090 = OpLoad %v4float %688
%1091 = OpCompositeExtract %float %1090 3
%1092 = OpLoad %v4float %688
%1093 = OpCompositeExtract %float %1092 3
%1094 = OpFSub %float %float_1 %1093
%1095 = OpLoad %v4float %689
%1096 = OpCompositeExtract %float %1095 3
%1097 = OpFMul %float %1094 %1096
%1098 = OpFAdd %float %1091 %1097
%1099 = OpCompositeConstruct %v4float %1087 %1088 %1089 %1098
OpStore %_42_blend_multiply %1099
%1100 = OpLoad %v4float %_42_blend_multiply
OpReturnValue %1100
%718 = OpLabel
%1103 = OpLoad %v4float %689
%1104 = OpCompositeExtract %float %1103 3
%1105 = OpLoad %v4float %688
%1106 = OpCompositeExtract %float %1105 3
%1107 = OpFMul %float %1104 %1106
OpStore %_44_alpha %1107
%1109 = OpLoad %v4float %688
%1110 = OpVectorShuffle %v3float %1109 %1109 0 1 2
%1111 = OpLoad %v4float %689
%1112 = OpCompositeExtract %float %1111 3
%1113 = OpVectorTimesScalar %v3float %1110 %1112
OpStore %_45_sda %1113
%1115 = OpLoad %v4float %689
%1116 = OpVectorShuffle %v3float %1115 %1115 0 1 2
%1117 = OpLoad %v4float %688
%1118 = OpCompositeExtract %float %1117 3
%1119 = OpVectorTimesScalar %v3float %1116 %1118
OpStore %_46_dsa %1119
%1120 = OpLoad %v3float %_45_sda
OpStore %1121 %1120
%1122 = OpLoad %v3float %_46_dsa
OpStore %1123 %1122
%1124 = OpFunctionCall %v3float %_blend_set_color_saturation %1121 %1123
OpStore %1125 %1124
%1126 = OpLoad %float %_44_alpha
OpStore %1127 %1126
%1128 = OpLoad %v3float %_46_dsa
OpStore %1129 %1128
%1130 = OpFunctionCall %v3float %_blend_set_color_luminance %1125 %1127 %1129
%1131 = OpLoad %v4float %689
%1132 = OpVectorShuffle %v3float %1131 %1131 0 1 2
%1133 = OpFAdd %v3float %1130 %1132
%1134 = OpLoad %v3float %_46_dsa
%1135 = OpFSub %v3float %1133 %1134
%1136 = OpLoad %v4float %688
%1137 = OpVectorShuffle %v3float %1136 %1136 0 1 2
%1138 = OpFAdd %v3float %1135 %1137
%1139 = OpLoad %v3float %_45_sda
%1140 = OpFSub %v3float %1138 %1139
%1141 = OpCompositeExtract %float %1140 0
%1142 = OpCompositeExtract %float %1140 1
%1143 = OpCompositeExtract %float %1140 2
%1144 = OpLoad %v4float %688
%1145 = OpCompositeExtract %float %1144 3
%1146 = OpLoad %v4float %689
%1147 = OpCompositeExtract %float %1146 3
%1148 = OpFAdd %float %1145 %1147
%1149 = OpLoad %float %_44_alpha
%1150 = OpFSub %float %1148 %1149
%1151 = OpCompositeConstruct %v4float %1141 %1142 %1143 %1150
OpStore %_43_blend_hue %1151
%1152 = OpLoad %v4float %_43_blend_hue
OpReturnValue %1152
%719 = OpLabel
%1155 = OpLoad %v4float %689
%1156 = OpCompositeExtract %float %1155 3
%1157 = OpLoad %v4float %688
%1158 = OpCompositeExtract %float %1157 3
%1159 = OpFMul %float %1156 %1158
OpStore %_48_alpha %1159
%1161 = OpLoad %v4float %688
%1162 = OpVectorShuffle %v3float %1161 %1161 0 1 2
%1163 = OpLoad %v4float %689
%1164 = OpCompositeExtract %float %1163 3
%1165 = OpVectorTimesScalar %v3float %1162 %1164
OpStore %_49_sda %1165
%1167 = OpLoad %v4float %689
%1168 = OpVectorShuffle %v3float %1167 %1167 0 1 2
%1169 = OpLoad %v4float %688
%1170 = OpCompositeExtract %float %1169 3
%1171 = OpVectorTimesScalar %v3float %1168 %1170
OpStore %_50_dsa %1171
%1172 = OpLoad %v3float %_50_dsa
OpStore %1173 %1172
%1174 = OpLoad %v3float %_49_sda
OpStore %1175 %1174
%1176 = OpFunctionCall %v3float %_blend_set_color_saturation %1173 %1175
OpStore %1177 %1176
%1178 = OpLoad %float %_48_alpha
OpStore %1179 %1178
%1180 = OpLoad %v3float %_50_dsa
OpStore %1181 %1180
%1182 = OpFunctionCall %v3float %_blend_set_color_luminance %1177 %1179 %1181
%1183 = OpLoad %v4float %689
%1184 = OpVectorShuffle %v3float %1183 %1183 0 1 2
%1185 = OpFAdd %v3float %1182 %1184
%1186 = OpLoad %v3float %_50_dsa
%1187 = OpFSub %v3float %1185 %1186
%1188 = OpLoad %v4float %688
%1189 = OpVectorShuffle %v3float %1188 %1188 0 1 2
%1190 = OpFAdd %v3float %1187 %1189
%1191 = OpLoad %v3float %_49_sda
%1192 = OpFSub %v3float %1190 %1191
%1193 = OpCompositeExtract %float %1192 0
%1194 = OpCompositeExtract %float %1192 1
%1195 = OpCompositeExtract %float %1192 2
%1196 = OpLoad %v4float %688
%1197 = OpCompositeExtract %float %1196 3
%1198 = OpLoad %v4float %689
%1199 = OpCompositeExtract %float %1198 3
%1200 = OpFAdd %float %1197 %1199
%1201 = OpLoad %float %_48_alpha
%1202 = OpFSub %float %1200 %1201
%1203 = OpCompositeConstruct %v4float %1193 %1194 %1195 %1202
OpStore %_47_blend_saturation %1203
%1204 = OpLoad %v4float %_47_blend_saturation
OpReturnValue %1204
%720 = OpLabel
%1207 = OpLoad %v4float %689
%1208 = OpCompositeExtract %float %1207 3
%1209 = OpLoad %v4float %688
%1210 = OpCompositeExtract %float %1209 3
%1211 = OpFMul %float %1208 %1210
OpStore %_52_alpha %1211
%1213 = OpLoad %v4float %688
%1214 = OpVectorShuffle %v3float %1213 %1213 0 1 2
%1215 = OpLoad %v4float %689
%1216 = OpCompositeExtract %float %1215 3
%1217 = OpVectorTimesScalar %v3float %1214 %1216
OpStore %_53_sda %1217
%1219 = OpLoad %v4float %689
%1220 = OpVectorShuffle %v3float %1219 %1219 0 1 2
%1221 = OpLoad %v4float %688
%1222 = OpCompositeExtract %float %1221 3
%1223 = OpVectorTimesScalar %v3float %1220 %1222
OpStore %_54_dsa %1223
%1224 = OpLoad %v3float %_53_sda
OpStore %1225 %1224
%1226 = OpLoad %float %_52_alpha
OpStore %1227 %1226
%1228 = OpLoad %v3float %_54_dsa
OpStore %1229 %1228
%1230 = OpFunctionCall %v3float %_blend_set_color_luminance %1225 %1227 %1229
%1231 = OpLoad %v4float %689
%1232 = OpVectorShuffle %v3float %1231 %1231 0 1 2
%1233 = OpFAdd %v3float %1230 %1232
%1234 = OpLoad %v3float %_54_dsa
%1235 = OpFSub %v3float %1233 %1234
%1236 = OpLoad %v4float %688
%1237 = OpVectorShuffle %v3float %1236 %1236 0 1 2
%1238 = OpFAdd %v3float %1235 %1237
%1239 = OpLoad %v3float %_53_sda
%1240 = OpFSub %v3float %1238 %1239
%1241 = OpCompositeExtract %float %1240 0
%1242 = OpCompositeExtract %float %1240 1
%1243 = OpCompositeExtract %float %1240 2
%1244 = OpLoad %v4float %688
%1245 = OpCompositeExtract %float %1244 3
%1246 = OpLoad %v4float %689
%1247 = OpCompositeExtract %float %1246 3
%1248 = OpFAdd %float %1245 %1247
%1249 = OpLoad %float %_52_alpha
%1250 = OpFSub %float %1248 %1249
%1251 = OpCompositeConstruct %v4float %1241 %1242 %1243 %1250
OpStore %_51_blend_color %1251
%1252 = OpLoad %v4float %_51_blend_color
OpReturnValue %1252
%721 = OpLabel
%1255 = OpLoad %v4float %689
%1256 = OpCompositeExtract %float %1255 3
%1257 = OpLoad %v4float %688
%1258 = OpCompositeExtract %float %1257 3
%1259 = OpFMul %float %1256 %1258
OpStore %_56_alpha %1259
%1261 = OpLoad %v4float %688
%1262 = OpVectorShuffle %v3float %1261 %1261 0 1 2
%1263 = OpLoad %v4float %689
%1264 = OpCompositeExtract %float %1263 3
%1265 = OpVectorTimesScalar %v3float %1262 %1264
OpStore %_57_sda %1265
%1267 = OpLoad %v4float %689
%1268 = OpVectorShuffle %v3float %1267 %1267 0 1 2
%1269 = OpLoad %v4float %688
%1270 = OpCompositeExtract %float %1269 3
%1271 = OpVectorTimesScalar %v3float %1268 %1270
OpStore %_58_dsa %1271
%1272 = OpLoad %v3float %_58_dsa
OpStore %1273 %1272
%1274 = OpLoad %float %_56_alpha
OpStore %1275 %1274
%1276 = OpLoad %v3float %_57_sda
OpStore %1277 %1276
%1278 = OpFunctionCall %v3float %_blend_set_color_luminance %1273 %1275 %1277
%1279 = OpLoad %v4float %689
%1280 = OpVectorShuffle %v3float %1279 %1279 0 1 2
%1281 = OpFAdd %v3float %1278 %1280
%1282 = OpLoad %v3float %_58_dsa
%1283 = OpFSub %v3float %1281 %1282
%1284 = OpLoad %v4float %688
%1285 = OpVectorShuffle %v3float %1284 %1284 0 1 2
%1286 = OpFAdd %v3float %1283 %1285
%1287 = OpLoad %v3float %_57_sda
%1288 = OpFSub %v3float %1286 %1287
%1289 = OpCompositeExtract %float %1288 0
%1290 = OpCompositeExtract %float %1288 1
%1291 = OpCompositeExtract %float %1288 2
%1292 = OpLoad %v4float %688
%1293 = OpCompositeExtract %float %1292 3
%1294 = OpLoad %v4float %689
%1295 = OpCompositeExtract %float %1294 3
%1296 = OpFAdd %float %1293 %1295
%1297 = OpLoad %float %_56_alpha
%1298 = OpFSub %float %1296 %1297
%1299 = OpCompositeConstruct %v4float %1289 %1290 %1291 %1298
OpStore %_55_blend_luminosity %1299
%1300 = OpLoad %v4float %_55_blend_luminosity
OpReturnValue %1300
%692 = OpLabel
OpReturnValue %1301
OpFunctionEnd
%main = OpFunction %void None %1303
%1304 = OpLabel
%1306 = OpVariable %_ptr_Function_int Function
%1308 = OpVariable %_ptr_Function_v4float Function
%1310 = OpVariable %_ptr_Function_v4float Function
OpStore %1306 %int_13
%1307 = OpLoad %v4float %src
OpStore %1308 %1307
%1309 = OpLoad %v4float %dst
OpStore %1310 %1309
%1311 = OpFunctionCall %v4float %blend %1306 %1308 %1310
OpStore %sk_FragColor %1311
OpReturn
OpFunctionEnd
