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
OpName %main "main"
OpName %_0_blend "_0_blend"
OpName %_1_loop "_1_loop"
OpName %_2_result "_2_result"
OpName %_3_result "_3_result"
OpName %_4_alpha "_4_alpha"
OpName %_5_sda "_5_sda"
OpName %_6_dsa "_6_dsa"
OpName %_7_alpha "_7_alpha"
OpName %_8_sda "_8_sda"
OpName %_9_dsa "_9_dsa"
OpName %_10_alpha "_10_alpha"
OpName %_11_sda "_11_sda"
OpName %_12_dsa "_12_dsa"
OpName %_13_alpha "_13_alpha"
OpName %_14_sda "_14_sda"
OpName %_15_dsa "_15_dsa"
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
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %511 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %560 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %578 RelaxedPrecision
OpDecorate %580 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %609 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %616 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %625 RelaxedPrecision
OpDecorate %629 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %636 RelaxedPrecision
OpDecorate %638 RelaxedPrecision
OpDecorate %644 RelaxedPrecision
OpDecorate %647 RelaxedPrecision
OpDecorate %651 RelaxedPrecision
OpDecorate %653 RelaxedPrecision
OpDecorate %659 RelaxedPrecision
OpDecorate %662 RelaxedPrecision
OpDecorate %666 RelaxedPrecision
OpDecorate %669 RelaxedPrecision
OpDecorate %722 RelaxedPrecision
OpDecorate %723 RelaxedPrecision
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
OpDecorate %738 RelaxedPrecision
OpDecorate %739 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %743 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %751 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
OpDecorate %754 RelaxedPrecision
OpDecorate %756 RelaxedPrecision
OpDecorate %758 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %763 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %769 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %781 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %792 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %794 RelaxedPrecision
OpDecorate %795 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %799 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %804 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %828 RelaxedPrecision
OpDecorate %829 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %847 RelaxedPrecision
OpDecorate %849 RelaxedPrecision
OpDecorate %851 RelaxedPrecision
OpDecorate %852 RelaxedPrecision
OpDecorate %853 RelaxedPrecision
OpDecorate %856 RelaxedPrecision
OpDecorate %860 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %867 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %876 RelaxedPrecision
OpDecorate %878 RelaxedPrecision
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
OpDecorate %915 RelaxedPrecision
OpDecorate %917 RelaxedPrecision
OpDecorate %920 RelaxedPrecision
OpDecorate %927 RelaxedPrecision
OpDecorate %928 RelaxedPrecision
OpDecorate %931 RelaxedPrecision
OpDecorate %935 RelaxedPrecision
OpDecorate %938 RelaxedPrecision
OpDecorate %942 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %953 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %956 RelaxedPrecision
OpDecorate %957 RelaxedPrecision
OpDecorate %959 RelaxedPrecision
OpDecorate %960 RelaxedPrecision
OpDecorate %962 RelaxedPrecision
OpDecorate %964 RelaxedPrecision
OpDecorate %966 RelaxedPrecision
OpDecorate %968 RelaxedPrecision
OpDecorate %971 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %977 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %986 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %989 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %996 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1001 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1013 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1022 RelaxedPrecision
OpDecorate %1024 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1028 RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1045 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1051 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1057 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1072 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1080 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1084 RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1094 RelaxedPrecision
OpDecorate %1095 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1101 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1107 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1113 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1122 RelaxedPrecision
OpDecorate %1124 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1135 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1145 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1151 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1161 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1166 RelaxedPrecision
OpDecorate %1168 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1176 RelaxedPrecision
OpDecorate %1177 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %1181 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1192 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1197 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1207 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1212 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1216 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1221 RelaxedPrecision
OpDecorate %1222 RelaxedPrecision
OpDecorate %1223 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1227 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1232 RelaxedPrecision
OpDecorate %1234 RelaxedPrecision
OpDecorate %1236 RelaxedPrecision
OpDecorate %1237 RelaxedPrecision
OpDecorate %1238 RelaxedPrecision
OpDecorate %1242 RelaxedPrecision
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
%441 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%452 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%539 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%570 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%571 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%void = OpTypeVoid
%674 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_13 = OpConstant %int 13
%721 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
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
%_0_n = OpVariable %_ptr_Function_float Function
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
%169 = OpLoad %v2float %123
%170 = OpCompositeExtract %float %169 0
%171 = OpLoad %v2float %122
%172 = OpCompositeExtract %float %171 1
%173 = OpFMul %float %170 %172
OpStore %_0_n %173
%175 = OpLoad %v2float %123
%176 = OpCompositeExtract %float %175 1
%177 = OpLoad %float %_0_n
%178 = OpLoad %float %delta
%179 = OpFDiv %float %177 %178
%174 = OpExtInst %float %1 FMin %176 %179
OpStore %delta %174
%180 = OpLoad %float %delta
%181 = OpLoad %v2float %122
%182 = OpCompositeExtract %float %181 1
%183 = OpFMul %float %180 %182
%184 = OpLoad %v2float %122
%185 = OpCompositeExtract %float %184 0
%186 = OpLoad %v2float %123
%187 = OpCompositeExtract %float %186 1
%188 = OpFSub %float %float_1 %187
%189 = OpFMul %float %185 %188
%190 = OpFAdd %float %183 %189
%191 = OpLoad %v2float %123
%192 = OpCompositeExtract %float %191 0
%193 = OpLoad %v2float %122
%194 = OpCompositeExtract %float %193 1
%195 = OpFSub %float %float_1 %194
%196 = OpFMul %float %192 %195
%197 = OpFAdd %float %190 %196
OpReturnValue %197
%148 = OpLabel
OpBranch %131
%131 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %22
%198 = OpFunctionParameter %_ptr_Function_v2float
%199 = OpFunctionParameter %_ptr_Function_v2float
%200 = OpLabel
%_1_n = OpVariable %_ptr_Function_float Function
%delta_0 = OpVariable %_ptr_Function_float Function
%201 = OpLoad %v2float %199
%202 = OpCompositeExtract %float %201 1
%203 = OpLoad %v2float %199
%204 = OpCompositeExtract %float %203 0
%205 = OpFOrdEqual %bool %202 %204
OpSelectionMerge %208 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%209 = OpLoad %v2float %198
%210 = OpCompositeExtract %float %209 1
%211 = OpLoad %v2float %199
%212 = OpCompositeExtract %float %211 1
%213 = OpFMul %float %210 %212
%214 = OpLoad %v2float %198
%215 = OpCompositeExtract %float %214 0
%216 = OpLoad %v2float %199
%217 = OpCompositeExtract %float %216 1
%218 = OpFSub %float %float_1 %217
%219 = OpFMul %float %215 %218
%220 = OpFAdd %float %213 %219
%221 = OpLoad %v2float %199
%222 = OpCompositeExtract %float %221 0
%223 = OpLoad %v2float %198
%224 = OpCompositeExtract %float %223 1
%225 = OpFSub %float %float_1 %224
%226 = OpFMul %float %222 %225
%227 = OpFAdd %float %220 %226
OpReturnValue %227
%207 = OpLabel
%228 = OpLoad %v2float %198
%229 = OpCompositeExtract %float %228 0
%230 = OpFOrdEqual %bool %229 %float_0
OpSelectionMerge %233 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%234 = OpLoad %v2float %199
%235 = OpCompositeExtract %float %234 0
%236 = OpLoad %v2float %198
%237 = OpCompositeExtract %float %236 1
%238 = OpFSub %float %float_1 %237
%239 = OpFMul %float %235 %238
OpReturnValue %239
%232 = OpLabel
%241 = OpLoad %v2float %199
%242 = OpCompositeExtract %float %241 1
%243 = OpLoad %v2float %199
%244 = OpCompositeExtract %float %243 0
%245 = OpFSub %float %242 %244
%246 = OpLoad %v2float %198
%247 = OpCompositeExtract %float %246 1
%248 = OpFMul %float %245 %247
OpStore %_1_n %248
%251 = OpLoad %v2float %199
%252 = OpCompositeExtract %float %251 1
%253 = OpLoad %float %_1_n
%254 = OpLoad %v2float %198
%255 = OpCompositeExtract %float %254 0
%256 = OpFDiv %float %253 %255
%257 = OpFSub %float %252 %256
%250 = OpExtInst %float %1 FMax %float_0 %257
OpStore %delta_0 %250
%258 = OpLoad %float %delta_0
%259 = OpLoad %v2float %198
%260 = OpCompositeExtract %float %259 1
%261 = OpFMul %float %258 %260
%262 = OpLoad %v2float %198
%263 = OpCompositeExtract %float %262 0
%264 = OpLoad %v2float %199
%265 = OpCompositeExtract %float %264 1
%266 = OpFSub %float %float_1 %265
%267 = OpFMul %float %263 %266
%268 = OpFAdd %float %261 %267
%269 = OpLoad %v2float %199
%270 = OpCompositeExtract %float %269 0
%271 = OpLoad %v2float %198
%272 = OpCompositeExtract %float %271 1
%273 = OpFSub %float %float_1 %272
%274 = OpFMul %float %270 %273
%275 = OpFAdd %float %268 %274
OpReturnValue %275
%233 = OpLabel
OpBranch %208
%208 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %22
%276 = OpFunctionParameter %_ptr_Function_v2float
%277 = OpFunctionParameter %_ptr_Function_v2float
%278 = OpLabel
%_2_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_3_n = OpVariable %_ptr_Function_float Function
%279 = OpLoad %v2float %276
%280 = OpCompositeExtract %float %279 0
%281 = OpFMul %float %float_2 %280
%282 = OpLoad %v2float %276
%283 = OpCompositeExtract %float %282 1
%284 = OpFOrdLessThanEqual %bool %281 %283
OpSelectionMerge %287 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%289 = OpLoad %v2float %277
%290 = OpCompositeExtract %float %289 0
%291 = OpLoad %v2float %277
%292 = OpCompositeExtract %float %291 0
%293 = OpFMul %float %290 %292
%294 = OpLoad %v2float %276
%295 = OpCompositeExtract %float %294 1
%296 = OpLoad %v2float %276
%297 = OpCompositeExtract %float %296 0
%298 = OpFMul %float %float_2 %297
%299 = OpFSub %float %295 %298
%300 = OpFMul %float %293 %299
OpStore %_2_n %300
%301 = OpLoad %float %_2_n
%302 = OpLoad %v2float %277
%303 = OpCompositeExtract %float %302 1
%304 = OpFDiv %float %301 %303
%305 = OpLoad %v2float %277
%306 = OpCompositeExtract %float %305 1
%307 = OpFSub %float %float_1 %306
%308 = OpLoad %v2float %276
%309 = OpCompositeExtract %float %308 0
%310 = OpFMul %float %307 %309
%311 = OpFAdd %float %304 %310
%312 = OpLoad %v2float %277
%313 = OpCompositeExtract %float %312 0
%315 = OpLoad %v2float %276
%316 = OpCompositeExtract %float %315 1
%314 = OpFNegate %float %316
%317 = OpLoad %v2float %276
%318 = OpCompositeExtract %float %317 0
%319 = OpFMul %float %float_2 %318
%320 = OpFAdd %float %314 %319
%321 = OpFAdd %float %320 %float_1
%322 = OpFMul %float %313 %321
%323 = OpFAdd %float %311 %322
OpReturnValue %323
%286 = OpLabel
%325 = OpLoad %v2float %277
%326 = OpCompositeExtract %float %325 0
%327 = OpFMul %float %float_4 %326
%328 = OpLoad %v2float %277
%329 = OpCompositeExtract %float %328 1
%330 = OpFOrdLessThanEqual %bool %327 %329
OpSelectionMerge %333 None
OpBranchConditional %330 %331 %332
%331 = OpLabel
%335 = OpLoad %v2float %277
%336 = OpCompositeExtract %float %335 0
%337 = OpLoad %v2float %277
%338 = OpCompositeExtract %float %337 0
%339 = OpFMul %float %336 %338
OpStore %DSqd %339
%341 = OpLoad %float %DSqd
%342 = OpLoad %v2float %277
%343 = OpCompositeExtract %float %342 0
%344 = OpFMul %float %341 %343
OpStore %DCub %344
%346 = OpLoad %v2float %277
%347 = OpCompositeExtract %float %346 1
%348 = OpLoad %v2float %277
%349 = OpCompositeExtract %float %348 1
%350 = OpFMul %float %347 %349
OpStore %DaSqd %350
%352 = OpLoad %float %DaSqd
%353 = OpLoad %v2float %277
%354 = OpCompositeExtract %float %353 1
%355 = OpFMul %float %352 %354
OpStore %DaCub %355
%357 = OpLoad %float %DaSqd
%358 = OpLoad %v2float %276
%359 = OpCompositeExtract %float %358 0
%360 = OpLoad %v2float %277
%361 = OpCompositeExtract %float %360 0
%363 = OpLoad %v2float %276
%364 = OpCompositeExtract %float %363 1
%365 = OpFMul %float %float_3 %364
%367 = OpLoad %v2float %276
%368 = OpCompositeExtract %float %367 0
%369 = OpFMul %float %float_6 %368
%370 = OpFSub %float %365 %369
%371 = OpFSub %float %370 %float_1
%372 = OpFMul %float %361 %371
%373 = OpFSub %float %359 %372
%374 = OpFMul %float %357 %373
%376 = OpLoad %v2float %277
%377 = OpCompositeExtract %float %376 1
%378 = OpFMul %float %float_12 %377
%379 = OpLoad %float %DSqd
%380 = OpFMul %float %378 %379
%381 = OpLoad %v2float %276
%382 = OpCompositeExtract %float %381 1
%383 = OpLoad %v2float %276
%384 = OpCompositeExtract %float %383 0
%385 = OpFMul %float %float_2 %384
%386 = OpFSub %float %382 %385
%387 = OpFMul %float %380 %386
%388 = OpFAdd %float %374 %387
%390 = OpLoad %float %DCub
%391 = OpFMul %float %float_16 %390
%392 = OpLoad %v2float %276
%393 = OpCompositeExtract %float %392 1
%394 = OpLoad %v2float %276
%395 = OpCompositeExtract %float %394 0
%396 = OpFMul %float %float_2 %395
%397 = OpFSub %float %393 %396
%398 = OpFMul %float %391 %397
%399 = OpFSub %float %388 %398
%400 = OpLoad %float %DaCub
%401 = OpLoad %v2float %276
%402 = OpCompositeExtract %float %401 0
%403 = OpFMul %float %400 %402
%404 = OpFSub %float %399 %403
OpStore %_3_n %404
%405 = OpLoad %float %_3_n
%406 = OpLoad %float %DaSqd
%407 = OpFDiv %float %405 %406
OpReturnValue %407
%332 = OpLabel
%408 = OpLoad %v2float %277
%409 = OpCompositeExtract %float %408 0
%410 = OpLoad %v2float %276
%411 = OpCompositeExtract %float %410 1
%412 = OpLoad %v2float %276
%413 = OpCompositeExtract %float %412 0
%414 = OpFMul %float %float_2 %413
%415 = OpFSub %float %411 %414
%416 = OpFAdd %float %415 %float_1
%417 = OpFMul %float %409 %416
%418 = OpLoad %v2float %276
%419 = OpCompositeExtract %float %418 0
%420 = OpFAdd %float %417 %419
%422 = OpLoad %v2float %277
%423 = OpCompositeExtract %float %422 1
%424 = OpLoad %v2float %277
%425 = OpCompositeExtract %float %424 0
%426 = OpFMul %float %423 %425
%421 = OpExtInst %float %1 Sqrt %426
%427 = OpLoad %v2float %276
%428 = OpCompositeExtract %float %427 1
%429 = OpLoad %v2float %276
%430 = OpCompositeExtract %float %429 0
%431 = OpFMul %float %float_2 %430
%432 = OpFSub %float %428 %431
%433 = OpFMul %float %421 %432
%434 = OpFSub %float %420 %433
%435 = OpLoad %v2float %277
%436 = OpCompositeExtract %float %435 1
%437 = OpLoad %v2float %276
%438 = OpCompositeExtract %float %437 0
%439 = OpFMul %float %436 %438
%440 = OpFSub %float %434 %439
OpReturnValue %440
%333 = OpLabel
OpBranch %287
%287 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %441
%443 = OpFunctionParameter %_ptr_Function_v3float
%444 = OpFunctionParameter %_ptr_Function_float
%445 = OpFunctionParameter %_ptr_Function_v3float
%446 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_0 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%_4_d = OpVariable %_ptr_Function_float Function
%_5_n = OpVariable %_ptr_Function_v3float Function
%_6_d = OpVariable %_ptr_Function_float Function
%453 = OpLoad %v3float %445
%448 = OpDot %float %452 %453
OpStore %lum %448
%455 = OpLoad %float %lum
%457 = OpLoad %v3float %443
%456 = OpDot %float %452 %457
%458 = OpFSub %float %455 %456
%459 = OpLoad %v3float %443
%460 = OpCompositeConstruct %v3float %458 %458 %458
%461 = OpFAdd %v3float %460 %459
OpStore %result_0 %461
%465 = OpLoad %v3float %result_0
%466 = OpCompositeExtract %float %465 0
%467 = OpLoad %v3float %result_0
%468 = OpCompositeExtract %float %467 1
%464 = OpExtInst %float %1 FMin %466 %468
%469 = OpLoad %v3float %result_0
%470 = OpCompositeExtract %float %469 2
%463 = OpExtInst %float %1 FMin %464 %470
OpStore %minComp %463
%474 = OpLoad %v3float %result_0
%475 = OpCompositeExtract %float %474 0
%476 = OpLoad %v3float %result_0
%477 = OpCompositeExtract %float %476 1
%473 = OpExtInst %float %1 FMax %475 %477
%478 = OpLoad %v3float %result_0
%479 = OpCompositeExtract %float %478 2
%472 = OpExtInst %float %1 FMax %473 %479
OpStore %maxComp %472
%481 = OpLoad %float %minComp
%482 = OpFOrdLessThan %bool %481 %float_0
OpSelectionMerge %484 None
OpBranchConditional %482 %483 %484
%483 = OpLabel
%485 = OpLoad %float %lum
%486 = OpLoad %float %minComp
%487 = OpFOrdNotEqual %bool %485 %486
OpBranch %484
%484 = OpLabel
%488 = OpPhi %bool %false %446 %487 %483
OpSelectionMerge %490 None
OpBranchConditional %488 %489 %490
%489 = OpLabel
%492 = OpLoad %float %lum
%493 = OpLoad %float %minComp
%494 = OpFSub %float %492 %493
OpStore %_4_d %494
%495 = OpLoad %float %lum
%496 = OpLoad %v3float %result_0
%497 = OpLoad %float %lum
%498 = OpCompositeConstruct %v3float %497 %497 %497
%499 = OpFSub %v3float %496 %498
%500 = OpLoad %float %lum
%501 = OpLoad %float %_4_d
%502 = OpFDiv %float %500 %501
%503 = OpVectorTimesScalar %v3float %499 %502
%504 = OpCompositeConstruct %v3float %495 %495 %495
%505 = OpFAdd %v3float %504 %503
OpStore %result_0 %505
OpBranch %490
%490 = OpLabel
%506 = OpLoad %float %maxComp
%507 = OpLoad %float %444
%508 = OpFOrdGreaterThan %bool %506 %507
OpSelectionMerge %510 None
OpBranchConditional %508 %509 %510
%509 = OpLabel
%511 = OpLoad %float %maxComp
%512 = OpLoad %float %lum
%513 = OpFOrdNotEqual %bool %511 %512
OpBranch %510
%510 = OpLabel
%514 = OpPhi %bool %false %490 %513 %509
OpSelectionMerge %517 None
OpBranchConditional %514 %515 %516
%515 = OpLabel
%519 = OpLoad %v3float %result_0
%520 = OpLoad %float %lum
%521 = OpCompositeConstruct %v3float %520 %520 %520
%522 = OpFSub %v3float %519 %521
%523 = OpLoad %float %444
%524 = OpLoad %float %lum
%525 = OpFSub %float %523 %524
%526 = OpVectorTimesScalar %v3float %522 %525
OpStore %_5_n %526
%528 = OpLoad %float %maxComp
%529 = OpLoad %float %lum
%530 = OpFSub %float %528 %529
OpStore %_6_d %530
%531 = OpLoad %float %lum
%532 = OpLoad %v3float %_5_n
%533 = OpLoad %float %_6_d
%534 = OpFDiv %float %float_1 %533
%535 = OpVectorTimesScalar %v3float %532 %534
%536 = OpCompositeConstruct %v3float %531 %531 %531
%537 = OpFAdd %v3float %536 %535
OpReturnValue %537
%516 = OpLabel
%538 = OpLoad %v3float %result_0
OpReturnValue %538
%517 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %539
%540 = OpFunctionParameter %_ptr_Function_v3float
%541 = OpFunctionParameter %_ptr_Function_float
%542 = OpLabel
%_7_n = OpVariable %_ptr_Function_float Function
%_8_d = OpVariable %_ptr_Function_float Function
%543 = OpLoad %v3float %540
%544 = OpCompositeExtract %float %543 0
%545 = OpLoad %v3float %540
%546 = OpCompositeExtract %float %545 2
%547 = OpFOrdLessThan %bool %544 %546
OpSelectionMerge %550 None
OpBranchConditional %547 %548 %549
%548 = OpLabel
%552 = OpLoad %float %541
%553 = OpLoad %v3float %540
%554 = OpCompositeExtract %float %553 1
%555 = OpLoad %v3float %540
%556 = OpCompositeExtract %float %555 0
%557 = OpFSub %float %554 %556
%558 = OpFMul %float %552 %557
OpStore %_7_n %558
%560 = OpLoad %v3float %540
%561 = OpCompositeExtract %float %560 2
%562 = OpLoad %v3float %540
%563 = OpCompositeExtract %float %562 0
%564 = OpFSub %float %561 %563
OpStore %_8_d %564
%565 = OpLoad %float %_7_n
%566 = OpLoad %float %_8_d
%567 = OpFDiv %float %565 %566
%568 = OpLoad %float %541
%569 = OpCompositeConstruct %v3float %float_0 %567 %568
OpReturnValue %569
%549 = OpLabel
OpReturnValue %570
%550 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %571
%572 = OpFunctionParameter %_ptr_Function_v3float
%573 = OpFunctionParameter %_ptr_Function_v3float
%574 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%610 = OpVariable %_ptr_Function_v3float Function
%612 = OpVariable %_ptr_Function_float Function
%624 = OpVariable %_ptr_Function_v3float Function
%626 = OpVariable %_ptr_Function_float Function
%631 = OpVariable %_ptr_Function_v3float Function
%633 = OpVariable %_ptr_Function_float Function
%646 = OpVariable %_ptr_Function_v3float Function
%648 = OpVariable %_ptr_Function_float Function
%661 = OpVariable %_ptr_Function_v3float Function
%663 = OpVariable %_ptr_Function_float Function
%668 = OpVariable %_ptr_Function_v3float Function
%670 = OpVariable %_ptr_Function_float Function
%578 = OpLoad %v3float %573
%579 = OpCompositeExtract %float %578 0
%580 = OpLoad %v3float %573
%581 = OpCompositeExtract %float %580 1
%577 = OpExtInst %float %1 FMax %579 %581
%582 = OpLoad %v3float %573
%583 = OpCompositeExtract %float %582 2
%576 = OpExtInst %float %1 FMax %577 %583
%586 = OpLoad %v3float %573
%587 = OpCompositeExtract %float %586 0
%588 = OpLoad %v3float %573
%589 = OpCompositeExtract %float %588 1
%585 = OpExtInst %float %1 FMin %587 %589
%590 = OpLoad %v3float %573
%591 = OpCompositeExtract %float %590 2
%584 = OpExtInst %float %1 FMin %585 %591
%592 = OpFSub %float %576 %584
OpStore %sat %592
%593 = OpLoad %v3float %572
%594 = OpCompositeExtract %float %593 0
%595 = OpLoad %v3float %572
%596 = OpCompositeExtract %float %595 1
%597 = OpFOrdLessThanEqual %bool %594 %596
OpSelectionMerge %600 None
OpBranchConditional %597 %598 %599
%598 = OpLabel
%601 = OpLoad %v3float %572
%602 = OpCompositeExtract %float %601 1
%603 = OpLoad %v3float %572
%604 = OpCompositeExtract %float %603 2
%605 = OpFOrdLessThanEqual %bool %602 %604
OpSelectionMerge %608 None
OpBranchConditional %605 %606 %607
%606 = OpLabel
%609 = OpLoad %v3float %572
OpStore %610 %609
%611 = OpLoad %float %sat
OpStore %612 %611
%613 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %610 %612
OpReturnValue %613
%607 = OpLabel
%614 = OpLoad %v3float %572
%615 = OpCompositeExtract %float %614 0
%616 = OpLoad %v3float %572
%617 = OpCompositeExtract %float %616 2
%618 = OpFOrdLessThanEqual %bool %615 %617
OpSelectionMerge %621 None
OpBranchConditional %618 %619 %620
%619 = OpLabel
%622 = OpLoad %v3float %572
%623 = OpVectorShuffle %v3float %622 %622 0 2 1
OpStore %624 %623
%625 = OpLoad %float %sat
OpStore %626 %625
%627 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %624 %626
%628 = OpVectorShuffle %v3float %627 %627 0 2 1
OpReturnValue %628
%620 = OpLabel
%629 = OpLoad %v3float %572
%630 = OpVectorShuffle %v3float %629 %629 2 0 1
OpStore %631 %630
%632 = OpLoad %float %sat
OpStore %633 %632
%634 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %631 %633
%635 = OpVectorShuffle %v3float %634 %634 1 2 0
OpReturnValue %635
%621 = OpLabel
OpBranch %608
%608 = OpLabel
OpBranch %600
%599 = OpLabel
%636 = OpLoad %v3float %572
%637 = OpCompositeExtract %float %636 0
%638 = OpLoad %v3float %572
%639 = OpCompositeExtract %float %638 2
%640 = OpFOrdLessThanEqual %bool %637 %639
OpSelectionMerge %643 None
OpBranchConditional %640 %641 %642
%641 = OpLabel
%644 = OpLoad %v3float %572
%645 = OpVectorShuffle %v3float %644 %644 1 0 2
OpStore %646 %645
%647 = OpLoad %float %sat
OpStore %648 %647
%649 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %646 %648
%650 = OpVectorShuffle %v3float %649 %649 1 0 2
OpReturnValue %650
%642 = OpLabel
%651 = OpLoad %v3float %572
%652 = OpCompositeExtract %float %651 1
%653 = OpLoad %v3float %572
%654 = OpCompositeExtract %float %653 2
%655 = OpFOrdLessThanEqual %bool %652 %654
OpSelectionMerge %658 None
OpBranchConditional %655 %656 %657
%656 = OpLabel
%659 = OpLoad %v3float %572
%660 = OpVectorShuffle %v3float %659 %659 1 2 0
OpStore %661 %660
%662 = OpLoad %float %sat
OpStore %663 %662
%664 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %661 %663
%665 = OpVectorShuffle %v3float %664 %664 2 0 1
OpReturnValue %665
%657 = OpLabel
%666 = OpLoad %v3float %572
%667 = OpVectorShuffle %v3float %666 %666 2 1 0
OpStore %668 %667
%669 = OpLoad %float %sat
OpStore %670 %669
%671 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %668 %670
%672 = OpVectorShuffle %v3float %671 %671 2 1 0
OpReturnValue %672
%658 = OpLabel
OpBranch %643
%643 = OpLabel
OpBranch %600
%600 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %674
%675 = OpLabel
%_0_blend = OpVariable %_ptr_Function_v4float Function
%_1_loop = OpVariable %_ptr_Function_int Function
%803 = OpVariable %_ptr_Function_v4float Function
%805 = OpVariable %_ptr_Function_v4float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%_3_result = OpVariable %_ptr_Function_v4float Function
%855 = OpVariable %_ptr_Function_v2float Function
%858 = OpVariable %_ptr_Function_v2float Function
%862 = OpVariable %_ptr_Function_v2float Function
%865 = OpVariable %_ptr_Function_v2float Function
%869 = OpVariable %_ptr_Function_v2float Function
%872 = OpVariable %_ptr_Function_v2float Function
%886 = OpVariable %_ptr_Function_v2float Function
%889 = OpVariable %_ptr_Function_v2float Function
%893 = OpVariable %_ptr_Function_v2float Function
%896 = OpVariable %_ptr_Function_v2float Function
%900 = OpVariable %_ptr_Function_v2float Function
%903 = OpVariable %_ptr_Function_v2float Function
%916 = OpVariable %_ptr_Function_v4float Function
%918 = OpVariable %_ptr_Function_v4float Function
%923 = OpVariable %_ptr_Function_v4float Function
%930 = OpVariable %_ptr_Function_v2float Function
%933 = OpVariable %_ptr_Function_v2float Function
%937 = OpVariable %_ptr_Function_v2float Function
%940 = OpVariable %_ptr_Function_v2float Function
%944 = OpVariable %_ptr_Function_v2float Function
%947 = OpVariable %_ptr_Function_v2float Function
%_4_alpha = OpVariable %_ptr_Function_float Function
%_5_sda = OpVariable %_ptr_Function_v3float Function
%_6_dsa = OpVariable %_ptr_Function_v3float Function
%1067 = OpVariable %_ptr_Function_v3float Function
%1069 = OpVariable %_ptr_Function_v3float Function
%1071 = OpVariable %_ptr_Function_v3float Function
%1073 = OpVariable %_ptr_Function_float Function
%1075 = OpVariable %_ptr_Function_v3float Function
%_7_alpha = OpVariable %_ptr_Function_float Function
%_8_sda = OpVariable %_ptr_Function_v3float Function
%_9_dsa = OpVariable %_ptr_Function_v3float Function
%1117 = OpVariable %_ptr_Function_v3float Function
%1119 = OpVariable %_ptr_Function_v3float Function
%1121 = OpVariable %_ptr_Function_v3float Function
%1123 = OpVariable %_ptr_Function_float Function
%1125 = OpVariable %_ptr_Function_v3float Function
%_10_alpha = OpVariable %_ptr_Function_float Function
%_11_sda = OpVariable %_ptr_Function_v3float Function
%_12_dsa = OpVariable %_ptr_Function_v3float Function
%1167 = OpVariable %_ptr_Function_v3float Function
%1169 = OpVariable %_ptr_Function_float Function
%1171 = OpVariable %_ptr_Function_v3float Function
%_13_alpha = OpVariable %_ptr_Function_float Function
%_14_sda = OpVariable %_ptr_Function_v3float Function
%_15_dsa = OpVariable %_ptr_Function_v3float Function
%1213 = OpVariable %_ptr_Function_v3float Function
%1215 = OpVariable %_ptr_Function_float Function
%1217 = OpVariable %_ptr_Function_v3float Function
OpStore %_1_loop %int_0
OpBranch %681
%681 = OpLabel
OpLoopMerge %685 %684 None
OpBranch %682
%682 = OpLabel
%686 = OpLoad %int %_1_loop
%688 = OpSLessThan %bool %686 %int_1
OpBranchConditional %688 %683 %685
%683 = OpLabel
OpSelectionMerge %690 None
OpSwitch %int_13 %720 0 %691 1 %692 2 %693 3 %694 4 %695 5 %696 6 %697 7 %698 8 %699 9 %700 10 %701 11 %702 12 %703 13 %704 14 %705 15 %706 16 %707 17 %708 18 %709 19 %710 20 %711 21 %712 22 %713 23 %714 24 %715 25 %716 26 %717 27 %718 28 %719
%691 = OpLabel
OpStore %_0_blend %721
OpBranch %684
%692 = OpLabel
%722 = OpLoad %v4float %src
OpStore %_0_blend %722
OpBranch %684
%693 = OpLabel
%723 = OpLoad %v4float %dst
OpStore %_0_blend %723
OpBranch %684
%694 = OpLabel
%724 = OpLoad %v4float %src
%725 = OpLoad %v4float %src
%726 = OpCompositeExtract %float %725 3
%727 = OpFSub %float %float_1 %726
%728 = OpLoad %v4float %dst
%729 = OpVectorTimesScalar %v4float %728 %727
%730 = OpFAdd %v4float %724 %729
OpStore %_0_blend %730
OpBranch %684
%695 = OpLabel
%731 = OpLoad %v4float %dst
%732 = OpCompositeExtract %float %731 3
%733 = OpFSub %float %float_1 %732
%734 = OpLoad %v4float %src
%735 = OpVectorTimesScalar %v4float %734 %733
%736 = OpLoad %v4float %dst
%737 = OpFAdd %v4float %735 %736
OpStore %_0_blend %737
OpBranch %684
%696 = OpLabel
%738 = OpLoad %v4float %src
%739 = OpLoad %v4float %dst
%740 = OpCompositeExtract %float %739 3
%741 = OpVectorTimesScalar %v4float %738 %740
OpStore %_0_blend %741
OpBranch %684
%697 = OpLabel
%742 = OpLoad %v4float %dst
%743 = OpLoad %v4float %src
%744 = OpCompositeExtract %float %743 3
%745 = OpVectorTimesScalar %v4float %742 %744
OpStore %_0_blend %745
OpBranch %684
%698 = OpLabel
%746 = OpLoad %v4float %dst
%747 = OpCompositeExtract %float %746 3
%748 = OpFSub %float %float_1 %747
%749 = OpLoad %v4float %src
%750 = OpVectorTimesScalar %v4float %749 %748
OpStore %_0_blend %750
OpBranch %684
%699 = OpLabel
%751 = OpLoad %v4float %src
%752 = OpCompositeExtract %float %751 3
%753 = OpFSub %float %float_1 %752
%754 = OpLoad %v4float %dst
%755 = OpVectorTimesScalar %v4float %754 %753
OpStore %_0_blend %755
OpBranch %684
%700 = OpLabel
%756 = OpLoad %v4float %dst
%757 = OpCompositeExtract %float %756 3
%758 = OpLoad %v4float %src
%759 = OpVectorTimesScalar %v4float %758 %757
%760 = OpLoad %v4float %src
%761 = OpCompositeExtract %float %760 3
%762 = OpFSub %float %float_1 %761
%763 = OpLoad %v4float %dst
%764 = OpVectorTimesScalar %v4float %763 %762
%765 = OpFAdd %v4float %759 %764
OpStore %_0_blend %765
OpBranch %684
%701 = OpLabel
%766 = OpLoad %v4float %dst
%767 = OpCompositeExtract %float %766 3
%768 = OpFSub %float %float_1 %767
%769 = OpLoad %v4float %src
%770 = OpVectorTimesScalar %v4float %769 %768
%771 = OpLoad %v4float %src
%772 = OpCompositeExtract %float %771 3
%773 = OpLoad %v4float %dst
%774 = OpVectorTimesScalar %v4float %773 %772
%775 = OpFAdd %v4float %770 %774
OpStore %_0_blend %775
OpBranch %684
%702 = OpLabel
%776 = OpLoad %v4float %dst
%777 = OpCompositeExtract %float %776 3
%778 = OpFSub %float %float_1 %777
%779 = OpLoad %v4float %src
%780 = OpVectorTimesScalar %v4float %779 %778
%781 = OpLoad %v4float %src
%782 = OpCompositeExtract %float %781 3
%783 = OpFSub %float %float_1 %782
%784 = OpLoad %v4float %dst
%785 = OpVectorTimesScalar %v4float %784 %783
%786 = OpFAdd %v4float %780 %785
OpStore %_0_blend %786
OpBranch %684
%703 = OpLabel
%788 = OpLoad %v4float %src
%789 = OpLoad %v4float %dst
%790 = OpFAdd %v4float %788 %789
%791 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%787 = OpExtInst %v4float %1 FMin %790 %791
OpStore %_0_blend %787
OpBranch %684
%704 = OpLabel
%792 = OpLoad %v4float %src
%793 = OpLoad %v4float %dst
%794 = OpFMul %v4float %792 %793
OpStore %_0_blend %794
OpBranch %684
%705 = OpLabel
%795 = OpLoad %v4float %src
%796 = OpLoad %v4float %src
%797 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%798 = OpFSub %v4float %797 %796
%799 = OpLoad %v4float %dst
%800 = OpFMul %v4float %798 %799
%801 = OpFAdd %v4float %795 %800
OpStore %_0_blend %801
OpBranch %684
%706 = OpLabel
%802 = OpLoad %v4float %src
OpStore %803 %802
%804 = OpLoad %v4float %dst
OpStore %805 %804
%806 = OpFunctionCall %v4float %blend_overlay %803 %805
OpStore %_0_blend %806
OpBranch %684
%707 = OpLabel
%808 = OpLoad %v4float %src
%809 = OpLoad %v4float %src
%810 = OpCompositeExtract %float %809 3
%811 = OpFSub %float %float_1 %810
%812 = OpLoad %v4float %dst
%813 = OpVectorTimesScalar %v4float %812 %811
%814 = OpFAdd %v4float %808 %813
OpStore %_2_result %814
%816 = OpLoad %v4float %_2_result
%817 = OpVectorShuffle %v3float %816 %816 0 1 2
%818 = OpLoad %v4float %dst
%819 = OpCompositeExtract %float %818 3
%820 = OpFSub %float %float_1 %819
%821 = OpLoad %v4float %src
%822 = OpVectorShuffle %v3float %821 %821 0 1 2
%823 = OpVectorTimesScalar %v3float %822 %820
%824 = OpLoad %v4float %dst
%825 = OpVectorShuffle %v3float %824 %824 0 1 2
%826 = OpFAdd %v3float %823 %825
%815 = OpExtInst %v3float %1 FMin %817 %826
%827 = OpLoad %v4float %_2_result
%828 = OpVectorShuffle %v4float %827 %815 4 5 6 3
OpStore %_2_result %828
%829 = OpLoad %v4float %_2_result
OpStore %_0_blend %829
OpBranch %684
%708 = OpLabel
%831 = OpLoad %v4float %src
%832 = OpLoad %v4float %src
%833 = OpCompositeExtract %float %832 3
%834 = OpFSub %float %float_1 %833
%835 = OpLoad %v4float %dst
%836 = OpVectorTimesScalar %v4float %835 %834
%837 = OpFAdd %v4float %831 %836
OpStore %_3_result %837
%839 = OpLoad %v4float %_3_result
%840 = OpVectorShuffle %v3float %839 %839 0 1 2
%841 = OpLoad %v4float %dst
%842 = OpCompositeExtract %float %841 3
%843 = OpFSub %float %float_1 %842
%844 = OpLoad %v4float %src
%845 = OpVectorShuffle %v3float %844 %844 0 1 2
%846 = OpVectorTimesScalar %v3float %845 %843
%847 = OpLoad %v4float %dst
%848 = OpVectorShuffle %v3float %847 %847 0 1 2
%849 = OpFAdd %v3float %846 %848
%838 = OpExtInst %v3float %1 FMax %840 %849
%850 = OpLoad %v4float %_3_result
%851 = OpVectorShuffle %v4float %850 %838 4 5 6 3
OpStore %_3_result %851
%852 = OpLoad %v4float %_3_result
OpStore %_0_blend %852
OpBranch %684
%709 = OpLabel
%853 = OpLoad %v4float %src
%854 = OpVectorShuffle %v2float %853 %853 0 3
OpStore %855 %854
%856 = OpLoad %v4float %dst
%857 = OpVectorShuffle %v2float %856 %856 0 3
OpStore %858 %857
%859 = OpFunctionCall %float %_color_dodge_component %855 %858
%860 = OpLoad %v4float %src
%861 = OpVectorShuffle %v2float %860 %860 1 3
OpStore %862 %861
%863 = OpLoad %v4float %dst
%864 = OpVectorShuffle %v2float %863 %863 1 3
OpStore %865 %864
%866 = OpFunctionCall %float %_color_dodge_component %862 %865
%867 = OpLoad %v4float %src
%868 = OpVectorShuffle %v2float %867 %867 2 3
OpStore %869 %868
%870 = OpLoad %v4float %dst
%871 = OpVectorShuffle %v2float %870 %870 2 3
OpStore %872 %871
%873 = OpFunctionCall %float %_color_dodge_component %869 %872
%874 = OpLoad %v4float %src
%875 = OpCompositeExtract %float %874 3
%876 = OpLoad %v4float %src
%877 = OpCompositeExtract %float %876 3
%878 = OpFSub %float %float_1 %877
%879 = OpLoad %v4float %dst
%880 = OpCompositeExtract %float %879 3
%881 = OpFMul %float %878 %880
%882 = OpFAdd %float %875 %881
%883 = OpCompositeConstruct %v4float %859 %866 %873 %882
OpStore %_0_blend %883
OpBranch %684
%710 = OpLabel
%884 = OpLoad %v4float %src
%885 = OpVectorShuffle %v2float %884 %884 0 3
OpStore %886 %885
%887 = OpLoad %v4float %dst
%888 = OpVectorShuffle %v2float %887 %887 0 3
OpStore %889 %888
%890 = OpFunctionCall %float %_color_burn_component %886 %889
%891 = OpLoad %v4float %src
%892 = OpVectorShuffle %v2float %891 %891 1 3
OpStore %893 %892
%894 = OpLoad %v4float %dst
%895 = OpVectorShuffle %v2float %894 %894 1 3
OpStore %896 %895
%897 = OpFunctionCall %float %_color_burn_component %893 %896
%898 = OpLoad %v4float %src
%899 = OpVectorShuffle %v2float %898 %898 2 3
OpStore %900 %899
%901 = OpLoad %v4float %dst
%902 = OpVectorShuffle %v2float %901 %901 2 3
OpStore %903 %902
%904 = OpFunctionCall %float %_color_burn_component %900 %903
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
OpBranch %684
%711 = OpLabel
%915 = OpLoad %v4float %dst
OpStore %916 %915
%917 = OpLoad %v4float %src
OpStore %918 %917
%919 = OpFunctionCall %v4float %blend_overlay %916 %918
OpStore %_0_blend %919
OpBranch %684
%712 = OpLabel
%920 = OpLoad %v4float %dst
%921 = OpCompositeExtract %float %920 3
%922 = OpFOrdEqual %bool %921 %float_0
OpSelectionMerge %926 None
OpBranchConditional %922 %924 %925
%924 = OpLabel
%927 = OpLoad %v4float %src
OpStore %923 %927
OpBranch %926
%925 = OpLabel
%928 = OpLoad %v4float %src
%929 = OpVectorShuffle %v2float %928 %928 0 3
OpStore %930 %929
%931 = OpLoad %v4float %dst
%932 = OpVectorShuffle %v2float %931 %931 0 3
OpStore %933 %932
%934 = OpFunctionCall %float %_soft_light_component %930 %933
%935 = OpLoad %v4float %src
%936 = OpVectorShuffle %v2float %935 %935 1 3
OpStore %937 %936
%938 = OpLoad %v4float %dst
%939 = OpVectorShuffle %v2float %938 %938 1 3
OpStore %940 %939
%941 = OpFunctionCall %float %_soft_light_component %937 %940
%942 = OpLoad %v4float %src
%943 = OpVectorShuffle %v2float %942 %942 2 3
OpStore %944 %943
%945 = OpLoad %v4float %dst
%946 = OpVectorShuffle %v2float %945 %945 2 3
OpStore %947 %946
%948 = OpFunctionCall %float %_soft_light_component %944 %947
%949 = OpLoad %v4float %src
%950 = OpCompositeExtract %float %949 3
%951 = OpLoad %v4float %src
%952 = OpCompositeExtract %float %951 3
%953 = OpFSub %float %float_1 %952
%954 = OpLoad %v4float %dst
%955 = OpCompositeExtract %float %954 3
%956 = OpFMul %float %953 %955
%957 = OpFAdd %float %950 %956
%958 = OpCompositeConstruct %v4float %934 %941 %948 %957
OpStore %923 %958
OpBranch %926
%926 = OpLabel
%959 = OpLoad %v4float %923
OpStore %_0_blend %959
OpBranch %684
%713 = OpLabel
%960 = OpLoad %v4float %src
%961 = OpVectorShuffle %v3float %960 %960 0 1 2
%962 = OpLoad %v4float %dst
%963 = OpVectorShuffle %v3float %962 %962 0 1 2
%964 = OpFAdd %v3float %961 %963
%966 = OpLoad %v4float %src
%967 = OpVectorShuffle %v3float %966 %966 0 1 2
%968 = OpLoad %v4float %dst
%969 = OpCompositeExtract %float %968 3
%970 = OpVectorTimesScalar %v3float %967 %969
%971 = OpLoad %v4float %dst
%972 = OpVectorShuffle %v3float %971 %971 0 1 2
%973 = OpLoad %v4float %src
%974 = OpCompositeExtract %float %973 3
%975 = OpVectorTimesScalar %v3float %972 %974
%965 = OpExtInst %v3float %1 FMin %970 %975
%976 = OpVectorTimesScalar %v3float %965 %float_2
%977 = OpFSub %v3float %964 %976
%978 = OpCompositeExtract %float %977 0
%979 = OpCompositeExtract %float %977 1
%980 = OpCompositeExtract %float %977 2
%981 = OpLoad %v4float %src
%982 = OpCompositeExtract %float %981 3
%983 = OpLoad %v4float %src
%984 = OpCompositeExtract %float %983 3
%985 = OpFSub %float %float_1 %984
%986 = OpLoad %v4float %dst
%987 = OpCompositeExtract %float %986 3
%988 = OpFMul %float %985 %987
%989 = OpFAdd %float %982 %988
%990 = OpCompositeConstruct %v4float %978 %979 %980 %989
OpStore %_0_blend %990
OpBranch %684
%714 = OpLabel
%991 = OpLoad %v4float %dst
%992 = OpVectorShuffle %v3float %991 %991 0 1 2
%993 = OpLoad %v4float %src
%994 = OpVectorShuffle %v3float %993 %993 0 1 2
%995 = OpFAdd %v3float %992 %994
%996 = OpLoad %v4float %dst
%997 = OpVectorShuffle %v3float %996 %996 0 1 2
%998 = OpVectorTimesScalar %v3float %997 %float_2
%999 = OpLoad %v4float %src
%1000 = OpVectorShuffle %v3float %999 %999 0 1 2
%1001 = OpFMul %v3float %998 %1000
%1002 = OpFSub %v3float %995 %1001
%1003 = OpCompositeExtract %float %1002 0
%1004 = OpCompositeExtract %float %1002 1
%1005 = OpCompositeExtract %float %1002 2
%1006 = OpLoad %v4float %src
%1007 = OpCompositeExtract %float %1006 3
%1008 = OpLoad %v4float %src
%1009 = OpCompositeExtract %float %1008 3
%1010 = OpFSub %float %float_1 %1009
%1011 = OpLoad %v4float %dst
%1012 = OpCompositeExtract %float %1011 3
%1013 = OpFMul %float %1010 %1012
%1014 = OpFAdd %float %1007 %1013
%1015 = OpCompositeConstruct %v4float %1003 %1004 %1005 %1014
OpStore %_0_blend %1015
OpBranch %684
%715 = OpLabel
%1016 = OpLoad %v4float %src
%1017 = OpCompositeExtract %float %1016 3
%1018 = OpFSub %float %float_1 %1017
%1019 = OpLoad %v4float %dst
%1020 = OpVectorShuffle %v3float %1019 %1019 0 1 2
%1021 = OpVectorTimesScalar %v3float %1020 %1018
%1022 = OpLoad %v4float %dst
%1023 = OpCompositeExtract %float %1022 3
%1024 = OpFSub %float %float_1 %1023
%1025 = OpLoad %v4float %src
%1026 = OpVectorShuffle %v3float %1025 %1025 0 1 2
%1027 = OpVectorTimesScalar %v3float %1026 %1024
%1028 = OpFAdd %v3float %1021 %1027
%1029 = OpLoad %v4float %src
%1030 = OpVectorShuffle %v3float %1029 %1029 0 1 2
%1031 = OpLoad %v4float %dst
%1032 = OpVectorShuffle %v3float %1031 %1031 0 1 2
%1033 = OpFMul %v3float %1030 %1032
%1034 = OpFAdd %v3float %1028 %1033
%1035 = OpCompositeExtract %float %1034 0
%1036 = OpCompositeExtract %float %1034 1
%1037 = OpCompositeExtract %float %1034 2
%1038 = OpLoad %v4float %src
%1039 = OpCompositeExtract %float %1038 3
%1040 = OpLoad %v4float %src
%1041 = OpCompositeExtract %float %1040 3
%1042 = OpFSub %float %float_1 %1041
%1043 = OpLoad %v4float %dst
%1044 = OpCompositeExtract %float %1043 3
%1045 = OpFMul %float %1042 %1044
%1046 = OpFAdd %float %1039 %1045
%1047 = OpCompositeConstruct %v4float %1035 %1036 %1037 %1046
OpStore %_0_blend %1047
OpBranch %684
%716 = OpLabel
%1049 = OpLoad %v4float %dst
%1050 = OpCompositeExtract %float %1049 3
%1051 = OpLoad %v4float %src
%1052 = OpCompositeExtract %float %1051 3
%1053 = OpFMul %float %1050 %1052
OpStore %_4_alpha %1053
%1055 = OpLoad %v4float %src
%1056 = OpVectorShuffle %v3float %1055 %1055 0 1 2
%1057 = OpLoad %v4float %dst
%1058 = OpCompositeExtract %float %1057 3
%1059 = OpVectorTimesScalar %v3float %1056 %1058
OpStore %_5_sda %1059
%1061 = OpLoad %v4float %dst
%1062 = OpVectorShuffle %v3float %1061 %1061 0 1 2
%1063 = OpLoad %v4float %src
%1064 = OpCompositeExtract %float %1063 3
%1065 = OpVectorTimesScalar %v3float %1062 %1064
OpStore %_6_dsa %1065
%1066 = OpLoad %v3float %_5_sda
OpStore %1067 %1066
%1068 = OpLoad %v3float %_6_dsa
OpStore %1069 %1068
%1070 = OpFunctionCall %v3float %_blend_set_color_saturation %1067 %1069
OpStore %1071 %1070
%1072 = OpLoad %float %_4_alpha
OpStore %1073 %1072
%1074 = OpLoad %v3float %_6_dsa
OpStore %1075 %1074
%1076 = OpFunctionCall %v3float %_blend_set_color_luminance %1071 %1073 %1075
%1077 = OpLoad %v4float %dst
%1078 = OpVectorShuffle %v3float %1077 %1077 0 1 2
%1079 = OpFAdd %v3float %1076 %1078
%1080 = OpLoad %v3float %_6_dsa
%1081 = OpFSub %v3float %1079 %1080
%1082 = OpLoad %v4float %src
%1083 = OpVectorShuffle %v3float %1082 %1082 0 1 2
%1084 = OpFAdd %v3float %1081 %1083
%1085 = OpLoad %v3float %_5_sda
%1086 = OpFSub %v3float %1084 %1085
%1087 = OpCompositeExtract %float %1086 0
%1088 = OpCompositeExtract %float %1086 1
%1089 = OpCompositeExtract %float %1086 2
%1090 = OpLoad %v4float %src
%1091 = OpCompositeExtract %float %1090 3
%1092 = OpLoad %v4float %dst
%1093 = OpCompositeExtract %float %1092 3
%1094 = OpFAdd %float %1091 %1093
%1095 = OpLoad %float %_4_alpha
%1096 = OpFSub %float %1094 %1095
%1097 = OpCompositeConstruct %v4float %1087 %1088 %1089 %1096
OpStore %_0_blend %1097
OpBranch %684
%717 = OpLabel
%1099 = OpLoad %v4float %dst
%1100 = OpCompositeExtract %float %1099 3
%1101 = OpLoad %v4float %src
%1102 = OpCompositeExtract %float %1101 3
%1103 = OpFMul %float %1100 %1102
OpStore %_7_alpha %1103
%1105 = OpLoad %v4float %src
%1106 = OpVectorShuffle %v3float %1105 %1105 0 1 2
%1107 = OpLoad %v4float %dst
%1108 = OpCompositeExtract %float %1107 3
%1109 = OpVectorTimesScalar %v3float %1106 %1108
OpStore %_8_sda %1109
%1111 = OpLoad %v4float %dst
%1112 = OpVectorShuffle %v3float %1111 %1111 0 1 2
%1113 = OpLoad %v4float %src
%1114 = OpCompositeExtract %float %1113 3
%1115 = OpVectorTimesScalar %v3float %1112 %1114
OpStore %_9_dsa %1115
%1116 = OpLoad %v3float %_9_dsa
OpStore %1117 %1116
%1118 = OpLoad %v3float %_8_sda
OpStore %1119 %1118
%1120 = OpFunctionCall %v3float %_blend_set_color_saturation %1117 %1119
OpStore %1121 %1120
%1122 = OpLoad %float %_7_alpha
OpStore %1123 %1122
%1124 = OpLoad %v3float %_9_dsa
OpStore %1125 %1124
%1126 = OpFunctionCall %v3float %_blend_set_color_luminance %1121 %1123 %1125
%1127 = OpLoad %v4float %dst
%1128 = OpVectorShuffle %v3float %1127 %1127 0 1 2
%1129 = OpFAdd %v3float %1126 %1128
%1130 = OpLoad %v3float %_9_dsa
%1131 = OpFSub %v3float %1129 %1130
%1132 = OpLoad %v4float %src
%1133 = OpVectorShuffle %v3float %1132 %1132 0 1 2
%1134 = OpFAdd %v3float %1131 %1133
%1135 = OpLoad %v3float %_8_sda
%1136 = OpFSub %v3float %1134 %1135
%1137 = OpCompositeExtract %float %1136 0
%1138 = OpCompositeExtract %float %1136 1
%1139 = OpCompositeExtract %float %1136 2
%1140 = OpLoad %v4float %src
%1141 = OpCompositeExtract %float %1140 3
%1142 = OpLoad %v4float %dst
%1143 = OpCompositeExtract %float %1142 3
%1144 = OpFAdd %float %1141 %1143
%1145 = OpLoad %float %_7_alpha
%1146 = OpFSub %float %1144 %1145
%1147 = OpCompositeConstruct %v4float %1137 %1138 %1139 %1146
OpStore %_0_blend %1147
OpBranch %684
%718 = OpLabel
%1149 = OpLoad %v4float %dst
%1150 = OpCompositeExtract %float %1149 3
%1151 = OpLoad %v4float %src
%1152 = OpCompositeExtract %float %1151 3
%1153 = OpFMul %float %1150 %1152
OpStore %_10_alpha %1153
%1155 = OpLoad %v4float %src
%1156 = OpVectorShuffle %v3float %1155 %1155 0 1 2
%1157 = OpLoad %v4float %dst
%1158 = OpCompositeExtract %float %1157 3
%1159 = OpVectorTimesScalar %v3float %1156 %1158
OpStore %_11_sda %1159
%1161 = OpLoad %v4float %dst
%1162 = OpVectorShuffle %v3float %1161 %1161 0 1 2
%1163 = OpLoad %v4float %src
%1164 = OpCompositeExtract %float %1163 3
%1165 = OpVectorTimesScalar %v3float %1162 %1164
OpStore %_12_dsa %1165
%1166 = OpLoad %v3float %_11_sda
OpStore %1167 %1166
%1168 = OpLoad %float %_10_alpha
OpStore %1169 %1168
%1170 = OpLoad %v3float %_12_dsa
OpStore %1171 %1170
%1172 = OpFunctionCall %v3float %_blend_set_color_luminance %1167 %1169 %1171
%1173 = OpLoad %v4float %dst
%1174 = OpVectorShuffle %v3float %1173 %1173 0 1 2
%1175 = OpFAdd %v3float %1172 %1174
%1176 = OpLoad %v3float %_12_dsa
%1177 = OpFSub %v3float %1175 %1176
%1178 = OpLoad %v4float %src
%1179 = OpVectorShuffle %v3float %1178 %1178 0 1 2
%1180 = OpFAdd %v3float %1177 %1179
%1181 = OpLoad %v3float %_11_sda
%1182 = OpFSub %v3float %1180 %1181
%1183 = OpCompositeExtract %float %1182 0
%1184 = OpCompositeExtract %float %1182 1
%1185 = OpCompositeExtract %float %1182 2
%1186 = OpLoad %v4float %src
%1187 = OpCompositeExtract %float %1186 3
%1188 = OpLoad %v4float %dst
%1189 = OpCompositeExtract %float %1188 3
%1190 = OpFAdd %float %1187 %1189
%1191 = OpLoad %float %_10_alpha
%1192 = OpFSub %float %1190 %1191
%1193 = OpCompositeConstruct %v4float %1183 %1184 %1185 %1192
OpStore %_0_blend %1193
OpBranch %684
%719 = OpLabel
%1195 = OpLoad %v4float %dst
%1196 = OpCompositeExtract %float %1195 3
%1197 = OpLoad %v4float %src
%1198 = OpCompositeExtract %float %1197 3
%1199 = OpFMul %float %1196 %1198
OpStore %_13_alpha %1199
%1201 = OpLoad %v4float %src
%1202 = OpVectorShuffle %v3float %1201 %1201 0 1 2
%1203 = OpLoad %v4float %dst
%1204 = OpCompositeExtract %float %1203 3
%1205 = OpVectorTimesScalar %v3float %1202 %1204
OpStore %_14_sda %1205
%1207 = OpLoad %v4float %dst
%1208 = OpVectorShuffle %v3float %1207 %1207 0 1 2
%1209 = OpLoad %v4float %src
%1210 = OpCompositeExtract %float %1209 3
%1211 = OpVectorTimesScalar %v3float %1208 %1210
OpStore %_15_dsa %1211
%1212 = OpLoad %v3float %_15_dsa
OpStore %1213 %1212
%1214 = OpLoad %float %_13_alpha
OpStore %1215 %1214
%1216 = OpLoad %v3float %_14_sda
OpStore %1217 %1216
%1218 = OpFunctionCall %v3float %_blend_set_color_luminance %1213 %1215 %1217
%1219 = OpLoad %v4float %dst
%1220 = OpVectorShuffle %v3float %1219 %1219 0 1 2
%1221 = OpFAdd %v3float %1218 %1220
%1222 = OpLoad %v3float %_15_dsa
%1223 = OpFSub %v3float %1221 %1222
%1224 = OpLoad %v4float %src
%1225 = OpVectorShuffle %v3float %1224 %1224 0 1 2
%1226 = OpFAdd %v3float %1223 %1225
%1227 = OpLoad %v3float %_14_sda
%1228 = OpFSub %v3float %1226 %1227
%1229 = OpCompositeExtract %float %1228 0
%1230 = OpCompositeExtract %float %1228 1
%1231 = OpCompositeExtract %float %1228 2
%1232 = OpLoad %v4float %src
%1233 = OpCompositeExtract %float %1232 3
%1234 = OpLoad %v4float %dst
%1235 = OpCompositeExtract %float %1234 3
%1236 = OpFAdd %float %1233 %1235
%1237 = OpLoad %float %_13_alpha
%1238 = OpFSub %float %1236 %1237
%1239 = OpCompositeConstruct %v4float %1229 %1230 %1231 %1238
OpStore %_0_blend %1239
OpBranch %684
%720 = OpLabel
OpStore %_0_blend %721
OpBranch %684
%690 = OpLabel
OpBranch %684
%684 = OpLabel
%1240 = OpLoad %int %_1_loop
%1241 = OpIAdd %int %1240 %int_1
OpStore %_1_loop %1241
OpBranch %681
%685 = OpLabel
%1242 = OpLoad %v4float %_0_blend
OpStore %sk_FragColor %1242
OpReturn
OpFunctionEnd
