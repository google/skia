### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

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
OpName %blend_darken "blend_darken"
OpName %result_0 "result"
OpName %blend_lighten "blend_lighten"
OpName %result_1 "result"
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
OpName %result_2 "result"
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
OpName %blend_hue "blend_hue"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %blend_saturation "blend_saturation"
OpName %alpha_0 "alpha"
OpName %sda_0 "sda"
OpName %dsa_0 "dsa"
OpName %blend_color "blend_color"
OpName %alpha_1 "alpha"
OpName %sda_1 "sda"
OpName %dsa_1 "dsa"
OpName %blend_luminosity "blend_luminosity"
OpName %alpha_2 "alpha"
OpName %sda_2 "sda"
OpName %dsa_2 "dsa"
OpName %blend "blend"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %535 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %540 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %554 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %559 RelaxedPrecision
OpDecorate %560 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %570 RelaxedPrecision
OpDecorate %571 RelaxedPrecision
OpDecorate %578 RelaxedPrecision
OpDecorate %579 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %584 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %596 RelaxedPrecision
OpDecorate %597 RelaxedPrecision
OpDecorate %602 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %616 RelaxedPrecision
OpDecorate %617 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
OpDecorate %625 RelaxedPrecision
OpDecorate %626 RelaxedPrecision
OpDecorate %627 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %639 RelaxedPrecision
OpDecorate %641 RelaxedPrecision
OpDecorate %645 RelaxedPrecision
OpDecorate %647 RelaxedPrecision
OpDecorate %649 RelaxedPrecision
OpDecorate %651 RelaxedPrecision
OpDecorate %652 RelaxedPrecision
OpDecorate %654 RelaxedPrecision
OpDecorate %660 RelaxedPrecision
OpDecorate %662 RelaxedPrecision
OpDecorate %668 RelaxedPrecision
OpDecorate %670 RelaxedPrecision
OpDecorate %673 RelaxedPrecision
OpDecorate %675 RelaxedPrecision
OpDecorate %681 RelaxedPrecision
OpDecorate %684 RelaxedPrecision
OpDecorate %688 RelaxedPrecision
OpDecorate %691 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %703 RelaxedPrecision
OpDecorate %706 RelaxedPrecision
OpDecorate %710 RelaxedPrecision
OpDecorate %712 RelaxedPrecision
OpDecorate %718 RelaxedPrecision
OpDecorate %721 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %740 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %750 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %761 RelaxedPrecision
OpDecorate %764 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %769 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %772 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %777 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %781 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %795 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %817 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %850 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %856 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %861 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %875 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %883 RelaxedPrecision
OpDecorate %884 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %893 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %897 RelaxedPrecision
OpDecorate %899 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %908 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %915 RelaxedPrecision
OpDecorate %917 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %920 RelaxedPrecision
OpDecorate %922 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %924 RelaxedPrecision
OpDecorate %928 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %932 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %976 RelaxedPrecision
OpDecorate %977 RelaxedPrecision
OpDecorate %978 RelaxedPrecision
OpDecorate %979 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %984 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %992 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %996 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1007 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1017 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1020 RelaxedPrecision
OpDecorate %1022 RelaxedPrecision
OpDecorate %1023 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1035 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1045 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1047 RelaxedPrecision
OpDecorate %1048 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1050 RelaxedPrecision
OpDecorate %1052 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1078 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1094 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1100 RelaxedPrecision
OpDecorate %1102 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1109 RelaxedPrecision
OpDecorate %1112 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1119 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1125 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1135 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1145 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1156 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1171 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1177 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1189 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1204 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1207 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1211 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1217 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1220 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1229 RelaxedPrecision
OpDecorate %1231 RelaxedPrecision
OpDecorate %1232 RelaxedPrecision
OpDecorate %1234 RelaxedPrecision
OpDecorate %1236 RelaxedPrecision
OpDecorate %1237 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1242 RelaxedPrecision
OpDecorate %1243 RelaxedPrecision
OpDecorate %1246 RelaxedPrecision
OpDecorate %1247 RelaxedPrecision
OpDecorate %1249 RelaxedPrecision
OpDecorate %1251 RelaxedPrecision
OpDecorate %1252 RelaxedPrecision
OpDecorate %1256 RelaxedPrecision
OpDecorate %1258 RelaxedPrecision
OpDecorate %1260 RelaxedPrecision
OpDecorate %1261 RelaxedPrecision
OpDecorate %1263 RelaxedPrecision
OpDecorate %1264 RelaxedPrecision
OpDecorate %1266 RelaxedPrecision
OpDecorate %1268 RelaxedPrecision
OpDecorate %1271 RelaxedPrecision
OpDecorate %1273 RelaxedPrecision
OpDecorate %1276 RelaxedPrecision
OpDecorate %1278 RelaxedPrecision
OpDecorate %1281 RelaxedPrecision
OpDecorate %1283 RelaxedPrecision
OpDecorate %1291 RelaxedPrecision
OpDecorate %1293 RelaxedPrecision
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
%29 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%71 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%float_0 = OpConstant %float 0
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v3float = OpTypePointer Function %v3float
%500 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%511 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%598 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%629 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%630 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%937 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%975 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1287 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%_blend_overlay_component = OpFunction %float None %29
%31 = OpFunctionParameter %_ptr_Function_v2float
%32 = OpFunctionParameter %_ptr_Function_v2float
%33 = OpLabel
%41 = OpVariable %_ptr_Function_float Function
%35 = OpLoad %v2float %32
%36 = OpCompositeExtract %float %35 0
%37 = OpFMul %float %float_2 %36
%38 = OpLoad %v2float %32
%39 = OpCompositeExtract %float %38 1
%40 = OpFOrdLessThanEqual %bool %37 %39
OpSelectionMerge %45 None
OpBranchConditional %40 %43 %44
%43 = OpLabel
%46 = OpLoad %v2float %31
%47 = OpCompositeExtract %float %46 0
%48 = OpFMul %float %float_2 %47
%49 = OpLoad %v2float %32
%50 = OpCompositeExtract %float %49 0
%51 = OpFMul %float %48 %50
OpStore %41 %51
OpBranch %45
%44 = OpLabel
%52 = OpLoad %v2float %31
%53 = OpCompositeExtract %float %52 1
%54 = OpLoad %v2float %32
%55 = OpCompositeExtract %float %54 1
%56 = OpFMul %float %53 %55
%57 = OpLoad %v2float %32
%58 = OpCompositeExtract %float %57 1
%59 = OpLoad %v2float %32
%60 = OpCompositeExtract %float %59 0
%61 = OpFSub %float %58 %60
%62 = OpFMul %float %float_2 %61
%63 = OpLoad %v2float %31
%64 = OpCompositeExtract %float %63 1
%65 = OpLoad %v2float %31
%66 = OpCompositeExtract %float %65 0
%67 = OpFSub %float %64 %66
%68 = OpFMul %float %62 %67
%69 = OpFSub %float %56 %68
OpStore %41 %69
OpBranch %45
%45 = OpLabel
%70 = OpLoad %float %41
OpReturnValue %70
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %71
%73 = OpFunctionParameter %_ptr_Function_v4float
%74 = OpFunctionParameter %_ptr_Function_v4float
%75 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%79 = OpVariable %_ptr_Function_v2float Function
%82 = OpVariable %_ptr_Function_v2float Function
%86 = OpVariable %_ptr_Function_v2float Function
%89 = OpVariable %_ptr_Function_v2float Function
%93 = OpVariable %_ptr_Function_v2float Function
%96 = OpVariable %_ptr_Function_v2float Function
%77 = OpLoad %v4float %73
%78 = OpVectorShuffle %v2float %77 %77 0 3
OpStore %79 %78
%80 = OpLoad %v4float %74
%81 = OpVectorShuffle %v2float %80 %80 0 3
OpStore %82 %81
%83 = OpFunctionCall %float %_blend_overlay_component %79 %82
%84 = OpLoad %v4float %73
%85 = OpVectorShuffle %v2float %84 %84 1 3
OpStore %86 %85
%87 = OpLoad %v4float %74
%88 = OpVectorShuffle %v2float %87 %87 1 3
OpStore %89 %88
%90 = OpFunctionCall %float %_blend_overlay_component %86 %89
%91 = OpLoad %v4float %73
%92 = OpVectorShuffle %v2float %91 %91 2 3
OpStore %93 %92
%94 = OpLoad %v4float %74
%95 = OpVectorShuffle %v2float %94 %94 2 3
OpStore %96 %95
%97 = OpFunctionCall %float %_blend_overlay_component %93 %96
%98 = OpLoad %v4float %73
%99 = OpCompositeExtract %float %98 3
%101 = OpLoad %v4float %73
%102 = OpCompositeExtract %float %101 3
%103 = OpFSub %float %float_1 %102
%104 = OpLoad %v4float %74
%105 = OpCompositeExtract %float %104 3
%106 = OpFMul %float %103 %105
%107 = OpFAdd %float %99 %106
%108 = OpCompositeConstruct %v4float %83 %90 %97 %107
OpStore %result %108
%109 = OpLoad %v4float %result
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%112 = OpLoad %v4float %74
%113 = OpVectorShuffle %v3float %112 %112 0 1 2
%114 = OpLoad %v4float %73
%115 = OpCompositeExtract %float %114 3
%116 = OpFSub %float %float_1 %115
%117 = OpVectorTimesScalar %v3float %113 %116
%118 = OpLoad %v4float %73
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%120 = OpLoad %v4float %74
%121 = OpCompositeExtract %float %120 3
%122 = OpFSub %float %float_1 %121
%123 = OpVectorTimesScalar %v3float %119 %122
%124 = OpFAdd %v3float %117 %123
%125 = OpFAdd %v3float %110 %124
%126 = OpLoad %v4float %result
%127 = OpVectorShuffle %v4float %126 %125 4 5 6 3
OpStore %result %127
%128 = OpLoad %v4float %result
OpReturnValue %128
OpFunctionEnd
%blend_darken = OpFunction %v4float None %71
%129 = OpFunctionParameter %_ptr_Function_v4float
%130 = OpFunctionParameter %_ptr_Function_v4float
%131 = OpLabel
%result_0 = OpVariable %_ptr_Function_v4float Function
%133 = OpLoad %v4float %129
%134 = OpLoad %v4float %129
%135 = OpCompositeExtract %float %134 3
%136 = OpFSub %float %float_1 %135
%137 = OpLoad %v4float %130
%138 = OpVectorTimesScalar %v4float %137 %136
%139 = OpFAdd %v4float %133 %138
OpStore %result_0 %139
%141 = OpLoad %v4float %result_0
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%143 = OpLoad %v4float %130
%144 = OpCompositeExtract %float %143 3
%145 = OpFSub %float %float_1 %144
%146 = OpLoad %v4float %129
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpVectorTimesScalar %v3float %147 %145
%149 = OpLoad %v4float %130
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%151 = OpFAdd %v3float %148 %150
%140 = OpExtInst %v3float %1 FMin %142 %151
%152 = OpLoad %v4float %result_0
%153 = OpVectorShuffle %v4float %152 %140 4 5 6 3
OpStore %result_0 %153
%154 = OpLoad %v4float %result_0
OpReturnValue %154
OpFunctionEnd
%blend_lighten = OpFunction %v4float None %71
%155 = OpFunctionParameter %_ptr_Function_v4float
%156 = OpFunctionParameter %_ptr_Function_v4float
%157 = OpLabel
%result_1 = OpVariable %_ptr_Function_v4float Function
%159 = OpLoad %v4float %155
%160 = OpLoad %v4float %155
%161 = OpCompositeExtract %float %160 3
%162 = OpFSub %float %float_1 %161
%163 = OpLoad %v4float %156
%164 = OpVectorTimesScalar %v4float %163 %162
%165 = OpFAdd %v4float %159 %164
OpStore %result_1 %165
%167 = OpLoad %v4float %result_1
%168 = OpVectorShuffle %v3float %167 %167 0 1 2
%169 = OpLoad %v4float %156
%170 = OpCompositeExtract %float %169 3
%171 = OpFSub %float %float_1 %170
%172 = OpLoad %v4float %155
%173 = OpVectorShuffle %v3float %172 %172 0 1 2
%174 = OpVectorTimesScalar %v3float %173 %171
%175 = OpLoad %v4float %156
%176 = OpVectorShuffle %v3float %175 %175 0 1 2
%177 = OpFAdd %v3float %174 %176
%166 = OpExtInst %v3float %1 FMax %168 %177
%178 = OpLoad %v4float %result_1
%179 = OpVectorShuffle %v4float %178 %166 4 5 6 3
OpStore %result_1 %179
%180 = OpLoad %v4float %result_1
OpReturnValue %180
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %29
%181 = OpFunctionParameter %_ptr_Function_v2float
%182 = OpFunctionParameter %_ptr_Function_v2float
%183 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_0_n = OpVariable %_ptr_Function_float Function
%184 = OpLoad %v2float %182
%185 = OpCompositeExtract %float %184 0
%187 = OpFOrdEqual %bool %185 %float_0
OpSelectionMerge %190 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%191 = OpLoad %v2float %181
%192 = OpCompositeExtract %float %191 0
%193 = OpLoad %v2float %182
%194 = OpCompositeExtract %float %193 1
%195 = OpFSub %float %float_1 %194
%196 = OpFMul %float %192 %195
OpReturnValue %196
%189 = OpLabel
%198 = OpLoad %v2float %181
%199 = OpCompositeExtract %float %198 1
%200 = OpLoad %v2float %181
%201 = OpCompositeExtract %float %200 0
%202 = OpFSub %float %199 %201
OpStore %delta %202
%203 = OpLoad %float %delta
%204 = OpFOrdEqual %bool %203 %float_0
OpSelectionMerge %207 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%208 = OpLoad %v2float %181
%209 = OpCompositeExtract %float %208 1
%210 = OpLoad %v2float %182
%211 = OpCompositeExtract %float %210 1
%212 = OpFMul %float %209 %211
%213 = OpLoad %v2float %181
%214 = OpCompositeExtract %float %213 0
%215 = OpLoad %v2float %182
%216 = OpCompositeExtract %float %215 1
%217 = OpFSub %float %float_1 %216
%218 = OpFMul %float %214 %217
%219 = OpFAdd %float %212 %218
%220 = OpLoad %v2float %182
%221 = OpCompositeExtract %float %220 0
%222 = OpLoad %v2float %181
%223 = OpCompositeExtract %float %222 1
%224 = OpFSub %float %float_1 %223
%225 = OpFMul %float %221 %224
%226 = OpFAdd %float %219 %225
OpReturnValue %226
%206 = OpLabel
%228 = OpLoad %v2float %182
%229 = OpCompositeExtract %float %228 0
%230 = OpLoad %v2float %181
%231 = OpCompositeExtract %float %230 1
%232 = OpFMul %float %229 %231
OpStore %_0_n %232
%234 = OpLoad %v2float %182
%235 = OpCompositeExtract %float %234 1
%236 = OpLoad %float %_0_n
%237 = OpLoad %float %delta
%238 = OpFDiv %float %236 %237
%233 = OpExtInst %float %1 FMin %235 %238
OpStore %delta %233
%239 = OpLoad %float %delta
%240 = OpLoad %v2float %181
%241 = OpCompositeExtract %float %240 1
%242 = OpFMul %float %239 %241
%243 = OpLoad %v2float %181
%244 = OpCompositeExtract %float %243 0
%245 = OpLoad %v2float %182
%246 = OpCompositeExtract %float %245 1
%247 = OpFSub %float %float_1 %246
%248 = OpFMul %float %244 %247
%249 = OpFAdd %float %242 %248
%250 = OpLoad %v2float %182
%251 = OpCompositeExtract %float %250 0
%252 = OpLoad %v2float %181
%253 = OpCompositeExtract %float %252 1
%254 = OpFSub %float %float_1 %253
%255 = OpFMul %float %251 %254
%256 = OpFAdd %float %249 %255
OpReturnValue %256
%207 = OpLabel
OpBranch %190
%190 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %29
%257 = OpFunctionParameter %_ptr_Function_v2float
%258 = OpFunctionParameter %_ptr_Function_v2float
%259 = OpLabel
%_1_n = OpVariable %_ptr_Function_float Function
%delta_0 = OpVariable %_ptr_Function_float Function
%260 = OpLoad %v2float %258
%261 = OpCompositeExtract %float %260 1
%262 = OpLoad %v2float %258
%263 = OpCompositeExtract %float %262 0
%264 = OpFOrdEqual %bool %261 %263
OpSelectionMerge %267 None
OpBranchConditional %264 %265 %266
%265 = OpLabel
%268 = OpLoad %v2float %257
%269 = OpCompositeExtract %float %268 1
%270 = OpLoad %v2float %258
%271 = OpCompositeExtract %float %270 1
%272 = OpFMul %float %269 %271
%273 = OpLoad %v2float %257
%274 = OpCompositeExtract %float %273 0
%275 = OpLoad %v2float %258
%276 = OpCompositeExtract %float %275 1
%277 = OpFSub %float %float_1 %276
%278 = OpFMul %float %274 %277
%279 = OpFAdd %float %272 %278
%280 = OpLoad %v2float %258
%281 = OpCompositeExtract %float %280 0
%282 = OpLoad %v2float %257
%283 = OpCompositeExtract %float %282 1
%284 = OpFSub %float %float_1 %283
%285 = OpFMul %float %281 %284
%286 = OpFAdd %float %279 %285
OpReturnValue %286
%266 = OpLabel
%287 = OpLoad %v2float %257
%288 = OpCompositeExtract %float %287 0
%289 = OpFOrdEqual %bool %288 %float_0
OpSelectionMerge %292 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%293 = OpLoad %v2float %258
%294 = OpCompositeExtract %float %293 0
%295 = OpLoad %v2float %257
%296 = OpCompositeExtract %float %295 1
%297 = OpFSub %float %float_1 %296
%298 = OpFMul %float %294 %297
OpReturnValue %298
%291 = OpLabel
%300 = OpLoad %v2float %258
%301 = OpCompositeExtract %float %300 1
%302 = OpLoad %v2float %258
%303 = OpCompositeExtract %float %302 0
%304 = OpFSub %float %301 %303
%305 = OpLoad %v2float %257
%306 = OpCompositeExtract %float %305 1
%307 = OpFMul %float %304 %306
OpStore %_1_n %307
%310 = OpLoad %v2float %258
%311 = OpCompositeExtract %float %310 1
%312 = OpLoad %float %_1_n
%313 = OpLoad %v2float %257
%314 = OpCompositeExtract %float %313 0
%315 = OpFDiv %float %312 %314
%316 = OpFSub %float %311 %315
%309 = OpExtInst %float %1 FMax %float_0 %316
OpStore %delta_0 %309
%317 = OpLoad %float %delta_0
%318 = OpLoad %v2float %257
%319 = OpCompositeExtract %float %318 1
%320 = OpFMul %float %317 %319
%321 = OpLoad %v2float %257
%322 = OpCompositeExtract %float %321 0
%323 = OpLoad %v2float %258
%324 = OpCompositeExtract %float %323 1
%325 = OpFSub %float %float_1 %324
%326 = OpFMul %float %322 %325
%327 = OpFAdd %float %320 %326
%328 = OpLoad %v2float %258
%329 = OpCompositeExtract %float %328 0
%330 = OpLoad %v2float %257
%331 = OpCompositeExtract %float %330 1
%332 = OpFSub %float %float_1 %331
%333 = OpFMul %float %329 %332
%334 = OpFAdd %float %327 %333
OpReturnValue %334
%292 = OpLabel
OpBranch %267
%267 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %29
%335 = OpFunctionParameter %_ptr_Function_v2float
%336 = OpFunctionParameter %_ptr_Function_v2float
%337 = OpLabel
%_2_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_3_n = OpVariable %_ptr_Function_float Function
%338 = OpLoad %v2float %335
%339 = OpCompositeExtract %float %338 0
%340 = OpFMul %float %float_2 %339
%341 = OpLoad %v2float %335
%342 = OpCompositeExtract %float %341 1
%343 = OpFOrdLessThanEqual %bool %340 %342
OpSelectionMerge %346 None
OpBranchConditional %343 %344 %345
%344 = OpLabel
%348 = OpLoad %v2float %336
%349 = OpCompositeExtract %float %348 0
%350 = OpLoad %v2float %336
%351 = OpCompositeExtract %float %350 0
%352 = OpFMul %float %349 %351
%353 = OpLoad %v2float %335
%354 = OpCompositeExtract %float %353 1
%355 = OpLoad %v2float %335
%356 = OpCompositeExtract %float %355 0
%357 = OpFMul %float %float_2 %356
%358 = OpFSub %float %354 %357
%359 = OpFMul %float %352 %358
OpStore %_2_n %359
%360 = OpLoad %float %_2_n
%361 = OpLoad %v2float %336
%362 = OpCompositeExtract %float %361 1
%363 = OpFDiv %float %360 %362
%364 = OpLoad %v2float %336
%365 = OpCompositeExtract %float %364 1
%366 = OpFSub %float %float_1 %365
%367 = OpLoad %v2float %335
%368 = OpCompositeExtract %float %367 0
%369 = OpFMul %float %366 %368
%370 = OpFAdd %float %363 %369
%371 = OpLoad %v2float %336
%372 = OpCompositeExtract %float %371 0
%374 = OpLoad %v2float %335
%375 = OpCompositeExtract %float %374 1
%373 = OpFNegate %float %375
%376 = OpLoad %v2float %335
%377 = OpCompositeExtract %float %376 0
%378 = OpFMul %float %float_2 %377
%379 = OpFAdd %float %373 %378
%380 = OpFAdd %float %379 %float_1
%381 = OpFMul %float %372 %380
%382 = OpFAdd %float %370 %381
OpReturnValue %382
%345 = OpLabel
%384 = OpLoad %v2float %336
%385 = OpCompositeExtract %float %384 0
%386 = OpFMul %float %float_4 %385
%387 = OpLoad %v2float %336
%388 = OpCompositeExtract %float %387 1
%389 = OpFOrdLessThanEqual %bool %386 %388
OpSelectionMerge %392 None
OpBranchConditional %389 %390 %391
%390 = OpLabel
%394 = OpLoad %v2float %336
%395 = OpCompositeExtract %float %394 0
%396 = OpLoad %v2float %336
%397 = OpCompositeExtract %float %396 0
%398 = OpFMul %float %395 %397
OpStore %DSqd %398
%400 = OpLoad %float %DSqd
%401 = OpLoad %v2float %336
%402 = OpCompositeExtract %float %401 0
%403 = OpFMul %float %400 %402
OpStore %DCub %403
%405 = OpLoad %v2float %336
%406 = OpCompositeExtract %float %405 1
%407 = OpLoad %v2float %336
%408 = OpCompositeExtract %float %407 1
%409 = OpFMul %float %406 %408
OpStore %DaSqd %409
%411 = OpLoad %float %DaSqd
%412 = OpLoad %v2float %336
%413 = OpCompositeExtract %float %412 1
%414 = OpFMul %float %411 %413
OpStore %DaCub %414
%416 = OpLoad %float %DaSqd
%417 = OpLoad %v2float %335
%418 = OpCompositeExtract %float %417 0
%419 = OpLoad %v2float %336
%420 = OpCompositeExtract %float %419 0
%422 = OpLoad %v2float %335
%423 = OpCompositeExtract %float %422 1
%424 = OpFMul %float %float_3 %423
%426 = OpLoad %v2float %335
%427 = OpCompositeExtract %float %426 0
%428 = OpFMul %float %float_6 %427
%429 = OpFSub %float %424 %428
%430 = OpFSub %float %429 %float_1
%431 = OpFMul %float %420 %430
%432 = OpFSub %float %418 %431
%433 = OpFMul %float %416 %432
%435 = OpLoad %v2float %336
%436 = OpCompositeExtract %float %435 1
%437 = OpFMul %float %float_12 %436
%438 = OpLoad %float %DSqd
%439 = OpFMul %float %437 %438
%440 = OpLoad %v2float %335
%441 = OpCompositeExtract %float %440 1
%442 = OpLoad %v2float %335
%443 = OpCompositeExtract %float %442 0
%444 = OpFMul %float %float_2 %443
%445 = OpFSub %float %441 %444
%446 = OpFMul %float %439 %445
%447 = OpFAdd %float %433 %446
%449 = OpLoad %float %DCub
%450 = OpFMul %float %float_16 %449
%451 = OpLoad %v2float %335
%452 = OpCompositeExtract %float %451 1
%453 = OpLoad %v2float %335
%454 = OpCompositeExtract %float %453 0
%455 = OpFMul %float %float_2 %454
%456 = OpFSub %float %452 %455
%457 = OpFMul %float %450 %456
%458 = OpFSub %float %447 %457
%459 = OpLoad %float %DaCub
%460 = OpLoad %v2float %335
%461 = OpCompositeExtract %float %460 0
%462 = OpFMul %float %459 %461
%463 = OpFSub %float %458 %462
OpStore %_3_n %463
%464 = OpLoad %float %_3_n
%465 = OpLoad %float %DaSqd
%466 = OpFDiv %float %464 %465
OpReturnValue %466
%391 = OpLabel
%467 = OpLoad %v2float %336
%468 = OpCompositeExtract %float %467 0
%469 = OpLoad %v2float %335
%470 = OpCompositeExtract %float %469 1
%471 = OpLoad %v2float %335
%472 = OpCompositeExtract %float %471 0
%473 = OpFMul %float %float_2 %472
%474 = OpFSub %float %470 %473
%475 = OpFAdd %float %474 %float_1
%476 = OpFMul %float %468 %475
%477 = OpLoad %v2float %335
%478 = OpCompositeExtract %float %477 0
%479 = OpFAdd %float %476 %478
%481 = OpLoad %v2float %336
%482 = OpCompositeExtract %float %481 1
%483 = OpLoad %v2float %336
%484 = OpCompositeExtract %float %483 0
%485 = OpFMul %float %482 %484
%480 = OpExtInst %float %1 Sqrt %485
%486 = OpLoad %v2float %335
%487 = OpCompositeExtract %float %486 1
%488 = OpLoad %v2float %335
%489 = OpCompositeExtract %float %488 0
%490 = OpFMul %float %float_2 %489
%491 = OpFSub %float %487 %490
%492 = OpFMul %float %480 %491
%493 = OpFSub %float %479 %492
%494 = OpLoad %v2float %336
%495 = OpCompositeExtract %float %494 1
%496 = OpLoad %v2float %335
%497 = OpCompositeExtract %float %496 0
%498 = OpFMul %float %495 %497
%499 = OpFSub %float %493 %498
OpReturnValue %499
%392 = OpLabel
OpBranch %346
%346 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %500
%502 = OpFunctionParameter %_ptr_Function_v3float
%503 = OpFunctionParameter %_ptr_Function_float
%504 = OpFunctionParameter %_ptr_Function_v3float
%505 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_2 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%512 = OpLoad %v3float %504
%507 = OpDot %float %511 %512
OpStore %lum %507
%514 = OpLoad %float %lum
%516 = OpLoad %v3float %502
%515 = OpDot %float %511 %516
%517 = OpFSub %float %514 %515
%518 = OpLoad %v3float %502
%519 = OpCompositeConstruct %v3float %517 %517 %517
%520 = OpFAdd %v3float %519 %518
OpStore %result_2 %520
%524 = OpLoad %v3float %result_2
%525 = OpCompositeExtract %float %524 0
%526 = OpLoad %v3float %result_2
%527 = OpCompositeExtract %float %526 1
%523 = OpExtInst %float %1 FMin %525 %527
%528 = OpLoad %v3float %result_2
%529 = OpCompositeExtract %float %528 2
%522 = OpExtInst %float %1 FMin %523 %529
OpStore %minComp %522
%533 = OpLoad %v3float %result_2
%534 = OpCompositeExtract %float %533 0
%535 = OpLoad %v3float %result_2
%536 = OpCompositeExtract %float %535 1
%532 = OpExtInst %float %1 FMax %534 %536
%537 = OpLoad %v3float %result_2
%538 = OpCompositeExtract %float %537 2
%531 = OpExtInst %float %1 FMax %532 %538
OpStore %maxComp %531
%540 = OpLoad %float %minComp
%541 = OpFOrdLessThan %bool %540 %float_0
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpLoad %float %lum
%545 = OpLoad %float %minComp
%546 = OpFOrdNotEqual %bool %544 %545
OpBranch %543
%543 = OpLabel
%547 = OpPhi %bool %false %505 %546 %542
OpSelectionMerge %549 None
OpBranchConditional %547 %548 %549
%548 = OpLabel
%551 = OpLoad %float %lum
%552 = OpLoad %float %minComp
%553 = OpFSub %float %551 %552
OpStore %_4_d %553
%554 = OpLoad %float %lum
%555 = OpLoad %v3float %result_2
%556 = OpLoad %float %lum
%557 = OpCompositeConstruct %v3float %556 %556 %556
%558 = OpFSub %v3float %555 %557
%559 = OpLoad %float %lum
%560 = OpLoad %float %_4_d
%561 = OpFDiv %float %559 %560
%562 = OpVectorTimesScalar %v3float %558 %561
%563 = OpCompositeConstruct %v3float %554 %554 %554
%564 = OpFAdd %v3float %563 %562
OpStore %result_2 %564
OpBranch %549
%549 = OpLabel
%565 = OpLoad %float %maxComp
%566 = OpLoad %float %503
%567 = OpFOrdGreaterThan %bool %565 %566
OpSelectionMerge %569 None
OpBranchConditional %567 %568 %569
%568 = OpLabel
%570 = OpLoad %float %maxComp
%571 = OpLoad %float %lum
%572 = OpFOrdNotEqual %bool %570 %571
OpBranch %569
%569 = OpLabel
%573 = OpPhi %bool %false %549 %572 %568
OpSelectionMerge %576 None
OpBranchConditional %573 %574 %575
%574 = OpLabel
%578 = OpLoad %v3float %result_2
%579 = OpLoad %float %lum
%580 = OpCompositeConstruct %v3float %579 %579 %579
%581 = OpFSub %v3float %578 %580
%582 = OpLoad %float %503
%583 = OpLoad %float %lum
%584 = OpFSub %float %582 %583
%585 = OpVectorTimesScalar %v3float %581 %584
OpStore %_5_n %585
%587 = OpLoad %float %maxComp
%588 = OpLoad %float %lum
%589 = OpFSub %float %587 %588
OpStore %_6_d %589
%590 = OpLoad %float %lum
%591 = OpLoad %v3float %_5_n
%592 = OpLoad %float %_6_d
%593 = OpFDiv %float %float_1 %592
%594 = OpVectorTimesScalar %v3float %591 %593
%595 = OpCompositeConstruct %v3float %590 %590 %590
%596 = OpFAdd %v3float %595 %594
OpReturnValue %596
%575 = OpLabel
%597 = OpLoad %v3float %result_2
OpReturnValue %597
%576 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %598
%599 = OpFunctionParameter %_ptr_Function_v3float
%600 = OpFunctionParameter %_ptr_Function_float
%601 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%602 = OpLoad %v3float %599
%603 = OpCompositeExtract %float %602 0
%604 = OpLoad %v3float %599
%605 = OpCompositeExtract %float %604 2
%606 = OpFOrdLessThan %bool %603 %605
OpSelectionMerge %609 None
OpBranchConditional %606 %607 %608
%607 = OpLabel
%611 = OpLoad %float %600
%612 = OpLoad %v3float %599
%613 = OpCompositeExtract %float %612 1
%614 = OpLoad %v3float %599
%615 = OpCompositeExtract %float %614 0
%616 = OpFSub %float %613 %615
%617 = OpFMul %float %611 %616
OpStore %_7_n %617
%619 = OpLoad %v3float %599
%620 = OpCompositeExtract %float %619 2
%621 = OpLoad %v3float %599
%622 = OpCompositeExtract %float %621 0
%623 = OpFSub %float %620 %622
OpStore %_8_d %623
%624 = OpLoad %float %_7_n
%625 = OpLoad %float %_8_d
%626 = OpFDiv %float %624 %625
%627 = OpLoad %float %600
%628 = OpCompositeConstruct %v3float %float_0 %626 %627
OpReturnValue %628
%608 = OpLabel
OpReturnValue %629
%609 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %630
%631 = OpFunctionParameter %_ptr_Function_v3float
%632 = OpFunctionParameter %_ptr_Function_v3float
%633 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%669 = OpVariable %_ptr_Function_v3float Function
%671 = OpVariable %_ptr_Function_float Function
%683 = OpVariable %_ptr_Function_v3float Function
%685 = OpVariable %_ptr_Function_float Function
%690 = OpVariable %_ptr_Function_v3float Function
%692 = OpVariable %_ptr_Function_float Function
%705 = OpVariable %_ptr_Function_v3float Function
%707 = OpVariable %_ptr_Function_float Function
%720 = OpVariable %_ptr_Function_v3float Function
%722 = OpVariable %_ptr_Function_float Function
%727 = OpVariable %_ptr_Function_v3float Function
%729 = OpVariable %_ptr_Function_float Function
%637 = OpLoad %v3float %632
%638 = OpCompositeExtract %float %637 0
%639 = OpLoad %v3float %632
%640 = OpCompositeExtract %float %639 1
%636 = OpExtInst %float %1 FMax %638 %640
%641 = OpLoad %v3float %632
%642 = OpCompositeExtract %float %641 2
%635 = OpExtInst %float %1 FMax %636 %642
%645 = OpLoad %v3float %632
%646 = OpCompositeExtract %float %645 0
%647 = OpLoad %v3float %632
%648 = OpCompositeExtract %float %647 1
%644 = OpExtInst %float %1 FMin %646 %648
%649 = OpLoad %v3float %632
%650 = OpCompositeExtract %float %649 2
%643 = OpExtInst %float %1 FMin %644 %650
%651 = OpFSub %float %635 %643
OpStore %sat %651
%652 = OpLoad %v3float %631
%653 = OpCompositeExtract %float %652 0
%654 = OpLoad %v3float %631
%655 = OpCompositeExtract %float %654 1
%656 = OpFOrdLessThanEqual %bool %653 %655
OpSelectionMerge %659 None
OpBranchConditional %656 %657 %658
%657 = OpLabel
%660 = OpLoad %v3float %631
%661 = OpCompositeExtract %float %660 1
%662 = OpLoad %v3float %631
%663 = OpCompositeExtract %float %662 2
%664 = OpFOrdLessThanEqual %bool %661 %663
OpSelectionMerge %667 None
OpBranchConditional %664 %665 %666
%665 = OpLabel
%668 = OpLoad %v3float %631
OpStore %669 %668
%670 = OpLoad %float %sat
OpStore %671 %670
%672 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %669 %671
OpReturnValue %672
%666 = OpLabel
%673 = OpLoad %v3float %631
%674 = OpCompositeExtract %float %673 0
%675 = OpLoad %v3float %631
%676 = OpCompositeExtract %float %675 2
%677 = OpFOrdLessThanEqual %bool %674 %676
OpSelectionMerge %680 None
OpBranchConditional %677 %678 %679
%678 = OpLabel
%681 = OpLoad %v3float %631
%682 = OpVectorShuffle %v3float %681 %681 0 2 1
OpStore %683 %682
%684 = OpLoad %float %sat
OpStore %685 %684
%686 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %683 %685
%687 = OpVectorShuffle %v3float %686 %686 0 2 1
OpReturnValue %687
%679 = OpLabel
%688 = OpLoad %v3float %631
%689 = OpVectorShuffle %v3float %688 %688 2 0 1
OpStore %690 %689
%691 = OpLoad %float %sat
OpStore %692 %691
%693 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %690 %692
%694 = OpVectorShuffle %v3float %693 %693 1 2 0
OpReturnValue %694
%680 = OpLabel
OpBranch %667
%667 = OpLabel
OpBranch %659
%658 = OpLabel
%695 = OpLoad %v3float %631
%696 = OpCompositeExtract %float %695 0
%697 = OpLoad %v3float %631
%698 = OpCompositeExtract %float %697 2
%699 = OpFOrdLessThanEqual %bool %696 %698
OpSelectionMerge %702 None
OpBranchConditional %699 %700 %701
%700 = OpLabel
%703 = OpLoad %v3float %631
%704 = OpVectorShuffle %v3float %703 %703 1 0 2
OpStore %705 %704
%706 = OpLoad %float %sat
OpStore %707 %706
%708 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %705 %707
%709 = OpVectorShuffle %v3float %708 %708 1 0 2
OpReturnValue %709
%701 = OpLabel
%710 = OpLoad %v3float %631
%711 = OpCompositeExtract %float %710 1
%712 = OpLoad %v3float %631
%713 = OpCompositeExtract %float %712 2
%714 = OpFOrdLessThanEqual %bool %711 %713
OpSelectionMerge %717 None
OpBranchConditional %714 %715 %716
%715 = OpLabel
%718 = OpLoad %v3float %631
%719 = OpVectorShuffle %v3float %718 %718 1 2 0
OpStore %720 %719
%721 = OpLoad %float %sat
OpStore %722 %721
%723 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %720 %722
%724 = OpVectorShuffle %v3float %723 %723 2 0 1
OpReturnValue %724
%716 = OpLabel
%725 = OpLoad %v3float %631
%726 = OpVectorShuffle %v3float %725 %725 2 1 0
OpStore %727 %726
%728 = OpLoad %float %sat
OpStore %729 %728
%730 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %727 %729
%731 = OpVectorShuffle %v3float %730 %730 2 1 0
OpReturnValue %731
%717 = OpLabel
OpBranch %702
%702 = OpLabel
OpBranch %659
%659 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hue = OpFunction %v4float None %71
%732 = OpFunctionParameter %_ptr_Function_v4float
%733 = OpFunctionParameter %_ptr_Function_v4float
%734 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%754 = OpVariable %_ptr_Function_v3float Function
%756 = OpVariable %_ptr_Function_v3float Function
%758 = OpVariable %_ptr_Function_v3float Function
%760 = OpVariable %_ptr_Function_float Function
%762 = OpVariable %_ptr_Function_v3float Function
%736 = OpLoad %v4float %733
%737 = OpCompositeExtract %float %736 3
%738 = OpLoad %v4float %732
%739 = OpCompositeExtract %float %738 3
%740 = OpFMul %float %737 %739
OpStore %alpha %740
%742 = OpLoad %v4float %732
%743 = OpVectorShuffle %v3float %742 %742 0 1 2
%744 = OpLoad %v4float %733
%745 = OpCompositeExtract %float %744 3
%746 = OpVectorTimesScalar %v3float %743 %745
OpStore %sda %746
%748 = OpLoad %v4float %733
%749 = OpVectorShuffle %v3float %748 %748 0 1 2
%750 = OpLoad %v4float %732
%751 = OpCompositeExtract %float %750 3
%752 = OpVectorTimesScalar %v3float %749 %751
OpStore %dsa %752
%753 = OpLoad %v3float %sda
OpStore %754 %753
%755 = OpLoad %v3float %dsa
OpStore %756 %755
%757 = OpFunctionCall %v3float %_blend_set_color_saturation %754 %756
OpStore %758 %757
%759 = OpLoad %float %alpha
OpStore %760 %759
%761 = OpLoad %v3float %dsa
OpStore %762 %761
%763 = OpFunctionCall %v3float %_blend_set_color_luminance %758 %760 %762
%764 = OpLoad %v4float %733
%765 = OpVectorShuffle %v3float %764 %764 0 1 2
%766 = OpFAdd %v3float %763 %765
%767 = OpLoad %v3float %dsa
%768 = OpFSub %v3float %766 %767
%769 = OpLoad %v4float %732
%770 = OpVectorShuffle %v3float %769 %769 0 1 2
%771 = OpFAdd %v3float %768 %770
%772 = OpLoad %v3float %sda
%773 = OpFSub %v3float %771 %772
%774 = OpCompositeExtract %float %773 0
%775 = OpCompositeExtract %float %773 1
%776 = OpCompositeExtract %float %773 2
%777 = OpLoad %v4float %732
%778 = OpCompositeExtract %float %777 3
%779 = OpLoad %v4float %733
%780 = OpCompositeExtract %float %779 3
%781 = OpFAdd %float %778 %780
%782 = OpLoad %float %alpha
%783 = OpFSub %float %781 %782
%784 = OpCompositeConstruct %v4float %774 %775 %776 %783
OpReturnValue %784
OpFunctionEnd
%blend_saturation = OpFunction %v4float None %71
%785 = OpFunctionParameter %_ptr_Function_v4float
%786 = OpFunctionParameter %_ptr_Function_v4float
%787 = OpLabel
%alpha_0 = OpVariable %_ptr_Function_float Function
%sda_0 = OpVariable %_ptr_Function_v3float Function
%dsa_0 = OpVariable %_ptr_Function_v3float Function
%807 = OpVariable %_ptr_Function_v3float Function
%809 = OpVariable %_ptr_Function_v3float Function
%811 = OpVariable %_ptr_Function_v3float Function
%813 = OpVariable %_ptr_Function_float Function
%815 = OpVariable %_ptr_Function_v3float Function
%789 = OpLoad %v4float %786
%790 = OpCompositeExtract %float %789 3
%791 = OpLoad %v4float %785
%792 = OpCompositeExtract %float %791 3
%793 = OpFMul %float %790 %792
OpStore %alpha_0 %793
%795 = OpLoad %v4float %785
%796 = OpVectorShuffle %v3float %795 %795 0 1 2
%797 = OpLoad %v4float %786
%798 = OpCompositeExtract %float %797 3
%799 = OpVectorTimesScalar %v3float %796 %798
OpStore %sda_0 %799
%801 = OpLoad %v4float %786
%802 = OpVectorShuffle %v3float %801 %801 0 1 2
%803 = OpLoad %v4float %785
%804 = OpCompositeExtract %float %803 3
%805 = OpVectorTimesScalar %v3float %802 %804
OpStore %dsa_0 %805
%806 = OpLoad %v3float %dsa_0
OpStore %807 %806
%808 = OpLoad %v3float %sda_0
OpStore %809 %808
%810 = OpFunctionCall %v3float %_blend_set_color_saturation %807 %809
OpStore %811 %810
%812 = OpLoad %float %alpha_0
OpStore %813 %812
%814 = OpLoad %v3float %dsa_0
OpStore %815 %814
%816 = OpFunctionCall %v3float %_blend_set_color_luminance %811 %813 %815
%817 = OpLoad %v4float %786
%818 = OpVectorShuffle %v3float %817 %817 0 1 2
%819 = OpFAdd %v3float %816 %818
%820 = OpLoad %v3float %dsa_0
%821 = OpFSub %v3float %819 %820
%822 = OpLoad %v4float %785
%823 = OpVectorShuffle %v3float %822 %822 0 1 2
%824 = OpFAdd %v3float %821 %823
%825 = OpLoad %v3float %sda_0
%826 = OpFSub %v3float %824 %825
%827 = OpCompositeExtract %float %826 0
%828 = OpCompositeExtract %float %826 1
%829 = OpCompositeExtract %float %826 2
%830 = OpLoad %v4float %785
%831 = OpCompositeExtract %float %830 3
%832 = OpLoad %v4float %786
%833 = OpCompositeExtract %float %832 3
%834 = OpFAdd %float %831 %833
%835 = OpLoad %float %alpha_0
%836 = OpFSub %float %834 %835
%837 = OpCompositeConstruct %v4float %827 %828 %829 %836
OpReturnValue %837
OpFunctionEnd
%blend_color = OpFunction %v4float None %71
%838 = OpFunctionParameter %_ptr_Function_v4float
%839 = OpFunctionParameter %_ptr_Function_v4float
%840 = OpLabel
%alpha_1 = OpVariable %_ptr_Function_float Function
%sda_1 = OpVariable %_ptr_Function_v3float Function
%dsa_1 = OpVariable %_ptr_Function_v3float Function
%860 = OpVariable %_ptr_Function_v3float Function
%862 = OpVariable %_ptr_Function_float Function
%864 = OpVariable %_ptr_Function_v3float Function
%842 = OpLoad %v4float %839
%843 = OpCompositeExtract %float %842 3
%844 = OpLoad %v4float %838
%845 = OpCompositeExtract %float %844 3
%846 = OpFMul %float %843 %845
OpStore %alpha_1 %846
%848 = OpLoad %v4float %838
%849 = OpVectorShuffle %v3float %848 %848 0 1 2
%850 = OpLoad %v4float %839
%851 = OpCompositeExtract %float %850 3
%852 = OpVectorTimesScalar %v3float %849 %851
OpStore %sda_1 %852
%854 = OpLoad %v4float %839
%855 = OpVectorShuffle %v3float %854 %854 0 1 2
%856 = OpLoad %v4float %838
%857 = OpCompositeExtract %float %856 3
%858 = OpVectorTimesScalar %v3float %855 %857
OpStore %dsa_1 %858
%859 = OpLoad %v3float %sda_1
OpStore %860 %859
%861 = OpLoad %float %alpha_1
OpStore %862 %861
%863 = OpLoad %v3float %dsa_1
OpStore %864 %863
%865 = OpFunctionCall %v3float %_blend_set_color_luminance %860 %862 %864
%866 = OpLoad %v4float %839
%867 = OpVectorShuffle %v3float %866 %866 0 1 2
%868 = OpFAdd %v3float %865 %867
%869 = OpLoad %v3float %dsa_1
%870 = OpFSub %v3float %868 %869
%871 = OpLoad %v4float %838
%872 = OpVectorShuffle %v3float %871 %871 0 1 2
%873 = OpFAdd %v3float %870 %872
%874 = OpLoad %v3float %sda_1
%875 = OpFSub %v3float %873 %874
%876 = OpCompositeExtract %float %875 0
%877 = OpCompositeExtract %float %875 1
%878 = OpCompositeExtract %float %875 2
%879 = OpLoad %v4float %838
%880 = OpCompositeExtract %float %879 3
%881 = OpLoad %v4float %839
%882 = OpCompositeExtract %float %881 3
%883 = OpFAdd %float %880 %882
%884 = OpLoad %float %alpha_1
%885 = OpFSub %float %883 %884
%886 = OpCompositeConstruct %v4float %876 %877 %878 %885
OpReturnValue %886
OpFunctionEnd
%blend_luminosity = OpFunction %v4float None %71
%887 = OpFunctionParameter %_ptr_Function_v4float
%888 = OpFunctionParameter %_ptr_Function_v4float
%889 = OpLabel
%alpha_2 = OpVariable %_ptr_Function_float Function
%sda_2 = OpVariable %_ptr_Function_v3float Function
%dsa_2 = OpVariable %_ptr_Function_v3float Function
%909 = OpVariable %_ptr_Function_v3float Function
%911 = OpVariable %_ptr_Function_float Function
%913 = OpVariable %_ptr_Function_v3float Function
%891 = OpLoad %v4float %888
%892 = OpCompositeExtract %float %891 3
%893 = OpLoad %v4float %887
%894 = OpCompositeExtract %float %893 3
%895 = OpFMul %float %892 %894
OpStore %alpha_2 %895
%897 = OpLoad %v4float %887
%898 = OpVectorShuffle %v3float %897 %897 0 1 2
%899 = OpLoad %v4float %888
%900 = OpCompositeExtract %float %899 3
%901 = OpVectorTimesScalar %v3float %898 %900
OpStore %sda_2 %901
%903 = OpLoad %v4float %888
%904 = OpVectorShuffle %v3float %903 %903 0 1 2
%905 = OpLoad %v4float %887
%906 = OpCompositeExtract %float %905 3
%907 = OpVectorTimesScalar %v3float %904 %906
OpStore %dsa_2 %907
%908 = OpLoad %v3float %dsa_2
OpStore %909 %908
%910 = OpLoad %float %alpha_2
OpStore %911 %910
%912 = OpLoad %v3float %sda_2
OpStore %913 %912
%914 = OpFunctionCall %v3float %_blend_set_color_luminance %909 %911 %913
%915 = OpLoad %v4float %888
%916 = OpVectorShuffle %v3float %915 %915 0 1 2
%917 = OpFAdd %v3float %914 %916
%918 = OpLoad %v3float %dsa_2
%919 = OpFSub %v3float %917 %918
%920 = OpLoad %v4float %887
%921 = OpVectorShuffle %v3float %920 %920 0 1 2
%922 = OpFAdd %v3float %919 %921
%923 = OpLoad %v3float %sda_2
%924 = OpFSub %v3float %922 %923
%925 = OpCompositeExtract %float %924 0
%926 = OpCompositeExtract %float %924 1
%927 = OpCompositeExtract %float %924 2
%928 = OpLoad %v4float %887
%929 = OpCompositeExtract %float %928 3
%930 = OpLoad %v4float %888
%931 = OpCompositeExtract %float %930 3
%932 = OpFAdd %float %929 %931
%933 = OpLoad %float %alpha_2
%934 = OpFSub %float %932 %933
%935 = OpCompositeConstruct %v4float %925 %926 %927 %934
OpReturnValue %935
OpFunctionEnd
%blend = OpFunction %v4float None %937
%939 = OpFunctionParameter %_ptr_Function_int
%940 = OpFunctionParameter %_ptr_Function_v4float
%941 = OpFunctionParameter %_ptr_Function_v4float
%942 = OpLabel
%1057 = OpVariable %_ptr_Function_v4float Function
%1059 = OpVariable %_ptr_Function_v4float Function
%1062 = OpVariable %_ptr_Function_v4float Function
%1064 = OpVariable %_ptr_Function_v4float Function
%1067 = OpVariable %_ptr_Function_v4float Function
%1069 = OpVariable %_ptr_Function_v4float Function
%1073 = OpVariable %_ptr_Function_v2float Function
%1076 = OpVariable %_ptr_Function_v2float Function
%1080 = OpVariable %_ptr_Function_v2float Function
%1083 = OpVariable %_ptr_Function_v2float Function
%1087 = OpVariable %_ptr_Function_v2float Function
%1090 = OpVariable %_ptr_Function_v2float Function
%1104 = OpVariable %_ptr_Function_v2float Function
%1107 = OpVariable %_ptr_Function_v2float Function
%1111 = OpVariable %_ptr_Function_v2float Function
%1114 = OpVariable %_ptr_Function_v2float Function
%1118 = OpVariable %_ptr_Function_v2float Function
%1121 = OpVariable %_ptr_Function_v2float Function
%1134 = OpVariable %_ptr_Function_v4float Function
%1136 = OpVariable %_ptr_Function_v4float Function
%1141 = OpVariable %_ptr_Function_v4float Function
%1148 = OpVariable %_ptr_Function_v2float Function
%1151 = OpVariable %_ptr_Function_v2float Function
%1155 = OpVariable %_ptr_Function_v2float Function
%1158 = OpVariable %_ptr_Function_v2float Function
%1162 = OpVariable %_ptr_Function_v2float Function
%1165 = OpVariable %_ptr_Function_v2float Function
%1267 = OpVariable %_ptr_Function_v4float Function
%1269 = OpVariable %_ptr_Function_v4float Function
%1272 = OpVariable %_ptr_Function_v4float Function
%1274 = OpVariable %_ptr_Function_v4float Function
%1277 = OpVariable %_ptr_Function_v4float Function
%1279 = OpVariable %_ptr_Function_v4float Function
%1282 = OpVariable %_ptr_Function_v4float Function
%1284 = OpVariable %_ptr_Function_v4float Function
%943 = OpLoad %int %939
OpSelectionMerge %944 None
OpSwitch %943 %974 0 %945 1 %946 2 %947 3 %948 4 %949 5 %950 6 %951 7 %952 8 %953 9 %954 10 %955 11 %956 12 %957 13 %958 14 %959 15 %960 16 %961 17 %962 18 %963 19 %964 20 %965 21 %966 22 %967 23 %968 24 %969 25 %970 26 %971 27 %972 28 %973
%945 = OpLabel
OpReturnValue %975
%946 = OpLabel
%976 = OpLoad %v4float %940
OpReturnValue %976
%947 = OpLabel
%977 = OpLoad %v4float %941
OpReturnValue %977
%948 = OpLabel
%978 = OpLoad %v4float %940
%979 = OpLoad %v4float %940
%980 = OpCompositeExtract %float %979 3
%981 = OpFSub %float %float_1 %980
%982 = OpLoad %v4float %941
%983 = OpVectorTimesScalar %v4float %982 %981
%984 = OpFAdd %v4float %978 %983
OpReturnValue %984
%949 = OpLabel
%985 = OpLoad %v4float %941
%986 = OpCompositeExtract %float %985 3
%987 = OpFSub %float %float_1 %986
%988 = OpLoad %v4float %940
%989 = OpVectorTimesScalar %v4float %988 %987
%990 = OpLoad %v4float %941
%991 = OpFAdd %v4float %989 %990
OpReturnValue %991
%950 = OpLabel
%992 = OpLoad %v4float %940
%993 = OpLoad %v4float %941
%994 = OpCompositeExtract %float %993 3
%995 = OpVectorTimesScalar %v4float %992 %994
OpReturnValue %995
%951 = OpLabel
%996 = OpLoad %v4float %941
%997 = OpLoad %v4float %940
%998 = OpCompositeExtract %float %997 3
%999 = OpVectorTimesScalar %v4float %996 %998
OpReturnValue %999
%952 = OpLabel
%1000 = OpLoad %v4float %941
%1001 = OpCompositeExtract %float %1000 3
%1002 = OpFSub %float %float_1 %1001
%1003 = OpLoad %v4float %940
%1004 = OpVectorTimesScalar %v4float %1003 %1002
OpReturnValue %1004
%953 = OpLabel
%1005 = OpLoad %v4float %940
%1006 = OpCompositeExtract %float %1005 3
%1007 = OpFSub %float %float_1 %1006
%1008 = OpLoad %v4float %941
%1009 = OpVectorTimesScalar %v4float %1008 %1007
OpReturnValue %1009
%954 = OpLabel
%1010 = OpLoad %v4float %941
%1011 = OpCompositeExtract %float %1010 3
%1012 = OpLoad %v4float %940
%1013 = OpVectorTimesScalar %v4float %1012 %1011
%1014 = OpLoad %v4float %940
%1015 = OpCompositeExtract %float %1014 3
%1016 = OpFSub %float %float_1 %1015
%1017 = OpLoad %v4float %941
%1018 = OpVectorTimesScalar %v4float %1017 %1016
%1019 = OpFAdd %v4float %1013 %1018
OpReturnValue %1019
%955 = OpLabel
%1020 = OpLoad %v4float %941
%1021 = OpCompositeExtract %float %1020 3
%1022 = OpFSub %float %float_1 %1021
%1023 = OpLoad %v4float %940
%1024 = OpVectorTimesScalar %v4float %1023 %1022
%1025 = OpLoad %v4float %940
%1026 = OpCompositeExtract %float %1025 3
%1027 = OpLoad %v4float %941
%1028 = OpVectorTimesScalar %v4float %1027 %1026
%1029 = OpFAdd %v4float %1024 %1028
OpReturnValue %1029
%956 = OpLabel
%1030 = OpLoad %v4float %941
%1031 = OpCompositeExtract %float %1030 3
%1032 = OpFSub %float %float_1 %1031
%1033 = OpLoad %v4float %940
%1034 = OpVectorTimesScalar %v4float %1033 %1032
%1035 = OpLoad %v4float %940
%1036 = OpCompositeExtract %float %1035 3
%1037 = OpFSub %float %float_1 %1036
%1038 = OpLoad %v4float %941
%1039 = OpVectorTimesScalar %v4float %1038 %1037
%1040 = OpFAdd %v4float %1034 %1039
OpReturnValue %1040
%957 = OpLabel
%1042 = OpLoad %v4float %940
%1043 = OpLoad %v4float %941
%1044 = OpFAdd %v4float %1042 %1043
%1045 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1041 = OpExtInst %v4float %1 FMin %1044 %1045
OpReturnValue %1041
%958 = OpLabel
%1046 = OpLoad %v4float %940
%1047 = OpLoad %v4float %941
%1048 = OpFMul %v4float %1046 %1047
OpReturnValue %1048
%959 = OpLabel
%1049 = OpLoad %v4float %940
%1050 = OpLoad %v4float %940
%1051 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1052 = OpFSub %v4float %1051 %1050
%1053 = OpLoad %v4float %941
%1054 = OpFMul %v4float %1052 %1053
%1055 = OpFAdd %v4float %1049 %1054
OpReturnValue %1055
%960 = OpLabel
%1056 = OpLoad %v4float %940
OpStore %1057 %1056
%1058 = OpLoad %v4float %941
OpStore %1059 %1058
%1060 = OpFunctionCall %v4float %blend_overlay %1057 %1059
OpReturnValue %1060
%961 = OpLabel
%1061 = OpLoad %v4float %940
OpStore %1062 %1061
%1063 = OpLoad %v4float %941
OpStore %1064 %1063
%1065 = OpFunctionCall %v4float %blend_darken %1062 %1064
OpReturnValue %1065
%962 = OpLabel
%1066 = OpLoad %v4float %940
OpStore %1067 %1066
%1068 = OpLoad %v4float %941
OpStore %1069 %1068
%1070 = OpFunctionCall %v4float %blend_lighten %1067 %1069
OpReturnValue %1070
%963 = OpLabel
%1071 = OpLoad %v4float %940
%1072 = OpVectorShuffle %v2float %1071 %1071 0 3
OpStore %1073 %1072
%1074 = OpLoad %v4float %941
%1075 = OpVectorShuffle %v2float %1074 %1074 0 3
OpStore %1076 %1075
%1077 = OpFunctionCall %float %_color_dodge_component %1073 %1076
%1078 = OpLoad %v4float %940
%1079 = OpVectorShuffle %v2float %1078 %1078 1 3
OpStore %1080 %1079
%1081 = OpLoad %v4float %941
%1082 = OpVectorShuffle %v2float %1081 %1081 1 3
OpStore %1083 %1082
%1084 = OpFunctionCall %float %_color_dodge_component %1080 %1083
%1085 = OpLoad %v4float %940
%1086 = OpVectorShuffle %v2float %1085 %1085 2 3
OpStore %1087 %1086
%1088 = OpLoad %v4float %941
%1089 = OpVectorShuffle %v2float %1088 %1088 2 3
OpStore %1090 %1089
%1091 = OpFunctionCall %float %_color_dodge_component %1087 %1090
%1092 = OpLoad %v4float %940
%1093 = OpCompositeExtract %float %1092 3
%1094 = OpLoad %v4float %940
%1095 = OpCompositeExtract %float %1094 3
%1096 = OpFSub %float %float_1 %1095
%1097 = OpLoad %v4float %941
%1098 = OpCompositeExtract %float %1097 3
%1099 = OpFMul %float %1096 %1098
%1100 = OpFAdd %float %1093 %1099
%1101 = OpCompositeConstruct %v4float %1077 %1084 %1091 %1100
OpReturnValue %1101
%964 = OpLabel
%1102 = OpLoad %v4float %940
%1103 = OpVectorShuffle %v2float %1102 %1102 0 3
OpStore %1104 %1103
%1105 = OpLoad %v4float %941
%1106 = OpVectorShuffle %v2float %1105 %1105 0 3
OpStore %1107 %1106
%1108 = OpFunctionCall %float %_color_burn_component %1104 %1107
%1109 = OpLoad %v4float %940
%1110 = OpVectorShuffle %v2float %1109 %1109 1 3
OpStore %1111 %1110
%1112 = OpLoad %v4float %941
%1113 = OpVectorShuffle %v2float %1112 %1112 1 3
OpStore %1114 %1113
%1115 = OpFunctionCall %float %_color_burn_component %1111 %1114
%1116 = OpLoad %v4float %940
%1117 = OpVectorShuffle %v2float %1116 %1116 2 3
OpStore %1118 %1117
%1119 = OpLoad %v4float %941
%1120 = OpVectorShuffle %v2float %1119 %1119 2 3
OpStore %1121 %1120
%1122 = OpFunctionCall %float %_color_burn_component %1118 %1121
%1123 = OpLoad %v4float %940
%1124 = OpCompositeExtract %float %1123 3
%1125 = OpLoad %v4float %940
%1126 = OpCompositeExtract %float %1125 3
%1127 = OpFSub %float %float_1 %1126
%1128 = OpLoad %v4float %941
%1129 = OpCompositeExtract %float %1128 3
%1130 = OpFMul %float %1127 %1129
%1131 = OpFAdd %float %1124 %1130
%1132 = OpCompositeConstruct %v4float %1108 %1115 %1122 %1131
OpReturnValue %1132
%965 = OpLabel
%1133 = OpLoad %v4float %941
OpStore %1134 %1133
%1135 = OpLoad %v4float %940
OpStore %1136 %1135
%1137 = OpFunctionCall %v4float %blend_overlay %1134 %1136
OpReturnValue %1137
%966 = OpLabel
%1138 = OpLoad %v4float %941
%1139 = OpCompositeExtract %float %1138 3
%1140 = OpFOrdEqual %bool %1139 %float_0
OpSelectionMerge %1144 None
OpBranchConditional %1140 %1142 %1143
%1142 = OpLabel
%1145 = OpLoad %v4float %940
OpStore %1141 %1145
OpBranch %1144
%1143 = OpLabel
%1146 = OpLoad %v4float %940
%1147 = OpVectorShuffle %v2float %1146 %1146 0 3
OpStore %1148 %1147
%1149 = OpLoad %v4float %941
%1150 = OpVectorShuffle %v2float %1149 %1149 0 3
OpStore %1151 %1150
%1152 = OpFunctionCall %float %_soft_light_component %1148 %1151
%1153 = OpLoad %v4float %940
%1154 = OpVectorShuffle %v2float %1153 %1153 1 3
OpStore %1155 %1154
%1156 = OpLoad %v4float %941
%1157 = OpVectorShuffle %v2float %1156 %1156 1 3
OpStore %1158 %1157
%1159 = OpFunctionCall %float %_soft_light_component %1155 %1158
%1160 = OpLoad %v4float %940
%1161 = OpVectorShuffle %v2float %1160 %1160 2 3
OpStore %1162 %1161
%1163 = OpLoad %v4float %941
%1164 = OpVectorShuffle %v2float %1163 %1163 2 3
OpStore %1165 %1164
%1166 = OpFunctionCall %float %_soft_light_component %1162 %1165
%1167 = OpLoad %v4float %940
%1168 = OpCompositeExtract %float %1167 3
%1169 = OpLoad %v4float %940
%1170 = OpCompositeExtract %float %1169 3
%1171 = OpFSub %float %float_1 %1170
%1172 = OpLoad %v4float %941
%1173 = OpCompositeExtract %float %1172 3
%1174 = OpFMul %float %1171 %1173
%1175 = OpFAdd %float %1168 %1174
%1176 = OpCompositeConstruct %v4float %1152 %1159 %1166 %1175
OpStore %1141 %1176
OpBranch %1144
%1144 = OpLabel
%1177 = OpLoad %v4float %1141
OpReturnValue %1177
%967 = OpLabel
%1178 = OpLoad %v4float %940
%1179 = OpVectorShuffle %v3float %1178 %1178 0 1 2
%1180 = OpLoad %v4float %941
%1181 = OpVectorShuffle %v3float %1180 %1180 0 1 2
%1182 = OpFAdd %v3float %1179 %1181
%1184 = OpLoad %v4float %940
%1185 = OpVectorShuffle %v3float %1184 %1184 0 1 2
%1186 = OpLoad %v4float %941
%1187 = OpCompositeExtract %float %1186 3
%1188 = OpVectorTimesScalar %v3float %1185 %1187
%1189 = OpLoad %v4float %941
%1190 = OpVectorShuffle %v3float %1189 %1189 0 1 2
%1191 = OpLoad %v4float %940
%1192 = OpCompositeExtract %float %1191 3
%1193 = OpVectorTimesScalar %v3float %1190 %1192
%1183 = OpExtInst %v3float %1 FMin %1188 %1193
%1194 = OpVectorTimesScalar %v3float %1183 %float_2
%1195 = OpFSub %v3float %1182 %1194
%1196 = OpCompositeExtract %float %1195 0
%1197 = OpCompositeExtract %float %1195 1
%1198 = OpCompositeExtract %float %1195 2
%1199 = OpLoad %v4float %940
%1200 = OpCompositeExtract %float %1199 3
%1201 = OpLoad %v4float %940
%1202 = OpCompositeExtract %float %1201 3
%1203 = OpFSub %float %float_1 %1202
%1204 = OpLoad %v4float %941
%1205 = OpCompositeExtract %float %1204 3
%1206 = OpFMul %float %1203 %1205
%1207 = OpFAdd %float %1200 %1206
%1208 = OpCompositeConstruct %v4float %1196 %1197 %1198 %1207
OpReturnValue %1208
%968 = OpLabel
%1209 = OpLoad %v4float %941
%1210 = OpVectorShuffle %v3float %1209 %1209 0 1 2
%1211 = OpLoad %v4float %940
%1212 = OpVectorShuffle %v3float %1211 %1211 0 1 2
%1213 = OpFAdd %v3float %1210 %1212
%1214 = OpLoad %v4float %941
%1215 = OpVectorShuffle %v3float %1214 %1214 0 1 2
%1216 = OpVectorTimesScalar %v3float %1215 %float_2
%1217 = OpLoad %v4float %940
%1218 = OpVectorShuffle %v3float %1217 %1217 0 1 2
%1219 = OpFMul %v3float %1216 %1218
%1220 = OpFSub %v3float %1213 %1219
%1221 = OpCompositeExtract %float %1220 0
%1222 = OpCompositeExtract %float %1220 1
%1223 = OpCompositeExtract %float %1220 2
%1224 = OpLoad %v4float %940
%1225 = OpCompositeExtract %float %1224 3
%1226 = OpLoad %v4float %940
%1227 = OpCompositeExtract %float %1226 3
%1228 = OpFSub %float %float_1 %1227
%1229 = OpLoad %v4float %941
%1230 = OpCompositeExtract %float %1229 3
%1231 = OpFMul %float %1228 %1230
%1232 = OpFAdd %float %1225 %1231
%1233 = OpCompositeConstruct %v4float %1221 %1222 %1223 %1232
OpReturnValue %1233
%969 = OpLabel
%1234 = OpLoad %v4float %940
%1235 = OpCompositeExtract %float %1234 3
%1236 = OpFSub %float %float_1 %1235
%1237 = OpLoad %v4float %941
%1238 = OpVectorShuffle %v3float %1237 %1237 0 1 2
%1239 = OpVectorTimesScalar %v3float %1238 %1236
%1240 = OpLoad %v4float %941
%1241 = OpCompositeExtract %float %1240 3
%1242 = OpFSub %float %float_1 %1241
%1243 = OpLoad %v4float %940
%1244 = OpVectorShuffle %v3float %1243 %1243 0 1 2
%1245 = OpVectorTimesScalar %v3float %1244 %1242
%1246 = OpFAdd %v3float %1239 %1245
%1247 = OpLoad %v4float %940
%1248 = OpVectorShuffle %v3float %1247 %1247 0 1 2
%1249 = OpLoad %v4float %941
%1250 = OpVectorShuffle %v3float %1249 %1249 0 1 2
%1251 = OpFMul %v3float %1248 %1250
%1252 = OpFAdd %v3float %1246 %1251
%1253 = OpCompositeExtract %float %1252 0
%1254 = OpCompositeExtract %float %1252 1
%1255 = OpCompositeExtract %float %1252 2
%1256 = OpLoad %v4float %940
%1257 = OpCompositeExtract %float %1256 3
%1258 = OpLoad %v4float %940
%1259 = OpCompositeExtract %float %1258 3
%1260 = OpFSub %float %float_1 %1259
%1261 = OpLoad %v4float %941
%1262 = OpCompositeExtract %float %1261 3
%1263 = OpFMul %float %1260 %1262
%1264 = OpFAdd %float %1257 %1263
%1265 = OpCompositeConstruct %v4float %1253 %1254 %1255 %1264
OpReturnValue %1265
%970 = OpLabel
%1266 = OpLoad %v4float %940
OpStore %1267 %1266
%1268 = OpLoad %v4float %941
OpStore %1269 %1268
%1270 = OpFunctionCall %v4float %blend_hue %1267 %1269
OpReturnValue %1270
%971 = OpLabel
%1271 = OpLoad %v4float %940
OpStore %1272 %1271
%1273 = OpLoad %v4float %941
OpStore %1274 %1273
%1275 = OpFunctionCall %v4float %blend_saturation %1272 %1274
OpReturnValue %1275
%972 = OpLabel
%1276 = OpLoad %v4float %940
OpStore %1277 %1276
%1278 = OpLoad %v4float %941
OpStore %1279 %1278
%1280 = OpFunctionCall %v4float %blend_color %1277 %1279
OpReturnValue %1280
%973 = OpLabel
%1281 = OpLoad %v4float %940
OpStore %1282 %1281
%1283 = OpLoad %v4float %941
OpStore %1284 %1283
%1285 = OpFunctionCall %v4float %blend_luminosity %1282 %1284
OpReturnValue %1285
%974 = OpLabel
OpReturnValue %975
%944 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %1287
%1288 = OpLabel
%1290 = OpVariable %_ptr_Function_int Function
%1292 = OpVariable %_ptr_Function_v4float Function
%1294 = OpVariable %_ptr_Function_v4float Function
OpStore %1290 %int_13
%1291 = OpLoad %v4float %src
OpStore %1292 %1291
%1293 = OpLoad %v4float %dst
OpStore %1294 %1293
%1295 = OpFunctionCall %v4float %blend %1290 %1292 %1294
OpStore %sk_FragColor %1295
OpReturn
OpFunctionEnd

1 error
