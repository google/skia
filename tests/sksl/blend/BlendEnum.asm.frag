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
OpName %_guarded_divide "_guarded_divide"
OpName %_color_dodge_component "_color_dodge_component"
OpName %delta "delta"
OpName %_color_burn_component "_color_burn_component"
OpName %delta_0 "delta"
OpName %_soft_light_component "_soft_light_component"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_guarded_divide_0 "_guarded_divide"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result_2 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %534 RelaxedPrecision
OpDecorate %535 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %554 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %583 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %596 RelaxedPrecision
OpDecorate %599 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %615 RelaxedPrecision
OpDecorate %617 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
OpDecorate %626 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %629 RelaxedPrecision
OpDecorate %631 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %638 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %650 RelaxedPrecision
OpDecorate %652 RelaxedPrecision
OpDecorate %656 RelaxedPrecision
OpDecorate %658 RelaxedPrecision
OpDecorate %660 RelaxedPrecision
OpDecorate %662 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %665 RelaxedPrecision
OpDecorate %671 RelaxedPrecision
OpDecorate %673 RelaxedPrecision
OpDecorate %679 RelaxedPrecision
OpDecorate %681 RelaxedPrecision
OpDecorate %684 RelaxedPrecision
OpDecorate %686 RelaxedPrecision
OpDecorate %692 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %699 RelaxedPrecision
OpDecorate %702 RelaxedPrecision
OpDecorate %706 RelaxedPrecision
OpDecorate %708 RelaxedPrecision
OpDecorate %714 RelaxedPrecision
OpDecorate %717 RelaxedPrecision
OpDecorate %721 RelaxedPrecision
OpDecorate %723 RelaxedPrecision
OpDecorate %729 RelaxedPrecision
OpDecorate %732 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %739 RelaxedPrecision
OpDecorate %747 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %751 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %761 RelaxedPrecision
OpDecorate %764 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %770 RelaxedPrecision
OpDecorate %772 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %777 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %790 RelaxedPrecision
OpDecorate %792 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %794 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %804 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %817 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %823 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %828 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %833 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
OpDecorate %847 RelaxedPrecision
OpDecorate %853 RelaxedPrecision
OpDecorate %855 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %861 RelaxedPrecision
OpDecorate %865 RelaxedPrecision
OpDecorate %867 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %872 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %877 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %880 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %882 RelaxedPrecision
OpDecorate %884 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %886 RelaxedPrecision
OpDecorate %890 RelaxedPrecision
OpDecorate %892 RelaxedPrecision
OpDecorate %894 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %902 RelaxedPrecision
OpDecorate %904 RelaxedPrecision
OpDecorate %906 RelaxedPrecision
OpDecorate %908 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %914 RelaxedPrecision
OpDecorate %916 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %921 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %926 RelaxedPrecision
OpDecorate %928 RelaxedPrecision
OpDecorate %929 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %931 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %935 RelaxedPrecision
OpDecorate %939 RelaxedPrecision
OpDecorate %941 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %944 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %989 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %992 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %996 RelaxedPrecision
OpDecorate %998 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1001 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1004 RelaxedPrecision
OpDecorate %1007 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1013 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %1023 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %1028 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1041 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1048 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
OpDecorate %1051 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1057 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1059 RelaxedPrecision
OpDecorate %1060 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1064 RelaxedPrecision
OpDecorate %1065 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1067 RelaxedPrecision
OpDecorate %1069 RelaxedPrecision
OpDecorate %1072 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1089 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1107 RelaxedPrecision
OpDecorate %1108 RelaxedPrecision
OpDecorate %1110 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1113 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1120 RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1139 RelaxedPrecision
OpDecorate %1141 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1156 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1164 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %1171 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1189 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
OpDecorate %1193 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1197 RelaxedPrecision
OpDecorate %1200 RelaxedPrecision
OpDecorate %1202 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1210 RelaxedPrecision
OpDecorate %1212 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1217 RelaxedPrecision
OpDecorate %1218 RelaxedPrecision
OpDecorate %1220 RelaxedPrecision
OpDecorate %1222 RelaxedPrecision
OpDecorate %1224 RelaxedPrecision
OpDecorate %1225 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1230 RelaxedPrecision
OpDecorate %1231 RelaxedPrecision
OpDecorate %1235 RelaxedPrecision
OpDecorate %1237 RelaxedPrecision
OpDecorate %1239 RelaxedPrecision
OpDecorate %1240 RelaxedPrecision
OpDecorate %1242 RelaxedPrecision
OpDecorate %1243 RelaxedPrecision
OpDecorate %1245 RelaxedPrecision
OpDecorate %1247 RelaxedPrecision
OpDecorate %1248 RelaxedPrecision
OpDecorate %1251 RelaxedPrecision
OpDecorate %1253 RelaxedPrecision
OpDecorate %1254 RelaxedPrecision
OpDecorate %1257 RelaxedPrecision
OpDecorate %1258 RelaxedPrecision
OpDecorate %1260 RelaxedPrecision
OpDecorate %1262 RelaxedPrecision
OpDecorate %1263 RelaxedPrecision
OpDecorate %1267 RelaxedPrecision
OpDecorate %1269 RelaxedPrecision
OpDecorate %1271 RelaxedPrecision
OpDecorate %1272 RelaxedPrecision
OpDecorate %1274 RelaxedPrecision
OpDecorate %1275 RelaxedPrecision
OpDecorate %1277 RelaxedPrecision
OpDecorate %1279 RelaxedPrecision
OpDecorate %1282 RelaxedPrecision
OpDecorate %1284 RelaxedPrecision
OpDecorate %1287 RelaxedPrecision
OpDecorate %1289 RelaxedPrecision
OpDecorate %1292 RelaxedPrecision
OpDecorate %1294 RelaxedPrecision
OpDecorate %1302 RelaxedPrecision
OpDecorate %1304 RelaxedPrecision
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
%31 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%73 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%183 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%float_0 = OpConstant %float 0
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v3float = OpTypePointer Function %v3float
%509 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%518 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%528 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%640 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%641 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%948 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%986 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1298 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%_blend_overlay_component = OpFunction %float None %31
%33 = OpFunctionParameter %_ptr_Function_v2float
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpLabel
%43 = OpVariable %_ptr_Function_float Function
%37 = OpLoad %v2float %34
%38 = OpCompositeExtract %float %37 0
%39 = OpFMul %float %float_2 %38
%40 = OpLoad %v2float %34
%41 = OpCompositeExtract %float %40 1
%42 = OpFOrdLessThanEqual %bool %39 %41
OpSelectionMerge %47 None
OpBranchConditional %42 %45 %46
%45 = OpLabel
%48 = OpLoad %v2float %33
%49 = OpCompositeExtract %float %48 0
%50 = OpFMul %float %float_2 %49
%51 = OpLoad %v2float %34
%52 = OpCompositeExtract %float %51 0
%53 = OpFMul %float %50 %52
OpStore %43 %53
OpBranch %47
%46 = OpLabel
%54 = OpLoad %v2float %33
%55 = OpCompositeExtract %float %54 1
%56 = OpLoad %v2float %34
%57 = OpCompositeExtract %float %56 1
%58 = OpFMul %float %55 %57
%59 = OpLoad %v2float %34
%60 = OpCompositeExtract %float %59 1
%61 = OpLoad %v2float %34
%62 = OpCompositeExtract %float %61 0
%63 = OpFSub %float %60 %62
%64 = OpFMul %float %float_2 %63
%65 = OpLoad %v2float %33
%66 = OpCompositeExtract %float %65 1
%67 = OpLoad %v2float %33
%68 = OpCompositeExtract %float %67 0
%69 = OpFSub %float %66 %68
%70 = OpFMul %float %64 %69
%71 = OpFSub %float %58 %70
OpStore %43 %71
OpBranch %47
%47 = OpLabel
%72 = OpLoad %float %43
OpReturnValue %72
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %73
%75 = OpFunctionParameter %_ptr_Function_v4float
%76 = OpFunctionParameter %_ptr_Function_v4float
%77 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%81 = OpVariable %_ptr_Function_v2float Function
%84 = OpVariable %_ptr_Function_v2float Function
%88 = OpVariable %_ptr_Function_v2float Function
%91 = OpVariable %_ptr_Function_v2float Function
%95 = OpVariable %_ptr_Function_v2float Function
%98 = OpVariable %_ptr_Function_v2float Function
%79 = OpLoad %v4float %75
%80 = OpVectorShuffle %v2float %79 %79 0 3
OpStore %81 %80
%82 = OpLoad %v4float %76
%83 = OpVectorShuffle %v2float %82 %82 0 3
OpStore %84 %83
%85 = OpFunctionCall %float %_blend_overlay_component %81 %84
%86 = OpLoad %v4float %75
%87 = OpVectorShuffle %v2float %86 %86 1 3
OpStore %88 %87
%89 = OpLoad %v4float %76
%90 = OpVectorShuffle %v2float %89 %89 1 3
OpStore %91 %90
%92 = OpFunctionCall %float %_blend_overlay_component %88 %91
%93 = OpLoad %v4float %75
%94 = OpVectorShuffle %v2float %93 %93 2 3
OpStore %95 %94
%96 = OpLoad %v4float %76
%97 = OpVectorShuffle %v2float %96 %96 2 3
OpStore %98 %97
%99 = OpFunctionCall %float %_blend_overlay_component %95 %98
%100 = OpLoad %v4float %75
%101 = OpCompositeExtract %float %100 3
%103 = OpLoad %v4float %75
%104 = OpCompositeExtract %float %103 3
%105 = OpFSub %float %float_1 %104
%106 = OpLoad %v4float %76
%107 = OpCompositeExtract %float %106 3
%108 = OpFMul %float %105 %107
%109 = OpFAdd %float %101 %108
%110 = OpCompositeConstruct %v4float %85 %92 %99 %109
OpStore %result %110
%111 = OpLoad %v4float %result
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%114 = OpLoad %v4float %76
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%116 = OpLoad %v4float %75
%117 = OpCompositeExtract %float %116 3
%118 = OpFSub %float %float_1 %117
%119 = OpVectorTimesScalar %v3float %115 %118
%120 = OpLoad %v4float %75
%121 = OpVectorShuffle %v3float %120 %120 0 1 2
%122 = OpLoad %v4float %76
%123 = OpCompositeExtract %float %122 3
%124 = OpFSub %float %float_1 %123
%125 = OpVectorTimesScalar %v3float %121 %124
%126 = OpFAdd %v3float %119 %125
%127 = OpFAdd %v3float %112 %126
%128 = OpLoad %v4float %result
%129 = OpVectorShuffle %v4float %128 %127 4 5 6 3
OpStore %result %129
%130 = OpLoad %v4float %result
OpReturnValue %130
OpFunctionEnd
%blend_darken = OpFunction %v4float None %73
%131 = OpFunctionParameter %_ptr_Function_v4float
%132 = OpFunctionParameter %_ptr_Function_v4float
%133 = OpLabel
%result_0 = OpVariable %_ptr_Function_v4float Function
%135 = OpLoad %v4float %131
%136 = OpLoad %v4float %131
%137 = OpCompositeExtract %float %136 3
%138 = OpFSub %float %float_1 %137
%139 = OpLoad %v4float %132
%140 = OpVectorTimesScalar %v4float %139 %138
%141 = OpFAdd %v4float %135 %140
OpStore %result_0 %141
%143 = OpLoad %v4float %result_0
%144 = OpVectorShuffle %v3float %143 %143 0 1 2
%145 = OpLoad %v4float %132
%146 = OpCompositeExtract %float %145 3
%147 = OpFSub %float %float_1 %146
%148 = OpLoad %v4float %131
%149 = OpVectorShuffle %v3float %148 %148 0 1 2
%150 = OpVectorTimesScalar %v3float %149 %147
%151 = OpLoad %v4float %132
%152 = OpVectorShuffle %v3float %151 %151 0 1 2
%153 = OpFAdd %v3float %150 %152
%142 = OpExtInst %v3float %1 FMin %144 %153
%154 = OpLoad %v4float %result_0
%155 = OpVectorShuffle %v4float %154 %142 4 5 6 3
OpStore %result_0 %155
%156 = OpLoad %v4float %result_0
OpReturnValue %156
OpFunctionEnd
%blend_lighten = OpFunction %v4float None %73
%157 = OpFunctionParameter %_ptr_Function_v4float
%158 = OpFunctionParameter %_ptr_Function_v4float
%159 = OpLabel
%result_1 = OpVariable %_ptr_Function_v4float Function
%161 = OpLoad %v4float %157
%162 = OpLoad %v4float %157
%163 = OpCompositeExtract %float %162 3
%164 = OpFSub %float %float_1 %163
%165 = OpLoad %v4float %158
%166 = OpVectorTimesScalar %v4float %165 %164
%167 = OpFAdd %v4float %161 %166
OpStore %result_1 %167
%169 = OpLoad %v4float %result_1
%170 = OpVectorShuffle %v3float %169 %169 0 1 2
%171 = OpLoad %v4float %158
%172 = OpCompositeExtract %float %171 3
%173 = OpFSub %float %float_1 %172
%174 = OpLoad %v4float %157
%175 = OpVectorShuffle %v3float %174 %174 0 1 2
%176 = OpVectorTimesScalar %v3float %175 %173
%177 = OpLoad %v4float %158
%178 = OpVectorShuffle %v3float %177 %177 0 1 2
%179 = OpFAdd %v3float %176 %178
%168 = OpExtInst %v3float %1 FMax %170 %179
%180 = OpLoad %v4float %result_1
%181 = OpVectorShuffle %v4float %180 %168 4 5 6 3
OpStore %result_1 %181
%182 = OpLoad %v4float %result_1
OpReturnValue %182
OpFunctionEnd
%_guarded_divide = OpFunction %float None %183
%184 = OpFunctionParameter %_ptr_Function_float
%185 = OpFunctionParameter %_ptr_Function_float
%186 = OpLabel
%187 = OpLoad %float %184
%188 = OpLoad %float %185
%189 = OpFDiv %float %187 %188
OpReturnValue %189
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %31
%190 = OpFunctionParameter %_ptr_Function_v2float
%191 = OpFunctionParameter %_ptr_Function_v2float
%192 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%244 = OpVariable %_ptr_Function_float Function
%246 = OpVariable %_ptr_Function_float Function
%193 = OpLoad %v2float %191
%194 = OpCompositeExtract %float %193 0
%196 = OpFOrdEqual %bool %194 %float_0
OpSelectionMerge %199 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%200 = OpLoad %v2float %190
%201 = OpCompositeExtract %float %200 0
%202 = OpLoad %v2float %191
%203 = OpCompositeExtract %float %202 1
%204 = OpFSub %float %float_1 %203
%205 = OpFMul %float %201 %204
OpReturnValue %205
%198 = OpLabel
%207 = OpLoad %v2float %190
%208 = OpCompositeExtract %float %207 1
%209 = OpLoad %v2float %190
%210 = OpCompositeExtract %float %209 0
%211 = OpFSub %float %208 %210
OpStore %delta %211
%212 = OpLoad %float %delta
%213 = OpFOrdEqual %bool %212 %float_0
OpSelectionMerge %216 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%217 = OpLoad %v2float %190
%218 = OpCompositeExtract %float %217 1
%219 = OpLoad %v2float %191
%220 = OpCompositeExtract %float %219 1
%221 = OpFMul %float %218 %220
%222 = OpLoad %v2float %190
%223 = OpCompositeExtract %float %222 0
%224 = OpLoad %v2float %191
%225 = OpCompositeExtract %float %224 1
%226 = OpFSub %float %float_1 %225
%227 = OpFMul %float %223 %226
%228 = OpFAdd %float %221 %227
%229 = OpLoad %v2float %191
%230 = OpCompositeExtract %float %229 0
%231 = OpLoad %v2float %190
%232 = OpCompositeExtract %float %231 1
%233 = OpFSub %float %float_1 %232
%234 = OpFMul %float %230 %233
%235 = OpFAdd %float %228 %234
OpReturnValue %235
%215 = OpLabel
%237 = OpLoad %v2float %191
%238 = OpCompositeExtract %float %237 1
%239 = OpLoad %v2float %191
%240 = OpCompositeExtract %float %239 0
%241 = OpLoad %v2float %190
%242 = OpCompositeExtract %float %241 1
%243 = OpFMul %float %240 %242
OpStore %244 %243
%245 = OpLoad %float %delta
OpStore %246 %245
%247 = OpFunctionCall %float %_guarded_divide %244 %246
%236 = OpExtInst %float %1 FMin %238 %247
OpStore %delta %236
%248 = OpLoad %float %delta
%249 = OpLoad %v2float %190
%250 = OpCompositeExtract %float %249 1
%251 = OpFMul %float %248 %250
%252 = OpLoad %v2float %190
%253 = OpCompositeExtract %float %252 0
%254 = OpLoad %v2float %191
%255 = OpCompositeExtract %float %254 1
%256 = OpFSub %float %float_1 %255
%257 = OpFMul %float %253 %256
%258 = OpFAdd %float %251 %257
%259 = OpLoad %v2float %191
%260 = OpCompositeExtract %float %259 0
%261 = OpLoad %v2float %190
%262 = OpCompositeExtract %float %261 1
%263 = OpFSub %float %float_1 %262
%264 = OpFMul %float %260 %263
%265 = OpFAdd %float %258 %264
OpReturnValue %265
%216 = OpLabel
OpBranch %199
%199 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component = OpFunction %float None %31
%266 = OpFunctionParameter %_ptr_Function_v2float
%267 = OpFunctionParameter %_ptr_Function_v2float
%268 = OpLabel
%delta_0 = OpVariable %_ptr_Function_float Function
%320 = OpVariable %_ptr_Function_float Function
%323 = OpVariable %_ptr_Function_float Function
%269 = OpLoad %v2float %267
%270 = OpCompositeExtract %float %269 1
%271 = OpLoad %v2float %267
%272 = OpCompositeExtract %float %271 0
%273 = OpFOrdEqual %bool %270 %272
OpSelectionMerge %276 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%277 = OpLoad %v2float %266
%278 = OpCompositeExtract %float %277 1
%279 = OpLoad %v2float %267
%280 = OpCompositeExtract %float %279 1
%281 = OpFMul %float %278 %280
%282 = OpLoad %v2float %266
%283 = OpCompositeExtract %float %282 0
%284 = OpLoad %v2float %267
%285 = OpCompositeExtract %float %284 1
%286 = OpFSub %float %float_1 %285
%287 = OpFMul %float %283 %286
%288 = OpFAdd %float %281 %287
%289 = OpLoad %v2float %267
%290 = OpCompositeExtract %float %289 0
%291 = OpLoad %v2float %266
%292 = OpCompositeExtract %float %291 1
%293 = OpFSub %float %float_1 %292
%294 = OpFMul %float %290 %293
%295 = OpFAdd %float %288 %294
OpReturnValue %295
%275 = OpLabel
%296 = OpLoad %v2float %266
%297 = OpCompositeExtract %float %296 0
%298 = OpFOrdEqual %bool %297 %float_0
OpSelectionMerge %301 None
OpBranchConditional %298 %299 %300
%299 = OpLabel
%302 = OpLoad %v2float %267
%303 = OpCompositeExtract %float %302 0
%304 = OpLoad %v2float %266
%305 = OpCompositeExtract %float %304 1
%306 = OpFSub %float %float_1 %305
%307 = OpFMul %float %303 %306
OpReturnValue %307
%300 = OpLabel
%310 = OpLoad %v2float %267
%311 = OpCompositeExtract %float %310 1
%312 = OpLoad %v2float %267
%313 = OpCompositeExtract %float %312 1
%314 = OpLoad %v2float %267
%315 = OpCompositeExtract %float %314 0
%316 = OpFSub %float %313 %315
%317 = OpLoad %v2float %266
%318 = OpCompositeExtract %float %317 1
%319 = OpFMul %float %316 %318
OpStore %320 %319
%321 = OpLoad %v2float %266
%322 = OpCompositeExtract %float %321 0
OpStore %323 %322
%324 = OpFunctionCall %float %_guarded_divide %320 %323
%325 = OpFSub %float %311 %324
%309 = OpExtInst %float %1 FMax %float_0 %325
OpStore %delta_0 %309
%326 = OpLoad %float %delta_0
%327 = OpLoad %v2float %266
%328 = OpCompositeExtract %float %327 1
%329 = OpFMul %float %326 %328
%330 = OpLoad %v2float %266
%331 = OpCompositeExtract %float %330 0
%332 = OpLoad %v2float %267
%333 = OpCompositeExtract %float %332 1
%334 = OpFSub %float %float_1 %333
%335 = OpFMul %float %331 %334
%336 = OpFAdd %float %329 %335
%337 = OpLoad %v2float %267
%338 = OpCompositeExtract %float %337 0
%339 = OpLoad %v2float %266
%340 = OpCompositeExtract %float %339 1
%341 = OpFSub %float %float_1 %340
%342 = OpFMul %float %338 %341
%343 = OpFAdd %float %336 %342
OpReturnValue %343
%301 = OpLabel
OpBranch %276
%276 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component = OpFunction %float None %31
%344 = OpFunctionParameter %_ptr_Function_v2float
%345 = OpFunctionParameter %_ptr_Function_v2float
%346 = OpLabel
%368 = OpVariable %_ptr_Function_float Function
%371 = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%472 = OpVariable %_ptr_Function_float Function
%474 = OpVariable %_ptr_Function_float Function
%347 = OpLoad %v2float %344
%348 = OpCompositeExtract %float %347 0
%349 = OpFMul %float %float_2 %348
%350 = OpLoad %v2float %344
%351 = OpCompositeExtract %float %350 1
%352 = OpFOrdLessThanEqual %bool %349 %351
OpSelectionMerge %355 None
OpBranchConditional %352 %353 %354
%353 = OpLabel
%356 = OpLoad %v2float %345
%357 = OpCompositeExtract %float %356 0
%358 = OpLoad %v2float %345
%359 = OpCompositeExtract %float %358 0
%360 = OpFMul %float %357 %359
%361 = OpLoad %v2float %344
%362 = OpCompositeExtract %float %361 1
%363 = OpLoad %v2float %344
%364 = OpCompositeExtract %float %363 0
%365 = OpFMul %float %float_2 %364
%366 = OpFSub %float %362 %365
%367 = OpFMul %float %360 %366
OpStore %368 %367
%369 = OpLoad %v2float %345
%370 = OpCompositeExtract %float %369 1
OpStore %371 %370
%372 = OpFunctionCall %float %_guarded_divide %368 %371
%373 = OpLoad %v2float %345
%374 = OpCompositeExtract %float %373 1
%375 = OpFSub %float %float_1 %374
%376 = OpLoad %v2float %344
%377 = OpCompositeExtract %float %376 0
%378 = OpFMul %float %375 %377
%379 = OpFAdd %float %372 %378
%380 = OpLoad %v2float %345
%381 = OpCompositeExtract %float %380 0
%383 = OpLoad %v2float %344
%384 = OpCompositeExtract %float %383 1
%382 = OpFNegate %float %384
%385 = OpLoad %v2float %344
%386 = OpCompositeExtract %float %385 0
%387 = OpFMul %float %float_2 %386
%388 = OpFAdd %float %382 %387
%389 = OpFAdd %float %388 %float_1
%390 = OpFMul %float %381 %389
%391 = OpFAdd %float %379 %390
OpReturnValue %391
%354 = OpLabel
%393 = OpLoad %v2float %345
%394 = OpCompositeExtract %float %393 0
%395 = OpFMul %float %float_4 %394
%396 = OpLoad %v2float %345
%397 = OpCompositeExtract %float %396 1
%398 = OpFOrdLessThanEqual %bool %395 %397
OpSelectionMerge %401 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%403 = OpLoad %v2float %345
%404 = OpCompositeExtract %float %403 0
%405 = OpLoad %v2float %345
%406 = OpCompositeExtract %float %405 0
%407 = OpFMul %float %404 %406
OpStore %DSqd %407
%409 = OpLoad %float %DSqd
%410 = OpLoad %v2float %345
%411 = OpCompositeExtract %float %410 0
%412 = OpFMul %float %409 %411
OpStore %DCub %412
%414 = OpLoad %v2float %345
%415 = OpCompositeExtract %float %414 1
%416 = OpLoad %v2float %345
%417 = OpCompositeExtract %float %416 1
%418 = OpFMul %float %415 %417
OpStore %DaSqd %418
%420 = OpLoad %float %DaSqd
%421 = OpLoad %v2float %345
%422 = OpCompositeExtract %float %421 1
%423 = OpFMul %float %420 %422
OpStore %DaCub %423
%424 = OpLoad %float %DaSqd
%425 = OpLoad %v2float %344
%426 = OpCompositeExtract %float %425 0
%427 = OpLoad %v2float %345
%428 = OpCompositeExtract %float %427 0
%430 = OpLoad %v2float %344
%431 = OpCompositeExtract %float %430 1
%432 = OpFMul %float %float_3 %431
%434 = OpLoad %v2float %344
%435 = OpCompositeExtract %float %434 0
%436 = OpFMul %float %float_6 %435
%437 = OpFSub %float %432 %436
%438 = OpFSub %float %437 %float_1
%439 = OpFMul %float %428 %438
%440 = OpFSub %float %426 %439
%441 = OpFMul %float %424 %440
%443 = OpLoad %v2float %345
%444 = OpCompositeExtract %float %443 1
%445 = OpFMul %float %float_12 %444
%446 = OpLoad %float %DSqd
%447 = OpFMul %float %445 %446
%448 = OpLoad %v2float %344
%449 = OpCompositeExtract %float %448 1
%450 = OpLoad %v2float %344
%451 = OpCompositeExtract %float %450 0
%452 = OpFMul %float %float_2 %451
%453 = OpFSub %float %449 %452
%454 = OpFMul %float %447 %453
%455 = OpFAdd %float %441 %454
%457 = OpLoad %float %DCub
%458 = OpFMul %float %float_16 %457
%459 = OpLoad %v2float %344
%460 = OpCompositeExtract %float %459 1
%461 = OpLoad %v2float %344
%462 = OpCompositeExtract %float %461 0
%463 = OpFMul %float %float_2 %462
%464 = OpFSub %float %460 %463
%465 = OpFMul %float %458 %464
%466 = OpFSub %float %455 %465
%467 = OpLoad %float %DaCub
%468 = OpLoad %v2float %344
%469 = OpCompositeExtract %float %468 0
%470 = OpFMul %float %467 %469
%471 = OpFSub %float %466 %470
OpStore %472 %471
%473 = OpLoad %float %DaSqd
OpStore %474 %473
%475 = OpFunctionCall %float %_guarded_divide %472 %474
OpReturnValue %475
%400 = OpLabel
%476 = OpLoad %v2float %345
%477 = OpCompositeExtract %float %476 0
%478 = OpLoad %v2float %344
%479 = OpCompositeExtract %float %478 1
%480 = OpLoad %v2float %344
%481 = OpCompositeExtract %float %480 0
%482 = OpFMul %float %float_2 %481
%483 = OpFSub %float %479 %482
%484 = OpFAdd %float %483 %float_1
%485 = OpFMul %float %477 %484
%486 = OpLoad %v2float %344
%487 = OpCompositeExtract %float %486 0
%488 = OpFAdd %float %485 %487
%490 = OpLoad %v2float %345
%491 = OpCompositeExtract %float %490 1
%492 = OpLoad %v2float %345
%493 = OpCompositeExtract %float %492 0
%494 = OpFMul %float %491 %493
%489 = OpExtInst %float %1 Sqrt %494
%495 = OpLoad %v2float %344
%496 = OpCompositeExtract %float %495 1
%497 = OpLoad %v2float %344
%498 = OpCompositeExtract %float %497 0
%499 = OpFMul %float %float_2 %498
%500 = OpFSub %float %496 %499
%501 = OpFMul %float %489 %500
%502 = OpFSub %float %488 %501
%503 = OpLoad %v2float %345
%504 = OpCompositeExtract %float %503 1
%505 = OpLoad %v2float %344
%506 = OpCompositeExtract %float %505 0
%507 = OpFMul %float %504 %506
%508 = OpFSub %float %502 %507
OpReturnValue %508
%401 = OpLabel
OpBranch %355
%355 = OpLabel
OpUnreachable
OpFunctionEnd
%_guarded_divide_0 = OpFunction %v3float None %509
%511 = OpFunctionParameter %_ptr_Function_v3float
%512 = OpFunctionParameter %_ptr_Function_float
%513 = OpLabel
%514 = OpLoad %v3float %511
%515 = OpLoad %float %512
%516 = OpFDiv %float %float_1 %515
%517 = OpVectorTimesScalar %v3float %514 %516
OpReturnValue %517
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %518
%519 = OpFunctionParameter %_ptr_Function_v3float
%520 = OpFunctionParameter %_ptr_Function_float
%521 = OpFunctionParameter %_ptr_Function_v3float
%522 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_2 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%573 = OpVariable %_ptr_Function_float Function
%577 = OpVariable %_ptr_Function_float Function
%603 = OpVariable %_ptr_Function_v3float Function
%607 = OpVariable %_ptr_Function_float Function
%529 = OpLoad %v3float %521
%524 = OpDot %float %528 %529
OpStore %lum %524
%531 = OpLoad %float %lum
%533 = OpLoad %v3float %519
%532 = OpDot %float %528 %533
%534 = OpFSub %float %531 %532
%535 = OpLoad %v3float %519
%536 = OpCompositeConstruct %v3float %534 %534 %534
%537 = OpFAdd %v3float %536 %535
OpStore %result_2 %537
%541 = OpLoad %v3float %result_2
%542 = OpCompositeExtract %float %541 0
%543 = OpLoad %v3float %result_2
%544 = OpCompositeExtract %float %543 1
%540 = OpExtInst %float %1 FMin %542 %544
%545 = OpLoad %v3float %result_2
%546 = OpCompositeExtract %float %545 2
%539 = OpExtInst %float %1 FMin %540 %546
OpStore %minComp %539
%550 = OpLoad %v3float %result_2
%551 = OpCompositeExtract %float %550 0
%552 = OpLoad %v3float %result_2
%553 = OpCompositeExtract %float %552 1
%549 = OpExtInst %float %1 FMax %551 %553
%554 = OpLoad %v3float %result_2
%555 = OpCompositeExtract %float %554 2
%548 = OpExtInst %float %1 FMax %549 %555
OpStore %maxComp %548
%557 = OpLoad %float %minComp
%558 = OpFOrdLessThan %bool %557 %float_0
OpSelectionMerge %560 None
OpBranchConditional %558 %559 %560
%559 = OpLabel
%561 = OpLoad %float %lum
%562 = OpLoad %float %minComp
%563 = OpFOrdNotEqual %bool %561 %562
OpBranch %560
%560 = OpLabel
%564 = OpPhi %bool %false %522 %563 %559
OpSelectionMerge %566 None
OpBranchConditional %564 %565 %566
%565 = OpLabel
%567 = OpLoad %float %lum
%568 = OpLoad %v3float %result_2
%569 = OpLoad %float %lum
%570 = OpCompositeConstruct %v3float %569 %569 %569
%571 = OpFSub %v3float %568 %570
%572 = OpLoad %float %lum
OpStore %573 %572
%574 = OpLoad %float %lum
%575 = OpLoad %float %minComp
%576 = OpFSub %float %574 %575
OpStore %577 %576
%578 = OpFunctionCall %float %_guarded_divide %573 %577
%579 = OpVectorTimesScalar %v3float %571 %578
%580 = OpCompositeConstruct %v3float %567 %567 %567
%581 = OpFAdd %v3float %580 %579
OpStore %result_2 %581
OpBranch %566
%566 = OpLabel
%582 = OpLoad %float %maxComp
%583 = OpLoad %float %520
%584 = OpFOrdGreaterThan %bool %582 %583
OpSelectionMerge %586 None
OpBranchConditional %584 %585 %586
%585 = OpLabel
%587 = OpLoad %float %maxComp
%588 = OpLoad %float %lum
%589 = OpFOrdNotEqual %bool %587 %588
OpBranch %586
%586 = OpLabel
%590 = OpPhi %bool %false %566 %589 %585
OpSelectionMerge %593 None
OpBranchConditional %590 %591 %592
%591 = OpLabel
%594 = OpLoad %float %lum
%595 = OpLoad %v3float %result_2
%596 = OpLoad %float %lum
%597 = OpCompositeConstruct %v3float %596 %596 %596
%598 = OpFSub %v3float %595 %597
%599 = OpLoad %float %520
%600 = OpLoad %float %lum
%601 = OpFSub %float %599 %600
%602 = OpVectorTimesScalar %v3float %598 %601
OpStore %603 %602
%604 = OpLoad %float %maxComp
%605 = OpLoad %float %lum
%606 = OpFSub %float %604 %605
OpStore %607 %606
%608 = OpFunctionCall %v3float %_guarded_divide_0 %603 %607
%609 = OpCompositeConstruct %v3float %594 %594 %594
%610 = OpFAdd %v3float %609 %608
OpReturnValue %610
%592 = OpLabel
%611 = OpLoad %v3float %result_2
OpReturnValue %611
%593 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %509
%612 = OpFunctionParameter %_ptr_Function_v3float
%613 = OpFunctionParameter %_ptr_Function_float
%614 = OpLabel
%630 = OpVariable %_ptr_Function_float Function
%636 = OpVariable %_ptr_Function_float Function
%615 = OpLoad %v3float %612
%616 = OpCompositeExtract %float %615 0
%617 = OpLoad %v3float %612
%618 = OpCompositeExtract %float %617 2
%619 = OpFOrdLessThan %bool %616 %618
OpSelectionMerge %622 None
OpBranchConditional %619 %620 %621
%620 = OpLabel
%623 = OpLoad %float %613
%624 = OpLoad %v3float %612
%625 = OpCompositeExtract %float %624 1
%626 = OpLoad %v3float %612
%627 = OpCompositeExtract %float %626 0
%628 = OpFSub %float %625 %627
%629 = OpFMul %float %623 %628
OpStore %630 %629
%631 = OpLoad %v3float %612
%632 = OpCompositeExtract %float %631 2
%633 = OpLoad %v3float %612
%634 = OpCompositeExtract %float %633 0
%635 = OpFSub %float %632 %634
OpStore %636 %635
%637 = OpFunctionCall %float %_guarded_divide %630 %636
%638 = OpLoad %float %613
%639 = OpCompositeConstruct %v3float %float_0 %637 %638
OpReturnValue %639
%621 = OpLabel
OpReturnValue %640
%622 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %641
%642 = OpFunctionParameter %_ptr_Function_v3float
%643 = OpFunctionParameter %_ptr_Function_v3float
%644 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%680 = OpVariable %_ptr_Function_v3float Function
%682 = OpVariable %_ptr_Function_float Function
%694 = OpVariable %_ptr_Function_v3float Function
%696 = OpVariable %_ptr_Function_float Function
%701 = OpVariable %_ptr_Function_v3float Function
%703 = OpVariable %_ptr_Function_float Function
%716 = OpVariable %_ptr_Function_v3float Function
%718 = OpVariable %_ptr_Function_float Function
%731 = OpVariable %_ptr_Function_v3float Function
%733 = OpVariable %_ptr_Function_float Function
%738 = OpVariable %_ptr_Function_v3float Function
%740 = OpVariable %_ptr_Function_float Function
%648 = OpLoad %v3float %643
%649 = OpCompositeExtract %float %648 0
%650 = OpLoad %v3float %643
%651 = OpCompositeExtract %float %650 1
%647 = OpExtInst %float %1 FMax %649 %651
%652 = OpLoad %v3float %643
%653 = OpCompositeExtract %float %652 2
%646 = OpExtInst %float %1 FMax %647 %653
%656 = OpLoad %v3float %643
%657 = OpCompositeExtract %float %656 0
%658 = OpLoad %v3float %643
%659 = OpCompositeExtract %float %658 1
%655 = OpExtInst %float %1 FMin %657 %659
%660 = OpLoad %v3float %643
%661 = OpCompositeExtract %float %660 2
%654 = OpExtInst %float %1 FMin %655 %661
%662 = OpFSub %float %646 %654
OpStore %sat %662
%663 = OpLoad %v3float %642
%664 = OpCompositeExtract %float %663 0
%665 = OpLoad %v3float %642
%666 = OpCompositeExtract %float %665 1
%667 = OpFOrdLessThanEqual %bool %664 %666
OpSelectionMerge %670 None
OpBranchConditional %667 %668 %669
%668 = OpLabel
%671 = OpLoad %v3float %642
%672 = OpCompositeExtract %float %671 1
%673 = OpLoad %v3float %642
%674 = OpCompositeExtract %float %673 2
%675 = OpFOrdLessThanEqual %bool %672 %674
OpSelectionMerge %678 None
OpBranchConditional %675 %676 %677
%676 = OpLabel
%679 = OpLoad %v3float %642
OpStore %680 %679
%681 = OpLoad %float %sat
OpStore %682 %681
%683 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %680 %682
OpReturnValue %683
%677 = OpLabel
%684 = OpLoad %v3float %642
%685 = OpCompositeExtract %float %684 0
%686 = OpLoad %v3float %642
%687 = OpCompositeExtract %float %686 2
%688 = OpFOrdLessThanEqual %bool %685 %687
OpSelectionMerge %691 None
OpBranchConditional %688 %689 %690
%689 = OpLabel
%692 = OpLoad %v3float %642
%693 = OpVectorShuffle %v3float %692 %692 0 2 1
OpStore %694 %693
%695 = OpLoad %float %sat
OpStore %696 %695
%697 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %694 %696
%698 = OpVectorShuffle %v3float %697 %697 0 2 1
OpReturnValue %698
%690 = OpLabel
%699 = OpLoad %v3float %642
%700 = OpVectorShuffle %v3float %699 %699 2 0 1
OpStore %701 %700
%702 = OpLoad %float %sat
OpStore %703 %702
%704 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %701 %703
%705 = OpVectorShuffle %v3float %704 %704 1 2 0
OpReturnValue %705
%691 = OpLabel
OpBranch %678
%678 = OpLabel
OpBranch %670
%669 = OpLabel
%706 = OpLoad %v3float %642
%707 = OpCompositeExtract %float %706 0
%708 = OpLoad %v3float %642
%709 = OpCompositeExtract %float %708 2
%710 = OpFOrdLessThanEqual %bool %707 %709
OpSelectionMerge %713 None
OpBranchConditional %710 %711 %712
%711 = OpLabel
%714 = OpLoad %v3float %642
%715 = OpVectorShuffle %v3float %714 %714 1 0 2
OpStore %716 %715
%717 = OpLoad %float %sat
OpStore %718 %717
%719 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %716 %718
%720 = OpVectorShuffle %v3float %719 %719 1 0 2
OpReturnValue %720
%712 = OpLabel
%721 = OpLoad %v3float %642
%722 = OpCompositeExtract %float %721 1
%723 = OpLoad %v3float %642
%724 = OpCompositeExtract %float %723 2
%725 = OpFOrdLessThanEqual %bool %722 %724
OpSelectionMerge %728 None
OpBranchConditional %725 %726 %727
%726 = OpLabel
%729 = OpLoad %v3float %642
%730 = OpVectorShuffle %v3float %729 %729 1 2 0
OpStore %731 %730
%732 = OpLoad %float %sat
OpStore %733 %732
%734 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %731 %733
%735 = OpVectorShuffle %v3float %734 %734 2 0 1
OpReturnValue %735
%727 = OpLabel
%736 = OpLoad %v3float %642
%737 = OpVectorShuffle %v3float %736 %736 2 1 0
OpStore %738 %737
%739 = OpLoad %float %sat
OpStore %740 %739
%741 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %738 %740
%742 = OpVectorShuffle %v3float %741 %741 2 1 0
OpReturnValue %742
%728 = OpLabel
OpBranch %713
%713 = OpLabel
OpBranch %670
%670 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hue = OpFunction %v4float None %73
%743 = OpFunctionParameter %_ptr_Function_v4float
%744 = OpFunctionParameter %_ptr_Function_v4float
%745 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%765 = OpVariable %_ptr_Function_v3float Function
%767 = OpVariable %_ptr_Function_v3float Function
%769 = OpVariable %_ptr_Function_v3float Function
%771 = OpVariable %_ptr_Function_float Function
%773 = OpVariable %_ptr_Function_v3float Function
%747 = OpLoad %v4float %744
%748 = OpCompositeExtract %float %747 3
%749 = OpLoad %v4float %743
%750 = OpCompositeExtract %float %749 3
%751 = OpFMul %float %748 %750
OpStore %alpha %751
%753 = OpLoad %v4float %743
%754 = OpVectorShuffle %v3float %753 %753 0 1 2
%755 = OpLoad %v4float %744
%756 = OpCompositeExtract %float %755 3
%757 = OpVectorTimesScalar %v3float %754 %756
OpStore %sda %757
%759 = OpLoad %v4float %744
%760 = OpVectorShuffle %v3float %759 %759 0 1 2
%761 = OpLoad %v4float %743
%762 = OpCompositeExtract %float %761 3
%763 = OpVectorTimesScalar %v3float %760 %762
OpStore %dsa %763
%764 = OpLoad %v3float %sda
OpStore %765 %764
%766 = OpLoad %v3float %dsa
OpStore %767 %766
%768 = OpFunctionCall %v3float %_blend_set_color_saturation %765 %767
OpStore %769 %768
%770 = OpLoad %float %alpha
OpStore %771 %770
%772 = OpLoad %v3float %dsa
OpStore %773 %772
%774 = OpFunctionCall %v3float %_blend_set_color_luminance %769 %771 %773
%775 = OpLoad %v4float %744
%776 = OpVectorShuffle %v3float %775 %775 0 1 2
%777 = OpFAdd %v3float %774 %776
%778 = OpLoad %v3float %dsa
%779 = OpFSub %v3float %777 %778
%780 = OpLoad %v4float %743
%781 = OpVectorShuffle %v3float %780 %780 0 1 2
%782 = OpFAdd %v3float %779 %781
%783 = OpLoad %v3float %sda
%784 = OpFSub %v3float %782 %783
%785 = OpCompositeExtract %float %784 0
%786 = OpCompositeExtract %float %784 1
%787 = OpCompositeExtract %float %784 2
%788 = OpLoad %v4float %743
%789 = OpCompositeExtract %float %788 3
%790 = OpLoad %v4float %744
%791 = OpCompositeExtract %float %790 3
%792 = OpFAdd %float %789 %791
%793 = OpLoad %float %alpha
%794 = OpFSub %float %792 %793
%795 = OpCompositeConstruct %v4float %785 %786 %787 %794
OpReturnValue %795
OpFunctionEnd
%blend_saturation = OpFunction %v4float None %73
%796 = OpFunctionParameter %_ptr_Function_v4float
%797 = OpFunctionParameter %_ptr_Function_v4float
%798 = OpLabel
%alpha_0 = OpVariable %_ptr_Function_float Function
%sda_0 = OpVariable %_ptr_Function_v3float Function
%dsa_0 = OpVariable %_ptr_Function_v3float Function
%818 = OpVariable %_ptr_Function_v3float Function
%820 = OpVariable %_ptr_Function_v3float Function
%822 = OpVariable %_ptr_Function_v3float Function
%824 = OpVariable %_ptr_Function_float Function
%826 = OpVariable %_ptr_Function_v3float Function
%800 = OpLoad %v4float %797
%801 = OpCompositeExtract %float %800 3
%802 = OpLoad %v4float %796
%803 = OpCompositeExtract %float %802 3
%804 = OpFMul %float %801 %803
OpStore %alpha_0 %804
%806 = OpLoad %v4float %796
%807 = OpVectorShuffle %v3float %806 %806 0 1 2
%808 = OpLoad %v4float %797
%809 = OpCompositeExtract %float %808 3
%810 = OpVectorTimesScalar %v3float %807 %809
OpStore %sda_0 %810
%812 = OpLoad %v4float %797
%813 = OpVectorShuffle %v3float %812 %812 0 1 2
%814 = OpLoad %v4float %796
%815 = OpCompositeExtract %float %814 3
%816 = OpVectorTimesScalar %v3float %813 %815
OpStore %dsa_0 %816
%817 = OpLoad %v3float %dsa_0
OpStore %818 %817
%819 = OpLoad %v3float %sda_0
OpStore %820 %819
%821 = OpFunctionCall %v3float %_blend_set_color_saturation %818 %820
OpStore %822 %821
%823 = OpLoad %float %alpha_0
OpStore %824 %823
%825 = OpLoad %v3float %dsa_0
OpStore %826 %825
%827 = OpFunctionCall %v3float %_blend_set_color_luminance %822 %824 %826
%828 = OpLoad %v4float %797
%829 = OpVectorShuffle %v3float %828 %828 0 1 2
%830 = OpFAdd %v3float %827 %829
%831 = OpLoad %v3float %dsa_0
%832 = OpFSub %v3float %830 %831
%833 = OpLoad %v4float %796
%834 = OpVectorShuffle %v3float %833 %833 0 1 2
%835 = OpFAdd %v3float %832 %834
%836 = OpLoad %v3float %sda_0
%837 = OpFSub %v3float %835 %836
%838 = OpCompositeExtract %float %837 0
%839 = OpCompositeExtract %float %837 1
%840 = OpCompositeExtract %float %837 2
%841 = OpLoad %v4float %796
%842 = OpCompositeExtract %float %841 3
%843 = OpLoad %v4float %797
%844 = OpCompositeExtract %float %843 3
%845 = OpFAdd %float %842 %844
%846 = OpLoad %float %alpha_0
%847 = OpFSub %float %845 %846
%848 = OpCompositeConstruct %v4float %838 %839 %840 %847
OpReturnValue %848
OpFunctionEnd
%blend_color = OpFunction %v4float None %73
%849 = OpFunctionParameter %_ptr_Function_v4float
%850 = OpFunctionParameter %_ptr_Function_v4float
%851 = OpLabel
%alpha_1 = OpVariable %_ptr_Function_float Function
%sda_1 = OpVariable %_ptr_Function_v3float Function
%dsa_1 = OpVariable %_ptr_Function_v3float Function
%871 = OpVariable %_ptr_Function_v3float Function
%873 = OpVariable %_ptr_Function_float Function
%875 = OpVariable %_ptr_Function_v3float Function
%853 = OpLoad %v4float %850
%854 = OpCompositeExtract %float %853 3
%855 = OpLoad %v4float %849
%856 = OpCompositeExtract %float %855 3
%857 = OpFMul %float %854 %856
OpStore %alpha_1 %857
%859 = OpLoad %v4float %849
%860 = OpVectorShuffle %v3float %859 %859 0 1 2
%861 = OpLoad %v4float %850
%862 = OpCompositeExtract %float %861 3
%863 = OpVectorTimesScalar %v3float %860 %862
OpStore %sda_1 %863
%865 = OpLoad %v4float %850
%866 = OpVectorShuffle %v3float %865 %865 0 1 2
%867 = OpLoad %v4float %849
%868 = OpCompositeExtract %float %867 3
%869 = OpVectorTimesScalar %v3float %866 %868
OpStore %dsa_1 %869
%870 = OpLoad %v3float %sda_1
OpStore %871 %870
%872 = OpLoad %float %alpha_1
OpStore %873 %872
%874 = OpLoad %v3float %dsa_1
OpStore %875 %874
%876 = OpFunctionCall %v3float %_blend_set_color_luminance %871 %873 %875
%877 = OpLoad %v4float %850
%878 = OpVectorShuffle %v3float %877 %877 0 1 2
%879 = OpFAdd %v3float %876 %878
%880 = OpLoad %v3float %dsa_1
%881 = OpFSub %v3float %879 %880
%882 = OpLoad %v4float %849
%883 = OpVectorShuffle %v3float %882 %882 0 1 2
%884 = OpFAdd %v3float %881 %883
%885 = OpLoad %v3float %sda_1
%886 = OpFSub %v3float %884 %885
%887 = OpCompositeExtract %float %886 0
%888 = OpCompositeExtract %float %886 1
%889 = OpCompositeExtract %float %886 2
%890 = OpLoad %v4float %849
%891 = OpCompositeExtract %float %890 3
%892 = OpLoad %v4float %850
%893 = OpCompositeExtract %float %892 3
%894 = OpFAdd %float %891 %893
%895 = OpLoad %float %alpha_1
%896 = OpFSub %float %894 %895
%897 = OpCompositeConstruct %v4float %887 %888 %889 %896
OpReturnValue %897
OpFunctionEnd
%blend_luminosity = OpFunction %v4float None %73
%898 = OpFunctionParameter %_ptr_Function_v4float
%899 = OpFunctionParameter %_ptr_Function_v4float
%900 = OpLabel
%alpha_2 = OpVariable %_ptr_Function_float Function
%sda_2 = OpVariable %_ptr_Function_v3float Function
%dsa_2 = OpVariable %_ptr_Function_v3float Function
%920 = OpVariable %_ptr_Function_v3float Function
%922 = OpVariable %_ptr_Function_float Function
%924 = OpVariable %_ptr_Function_v3float Function
%902 = OpLoad %v4float %899
%903 = OpCompositeExtract %float %902 3
%904 = OpLoad %v4float %898
%905 = OpCompositeExtract %float %904 3
%906 = OpFMul %float %903 %905
OpStore %alpha_2 %906
%908 = OpLoad %v4float %898
%909 = OpVectorShuffle %v3float %908 %908 0 1 2
%910 = OpLoad %v4float %899
%911 = OpCompositeExtract %float %910 3
%912 = OpVectorTimesScalar %v3float %909 %911
OpStore %sda_2 %912
%914 = OpLoad %v4float %899
%915 = OpVectorShuffle %v3float %914 %914 0 1 2
%916 = OpLoad %v4float %898
%917 = OpCompositeExtract %float %916 3
%918 = OpVectorTimesScalar %v3float %915 %917
OpStore %dsa_2 %918
%919 = OpLoad %v3float %dsa_2
OpStore %920 %919
%921 = OpLoad %float %alpha_2
OpStore %922 %921
%923 = OpLoad %v3float %sda_2
OpStore %924 %923
%925 = OpFunctionCall %v3float %_blend_set_color_luminance %920 %922 %924
%926 = OpLoad %v4float %899
%927 = OpVectorShuffle %v3float %926 %926 0 1 2
%928 = OpFAdd %v3float %925 %927
%929 = OpLoad %v3float %dsa_2
%930 = OpFSub %v3float %928 %929
%931 = OpLoad %v4float %898
%932 = OpVectorShuffle %v3float %931 %931 0 1 2
%933 = OpFAdd %v3float %930 %932
%934 = OpLoad %v3float %sda_2
%935 = OpFSub %v3float %933 %934
%936 = OpCompositeExtract %float %935 0
%937 = OpCompositeExtract %float %935 1
%938 = OpCompositeExtract %float %935 2
%939 = OpLoad %v4float %898
%940 = OpCompositeExtract %float %939 3
%941 = OpLoad %v4float %899
%942 = OpCompositeExtract %float %941 3
%943 = OpFAdd %float %940 %942
%944 = OpLoad %float %alpha_2
%945 = OpFSub %float %943 %944
%946 = OpCompositeConstruct %v4float %936 %937 %938 %945
OpReturnValue %946
OpFunctionEnd
%blend = OpFunction %v4float None %948
%950 = OpFunctionParameter %_ptr_Function_int
%951 = OpFunctionParameter %_ptr_Function_v4float
%952 = OpFunctionParameter %_ptr_Function_v4float
%953 = OpLabel
%1068 = OpVariable %_ptr_Function_v4float Function
%1070 = OpVariable %_ptr_Function_v4float Function
%1073 = OpVariable %_ptr_Function_v4float Function
%1075 = OpVariable %_ptr_Function_v4float Function
%1078 = OpVariable %_ptr_Function_v4float Function
%1080 = OpVariable %_ptr_Function_v4float Function
%1084 = OpVariable %_ptr_Function_v2float Function
%1087 = OpVariable %_ptr_Function_v2float Function
%1091 = OpVariable %_ptr_Function_v2float Function
%1094 = OpVariable %_ptr_Function_v2float Function
%1098 = OpVariable %_ptr_Function_v2float Function
%1101 = OpVariable %_ptr_Function_v2float Function
%1115 = OpVariable %_ptr_Function_v2float Function
%1118 = OpVariable %_ptr_Function_v2float Function
%1122 = OpVariable %_ptr_Function_v2float Function
%1125 = OpVariable %_ptr_Function_v2float Function
%1129 = OpVariable %_ptr_Function_v2float Function
%1132 = OpVariable %_ptr_Function_v2float Function
%1145 = OpVariable %_ptr_Function_v4float Function
%1147 = OpVariable %_ptr_Function_v4float Function
%1152 = OpVariable %_ptr_Function_v4float Function
%1159 = OpVariable %_ptr_Function_v2float Function
%1162 = OpVariable %_ptr_Function_v2float Function
%1166 = OpVariable %_ptr_Function_v2float Function
%1169 = OpVariable %_ptr_Function_v2float Function
%1173 = OpVariable %_ptr_Function_v2float Function
%1176 = OpVariable %_ptr_Function_v2float Function
%1278 = OpVariable %_ptr_Function_v4float Function
%1280 = OpVariable %_ptr_Function_v4float Function
%1283 = OpVariable %_ptr_Function_v4float Function
%1285 = OpVariable %_ptr_Function_v4float Function
%1288 = OpVariable %_ptr_Function_v4float Function
%1290 = OpVariable %_ptr_Function_v4float Function
%1293 = OpVariable %_ptr_Function_v4float Function
%1295 = OpVariable %_ptr_Function_v4float Function
%954 = OpLoad %int %950
OpSelectionMerge %955 None
OpSwitch %954 %985 0 %956 1 %957 2 %958 3 %959 4 %960 5 %961 6 %962 7 %963 8 %964 9 %965 10 %966 11 %967 12 %968 13 %969 14 %970 15 %971 16 %972 17 %973 18 %974 19 %975 20 %976 21 %977 22 %978 23 %979 24 %980 25 %981 26 %982 27 %983 28 %984
%956 = OpLabel
OpReturnValue %986
%957 = OpLabel
%987 = OpLoad %v4float %951
OpReturnValue %987
%958 = OpLabel
%988 = OpLoad %v4float %952
OpReturnValue %988
%959 = OpLabel
%989 = OpLoad %v4float %951
%990 = OpLoad %v4float %951
%991 = OpCompositeExtract %float %990 3
%992 = OpFSub %float %float_1 %991
%993 = OpLoad %v4float %952
%994 = OpVectorTimesScalar %v4float %993 %992
%995 = OpFAdd %v4float %989 %994
OpReturnValue %995
%960 = OpLabel
%996 = OpLoad %v4float %952
%997 = OpCompositeExtract %float %996 3
%998 = OpFSub %float %float_1 %997
%999 = OpLoad %v4float %951
%1000 = OpVectorTimesScalar %v4float %999 %998
%1001 = OpLoad %v4float %952
%1002 = OpFAdd %v4float %1000 %1001
OpReturnValue %1002
%961 = OpLabel
%1003 = OpLoad %v4float %951
%1004 = OpLoad %v4float %952
%1005 = OpCompositeExtract %float %1004 3
%1006 = OpVectorTimesScalar %v4float %1003 %1005
OpReturnValue %1006
%962 = OpLabel
%1007 = OpLoad %v4float %952
%1008 = OpLoad %v4float %951
%1009 = OpCompositeExtract %float %1008 3
%1010 = OpVectorTimesScalar %v4float %1007 %1009
OpReturnValue %1010
%963 = OpLabel
%1011 = OpLoad %v4float %952
%1012 = OpCompositeExtract %float %1011 3
%1013 = OpFSub %float %float_1 %1012
%1014 = OpLoad %v4float %951
%1015 = OpVectorTimesScalar %v4float %1014 %1013
OpReturnValue %1015
%964 = OpLabel
%1016 = OpLoad %v4float %951
%1017 = OpCompositeExtract %float %1016 3
%1018 = OpFSub %float %float_1 %1017
%1019 = OpLoad %v4float %952
%1020 = OpVectorTimesScalar %v4float %1019 %1018
OpReturnValue %1020
%965 = OpLabel
%1021 = OpLoad %v4float %952
%1022 = OpCompositeExtract %float %1021 3
%1023 = OpLoad %v4float %951
%1024 = OpVectorTimesScalar %v4float %1023 %1022
%1025 = OpLoad %v4float %951
%1026 = OpCompositeExtract %float %1025 3
%1027 = OpFSub %float %float_1 %1026
%1028 = OpLoad %v4float %952
%1029 = OpVectorTimesScalar %v4float %1028 %1027
%1030 = OpFAdd %v4float %1024 %1029
OpReturnValue %1030
%966 = OpLabel
%1031 = OpLoad %v4float %952
%1032 = OpCompositeExtract %float %1031 3
%1033 = OpFSub %float %float_1 %1032
%1034 = OpLoad %v4float %951
%1035 = OpVectorTimesScalar %v4float %1034 %1033
%1036 = OpLoad %v4float %951
%1037 = OpCompositeExtract %float %1036 3
%1038 = OpLoad %v4float %952
%1039 = OpVectorTimesScalar %v4float %1038 %1037
%1040 = OpFAdd %v4float %1035 %1039
OpReturnValue %1040
%967 = OpLabel
%1041 = OpLoad %v4float %952
%1042 = OpCompositeExtract %float %1041 3
%1043 = OpFSub %float %float_1 %1042
%1044 = OpLoad %v4float %951
%1045 = OpVectorTimesScalar %v4float %1044 %1043
%1046 = OpLoad %v4float %951
%1047 = OpCompositeExtract %float %1046 3
%1048 = OpFSub %float %float_1 %1047
%1049 = OpLoad %v4float %952
%1050 = OpVectorTimesScalar %v4float %1049 %1048
%1051 = OpFAdd %v4float %1045 %1050
OpReturnValue %1051
%968 = OpLabel
%1053 = OpLoad %v4float %951
%1054 = OpLoad %v4float %952
%1055 = OpFAdd %v4float %1053 %1054
%1056 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1052 = OpExtInst %v4float %1 FMin %1055 %1056
OpReturnValue %1052
%969 = OpLabel
%1057 = OpLoad %v4float %951
%1058 = OpLoad %v4float %952
%1059 = OpFMul %v4float %1057 %1058
OpReturnValue %1059
%970 = OpLabel
%1060 = OpLoad %v4float %951
%1061 = OpLoad %v4float %951
%1062 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1063 = OpFSub %v4float %1062 %1061
%1064 = OpLoad %v4float %952
%1065 = OpFMul %v4float %1063 %1064
%1066 = OpFAdd %v4float %1060 %1065
OpReturnValue %1066
%971 = OpLabel
%1067 = OpLoad %v4float %951
OpStore %1068 %1067
%1069 = OpLoad %v4float %952
OpStore %1070 %1069
%1071 = OpFunctionCall %v4float %blend_overlay %1068 %1070
OpReturnValue %1071
%972 = OpLabel
%1072 = OpLoad %v4float %951
OpStore %1073 %1072
%1074 = OpLoad %v4float %952
OpStore %1075 %1074
%1076 = OpFunctionCall %v4float %blend_darken %1073 %1075
OpReturnValue %1076
%973 = OpLabel
%1077 = OpLoad %v4float %951
OpStore %1078 %1077
%1079 = OpLoad %v4float %952
OpStore %1080 %1079
%1081 = OpFunctionCall %v4float %blend_lighten %1078 %1080
OpReturnValue %1081
%974 = OpLabel
%1082 = OpLoad %v4float %951
%1083 = OpVectorShuffle %v2float %1082 %1082 0 3
OpStore %1084 %1083
%1085 = OpLoad %v4float %952
%1086 = OpVectorShuffle %v2float %1085 %1085 0 3
OpStore %1087 %1086
%1088 = OpFunctionCall %float %_color_dodge_component %1084 %1087
%1089 = OpLoad %v4float %951
%1090 = OpVectorShuffle %v2float %1089 %1089 1 3
OpStore %1091 %1090
%1092 = OpLoad %v4float %952
%1093 = OpVectorShuffle %v2float %1092 %1092 1 3
OpStore %1094 %1093
%1095 = OpFunctionCall %float %_color_dodge_component %1091 %1094
%1096 = OpLoad %v4float %951
%1097 = OpVectorShuffle %v2float %1096 %1096 2 3
OpStore %1098 %1097
%1099 = OpLoad %v4float %952
%1100 = OpVectorShuffle %v2float %1099 %1099 2 3
OpStore %1101 %1100
%1102 = OpFunctionCall %float %_color_dodge_component %1098 %1101
%1103 = OpLoad %v4float %951
%1104 = OpCompositeExtract %float %1103 3
%1105 = OpLoad %v4float %951
%1106 = OpCompositeExtract %float %1105 3
%1107 = OpFSub %float %float_1 %1106
%1108 = OpLoad %v4float %952
%1109 = OpCompositeExtract %float %1108 3
%1110 = OpFMul %float %1107 %1109
%1111 = OpFAdd %float %1104 %1110
%1112 = OpCompositeConstruct %v4float %1088 %1095 %1102 %1111
OpReturnValue %1112
%975 = OpLabel
%1113 = OpLoad %v4float %951
%1114 = OpVectorShuffle %v2float %1113 %1113 0 3
OpStore %1115 %1114
%1116 = OpLoad %v4float %952
%1117 = OpVectorShuffle %v2float %1116 %1116 0 3
OpStore %1118 %1117
%1119 = OpFunctionCall %float %_color_burn_component %1115 %1118
%1120 = OpLoad %v4float %951
%1121 = OpVectorShuffle %v2float %1120 %1120 1 3
OpStore %1122 %1121
%1123 = OpLoad %v4float %952
%1124 = OpVectorShuffle %v2float %1123 %1123 1 3
OpStore %1125 %1124
%1126 = OpFunctionCall %float %_color_burn_component %1122 %1125
%1127 = OpLoad %v4float %951
%1128 = OpVectorShuffle %v2float %1127 %1127 2 3
OpStore %1129 %1128
%1130 = OpLoad %v4float %952
%1131 = OpVectorShuffle %v2float %1130 %1130 2 3
OpStore %1132 %1131
%1133 = OpFunctionCall %float %_color_burn_component %1129 %1132
%1134 = OpLoad %v4float %951
%1135 = OpCompositeExtract %float %1134 3
%1136 = OpLoad %v4float %951
%1137 = OpCompositeExtract %float %1136 3
%1138 = OpFSub %float %float_1 %1137
%1139 = OpLoad %v4float %952
%1140 = OpCompositeExtract %float %1139 3
%1141 = OpFMul %float %1138 %1140
%1142 = OpFAdd %float %1135 %1141
%1143 = OpCompositeConstruct %v4float %1119 %1126 %1133 %1142
OpReturnValue %1143
%976 = OpLabel
%1144 = OpLoad %v4float %952
OpStore %1145 %1144
%1146 = OpLoad %v4float %951
OpStore %1147 %1146
%1148 = OpFunctionCall %v4float %blend_overlay %1145 %1147
OpReturnValue %1148
%977 = OpLabel
%1149 = OpLoad %v4float %952
%1150 = OpCompositeExtract %float %1149 3
%1151 = OpFOrdEqual %bool %1150 %float_0
OpSelectionMerge %1155 None
OpBranchConditional %1151 %1153 %1154
%1153 = OpLabel
%1156 = OpLoad %v4float %951
OpStore %1152 %1156
OpBranch %1155
%1154 = OpLabel
%1157 = OpLoad %v4float %951
%1158 = OpVectorShuffle %v2float %1157 %1157 0 3
OpStore %1159 %1158
%1160 = OpLoad %v4float %952
%1161 = OpVectorShuffle %v2float %1160 %1160 0 3
OpStore %1162 %1161
%1163 = OpFunctionCall %float %_soft_light_component %1159 %1162
%1164 = OpLoad %v4float %951
%1165 = OpVectorShuffle %v2float %1164 %1164 1 3
OpStore %1166 %1165
%1167 = OpLoad %v4float %952
%1168 = OpVectorShuffle %v2float %1167 %1167 1 3
OpStore %1169 %1168
%1170 = OpFunctionCall %float %_soft_light_component %1166 %1169
%1171 = OpLoad %v4float %951
%1172 = OpVectorShuffle %v2float %1171 %1171 2 3
OpStore %1173 %1172
%1174 = OpLoad %v4float %952
%1175 = OpVectorShuffle %v2float %1174 %1174 2 3
OpStore %1176 %1175
%1177 = OpFunctionCall %float %_soft_light_component %1173 %1176
%1178 = OpLoad %v4float %951
%1179 = OpCompositeExtract %float %1178 3
%1180 = OpLoad %v4float %951
%1181 = OpCompositeExtract %float %1180 3
%1182 = OpFSub %float %float_1 %1181
%1183 = OpLoad %v4float %952
%1184 = OpCompositeExtract %float %1183 3
%1185 = OpFMul %float %1182 %1184
%1186 = OpFAdd %float %1179 %1185
%1187 = OpCompositeConstruct %v4float %1163 %1170 %1177 %1186
OpStore %1152 %1187
OpBranch %1155
%1155 = OpLabel
%1188 = OpLoad %v4float %1152
OpReturnValue %1188
%978 = OpLabel
%1189 = OpLoad %v4float %951
%1190 = OpVectorShuffle %v3float %1189 %1189 0 1 2
%1191 = OpLoad %v4float %952
%1192 = OpVectorShuffle %v3float %1191 %1191 0 1 2
%1193 = OpFAdd %v3float %1190 %1192
%1195 = OpLoad %v4float %951
%1196 = OpVectorShuffle %v3float %1195 %1195 0 1 2
%1197 = OpLoad %v4float %952
%1198 = OpCompositeExtract %float %1197 3
%1199 = OpVectorTimesScalar %v3float %1196 %1198
%1200 = OpLoad %v4float %952
%1201 = OpVectorShuffle %v3float %1200 %1200 0 1 2
%1202 = OpLoad %v4float %951
%1203 = OpCompositeExtract %float %1202 3
%1204 = OpVectorTimesScalar %v3float %1201 %1203
%1194 = OpExtInst %v3float %1 FMin %1199 %1204
%1205 = OpVectorTimesScalar %v3float %1194 %float_2
%1206 = OpFSub %v3float %1193 %1205
%1207 = OpCompositeExtract %float %1206 0
%1208 = OpCompositeExtract %float %1206 1
%1209 = OpCompositeExtract %float %1206 2
%1210 = OpLoad %v4float %951
%1211 = OpCompositeExtract %float %1210 3
%1212 = OpLoad %v4float %951
%1213 = OpCompositeExtract %float %1212 3
%1214 = OpFSub %float %float_1 %1213
%1215 = OpLoad %v4float %952
%1216 = OpCompositeExtract %float %1215 3
%1217 = OpFMul %float %1214 %1216
%1218 = OpFAdd %float %1211 %1217
%1219 = OpCompositeConstruct %v4float %1207 %1208 %1209 %1218
OpReturnValue %1219
%979 = OpLabel
%1220 = OpLoad %v4float %952
%1221 = OpVectorShuffle %v3float %1220 %1220 0 1 2
%1222 = OpLoad %v4float %951
%1223 = OpVectorShuffle %v3float %1222 %1222 0 1 2
%1224 = OpFAdd %v3float %1221 %1223
%1225 = OpLoad %v4float %952
%1226 = OpVectorShuffle %v3float %1225 %1225 0 1 2
%1227 = OpVectorTimesScalar %v3float %1226 %float_2
%1228 = OpLoad %v4float %951
%1229 = OpVectorShuffle %v3float %1228 %1228 0 1 2
%1230 = OpFMul %v3float %1227 %1229
%1231 = OpFSub %v3float %1224 %1230
%1232 = OpCompositeExtract %float %1231 0
%1233 = OpCompositeExtract %float %1231 1
%1234 = OpCompositeExtract %float %1231 2
%1235 = OpLoad %v4float %951
%1236 = OpCompositeExtract %float %1235 3
%1237 = OpLoad %v4float %951
%1238 = OpCompositeExtract %float %1237 3
%1239 = OpFSub %float %float_1 %1238
%1240 = OpLoad %v4float %952
%1241 = OpCompositeExtract %float %1240 3
%1242 = OpFMul %float %1239 %1241
%1243 = OpFAdd %float %1236 %1242
%1244 = OpCompositeConstruct %v4float %1232 %1233 %1234 %1243
OpReturnValue %1244
%980 = OpLabel
%1245 = OpLoad %v4float %951
%1246 = OpCompositeExtract %float %1245 3
%1247 = OpFSub %float %float_1 %1246
%1248 = OpLoad %v4float %952
%1249 = OpVectorShuffle %v3float %1248 %1248 0 1 2
%1250 = OpVectorTimesScalar %v3float %1249 %1247
%1251 = OpLoad %v4float %952
%1252 = OpCompositeExtract %float %1251 3
%1253 = OpFSub %float %float_1 %1252
%1254 = OpLoad %v4float %951
%1255 = OpVectorShuffle %v3float %1254 %1254 0 1 2
%1256 = OpVectorTimesScalar %v3float %1255 %1253
%1257 = OpFAdd %v3float %1250 %1256
%1258 = OpLoad %v4float %951
%1259 = OpVectorShuffle %v3float %1258 %1258 0 1 2
%1260 = OpLoad %v4float %952
%1261 = OpVectorShuffle %v3float %1260 %1260 0 1 2
%1262 = OpFMul %v3float %1259 %1261
%1263 = OpFAdd %v3float %1257 %1262
%1264 = OpCompositeExtract %float %1263 0
%1265 = OpCompositeExtract %float %1263 1
%1266 = OpCompositeExtract %float %1263 2
%1267 = OpLoad %v4float %951
%1268 = OpCompositeExtract %float %1267 3
%1269 = OpLoad %v4float %951
%1270 = OpCompositeExtract %float %1269 3
%1271 = OpFSub %float %float_1 %1270
%1272 = OpLoad %v4float %952
%1273 = OpCompositeExtract %float %1272 3
%1274 = OpFMul %float %1271 %1273
%1275 = OpFAdd %float %1268 %1274
%1276 = OpCompositeConstruct %v4float %1264 %1265 %1266 %1275
OpReturnValue %1276
%981 = OpLabel
%1277 = OpLoad %v4float %951
OpStore %1278 %1277
%1279 = OpLoad %v4float %952
OpStore %1280 %1279
%1281 = OpFunctionCall %v4float %blend_hue %1278 %1280
OpReturnValue %1281
%982 = OpLabel
%1282 = OpLoad %v4float %951
OpStore %1283 %1282
%1284 = OpLoad %v4float %952
OpStore %1285 %1284
%1286 = OpFunctionCall %v4float %blend_saturation %1283 %1285
OpReturnValue %1286
%983 = OpLabel
%1287 = OpLoad %v4float %951
OpStore %1288 %1287
%1289 = OpLoad %v4float %952
OpStore %1290 %1289
%1291 = OpFunctionCall %v4float %blend_color %1288 %1290
OpReturnValue %1291
%984 = OpLabel
%1292 = OpLoad %v4float %951
OpStore %1293 %1292
%1294 = OpLoad %v4float %952
OpStore %1295 %1294
%1296 = OpFunctionCall %v4float %blend_luminosity %1293 %1295
OpReturnValue %1296
%985 = OpLabel
OpReturnValue %986
%955 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %1298
%1299 = OpLabel
%1301 = OpVariable %_ptr_Function_int Function
%1303 = OpVariable %_ptr_Function_v4float Function
%1305 = OpVariable %_ptr_Function_v4float Function
OpStore %1301 %int_13
%1302 = OpLoad %v4float %src
OpStore %1303 %1302
%1304 = OpLoad %v4float %dst
OpStore %1305 %1304
%1306 = OpFunctionCall %v4float %blend %1301 %1303 %1305
OpStore %sk_FragColor %1306
OpReturn
OpFunctionEnd

1 error
