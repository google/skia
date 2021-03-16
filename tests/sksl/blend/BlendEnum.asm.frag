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
OpName %main "main"
OpName %_0_blend "_0_blend"
OpName %_1_result "_1_result"
OpName %_2_result "_2_result"
OpName %_3_alpha "_3_alpha"
OpName %_4_sda "_4_sda"
OpName %_5_dsa "_5_dsa"
OpName %_6_alpha "_6_alpha"
OpName %_7_sda "_7_sda"
OpName %_8_dsa "_8_dsa"
OpName %_9_alpha "_9_alpha"
OpName %_10_sda "_10_sda"
OpName %_11_dsa "_11_dsa"
OpName %_12_alpha "_12_alpha"
OpName %_13_sda "_13_sda"
OpName %_14_dsa "_14_dsa"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
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
OpDecorate %681 RelaxedPrecision
OpDecorate %684 RelaxedPrecision
OpDecorate %685 RelaxedPrecision
OpDecorate %687 RelaxedPrecision
OpDecorate %689 RelaxedPrecision
OpDecorate %693 RelaxedPrecision
OpDecorate %694 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %700 RelaxedPrecision
OpDecorate %705 RelaxedPrecision
OpDecorate %707 RelaxedPrecision
OpDecorate %709 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %713 RelaxedPrecision
OpDecorate %715 RelaxedPrecision
OpDecorate %718 RelaxedPrecision
OpDecorate %720 RelaxedPrecision
OpDecorate %722 RelaxedPrecision
OpDecorate %726 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %731 RelaxedPrecision
OpDecorate %734 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %740 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %747 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %751 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %757 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
OpDecorate %792 RelaxedPrecision
OpDecorate %794 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %798 RelaxedPrecision
OpDecorate %799 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %815 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %829 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %838 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %849 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %872 RelaxedPrecision
OpDecorate %877 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %886 RelaxedPrecision
OpDecorate %889 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %893 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %898 RelaxedPrecision
OpDecorate %900 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %908 RelaxedPrecision
OpDecorate %911 RelaxedPrecision
OpDecorate %915 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %922 RelaxedPrecision
OpDecorate %927 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %932 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
OpDecorate %937 RelaxedPrecision
OpDecorate %940 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %947 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %953 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %959 RelaxedPrecision
OpDecorate %962 RelaxedPrecision
OpDecorate %964 RelaxedPrecision
OpDecorate %966 RelaxedPrecision
OpDecorate %968 RelaxedPrecision
OpDecorate %969 RelaxedPrecision
OpDecorate %972 RelaxedPrecision
OpDecorate %974 RelaxedPrecision
OpDecorate %976 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %984 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %989 RelaxedPrecision
OpDecorate %992 RelaxedPrecision
OpDecorate %994 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1007 RelaxedPrecision
OpDecorate %1009 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1017 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1022 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1048 RelaxedPrecision
OpDecorate %1050 RelaxedPrecision
OpDecorate %1051 RelaxedPrecision
OpDecorate %1052 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1057 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1069 RelaxedPrecision
OpDecorate %1070 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1093 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1098 RelaxedPrecision
OpDecorate %1102 RelaxedPrecision
OpDecorate %1104 RelaxedPrecision
OpDecorate %1108 RelaxedPrecision
OpDecorate %1110 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1112 RelaxedPrecision
OpDecorate %1114 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1117 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1139 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1145 RelaxedPrecision
OpDecorate %1150 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1156 RelaxedPrecision
OpDecorate %1158 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1164 RelaxedPrecision
OpDecorate %1166 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1168 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1179 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1193 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1198 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1212 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1216 RelaxedPrecision
OpDecorate %1220 RelaxedPrecision
OpDecorate %1222 RelaxedPrecision
OpDecorate %1223 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1229 RelaxedPrecision
OpDecorate %1230 RelaxedPrecision
OpDecorate %1235 RelaxedPrecision
OpDecorate %1238 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1241 RelaxedPrecision
OpDecorate %1242 RelaxedPrecision
OpDecorate %1245 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%1244 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
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
%698 = OpVariable %_ptr_Function_v4float Function
%701 = OpVariable %_ptr_Function_v4float Function
%_1_result = OpVariable %_ptr_Function_v4float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%764 = OpVariable %_ptr_Function_v2float Function
%768 = OpVariable %_ptr_Function_v2float Function
%773 = OpVariable %_ptr_Function_v2float Function
%777 = OpVariable %_ptr_Function_v2float Function
%782 = OpVariable %_ptr_Function_v2float Function
%786 = OpVariable %_ptr_Function_v2float Function
%804 = OpVariable %_ptr_Function_v2float Function
%808 = OpVariable %_ptr_Function_v2float Function
%813 = OpVariable %_ptr_Function_v2float Function
%817 = OpVariable %_ptr_Function_v2float Function
%822 = OpVariable %_ptr_Function_v2float Function
%826 = OpVariable %_ptr_Function_v2float Function
%843 = OpVariable %_ptr_Function_v4float Function
%846 = OpVariable %_ptr_Function_v4float Function
%852 = OpVariable %_ptr_Function_v4float Function
%861 = OpVariable %_ptr_Function_v2float Function
%865 = OpVariable %_ptr_Function_v2float Function
%870 = OpVariable %_ptr_Function_v2float Function
%874 = OpVariable %_ptr_Function_v2float Function
%879 = OpVariable %_ptr_Function_v2float Function
%883 = OpVariable %_ptr_Function_v2float Function
%_3_alpha = OpVariable %_ptr_Function_float Function
%_4_sda = OpVariable %_ptr_Function_v3float Function
%_5_dsa = OpVariable %_ptr_Function_v3float Function
%1037 = OpVariable %_ptr_Function_v3float Function
%1039 = OpVariable %_ptr_Function_v3float Function
%1041 = OpVariable %_ptr_Function_v3float Function
%1043 = OpVariable %_ptr_Function_float Function
%1045 = OpVariable %_ptr_Function_v3float Function
%_6_alpha = OpVariable %_ptr_Function_float Function
%_7_sda = OpVariable %_ptr_Function_v3float Function
%_8_dsa = OpVariable %_ptr_Function_v3float Function
%1097 = OpVariable %_ptr_Function_v3float Function
%1099 = OpVariable %_ptr_Function_v3float Function
%1101 = OpVariable %_ptr_Function_v3float Function
%1103 = OpVariable %_ptr_Function_float Function
%1105 = OpVariable %_ptr_Function_v3float Function
%_9_alpha = OpVariable %_ptr_Function_float Function
%_10_sda = OpVariable %_ptr_Function_v3float Function
%_11_dsa = OpVariable %_ptr_Function_v3float Function
%1157 = OpVariable %_ptr_Function_v3float Function
%1159 = OpVariable %_ptr_Function_float Function
%1161 = OpVariable %_ptr_Function_v3float Function
%_12_alpha = OpVariable %_ptr_Function_float Function
%_13_sda = OpVariable %_ptr_Function_v3float Function
%_14_dsa = OpVariable %_ptr_Function_v3float Function
%1213 = OpVariable %_ptr_Function_v3float Function
%1215 = OpVariable %_ptr_Function_float Function
%1217 = OpVariable %_ptr_Function_v3float Function
%677 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%681 = OpLoad %v4float %677
%682 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%684 = OpLoad %v4float %682
%685 = OpFMul %v4float %681 %684
OpStore %_0_blend %685
%686 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%687 = OpLoad %v4float %686
%688 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%689 = OpLoad %v4float %688
%690 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%691 = OpFSub %v4float %690 %689
%692 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%693 = OpLoad %v4float %692
%694 = OpFMul %v4float %691 %693
%695 = OpFAdd %v4float %687 %694
OpStore %_0_blend %695
%696 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%697 = OpLoad %v4float %696
OpStore %698 %697
%699 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%700 = OpLoad %v4float %699
OpStore %701 %700
%702 = OpFunctionCall %v4float %blend_overlay %698 %701
OpStore %_0_blend %702
%704 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%705 = OpLoad %v4float %704
%706 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%707 = OpLoad %v4float %706
%708 = OpCompositeExtract %float %707 3
%709 = OpFSub %float %float_1 %708
%710 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%711 = OpLoad %v4float %710
%712 = OpVectorTimesScalar %v4float %711 %709
%713 = OpFAdd %v4float %705 %712
OpStore %_1_result %713
%715 = OpLoad %v4float %_1_result
%716 = OpVectorShuffle %v3float %715 %715 0 1 2
%717 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%718 = OpLoad %v4float %717
%719 = OpCompositeExtract %float %718 3
%720 = OpFSub %float %float_1 %719
%721 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%722 = OpLoad %v4float %721
%723 = OpVectorShuffle %v3float %722 %722 0 1 2
%724 = OpVectorTimesScalar %v3float %723 %720
%725 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%726 = OpLoad %v4float %725
%727 = OpVectorShuffle %v3float %726 %726 0 1 2
%728 = OpFAdd %v3float %724 %727
%714 = OpExtInst %v3float %1 FMin %716 %728
%729 = OpLoad %v4float %_1_result
%730 = OpVectorShuffle %v4float %729 %714 4 5 6 3
OpStore %_1_result %730
%731 = OpLoad %v4float %_1_result
OpStore %_0_blend %731
%733 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%734 = OpLoad %v4float %733
%735 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%736 = OpLoad %v4float %735
%737 = OpCompositeExtract %float %736 3
%738 = OpFSub %float %float_1 %737
%739 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%740 = OpLoad %v4float %739
%741 = OpVectorTimesScalar %v4float %740 %738
%742 = OpFAdd %v4float %734 %741
OpStore %_2_result %742
%744 = OpLoad %v4float %_2_result
%745 = OpVectorShuffle %v3float %744 %744 0 1 2
%746 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%747 = OpLoad %v4float %746
%748 = OpCompositeExtract %float %747 3
%749 = OpFSub %float %float_1 %748
%750 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%751 = OpLoad %v4float %750
%752 = OpVectorShuffle %v3float %751 %751 0 1 2
%753 = OpVectorTimesScalar %v3float %752 %749
%754 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%755 = OpLoad %v4float %754
%756 = OpVectorShuffle %v3float %755 %755 0 1 2
%757 = OpFAdd %v3float %753 %756
%743 = OpExtInst %v3float %1 FMax %745 %757
%758 = OpLoad %v4float %_2_result
%759 = OpVectorShuffle %v4float %758 %743 4 5 6 3
OpStore %_2_result %759
%760 = OpLoad %v4float %_2_result
OpStore %_0_blend %760
%761 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%762 = OpLoad %v4float %761
%763 = OpVectorShuffle %v2float %762 %762 0 3
OpStore %764 %763
%765 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%766 = OpLoad %v4float %765
%767 = OpVectorShuffle %v2float %766 %766 0 3
OpStore %768 %767
%769 = OpFunctionCall %float %_color_dodge_component %764 %768
%770 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%771 = OpLoad %v4float %770
%772 = OpVectorShuffle %v2float %771 %771 1 3
OpStore %773 %772
%774 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%775 = OpLoad %v4float %774
%776 = OpVectorShuffle %v2float %775 %775 1 3
OpStore %777 %776
%778 = OpFunctionCall %float %_color_dodge_component %773 %777
%779 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%780 = OpLoad %v4float %779
%781 = OpVectorShuffle %v2float %780 %780 2 3
OpStore %782 %781
%783 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%784 = OpLoad %v4float %783
%785 = OpVectorShuffle %v2float %784 %784 2 3
OpStore %786 %785
%787 = OpFunctionCall %float %_color_dodge_component %782 %786
%788 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%789 = OpLoad %v4float %788
%790 = OpCompositeExtract %float %789 3
%791 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%792 = OpLoad %v4float %791
%793 = OpCompositeExtract %float %792 3
%794 = OpFSub %float %float_1 %793
%795 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%796 = OpLoad %v4float %795
%797 = OpCompositeExtract %float %796 3
%798 = OpFMul %float %794 %797
%799 = OpFAdd %float %790 %798
%800 = OpCompositeConstruct %v4float %769 %778 %787 %799
OpStore %_0_blend %800
%801 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%802 = OpLoad %v4float %801
%803 = OpVectorShuffle %v2float %802 %802 0 3
OpStore %804 %803
%805 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%806 = OpLoad %v4float %805
%807 = OpVectorShuffle %v2float %806 %806 0 3
OpStore %808 %807
%809 = OpFunctionCall %float %_color_burn_component %804 %808
%810 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%811 = OpLoad %v4float %810
%812 = OpVectorShuffle %v2float %811 %811 1 3
OpStore %813 %812
%814 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%815 = OpLoad %v4float %814
%816 = OpVectorShuffle %v2float %815 %815 1 3
OpStore %817 %816
%818 = OpFunctionCall %float %_color_burn_component %813 %817
%819 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%820 = OpLoad %v4float %819
%821 = OpVectorShuffle %v2float %820 %820 2 3
OpStore %822 %821
%823 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%824 = OpLoad %v4float %823
%825 = OpVectorShuffle %v2float %824 %824 2 3
OpStore %826 %825
%827 = OpFunctionCall %float %_color_burn_component %822 %826
%828 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%829 = OpLoad %v4float %828
%830 = OpCompositeExtract %float %829 3
%831 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%832 = OpLoad %v4float %831
%833 = OpCompositeExtract %float %832 3
%834 = OpFSub %float %float_1 %833
%835 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%836 = OpLoad %v4float %835
%837 = OpCompositeExtract %float %836 3
%838 = OpFMul %float %834 %837
%839 = OpFAdd %float %830 %838
%840 = OpCompositeConstruct %v4float %809 %818 %827 %839
OpStore %_0_blend %840
%841 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%842 = OpLoad %v4float %841
OpStore %843 %842
%844 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%845 = OpLoad %v4float %844
OpStore %846 %845
%847 = OpFunctionCall %v4float %blend_overlay %843 %846
OpStore %_0_blend %847
%848 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%849 = OpLoad %v4float %848
%850 = OpCompositeExtract %float %849 3
%851 = OpFOrdEqual %bool %850 %float_0
OpSelectionMerge %855 None
OpBranchConditional %851 %853 %854
%853 = OpLabel
%856 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%857 = OpLoad %v4float %856
OpStore %852 %857
OpBranch %855
%854 = OpLabel
%858 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%859 = OpLoad %v4float %858
%860 = OpVectorShuffle %v2float %859 %859 0 3
OpStore %861 %860
%862 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%863 = OpLoad %v4float %862
%864 = OpVectorShuffle %v2float %863 %863 0 3
OpStore %865 %864
%866 = OpFunctionCall %float %_soft_light_component %861 %865
%867 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%868 = OpLoad %v4float %867
%869 = OpVectorShuffle %v2float %868 %868 1 3
OpStore %870 %869
%871 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%872 = OpLoad %v4float %871
%873 = OpVectorShuffle %v2float %872 %872 1 3
OpStore %874 %873
%875 = OpFunctionCall %float %_soft_light_component %870 %874
%876 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%877 = OpLoad %v4float %876
%878 = OpVectorShuffle %v2float %877 %877 2 3
OpStore %879 %878
%880 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%881 = OpLoad %v4float %880
%882 = OpVectorShuffle %v2float %881 %881 2 3
OpStore %883 %882
%884 = OpFunctionCall %float %_soft_light_component %879 %883
%885 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%886 = OpLoad %v4float %885
%887 = OpCompositeExtract %float %886 3
%888 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%889 = OpLoad %v4float %888
%890 = OpCompositeExtract %float %889 3
%891 = OpFSub %float %float_1 %890
%892 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%893 = OpLoad %v4float %892
%894 = OpCompositeExtract %float %893 3
%895 = OpFMul %float %891 %894
%896 = OpFAdd %float %887 %895
%897 = OpCompositeConstruct %v4float %866 %875 %884 %896
OpStore %852 %897
OpBranch %855
%855 = OpLabel
%898 = OpLoad %v4float %852
OpStore %_0_blend %898
%899 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%900 = OpLoad %v4float %899
%901 = OpVectorShuffle %v3float %900 %900 0 1 2
%902 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%903 = OpLoad %v4float %902
%904 = OpVectorShuffle %v3float %903 %903 0 1 2
%905 = OpFAdd %v3float %901 %904
%907 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%908 = OpLoad %v4float %907
%909 = OpVectorShuffle %v3float %908 %908 0 1 2
%910 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%911 = OpLoad %v4float %910
%912 = OpCompositeExtract %float %911 3
%913 = OpVectorTimesScalar %v3float %909 %912
%914 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%915 = OpLoad %v4float %914
%916 = OpVectorShuffle %v3float %915 %915 0 1 2
%917 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%918 = OpLoad %v4float %917
%919 = OpCompositeExtract %float %918 3
%920 = OpVectorTimesScalar %v3float %916 %919
%906 = OpExtInst %v3float %1 FMin %913 %920
%921 = OpVectorTimesScalar %v3float %906 %float_2
%922 = OpFSub %v3float %905 %921
%923 = OpCompositeExtract %float %922 0
%924 = OpCompositeExtract %float %922 1
%925 = OpCompositeExtract %float %922 2
%926 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%927 = OpLoad %v4float %926
%928 = OpCompositeExtract %float %927 3
%929 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%930 = OpLoad %v4float %929
%931 = OpCompositeExtract %float %930 3
%932 = OpFSub %float %float_1 %931
%933 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%934 = OpLoad %v4float %933
%935 = OpCompositeExtract %float %934 3
%936 = OpFMul %float %932 %935
%937 = OpFAdd %float %928 %936
%938 = OpCompositeConstruct %v4float %923 %924 %925 %937
OpStore %_0_blend %938
%939 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%940 = OpLoad %v4float %939
%941 = OpVectorShuffle %v3float %940 %940 0 1 2
%942 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%943 = OpLoad %v4float %942
%944 = OpVectorShuffle %v3float %943 %943 0 1 2
%945 = OpFAdd %v3float %941 %944
%946 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%947 = OpLoad %v4float %946
%948 = OpVectorShuffle %v3float %947 %947 0 1 2
%949 = OpVectorTimesScalar %v3float %948 %float_2
%950 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%951 = OpLoad %v4float %950
%952 = OpVectorShuffle %v3float %951 %951 0 1 2
%953 = OpFMul %v3float %949 %952
%954 = OpFSub %v3float %945 %953
%955 = OpCompositeExtract %float %954 0
%956 = OpCompositeExtract %float %954 1
%957 = OpCompositeExtract %float %954 2
%958 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%959 = OpLoad %v4float %958
%960 = OpCompositeExtract %float %959 3
%961 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%962 = OpLoad %v4float %961
%963 = OpCompositeExtract %float %962 3
%964 = OpFSub %float %float_1 %963
%965 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%966 = OpLoad %v4float %965
%967 = OpCompositeExtract %float %966 3
%968 = OpFMul %float %964 %967
%969 = OpFAdd %float %960 %968
%970 = OpCompositeConstruct %v4float %955 %956 %957 %969
OpStore %_0_blend %970
%971 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%972 = OpLoad %v4float %971
%973 = OpCompositeExtract %float %972 3
%974 = OpFSub %float %float_1 %973
%975 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%976 = OpLoad %v4float %975
%977 = OpVectorShuffle %v3float %976 %976 0 1 2
%978 = OpVectorTimesScalar %v3float %977 %974
%979 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%980 = OpLoad %v4float %979
%981 = OpCompositeExtract %float %980 3
%982 = OpFSub %float %float_1 %981
%983 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%984 = OpLoad %v4float %983
%985 = OpVectorShuffle %v3float %984 %984 0 1 2
%986 = OpVectorTimesScalar %v3float %985 %982
%987 = OpFAdd %v3float %978 %986
%988 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%989 = OpLoad %v4float %988
%990 = OpVectorShuffle %v3float %989 %989 0 1 2
%991 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%992 = OpLoad %v4float %991
%993 = OpVectorShuffle %v3float %992 %992 0 1 2
%994 = OpFMul %v3float %990 %993
%995 = OpFAdd %v3float %987 %994
%996 = OpCompositeExtract %float %995 0
%997 = OpCompositeExtract %float %995 1
%998 = OpCompositeExtract %float %995 2
%999 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1000 = OpLoad %v4float %999
%1001 = OpCompositeExtract %float %1000 3
%1002 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1003 = OpLoad %v4float %1002
%1004 = OpCompositeExtract %float %1003 3
%1005 = OpFSub %float %float_1 %1004
%1006 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1007 = OpLoad %v4float %1006
%1008 = OpCompositeExtract %float %1007 3
%1009 = OpFMul %float %1005 %1008
%1010 = OpFAdd %float %1001 %1009
%1011 = OpCompositeConstruct %v4float %996 %997 %998 %1010
OpStore %_0_blend %1011
%1013 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1014 = OpLoad %v4float %1013
%1015 = OpCompositeExtract %float %1014 3
%1016 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1017 = OpLoad %v4float %1016
%1018 = OpCompositeExtract %float %1017 3
%1019 = OpFMul %float %1015 %1018
OpStore %_3_alpha %1019
%1021 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1022 = OpLoad %v4float %1021
%1023 = OpVectorShuffle %v3float %1022 %1022 0 1 2
%1024 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1025 = OpLoad %v4float %1024
%1026 = OpCompositeExtract %float %1025 3
%1027 = OpVectorTimesScalar %v3float %1023 %1026
OpStore %_4_sda %1027
%1029 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1030 = OpLoad %v4float %1029
%1031 = OpVectorShuffle %v3float %1030 %1030 0 1 2
%1032 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1033 = OpLoad %v4float %1032
%1034 = OpCompositeExtract %float %1033 3
%1035 = OpVectorTimesScalar %v3float %1031 %1034
OpStore %_5_dsa %1035
%1036 = OpLoad %v3float %_4_sda
OpStore %1037 %1036
%1038 = OpLoad %v3float %_5_dsa
OpStore %1039 %1038
%1040 = OpFunctionCall %v3float %_blend_set_color_saturation %1037 %1039
OpStore %1041 %1040
%1042 = OpLoad %float %_3_alpha
OpStore %1043 %1042
%1044 = OpLoad %v3float %_5_dsa
OpStore %1045 %1044
%1046 = OpFunctionCall %v3float %_blend_set_color_luminance %1041 %1043 %1045
%1047 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1048 = OpLoad %v4float %1047
%1049 = OpVectorShuffle %v3float %1048 %1048 0 1 2
%1050 = OpFAdd %v3float %1046 %1049
%1051 = OpLoad %v3float %_5_dsa
%1052 = OpFSub %v3float %1050 %1051
%1053 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1054 = OpLoad %v4float %1053
%1055 = OpVectorShuffle %v3float %1054 %1054 0 1 2
%1056 = OpFAdd %v3float %1052 %1055
%1057 = OpLoad %v3float %_4_sda
%1058 = OpFSub %v3float %1056 %1057
%1059 = OpCompositeExtract %float %1058 0
%1060 = OpCompositeExtract %float %1058 1
%1061 = OpCompositeExtract %float %1058 2
%1062 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1063 = OpLoad %v4float %1062
%1064 = OpCompositeExtract %float %1063 3
%1065 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1066 = OpLoad %v4float %1065
%1067 = OpCompositeExtract %float %1066 3
%1068 = OpFAdd %float %1064 %1067
%1069 = OpLoad %float %_3_alpha
%1070 = OpFSub %float %1068 %1069
%1071 = OpCompositeConstruct %v4float %1059 %1060 %1061 %1070
OpStore %_0_blend %1071
%1073 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1074 = OpLoad %v4float %1073
%1075 = OpCompositeExtract %float %1074 3
%1076 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1077 = OpLoad %v4float %1076
%1078 = OpCompositeExtract %float %1077 3
%1079 = OpFMul %float %1075 %1078
OpStore %_6_alpha %1079
%1081 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1082 = OpLoad %v4float %1081
%1083 = OpVectorShuffle %v3float %1082 %1082 0 1 2
%1084 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1085 = OpLoad %v4float %1084
%1086 = OpCompositeExtract %float %1085 3
%1087 = OpVectorTimesScalar %v3float %1083 %1086
OpStore %_7_sda %1087
%1089 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1090 = OpLoad %v4float %1089
%1091 = OpVectorShuffle %v3float %1090 %1090 0 1 2
%1092 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1093 = OpLoad %v4float %1092
%1094 = OpCompositeExtract %float %1093 3
%1095 = OpVectorTimesScalar %v3float %1091 %1094
OpStore %_8_dsa %1095
%1096 = OpLoad %v3float %_8_dsa
OpStore %1097 %1096
%1098 = OpLoad %v3float %_7_sda
OpStore %1099 %1098
%1100 = OpFunctionCall %v3float %_blend_set_color_saturation %1097 %1099
OpStore %1101 %1100
%1102 = OpLoad %float %_6_alpha
OpStore %1103 %1102
%1104 = OpLoad %v3float %_8_dsa
OpStore %1105 %1104
%1106 = OpFunctionCall %v3float %_blend_set_color_luminance %1101 %1103 %1105
%1107 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1108 = OpLoad %v4float %1107
%1109 = OpVectorShuffle %v3float %1108 %1108 0 1 2
%1110 = OpFAdd %v3float %1106 %1109
%1111 = OpLoad %v3float %_8_dsa
%1112 = OpFSub %v3float %1110 %1111
%1113 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1114 = OpLoad %v4float %1113
%1115 = OpVectorShuffle %v3float %1114 %1114 0 1 2
%1116 = OpFAdd %v3float %1112 %1115
%1117 = OpLoad %v3float %_7_sda
%1118 = OpFSub %v3float %1116 %1117
%1119 = OpCompositeExtract %float %1118 0
%1120 = OpCompositeExtract %float %1118 1
%1121 = OpCompositeExtract %float %1118 2
%1122 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1123 = OpLoad %v4float %1122
%1124 = OpCompositeExtract %float %1123 3
%1125 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1126 = OpLoad %v4float %1125
%1127 = OpCompositeExtract %float %1126 3
%1128 = OpFAdd %float %1124 %1127
%1129 = OpLoad %float %_6_alpha
%1130 = OpFSub %float %1128 %1129
%1131 = OpCompositeConstruct %v4float %1119 %1120 %1121 %1130
OpStore %_0_blend %1131
%1133 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1134 = OpLoad %v4float %1133
%1135 = OpCompositeExtract %float %1134 3
%1136 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1137 = OpLoad %v4float %1136
%1138 = OpCompositeExtract %float %1137 3
%1139 = OpFMul %float %1135 %1138
OpStore %_9_alpha %1139
%1141 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1142 = OpLoad %v4float %1141
%1143 = OpVectorShuffle %v3float %1142 %1142 0 1 2
%1144 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1145 = OpLoad %v4float %1144
%1146 = OpCompositeExtract %float %1145 3
%1147 = OpVectorTimesScalar %v3float %1143 %1146
OpStore %_10_sda %1147
%1149 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1150 = OpLoad %v4float %1149
%1151 = OpVectorShuffle %v3float %1150 %1150 0 1 2
%1152 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1153 = OpLoad %v4float %1152
%1154 = OpCompositeExtract %float %1153 3
%1155 = OpVectorTimesScalar %v3float %1151 %1154
OpStore %_11_dsa %1155
%1156 = OpLoad %v3float %_10_sda
OpStore %1157 %1156
%1158 = OpLoad %float %_9_alpha
OpStore %1159 %1158
%1160 = OpLoad %v3float %_11_dsa
OpStore %1161 %1160
%1162 = OpFunctionCall %v3float %_blend_set_color_luminance %1157 %1159 %1161
%1163 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1164 = OpLoad %v4float %1163
%1165 = OpVectorShuffle %v3float %1164 %1164 0 1 2
%1166 = OpFAdd %v3float %1162 %1165
%1167 = OpLoad %v3float %_11_dsa
%1168 = OpFSub %v3float %1166 %1167
%1169 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1170 = OpLoad %v4float %1169
%1171 = OpVectorShuffle %v3float %1170 %1170 0 1 2
%1172 = OpFAdd %v3float %1168 %1171
%1173 = OpLoad %v3float %_10_sda
%1174 = OpFSub %v3float %1172 %1173
%1175 = OpCompositeExtract %float %1174 0
%1176 = OpCompositeExtract %float %1174 1
%1177 = OpCompositeExtract %float %1174 2
%1178 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1179 = OpLoad %v4float %1178
%1180 = OpCompositeExtract %float %1179 3
%1181 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1182 = OpLoad %v4float %1181
%1183 = OpCompositeExtract %float %1182 3
%1184 = OpFAdd %float %1180 %1183
%1185 = OpLoad %float %_9_alpha
%1186 = OpFSub %float %1184 %1185
%1187 = OpCompositeConstruct %v4float %1175 %1176 %1177 %1186
OpStore %_0_blend %1187
%1189 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1190 = OpLoad %v4float %1189
%1191 = OpCompositeExtract %float %1190 3
%1192 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1193 = OpLoad %v4float %1192
%1194 = OpCompositeExtract %float %1193 3
%1195 = OpFMul %float %1191 %1194
OpStore %_12_alpha %1195
%1197 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1198 = OpLoad %v4float %1197
%1199 = OpVectorShuffle %v3float %1198 %1198 0 1 2
%1200 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1201 = OpLoad %v4float %1200
%1202 = OpCompositeExtract %float %1201 3
%1203 = OpVectorTimesScalar %v3float %1199 %1202
OpStore %_13_sda %1203
%1205 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1206 = OpLoad %v4float %1205
%1207 = OpVectorShuffle %v3float %1206 %1206 0 1 2
%1208 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1209 = OpLoad %v4float %1208
%1210 = OpCompositeExtract %float %1209 3
%1211 = OpVectorTimesScalar %v3float %1207 %1210
OpStore %_14_dsa %1211
%1212 = OpLoad %v3float %_14_dsa
OpStore %1213 %1212
%1214 = OpLoad %float %_12_alpha
OpStore %1215 %1214
%1216 = OpLoad %v3float %_13_sda
OpStore %1217 %1216
%1218 = OpFunctionCall %v3float %_blend_set_color_luminance %1213 %1215 %1217
%1219 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1220 = OpLoad %v4float %1219
%1221 = OpVectorShuffle %v3float %1220 %1220 0 1 2
%1222 = OpFAdd %v3float %1218 %1221
%1223 = OpLoad %v3float %_14_dsa
%1224 = OpFSub %v3float %1222 %1223
%1225 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1226 = OpLoad %v4float %1225
%1227 = OpVectorShuffle %v3float %1226 %1226 0 1 2
%1228 = OpFAdd %v3float %1224 %1227
%1229 = OpLoad %v3float %_13_sda
%1230 = OpFSub %v3float %1228 %1229
%1231 = OpCompositeExtract %float %1230 0
%1232 = OpCompositeExtract %float %1230 1
%1233 = OpCompositeExtract %float %1230 2
%1234 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%1235 = OpLoad %v4float %1234
%1236 = OpCompositeExtract %float %1235 3
%1237 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%1238 = OpLoad %v4float %1237
%1239 = OpCompositeExtract %float %1238 3
%1240 = OpFAdd %float %1236 %1239
%1241 = OpLoad %float %_12_alpha
%1242 = OpFSub %float %1240 %1241
%1243 = OpCompositeConstruct %v4float %1231 %1232 %1233 %1242
OpStore %_0_blend %1243
OpStore %_0_blend %1244
%1245 = OpLoad %v4float %_0_blend
OpStore %sk_FragColor %1245
OpReturn
OpFunctionEnd
