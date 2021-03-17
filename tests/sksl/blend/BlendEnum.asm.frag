OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_blend_overlay_component "_blend_overlay_component"
OpName %blend_overlay "blend_overlay"
OpName %result "result"
OpName %_color_dodge_component "_color_dodge_component"
OpName %delta "delta"
OpName %_0_n "_0_n"
OpName %_color_burn_component "_color_burn_component"
OpName %_1_n "_1_n"
OpName %delta_0 "delta"
OpName %_soft_light_component "_soft_light_component"
OpName %_2_n "_2_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_3_n "_3_n"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result_0 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_4_d "_4_d"
OpName %_5_n "_5_n"
OpName %_6_d "_6_d"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_7_n "_7_n"
OpName %_8_d "_8_d"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %sat "sat"
OpName %blend "blend"
OpName %_9_result "_9_result"
OpName %_10_result "_10_result"
OpName %_11_alpha "_11_alpha"
OpName %_12_sda "_12_sda"
OpName %_13_dsa "_13_dsa"
OpName %_14_alpha "_14_alpha"
OpName %_15_sda "_15_sda"
OpName %_16_dsa "_16_dsa"
OpName %_17_alpha "_17_alpha"
OpName %_18_sda "_18_sda"
OpName %_19_dsa "_19_dsa"
OpName %_20_alpha "_20_alpha"
OpName %_21_sda "_21_sda"
OpName %_22_dsa "_22_dsa"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
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
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
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
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %534 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %539 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %554 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %559 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %579 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %596 RelaxedPrecision
OpDecorate %602 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %610 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %615 RelaxedPrecision
OpDecorate %617 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %626 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %639 RelaxedPrecision
OpDecorate %645 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %652 RelaxedPrecision
OpDecorate %654 RelaxedPrecision
OpDecorate %660 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %667 RelaxedPrecision
OpDecorate %670 RelaxedPrecision
OpDecorate %681 RelaxedPrecision
OpDecorate %714 RelaxedPrecision
OpDecorate %715 RelaxedPrecision
OpDecorate %716 RelaxedPrecision
OpDecorate %717 RelaxedPrecision
OpDecorate %719 RelaxedPrecision
OpDecorate %720 RelaxedPrecision
OpDecorate %722 RelaxedPrecision
OpDecorate %723 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %726 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %729 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %731 RelaxedPrecision
OpDecorate %734 RelaxedPrecision
OpDecorate %735 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %740 RelaxedPrecision
OpDecorate %741 RelaxedPrecision
OpDecorate %743 RelaxedPrecision
OpDecorate %745 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %750 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %754 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %757 RelaxedPrecision
OpDecorate %758 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %761 RelaxedPrecision
OpDecorate %763 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %770 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %781 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %785 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %787 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %792 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %794 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %804 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %810 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
OpDecorate %813 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %823 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %827 RelaxedPrecision
OpDecorate %829 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %833 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %852 RelaxedPrecision
OpDecorate %855 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %862 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %876 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %883 RelaxedPrecision
OpDecorate %886 RelaxedPrecision
OpDecorate %890 RelaxedPrecision
OpDecorate %893 RelaxedPrecision
OpDecorate %897 RelaxedPrecision
OpDecorate %899 RelaxedPrecision
OpDecorate %901 RelaxedPrecision
OpDecorate %902 RelaxedPrecision
OpDecorate %904 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %909 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %920 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %927 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %937 RelaxedPrecision
OpDecorate %941 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %946 RelaxedPrecision
OpDecorate %948 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %952 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %956 RelaxedPrecision
OpDecorate %958 RelaxedPrecision
OpDecorate %960 RelaxedPrecision
OpDecorate %963 RelaxedPrecision
OpDecorate %965 RelaxedPrecision
OpDecorate %969 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %975 RelaxedPrecision
OpDecorate %977 RelaxedPrecision
OpDecorate %978 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %994 RelaxedPrecision
OpDecorate %998 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1017 RelaxedPrecision
OpDecorate %1020 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %1023 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1026 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %1035 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1041 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1045 RelaxedPrecision
OpDecorate %1047 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1060 RelaxedPrecision
OpDecorate %1064 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1069 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %1072 RelaxedPrecision
OpDecorate %1073 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1076 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1078 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1084 RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1087 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1091 RelaxedPrecision
OpDecorate %1093 RelaxedPrecision
OpDecorate %1095 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1108 RelaxedPrecision
OpDecorate %1110 RelaxedPrecision
OpDecorate %1114 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1119 RelaxedPrecision
OpDecorate %1121 RelaxedPrecision
OpDecorate %1122 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1124 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1141 RelaxedPrecision
OpDecorate %1143 RelaxedPrecision
OpDecorate %1145 RelaxedPrecision
OpDecorate %1147 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1158 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1162 RelaxedPrecision
OpDecorate %1165 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1168 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1187 RelaxedPrecision
OpDecorate %1189 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1193 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1204 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1208 RelaxedPrecision
OpDecorate %1211 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1216 RelaxedPrecision
OpDecorate %1218 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1220 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1229 RelaxedPrecision
OpDecorate %1230 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1244 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
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
%442 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%453 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%540 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%571 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%572 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%675 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%713 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1233 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%_0_n = OpVariable %_ptr_Function_float Function
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
%170 = OpLoad %v2float %124
%171 = OpCompositeExtract %float %170 0
%172 = OpLoad %v2float %123
%173 = OpCompositeExtract %float %172 1
%174 = OpFMul %float %171 %173
OpStore %_0_n %174
%176 = OpLoad %v2float %124
%177 = OpCompositeExtract %float %176 1
%178 = OpLoad %float %_0_n
%179 = OpLoad %float %delta
%180 = OpFDiv %float %178 %179
%175 = OpExtInst %float %1 FMin %177 %180
OpStore %delta %175
%181 = OpLoad %float %delta
%182 = OpLoad %v2float %123
%183 = OpCompositeExtract %float %182 1
%184 = OpFMul %float %181 %183
%185 = OpLoad %v2float %123
%186 = OpCompositeExtract %float %185 0
%187 = OpLoad %v2float %124
%188 = OpCompositeExtract %float %187 1
%189 = OpFSub %float %float_1 %188
%190 = OpFMul %float %186 %189
%191 = OpFAdd %float %184 %190
%192 = OpLoad %v2float %124
%193 = OpCompositeExtract %float %192 0
%194 = OpLoad %v2float %123
%195 = OpCompositeExtract %float %194 1
%196 = OpFSub %float %float_1 %195
%197 = OpFMul %float %193 %196
%198 = OpFAdd %float %191 %197
OpReturnValue %198
%149 = OpLabel
OpBranch %132
%132 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %23
%199 = OpFunctionParameter %_ptr_Function_v2float
%200 = OpFunctionParameter %_ptr_Function_v2float
%201 = OpLabel
%_1_n = OpVariable %_ptr_Function_float Function
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
%242 = OpLoad %v2float %200
%243 = OpCompositeExtract %float %242 1
%244 = OpLoad %v2float %200
%245 = OpCompositeExtract %float %244 0
%246 = OpFSub %float %243 %245
%247 = OpLoad %v2float %199
%248 = OpCompositeExtract %float %247 1
%249 = OpFMul %float %246 %248
OpStore %_1_n %249
%252 = OpLoad %v2float %200
%253 = OpCompositeExtract %float %252 1
%254 = OpLoad %float %_1_n
%255 = OpLoad %v2float %199
%256 = OpCompositeExtract %float %255 0
%257 = OpFDiv %float %254 %256
%258 = OpFSub %float %253 %257
%251 = OpExtInst %float %1 FMax %float_0 %258
OpStore %delta_0 %251
%259 = OpLoad %float %delta_0
%260 = OpLoad %v2float %199
%261 = OpCompositeExtract %float %260 1
%262 = OpFMul %float %259 %261
%263 = OpLoad %v2float %199
%264 = OpCompositeExtract %float %263 0
%265 = OpLoad %v2float %200
%266 = OpCompositeExtract %float %265 1
%267 = OpFSub %float %float_1 %266
%268 = OpFMul %float %264 %267
%269 = OpFAdd %float %262 %268
%270 = OpLoad %v2float %200
%271 = OpCompositeExtract %float %270 0
%272 = OpLoad %v2float %199
%273 = OpCompositeExtract %float %272 1
%274 = OpFSub %float %float_1 %273
%275 = OpFMul %float %271 %274
%276 = OpFAdd %float %269 %275
OpReturnValue %276
%234 = OpLabel
OpBranch %209
%209 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %23
%277 = OpFunctionParameter %_ptr_Function_v2float
%278 = OpFunctionParameter %_ptr_Function_v2float
%279 = OpLabel
%_2_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_3_n = OpVariable %_ptr_Function_float Function
%280 = OpLoad %v2float %277
%281 = OpCompositeExtract %float %280 0
%282 = OpFMul %float %float_2 %281
%283 = OpLoad %v2float %277
%284 = OpCompositeExtract %float %283 1
%285 = OpFOrdLessThanEqual %bool %282 %284
OpSelectionMerge %288 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%290 = OpLoad %v2float %278
%291 = OpCompositeExtract %float %290 0
%292 = OpLoad %v2float %278
%293 = OpCompositeExtract %float %292 0
%294 = OpFMul %float %291 %293
%295 = OpLoad %v2float %277
%296 = OpCompositeExtract %float %295 1
%297 = OpLoad %v2float %277
%298 = OpCompositeExtract %float %297 0
%299 = OpFMul %float %float_2 %298
%300 = OpFSub %float %296 %299
%301 = OpFMul %float %294 %300
OpStore %_2_n %301
%302 = OpLoad %float %_2_n
%303 = OpLoad %v2float %278
%304 = OpCompositeExtract %float %303 1
%305 = OpFDiv %float %302 %304
%306 = OpLoad %v2float %278
%307 = OpCompositeExtract %float %306 1
%308 = OpFSub %float %float_1 %307
%309 = OpLoad %v2float %277
%310 = OpCompositeExtract %float %309 0
%311 = OpFMul %float %308 %310
%312 = OpFAdd %float %305 %311
%313 = OpLoad %v2float %278
%314 = OpCompositeExtract %float %313 0
%316 = OpLoad %v2float %277
%317 = OpCompositeExtract %float %316 1
%315 = OpFNegate %float %317
%318 = OpLoad %v2float %277
%319 = OpCompositeExtract %float %318 0
%320 = OpFMul %float %float_2 %319
%321 = OpFAdd %float %315 %320
%322 = OpFAdd %float %321 %float_1
%323 = OpFMul %float %314 %322
%324 = OpFAdd %float %312 %323
OpReturnValue %324
%287 = OpLabel
%326 = OpLoad %v2float %278
%327 = OpCompositeExtract %float %326 0
%328 = OpFMul %float %float_4 %327
%329 = OpLoad %v2float %278
%330 = OpCompositeExtract %float %329 1
%331 = OpFOrdLessThanEqual %bool %328 %330
OpSelectionMerge %334 None
OpBranchConditional %331 %332 %333
%332 = OpLabel
%336 = OpLoad %v2float %278
%337 = OpCompositeExtract %float %336 0
%338 = OpLoad %v2float %278
%339 = OpCompositeExtract %float %338 0
%340 = OpFMul %float %337 %339
OpStore %DSqd %340
%342 = OpLoad %float %DSqd
%343 = OpLoad %v2float %278
%344 = OpCompositeExtract %float %343 0
%345 = OpFMul %float %342 %344
OpStore %DCub %345
%347 = OpLoad %v2float %278
%348 = OpCompositeExtract %float %347 1
%349 = OpLoad %v2float %278
%350 = OpCompositeExtract %float %349 1
%351 = OpFMul %float %348 %350
OpStore %DaSqd %351
%353 = OpLoad %float %DaSqd
%354 = OpLoad %v2float %278
%355 = OpCompositeExtract %float %354 1
%356 = OpFMul %float %353 %355
OpStore %DaCub %356
%358 = OpLoad %float %DaSqd
%359 = OpLoad %v2float %277
%360 = OpCompositeExtract %float %359 0
%361 = OpLoad %v2float %278
%362 = OpCompositeExtract %float %361 0
%364 = OpLoad %v2float %277
%365 = OpCompositeExtract %float %364 1
%366 = OpFMul %float %float_3 %365
%368 = OpLoad %v2float %277
%369 = OpCompositeExtract %float %368 0
%370 = OpFMul %float %float_6 %369
%371 = OpFSub %float %366 %370
%372 = OpFSub %float %371 %float_1
%373 = OpFMul %float %362 %372
%374 = OpFSub %float %360 %373
%375 = OpFMul %float %358 %374
%377 = OpLoad %v2float %278
%378 = OpCompositeExtract %float %377 1
%379 = OpFMul %float %float_12 %378
%380 = OpLoad %float %DSqd
%381 = OpFMul %float %379 %380
%382 = OpLoad %v2float %277
%383 = OpCompositeExtract %float %382 1
%384 = OpLoad %v2float %277
%385 = OpCompositeExtract %float %384 0
%386 = OpFMul %float %float_2 %385
%387 = OpFSub %float %383 %386
%388 = OpFMul %float %381 %387
%389 = OpFAdd %float %375 %388
%391 = OpLoad %float %DCub
%392 = OpFMul %float %float_16 %391
%393 = OpLoad %v2float %277
%394 = OpCompositeExtract %float %393 1
%395 = OpLoad %v2float %277
%396 = OpCompositeExtract %float %395 0
%397 = OpFMul %float %float_2 %396
%398 = OpFSub %float %394 %397
%399 = OpFMul %float %392 %398
%400 = OpFSub %float %389 %399
%401 = OpLoad %float %DaCub
%402 = OpLoad %v2float %277
%403 = OpCompositeExtract %float %402 0
%404 = OpFMul %float %401 %403
%405 = OpFSub %float %400 %404
OpStore %_3_n %405
%406 = OpLoad %float %_3_n
%407 = OpLoad %float %DaSqd
%408 = OpFDiv %float %406 %407
OpReturnValue %408
%333 = OpLabel
%409 = OpLoad %v2float %278
%410 = OpCompositeExtract %float %409 0
%411 = OpLoad %v2float %277
%412 = OpCompositeExtract %float %411 1
%413 = OpLoad %v2float %277
%414 = OpCompositeExtract %float %413 0
%415 = OpFMul %float %float_2 %414
%416 = OpFSub %float %412 %415
%417 = OpFAdd %float %416 %float_1
%418 = OpFMul %float %410 %417
%419 = OpLoad %v2float %277
%420 = OpCompositeExtract %float %419 0
%421 = OpFAdd %float %418 %420
%423 = OpLoad %v2float %278
%424 = OpCompositeExtract %float %423 1
%425 = OpLoad %v2float %278
%426 = OpCompositeExtract %float %425 0
%427 = OpFMul %float %424 %426
%422 = OpExtInst %float %1 Sqrt %427
%428 = OpLoad %v2float %277
%429 = OpCompositeExtract %float %428 1
%430 = OpLoad %v2float %277
%431 = OpCompositeExtract %float %430 0
%432 = OpFMul %float %float_2 %431
%433 = OpFSub %float %429 %432
%434 = OpFMul %float %422 %433
%435 = OpFSub %float %421 %434
%436 = OpLoad %v2float %278
%437 = OpCompositeExtract %float %436 1
%438 = OpLoad %v2float %277
%439 = OpCompositeExtract %float %438 0
%440 = OpFMul %float %437 %439
%441 = OpFSub %float %435 %440
OpReturnValue %441
%334 = OpLabel
OpBranch %288
%288 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %442
%444 = OpFunctionParameter %_ptr_Function_v3float
%445 = OpFunctionParameter %_ptr_Function_float
%446 = OpFunctionParameter %_ptr_Function_v3float
%447 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_0 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%454 = OpLoad %v3float %446
%449 = OpDot %float %453 %454
OpStore %lum %449
%456 = OpLoad %float %lum
%458 = OpLoad %v3float %444
%457 = OpDot %float %453 %458
%459 = OpFSub %float %456 %457
%460 = OpLoad %v3float %444
%461 = OpCompositeConstruct %v3float %459 %459 %459
%462 = OpFAdd %v3float %461 %460
OpStore %result_0 %462
%466 = OpLoad %v3float %result_0
%467 = OpCompositeExtract %float %466 0
%468 = OpLoad %v3float %result_0
%469 = OpCompositeExtract %float %468 1
%465 = OpExtInst %float %1 FMin %467 %469
%470 = OpLoad %v3float %result_0
%471 = OpCompositeExtract %float %470 2
%464 = OpExtInst %float %1 FMin %465 %471
OpStore %minComp %464
%475 = OpLoad %v3float %result_0
%476 = OpCompositeExtract %float %475 0
%477 = OpLoad %v3float %result_0
%478 = OpCompositeExtract %float %477 1
%474 = OpExtInst %float %1 FMax %476 %478
%479 = OpLoad %v3float %result_0
%480 = OpCompositeExtract %float %479 2
%473 = OpExtInst %float %1 FMax %474 %480
OpStore %maxComp %473
%482 = OpLoad %float %minComp
%483 = OpFOrdLessThan %bool %482 %float_0
OpSelectionMerge %485 None
OpBranchConditional %483 %484 %485
%484 = OpLabel
%486 = OpLoad %float %lum
%487 = OpLoad %float %minComp
%488 = OpFOrdNotEqual %bool %486 %487
OpBranch %485
%485 = OpLabel
%489 = OpPhi %bool %false %447 %488 %484
OpSelectionMerge %491 None
OpBranchConditional %489 %490 %491
%490 = OpLabel
%493 = OpLoad %float %lum
%494 = OpLoad %float %minComp
%495 = OpFSub %float %493 %494
OpStore %_4_d %495
%496 = OpLoad %float %lum
%497 = OpLoad %v3float %result_0
%498 = OpLoad %float %lum
%499 = OpCompositeConstruct %v3float %498 %498 %498
%500 = OpFSub %v3float %497 %499
%501 = OpLoad %float %lum
%502 = OpLoad %float %_4_d
%503 = OpFDiv %float %501 %502
%504 = OpVectorTimesScalar %v3float %500 %503
%505 = OpCompositeConstruct %v3float %496 %496 %496
%506 = OpFAdd %v3float %505 %504
OpStore %result_0 %506
OpBranch %491
%491 = OpLabel
%507 = OpLoad %float %maxComp
%508 = OpLoad %float %445
%509 = OpFOrdGreaterThan %bool %507 %508
OpSelectionMerge %511 None
OpBranchConditional %509 %510 %511
%510 = OpLabel
%512 = OpLoad %float %maxComp
%513 = OpLoad %float %lum
%514 = OpFOrdNotEqual %bool %512 %513
OpBranch %511
%511 = OpLabel
%515 = OpPhi %bool %false %491 %514 %510
OpSelectionMerge %518 None
OpBranchConditional %515 %516 %517
%516 = OpLabel
%520 = OpLoad %v3float %result_0
%521 = OpLoad %float %lum
%522 = OpCompositeConstruct %v3float %521 %521 %521
%523 = OpFSub %v3float %520 %522
%524 = OpLoad %float %445
%525 = OpLoad %float %lum
%526 = OpFSub %float %524 %525
%527 = OpVectorTimesScalar %v3float %523 %526
OpStore %_5_n %527
%529 = OpLoad %float %maxComp
%530 = OpLoad %float %lum
%531 = OpFSub %float %529 %530
OpStore %_6_d %531
%532 = OpLoad %float %lum
%533 = OpLoad %v3float %_5_n
%534 = OpLoad %float %_6_d
%535 = OpFDiv %float %float_1 %534
%536 = OpVectorTimesScalar %v3float %533 %535
%537 = OpCompositeConstruct %v3float %532 %532 %532
%538 = OpFAdd %v3float %537 %536
OpReturnValue %538
%517 = OpLabel
%539 = OpLoad %v3float %result_0
OpReturnValue %539
%518 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %540
%541 = OpFunctionParameter %_ptr_Function_v3float
%542 = OpFunctionParameter %_ptr_Function_float
%543 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%544 = OpLoad %v3float %541
%545 = OpCompositeExtract %float %544 0
%546 = OpLoad %v3float %541
%547 = OpCompositeExtract %float %546 2
%548 = OpFOrdLessThan %bool %545 %547
OpSelectionMerge %551 None
OpBranchConditional %548 %549 %550
%549 = OpLabel
%553 = OpLoad %float %542
%554 = OpLoad %v3float %541
%555 = OpCompositeExtract %float %554 1
%556 = OpLoad %v3float %541
%557 = OpCompositeExtract %float %556 0
%558 = OpFSub %float %555 %557
%559 = OpFMul %float %553 %558
OpStore %_7_n %559
%561 = OpLoad %v3float %541
%562 = OpCompositeExtract %float %561 2
%563 = OpLoad %v3float %541
%564 = OpCompositeExtract %float %563 0
%565 = OpFSub %float %562 %564
OpStore %_8_d %565
%566 = OpLoad %float %_7_n
%567 = OpLoad %float %_8_d
%568 = OpFDiv %float %566 %567
%569 = OpLoad %float %542
%570 = OpCompositeConstruct %v3float %float_0 %568 %569
OpReturnValue %570
%550 = OpLabel
OpReturnValue %571
%551 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %572
%573 = OpFunctionParameter %_ptr_Function_v3float
%574 = OpFunctionParameter %_ptr_Function_v3float
%575 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%611 = OpVariable %_ptr_Function_v3float Function
%613 = OpVariable %_ptr_Function_float Function
%625 = OpVariable %_ptr_Function_v3float Function
%627 = OpVariable %_ptr_Function_float Function
%632 = OpVariable %_ptr_Function_v3float Function
%634 = OpVariable %_ptr_Function_float Function
%647 = OpVariable %_ptr_Function_v3float Function
%649 = OpVariable %_ptr_Function_float Function
%662 = OpVariable %_ptr_Function_v3float Function
%664 = OpVariable %_ptr_Function_float Function
%669 = OpVariable %_ptr_Function_v3float Function
%671 = OpVariable %_ptr_Function_float Function
%579 = OpLoad %v3float %574
%580 = OpCompositeExtract %float %579 0
%581 = OpLoad %v3float %574
%582 = OpCompositeExtract %float %581 1
%578 = OpExtInst %float %1 FMax %580 %582
%583 = OpLoad %v3float %574
%584 = OpCompositeExtract %float %583 2
%577 = OpExtInst %float %1 FMax %578 %584
%587 = OpLoad %v3float %574
%588 = OpCompositeExtract %float %587 0
%589 = OpLoad %v3float %574
%590 = OpCompositeExtract %float %589 1
%586 = OpExtInst %float %1 FMin %588 %590
%591 = OpLoad %v3float %574
%592 = OpCompositeExtract %float %591 2
%585 = OpExtInst %float %1 FMin %586 %592
%593 = OpFSub %float %577 %585
OpStore %sat %593
%594 = OpLoad %v3float %573
%595 = OpCompositeExtract %float %594 0
%596 = OpLoad %v3float %573
%597 = OpCompositeExtract %float %596 1
%598 = OpFOrdLessThanEqual %bool %595 %597
OpSelectionMerge %601 None
OpBranchConditional %598 %599 %600
%599 = OpLabel
%602 = OpLoad %v3float %573
%603 = OpCompositeExtract %float %602 1
%604 = OpLoad %v3float %573
%605 = OpCompositeExtract %float %604 2
%606 = OpFOrdLessThanEqual %bool %603 %605
OpSelectionMerge %609 None
OpBranchConditional %606 %607 %608
%607 = OpLabel
%610 = OpLoad %v3float %573
OpStore %611 %610
%612 = OpLoad %float %sat
OpStore %613 %612
%614 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %611 %613
OpReturnValue %614
%608 = OpLabel
%615 = OpLoad %v3float %573
%616 = OpCompositeExtract %float %615 0
%617 = OpLoad %v3float %573
%618 = OpCompositeExtract %float %617 2
%619 = OpFOrdLessThanEqual %bool %616 %618
OpSelectionMerge %622 None
OpBranchConditional %619 %620 %621
%620 = OpLabel
%623 = OpLoad %v3float %573
%624 = OpVectorShuffle %v3float %623 %623 0 2 1
OpStore %625 %624
%626 = OpLoad %float %sat
OpStore %627 %626
%628 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %625 %627
%629 = OpVectorShuffle %v3float %628 %628 0 2 1
OpReturnValue %629
%621 = OpLabel
%630 = OpLoad %v3float %573
%631 = OpVectorShuffle %v3float %630 %630 2 0 1
OpStore %632 %631
%633 = OpLoad %float %sat
OpStore %634 %633
%635 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %632 %634
%636 = OpVectorShuffle %v3float %635 %635 1 2 0
OpReturnValue %636
%622 = OpLabel
OpBranch %609
%609 = OpLabel
OpBranch %601
%600 = OpLabel
%637 = OpLoad %v3float %573
%638 = OpCompositeExtract %float %637 0
%639 = OpLoad %v3float %573
%640 = OpCompositeExtract %float %639 2
%641 = OpFOrdLessThanEqual %bool %638 %640
OpSelectionMerge %644 None
OpBranchConditional %641 %642 %643
%642 = OpLabel
%645 = OpLoad %v3float %573
%646 = OpVectorShuffle %v3float %645 %645 1 0 2
OpStore %647 %646
%648 = OpLoad %float %sat
OpStore %649 %648
%650 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %647 %649
%651 = OpVectorShuffle %v3float %650 %650 1 0 2
OpReturnValue %651
%643 = OpLabel
%652 = OpLoad %v3float %573
%653 = OpCompositeExtract %float %652 1
%654 = OpLoad %v3float %573
%655 = OpCompositeExtract %float %654 2
%656 = OpFOrdLessThanEqual %bool %653 %655
OpSelectionMerge %659 None
OpBranchConditional %656 %657 %658
%657 = OpLabel
%660 = OpLoad %v3float %573
%661 = OpVectorShuffle %v3float %660 %660 1 2 0
OpStore %662 %661
%663 = OpLoad %float %sat
OpStore %664 %663
%665 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %662 %664
%666 = OpVectorShuffle %v3float %665 %665 2 0 1
OpReturnValue %666
%658 = OpLabel
%667 = OpLoad %v3float %573
%668 = OpVectorShuffle %v3float %667 %667 2 1 0
OpStore %669 %668
%670 = OpLoad %float %sat
OpStore %671 %670
%672 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %669 %671
%673 = OpVectorShuffle %v3float %672 %672 2 1 0
OpReturnValue %673
%659 = OpLabel
OpBranch %644
%644 = OpLabel
OpBranch %601
%601 = OpLabel
OpUnreachable
OpFunctionEnd
%blend = OpFunction %v4float None %675
%677 = OpFunctionParameter %_ptr_Function_int
%678 = OpFunctionParameter %_ptr_Function_v4float
%679 = OpFunctionParameter %_ptr_Function_v4float
%680 = OpLabel
%795 = OpVariable %_ptr_Function_v4float Function
%797 = OpVariable %_ptr_Function_v4float Function
%_9_result = OpVariable %_ptr_Function_v4float Function
%_10_result = OpVariable %_ptr_Function_v4float Function
%847 = OpVariable %_ptr_Function_v2float Function
%850 = OpVariable %_ptr_Function_v2float Function
%854 = OpVariable %_ptr_Function_v2float Function
%857 = OpVariable %_ptr_Function_v2float Function
%861 = OpVariable %_ptr_Function_v2float Function
%864 = OpVariable %_ptr_Function_v2float Function
%878 = OpVariable %_ptr_Function_v2float Function
%881 = OpVariable %_ptr_Function_v2float Function
%885 = OpVariable %_ptr_Function_v2float Function
%888 = OpVariable %_ptr_Function_v2float Function
%892 = OpVariable %_ptr_Function_v2float Function
%895 = OpVariable %_ptr_Function_v2float Function
%908 = OpVariable %_ptr_Function_v4float Function
%910 = OpVariable %_ptr_Function_v4float Function
%915 = OpVariable %_ptr_Function_v4float Function
%922 = OpVariable %_ptr_Function_v2float Function
%925 = OpVariable %_ptr_Function_v2float Function
%929 = OpVariable %_ptr_Function_v2float Function
%932 = OpVariable %_ptr_Function_v2float Function
%936 = OpVariable %_ptr_Function_v2float Function
%939 = OpVariable %_ptr_Function_v2float Function
%_11_alpha = OpVariable %_ptr_Function_float Function
%_12_sda = OpVariable %_ptr_Function_v3float Function
%_13_dsa = OpVariable %_ptr_Function_v3float Function
%1059 = OpVariable %_ptr_Function_v3float Function
%1061 = OpVariable %_ptr_Function_v3float Function
%1063 = OpVariable %_ptr_Function_v3float Function
%1065 = OpVariable %_ptr_Function_float Function
%1067 = OpVariable %_ptr_Function_v3float Function
%_14_alpha = OpVariable %_ptr_Function_float Function
%_15_sda = OpVariable %_ptr_Function_v3float Function
%_16_dsa = OpVariable %_ptr_Function_v3float Function
%1109 = OpVariable %_ptr_Function_v3float Function
%1111 = OpVariable %_ptr_Function_v3float Function
%1113 = OpVariable %_ptr_Function_v3float Function
%1115 = OpVariable %_ptr_Function_float Function
%1117 = OpVariable %_ptr_Function_v3float Function
%_17_alpha = OpVariable %_ptr_Function_float Function
%_18_sda = OpVariable %_ptr_Function_v3float Function
%_19_dsa = OpVariable %_ptr_Function_v3float Function
%1159 = OpVariable %_ptr_Function_v3float Function
%1161 = OpVariable %_ptr_Function_float Function
%1163 = OpVariable %_ptr_Function_v3float Function
%_20_alpha = OpVariable %_ptr_Function_float Function
%_21_sda = OpVariable %_ptr_Function_v3float Function
%_22_dsa = OpVariable %_ptr_Function_v3float Function
%1205 = OpVariable %_ptr_Function_v3float Function
%1207 = OpVariable %_ptr_Function_float Function
%1209 = OpVariable %_ptr_Function_v3float Function
%681 = OpLoad %int %677
OpSelectionMerge %682 None
OpSwitch %681 %712 0 %683 1 %684 2 %685 3 %686 4 %687 5 %688 6 %689 7 %690 8 %691 9 %692 10 %693 11 %694 12 %695 13 %696 14 %697 15 %698 16 %699 17 %700 18 %701 19 %702 20 %703 21 %704 22 %705 23 %706 24 %707 25 %708 26 %709 27 %710 28 %711
%683 = OpLabel
OpReturnValue %713
%684 = OpLabel
%714 = OpLoad %v4float %678
OpReturnValue %714
%685 = OpLabel
%715 = OpLoad %v4float %679
OpReturnValue %715
%686 = OpLabel
%716 = OpLoad %v4float %678
%717 = OpLoad %v4float %678
%718 = OpCompositeExtract %float %717 3
%719 = OpFSub %float %float_1 %718
%720 = OpLoad %v4float %679
%721 = OpVectorTimesScalar %v4float %720 %719
%722 = OpFAdd %v4float %716 %721
OpReturnValue %722
%687 = OpLabel
%723 = OpLoad %v4float %679
%724 = OpCompositeExtract %float %723 3
%725 = OpFSub %float %float_1 %724
%726 = OpLoad %v4float %678
%727 = OpVectorTimesScalar %v4float %726 %725
%728 = OpLoad %v4float %679
%729 = OpFAdd %v4float %727 %728
OpReturnValue %729
%688 = OpLabel
%730 = OpLoad %v4float %678
%731 = OpLoad %v4float %679
%732 = OpCompositeExtract %float %731 3
%733 = OpVectorTimesScalar %v4float %730 %732
OpReturnValue %733
%689 = OpLabel
%734 = OpLoad %v4float %679
%735 = OpLoad %v4float %678
%736 = OpCompositeExtract %float %735 3
%737 = OpVectorTimesScalar %v4float %734 %736
OpReturnValue %737
%690 = OpLabel
%738 = OpLoad %v4float %679
%739 = OpCompositeExtract %float %738 3
%740 = OpFSub %float %float_1 %739
%741 = OpLoad %v4float %678
%742 = OpVectorTimesScalar %v4float %741 %740
OpReturnValue %742
%691 = OpLabel
%743 = OpLoad %v4float %678
%744 = OpCompositeExtract %float %743 3
%745 = OpFSub %float %float_1 %744
%746 = OpLoad %v4float %679
%747 = OpVectorTimesScalar %v4float %746 %745
OpReturnValue %747
%692 = OpLabel
%748 = OpLoad %v4float %679
%749 = OpCompositeExtract %float %748 3
%750 = OpLoad %v4float %678
%751 = OpVectorTimesScalar %v4float %750 %749
%752 = OpLoad %v4float %678
%753 = OpCompositeExtract %float %752 3
%754 = OpFSub %float %float_1 %753
%755 = OpLoad %v4float %679
%756 = OpVectorTimesScalar %v4float %755 %754
%757 = OpFAdd %v4float %751 %756
OpReturnValue %757
%693 = OpLabel
%758 = OpLoad %v4float %679
%759 = OpCompositeExtract %float %758 3
%760 = OpFSub %float %float_1 %759
%761 = OpLoad %v4float %678
%762 = OpVectorTimesScalar %v4float %761 %760
%763 = OpLoad %v4float %678
%764 = OpCompositeExtract %float %763 3
%765 = OpLoad %v4float %679
%766 = OpVectorTimesScalar %v4float %765 %764
%767 = OpFAdd %v4float %762 %766
OpReturnValue %767
%694 = OpLabel
%768 = OpLoad %v4float %679
%769 = OpCompositeExtract %float %768 3
%770 = OpFSub %float %float_1 %769
%771 = OpLoad %v4float %678
%772 = OpVectorTimesScalar %v4float %771 %770
%773 = OpLoad %v4float %678
%774 = OpCompositeExtract %float %773 3
%775 = OpFSub %float %float_1 %774
%776 = OpLoad %v4float %679
%777 = OpVectorTimesScalar %v4float %776 %775
%778 = OpFAdd %v4float %772 %777
OpReturnValue %778
%695 = OpLabel
%780 = OpLoad %v4float %678
%781 = OpLoad %v4float %679
%782 = OpFAdd %v4float %780 %781
%783 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%779 = OpExtInst %v4float %1 FMin %782 %783
OpReturnValue %779
%696 = OpLabel
%784 = OpLoad %v4float %678
%785 = OpLoad %v4float %679
%786 = OpFMul %v4float %784 %785
OpReturnValue %786
%697 = OpLabel
%787 = OpLoad %v4float %678
%788 = OpLoad %v4float %678
%789 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%790 = OpFSub %v4float %789 %788
%791 = OpLoad %v4float %679
%792 = OpFMul %v4float %790 %791
%793 = OpFAdd %v4float %787 %792
OpReturnValue %793
%698 = OpLabel
%794 = OpLoad %v4float %678
OpStore %795 %794
%796 = OpLoad %v4float %679
OpStore %797 %796
%798 = OpFunctionCall %v4float %blend_overlay %795 %797
OpReturnValue %798
%699 = OpLabel
%800 = OpLoad %v4float %678
%801 = OpLoad %v4float %678
%802 = OpCompositeExtract %float %801 3
%803 = OpFSub %float %float_1 %802
%804 = OpLoad %v4float %679
%805 = OpVectorTimesScalar %v4float %804 %803
%806 = OpFAdd %v4float %800 %805
OpStore %_9_result %806
%808 = OpLoad %v4float %_9_result
%809 = OpVectorShuffle %v3float %808 %808 0 1 2
%810 = OpLoad %v4float %679
%811 = OpCompositeExtract %float %810 3
%812 = OpFSub %float %float_1 %811
%813 = OpLoad %v4float %678
%814 = OpVectorShuffle %v3float %813 %813 0 1 2
%815 = OpVectorTimesScalar %v3float %814 %812
%816 = OpLoad %v4float %679
%817 = OpVectorShuffle %v3float %816 %816 0 1 2
%818 = OpFAdd %v3float %815 %817
%807 = OpExtInst %v3float %1 FMin %809 %818
%819 = OpLoad %v4float %_9_result
%820 = OpVectorShuffle %v4float %819 %807 4 5 6 3
OpStore %_9_result %820
%821 = OpLoad %v4float %_9_result
OpReturnValue %821
%700 = OpLabel
%823 = OpLoad %v4float %678
%824 = OpLoad %v4float %678
%825 = OpCompositeExtract %float %824 3
%826 = OpFSub %float %float_1 %825
%827 = OpLoad %v4float %679
%828 = OpVectorTimesScalar %v4float %827 %826
%829 = OpFAdd %v4float %823 %828
OpStore %_10_result %829
%831 = OpLoad %v4float %_10_result
%832 = OpVectorShuffle %v3float %831 %831 0 1 2
%833 = OpLoad %v4float %679
%834 = OpCompositeExtract %float %833 3
%835 = OpFSub %float %float_1 %834
%836 = OpLoad %v4float %678
%837 = OpVectorShuffle %v3float %836 %836 0 1 2
%838 = OpVectorTimesScalar %v3float %837 %835
%839 = OpLoad %v4float %679
%840 = OpVectorShuffle %v3float %839 %839 0 1 2
%841 = OpFAdd %v3float %838 %840
%830 = OpExtInst %v3float %1 FMax %832 %841
%842 = OpLoad %v4float %_10_result
%843 = OpVectorShuffle %v4float %842 %830 4 5 6 3
OpStore %_10_result %843
%844 = OpLoad %v4float %_10_result
OpReturnValue %844
%701 = OpLabel
%845 = OpLoad %v4float %678
%846 = OpVectorShuffle %v2float %845 %845 0 3
OpStore %847 %846
%848 = OpLoad %v4float %679
%849 = OpVectorShuffle %v2float %848 %848 0 3
OpStore %850 %849
%851 = OpFunctionCall %float %_color_dodge_component %847 %850
%852 = OpLoad %v4float %678
%853 = OpVectorShuffle %v2float %852 %852 1 3
OpStore %854 %853
%855 = OpLoad %v4float %679
%856 = OpVectorShuffle %v2float %855 %855 1 3
OpStore %857 %856
%858 = OpFunctionCall %float %_color_dodge_component %854 %857
%859 = OpLoad %v4float %678
%860 = OpVectorShuffle %v2float %859 %859 2 3
OpStore %861 %860
%862 = OpLoad %v4float %679
%863 = OpVectorShuffle %v2float %862 %862 2 3
OpStore %864 %863
%865 = OpFunctionCall %float %_color_dodge_component %861 %864
%866 = OpLoad %v4float %678
%867 = OpCompositeExtract %float %866 3
%868 = OpLoad %v4float %678
%869 = OpCompositeExtract %float %868 3
%870 = OpFSub %float %float_1 %869
%871 = OpLoad %v4float %679
%872 = OpCompositeExtract %float %871 3
%873 = OpFMul %float %870 %872
%874 = OpFAdd %float %867 %873
%875 = OpCompositeConstruct %v4float %851 %858 %865 %874
OpReturnValue %875
%702 = OpLabel
%876 = OpLoad %v4float %678
%877 = OpVectorShuffle %v2float %876 %876 0 3
OpStore %878 %877
%879 = OpLoad %v4float %679
%880 = OpVectorShuffle %v2float %879 %879 0 3
OpStore %881 %880
%882 = OpFunctionCall %float %_color_burn_component %878 %881
%883 = OpLoad %v4float %678
%884 = OpVectorShuffle %v2float %883 %883 1 3
OpStore %885 %884
%886 = OpLoad %v4float %679
%887 = OpVectorShuffle %v2float %886 %886 1 3
OpStore %888 %887
%889 = OpFunctionCall %float %_color_burn_component %885 %888
%890 = OpLoad %v4float %678
%891 = OpVectorShuffle %v2float %890 %890 2 3
OpStore %892 %891
%893 = OpLoad %v4float %679
%894 = OpVectorShuffle %v2float %893 %893 2 3
OpStore %895 %894
%896 = OpFunctionCall %float %_color_burn_component %892 %895
%897 = OpLoad %v4float %678
%898 = OpCompositeExtract %float %897 3
%899 = OpLoad %v4float %678
%900 = OpCompositeExtract %float %899 3
%901 = OpFSub %float %float_1 %900
%902 = OpLoad %v4float %679
%903 = OpCompositeExtract %float %902 3
%904 = OpFMul %float %901 %903
%905 = OpFAdd %float %898 %904
%906 = OpCompositeConstruct %v4float %882 %889 %896 %905
OpReturnValue %906
%703 = OpLabel
%907 = OpLoad %v4float %679
OpStore %908 %907
%909 = OpLoad %v4float %678
OpStore %910 %909
%911 = OpFunctionCall %v4float %blend_overlay %908 %910
OpReturnValue %911
%704 = OpLabel
%912 = OpLoad %v4float %679
%913 = OpCompositeExtract %float %912 3
%914 = OpFOrdEqual %bool %913 %float_0
OpSelectionMerge %918 None
OpBranchConditional %914 %916 %917
%916 = OpLabel
%919 = OpLoad %v4float %678
OpStore %915 %919
OpBranch %918
%917 = OpLabel
%920 = OpLoad %v4float %678
%921 = OpVectorShuffle %v2float %920 %920 0 3
OpStore %922 %921
%923 = OpLoad %v4float %679
%924 = OpVectorShuffle %v2float %923 %923 0 3
OpStore %925 %924
%926 = OpFunctionCall %float %_soft_light_component %922 %925
%927 = OpLoad %v4float %678
%928 = OpVectorShuffle %v2float %927 %927 1 3
OpStore %929 %928
%930 = OpLoad %v4float %679
%931 = OpVectorShuffle %v2float %930 %930 1 3
OpStore %932 %931
%933 = OpFunctionCall %float %_soft_light_component %929 %932
%934 = OpLoad %v4float %678
%935 = OpVectorShuffle %v2float %934 %934 2 3
OpStore %936 %935
%937 = OpLoad %v4float %679
%938 = OpVectorShuffle %v2float %937 %937 2 3
OpStore %939 %938
%940 = OpFunctionCall %float %_soft_light_component %936 %939
%941 = OpLoad %v4float %678
%942 = OpCompositeExtract %float %941 3
%943 = OpLoad %v4float %678
%944 = OpCompositeExtract %float %943 3
%945 = OpFSub %float %float_1 %944
%946 = OpLoad %v4float %679
%947 = OpCompositeExtract %float %946 3
%948 = OpFMul %float %945 %947
%949 = OpFAdd %float %942 %948
%950 = OpCompositeConstruct %v4float %926 %933 %940 %949
OpStore %915 %950
OpBranch %918
%918 = OpLabel
%951 = OpLoad %v4float %915
OpReturnValue %951
%705 = OpLabel
%952 = OpLoad %v4float %678
%953 = OpVectorShuffle %v3float %952 %952 0 1 2
%954 = OpLoad %v4float %679
%955 = OpVectorShuffle %v3float %954 %954 0 1 2
%956 = OpFAdd %v3float %953 %955
%958 = OpLoad %v4float %678
%959 = OpVectorShuffle %v3float %958 %958 0 1 2
%960 = OpLoad %v4float %679
%961 = OpCompositeExtract %float %960 3
%962 = OpVectorTimesScalar %v3float %959 %961
%963 = OpLoad %v4float %679
%964 = OpVectorShuffle %v3float %963 %963 0 1 2
%965 = OpLoad %v4float %678
%966 = OpCompositeExtract %float %965 3
%967 = OpVectorTimesScalar %v3float %964 %966
%957 = OpExtInst %v3float %1 FMin %962 %967
%968 = OpVectorTimesScalar %v3float %957 %float_2
%969 = OpFSub %v3float %956 %968
%970 = OpCompositeExtract %float %969 0
%971 = OpCompositeExtract %float %969 1
%972 = OpCompositeExtract %float %969 2
%973 = OpLoad %v4float %678
%974 = OpCompositeExtract %float %973 3
%975 = OpLoad %v4float %678
%976 = OpCompositeExtract %float %975 3
%977 = OpFSub %float %float_1 %976
%978 = OpLoad %v4float %679
%979 = OpCompositeExtract %float %978 3
%980 = OpFMul %float %977 %979
%981 = OpFAdd %float %974 %980
%982 = OpCompositeConstruct %v4float %970 %971 %972 %981
OpReturnValue %982
%706 = OpLabel
%983 = OpLoad %v4float %679
%984 = OpVectorShuffle %v3float %983 %983 0 1 2
%985 = OpLoad %v4float %678
%986 = OpVectorShuffle %v3float %985 %985 0 1 2
%987 = OpFAdd %v3float %984 %986
%988 = OpLoad %v4float %679
%989 = OpVectorShuffle %v3float %988 %988 0 1 2
%990 = OpVectorTimesScalar %v3float %989 %float_2
%991 = OpLoad %v4float %678
%992 = OpVectorShuffle %v3float %991 %991 0 1 2
%993 = OpFMul %v3float %990 %992
%994 = OpFSub %v3float %987 %993
%995 = OpCompositeExtract %float %994 0
%996 = OpCompositeExtract %float %994 1
%997 = OpCompositeExtract %float %994 2
%998 = OpLoad %v4float %678
%999 = OpCompositeExtract %float %998 3
%1000 = OpLoad %v4float %678
%1001 = OpCompositeExtract %float %1000 3
%1002 = OpFSub %float %float_1 %1001
%1003 = OpLoad %v4float %679
%1004 = OpCompositeExtract %float %1003 3
%1005 = OpFMul %float %1002 %1004
%1006 = OpFAdd %float %999 %1005
%1007 = OpCompositeConstruct %v4float %995 %996 %997 %1006
OpReturnValue %1007
%707 = OpLabel
%1008 = OpLoad %v4float %678
%1009 = OpCompositeExtract %float %1008 3
%1010 = OpFSub %float %float_1 %1009
%1011 = OpLoad %v4float %679
%1012 = OpVectorShuffle %v3float %1011 %1011 0 1 2
%1013 = OpVectorTimesScalar %v3float %1012 %1010
%1014 = OpLoad %v4float %679
%1015 = OpCompositeExtract %float %1014 3
%1016 = OpFSub %float %float_1 %1015
%1017 = OpLoad %v4float %678
%1018 = OpVectorShuffle %v3float %1017 %1017 0 1 2
%1019 = OpVectorTimesScalar %v3float %1018 %1016
%1020 = OpFAdd %v3float %1013 %1019
%1021 = OpLoad %v4float %678
%1022 = OpVectorShuffle %v3float %1021 %1021 0 1 2
%1023 = OpLoad %v4float %679
%1024 = OpVectorShuffle %v3float %1023 %1023 0 1 2
%1025 = OpFMul %v3float %1022 %1024
%1026 = OpFAdd %v3float %1020 %1025
%1027 = OpCompositeExtract %float %1026 0
%1028 = OpCompositeExtract %float %1026 1
%1029 = OpCompositeExtract %float %1026 2
%1030 = OpLoad %v4float %678
%1031 = OpCompositeExtract %float %1030 3
%1032 = OpLoad %v4float %678
%1033 = OpCompositeExtract %float %1032 3
%1034 = OpFSub %float %float_1 %1033
%1035 = OpLoad %v4float %679
%1036 = OpCompositeExtract %float %1035 3
%1037 = OpFMul %float %1034 %1036
%1038 = OpFAdd %float %1031 %1037
%1039 = OpCompositeConstruct %v4float %1027 %1028 %1029 %1038
OpReturnValue %1039
%708 = OpLabel
%1041 = OpLoad %v4float %679
%1042 = OpCompositeExtract %float %1041 3
%1043 = OpLoad %v4float %678
%1044 = OpCompositeExtract %float %1043 3
%1045 = OpFMul %float %1042 %1044
OpStore %_11_alpha %1045
%1047 = OpLoad %v4float %678
%1048 = OpVectorShuffle %v3float %1047 %1047 0 1 2
%1049 = OpLoad %v4float %679
%1050 = OpCompositeExtract %float %1049 3
%1051 = OpVectorTimesScalar %v3float %1048 %1050
OpStore %_12_sda %1051
%1053 = OpLoad %v4float %679
%1054 = OpVectorShuffle %v3float %1053 %1053 0 1 2
%1055 = OpLoad %v4float %678
%1056 = OpCompositeExtract %float %1055 3
%1057 = OpVectorTimesScalar %v3float %1054 %1056
OpStore %_13_dsa %1057
%1058 = OpLoad %v3float %_12_sda
OpStore %1059 %1058
%1060 = OpLoad %v3float %_13_dsa
OpStore %1061 %1060
%1062 = OpFunctionCall %v3float %_blend_set_color_saturation %1059 %1061
OpStore %1063 %1062
%1064 = OpLoad %float %_11_alpha
OpStore %1065 %1064
%1066 = OpLoad %v3float %_13_dsa
OpStore %1067 %1066
%1068 = OpFunctionCall %v3float %_blend_set_color_luminance %1063 %1065 %1067
%1069 = OpLoad %v4float %679
%1070 = OpVectorShuffle %v3float %1069 %1069 0 1 2
%1071 = OpFAdd %v3float %1068 %1070
%1072 = OpLoad %v3float %_13_dsa
%1073 = OpFSub %v3float %1071 %1072
%1074 = OpLoad %v4float %678
%1075 = OpVectorShuffle %v3float %1074 %1074 0 1 2
%1076 = OpFAdd %v3float %1073 %1075
%1077 = OpLoad %v3float %_12_sda
%1078 = OpFSub %v3float %1076 %1077
%1079 = OpCompositeExtract %float %1078 0
%1080 = OpCompositeExtract %float %1078 1
%1081 = OpCompositeExtract %float %1078 2
%1082 = OpLoad %v4float %678
%1083 = OpCompositeExtract %float %1082 3
%1084 = OpLoad %v4float %679
%1085 = OpCompositeExtract %float %1084 3
%1086 = OpFAdd %float %1083 %1085
%1087 = OpLoad %float %_11_alpha
%1088 = OpFSub %float %1086 %1087
%1089 = OpCompositeConstruct %v4float %1079 %1080 %1081 %1088
OpReturnValue %1089
%709 = OpLabel
%1091 = OpLoad %v4float %679
%1092 = OpCompositeExtract %float %1091 3
%1093 = OpLoad %v4float %678
%1094 = OpCompositeExtract %float %1093 3
%1095 = OpFMul %float %1092 %1094
OpStore %_14_alpha %1095
%1097 = OpLoad %v4float %678
%1098 = OpVectorShuffle %v3float %1097 %1097 0 1 2
%1099 = OpLoad %v4float %679
%1100 = OpCompositeExtract %float %1099 3
%1101 = OpVectorTimesScalar %v3float %1098 %1100
OpStore %_15_sda %1101
%1103 = OpLoad %v4float %679
%1104 = OpVectorShuffle %v3float %1103 %1103 0 1 2
%1105 = OpLoad %v4float %678
%1106 = OpCompositeExtract %float %1105 3
%1107 = OpVectorTimesScalar %v3float %1104 %1106
OpStore %_16_dsa %1107
%1108 = OpLoad %v3float %_16_dsa
OpStore %1109 %1108
%1110 = OpLoad %v3float %_15_sda
OpStore %1111 %1110
%1112 = OpFunctionCall %v3float %_blend_set_color_saturation %1109 %1111
OpStore %1113 %1112
%1114 = OpLoad %float %_14_alpha
OpStore %1115 %1114
%1116 = OpLoad %v3float %_16_dsa
OpStore %1117 %1116
%1118 = OpFunctionCall %v3float %_blend_set_color_luminance %1113 %1115 %1117
%1119 = OpLoad %v4float %679
%1120 = OpVectorShuffle %v3float %1119 %1119 0 1 2
%1121 = OpFAdd %v3float %1118 %1120
%1122 = OpLoad %v3float %_16_dsa
%1123 = OpFSub %v3float %1121 %1122
%1124 = OpLoad %v4float %678
%1125 = OpVectorShuffle %v3float %1124 %1124 0 1 2
%1126 = OpFAdd %v3float %1123 %1125
%1127 = OpLoad %v3float %_15_sda
%1128 = OpFSub %v3float %1126 %1127
%1129 = OpCompositeExtract %float %1128 0
%1130 = OpCompositeExtract %float %1128 1
%1131 = OpCompositeExtract %float %1128 2
%1132 = OpLoad %v4float %678
%1133 = OpCompositeExtract %float %1132 3
%1134 = OpLoad %v4float %679
%1135 = OpCompositeExtract %float %1134 3
%1136 = OpFAdd %float %1133 %1135
%1137 = OpLoad %float %_14_alpha
%1138 = OpFSub %float %1136 %1137
%1139 = OpCompositeConstruct %v4float %1129 %1130 %1131 %1138
OpReturnValue %1139
%710 = OpLabel
%1141 = OpLoad %v4float %679
%1142 = OpCompositeExtract %float %1141 3
%1143 = OpLoad %v4float %678
%1144 = OpCompositeExtract %float %1143 3
%1145 = OpFMul %float %1142 %1144
OpStore %_17_alpha %1145
%1147 = OpLoad %v4float %678
%1148 = OpVectorShuffle %v3float %1147 %1147 0 1 2
%1149 = OpLoad %v4float %679
%1150 = OpCompositeExtract %float %1149 3
%1151 = OpVectorTimesScalar %v3float %1148 %1150
OpStore %_18_sda %1151
%1153 = OpLoad %v4float %679
%1154 = OpVectorShuffle %v3float %1153 %1153 0 1 2
%1155 = OpLoad %v4float %678
%1156 = OpCompositeExtract %float %1155 3
%1157 = OpVectorTimesScalar %v3float %1154 %1156
OpStore %_19_dsa %1157
%1158 = OpLoad %v3float %_18_sda
OpStore %1159 %1158
%1160 = OpLoad %float %_17_alpha
OpStore %1161 %1160
%1162 = OpLoad %v3float %_19_dsa
OpStore %1163 %1162
%1164 = OpFunctionCall %v3float %_blend_set_color_luminance %1159 %1161 %1163
%1165 = OpLoad %v4float %679
%1166 = OpVectorShuffle %v3float %1165 %1165 0 1 2
%1167 = OpFAdd %v3float %1164 %1166
%1168 = OpLoad %v3float %_19_dsa
%1169 = OpFSub %v3float %1167 %1168
%1170 = OpLoad %v4float %678
%1171 = OpVectorShuffle %v3float %1170 %1170 0 1 2
%1172 = OpFAdd %v3float %1169 %1171
%1173 = OpLoad %v3float %_18_sda
%1174 = OpFSub %v3float %1172 %1173
%1175 = OpCompositeExtract %float %1174 0
%1176 = OpCompositeExtract %float %1174 1
%1177 = OpCompositeExtract %float %1174 2
%1178 = OpLoad %v4float %678
%1179 = OpCompositeExtract %float %1178 3
%1180 = OpLoad %v4float %679
%1181 = OpCompositeExtract %float %1180 3
%1182 = OpFAdd %float %1179 %1181
%1183 = OpLoad %float %_17_alpha
%1184 = OpFSub %float %1182 %1183
%1185 = OpCompositeConstruct %v4float %1175 %1176 %1177 %1184
OpReturnValue %1185
%711 = OpLabel
%1187 = OpLoad %v4float %679
%1188 = OpCompositeExtract %float %1187 3
%1189 = OpLoad %v4float %678
%1190 = OpCompositeExtract %float %1189 3
%1191 = OpFMul %float %1188 %1190
OpStore %_20_alpha %1191
%1193 = OpLoad %v4float %678
%1194 = OpVectorShuffle %v3float %1193 %1193 0 1 2
%1195 = OpLoad %v4float %679
%1196 = OpCompositeExtract %float %1195 3
%1197 = OpVectorTimesScalar %v3float %1194 %1196
OpStore %_21_sda %1197
%1199 = OpLoad %v4float %679
%1200 = OpVectorShuffle %v3float %1199 %1199 0 1 2
%1201 = OpLoad %v4float %678
%1202 = OpCompositeExtract %float %1201 3
%1203 = OpVectorTimesScalar %v3float %1200 %1202
OpStore %_22_dsa %1203
%1204 = OpLoad %v3float %_22_dsa
OpStore %1205 %1204
%1206 = OpLoad %float %_20_alpha
OpStore %1207 %1206
%1208 = OpLoad %v3float %_21_sda
OpStore %1209 %1208
%1210 = OpFunctionCall %v3float %_blend_set_color_luminance %1205 %1207 %1209
%1211 = OpLoad %v4float %679
%1212 = OpVectorShuffle %v3float %1211 %1211 0 1 2
%1213 = OpFAdd %v3float %1210 %1212
%1214 = OpLoad %v3float %_22_dsa
%1215 = OpFSub %v3float %1213 %1214
%1216 = OpLoad %v4float %678
%1217 = OpVectorShuffle %v3float %1216 %1216 0 1 2
%1218 = OpFAdd %v3float %1215 %1217
%1219 = OpLoad %v3float %_21_sda
%1220 = OpFSub %v3float %1218 %1219
%1221 = OpCompositeExtract %float %1220 0
%1222 = OpCompositeExtract %float %1220 1
%1223 = OpCompositeExtract %float %1220 2
%1224 = OpLoad %v4float %678
%1225 = OpCompositeExtract %float %1224 3
%1226 = OpLoad %v4float %679
%1227 = OpCompositeExtract %float %1226 3
%1228 = OpFAdd %float %1225 %1227
%1229 = OpLoad %float %_20_alpha
%1230 = OpFSub %float %1228 %1229
%1231 = OpCompositeConstruct %v4float %1221 %1222 %1223 %1230
OpReturnValue %1231
%712 = OpLabel
OpReturnValue %713
%682 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %1233
%1234 = OpLabel
%1236 = OpVariable %_ptr_Function_int Function
%1241 = OpVariable %_ptr_Function_v4float Function
%1245 = OpVariable %_ptr_Function_v4float Function
OpStore %1236 %int_13
%1237 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%1240 = OpLoad %v4float %1237
OpStore %1241 %1240
%1242 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%1244 = OpLoad %v4float %1242
OpStore %1245 %1244
%1246 = OpFunctionCall %v4float %blend %1236 %1241 %1245
OpStore %sk_FragColor %1246
OpReturn
OpFunctionEnd
