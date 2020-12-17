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
OpName %_4_n "_4_n"
OpName %_color_burn_component "_color_burn_component"
OpName %_6_n "_6_n"
OpName %delta_0 "delta"
OpName %_soft_light_component "_soft_light_component"
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_10_n "_10_n"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result_0 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %_blend_set_color_saturation "_blend_set_color_saturation"
OpName %sat "sat"
OpName %blend "blend"
OpName %_32_result "_32_result"
OpName %_35_result "_35_result"
OpName %_44_alpha "_44_alpha"
OpName %_45_sda "_45_sda"
OpName %_46_dsa "_46_dsa"
OpName %_48_alpha "_48_alpha"
OpName %_49_sda "_49_sda"
OpName %_50_dsa "_50_dsa"
OpName %_52_alpha "_52_alpha"
OpName %_53_sda "_53_sda"
OpName %_54_dsa "_54_dsa"
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
OpDecorate %316 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
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
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %536 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %542 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %554 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %560 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %577 RelaxedPrecision
OpDecorate %579 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %608 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %613 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %626 RelaxedPrecision
OpDecorate %629 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %641 RelaxedPrecision
OpDecorate %644 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %650 RelaxedPrecision
OpDecorate %656 RelaxedPrecision
OpDecorate %659 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %666 RelaxedPrecision
OpDecorate %677 RelaxedPrecision
OpDecorate %709 RelaxedPrecision
OpDecorate %710 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %712 RelaxedPrecision
OpDecorate %714 RelaxedPrecision
OpDecorate %715 RelaxedPrecision
OpDecorate %717 RelaxedPrecision
OpDecorate %718 RelaxedPrecision
OpDecorate %720 RelaxedPrecision
OpDecorate %721 RelaxedPrecision
OpDecorate %723 RelaxedPrecision
OpDecorate %724 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %726 RelaxedPrecision
OpDecorate %729 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %733 RelaxedPrecision
OpDecorate %735 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %740 RelaxedPrecision
OpDecorate %741 RelaxedPrecision
OpDecorate %743 RelaxedPrecision
OpDecorate %745 RelaxedPrecision
OpDecorate %747 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %750 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %756 RelaxedPrecision
OpDecorate %758 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %763 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %770 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %777 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %781 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %787 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %795 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %798 RelaxedPrecision
OpDecorate %799 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %805 RelaxedPrecision
OpDecorate %807 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %813 RelaxedPrecision
OpDecorate %815 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %828 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %838 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %840 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %847 RelaxedPrecision
OpDecorate %850 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %861 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %865 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %878 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %888 RelaxedPrecision
OpDecorate %892 RelaxedPrecision
OpDecorate %894 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %897 RelaxedPrecision
OpDecorate %899 RelaxedPrecision
OpDecorate %900 RelaxedPrecision
OpDecorate %902 RelaxedPrecision
OpDecorate %904 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %914 RelaxedPrecision
OpDecorate %915 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %922 RelaxedPrecision
OpDecorate %925 RelaxedPrecision
OpDecorate %929 RelaxedPrecision
OpDecorate %932 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
OpDecorate %938 RelaxedPrecision
OpDecorate %940 RelaxedPrecision
OpDecorate %941 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %944 RelaxedPrecision
OpDecorate %946 RelaxedPrecision
OpDecorate %947 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %953 RelaxedPrecision
OpDecorate %955 RelaxedPrecision
OpDecorate %958 RelaxedPrecision
OpDecorate %960 RelaxedPrecision
OpDecorate %964 RelaxedPrecision
OpDecorate %968 RelaxedPrecision
OpDecorate %970 RelaxedPrecision
OpDecorate %972 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %975 RelaxedPrecision
OpDecorate %976 RelaxedPrecision
OpDecorate %978 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %986 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %989 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %998 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1001 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1009 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1015 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1020 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1048 RelaxedPrecision
OpDecorate %1050 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1059 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1064 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1067 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1069 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %1072 RelaxedPrecision
OpDecorate %1073 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1083 RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1094 RelaxedPrecision
OpDecorate %1098 RelaxedPrecision
OpDecorate %1100 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1109 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1114 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1117 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1119 RelaxedPrecision
OpDecorate %1121 RelaxedPrecision
OpDecorate %1122 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1148 RelaxedPrecision
OpDecorate %1150 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1162 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1164 RelaxedPrecision
OpDecorate %1165 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1168 RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1177 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1179 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1194 RelaxedPrecision
OpDecorate %1196 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1208 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1210 RelaxedPrecision
OpDecorate %1211 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1221 RelaxedPrecision
OpDecorate %1223 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1225 RelaxedPrecision
OpDecorate %1233 RelaxedPrecision
OpDecorate %1235 RelaxedPrecision
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
%442 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%450 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%458 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%538 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%566 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%568 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%671 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%708 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%1227 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1229 = OpTypeFunction %void
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
%170 = OpLoad %v2float %124
%171 = OpCompositeExtract %float %170 0
%172 = OpLoad %v2float %123
%173 = OpCompositeExtract %float %172 1
%174 = OpFMul %float %171 %173
OpStore %_4_n %174
%176 = OpLoad %v2float %124
%177 = OpCompositeExtract %float %176 1
%178 = OpLoad %float %_4_n
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
%242 = OpLoad %v2float %200
%243 = OpCompositeExtract %float %242 1
%244 = OpLoad %v2float %200
%245 = OpCompositeExtract %float %244 0
%246 = OpFSub %float %243 %245
%247 = OpLoad %v2float %199
%248 = OpCompositeExtract %float %247 1
%249 = OpFMul %float %246 %248
OpStore %_6_n %249
%252 = OpLoad %v2float %200
%253 = OpCompositeExtract %float %252 1
%254 = OpLoad %float %_6_n
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
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
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
OpStore %_8_n %301
%302 = OpLoad %float %_8_n
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
OpStore %_10_n %405
%406 = OpLoad %float %_10_n
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
%516 = OpVariable %_ptr_Function_v3float Function
%454 = OpLoad %v3float %446
%449 = OpDot %float %450 %454
OpStore %lum %449
%456 = OpLoad %float %lum
%459 = OpLoad %v3float %444
%457 = OpDot %float %458 %459
%460 = OpFSub %float %456 %457
%461 = OpLoad %v3float %444
%462 = OpCompositeConstruct %v3float %460 %460 %460
%463 = OpFAdd %v3float %462 %461
OpStore %result_0 %463
%467 = OpLoad %v3float %result_0
%468 = OpCompositeExtract %float %467 0
%469 = OpLoad %v3float %result_0
%470 = OpCompositeExtract %float %469 1
%466 = OpExtInst %float %1 FMin %468 %470
%471 = OpLoad %v3float %result_0
%472 = OpCompositeExtract %float %471 2
%465 = OpExtInst %float %1 FMin %466 %472
OpStore %minComp %465
%476 = OpLoad %v3float %result_0
%477 = OpCompositeExtract %float %476 0
%478 = OpLoad %v3float %result_0
%479 = OpCompositeExtract %float %478 1
%475 = OpExtInst %float %1 FMax %477 %479
%480 = OpLoad %v3float %result_0
%481 = OpCompositeExtract %float %480 2
%474 = OpExtInst %float %1 FMax %475 %481
OpStore %maxComp %474
%483 = OpLoad %float %minComp
%484 = OpFOrdLessThan %bool %483 %float_0
OpSelectionMerge %486 None
OpBranchConditional %484 %485 %486
%485 = OpLabel
%487 = OpLoad %float %lum
%488 = OpLoad %float %minComp
%489 = OpFOrdNotEqual %bool %487 %488
OpBranch %486
%486 = OpLabel
%490 = OpPhi %bool %false %447 %489 %485
OpSelectionMerge %492 None
OpBranchConditional %490 %491 %492
%491 = OpLabel
%493 = OpLoad %float %lum
%494 = OpLoad %v3float %result_0
%495 = OpLoad %float %lum
%496 = OpCompositeConstruct %v3float %495 %495 %495
%497 = OpFSub %v3float %494 %496
%498 = OpLoad %float %lum
%499 = OpVectorTimesScalar %v3float %497 %498
%500 = OpLoad %float %lum
%501 = OpLoad %float %minComp
%502 = OpFSub %float %500 %501
%503 = OpFDiv %float %float_1 %502
%504 = OpVectorTimesScalar %v3float %499 %503
%505 = OpCompositeConstruct %v3float %493 %493 %493
%506 = OpFAdd %v3float %505 %504
OpStore %result_0 %506
OpBranch %492
%492 = OpLabel
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
%515 = OpPhi %bool %false %492 %514 %510
OpSelectionMerge %519 None
OpBranchConditional %515 %517 %518
%517 = OpLabel
%520 = OpLoad %float %lum
%521 = OpLoad %v3float %result_0
%522 = OpLoad %float %lum
%523 = OpCompositeConstruct %v3float %522 %522 %522
%524 = OpFSub %v3float %521 %523
%525 = OpLoad %float %445
%526 = OpLoad %float %lum
%527 = OpFSub %float %525 %526
%528 = OpVectorTimesScalar %v3float %524 %527
%529 = OpLoad %float %maxComp
%530 = OpLoad %float %lum
%531 = OpFSub %float %529 %530
%532 = OpFDiv %float %float_1 %531
%533 = OpVectorTimesScalar %v3float %528 %532
%534 = OpCompositeConstruct %v3float %520 %520 %520
%535 = OpFAdd %v3float %534 %533
OpStore %516 %535
OpBranch %519
%518 = OpLabel
%536 = OpLoad %v3float %result_0
OpStore %516 %536
OpBranch %519
%519 = OpLabel
%537 = OpLoad %v3float %516
OpReturnValue %537
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %538
%539 = OpFunctionParameter %_ptr_Function_v3float
%540 = OpFunctionParameter %_ptr_Function_float
%541 = OpLabel
%547 = OpVariable %_ptr_Function_v3float Function
%542 = OpLoad %v3float %539
%543 = OpCompositeExtract %float %542 0
%544 = OpLoad %v3float %539
%545 = OpCompositeExtract %float %544 2
%546 = OpFOrdLessThan %bool %543 %545
OpSelectionMerge %550 None
OpBranchConditional %546 %548 %549
%548 = OpLabel
%551 = OpLoad %float %540
%552 = OpLoad %v3float %539
%553 = OpCompositeExtract %float %552 1
%554 = OpLoad %v3float %539
%555 = OpCompositeExtract %float %554 0
%556 = OpFSub %float %553 %555
%557 = OpFMul %float %551 %556
%558 = OpLoad %v3float %539
%559 = OpCompositeExtract %float %558 2
%560 = OpLoad %v3float %539
%561 = OpCompositeExtract %float %560 0
%562 = OpFSub %float %559 %561
%563 = OpFDiv %float %557 %562
%564 = OpLoad %float %540
%565 = OpCompositeConstruct %v3float %float_0 %563 %564
OpStore %547 %565
OpBranch %550
%549 = OpLabel
OpStore %547 %566
OpBranch %550
%550 = OpLabel
%567 = OpLoad %v3float %547
OpReturnValue %567
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %568
%569 = OpFunctionParameter %_ptr_Function_v3float
%570 = OpFunctionParameter %_ptr_Function_v3float
%571 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%607 = OpVariable %_ptr_Function_v3float Function
%609 = OpVariable %_ptr_Function_float Function
%621 = OpVariable %_ptr_Function_v3float Function
%623 = OpVariable %_ptr_Function_float Function
%628 = OpVariable %_ptr_Function_v3float Function
%630 = OpVariable %_ptr_Function_float Function
%643 = OpVariable %_ptr_Function_v3float Function
%645 = OpVariable %_ptr_Function_float Function
%658 = OpVariable %_ptr_Function_v3float Function
%660 = OpVariable %_ptr_Function_float Function
%665 = OpVariable %_ptr_Function_v3float Function
%667 = OpVariable %_ptr_Function_float Function
%575 = OpLoad %v3float %570
%576 = OpCompositeExtract %float %575 0
%577 = OpLoad %v3float %570
%578 = OpCompositeExtract %float %577 1
%574 = OpExtInst %float %1 FMax %576 %578
%579 = OpLoad %v3float %570
%580 = OpCompositeExtract %float %579 2
%573 = OpExtInst %float %1 FMax %574 %580
%583 = OpLoad %v3float %570
%584 = OpCompositeExtract %float %583 0
%585 = OpLoad %v3float %570
%586 = OpCompositeExtract %float %585 1
%582 = OpExtInst %float %1 FMin %584 %586
%587 = OpLoad %v3float %570
%588 = OpCompositeExtract %float %587 2
%581 = OpExtInst %float %1 FMin %582 %588
%589 = OpFSub %float %573 %581
OpStore %sat %589
%590 = OpLoad %v3float %569
%591 = OpCompositeExtract %float %590 0
%592 = OpLoad %v3float %569
%593 = OpCompositeExtract %float %592 1
%594 = OpFOrdLessThanEqual %bool %591 %593
OpSelectionMerge %597 None
OpBranchConditional %594 %595 %596
%595 = OpLabel
%598 = OpLoad %v3float %569
%599 = OpCompositeExtract %float %598 1
%600 = OpLoad %v3float %569
%601 = OpCompositeExtract %float %600 2
%602 = OpFOrdLessThanEqual %bool %599 %601
OpSelectionMerge %605 None
OpBranchConditional %602 %603 %604
%603 = OpLabel
%606 = OpLoad %v3float %569
OpStore %607 %606
%608 = OpLoad %float %sat
OpStore %609 %608
%610 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %607 %609
OpReturnValue %610
%604 = OpLabel
%611 = OpLoad %v3float %569
%612 = OpCompositeExtract %float %611 0
%613 = OpLoad %v3float %569
%614 = OpCompositeExtract %float %613 2
%615 = OpFOrdLessThanEqual %bool %612 %614
OpSelectionMerge %618 None
OpBranchConditional %615 %616 %617
%616 = OpLabel
%619 = OpLoad %v3float %569
%620 = OpVectorShuffle %v3float %619 %619 0 2 1
OpStore %621 %620
%622 = OpLoad %float %sat
OpStore %623 %622
%624 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %621 %623
%625 = OpVectorShuffle %v3float %624 %624 0 2 1
OpReturnValue %625
%617 = OpLabel
%626 = OpLoad %v3float %569
%627 = OpVectorShuffle %v3float %626 %626 2 0 1
OpStore %628 %627
%629 = OpLoad %float %sat
OpStore %630 %629
%631 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %628 %630
%632 = OpVectorShuffle %v3float %631 %631 1 2 0
OpReturnValue %632
%618 = OpLabel
OpBranch %605
%605 = OpLabel
OpBranch %597
%596 = OpLabel
%633 = OpLoad %v3float %569
%634 = OpCompositeExtract %float %633 0
%635 = OpLoad %v3float %569
%636 = OpCompositeExtract %float %635 2
%637 = OpFOrdLessThanEqual %bool %634 %636
OpSelectionMerge %640 None
OpBranchConditional %637 %638 %639
%638 = OpLabel
%641 = OpLoad %v3float %569
%642 = OpVectorShuffle %v3float %641 %641 1 0 2
OpStore %643 %642
%644 = OpLoad %float %sat
OpStore %645 %644
%646 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %643 %645
%647 = OpVectorShuffle %v3float %646 %646 1 0 2
OpReturnValue %647
%639 = OpLabel
%648 = OpLoad %v3float %569
%649 = OpCompositeExtract %float %648 1
%650 = OpLoad %v3float %569
%651 = OpCompositeExtract %float %650 2
%652 = OpFOrdLessThanEqual %bool %649 %651
OpSelectionMerge %655 None
OpBranchConditional %652 %653 %654
%653 = OpLabel
%656 = OpLoad %v3float %569
%657 = OpVectorShuffle %v3float %656 %656 1 2 0
OpStore %658 %657
%659 = OpLoad %float %sat
OpStore %660 %659
%661 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %658 %660
%662 = OpVectorShuffle %v3float %661 %661 2 0 1
OpReturnValue %662
%654 = OpLabel
%663 = OpLoad %v3float %569
%664 = OpVectorShuffle %v3float %663 %663 2 1 0
OpStore %665 %664
%666 = OpLoad %float %sat
OpStore %667 %666
%668 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %665 %667
%669 = OpVectorShuffle %v3float %668 %668 2 1 0
OpReturnValue %669
%655 = OpLabel
OpBranch %640
%640 = OpLabel
OpBranch %597
%597 = OpLabel
OpUnreachable
OpFunctionEnd
%blend = OpFunction %v4float None %671
%673 = OpFunctionParameter %_ptr_Function_int
%674 = OpFunctionParameter %_ptr_Function_v4float
%675 = OpFunctionParameter %_ptr_Function_v4float
%676 = OpLabel
%790 = OpVariable %_ptr_Function_v4float Function
%792 = OpVariable %_ptr_Function_v4float Function
%_32_result = OpVariable %_ptr_Function_v4float Function
%_35_result = OpVariable %_ptr_Function_v4float Function
%842 = OpVariable %_ptr_Function_v2float Function
%845 = OpVariable %_ptr_Function_v2float Function
%849 = OpVariable %_ptr_Function_v2float Function
%852 = OpVariable %_ptr_Function_v2float Function
%856 = OpVariable %_ptr_Function_v2float Function
%859 = OpVariable %_ptr_Function_v2float Function
%873 = OpVariable %_ptr_Function_v2float Function
%876 = OpVariable %_ptr_Function_v2float Function
%880 = OpVariable %_ptr_Function_v2float Function
%883 = OpVariable %_ptr_Function_v2float Function
%887 = OpVariable %_ptr_Function_v2float Function
%890 = OpVariable %_ptr_Function_v2float Function
%903 = OpVariable %_ptr_Function_v4float Function
%905 = OpVariable %_ptr_Function_v4float Function
%910 = OpVariable %_ptr_Function_v4float Function
%917 = OpVariable %_ptr_Function_v2float Function
%920 = OpVariable %_ptr_Function_v2float Function
%924 = OpVariable %_ptr_Function_v2float Function
%927 = OpVariable %_ptr_Function_v2float Function
%931 = OpVariable %_ptr_Function_v2float Function
%934 = OpVariable %_ptr_Function_v2float Function
%_44_alpha = OpVariable %_ptr_Function_float Function
%_45_sda = OpVariable %_ptr_Function_v3float Function
%_46_dsa = OpVariable %_ptr_Function_v3float Function
%1054 = OpVariable %_ptr_Function_v3float Function
%1056 = OpVariable %_ptr_Function_v3float Function
%1058 = OpVariable %_ptr_Function_v3float Function
%1060 = OpVariable %_ptr_Function_float Function
%1062 = OpVariable %_ptr_Function_v3float Function
%_48_alpha = OpVariable %_ptr_Function_float Function
%_49_sda = OpVariable %_ptr_Function_v3float Function
%_50_dsa = OpVariable %_ptr_Function_v3float Function
%1104 = OpVariable %_ptr_Function_v3float Function
%1106 = OpVariable %_ptr_Function_v3float Function
%1108 = OpVariable %_ptr_Function_v3float Function
%1110 = OpVariable %_ptr_Function_float Function
%1112 = OpVariable %_ptr_Function_v3float Function
%_52_alpha = OpVariable %_ptr_Function_float Function
%_53_sda = OpVariable %_ptr_Function_v3float Function
%_54_dsa = OpVariable %_ptr_Function_v3float Function
%1154 = OpVariable %_ptr_Function_v3float Function
%1156 = OpVariable %_ptr_Function_float Function
%1158 = OpVariable %_ptr_Function_v3float Function
%_56_alpha = OpVariable %_ptr_Function_float Function
%_57_sda = OpVariable %_ptr_Function_v3float Function
%_58_dsa = OpVariable %_ptr_Function_v3float Function
%1200 = OpVariable %_ptr_Function_v3float Function
%1202 = OpVariable %_ptr_Function_float Function
%1204 = OpVariable %_ptr_Function_v3float Function
%677 = OpLoad %int %673
OpSelectionMerge %678 None
OpSwitch %677 %678 0 %679 1 %680 2 %681 3 %682 4 %683 5 %684 6 %685 7 %686 8 %687 9 %688 10 %689 11 %690 12 %691 13 %692 14 %693 15 %694 16 %695 17 %696 18 %697 19 %698 20 %699 21 %700 22 %701 23 %702 24 %703 25 %704 26 %705 27 %706 28 %707
%679 = OpLabel
OpReturnValue %708
%680 = OpLabel
%709 = OpLoad %v4float %674
OpReturnValue %709
%681 = OpLabel
%710 = OpLoad %v4float %675
OpReturnValue %710
%682 = OpLabel
%711 = OpLoad %v4float %674
%712 = OpLoad %v4float %674
%713 = OpCompositeExtract %float %712 3
%714 = OpFSub %float %float_1 %713
%715 = OpLoad %v4float %675
%716 = OpVectorTimesScalar %v4float %715 %714
%717 = OpFAdd %v4float %711 %716
OpReturnValue %717
%683 = OpLabel
%718 = OpLoad %v4float %675
%719 = OpCompositeExtract %float %718 3
%720 = OpFSub %float %float_1 %719
%721 = OpLoad %v4float %674
%722 = OpVectorTimesScalar %v4float %721 %720
%723 = OpLoad %v4float %675
%724 = OpFAdd %v4float %722 %723
OpReturnValue %724
%684 = OpLabel
%725 = OpLoad %v4float %674
%726 = OpLoad %v4float %675
%727 = OpCompositeExtract %float %726 3
%728 = OpVectorTimesScalar %v4float %725 %727
OpReturnValue %728
%685 = OpLabel
%729 = OpLoad %v4float %675
%730 = OpLoad %v4float %674
%731 = OpCompositeExtract %float %730 3
%732 = OpVectorTimesScalar %v4float %729 %731
OpReturnValue %732
%686 = OpLabel
%733 = OpLoad %v4float %675
%734 = OpCompositeExtract %float %733 3
%735 = OpFSub %float %float_1 %734
%736 = OpLoad %v4float %674
%737 = OpVectorTimesScalar %v4float %736 %735
OpReturnValue %737
%687 = OpLabel
%738 = OpLoad %v4float %674
%739 = OpCompositeExtract %float %738 3
%740 = OpFSub %float %float_1 %739
%741 = OpLoad %v4float %675
%742 = OpVectorTimesScalar %v4float %741 %740
OpReturnValue %742
%688 = OpLabel
%743 = OpLoad %v4float %675
%744 = OpCompositeExtract %float %743 3
%745 = OpLoad %v4float %674
%746 = OpVectorTimesScalar %v4float %745 %744
%747 = OpLoad %v4float %674
%748 = OpCompositeExtract %float %747 3
%749 = OpFSub %float %float_1 %748
%750 = OpLoad %v4float %675
%751 = OpVectorTimesScalar %v4float %750 %749
%752 = OpFAdd %v4float %746 %751
OpReturnValue %752
%689 = OpLabel
%753 = OpLoad %v4float %675
%754 = OpCompositeExtract %float %753 3
%755 = OpFSub %float %float_1 %754
%756 = OpLoad %v4float %674
%757 = OpVectorTimesScalar %v4float %756 %755
%758 = OpLoad %v4float %674
%759 = OpCompositeExtract %float %758 3
%760 = OpLoad %v4float %675
%761 = OpVectorTimesScalar %v4float %760 %759
%762 = OpFAdd %v4float %757 %761
OpReturnValue %762
%690 = OpLabel
%763 = OpLoad %v4float %675
%764 = OpCompositeExtract %float %763 3
%765 = OpFSub %float %float_1 %764
%766 = OpLoad %v4float %674
%767 = OpVectorTimesScalar %v4float %766 %765
%768 = OpLoad %v4float %674
%769 = OpCompositeExtract %float %768 3
%770 = OpFSub %float %float_1 %769
%771 = OpLoad %v4float %675
%772 = OpVectorTimesScalar %v4float %771 %770
%773 = OpFAdd %v4float %767 %772
OpReturnValue %773
%691 = OpLabel
%775 = OpLoad %v4float %674
%776 = OpLoad %v4float %675
%777 = OpFAdd %v4float %775 %776
%778 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%774 = OpExtInst %v4float %1 FMin %777 %778
OpReturnValue %774
%692 = OpLabel
%779 = OpLoad %v4float %674
%780 = OpLoad %v4float %675
%781 = OpFMul %v4float %779 %780
OpReturnValue %781
%693 = OpLabel
%782 = OpLoad %v4float %674
%783 = OpLoad %v4float %674
%784 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%785 = OpFSub %v4float %784 %783
%786 = OpLoad %v4float %675
%787 = OpFMul %v4float %785 %786
%788 = OpFAdd %v4float %782 %787
OpReturnValue %788
%694 = OpLabel
%789 = OpLoad %v4float %674
OpStore %790 %789
%791 = OpLoad %v4float %675
OpStore %792 %791
%793 = OpFunctionCall %v4float %blend_overlay %790 %792
OpReturnValue %793
%695 = OpLabel
%795 = OpLoad %v4float %674
%796 = OpLoad %v4float %674
%797 = OpCompositeExtract %float %796 3
%798 = OpFSub %float %float_1 %797
%799 = OpLoad %v4float %675
%800 = OpVectorTimesScalar %v4float %799 %798
%801 = OpFAdd %v4float %795 %800
OpStore %_32_result %801
%803 = OpLoad %v4float %_32_result
%804 = OpVectorShuffle %v3float %803 %803 0 1 2
%805 = OpLoad %v4float %675
%806 = OpCompositeExtract %float %805 3
%807 = OpFSub %float %float_1 %806
%808 = OpLoad %v4float %674
%809 = OpVectorShuffle %v3float %808 %808 0 1 2
%810 = OpVectorTimesScalar %v3float %809 %807
%811 = OpLoad %v4float %675
%812 = OpVectorShuffle %v3float %811 %811 0 1 2
%813 = OpFAdd %v3float %810 %812
%802 = OpExtInst %v3float %1 FMin %804 %813
%814 = OpLoad %v4float %_32_result
%815 = OpVectorShuffle %v4float %814 %802 4 5 6 3
OpStore %_32_result %815
%816 = OpLoad %v4float %_32_result
OpReturnValue %816
%696 = OpLabel
%818 = OpLoad %v4float %674
%819 = OpLoad %v4float %674
%820 = OpCompositeExtract %float %819 3
%821 = OpFSub %float %float_1 %820
%822 = OpLoad %v4float %675
%823 = OpVectorTimesScalar %v4float %822 %821
%824 = OpFAdd %v4float %818 %823
OpStore %_35_result %824
%826 = OpLoad %v4float %_35_result
%827 = OpVectorShuffle %v3float %826 %826 0 1 2
%828 = OpLoad %v4float %675
%829 = OpCompositeExtract %float %828 3
%830 = OpFSub %float %float_1 %829
%831 = OpLoad %v4float %674
%832 = OpVectorShuffle %v3float %831 %831 0 1 2
%833 = OpVectorTimesScalar %v3float %832 %830
%834 = OpLoad %v4float %675
%835 = OpVectorShuffle %v3float %834 %834 0 1 2
%836 = OpFAdd %v3float %833 %835
%825 = OpExtInst %v3float %1 FMax %827 %836
%837 = OpLoad %v4float %_35_result
%838 = OpVectorShuffle %v4float %837 %825 4 5 6 3
OpStore %_35_result %838
%839 = OpLoad %v4float %_35_result
OpReturnValue %839
%697 = OpLabel
%840 = OpLoad %v4float %674
%841 = OpVectorShuffle %v2float %840 %840 0 3
OpStore %842 %841
%843 = OpLoad %v4float %675
%844 = OpVectorShuffle %v2float %843 %843 0 3
OpStore %845 %844
%846 = OpFunctionCall %float %_color_dodge_component %842 %845
%847 = OpLoad %v4float %674
%848 = OpVectorShuffle %v2float %847 %847 1 3
OpStore %849 %848
%850 = OpLoad %v4float %675
%851 = OpVectorShuffle %v2float %850 %850 1 3
OpStore %852 %851
%853 = OpFunctionCall %float %_color_dodge_component %849 %852
%854 = OpLoad %v4float %674
%855 = OpVectorShuffle %v2float %854 %854 2 3
OpStore %856 %855
%857 = OpLoad %v4float %675
%858 = OpVectorShuffle %v2float %857 %857 2 3
OpStore %859 %858
%860 = OpFunctionCall %float %_color_dodge_component %856 %859
%861 = OpLoad %v4float %674
%862 = OpCompositeExtract %float %861 3
%863 = OpLoad %v4float %674
%864 = OpCompositeExtract %float %863 3
%865 = OpFSub %float %float_1 %864
%866 = OpLoad %v4float %675
%867 = OpCompositeExtract %float %866 3
%868 = OpFMul %float %865 %867
%869 = OpFAdd %float %862 %868
%870 = OpCompositeConstruct %v4float %846 %853 %860 %869
OpReturnValue %870
%698 = OpLabel
%871 = OpLoad %v4float %674
%872 = OpVectorShuffle %v2float %871 %871 0 3
OpStore %873 %872
%874 = OpLoad %v4float %675
%875 = OpVectorShuffle %v2float %874 %874 0 3
OpStore %876 %875
%877 = OpFunctionCall %float %_color_burn_component %873 %876
%878 = OpLoad %v4float %674
%879 = OpVectorShuffle %v2float %878 %878 1 3
OpStore %880 %879
%881 = OpLoad %v4float %675
%882 = OpVectorShuffle %v2float %881 %881 1 3
OpStore %883 %882
%884 = OpFunctionCall %float %_color_burn_component %880 %883
%885 = OpLoad %v4float %674
%886 = OpVectorShuffle %v2float %885 %885 2 3
OpStore %887 %886
%888 = OpLoad %v4float %675
%889 = OpVectorShuffle %v2float %888 %888 2 3
OpStore %890 %889
%891 = OpFunctionCall %float %_color_burn_component %887 %890
%892 = OpLoad %v4float %674
%893 = OpCompositeExtract %float %892 3
%894 = OpLoad %v4float %674
%895 = OpCompositeExtract %float %894 3
%896 = OpFSub %float %float_1 %895
%897 = OpLoad %v4float %675
%898 = OpCompositeExtract %float %897 3
%899 = OpFMul %float %896 %898
%900 = OpFAdd %float %893 %899
%901 = OpCompositeConstruct %v4float %877 %884 %891 %900
OpReturnValue %901
%699 = OpLabel
%902 = OpLoad %v4float %675
OpStore %903 %902
%904 = OpLoad %v4float %674
OpStore %905 %904
%906 = OpFunctionCall %v4float %blend_overlay %903 %905
OpReturnValue %906
%700 = OpLabel
%907 = OpLoad %v4float %675
%908 = OpCompositeExtract %float %907 3
%909 = OpFOrdEqual %bool %908 %float_0
OpSelectionMerge %913 None
OpBranchConditional %909 %911 %912
%911 = OpLabel
%914 = OpLoad %v4float %674
OpStore %910 %914
OpBranch %913
%912 = OpLabel
%915 = OpLoad %v4float %674
%916 = OpVectorShuffle %v2float %915 %915 0 3
OpStore %917 %916
%918 = OpLoad %v4float %675
%919 = OpVectorShuffle %v2float %918 %918 0 3
OpStore %920 %919
%921 = OpFunctionCall %float %_soft_light_component %917 %920
%922 = OpLoad %v4float %674
%923 = OpVectorShuffle %v2float %922 %922 1 3
OpStore %924 %923
%925 = OpLoad %v4float %675
%926 = OpVectorShuffle %v2float %925 %925 1 3
OpStore %927 %926
%928 = OpFunctionCall %float %_soft_light_component %924 %927
%929 = OpLoad %v4float %674
%930 = OpVectorShuffle %v2float %929 %929 2 3
OpStore %931 %930
%932 = OpLoad %v4float %675
%933 = OpVectorShuffle %v2float %932 %932 2 3
OpStore %934 %933
%935 = OpFunctionCall %float %_soft_light_component %931 %934
%936 = OpLoad %v4float %674
%937 = OpCompositeExtract %float %936 3
%938 = OpLoad %v4float %674
%939 = OpCompositeExtract %float %938 3
%940 = OpFSub %float %float_1 %939
%941 = OpLoad %v4float %675
%942 = OpCompositeExtract %float %941 3
%943 = OpFMul %float %940 %942
%944 = OpFAdd %float %937 %943
%945 = OpCompositeConstruct %v4float %921 %928 %935 %944
OpStore %910 %945
OpBranch %913
%913 = OpLabel
%946 = OpLoad %v4float %910
OpReturnValue %946
%701 = OpLabel
%947 = OpLoad %v4float %674
%948 = OpVectorShuffle %v3float %947 %947 0 1 2
%949 = OpLoad %v4float %675
%950 = OpVectorShuffle %v3float %949 %949 0 1 2
%951 = OpFAdd %v3float %948 %950
%953 = OpLoad %v4float %674
%954 = OpVectorShuffle %v3float %953 %953 0 1 2
%955 = OpLoad %v4float %675
%956 = OpCompositeExtract %float %955 3
%957 = OpVectorTimesScalar %v3float %954 %956
%958 = OpLoad %v4float %675
%959 = OpVectorShuffle %v3float %958 %958 0 1 2
%960 = OpLoad %v4float %674
%961 = OpCompositeExtract %float %960 3
%962 = OpVectorTimesScalar %v3float %959 %961
%952 = OpExtInst %v3float %1 FMin %957 %962
%963 = OpVectorTimesScalar %v3float %952 %float_2
%964 = OpFSub %v3float %951 %963
%965 = OpCompositeExtract %float %964 0
%966 = OpCompositeExtract %float %964 1
%967 = OpCompositeExtract %float %964 2
%968 = OpLoad %v4float %674
%969 = OpCompositeExtract %float %968 3
%970 = OpLoad %v4float %674
%971 = OpCompositeExtract %float %970 3
%972 = OpFSub %float %float_1 %971
%973 = OpLoad %v4float %675
%974 = OpCompositeExtract %float %973 3
%975 = OpFMul %float %972 %974
%976 = OpFAdd %float %969 %975
%977 = OpCompositeConstruct %v4float %965 %966 %967 %976
OpReturnValue %977
%702 = OpLabel
%978 = OpLoad %v4float %675
%979 = OpVectorShuffle %v3float %978 %978 0 1 2
%980 = OpLoad %v4float %674
%981 = OpVectorShuffle %v3float %980 %980 0 1 2
%982 = OpFAdd %v3float %979 %981
%983 = OpLoad %v4float %675
%984 = OpVectorShuffle %v3float %983 %983 0 1 2
%985 = OpVectorTimesScalar %v3float %984 %float_2
%986 = OpLoad %v4float %674
%987 = OpVectorShuffle %v3float %986 %986 0 1 2
%988 = OpFMul %v3float %985 %987
%989 = OpFSub %v3float %982 %988
%990 = OpCompositeExtract %float %989 0
%991 = OpCompositeExtract %float %989 1
%992 = OpCompositeExtract %float %989 2
%993 = OpLoad %v4float %674
%994 = OpCompositeExtract %float %993 3
%995 = OpLoad %v4float %674
%996 = OpCompositeExtract %float %995 3
%997 = OpFSub %float %float_1 %996
%998 = OpLoad %v4float %675
%999 = OpCompositeExtract %float %998 3
%1000 = OpFMul %float %997 %999
%1001 = OpFAdd %float %994 %1000
%1002 = OpCompositeConstruct %v4float %990 %991 %992 %1001
OpReturnValue %1002
%703 = OpLabel
%1003 = OpLoad %v4float %674
%1004 = OpCompositeExtract %float %1003 3
%1005 = OpFSub %float %float_1 %1004
%1006 = OpLoad %v4float %675
%1007 = OpVectorShuffle %v3float %1006 %1006 0 1 2
%1008 = OpVectorTimesScalar %v3float %1007 %1005
%1009 = OpLoad %v4float %675
%1010 = OpCompositeExtract %float %1009 3
%1011 = OpFSub %float %float_1 %1010
%1012 = OpLoad %v4float %674
%1013 = OpVectorShuffle %v3float %1012 %1012 0 1 2
%1014 = OpVectorTimesScalar %v3float %1013 %1011
%1015 = OpFAdd %v3float %1008 %1014
%1016 = OpLoad %v4float %674
%1017 = OpVectorShuffle %v3float %1016 %1016 0 1 2
%1018 = OpLoad %v4float %675
%1019 = OpVectorShuffle %v3float %1018 %1018 0 1 2
%1020 = OpFMul %v3float %1017 %1019
%1021 = OpFAdd %v3float %1015 %1020
%1022 = OpCompositeExtract %float %1021 0
%1023 = OpCompositeExtract %float %1021 1
%1024 = OpCompositeExtract %float %1021 2
%1025 = OpLoad %v4float %674
%1026 = OpCompositeExtract %float %1025 3
%1027 = OpLoad %v4float %674
%1028 = OpCompositeExtract %float %1027 3
%1029 = OpFSub %float %float_1 %1028
%1030 = OpLoad %v4float %675
%1031 = OpCompositeExtract %float %1030 3
%1032 = OpFMul %float %1029 %1031
%1033 = OpFAdd %float %1026 %1032
%1034 = OpCompositeConstruct %v4float %1022 %1023 %1024 %1033
OpReturnValue %1034
%704 = OpLabel
%1036 = OpLoad %v4float %675
%1037 = OpCompositeExtract %float %1036 3
%1038 = OpLoad %v4float %674
%1039 = OpCompositeExtract %float %1038 3
%1040 = OpFMul %float %1037 %1039
OpStore %_44_alpha %1040
%1042 = OpLoad %v4float %674
%1043 = OpVectorShuffle %v3float %1042 %1042 0 1 2
%1044 = OpLoad %v4float %675
%1045 = OpCompositeExtract %float %1044 3
%1046 = OpVectorTimesScalar %v3float %1043 %1045
OpStore %_45_sda %1046
%1048 = OpLoad %v4float %675
%1049 = OpVectorShuffle %v3float %1048 %1048 0 1 2
%1050 = OpLoad %v4float %674
%1051 = OpCompositeExtract %float %1050 3
%1052 = OpVectorTimesScalar %v3float %1049 %1051
OpStore %_46_dsa %1052
%1053 = OpLoad %v3float %_45_sda
OpStore %1054 %1053
%1055 = OpLoad %v3float %_46_dsa
OpStore %1056 %1055
%1057 = OpFunctionCall %v3float %_blend_set_color_saturation %1054 %1056
OpStore %1058 %1057
%1059 = OpLoad %float %_44_alpha
OpStore %1060 %1059
%1061 = OpLoad %v3float %_46_dsa
OpStore %1062 %1061
%1063 = OpFunctionCall %v3float %_blend_set_color_luminance %1058 %1060 %1062
%1064 = OpLoad %v4float %675
%1065 = OpVectorShuffle %v3float %1064 %1064 0 1 2
%1066 = OpFAdd %v3float %1063 %1065
%1067 = OpLoad %v3float %_46_dsa
%1068 = OpFSub %v3float %1066 %1067
%1069 = OpLoad %v4float %674
%1070 = OpVectorShuffle %v3float %1069 %1069 0 1 2
%1071 = OpFAdd %v3float %1068 %1070
%1072 = OpLoad %v3float %_45_sda
%1073 = OpFSub %v3float %1071 %1072
%1074 = OpCompositeExtract %float %1073 0
%1075 = OpCompositeExtract %float %1073 1
%1076 = OpCompositeExtract %float %1073 2
%1077 = OpLoad %v4float %674
%1078 = OpCompositeExtract %float %1077 3
%1079 = OpLoad %v4float %675
%1080 = OpCompositeExtract %float %1079 3
%1081 = OpFAdd %float %1078 %1080
%1082 = OpLoad %float %_44_alpha
%1083 = OpFSub %float %1081 %1082
%1084 = OpCompositeConstruct %v4float %1074 %1075 %1076 %1083
OpReturnValue %1084
%705 = OpLabel
%1086 = OpLoad %v4float %675
%1087 = OpCompositeExtract %float %1086 3
%1088 = OpLoad %v4float %674
%1089 = OpCompositeExtract %float %1088 3
%1090 = OpFMul %float %1087 %1089
OpStore %_48_alpha %1090
%1092 = OpLoad %v4float %674
%1093 = OpVectorShuffle %v3float %1092 %1092 0 1 2
%1094 = OpLoad %v4float %675
%1095 = OpCompositeExtract %float %1094 3
%1096 = OpVectorTimesScalar %v3float %1093 %1095
OpStore %_49_sda %1096
%1098 = OpLoad %v4float %675
%1099 = OpVectorShuffle %v3float %1098 %1098 0 1 2
%1100 = OpLoad %v4float %674
%1101 = OpCompositeExtract %float %1100 3
%1102 = OpVectorTimesScalar %v3float %1099 %1101
OpStore %_50_dsa %1102
%1103 = OpLoad %v3float %_50_dsa
OpStore %1104 %1103
%1105 = OpLoad %v3float %_49_sda
OpStore %1106 %1105
%1107 = OpFunctionCall %v3float %_blend_set_color_saturation %1104 %1106
OpStore %1108 %1107
%1109 = OpLoad %float %_48_alpha
OpStore %1110 %1109
%1111 = OpLoad %v3float %_50_dsa
OpStore %1112 %1111
%1113 = OpFunctionCall %v3float %_blend_set_color_luminance %1108 %1110 %1112
%1114 = OpLoad %v4float %675
%1115 = OpVectorShuffle %v3float %1114 %1114 0 1 2
%1116 = OpFAdd %v3float %1113 %1115
%1117 = OpLoad %v3float %_50_dsa
%1118 = OpFSub %v3float %1116 %1117
%1119 = OpLoad %v4float %674
%1120 = OpVectorShuffle %v3float %1119 %1119 0 1 2
%1121 = OpFAdd %v3float %1118 %1120
%1122 = OpLoad %v3float %_49_sda
%1123 = OpFSub %v3float %1121 %1122
%1124 = OpCompositeExtract %float %1123 0
%1125 = OpCompositeExtract %float %1123 1
%1126 = OpCompositeExtract %float %1123 2
%1127 = OpLoad %v4float %674
%1128 = OpCompositeExtract %float %1127 3
%1129 = OpLoad %v4float %675
%1130 = OpCompositeExtract %float %1129 3
%1131 = OpFAdd %float %1128 %1130
%1132 = OpLoad %float %_48_alpha
%1133 = OpFSub %float %1131 %1132
%1134 = OpCompositeConstruct %v4float %1124 %1125 %1126 %1133
OpReturnValue %1134
%706 = OpLabel
%1136 = OpLoad %v4float %675
%1137 = OpCompositeExtract %float %1136 3
%1138 = OpLoad %v4float %674
%1139 = OpCompositeExtract %float %1138 3
%1140 = OpFMul %float %1137 %1139
OpStore %_52_alpha %1140
%1142 = OpLoad %v4float %674
%1143 = OpVectorShuffle %v3float %1142 %1142 0 1 2
%1144 = OpLoad %v4float %675
%1145 = OpCompositeExtract %float %1144 3
%1146 = OpVectorTimesScalar %v3float %1143 %1145
OpStore %_53_sda %1146
%1148 = OpLoad %v4float %675
%1149 = OpVectorShuffle %v3float %1148 %1148 0 1 2
%1150 = OpLoad %v4float %674
%1151 = OpCompositeExtract %float %1150 3
%1152 = OpVectorTimesScalar %v3float %1149 %1151
OpStore %_54_dsa %1152
%1153 = OpLoad %v3float %_53_sda
OpStore %1154 %1153
%1155 = OpLoad %float %_52_alpha
OpStore %1156 %1155
%1157 = OpLoad %v3float %_54_dsa
OpStore %1158 %1157
%1159 = OpFunctionCall %v3float %_blend_set_color_luminance %1154 %1156 %1158
%1160 = OpLoad %v4float %675
%1161 = OpVectorShuffle %v3float %1160 %1160 0 1 2
%1162 = OpFAdd %v3float %1159 %1161
%1163 = OpLoad %v3float %_54_dsa
%1164 = OpFSub %v3float %1162 %1163
%1165 = OpLoad %v4float %674
%1166 = OpVectorShuffle %v3float %1165 %1165 0 1 2
%1167 = OpFAdd %v3float %1164 %1166
%1168 = OpLoad %v3float %_53_sda
%1169 = OpFSub %v3float %1167 %1168
%1170 = OpCompositeExtract %float %1169 0
%1171 = OpCompositeExtract %float %1169 1
%1172 = OpCompositeExtract %float %1169 2
%1173 = OpLoad %v4float %674
%1174 = OpCompositeExtract %float %1173 3
%1175 = OpLoad %v4float %675
%1176 = OpCompositeExtract %float %1175 3
%1177 = OpFAdd %float %1174 %1176
%1178 = OpLoad %float %_52_alpha
%1179 = OpFSub %float %1177 %1178
%1180 = OpCompositeConstruct %v4float %1170 %1171 %1172 %1179
OpReturnValue %1180
%707 = OpLabel
%1182 = OpLoad %v4float %675
%1183 = OpCompositeExtract %float %1182 3
%1184 = OpLoad %v4float %674
%1185 = OpCompositeExtract %float %1184 3
%1186 = OpFMul %float %1183 %1185
OpStore %_56_alpha %1186
%1188 = OpLoad %v4float %674
%1189 = OpVectorShuffle %v3float %1188 %1188 0 1 2
%1190 = OpLoad %v4float %675
%1191 = OpCompositeExtract %float %1190 3
%1192 = OpVectorTimesScalar %v3float %1189 %1191
OpStore %_57_sda %1192
%1194 = OpLoad %v4float %675
%1195 = OpVectorShuffle %v3float %1194 %1194 0 1 2
%1196 = OpLoad %v4float %674
%1197 = OpCompositeExtract %float %1196 3
%1198 = OpVectorTimesScalar %v3float %1195 %1197
OpStore %_58_dsa %1198
%1199 = OpLoad %v3float %_58_dsa
OpStore %1200 %1199
%1201 = OpLoad %float %_56_alpha
OpStore %1202 %1201
%1203 = OpLoad %v3float %_57_sda
OpStore %1204 %1203
%1205 = OpFunctionCall %v3float %_blend_set_color_luminance %1200 %1202 %1204
%1206 = OpLoad %v4float %675
%1207 = OpVectorShuffle %v3float %1206 %1206 0 1 2
%1208 = OpFAdd %v3float %1205 %1207
%1209 = OpLoad %v3float %_58_dsa
%1210 = OpFSub %v3float %1208 %1209
%1211 = OpLoad %v4float %674
%1212 = OpVectorShuffle %v3float %1211 %1211 0 1 2
%1213 = OpFAdd %v3float %1210 %1212
%1214 = OpLoad %v3float %_57_sda
%1215 = OpFSub %v3float %1213 %1214
%1216 = OpCompositeExtract %float %1215 0
%1217 = OpCompositeExtract %float %1215 1
%1218 = OpCompositeExtract %float %1215 2
%1219 = OpLoad %v4float %674
%1220 = OpCompositeExtract %float %1219 3
%1221 = OpLoad %v4float %675
%1222 = OpCompositeExtract %float %1221 3
%1223 = OpFAdd %float %1220 %1222
%1224 = OpLoad %float %_56_alpha
%1225 = OpFSub %float %1223 %1224
%1226 = OpCompositeConstruct %v4float %1216 %1217 %1218 %1225
OpReturnValue %1226
%678 = OpLabel
OpReturnValue %1227
OpFunctionEnd
%main = OpFunction %void None %1229
%1230 = OpLabel
%1232 = OpVariable %_ptr_Function_int Function
%1234 = OpVariable %_ptr_Function_v4float Function
%1236 = OpVariable %_ptr_Function_v4float Function
OpStore %1232 %int_13
%1233 = OpLoad %v4float %src
OpStore %1234 %1233
%1235 = OpLoad %v4float %dst
OpStore %1236 %1235
%1237 = OpFunctionCall %v4float %blend %1232 %1234 %1236
OpStore %sk_FragColor %1237
OpReturn
OpFunctionEnd
