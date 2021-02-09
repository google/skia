OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_clear "blend_clear"
OpName %blend_src "blend_src"
OpName %blend_dst "blend_dst"
OpName %blend_src_over "blend_src_over"
OpName %blend_dst_over "blend_dst_over"
OpName %blend_src_in "blend_src_in"
OpName %blend_dst_in "blend_dst_in"
OpName %blend_src_out "blend_src_out"
OpName %blend_dst_out "blend_dst_out"
OpName %blend_src_atop "blend_src_atop"
OpName %blend_dst_atop "blend_dst_atop"
OpName %blend_xor "blend_xor"
OpName %blend_plus "blend_plus"
OpName %blend_modulate "blend_modulate"
OpName %blend_screen "blend_screen"
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
OpName %_4_n "_4_n"
OpName %blend_color_dodge "blend_color_dodge"
OpName %_color_burn_component "_color_burn_component"
OpName %_6_n "_6_n"
OpName %delta_0 "delta"
OpName %blend_color_burn "blend_color_burn"
OpName %blend_hard_light "blend_hard_light"
OpName %_soft_light_component "_soft_light_component"
OpName %_8_n "_8_n"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_10_n "_10_n"
OpName %blend_soft_light "blend_soft_light"
OpName %blend_difference "blend_difference"
OpName %blend_exclusion "blend_exclusion"
OpName %blend_multiply "blend_multiply"
OpName %_blend_color_luminance "_blend_color_luminance"
OpName %_blend_set_color_luminance "_blend_set_color_luminance"
OpName %lum "lum"
OpName %result_2 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_color_saturation "_blend_color_saturation"
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
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %510 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %540 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %559 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %584 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %597 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
OpDecorate %608 RelaxedPrecision
OpDecorate %607 RelaxedPrecision
OpDecorate %610 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %613 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %615 RelaxedPrecision
OpDecorate %616 RelaxedPrecision
OpDecorate %618 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %639 RelaxedPrecision
OpDecorate %641 RelaxedPrecision
OpDecorate %643 RelaxedPrecision
OpDecorate %645 RelaxedPrecision
OpDecorate %646 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %650 RelaxedPrecision
OpDecorate %651 RelaxedPrecision
OpDecorate %653 RelaxedPrecision
OpDecorate %656 RelaxedPrecision
OpDecorate %658 RelaxedPrecision
OpDecorate %660 RelaxedPrecision
OpDecorate %662 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %664 RelaxedPrecision
OpDecorate %665 RelaxedPrecision
OpDecorate %666 RelaxedPrecision
OpDecorate %667 RelaxedPrecision
OpDecorate %669 RelaxedPrecision
OpDecorate %671 RelaxedPrecision
OpDecorate %672 RelaxedPrecision
OpDecorate %673 RelaxedPrecision
OpDecorate %674 RelaxedPrecision
OpDecorate %676 RelaxedPrecision
OpDecorate %678 RelaxedPrecision
OpDecorate %679 RelaxedPrecision
OpDecorate %680 RelaxedPrecision
OpDecorate %681 RelaxedPrecision
OpDecorate %683 RelaxedPrecision
OpDecorate %684 RelaxedPrecision
OpDecorate %685 RelaxedPrecision
OpDecorate %687 RelaxedPrecision
OpDecorate %689 RelaxedPrecision
OpDecorate %690 RelaxedPrecision
OpDecorate %691 RelaxedPrecision
OpDecorate %692 RelaxedPrecision
OpDecorate %693 RelaxedPrecision
OpDecorate %694 RelaxedPrecision
OpDecorate %696 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
OpDecorate %699 RelaxedPrecision
OpDecorate %700 RelaxedPrecision
OpDecorate %701 RelaxedPrecision
OpDecorate %703 RelaxedPrecision
OpDecorate %705 RelaxedPrecision
OpDecorate %707 RelaxedPrecision
OpDecorate %708 RelaxedPrecision
OpDecorate %709 RelaxedPrecision
OpDecorate %710 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %713 RelaxedPrecision
OpDecorate %715 RelaxedPrecision
OpDecorate %717 RelaxedPrecision
OpDecorate %719 RelaxedPrecision
OpDecorate %720 RelaxedPrecision
OpDecorate %722 RelaxedPrecision
OpDecorate %724 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %726 RelaxedPrecision
OpDecorate %727 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %732 RelaxedPrecision
OpDecorate %733 RelaxedPrecision
OpDecorate %737 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %745 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %770 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %774 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %780 RelaxedPrecision
OpDecorate %782 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %791 RelaxedPrecision
OpDecorate %793 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %805 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %829 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %833 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %850 RelaxedPrecision
OpDecorate %851 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %855 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %860 RelaxedPrecision
OpDecorate %864 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %871 RelaxedPrecision
OpDecorate %872 RelaxedPrecision
OpDecorate %883 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %893 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %897 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %914 RelaxedPrecision
OpDecorate %916 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %924 RelaxedPrecision
OpDecorate %929 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %931 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
OpDecorate %937 RelaxedPrecision
OpDecorate %938 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %944 RelaxedPrecision
OpDecorate %948 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %956 RelaxedPrecision
OpDecorate %957 RelaxedPrecision
OpDecorate %958 RelaxedPrecision
OpDecorate %961 RelaxedPrecision
OpDecorate %962 RelaxedPrecision
OpDecorate %963 RelaxedPrecision
OpDecorate %965 RelaxedPrecision
OpDecorate %966 RelaxedPrecision
OpDecorate %967 RelaxedPrecision
OpDecorate %972 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %978 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %986 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %992 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1007 RelaxedPrecision
OpDecorate %1009 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1013 RelaxedPrecision
OpDecorate %1015 RelaxedPrecision
OpDecorate %1017 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1022 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1044 RelaxedPrecision
OpDecorate %1045 RelaxedPrecision
OpDecorate %1047 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1084 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1114 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1121 RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1135 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1141 RelaxedPrecision
OpDecorate %1143 RelaxedPrecision
OpDecorate %1146 RelaxedPrecision
OpDecorate %1148 RelaxedPrecision
OpDecorate %1152 RelaxedPrecision
OpDecorate %1154 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1159 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1161 RelaxedPrecision
OpDecorate %1162 RelaxedPrecision
OpDecorate %1164 RelaxedPrecision
OpDecorate %1165 RelaxedPrecision
OpDecorate %1166 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1176 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1194 RelaxedPrecision
OpDecorate %1196 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1205 RelaxedPrecision
OpDecorate %1207 RelaxedPrecision
OpDecorate %1210 RelaxedPrecision
OpDecorate %1212 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1214 RelaxedPrecision
OpDecorate %1215 RelaxedPrecision
OpDecorate %1217 RelaxedPrecision
OpDecorate %1218 RelaxedPrecision
OpDecorate %1219 RelaxedPrecision
OpDecorate %1223 RelaxedPrecision
OpDecorate %1225 RelaxedPrecision
OpDecorate %1227 RelaxedPrecision
OpDecorate %1228 RelaxedPrecision
OpDecorate %1229 RelaxedPrecision
OpDecorate %1235 RelaxedPrecision
OpDecorate %1237 RelaxedPrecision
OpDecorate %1239 RelaxedPrecision
OpDecorate %1241 RelaxedPrecision
OpDecorate %1243 RelaxedPrecision
OpDecorate %1247 RelaxedPrecision
OpDecorate %1249 RelaxedPrecision
OpDecorate %1252 RelaxedPrecision
OpDecorate %1254 RelaxedPrecision
OpDecorate %1256 RelaxedPrecision
OpDecorate %1259 RelaxedPrecision
OpDecorate %1261 RelaxedPrecision
OpDecorate %1262 RelaxedPrecision
OpDecorate %1263 RelaxedPrecision
OpDecorate %1264 RelaxedPrecision
OpDecorate %1266 RelaxedPrecision
OpDecorate %1267 RelaxedPrecision
OpDecorate %1268 RelaxedPrecision
OpDecorate %1272 RelaxedPrecision
OpDecorate %1274 RelaxedPrecision
OpDecorate %1276 RelaxedPrecision
OpDecorate %1277 RelaxedPrecision
OpDecorate %1278 RelaxedPrecision
OpDecorate %1284 RelaxedPrecision
OpDecorate %1286 RelaxedPrecision
OpDecorate %1288 RelaxedPrecision
OpDecorate %1290 RelaxedPrecision
OpDecorate %1292 RelaxedPrecision
OpDecorate %1296 RelaxedPrecision
OpDecorate %1298 RelaxedPrecision
OpDecorate %1301 RelaxedPrecision
OpDecorate %1303 RelaxedPrecision
OpDecorate %1305 RelaxedPrecision
OpDecorate %1308 RelaxedPrecision
OpDecorate %1310 RelaxedPrecision
OpDecorate %1311 RelaxedPrecision
OpDecorate %1312 RelaxedPrecision
OpDecorate %1313 RelaxedPrecision
OpDecorate %1315 RelaxedPrecision
OpDecorate %1316 RelaxedPrecision
OpDecorate %1317 RelaxedPrecision
OpDecorate %1321 RelaxedPrecision
OpDecorate %1323 RelaxedPrecision
OpDecorate %1325 RelaxedPrecision
OpDecorate %1326 RelaxedPrecision
OpDecorate %1327 RelaxedPrecision
OpDecorate %1336 RelaxedPrecision
OpDecorate %1367 RelaxedPrecision
OpDecorate %1368 RelaxedPrecision
OpDecorate %1369 RelaxedPrecision
OpDecorate %1370 RelaxedPrecision
OpDecorate %1372 RelaxedPrecision
OpDecorate %1373 RelaxedPrecision
OpDecorate %1375 RelaxedPrecision
OpDecorate %1376 RelaxedPrecision
OpDecorate %1378 RelaxedPrecision
OpDecorate %1379 RelaxedPrecision
OpDecorate %1381 RelaxedPrecision
OpDecorate %1382 RelaxedPrecision
OpDecorate %1383 RelaxedPrecision
OpDecorate %1384 RelaxedPrecision
OpDecorate %1387 RelaxedPrecision
OpDecorate %1388 RelaxedPrecision
OpDecorate %1391 RelaxedPrecision
OpDecorate %1393 RelaxedPrecision
OpDecorate %1394 RelaxedPrecision
OpDecorate %1396 RelaxedPrecision
OpDecorate %1398 RelaxedPrecision
OpDecorate %1399 RelaxedPrecision
OpDecorate %1401 RelaxedPrecision
OpDecorate %1403 RelaxedPrecision
OpDecorate %1405 RelaxedPrecision
OpDecorate %1407 RelaxedPrecision
OpDecorate %1408 RelaxedPrecision
OpDecorate %1410 RelaxedPrecision
OpDecorate %1411 RelaxedPrecision
OpDecorate %1413 RelaxedPrecision
OpDecorate %1414 RelaxedPrecision
OpDecorate %1416 RelaxedPrecision
OpDecorate %1418 RelaxedPrecision
OpDecorate %1420 RelaxedPrecision
OpDecorate %1421 RelaxedPrecision
OpDecorate %1423 RelaxedPrecision
OpDecorate %1424 RelaxedPrecision
OpDecorate %1426 RelaxedPrecision
OpDecorate %1428 RelaxedPrecision
OpDecorate %1429 RelaxedPrecision
OpDecorate %1431 RelaxedPrecision
OpDecorate %1433 RelaxedPrecision
OpDecorate %1434 RelaxedPrecision
OpDecorate %1435 RelaxedPrecision
OpDecorate %1436 RelaxedPrecision
OpDecorate %1437 RelaxedPrecision
OpDecorate %1438 RelaxedPrecision
OpDecorate %1439 RelaxedPrecision
OpDecorate %1440 RelaxedPrecision
OpDecorate %1441 RelaxedPrecision
OpDecorate %1444 RelaxedPrecision
OpDecorate %1445 RelaxedPrecision
OpDecorate %1446 RelaxedPrecision
OpDecorate %1447 RelaxedPrecision
OpDecorate %1449 RelaxedPrecision
OpDecorate %1453 RelaxedPrecision
OpDecorate %1454 RelaxedPrecision
OpDecorate %1456 RelaxedPrecision
OpDecorate %1457 RelaxedPrecision
OpDecorate %1459 RelaxedPrecision
OpDecorate %1461 RelaxedPrecision
OpDecorate %1463 RelaxedPrecision
OpDecorate %1465 RelaxedPrecision
OpDecorate %1466 RelaxedPrecision
OpDecorate %1469 RelaxedPrecision
OpDecorate %1471 RelaxedPrecision
OpDecorate %1473 RelaxedPrecision
OpDecorate %1474 RelaxedPrecision
OpDecorate %1476 RelaxedPrecision
OpDecorate %1477 RelaxedPrecision
OpDecorate %1479 RelaxedPrecision
OpDecorate %1480 RelaxedPrecision
OpDecorate %1482 RelaxedPrecision
OpDecorate %1484 RelaxedPrecision
OpDecorate %1486 RelaxedPrecision
OpDecorate %1488 RelaxedPrecision
OpDecorate %1489 RelaxedPrecision
OpDecorate %1492 RelaxedPrecision
OpDecorate %1494 RelaxedPrecision
OpDecorate %1496 RelaxedPrecision
OpDecorate %1497 RelaxedPrecision
OpDecorate %1498 RelaxedPrecision
OpDecorate %1501 RelaxedPrecision
OpDecorate %1505 RelaxedPrecision
OpDecorate %1508 RelaxedPrecision
OpDecorate %1512 RelaxedPrecision
OpDecorate %1515 RelaxedPrecision
OpDecorate %1519 RelaxedPrecision
OpDecorate %1521 RelaxedPrecision
OpDecorate %1523 RelaxedPrecision
OpDecorate %1524 RelaxedPrecision
OpDecorate %1526 RelaxedPrecision
OpDecorate %1527 RelaxedPrecision
OpDecorate %1529 RelaxedPrecision
OpDecorate %1532 RelaxedPrecision
OpDecorate %1536 RelaxedPrecision
OpDecorate %1539 RelaxedPrecision
OpDecorate %1543 RelaxedPrecision
OpDecorate %1546 RelaxedPrecision
OpDecorate %1550 RelaxedPrecision
OpDecorate %1552 RelaxedPrecision
OpDecorate %1554 RelaxedPrecision
OpDecorate %1555 RelaxedPrecision
OpDecorate %1557 RelaxedPrecision
OpDecorate %1558 RelaxedPrecision
OpDecorate %1560 RelaxedPrecision
OpDecorate %1562 RelaxedPrecision
OpDecorate %1565 RelaxedPrecision
OpDecorate %1572 RelaxedPrecision
OpDecorate %1573 RelaxedPrecision
OpDecorate %1576 RelaxedPrecision
OpDecorate %1580 RelaxedPrecision
OpDecorate %1583 RelaxedPrecision
OpDecorate %1587 RelaxedPrecision
OpDecorate %1590 RelaxedPrecision
OpDecorate %1594 RelaxedPrecision
OpDecorate %1596 RelaxedPrecision
OpDecorate %1598 RelaxedPrecision
OpDecorate %1599 RelaxedPrecision
OpDecorate %1601 RelaxedPrecision
OpDecorate %1602 RelaxedPrecision
OpDecorate %1604 RelaxedPrecision
OpDecorate %1605 RelaxedPrecision
OpDecorate %1607 RelaxedPrecision
OpDecorate %1609 RelaxedPrecision
OpDecorate %1611 RelaxedPrecision
OpDecorate %1613 RelaxedPrecision
OpDecorate %1616 RelaxedPrecision
OpDecorate %1618 RelaxedPrecision
OpDecorate %1622 RelaxedPrecision
OpDecorate %1626 RelaxedPrecision
OpDecorate %1628 RelaxedPrecision
OpDecorate %1630 RelaxedPrecision
OpDecorate %1631 RelaxedPrecision
OpDecorate %1633 RelaxedPrecision
OpDecorate %1634 RelaxedPrecision
OpDecorate %1636 RelaxedPrecision
OpDecorate %1638 RelaxedPrecision
OpDecorate %1640 RelaxedPrecision
OpDecorate %1641 RelaxedPrecision
OpDecorate %1644 RelaxedPrecision
OpDecorate %1646 RelaxedPrecision
OpDecorate %1647 RelaxedPrecision
OpDecorate %1651 RelaxedPrecision
OpDecorate %1653 RelaxedPrecision
OpDecorate %1655 RelaxedPrecision
OpDecorate %1656 RelaxedPrecision
OpDecorate %1658 RelaxedPrecision
OpDecorate %1659 RelaxedPrecision
OpDecorate %1661 RelaxedPrecision
OpDecorate %1663 RelaxedPrecision
OpDecorate %1664 RelaxedPrecision
OpDecorate %1667 RelaxedPrecision
OpDecorate %1669 RelaxedPrecision
OpDecorate %1670 RelaxedPrecision
OpDecorate %1673 RelaxedPrecision
OpDecorate %1674 RelaxedPrecision
OpDecorate %1676 RelaxedPrecision
OpDecorate %1678 RelaxedPrecision
OpDecorate %1679 RelaxedPrecision
OpDecorate %1683 RelaxedPrecision
OpDecorate %1685 RelaxedPrecision
OpDecorate %1687 RelaxedPrecision
OpDecorate %1688 RelaxedPrecision
OpDecorate %1690 RelaxedPrecision
OpDecorate %1691 RelaxedPrecision
OpDecorate %1694 RelaxedPrecision
OpDecorate %1696 RelaxedPrecision
OpDecorate %1698 RelaxedPrecision
OpDecorate %1700 RelaxedPrecision
OpDecorate %1702 RelaxedPrecision
OpDecorate %1706 RelaxedPrecision
OpDecorate %1708 RelaxedPrecision
OpDecorate %1711 RelaxedPrecision
OpDecorate %1713 RelaxedPrecision
OpDecorate %1717 RelaxedPrecision
OpDecorate %1719 RelaxedPrecision
OpDecorate %1722 RelaxedPrecision
OpDecorate %1724 RelaxedPrecision
OpDecorate %1725 RelaxedPrecision
OpDecorate %1726 RelaxedPrecision
OpDecorate %1727 RelaxedPrecision
OpDecorate %1729 RelaxedPrecision
OpDecorate %1730 RelaxedPrecision
OpDecorate %1731 RelaxedPrecision
OpDecorate %1735 RelaxedPrecision
OpDecorate %1737 RelaxedPrecision
OpDecorate %1739 RelaxedPrecision
OpDecorate %1740 RelaxedPrecision
OpDecorate %1741 RelaxedPrecision
OpDecorate %1744 RelaxedPrecision
OpDecorate %1746 RelaxedPrecision
OpDecorate %1748 RelaxedPrecision
OpDecorate %1750 RelaxedPrecision
OpDecorate %1752 RelaxedPrecision
OpDecorate %1756 RelaxedPrecision
OpDecorate %1758 RelaxedPrecision
OpDecorate %1761 RelaxedPrecision
OpDecorate %1763 RelaxedPrecision
OpDecorate %1767 RelaxedPrecision
OpDecorate %1769 RelaxedPrecision
OpDecorate %1772 RelaxedPrecision
OpDecorate %1774 RelaxedPrecision
OpDecorate %1775 RelaxedPrecision
OpDecorate %1776 RelaxedPrecision
OpDecorate %1777 RelaxedPrecision
OpDecorate %1779 RelaxedPrecision
OpDecorate %1780 RelaxedPrecision
OpDecorate %1781 RelaxedPrecision
OpDecorate %1785 RelaxedPrecision
OpDecorate %1787 RelaxedPrecision
OpDecorate %1789 RelaxedPrecision
OpDecorate %1790 RelaxedPrecision
OpDecorate %1791 RelaxedPrecision
OpDecorate %1794 RelaxedPrecision
OpDecorate %1796 RelaxedPrecision
OpDecorate %1798 RelaxedPrecision
OpDecorate %1800 RelaxedPrecision
OpDecorate %1802 RelaxedPrecision
OpDecorate %1806 RelaxedPrecision
OpDecorate %1808 RelaxedPrecision
OpDecorate %1811 RelaxedPrecision
OpDecorate %1813 RelaxedPrecision
OpDecorate %1815 RelaxedPrecision
OpDecorate %1818 RelaxedPrecision
OpDecorate %1820 RelaxedPrecision
OpDecorate %1821 RelaxedPrecision
OpDecorate %1822 RelaxedPrecision
OpDecorate %1823 RelaxedPrecision
OpDecorate %1825 RelaxedPrecision
OpDecorate %1826 RelaxedPrecision
OpDecorate %1827 RelaxedPrecision
OpDecorate %1831 RelaxedPrecision
OpDecorate %1833 RelaxedPrecision
OpDecorate %1835 RelaxedPrecision
OpDecorate %1836 RelaxedPrecision
OpDecorate %1837 RelaxedPrecision
OpDecorate %1840 RelaxedPrecision
OpDecorate %1842 RelaxedPrecision
OpDecorate %1844 RelaxedPrecision
OpDecorate %1846 RelaxedPrecision
OpDecorate %1848 RelaxedPrecision
OpDecorate %1852 RelaxedPrecision
OpDecorate %1854 RelaxedPrecision
OpDecorate %1857 RelaxedPrecision
OpDecorate %1859 RelaxedPrecision
OpDecorate %1861 RelaxedPrecision
OpDecorate %1864 RelaxedPrecision
OpDecorate %1866 RelaxedPrecision
OpDecorate %1867 RelaxedPrecision
OpDecorate %1868 RelaxedPrecision
OpDecorate %1869 RelaxedPrecision
OpDecorate %1871 RelaxedPrecision
OpDecorate %1872 RelaxedPrecision
OpDecorate %1873 RelaxedPrecision
OpDecorate %1877 RelaxedPrecision
OpDecorate %1879 RelaxedPrecision
OpDecorate %1881 RelaxedPrecision
OpDecorate %1882 RelaxedPrecision
OpDecorate %1883 RelaxedPrecision
OpDecorate %1890 RelaxedPrecision
OpDecorate %1892 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%53 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%59 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%184 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%333 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%float_4 = OpConstant %float 4
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%_ptr_Function_v3float = OpTypePointer Function %v3float
%874 = OpTypeFunction %float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%882 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%884 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%false = OpConstantFalse %bool
%993 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%1021 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%1023 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%1330 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%1886 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%blend_clear = OpFunction %v4float None %53
%55 = OpFunctionParameter %_ptr_Function_v4float
%56 = OpFunctionParameter %_ptr_Function_v4float
%57 = OpLabel
OpReturnValue %59
OpFunctionEnd
%blend_src = OpFunction %v4float None %53
%60 = OpFunctionParameter %_ptr_Function_v4float
%61 = OpFunctionParameter %_ptr_Function_v4float
%62 = OpLabel
%63 = OpLoad %v4float %60
OpReturnValue %63
OpFunctionEnd
%blend_dst = OpFunction %v4float None %53
%64 = OpFunctionParameter %_ptr_Function_v4float
%65 = OpFunctionParameter %_ptr_Function_v4float
%66 = OpLabel
%67 = OpLoad %v4float %65
OpReturnValue %67
OpFunctionEnd
%blend_src_over = OpFunction %v4float None %53
%68 = OpFunctionParameter %_ptr_Function_v4float
%69 = OpFunctionParameter %_ptr_Function_v4float
%70 = OpLabel
%71 = OpLoad %v4float %68
%73 = OpLoad %v4float %68
%74 = OpCompositeExtract %float %73 3
%75 = OpFSub %float %float_1 %74
%76 = OpLoad %v4float %69
%77 = OpVectorTimesScalar %v4float %76 %75
%78 = OpFAdd %v4float %71 %77
OpReturnValue %78
OpFunctionEnd
%blend_dst_over = OpFunction %v4float None %53
%79 = OpFunctionParameter %_ptr_Function_v4float
%80 = OpFunctionParameter %_ptr_Function_v4float
%81 = OpLabel
%82 = OpLoad %v4float %80
%83 = OpCompositeExtract %float %82 3
%84 = OpFSub %float %float_1 %83
%85 = OpLoad %v4float %79
%86 = OpVectorTimesScalar %v4float %85 %84
%87 = OpLoad %v4float %80
%88 = OpFAdd %v4float %86 %87
OpReturnValue %88
OpFunctionEnd
%blend_src_in = OpFunction %v4float None %53
%89 = OpFunctionParameter %_ptr_Function_v4float
%90 = OpFunctionParameter %_ptr_Function_v4float
%91 = OpLabel
%92 = OpLoad %v4float %89
%93 = OpLoad %v4float %90
%94 = OpCompositeExtract %float %93 3
%95 = OpVectorTimesScalar %v4float %92 %94
OpReturnValue %95
OpFunctionEnd
%blend_dst_in = OpFunction %v4float None %53
%96 = OpFunctionParameter %_ptr_Function_v4float
%97 = OpFunctionParameter %_ptr_Function_v4float
%98 = OpLabel
%99 = OpLoad %v4float %97
%100 = OpLoad %v4float %96
%101 = OpCompositeExtract %float %100 3
%102 = OpVectorTimesScalar %v4float %99 %101
OpReturnValue %102
OpFunctionEnd
%blend_src_out = OpFunction %v4float None %53
%103 = OpFunctionParameter %_ptr_Function_v4float
%104 = OpFunctionParameter %_ptr_Function_v4float
%105 = OpLabel
%106 = OpLoad %v4float %104
%107 = OpCompositeExtract %float %106 3
%108 = OpFSub %float %float_1 %107
%109 = OpLoad %v4float %103
%110 = OpVectorTimesScalar %v4float %109 %108
OpReturnValue %110
OpFunctionEnd
%blend_dst_out = OpFunction %v4float None %53
%111 = OpFunctionParameter %_ptr_Function_v4float
%112 = OpFunctionParameter %_ptr_Function_v4float
%113 = OpLabel
%114 = OpLoad %v4float %111
%115 = OpCompositeExtract %float %114 3
%116 = OpFSub %float %float_1 %115
%117 = OpLoad %v4float %112
%118 = OpVectorTimesScalar %v4float %117 %116
OpReturnValue %118
OpFunctionEnd
%blend_src_atop = OpFunction %v4float None %53
%119 = OpFunctionParameter %_ptr_Function_v4float
%120 = OpFunctionParameter %_ptr_Function_v4float
%121 = OpLabel
%122 = OpLoad %v4float %120
%123 = OpCompositeExtract %float %122 3
%124 = OpLoad %v4float %119
%125 = OpVectorTimesScalar %v4float %124 %123
%126 = OpLoad %v4float %119
%127 = OpCompositeExtract %float %126 3
%128 = OpFSub %float %float_1 %127
%129 = OpLoad %v4float %120
%130 = OpVectorTimesScalar %v4float %129 %128
%131 = OpFAdd %v4float %125 %130
OpReturnValue %131
OpFunctionEnd
%blend_dst_atop = OpFunction %v4float None %53
%132 = OpFunctionParameter %_ptr_Function_v4float
%133 = OpFunctionParameter %_ptr_Function_v4float
%134 = OpLabel
%135 = OpLoad %v4float %133
%136 = OpCompositeExtract %float %135 3
%137 = OpFSub %float %float_1 %136
%138 = OpLoad %v4float %132
%139 = OpVectorTimesScalar %v4float %138 %137
%140 = OpLoad %v4float %132
%141 = OpCompositeExtract %float %140 3
%142 = OpLoad %v4float %133
%143 = OpVectorTimesScalar %v4float %142 %141
%144 = OpFAdd %v4float %139 %143
OpReturnValue %144
OpFunctionEnd
%blend_xor = OpFunction %v4float None %53
%145 = OpFunctionParameter %_ptr_Function_v4float
%146 = OpFunctionParameter %_ptr_Function_v4float
%147 = OpLabel
%148 = OpLoad %v4float %146
%149 = OpCompositeExtract %float %148 3
%150 = OpFSub %float %float_1 %149
%151 = OpLoad %v4float %145
%152 = OpVectorTimesScalar %v4float %151 %150
%153 = OpLoad %v4float %145
%154 = OpCompositeExtract %float %153 3
%155 = OpFSub %float %float_1 %154
%156 = OpLoad %v4float %146
%157 = OpVectorTimesScalar %v4float %156 %155
%158 = OpFAdd %v4float %152 %157
OpReturnValue %158
OpFunctionEnd
%blend_plus = OpFunction %v4float None %53
%159 = OpFunctionParameter %_ptr_Function_v4float
%160 = OpFunctionParameter %_ptr_Function_v4float
%161 = OpLabel
%163 = OpLoad %v4float %159
%164 = OpLoad %v4float %160
%165 = OpFAdd %v4float %163 %164
%166 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%162 = OpExtInst %v4float %1 FMin %165 %166
OpReturnValue %162
OpFunctionEnd
%blend_modulate = OpFunction %v4float None %53
%167 = OpFunctionParameter %_ptr_Function_v4float
%168 = OpFunctionParameter %_ptr_Function_v4float
%169 = OpLabel
%170 = OpLoad %v4float %167
%171 = OpLoad %v4float %168
%172 = OpFMul %v4float %170 %171
OpReturnValue %172
OpFunctionEnd
%blend_screen = OpFunction %v4float None %53
%173 = OpFunctionParameter %_ptr_Function_v4float
%174 = OpFunctionParameter %_ptr_Function_v4float
%175 = OpLabel
%176 = OpLoad %v4float %173
%177 = OpLoad %v4float %173
%178 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%179 = OpFSub %v4float %178 %177
%180 = OpLoad %v4float %174
%181 = OpFMul %v4float %179 %180
%182 = OpFAdd %v4float %176 %181
OpReturnValue %182
OpFunctionEnd
%_blend_overlay_component = OpFunction %float None %184
%186 = OpFunctionParameter %_ptr_Function_v2float
%187 = OpFunctionParameter %_ptr_Function_v2float
%188 = OpLabel
%196 = OpVariable %_ptr_Function_float Function
%190 = OpLoad %v2float %187
%191 = OpCompositeExtract %float %190 0
%192 = OpFMul %float %float_2 %191
%193 = OpLoad %v2float %187
%194 = OpCompositeExtract %float %193 1
%195 = OpFOrdLessThanEqual %bool %192 %194
OpSelectionMerge %200 None
OpBranchConditional %195 %198 %199
%198 = OpLabel
%201 = OpLoad %v2float %186
%202 = OpCompositeExtract %float %201 0
%203 = OpFMul %float %float_2 %202
%204 = OpLoad %v2float %187
%205 = OpCompositeExtract %float %204 0
%206 = OpFMul %float %203 %205
OpStore %196 %206
OpBranch %200
%199 = OpLabel
%207 = OpLoad %v2float %186
%208 = OpCompositeExtract %float %207 1
%209 = OpLoad %v2float %187
%210 = OpCompositeExtract %float %209 1
%211 = OpFMul %float %208 %210
%212 = OpLoad %v2float %187
%213 = OpCompositeExtract %float %212 1
%214 = OpLoad %v2float %187
%215 = OpCompositeExtract %float %214 0
%216 = OpFSub %float %213 %215
%217 = OpFMul %float %float_2 %216
%218 = OpLoad %v2float %186
%219 = OpCompositeExtract %float %218 1
%220 = OpLoad %v2float %186
%221 = OpCompositeExtract %float %220 0
%222 = OpFSub %float %219 %221
%223 = OpFMul %float %217 %222
%224 = OpFSub %float %211 %223
OpStore %196 %224
OpBranch %200
%200 = OpLabel
%225 = OpLoad %float %196
OpReturnValue %225
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %53
%226 = OpFunctionParameter %_ptr_Function_v4float
%227 = OpFunctionParameter %_ptr_Function_v4float
%228 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%232 = OpVariable %_ptr_Function_v2float Function
%235 = OpVariable %_ptr_Function_v2float Function
%239 = OpVariable %_ptr_Function_v2float Function
%242 = OpVariable %_ptr_Function_v2float Function
%246 = OpVariable %_ptr_Function_v2float Function
%249 = OpVariable %_ptr_Function_v2float Function
%230 = OpLoad %v4float %226
%231 = OpVectorShuffle %v2float %230 %230 0 3
OpStore %232 %231
%233 = OpLoad %v4float %227
%234 = OpVectorShuffle %v2float %233 %233 0 3
OpStore %235 %234
%236 = OpFunctionCall %float %_blend_overlay_component %232 %235
%237 = OpLoad %v4float %226
%238 = OpVectorShuffle %v2float %237 %237 1 3
OpStore %239 %238
%240 = OpLoad %v4float %227
%241 = OpVectorShuffle %v2float %240 %240 1 3
OpStore %242 %241
%243 = OpFunctionCall %float %_blend_overlay_component %239 %242
%244 = OpLoad %v4float %226
%245 = OpVectorShuffle %v2float %244 %244 2 3
OpStore %246 %245
%247 = OpLoad %v4float %227
%248 = OpVectorShuffle %v2float %247 %247 2 3
OpStore %249 %248
%250 = OpFunctionCall %float %_blend_overlay_component %246 %249
%251 = OpLoad %v4float %226
%252 = OpCompositeExtract %float %251 3
%253 = OpLoad %v4float %226
%254 = OpCompositeExtract %float %253 3
%255 = OpFSub %float %float_1 %254
%256 = OpLoad %v4float %227
%257 = OpCompositeExtract %float %256 3
%258 = OpFMul %float %255 %257
%259 = OpFAdd %float %252 %258
%260 = OpCompositeConstruct %v4float %236 %243 %250 %259
OpStore %result %260
%261 = OpLoad %v4float %result
%262 = OpVectorShuffle %v3float %261 %261 0 1 2
%264 = OpLoad %v4float %227
%265 = OpVectorShuffle %v3float %264 %264 0 1 2
%266 = OpLoad %v4float %226
%267 = OpCompositeExtract %float %266 3
%268 = OpFSub %float %float_1 %267
%269 = OpVectorTimesScalar %v3float %265 %268
%270 = OpLoad %v4float %226
%271 = OpVectorShuffle %v3float %270 %270 0 1 2
%272 = OpLoad %v4float %227
%273 = OpCompositeExtract %float %272 3
%274 = OpFSub %float %float_1 %273
%275 = OpVectorTimesScalar %v3float %271 %274
%276 = OpFAdd %v3float %269 %275
%277 = OpFAdd %v3float %262 %276
%278 = OpLoad %v4float %result
%279 = OpVectorShuffle %v4float %278 %277 4 5 6 3
OpStore %result %279
%280 = OpLoad %v4float %result
OpReturnValue %280
OpFunctionEnd
%blend_darken = OpFunction %v4float None %53
%281 = OpFunctionParameter %_ptr_Function_v4float
%282 = OpFunctionParameter %_ptr_Function_v4float
%283 = OpLabel
%result_0 = OpVariable %_ptr_Function_v4float Function
%285 = OpLoad %v4float %281
%286 = OpLoad %v4float %281
%287 = OpCompositeExtract %float %286 3
%288 = OpFSub %float %float_1 %287
%289 = OpLoad %v4float %282
%290 = OpVectorTimesScalar %v4float %289 %288
%291 = OpFAdd %v4float %285 %290
OpStore %result_0 %291
%293 = OpLoad %v4float %result_0
%294 = OpVectorShuffle %v3float %293 %293 0 1 2
%295 = OpLoad %v4float %282
%296 = OpCompositeExtract %float %295 3
%297 = OpFSub %float %float_1 %296
%298 = OpLoad %v4float %281
%299 = OpVectorShuffle %v3float %298 %298 0 1 2
%300 = OpVectorTimesScalar %v3float %299 %297
%301 = OpLoad %v4float %282
%302 = OpVectorShuffle %v3float %301 %301 0 1 2
%303 = OpFAdd %v3float %300 %302
%292 = OpExtInst %v3float %1 FMin %294 %303
%304 = OpLoad %v4float %result_0
%305 = OpVectorShuffle %v4float %304 %292 4 5 6 3
OpStore %result_0 %305
%306 = OpLoad %v4float %result_0
OpReturnValue %306
OpFunctionEnd
%blend_lighten = OpFunction %v4float None %53
%307 = OpFunctionParameter %_ptr_Function_v4float
%308 = OpFunctionParameter %_ptr_Function_v4float
%309 = OpLabel
%result_1 = OpVariable %_ptr_Function_v4float Function
%311 = OpLoad %v4float %307
%312 = OpLoad %v4float %307
%313 = OpCompositeExtract %float %312 3
%314 = OpFSub %float %float_1 %313
%315 = OpLoad %v4float %308
%316 = OpVectorTimesScalar %v4float %315 %314
%317 = OpFAdd %v4float %311 %316
OpStore %result_1 %317
%319 = OpLoad %v4float %result_1
%320 = OpVectorShuffle %v3float %319 %319 0 1 2
%321 = OpLoad %v4float %308
%322 = OpCompositeExtract %float %321 3
%323 = OpFSub %float %float_1 %322
%324 = OpLoad %v4float %307
%325 = OpVectorShuffle %v3float %324 %324 0 1 2
%326 = OpVectorTimesScalar %v3float %325 %323
%327 = OpLoad %v4float %308
%328 = OpVectorShuffle %v3float %327 %327 0 1 2
%329 = OpFAdd %v3float %326 %328
%318 = OpExtInst %v3float %1 FMax %320 %329
%330 = OpLoad %v4float %result_1
%331 = OpVectorShuffle %v4float %330 %318 4 5 6 3
OpStore %result_1 %331
%332 = OpLoad %v4float %result_1
OpReturnValue %332
OpFunctionEnd
%_guarded_divide = OpFunction %float None %333
%334 = OpFunctionParameter %_ptr_Function_float
%335 = OpFunctionParameter %_ptr_Function_float
%336 = OpLabel
%337 = OpLoad %float %334
%338 = OpLoad %float %335
%339 = OpFDiv %float %337 %338
OpReturnValue %339
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %184
%340 = OpFunctionParameter %_ptr_Function_v2float
%341 = OpFunctionParameter %_ptr_Function_v2float
%342 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_4_n = OpVariable %_ptr_Function_float Function
%343 = OpLoad %v2float %341
%344 = OpCompositeExtract %float %343 0
%345 = OpFOrdEqual %bool %344 %float_0
OpSelectionMerge %348 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%349 = OpLoad %v2float %340
%350 = OpCompositeExtract %float %349 0
%351 = OpLoad %v2float %341
%352 = OpCompositeExtract %float %351 1
%353 = OpFSub %float %float_1 %352
%354 = OpFMul %float %350 %353
OpReturnValue %354
%347 = OpLabel
%356 = OpLoad %v2float %340
%357 = OpCompositeExtract %float %356 1
%358 = OpLoad %v2float %340
%359 = OpCompositeExtract %float %358 0
%360 = OpFSub %float %357 %359
OpStore %delta %360
%361 = OpLoad %float %delta
%362 = OpFOrdEqual %bool %361 %float_0
OpSelectionMerge %365 None
OpBranchConditional %362 %363 %364
%363 = OpLabel
%366 = OpLoad %v2float %340
%367 = OpCompositeExtract %float %366 1
%368 = OpLoad %v2float %341
%369 = OpCompositeExtract %float %368 1
%370 = OpFMul %float %367 %369
%371 = OpLoad %v2float %340
%372 = OpCompositeExtract %float %371 0
%373 = OpLoad %v2float %341
%374 = OpCompositeExtract %float %373 1
%375 = OpFSub %float %float_1 %374
%376 = OpFMul %float %372 %375
%377 = OpFAdd %float %370 %376
%378 = OpLoad %v2float %341
%379 = OpCompositeExtract %float %378 0
%380 = OpLoad %v2float %340
%381 = OpCompositeExtract %float %380 1
%382 = OpFSub %float %float_1 %381
%383 = OpFMul %float %379 %382
%384 = OpFAdd %float %377 %383
OpReturnValue %384
%364 = OpLabel
%386 = OpLoad %v2float %341
%387 = OpCompositeExtract %float %386 0
%388 = OpLoad %v2float %340
%389 = OpCompositeExtract %float %388 1
%390 = OpFMul %float %387 %389
OpStore %_4_n %390
%392 = OpLoad %v2float %341
%393 = OpCompositeExtract %float %392 1
%394 = OpLoad %float %_4_n
%395 = OpLoad %float %delta
%396 = OpFDiv %float %394 %395
%391 = OpExtInst %float %1 FMin %393 %396
OpStore %delta %391
%397 = OpLoad %float %delta
%398 = OpLoad %v2float %340
%399 = OpCompositeExtract %float %398 1
%400 = OpFMul %float %397 %399
%401 = OpLoad %v2float %340
%402 = OpCompositeExtract %float %401 0
%403 = OpLoad %v2float %341
%404 = OpCompositeExtract %float %403 1
%405 = OpFSub %float %float_1 %404
%406 = OpFMul %float %402 %405
%407 = OpFAdd %float %400 %406
%408 = OpLoad %v2float %341
%409 = OpCompositeExtract %float %408 0
%410 = OpLoad %v2float %340
%411 = OpCompositeExtract %float %410 1
%412 = OpFSub %float %float_1 %411
%413 = OpFMul %float %409 %412
%414 = OpFAdd %float %407 %413
OpReturnValue %414
%365 = OpLabel
OpBranch %348
%348 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_dodge = OpFunction %v4float None %53
%415 = OpFunctionParameter %_ptr_Function_v4float
%416 = OpFunctionParameter %_ptr_Function_v4float
%417 = OpLabel
%420 = OpVariable %_ptr_Function_v2float Function
%423 = OpVariable %_ptr_Function_v2float Function
%427 = OpVariable %_ptr_Function_v2float Function
%430 = OpVariable %_ptr_Function_v2float Function
%434 = OpVariable %_ptr_Function_v2float Function
%437 = OpVariable %_ptr_Function_v2float Function
%418 = OpLoad %v4float %415
%419 = OpVectorShuffle %v2float %418 %418 0 3
OpStore %420 %419
%421 = OpLoad %v4float %416
%422 = OpVectorShuffle %v2float %421 %421 0 3
OpStore %423 %422
%424 = OpFunctionCall %float %_color_dodge_component %420 %423
%425 = OpLoad %v4float %415
%426 = OpVectorShuffle %v2float %425 %425 1 3
OpStore %427 %426
%428 = OpLoad %v4float %416
%429 = OpVectorShuffle %v2float %428 %428 1 3
OpStore %430 %429
%431 = OpFunctionCall %float %_color_dodge_component %427 %430
%432 = OpLoad %v4float %415
%433 = OpVectorShuffle %v2float %432 %432 2 3
OpStore %434 %433
%435 = OpLoad %v4float %416
%436 = OpVectorShuffle %v2float %435 %435 2 3
OpStore %437 %436
%438 = OpFunctionCall %float %_color_dodge_component %434 %437
%439 = OpLoad %v4float %415
%440 = OpCompositeExtract %float %439 3
%441 = OpLoad %v4float %415
%442 = OpCompositeExtract %float %441 3
%443 = OpFSub %float %float_1 %442
%444 = OpLoad %v4float %416
%445 = OpCompositeExtract %float %444 3
%446 = OpFMul %float %443 %445
%447 = OpFAdd %float %440 %446
%448 = OpCompositeConstruct %v4float %424 %431 %438 %447
OpReturnValue %448
OpFunctionEnd
%_color_burn_component = OpFunction %float None %184
%449 = OpFunctionParameter %_ptr_Function_v2float
%450 = OpFunctionParameter %_ptr_Function_v2float
%451 = OpLabel
%_6_n = OpVariable %_ptr_Function_float Function
%delta_0 = OpVariable %_ptr_Function_float Function
%452 = OpLoad %v2float %450
%453 = OpCompositeExtract %float %452 1
%454 = OpLoad %v2float %450
%455 = OpCompositeExtract %float %454 0
%456 = OpFOrdEqual %bool %453 %455
OpSelectionMerge %459 None
OpBranchConditional %456 %457 %458
%457 = OpLabel
%460 = OpLoad %v2float %449
%461 = OpCompositeExtract %float %460 1
%462 = OpLoad %v2float %450
%463 = OpCompositeExtract %float %462 1
%464 = OpFMul %float %461 %463
%465 = OpLoad %v2float %449
%466 = OpCompositeExtract %float %465 0
%467 = OpLoad %v2float %450
%468 = OpCompositeExtract %float %467 1
%469 = OpFSub %float %float_1 %468
%470 = OpFMul %float %466 %469
%471 = OpFAdd %float %464 %470
%472 = OpLoad %v2float %450
%473 = OpCompositeExtract %float %472 0
%474 = OpLoad %v2float %449
%475 = OpCompositeExtract %float %474 1
%476 = OpFSub %float %float_1 %475
%477 = OpFMul %float %473 %476
%478 = OpFAdd %float %471 %477
OpReturnValue %478
%458 = OpLabel
%479 = OpLoad %v2float %449
%480 = OpCompositeExtract %float %479 0
%481 = OpFOrdEqual %bool %480 %float_0
OpSelectionMerge %484 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%485 = OpLoad %v2float %450
%486 = OpCompositeExtract %float %485 0
%487 = OpLoad %v2float %449
%488 = OpCompositeExtract %float %487 1
%489 = OpFSub %float %float_1 %488
%490 = OpFMul %float %486 %489
OpReturnValue %490
%483 = OpLabel
%492 = OpLoad %v2float %450
%493 = OpCompositeExtract %float %492 1
%494 = OpLoad %v2float %450
%495 = OpCompositeExtract %float %494 0
%496 = OpFSub %float %493 %495
%497 = OpLoad %v2float %449
%498 = OpCompositeExtract %float %497 1
%499 = OpFMul %float %496 %498
OpStore %_6_n %499
%502 = OpLoad %v2float %450
%503 = OpCompositeExtract %float %502 1
%504 = OpLoad %float %_6_n
%505 = OpLoad %v2float %449
%506 = OpCompositeExtract %float %505 0
%507 = OpFDiv %float %504 %506
%508 = OpFSub %float %503 %507
%501 = OpExtInst %float %1 FMax %float_0 %508
OpStore %delta_0 %501
%509 = OpLoad %float %delta_0
%510 = OpLoad %v2float %449
%511 = OpCompositeExtract %float %510 1
%512 = OpFMul %float %509 %511
%513 = OpLoad %v2float %449
%514 = OpCompositeExtract %float %513 0
%515 = OpLoad %v2float %450
%516 = OpCompositeExtract %float %515 1
%517 = OpFSub %float %float_1 %516
%518 = OpFMul %float %514 %517
%519 = OpFAdd %float %512 %518
%520 = OpLoad %v2float %450
%521 = OpCompositeExtract %float %520 0
%522 = OpLoad %v2float %449
%523 = OpCompositeExtract %float %522 1
%524 = OpFSub %float %float_1 %523
%525 = OpFMul %float %521 %524
%526 = OpFAdd %float %519 %525
OpReturnValue %526
%484 = OpLabel
OpBranch %459
%459 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_burn = OpFunction %v4float None %53
%527 = OpFunctionParameter %_ptr_Function_v4float
%528 = OpFunctionParameter %_ptr_Function_v4float
%529 = OpLabel
%532 = OpVariable %_ptr_Function_v2float Function
%535 = OpVariable %_ptr_Function_v2float Function
%539 = OpVariable %_ptr_Function_v2float Function
%542 = OpVariable %_ptr_Function_v2float Function
%546 = OpVariable %_ptr_Function_v2float Function
%549 = OpVariable %_ptr_Function_v2float Function
%530 = OpLoad %v4float %527
%531 = OpVectorShuffle %v2float %530 %530 0 3
OpStore %532 %531
%533 = OpLoad %v4float %528
%534 = OpVectorShuffle %v2float %533 %533 0 3
OpStore %535 %534
%536 = OpFunctionCall %float %_color_burn_component %532 %535
%537 = OpLoad %v4float %527
%538 = OpVectorShuffle %v2float %537 %537 1 3
OpStore %539 %538
%540 = OpLoad %v4float %528
%541 = OpVectorShuffle %v2float %540 %540 1 3
OpStore %542 %541
%543 = OpFunctionCall %float %_color_burn_component %539 %542
%544 = OpLoad %v4float %527
%545 = OpVectorShuffle %v2float %544 %544 2 3
OpStore %546 %545
%547 = OpLoad %v4float %528
%548 = OpVectorShuffle %v2float %547 %547 2 3
OpStore %549 %548
%550 = OpFunctionCall %float %_color_burn_component %546 %549
%551 = OpLoad %v4float %527
%552 = OpCompositeExtract %float %551 3
%553 = OpLoad %v4float %527
%554 = OpCompositeExtract %float %553 3
%555 = OpFSub %float %float_1 %554
%556 = OpLoad %v4float %528
%557 = OpCompositeExtract %float %556 3
%558 = OpFMul %float %555 %557
%559 = OpFAdd %float %552 %558
%560 = OpCompositeConstruct %v4float %536 %543 %550 %559
OpReturnValue %560
OpFunctionEnd
%blend_hard_light = OpFunction %v4float None %53
%561 = OpFunctionParameter %_ptr_Function_v4float
%562 = OpFunctionParameter %_ptr_Function_v4float
%563 = OpLabel
%565 = OpVariable %_ptr_Function_v4float Function
%567 = OpVariable %_ptr_Function_v4float Function
%564 = OpLoad %v4float %562
OpStore %565 %564
%566 = OpLoad %v4float %561
OpStore %567 %566
%568 = OpFunctionCall %v4float %blend_overlay %565 %567
OpReturnValue %568
OpFunctionEnd
%_soft_light_component = OpFunction %float None %184
%569 = OpFunctionParameter %_ptr_Function_v2float
%570 = OpFunctionParameter %_ptr_Function_v2float
%571 = OpLabel
%_8_n = OpVariable %_ptr_Function_float Function
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_float Function
%572 = OpLoad %v2float %569
%573 = OpCompositeExtract %float %572 0
%574 = OpFMul %float %float_2 %573
%575 = OpLoad %v2float %569
%576 = OpCompositeExtract %float %575 1
%577 = OpFOrdLessThanEqual %bool %574 %576
OpSelectionMerge %580 None
OpBranchConditional %577 %578 %579
%578 = OpLabel
%582 = OpLoad %v2float %570
%583 = OpCompositeExtract %float %582 0
%584 = OpLoad %v2float %570
%585 = OpCompositeExtract %float %584 0
%586 = OpFMul %float %583 %585
%587 = OpLoad %v2float %569
%588 = OpCompositeExtract %float %587 1
%589 = OpLoad %v2float %569
%590 = OpCompositeExtract %float %589 0
%591 = OpFMul %float %float_2 %590
%592 = OpFSub %float %588 %591
%593 = OpFMul %float %586 %592
OpStore %_8_n %593
%594 = OpLoad %float %_8_n
%595 = OpLoad %v2float %570
%596 = OpCompositeExtract %float %595 1
%597 = OpFDiv %float %594 %596
%598 = OpLoad %v2float %570
%599 = OpCompositeExtract %float %598 1
%600 = OpFSub %float %float_1 %599
%601 = OpLoad %v2float %569
%602 = OpCompositeExtract %float %601 0
%603 = OpFMul %float %600 %602
%604 = OpFAdd %float %597 %603
%605 = OpLoad %v2float %570
%606 = OpCompositeExtract %float %605 0
%608 = OpLoad %v2float %569
%609 = OpCompositeExtract %float %608 1
%607 = OpFNegate %float %609
%610 = OpLoad %v2float %569
%611 = OpCompositeExtract %float %610 0
%612 = OpFMul %float %float_2 %611
%613 = OpFAdd %float %607 %612
%614 = OpFAdd %float %613 %float_1
%615 = OpFMul %float %606 %614
%616 = OpFAdd %float %604 %615
OpReturnValue %616
%579 = OpLabel
%618 = OpLoad %v2float %570
%619 = OpCompositeExtract %float %618 0
%620 = OpFMul %float %float_4 %619
%621 = OpLoad %v2float %570
%622 = OpCompositeExtract %float %621 1
%623 = OpFOrdLessThanEqual %bool %620 %622
OpSelectionMerge %626 None
OpBranchConditional %623 %624 %625
%624 = OpLabel
%628 = OpLoad %v2float %570
%629 = OpCompositeExtract %float %628 0
%630 = OpLoad %v2float %570
%631 = OpCompositeExtract %float %630 0
%632 = OpFMul %float %629 %631
OpStore %DSqd %632
%634 = OpLoad %float %DSqd
%635 = OpLoad %v2float %570
%636 = OpCompositeExtract %float %635 0
%637 = OpFMul %float %634 %636
OpStore %DCub %637
%639 = OpLoad %v2float %570
%640 = OpCompositeExtract %float %639 1
%641 = OpLoad %v2float %570
%642 = OpCompositeExtract %float %641 1
%643 = OpFMul %float %640 %642
OpStore %DaSqd %643
%645 = OpLoad %float %DaSqd
%646 = OpLoad %v2float %570
%647 = OpCompositeExtract %float %646 1
%648 = OpFMul %float %645 %647
OpStore %DaCub %648
%650 = OpLoad %float %DaSqd
%651 = OpLoad %v2float %569
%652 = OpCompositeExtract %float %651 0
%653 = OpLoad %v2float %570
%654 = OpCompositeExtract %float %653 0
%656 = OpLoad %v2float %569
%657 = OpCompositeExtract %float %656 1
%658 = OpFMul %float %float_3 %657
%660 = OpLoad %v2float %569
%661 = OpCompositeExtract %float %660 0
%662 = OpFMul %float %float_6 %661
%663 = OpFSub %float %658 %662
%664 = OpFSub %float %663 %float_1
%665 = OpFMul %float %654 %664
%666 = OpFSub %float %652 %665
%667 = OpFMul %float %650 %666
%669 = OpLoad %v2float %570
%670 = OpCompositeExtract %float %669 1
%671 = OpFMul %float %float_12 %670
%672 = OpLoad %float %DSqd
%673 = OpFMul %float %671 %672
%674 = OpLoad %v2float %569
%675 = OpCompositeExtract %float %674 1
%676 = OpLoad %v2float %569
%677 = OpCompositeExtract %float %676 0
%678 = OpFMul %float %float_2 %677
%679 = OpFSub %float %675 %678
%680 = OpFMul %float %673 %679
%681 = OpFAdd %float %667 %680
%683 = OpLoad %float %DCub
%684 = OpFMul %float %float_16 %683
%685 = OpLoad %v2float %569
%686 = OpCompositeExtract %float %685 1
%687 = OpLoad %v2float %569
%688 = OpCompositeExtract %float %687 0
%689 = OpFMul %float %float_2 %688
%690 = OpFSub %float %686 %689
%691 = OpFMul %float %684 %690
%692 = OpFSub %float %681 %691
%693 = OpLoad %float %DaCub
%694 = OpLoad %v2float %569
%695 = OpCompositeExtract %float %694 0
%696 = OpFMul %float %693 %695
%697 = OpFSub %float %692 %696
OpStore %_10_n %697
%698 = OpLoad %float %_10_n
%699 = OpLoad %float %DaSqd
%700 = OpFDiv %float %698 %699
OpReturnValue %700
%625 = OpLabel
%701 = OpLoad %v2float %570
%702 = OpCompositeExtract %float %701 0
%703 = OpLoad %v2float %569
%704 = OpCompositeExtract %float %703 1
%705 = OpLoad %v2float %569
%706 = OpCompositeExtract %float %705 0
%707 = OpFMul %float %float_2 %706
%708 = OpFSub %float %704 %707
%709 = OpFAdd %float %708 %float_1
%710 = OpFMul %float %702 %709
%711 = OpLoad %v2float %569
%712 = OpCompositeExtract %float %711 0
%713 = OpFAdd %float %710 %712
%715 = OpLoad %v2float %570
%716 = OpCompositeExtract %float %715 1
%717 = OpLoad %v2float %570
%718 = OpCompositeExtract %float %717 0
%719 = OpFMul %float %716 %718
%714 = OpExtInst %float %1 Sqrt %719
%720 = OpLoad %v2float %569
%721 = OpCompositeExtract %float %720 1
%722 = OpLoad %v2float %569
%723 = OpCompositeExtract %float %722 0
%724 = OpFMul %float %float_2 %723
%725 = OpFSub %float %721 %724
%726 = OpFMul %float %714 %725
%727 = OpFSub %float %713 %726
%728 = OpLoad %v2float %570
%729 = OpCompositeExtract %float %728 1
%730 = OpLoad %v2float %569
%731 = OpCompositeExtract %float %730 0
%732 = OpFMul %float %729 %731
%733 = OpFSub %float %727 %732
OpReturnValue %733
%626 = OpLabel
OpBranch %580
%580 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_soft_light = OpFunction %v4float None %53
%734 = OpFunctionParameter %_ptr_Function_v4float
%735 = OpFunctionParameter %_ptr_Function_v4float
%736 = OpLabel
%740 = OpVariable %_ptr_Function_v4float Function
%747 = OpVariable %_ptr_Function_v2float Function
%750 = OpVariable %_ptr_Function_v2float Function
%754 = OpVariable %_ptr_Function_v2float Function
%757 = OpVariable %_ptr_Function_v2float Function
%761 = OpVariable %_ptr_Function_v2float Function
%764 = OpVariable %_ptr_Function_v2float Function
%737 = OpLoad %v4float %735
%738 = OpCompositeExtract %float %737 3
%739 = OpFOrdEqual %bool %738 %float_0
OpSelectionMerge %743 None
OpBranchConditional %739 %741 %742
%741 = OpLabel
%744 = OpLoad %v4float %734
OpStore %740 %744
OpBranch %743
%742 = OpLabel
%745 = OpLoad %v4float %734
%746 = OpVectorShuffle %v2float %745 %745 0 3
OpStore %747 %746
%748 = OpLoad %v4float %735
%749 = OpVectorShuffle %v2float %748 %748 0 3
OpStore %750 %749
%751 = OpFunctionCall %float %_soft_light_component %747 %750
%752 = OpLoad %v4float %734
%753 = OpVectorShuffle %v2float %752 %752 1 3
OpStore %754 %753
%755 = OpLoad %v4float %735
%756 = OpVectorShuffle %v2float %755 %755 1 3
OpStore %757 %756
%758 = OpFunctionCall %float %_soft_light_component %754 %757
%759 = OpLoad %v4float %734
%760 = OpVectorShuffle %v2float %759 %759 2 3
OpStore %761 %760
%762 = OpLoad %v4float %735
%763 = OpVectorShuffle %v2float %762 %762 2 3
OpStore %764 %763
%765 = OpFunctionCall %float %_soft_light_component %761 %764
%766 = OpLoad %v4float %734
%767 = OpCompositeExtract %float %766 3
%768 = OpLoad %v4float %734
%769 = OpCompositeExtract %float %768 3
%770 = OpFSub %float %float_1 %769
%771 = OpLoad %v4float %735
%772 = OpCompositeExtract %float %771 3
%773 = OpFMul %float %770 %772
%774 = OpFAdd %float %767 %773
%775 = OpCompositeConstruct %v4float %751 %758 %765 %774
OpStore %740 %775
OpBranch %743
%743 = OpLabel
%776 = OpLoad %v4float %740
OpReturnValue %776
OpFunctionEnd
%blend_difference = OpFunction %v4float None %53
%777 = OpFunctionParameter %_ptr_Function_v4float
%778 = OpFunctionParameter %_ptr_Function_v4float
%779 = OpLabel
%780 = OpLoad %v4float %777
%781 = OpVectorShuffle %v3float %780 %780 0 1 2
%782 = OpLoad %v4float %778
%783 = OpVectorShuffle %v3float %782 %782 0 1 2
%784 = OpFAdd %v3float %781 %783
%786 = OpLoad %v4float %777
%787 = OpVectorShuffle %v3float %786 %786 0 1 2
%788 = OpLoad %v4float %778
%789 = OpCompositeExtract %float %788 3
%790 = OpVectorTimesScalar %v3float %787 %789
%791 = OpLoad %v4float %778
%792 = OpVectorShuffle %v3float %791 %791 0 1 2
%793 = OpLoad %v4float %777
%794 = OpCompositeExtract %float %793 3
%795 = OpVectorTimesScalar %v3float %792 %794
%785 = OpExtInst %v3float %1 FMin %790 %795
%796 = OpVectorTimesScalar %v3float %785 %float_2
%797 = OpFSub %v3float %784 %796
%798 = OpCompositeExtract %float %797 0
%799 = OpCompositeExtract %float %797 1
%800 = OpCompositeExtract %float %797 2
%801 = OpLoad %v4float %777
%802 = OpCompositeExtract %float %801 3
%803 = OpLoad %v4float %777
%804 = OpCompositeExtract %float %803 3
%805 = OpFSub %float %float_1 %804
%806 = OpLoad %v4float %778
%807 = OpCompositeExtract %float %806 3
%808 = OpFMul %float %805 %807
%809 = OpFAdd %float %802 %808
%810 = OpCompositeConstruct %v4float %798 %799 %800 %809
OpReturnValue %810
OpFunctionEnd
%blend_exclusion = OpFunction %v4float None %53
%811 = OpFunctionParameter %_ptr_Function_v4float
%812 = OpFunctionParameter %_ptr_Function_v4float
%813 = OpLabel
%814 = OpLoad %v4float %812
%815 = OpVectorShuffle %v3float %814 %814 0 1 2
%816 = OpLoad %v4float %811
%817 = OpVectorShuffle %v3float %816 %816 0 1 2
%818 = OpFAdd %v3float %815 %817
%819 = OpLoad %v4float %812
%820 = OpVectorShuffle %v3float %819 %819 0 1 2
%821 = OpVectorTimesScalar %v3float %820 %float_2
%822 = OpLoad %v4float %811
%823 = OpVectorShuffle %v3float %822 %822 0 1 2
%824 = OpFMul %v3float %821 %823
%825 = OpFSub %v3float %818 %824
%826 = OpCompositeExtract %float %825 0
%827 = OpCompositeExtract %float %825 1
%828 = OpCompositeExtract %float %825 2
%829 = OpLoad %v4float %811
%830 = OpCompositeExtract %float %829 3
%831 = OpLoad %v4float %811
%832 = OpCompositeExtract %float %831 3
%833 = OpFSub %float %float_1 %832
%834 = OpLoad %v4float %812
%835 = OpCompositeExtract %float %834 3
%836 = OpFMul %float %833 %835
%837 = OpFAdd %float %830 %836
%838 = OpCompositeConstruct %v4float %826 %827 %828 %837
OpReturnValue %838
OpFunctionEnd
%blend_multiply = OpFunction %v4float None %53
%839 = OpFunctionParameter %_ptr_Function_v4float
%840 = OpFunctionParameter %_ptr_Function_v4float
%841 = OpLabel
%842 = OpLoad %v4float %839
%843 = OpCompositeExtract %float %842 3
%844 = OpFSub %float %float_1 %843
%845 = OpLoad %v4float %840
%846 = OpVectorShuffle %v3float %845 %845 0 1 2
%847 = OpVectorTimesScalar %v3float %846 %844
%848 = OpLoad %v4float %840
%849 = OpCompositeExtract %float %848 3
%850 = OpFSub %float %float_1 %849
%851 = OpLoad %v4float %839
%852 = OpVectorShuffle %v3float %851 %851 0 1 2
%853 = OpVectorTimesScalar %v3float %852 %850
%854 = OpFAdd %v3float %847 %853
%855 = OpLoad %v4float %839
%856 = OpVectorShuffle %v3float %855 %855 0 1 2
%857 = OpLoad %v4float %840
%858 = OpVectorShuffle %v3float %857 %857 0 1 2
%859 = OpFMul %v3float %856 %858
%860 = OpFAdd %v3float %854 %859
%861 = OpCompositeExtract %float %860 0
%862 = OpCompositeExtract %float %860 1
%863 = OpCompositeExtract %float %860 2
%864 = OpLoad %v4float %839
%865 = OpCompositeExtract %float %864 3
%866 = OpLoad %v4float %839
%867 = OpCompositeExtract %float %866 3
%868 = OpFSub %float %float_1 %867
%869 = OpLoad %v4float %840
%870 = OpCompositeExtract %float %869 3
%871 = OpFMul %float %868 %870
%872 = OpFAdd %float %865 %871
%873 = OpCompositeConstruct %v4float %861 %862 %863 %872
OpReturnValue %873
OpFunctionEnd
%_blend_color_luminance = OpFunction %float None %874
%876 = OpFunctionParameter %_ptr_Function_v3float
%877 = OpLabel
%883 = OpLoad %v3float %876
%878 = OpDot %float %882 %883
OpReturnValue %878
OpFunctionEnd
%_blend_set_color_luminance = OpFunction %v3float None %884
%885 = OpFunctionParameter %_ptr_Function_v3float
%886 = OpFunctionParameter %_ptr_Function_float
%887 = OpFunctionParameter %_ptr_Function_v3float
%888 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_2 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%952 = OpVariable %_ptr_Function_v3float Function
%891 = OpLoad %v3float %887
%890 = OpDot %float %882 %891
OpStore %lum %890
%893 = OpLoad %float %lum
%895 = OpLoad %v3float %885
%894 = OpDot %float %882 %895
%896 = OpFSub %float %893 %894
%897 = OpLoad %v3float %885
%898 = OpCompositeConstruct %v3float %896 %896 %896
%899 = OpFAdd %v3float %898 %897
OpStore %result_2 %899
%903 = OpLoad %v3float %result_2
%904 = OpCompositeExtract %float %903 0
%905 = OpLoad %v3float %result_2
%906 = OpCompositeExtract %float %905 1
%902 = OpExtInst %float %1 FMin %904 %906
%907 = OpLoad %v3float %result_2
%908 = OpCompositeExtract %float %907 2
%901 = OpExtInst %float %1 FMin %902 %908
OpStore %minComp %901
%912 = OpLoad %v3float %result_2
%913 = OpCompositeExtract %float %912 0
%914 = OpLoad %v3float %result_2
%915 = OpCompositeExtract %float %914 1
%911 = OpExtInst %float %1 FMax %913 %915
%916 = OpLoad %v3float %result_2
%917 = OpCompositeExtract %float %916 2
%910 = OpExtInst %float %1 FMax %911 %917
OpStore %maxComp %910
%919 = OpLoad %float %minComp
%920 = OpFOrdLessThan %bool %919 %float_0
OpSelectionMerge %922 None
OpBranchConditional %920 %921 %922
%921 = OpLabel
%923 = OpLoad %float %lum
%924 = OpLoad %float %minComp
%925 = OpFOrdNotEqual %bool %923 %924
OpBranch %922
%922 = OpLabel
%926 = OpPhi %bool %false %888 %925 %921
OpSelectionMerge %928 None
OpBranchConditional %926 %927 %928
%927 = OpLabel
%929 = OpLoad %float %lum
%930 = OpLoad %v3float %result_2
%931 = OpLoad %float %lum
%932 = OpCompositeConstruct %v3float %931 %931 %931
%933 = OpFSub %v3float %930 %932
%934 = OpLoad %float %lum
%935 = OpVectorTimesScalar %v3float %933 %934
%936 = OpLoad %float %lum
%937 = OpLoad %float %minComp
%938 = OpFSub %float %936 %937
%939 = OpFDiv %float %float_1 %938
%940 = OpVectorTimesScalar %v3float %935 %939
%941 = OpCompositeConstruct %v3float %929 %929 %929
%942 = OpFAdd %v3float %941 %940
OpStore %result_2 %942
OpBranch %928
%928 = OpLabel
%943 = OpLoad %float %maxComp
%944 = OpLoad %float %886
%945 = OpFOrdGreaterThan %bool %943 %944
OpSelectionMerge %947 None
OpBranchConditional %945 %946 %947
%946 = OpLabel
%948 = OpLoad %float %maxComp
%949 = OpLoad %float %lum
%950 = OpFOrdNotEqual %bool %948 %949
OpBranch %947
%947 = OpLabel
%951 = OpPhi %bool %false %928 %950 %946
OpSelectionMerge %955 None
OpBranchConditional %951 %953 %954
%953 = OpLabel
%956 = OpLoad %float %lum
%957 = OpLoad %v3float %result_2
%958 = OpLoad %float %lum
%959 = OpCompositeConstruct %v3float %958 %958 %958
%960 = OpFSub %v3float %957 %959
%961 = OpLoad %float %886
%962 = OpLoad %float %lum
%963 = OpFSub %float %961 %962
%964 = OpVectorTimesScalar %v3float %960 %963
%965 = OpLoad %float %maxComp
%966 = OpLoad %float %lum
%967 = OpFSub %float %965 %966
%968 = OpFDiv %float %float_1 %967
%969 = OpVectorTimesScalar %v3float %964 %968
%970 = OpCompositeConstruct %v3float %956 %956 %956
%971 = OpFAdd %v3float %970 %969
OpStore %952 %971
OpBranch %955
%954 = OpLabel
%972 = OpLoad %v3float %result_2
OpStore %952 %972
OpBranch %955
%955 = OpLabel
%973 = OpLoad %v3float %952
OpReturnValue %973
OpFunctionEnd
%_blend_color_saturation = OpFunction %float None %874
%974 = OpFunctionParameter %_ptr_Function_v3float
%975 = OpLabel
%978 = OpLoad %v3float %974
%979 = OpCompositeExtract %float %978 0
%980 = OpLoad %v3float %974
%981 = OpCompositeExtract %float %980 1
%977 = OpExtInst %float %1 FMax %979 %981
%982 = OpLoad %v3float %974
%983 = OpCompositeExtract %float %982 2
%976 = OpExtInst %float %1 FMax %977 %983
%986 = OpLoad %v3float %974
%987 = OpCompositeExtract %float %986 0
%988 = OpLoad %v3float %974
%989 = OpCompositeExtract %float %988 1
%985 = OpExtInst %float %1 FMin %987 %989
%990 = OpLoad %v3float %974
%991 = OpCompositeExtract %float %990 2
%984 = OpExtInst %float %1 FMin %985 %991
%992 = OpFSub %float %976 %984
OpReturnValue %992
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %993
%994 = OpFunctionParameter %_ptr_Function_v3float
%995 = OpFunctionParameter %_ptr_Function_float
%996 = OpLabel
%1002 = OpVariable %_ptr_Function_v3float Function
%997 = OpLoad %v3float %994
%998 = OpCompositeExtract %float %997 0
%999 = OpLoad %v3float %994
%1000 = OpCompositeExtract %float %999 2
%1001 = OpFOrdLessThan %bool %998 %1000
OpSelectionMerge %1005 None
OpBranchConditional %1001 %1003 %1004
%1003 = OpLabel
%1006 = OpLoad %float %995
%1007 = OpLoad %v3float %994
%1008 = OpCompositeExtract %float %1007 1
%1009 = OpLoad %v3float %994
%1010 = OpCompositeExtract %float %1009 0
%1011 = OpFSub %float %1008 %1010
%1012 = OpFMul %float %1006 %1011
%1013 = OpLoad %v3float %994
%1014 = OpCompositeExtract %float %1013 2
%1015 = OpLoad %v3float %994
%1016 = OpCompositeExtract %float %1015 0
%1017 = OpFSub %float %1014 %1016
%1018 = OpFDiv %float %1012 %1017
%1019 = OpLoad %float %995
%1020 = OpCompositeConstruct %v3float %float_0 %1018 %1019
OpStore %1002 %1020
OpBranch %1005
%1004 = OpLabel
OpStore %1002 %1021
OpBranch %1005
%1005 = OpLabel
%1022 = OpLoad %v3float %1002
OpReturnValue %1022
OpFunctionEnd
%_blend_set_color_saturation = OpFunction %v3float None %1023
%1024 = OpFunctionParameter %_ptr_Function_v3float
%1025 = OpFunctionParameter %_ptr_Function_v3float
%1026 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%1062 = OpVariable %_ptr_Function_v3float Function
%1064 = OpVariable %_ptr_Function_float Function
%1076 = OpVariable %_ptr_Function_v3float Function
%1078 = OpVariable %_ptr_Function_float Function
%1083 = OpVariable %_ptr_Function_v3float Function
%1085 = OpVariable %_ptr_Function_float Function
%1098 = OpVariable %_ptr_Function_v3float Function
%1100 = OpVariable %_ptr_Function_float Function
%1113 = OpVariable %_ptr_Function_v3float Function
%1115 = OpVariable %_ptr_Function_float Function
%1120 = OpVariable %_ptr_Function_v3float Function
%1122 = OpVariable %_ptr_Function_float Function
%1030 = OpLoad %v3float %1025
%1031 = OpCompositeExtract %float %1030 0
%1032 = OpLoad %v3float %1025
%1033 = OpCompositeExtract %float %1032 1
%1029 = OpExtInst %float %1 FMax %1031 %1033
%1034 = OpLoad %v3float %1025
%1035 = OpCompositeExtract %float %1034 2
%1028 = OpExtInst %float %1 FMax %1029 %1035
%1038 = OpLoad %v3float %1025
%1039 = OpCompositeExtract %float %1038 0
%1040 = OpLoad %v3float %1025
%1041 = OpCompositeExtract %float %1040 1
%1037 = OpExtInst %float %1 FMin %1039 %1041
%1042 = OpLoad %v3float %1025
%1043 = OpCompositeExtract %float %1042 2
%1036 = OpExtInst %float %1 FMin %1037 %1043
%1044 = OpFSub %float %1028 %1036
OpStore %sat %1044
%1045 = OpLoad %v3float %1024
%1046 = OpCompositeExtract %float %1045 0
%1047 = OpLoad %v3float %1024
%1048 = OpCompositeExtract %float %1047 1
%1049 = OpFOrdLessThanEqual %bool %1046 %1048
OpSelectionMerge %1052 None
OpBranchConditional %1049 %1050 %1051
%1050 = OpLabel
%1053 = OpLoad %v3float %1024
%1054 = OpCompositeExtract %float %1053 1
%1055 = OpLoad %v3float %1024
%1056 = OpCompositeExtract %float %1055 2
%1057 = OpFOrdLessThanEqual %bool %1054 %1056
OpSelectionMerge %1060 None
OpBranchConditional %1057 %1058 %1059
%1058 = OpLabel
%1061 = OpLoad %v3float %1024
OpStore %1062 %1061
%1063 = OpLoad %float %sat
OpStore %1064 %1063
%1065 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1062 %1064
OpReturnValue %1065
%1059 = OpLabel
%1066 = OpLoad %v3float %1024
%1067 = OpCompositeExtract %float %1066 0
%1068 = OpLoad %v3float %1024
%1069 = OpCompositeExtract %float %1068 2
%1070 = OpFOrdLessThanEqual %bool %1067 %1069
OpSelectionMerge %1073 None
OpBranchConditional %1070 %1071 %1072
%1071 = OpLabel
%1074 = OpLoad %v3float %1024
%1075 = OpVectorShuffle %v3float %1074 %1074 0 2 1
OpStore %1076 %1075
%1077 = OpLoad %float %sat
OpStore %1078 %1077
%1079 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1076 %1078
%1080 = OpVectorShuffle %v3float %1079 %1079 0 2 1
OpReturnValue %1080
%1072 = OpLabel
%1081 = OpLoad %v3float %1024
%1082 = OpVectorShuffle %v3float %1081 %1081 2 0 1
OpStore %1083 %1082
%1084 = OpLoad %float %sat
OpStore %1085 %1084
%1086 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1083 %1085
%1087 = OpVectorShuffle %v3float %1086 %1086 1 2 0
OpReturnValue %1087
%1073 = OpLabel
OpBranch %1060
%1060 = OpLabel
OpBranch %1052
%1051 = OpLabel
%1088 = OpLoad %v3float %1024
%1089 = OpCompositeExtract %float %1088 0
%1090 = OpLoad %v3float %1024
%1091 = OpCompositeExtract %float %1090 2
%1092 = OpFOrdLessThanEqual %bool %1089 %1091
OpSelectionMerge %1095 None
OpBranchConditional %1092 %1093 %1094
%1093 = OpLabel
%1096 = OpLoad %v3float %1024
%1097 = OpVectorShuffle %v3float %1096 %1096 1 0 2
OpStore %1098 %1097
%1099 = OpLoad %float %sat
OpStore %1100 %1099
%1101 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1098 %1100
%1102 = OpVectorShuffle %v3float %1101 %1101 1 0 2
OpReturnValue %1102
%1094 = OpLabel
%1103 = OpLoad %v3float %1024
%1104 = OpCompositeExtract %float %1103 1
%1105 = OpLoad %v3float %1024
%1106 = OpCompositeExtract %float %1105 2
%1107 = OpFOrdLessThanEqual %bool %1104 %1106
OpSelectionMerge %1110 None
OpBranchConditional %1107 %1108 %1109
%1108 = OpLabel
%1111 = OpLoad %v3float %1024
%1112 = OpVectorShuffle %v3float %1111 %1111 1 2 0
OpStore %1113 %1112
%1114 = OpLoad %float %sat
OpStore %1115 %1114
%1116 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1113 %1115
%1117 = OpVectorShuffle %v3float %1116 %1116 2 0 1
OpReturnValue %1117
%1109 = OpLabel
%1118 = OpLoad %v3float %1024
%1119 = OpVectorShuffle %v3float %1118 %1118 2 1 0
OpStore %1120 %1119
%1121 = OpLoad %float %sat
OpStore %1122 %1121
%1123 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %1120 %1122
%1124 = OpVectorShuffle %v3float %1123 %1123 2 1 0
OpReturnValue %1124
%1110 = OpLabel
OpBranch %1095
%1095 = OpLabel
OpBranch %1052
%1052 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_hue = OpFunction %v4float None %53
%1125 = OpFunctionParameter %_ptr_Function_v4float
%1126 = OpFunctionParameter %_ptr_Function_v4float
%1127 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%1147 = OpVariable %_ptr_Function_v3float Function
%1149 = OpVariable %_ptr_Function_v3float Function
%1151 = OpVariable %_ptr_Function_v3float Function
%1153 = OpVariable %_ptr_Function_float Function
%1155 = OpVariable %_ptr_Function_v3float Function
%1129 = OpLoad %v4float %1126
%1130 = OpCompositeExtract %float %1129 3
%1131 = OpLoad %v4float %1125
%1132 = OpCompositeExtract %float %1131 3
%1133 = OpFMul %float %1130 %1132
OpStore %alpha %1133
%1135 = OpLoad %v4float %1125
%1136 = OpVectorShuffle %v3float %1135 %1135 0 1 2
%1137 = OpLoad %v4float %1126
%1138 = OpCompositeExtract %float %1137 3
%1139 = OpVectorTimesScalar %v3float %1136 %1138
OpStore %sda %1139
%1141 = OpLoad %v4float %1126
%1142 = OpVectorShuffle %v3float %1141 %1141 0 1 2
%1143 = OpLoad %v4float %1125
%1144 = OpCompositeExtract %float %1143 3
%1145 = OpVectorTimesScalar %v3float %1142 %1144
OpStore %dsa %1145
%1146 = OpLoad %v3float %sda
OpStore %1147 %1146
%1148 = OpLoad %v3float %dsa
OpStore %1149 %1148
%1150 = OpFunctionCall %v3float %_blend_set_color_saturation %1147 %1149
OpStore %1151 %1150
%1152 = OpLoad %float %alpha
OpStore %1153 %1152
%1154 = OpLoad %v3float %dsa
OpStore %1155 %1154
%1156 = OpFunctionCall %v3float %_blend_set_color_luminance %1151 %1153 %1155
%1157 = OpLoad %v4float %1126
%1158 = OpVectorShuffle %v3float %1157 %1157 0 1 2
%1159 = OpFAdd %v3float %1156 %1158
%1160 = OpLoad %v3float %dsa
%1161 = OpFSub %v3float %1159 %1160
%1162 = OpLoad %v4float %1125
%1163 = OpVectorShuffle %v3float %1162 %1162 0 1 2
%1164 = OpFAdd %v3float %1161 %1163
%1165 = OpLoad %v3float %sda
%1166 = OpFSub %v3float %1164 %1165
%1167 = OpCompositeExtract %float %1166 0
%1168 = OpCompositeExtract %float %1166 1
%1169 = OpCompositeExtract %float %1166 2
%1170 = OpLoad %v4float %1125
%1171 = OpCompositeExtract %float %1170 3
%1172 = OpLoad %v4float %1126
%1173 = OpCompositeExtract %float %1172 3
%1174 = OpFAdd %float %1171 %1173
%1175 = OpLoad %float %alpha
%1176 = OpFSub %float %1174 %1175
%1177 = OpCompositeConstruct %v4float %1167 %1168 %1169 %1176
OpReturnValue %1177
OpFunctionEnd
%blend_saturation = OpFunction %v4float None %53
%1178 = OpFunctionParameter %_ptr_Function_v4float
%1179 = OpFunctionParameter %_ptr_Function_v4float
%1180 = OpLabel
%alpha_0 = OpVariable %_ptr_Function_float Function
%sda_0 = OpVariable %_ptr_Function_v3float Function
%dsa_0 = OpVariable %_ptr_Function_v3float Function
%1200 = OpVariable %_ptr_Function_v3float Function
%1202 = OpVariable %_ptr_Function_v3float Function
%1204 = OpVariable %_ptr_Function_v3float Function
%1206 = OpVariable %_ptr_Function_float Function
%1208 = OpVariable %_ptr_Function_v3float Function
%1182 = OpLoad %v4float %1179
%1183 = OpCompositeExtract %float %1182 3
%1184 = OpLoad %v4float %1178
%1185 = OpCompositeExtract %float %1184 3
%1186 = OpFMul %float %1183 %1185
OpStore %alpha_0 %1186
%1188 = OpLoad %v4float %1178
%1189 = OpVectorShuffle %v3float %1188 %1188 0 1 2
%1190 = OpLoad %v4float %1179
%1191 = OpCompositeExtract %float %1190 3
%1192 = OpVectorTimesScalar %v3float %1189 %1191
OpStore %sda_0 %1192
%1194 = OpLoad %v4float %1179
%1195 = OpVectorShuffle %v3float %1194 %1194 0 1 2
%1196 = OpLoad %v4float %1178
%1197 = OpCompositeExtract %float %1196 3
%1198 = OpVectorTimesScalar %v3float %1195 %1197
OpStore %dsa_0 %1198
%1199 = OpLoad %v3float %dsa_0
OpStore %1200 %1199
%1201 = OpLoad %v3float %sda_0
OpStore %1202 %1201
%1203 = OpFunctionCall %v3float %_blend_set_color_saturation %1200 %1202
OpStore %1204 %1203
%1205 = OpLoad %float %alpha_0
OpStore %1206 %1205
%1207 = OpLoad %v3float %dsa_0
OpStore %1208 %1207
%1209 = OpFunctionCall %v3float %_blend_set_color_luminance %1204 %1206 %1208
%1210 = OpLoad %v4float %1179
%1211 = OpVectorShuffle %v3float %1210 %1210 0 1 2
%1212 = OpFAdd %v3float %1209 %1211
%1213 = OpLoad %v3float %dsa_0
%1214 = OpFSub %v3float %1212 %1213
%1215 = OpLoad %v4float %1178
%1216 = OpVectorShuffle %v3float %1215 %1215 0 1 2
%1217 = OpFAdd %v3float %1214 %1216
%1218 = OpLoad %v3float %sda_0
%1219 = OpFSub %v3float %1217 %1218
%1220 = OpCompositeExtract %float %1219 0
%1221 = OpCompositeExtract %float %1219 1
%1222 = OpCompositeExtract %float %1219 2
%1223 = OpLoad %v4float %1178
%1224 = OpCompositeExtract %float %1223 3
%1225 = OpLoad %v4float %1179
%1226 = OpCompositeExtract %float %1225 3
%1227 = OpFAdd %float %1224 %1226
%1228 = OpLoad %float %alpha_0
%1229 = OpFSub %float %1227 %1228
%1230 = OpCompositeConstruct %v4float %1220 %1221 %1222 %1229
OpReturnValue %1230
OpFunctionEnd
%blend_color = OpFunction %v4float None %53
%1231 = OpFunctionParameter %_ptr_Function_v4float
%1232 = OpFunctionParameter %_ptr_Function_v4float
%1233 = OpLabel
%alpha_1 = OpVariable %_ptr_Function_float Function
%sda_1 = OpVariable %_ptr_Function_v3float Function
%dsa_1 = OpVariable %_ptr_Function_v3float Function
%1253 = OpVariable %_ptr_Function_v3float Function
%1255 = OpVariable %_ptr_Function_float Function
%1257 = OpVariable %_ptr_Function_v3float Function
%1235 = OpLoad %v4float %1232
%1236 = OpCompositeExtract %float %1235 3
%1237 = OpLoad %v4float %1231
%1238 = OpCompositeExtract %float %1237 3
%1239 = OpFMul %float %1236 %1238
OpStore %alpha_1 %1239
%1241 = OpLoad %v4float %1231
%1242 = OpVectorShuffle %v3float %1241 %1241 0 1 2
%1243 = OpLoad %v4float %1232
%1244 = OpCompositeExtract %float %1243 3
%1245 = OpVectorTimesScalar %v3float %1242 %1244
OpStore %sda_1 %1245
%1247 = OpLoad %v4float %1232
%1248 = OpVectorShuffle %v3float %1247 %1247 0 1 2
%1249 = OpLoad %v4float %1231
%1250 = OpCompositeExtract %float %1249 3
%1251 = OpVectorTimesScalar %v3float %1248 %1250
OpStore %dsa_1 %1251
%1252 = OpLoad %v3float %sda_1
OpStore %1253 %1252
%1254 = OpLoad %float %alpha_1
OpStore %1255 %1254
%1256 = OpLoad %v3float %dsa_1
OpStore %1257 %1256
%1258 = OpFunctionCall %v3float %_blend_set_color_luminance %1253 %1255 %1257
%1259 = OpLoad %v4float %1232
%1260 = OpVectorShuffle %v3float %1259 %1259 0 1 2
%1261 = OpFAdd %v3float %1258 %1260
%1262 = OpLoad %v3float %dsa_1
%1263 = OpFSub %v3float %1261 %1262
%1264 = OpLoad %v4float %1231
%1265 = OpVectorShuffle %v3float %1264 %1264 0 1 2
%1266 = OpFAdd %v3float %1263 %1265
%1267 = OpLoad %v3float %sda_1
%1268 = OpFSub %v3float %1266 %1267
%1269 = OpCompositeExtract %float %1268 0
%1270 = OpCompositeExtract %float %1268 1
%1271 = OpCompositeExtract %float %1268 2
%1272 = OpLoad %v4float %1231
%1273 = OpCompositeExtract %float %1272 3
%1274 = OpLoad %v4float %1232
%1275 = OpCompositeExtract %float %1274 3
%1276 = OpFAdd %float %1273 %1275
%1277 = OpLoad %float %alpha_1
%1278 = OpFSub %float %1276 %1277
%1279 = OpCompositeConstruct %v4float %1269 %1270 %1271 %1278
OpReturnValue %1279
OpFunctionEnd
%blend_luminosity = OpFunction %v4float None %53
%1280 = OpFunctionParameter %_ptr_Function_v4float
%1281 = OpFunctionParameter %_ptr_Function_v4float
%1282 = OpLabel
%alpha_2 = OpVariable %_ptr_Function_float Function
%sda_2 = OpVariable %_ptr_Function_v3float Function
%dsa_2 = OpVariable %_ptr_Function_v3float Function
%1302 = OpVariable %_ptr_Function_v3float Function
%1304 = OpVariable %_ptr_Function_float Function
%1306 = OpVariable %_ptr_Function_v3float Function
%1284 = OpLoad %v4float %1281
%1285 = OpCompositeExtract %float %1284 3
%1286 = OpLoad %v4float %1280
%1287 = OpCompositeExtract %float %1286 3
%1288 = OpFMul %float %1285 %1287
OpStore %alpha_2 %1288
%1290 = OpLoad %v4float %1280
%1291 = OpVectorShuffle %v3float %1290 %1290 0 1 2
%1292 = OpLoad %v4float %1281
%1293 = OpCompositeExtract %float %1292 3
%1294 = OpVectorTimesScalar %v3float %1291 %1293
OpStore %sda_2 %1294
%1296 = OpLoad %v4float %1281
%1297 = OpVectorShuffle %v3float %1296 %1296 0 1 2
%1298 = OpLoad %v4float %1280
%1299 = OpCompositeExtract %float %1298 3
%1300 = OpVectorTimesScalar %v3float %1297 %1299
OpStore %dsa_2 %1300
%1301 = OpLoad %v3float %dsa_2
OpStore %1302 %1301
%1303 = OpLoad %float %alpha_2
OpStore %1304 %1303
%1305 = OpLoad %v3float %sda_2
OpStore %1306 %1305
%1307 = OpFunctionCall %v3float %_blend_set_color_luminance %1302 %1304 %1306
%1308 = OpLoad %v4float %1281
%1309 = OpVectorShuffle %v3float %1308 %1308 0 1 2
%1310 = OpFAdd %v3float %1307 %1309
%1311 = OpLoad %v3float %dsa_2
%1312 = OpFSub %v3float %1310 %1311
%1313 = OpLoad %v4float %1280
%1314 = OpVectorShuffle %v3float %1313 %1313 0 1 2
%1315 = OpFAdd %v3float %1312 %1314
%1316 = OpLoad %v3float %sda_2
%1317 = OpFSub %v3float %1315 %1316
%1318 = OpCompositeExtract %float %1317 0
%1319 = OpCompositeExtract %float %1317 1
%1320 = OpCompositeExtract %float %1317 2
%1321 = OpLoad %v4float %1280
%1322 = OpCompositeExtract %float %1321 3
%1323 = OpLoad %v4float %1281
%1324 = OpCompositeExtract %float %1323 3
%1325 = OpFAdd %float %1322 %1324
%1326 = OpLoad %float %alpha_2
%1327 = OpFSub %float %1325 %1326
%1328 = OpCompositeConstruct %v4float %1318 %1319 %1320 %1327
OpReturnValue %1328
OpFunctionEnd
%blend = OpFunction %v4float None %1330
%1332 = OpFunctionParameter %_ptr_Function_int
%1333 = OpFunctionParameter %_ptr_Function_v4float
%1334 = OpFunctionParameter %_ptr_Function_v4float
%1335 = OpLabel
%1448 = OpVariable %_ptr_Function_v4float Function
%1450 = OpVariable %_ptr_Function_v4float Function
%_32_result = OpVariable %_ptr_Function_v4float Function
%_35_result = OpVariable %_ptr_Function_v4float Function
%1500 = OpVariable %_ptr_Function_v2float Function
%1503 = OpVariable %_ptr_Function_v2float Function
%1507 = OpVariable %_ptr_Function_v2float Function
%1510 = OpVariable %_ptr_Function_v2float Function
%1514 = OpVariable %_ptr_Function_v2float Function
%1517 = OpVariable %_ptr_Function_v2float Function
%1531 = OpVariable %_ptr_Function_v2float Function
%1534 = OpVariable %_ptr_Function_v2float Function
%1538 = OpVariable %_ptr_Function_v2float Function
%1541 = OpVariable %_ptr_Function_v2float Function
%1545 = OpVariable %_ptr_Function_v2float Function
%1548 = OpVariable %_ptr_Function_v2float Function
%1561 = OpVariable %_ptr_Function_v4float Function
%1563 = OpVariable %_ptr_Function_v4float Function
%1568 = OpVariable %_ptr_Function_v4float Function
%1575 = OpVariable %_ptr_Function_v2float Function
%1578 = OpVariable %_ptr_Function_v2float Function
%1582 = OpVariable %_ptr_Function_v2float Function
%1585 = OpVariable %_ptr_Function_v2float Function
%1589 = OpVariable %_ptr_Function_v2float Function
%1592 = OpVariable %_ptr_Function_v2float Function
%_44_alpha = OpVariable %_ptr_Function_float Function
%_45_sda = OpVariable %_ptr_Function_v3float Function
%_46_dsa = OpVariable %_ptr_Function_v3float Function
%1712 = OpVariable %_ptr_Function_v3float Function
%1714 = OpVariable %_ptr_Function_v3float Function
%1716 = OpVariable %_ptr_Function_v3float Function
%1718 = OpVariable %_ptr_Function_float Function
%1720 = OpVariable %_ptr_Function_v3float Function
%_48_alpha = OpVariable %_ptr_Function_float Function
%_49_sda = OpVariable %_ptr_Function_v3float Function
%_50_dsa = OpVariable %_ptr_Function_v3float Function
%1762 = OpVariable %_ptr_Function_v3float Function
%1764 = OpVariable %_ptr_Function_v3float Function
%1766 = OpVariable %_ptr_Function_v3float Function
%1768 = OpVariable %_ptr_Function_float Function
%1770 = OpVariable %_ptr_Function_v3float Function
%_52_alpha = OpVariable %_ptr_Function_float Function
%_53_sda = OpVariable %_ptr_Function_v3float Function
%_54_dsa = OpVariable %_ptr_Function_v3float Function
%1812 = OpVariable %_ptr_Function_v3float Function
%1814 = OpVariable %_ptr_Function_float Function
%1816 = OpVariable %_ptr_Function_v3float Function
%_56_alpha = OpVariable %_ptr_Function_float Function
%_57_sda = OpVariable %_ptr_Function_v3float Function
%_58_dsa = OpVariable %_ptr_Function_v3float Function
%1858 = OpVariable %_ptr_Function_v3float Function
%1860 = OpVariable %_ptr_Function_float Function
%1862 = OpVariable %_ptr_Function_v3float Function
%1336 = OpLoad %int %1332
OpSelectionMerge %1337 None
OpSwitch %1336 %1337 0 %1338 1 %1339 2 %1340 3 %1341 4 %1342 5 %1343 6 %1344 7 %1345 8 %1346 9 %1347 10 %1348 11 %1349 12 %1350 13 %1351 14 %1352 15 %1353 16 %1354 17 %1355 18 %1356 19 %1357 20 %1358 21 %1359 22 %1360 23 %1361 24 %1362 25 %1363 26 %1364 27 %1365 28 %1366
%1338 = OpLabel
OpReturnValue %59
%1339 = OpLabel
%1367 = OpLoad %v4float %1333
OpReturnValue %1367
%1340 = OpLabel
%1368 = OpLoad %v4float %1334
OpReturnValue %1368
%1341 = OpLabel
%1369 = OpLoad %v4float %1333
%1370 = OpLoad %v4float %1333
%1371 = OpCompositeExtract %float %1370 3
%1372 = OpFSub %float %float_1 %1371
%1373 = OpLoad %v4float %1334
%1374 = OpVectorTimesScalar %v4float %1373 %1372
%1375 = OpFAdd %v4float %1369 %1374
OpReturnValue %1375
%1342 = OpLabel
%1376 = OpLoad %v4float %1334
%1377 = OpCompositeExtract %float %1376 3
%1378 = OpFSub %float %float_1 %1377
%1379 = OpLoad %v4float %1333
%1380 = OpVectorTimesScalar %v4float %1379 %1378
%1381 = OpLoad %v4float %1334
%1382 = OpFAdd %v4float %1380 %1381
OpReturnValue %1382
%1343 = OpLabel
%1383 = OpLoad %v4float %1333
%1384 = OpLoad %v4float %1334
%1385 = OpCompositeExtract %float %1384 3
%1386 = OpVectorTimesScalar %v4float %1383 %1385
OpReturnValue %1386
%1344 = OpLabel
%1387 = OpLoad %v4float %1334
%1388 = OpLoad %v4float %1333
%1389 = OpCompositeExtract %float %1388 3
%1390 = OpVectorTimesScalar %v4float %1387 %1389
OpReturnValue %1390
%1345 = OpLabel
%1391 = OpLoad %v4float %1334
%1392 = OpCompositeExtract %float %1391 3
%1393 = OpFSub %float %float_1 %1392
%1394 = OpLoad %v4float %1333
%1395 = OpVectorTimesScalar %v4float %1394 %1393
OpReturnValue %1395
%1346 = OpLabel
%1396 = OpLoad %v4float %1333
%1397 = OpCompositeExtract %float %1396 3
%1398 = OpFSub %float %float_1 %1397
%1399 = OpLoad %v4float %1334
%1400 = OpVectorTimesScalar %v4float %1399 %1398
OpReturnValue %1400
%1347 = OpLabel
%1401 = OpLoad %v4float %1334
%1402 = OpCompositeExtract %float %1401 3
%1403 = OpLoad %v4float %1333
%1404 = OpVectorTimesScalar %v4float %1403 %1402
%1405 = OpLoad %v4float %1333
%1406 = OpCompositeExtract %float %1405 3
%1407 = OpFSub %float %float_1 %1406
%1408 = OpLoad %v4float %1334
%1409 = OpVectorTimesScalar %v4float %1408 %1407
%1410 = OpFAdd %v4float %1404 %1409
OpReturnValue %1410
%1348 = OpLabel
%1411 = OpLoad %v4float %1334
%1412 = OpCompositeExtract %float %1411 3
%1413 = OpFSub %float %float_1 %1412
%1414 = OpLoad %v4float %1333
%1415 = OpVectorTimesScalar %v4float %1414 %1413
%1416 = OpLoad %v4float %1333
%1417 = OpCompositeExtract %float %1416 3
%1418 = OpLoad %v4float %1334
%1419 = OpVectorTimesScalar %v4float %1418 %1417
%1420 = OpFAdd %v4float %1415 %1419
OpReturnValue %1420
%1349 = OpLabel
%1421 = OpLoad %v4float %1334
%1422 = OpCompositeExtract %float %1421 3
%1423 = OpFSub %float %float_1 %1422
%1424 = OpLoad %v4float %1333
%1425 = OpVectorTimesScalar %v4float %1424 %1423
%1426 = OpLoad %v4float %1333
%1427 = OpCompositeExtract %float %1426 3
%1428 = OpFSub %float %float_1 %1427
%1429 = OpLoad %v4float %1334
%1430 = OpVectorTimesScalar %v4float %1429 %1428
%1431 = OpFAdd %v4float %1425 %1430
OpReturnValue %1431
%1350 = OpLabel
%1433 = OpLoad %v4float %1333
%1434 = OpLoad %v4float %1334
%1435 = OpFAdd %v4float %1433 %1434
%1436 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1432 = OpExtInst %v4float %1 FMin %1435 %1436
OpReturnValue %1432
%1351 = OpLabel
%1437 = OpLoad %v4float %1333
%1438 = OpLoad %v4float %1334
%1439 = OpFMul %v4float %1437 %1438
OpReturnValue %1439
%1352 = OpLabel
%1440 = OpLoad %v4float %1333
%1441 = OpLoad %v4float %1333
%1442 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%1443 = OpFSub %v4float %1442 %1441
%1444 = OpLoad %v4float %1334
%1445 = OpFMul %v4float %1443 %1444
%1446 = OpFAdd %v4float %1440 %1445
OpReturnValue %1446
%1353 = OpLabel
%1447 = OpLoad %v4float %1333
OpStore %1448 %1447
%1449 = OpLoad %v4float %1334
OpStore %1450 %1449
%1451 = OpFunctionCall %v4float %blend_overlay %1448 %1450
OpReturnValue %1451
%1354 = OpLabel
%1453 = OpLoad %v4float %1333
%1454 = OpLoad %v4float %1333
%1455 = OpCompositeExtract %float %1454 3
%1456 = OpFSub %float %float_1 %1455
%1457 = OpLoad %v4float %1334
%1458 = OpVectorTimesScalar %v4float %1457 %1456
%1459 = OpFAdd %v4float %1453 %1458
OpStore %_32_result %1459
%1461 = OpLoad %v4float %_32_result
%1462 = OpVectorShuffle %v3float %1461 %1461 0 1 2
%1463 = OpLoad %v4float %1334
%1464 = OpCompositeExtract %float %1463 3
%1465 = OpFSub %float %float_1 %1464
%1466 = OpLoad %v4float %1333
%1467 = OpVectorShuffle %v3float %1466 %1466 0 1 2
%1468 = OpVectorTimesScalar %v3float %1467 %1465
%1469 = OpLoad %v4float %1334
%1470 = OpVectorShuffle %v3float %1469 %1469 0 1 2
%1471 = OpFAdd %v3float %1468 %1470
%1460 = OpExtInst %v3float %1 FMin %1462 %1471
%1472 = OpLoad %v4float %_32_result
%1473 = OpVectorShuffle %v4float %1472 %1460 4 5 6 3
OpStore %_32_result %1473
%1474 = OpLoad %v4float %_32_result
OpReturnValue %1474
%1355 = OpLabel
%1476 = OpLoad %v4float %1333
%1477 = OpLoad %v4float %1333
%1478 = OpCompositeExtract %float %1477 3
%1479 = OpFSub %float %float_1 %1478
%1480 = OpLoad %v4float %1334
%1481 = OpVectorTimesScalar %v4float %1480 %1479
%1482 = OpFAdd %v4float %1476 %1481
OpStore %_35_result %1482
%1484 = OpLoad %v4float %_35_result
%1485 = OpVectorShuffle %v3float %1484 %1484 0 1 2
%1486 = OpLoad %v4float %1334
%1487 = OpCompositeExtract %float %1486 3
%1488 = OpFSub %float %float_1 %1487
%1489 = OpLoad %v4float %1333
%1490 = OpVectorShuffle %v3float %1489 %1489 0 1 2
%1491 = OpVectorTimesScalar %v3float %1490 %1488
%1492 = OpLoad %v4float %1334
%1493 = OpVectorShuffle %v3float %1492 %1492 0 1 2
%1494 = OpFAdd %v3float %1491 %1493
%1483 = OpExtInst %v3float %1 FMax %1485 %1494
%1495 = OpLoad %v4float %_35_result
%1496 = OpVectorShuffle %v4float %1495 %1483 4 5 6 3
OpStore %_35_result %1496
%1497 = OpLoad %v4float %_35_result
OpReturnValue %1497
%1356 = OpLabel
%1498 = OpLoad %v4float %1333
%1499 = OpVectorShuffle %v2float %1498 %1498 0 3
OpStore %1500 %1499
%1501 = OpLoad %v4float %1334
%1502 = OpVectorShuffle %v2float %1501 %1501 0 3
OpStore %1503 %1502
%1504 = OpFunctionCall %float %_color_dodge_component %1500 %1503
%1505 = OpLoad %v4float %1333
%1506 = OpVectorShuffle %v2float %1505 %1505 1 3
OpStore %1507 %1506
%1508 = OpLoad %v4float %1334
%1509 = OpVectorShuffle %v2float %1508 %1508 1 3
OpStore %1510 %1509
%1511 = OpFunctionCall %float %_color_dodge_component %1507 %1510
%1512 = OpLoad %v4float %1333
%1513 = OpVectorShuffle %v2float %1512 %1512 2 3
OpStore %1514 %1513
%1515 = OpLoad %v4float %1334
%1516 = OpVectorShuffle %v2float %1515 %1515 2 3
OpStore %1517 %1516
%1518 = OpFunctionCall %float %_color_dodge_component %1514 %1517
%1519 = OpLoad %v4float %1333
%1520 = OpCompositeExtract %float %1519 3
%1521 = OpLoad %v4float %1333
%1522 = OpCompositeExtract %float %1521 3
%1523 = OpFSub %float %float_1 %1522
%1524 = OpLoad %v4float %1334
%1525 = OpCompositeExtract %float %1524 3
%1526 = OpFMul %float %1523 %1525
%1527 = OpFAdd %float %1520 %1526
%1528 = OpCompositeConstruct %v4float %1504 %1511 %1518 %1527
OpReturnValue %1528
%1357 = OpLabel
%1529 = OpLoad %v4float %1333
%1530 = OpVectorShuffle %v2float %1529 %1529 0 3
OpStore %1531 %1530
%1532 = OpLoad %v4float %1334
%1533 = OpVectorShuffle %v2float %1532 %1532 0 3
OpStore %1534 %1533
%1535 = OpFunctionCall %float %_color_burn_component %1531 %1534
%1536 = OpLoad %v4float %1333
%1537 = OpVectorShuffle %v2float %1536 %1536 1 3
OpStore %1538 %1537
%1539 = OpLoad %v4float %1334
%1540 = OpVectorShuffle %v2float %1539 %1539 1 3
OpStore %1541 %1540
%1542 = OpFunctionCall %float %_color_burn_component %1538 %1541
%1543 = OpLoad %v4float %1333
%1544 = OpVectorShuffle %v2float %1543 %1543 2 3
OpStore %1545 %1544
%1546 = OpLoad %v4float %1334
%1547 = OpVectorShuffle %v2float %1546 %1546 2 3
OpStore %1548 %1547
%1549 = OpFunctionCall %float %_color_burn_component %1545 %1548
%1550 = OpLoad %v4float %1333
%1551 = OpCompositeExtract %float %1550 3
%1552 = OpLoad %v4float %1333
%1553 = OpCompositeExtract %float %1552 3
%1554 = OpFSub %float %float_1 %1553
%1555 = OpLoad %v4float %1334
%1556 = OpCompositeExtract %float %1555 3
%1557 = OpFMul %float %1554 %1556
%1558 = OpFAdd %float %1551 %1557
%1559 = OpCompositeConstruct %v4float %1535 %1542 %1549 %1558
OpReturnValue %1559
%1358 = OpLabel
%1560 = OpLoad %v4float %1334
OpStore %1561 %1560
%1562 = OpLoad %v4float %1333
OpStore %1563 %1562
%1564 = OpFunctionCall %v4float %blend_overlay %1561 %1563
OpReturnValue %1564
%1359 = OpLabel
%1565 = OpLoad %v4float %1334
%1566 = OpCompositeExtract %float %1565 3
%1567 = OpFOrdEqual %bool %1566 %float_0
OpSelectionMerge %1571 None
OpBranchConditional %1567 %1569 %1570
%1569 = OpLabel
%1572 = OpLoad %v4float %1333
OpStore %1568 %1572
OpBranch %1571
%1570 = OpLabel
%1573 = OpLoad %v4float %1333
%1574 = OpVectorShuffle %v2float %1573 %1573 0 3
OpStore %1575 %1574
%1576 = OpLoad %v4float %1334
%1577 = OpVectorShuffle %v2float %1576 %1576 0 3
OpStore %1578 %1577
%1579 = OpFunctionCall %float %_soft_light_component %1575 %1578
%1580 = OpLoad %v4float %1333
%1581 = OpVectorShuffle %v2float %1580 %1580 1 3
OpStore %1582 %1581
%1583 = OpLoad %v4float %1334
%1584 = OpVectorShuffle %v2float %1583 %1583 1 3
OpStore %1585 %1584
%1586 = OpFunctionCall %float %_soft_light_component %1582 %1585
%1587 = OpLoad %v4float %1333
%1588 = OpVectorShuffle %v2float %1587 %1587 2 3
OpStore %1589 %1588
%1590 = OpLoad %v4float %1334
%1591 = OpVectorShuffle %v2float %1590 %1590 2 3
OpStore %1592 %1591
%1593 = OpFunctionCall %float %_soft_light_component %1589 %1592
%1594 = OpLoad %v4float %1333
%1595 = OpCompositeExtract %float %1594 3
%1596 = OpLoad %v4float %1333
%1597 = OpCompositeExtract %float %1596 3
%1598 = OpFSub %float %float_1 %1597
%1599 = OpLoad %v4float %1334
%1600 = OpCompositeExtract %float %1599 3
%1601 = OpFMul %float %1598 %1600
%1602 = OpFAdd %float %1595 %1601
%1603 = OpCompositeConstruct %v4float %1579 %1586 %1593 %1602
OpStore %1568 %1603
OpBranch %1571
%1571 = OpLabel
%1604 = OpLoad %v4float %1568
OpReturnValue %1604
%1360 = OpLabel
%1605 = OpLoad %v4float %1333
%1606 = OpVectorShuffle %v3float %1605 %1605 0 1 2
%1607 = OpLoad %v4float %1334
%1608 = OpVectorShuffle %v3float %1607 %1607 0 1 2
%1609 = OpFAdd %v3float %1606 %1608
%1611 = OpLoad %v4float %1333
%1612 = OpVectorShuffle %v3float %1611 %1611 0 1 2
%1613 = OpLoad %v4float %1334
%1614 = OpCompositeExtract %float %1613 3
%1615 = OpVectorTimesScalar %v3float %1612 %1614
%1616 = OpLoad %v4float %1334
%1617 = OpVectorShuffle %v3float %1616 %1616 0 1 2
%1618 = OpLoad %v4float %1333
%1619 = OpCompositeExtract %float %1618 3
%1620 = OpVectorTimesScalar %v3float %1617 %1619
%1610 = OpExtInst %v3float %1 FMin %1615 %1620
%1621 = OpVectorTimesScalar %v3float %1610 %float_2
%1622 = OpFSub %v3float %1609 %1621
%1623 = OpCompositeExtract %float %1622 0
%1624 = OpCompositeExtract %float %1622 1
%1625 = OpCompositeExtract %float %1622 2
%1626 = OpLoad %v4float %1333
%1627 = OpCompositeExtract %float %1626 3
%1628 = OpLoad %v4float %1333
%1629 = OpCompositeExtract %float %1628 3
%1630 = OpFSub %float %float_1 %1629
%1631 = OpLoad %v4float %1334
%1632 = OpCompositeExtract %float %1631 3
%1633 = OpFMul %float %1630 %1632
%1634 = OpFAdd %float %1627 %1633
%1635 = OpCompositeConstruct %v4float %1623 %1624 %1625 %1634
OpReturnValue %1635
%1361 = OpLabel
%1636 = OpLoad %v4float %1334
%1637 = OpVectorShuffle %v3float %1636 %1636 0 1 2
%1638 = OpLoad %v4float %1333
%1639 = OpVectorShuffle %v3float %1638 %1638 0 1 2
%1640 = OpFAdd %v3float %1637 %1639
%1641 = OpLoad %v4float %1334
%1642 = OpVectorShuffle %v3float %1641 %1641 0 1 2
%1643 = OpVectorTimesScalar %v3float %1642 %float_2
%1644 = OpLoad %v4float %1333
%1645 = OpVectorShuffle %v3float %1644 %1644 0 1 2
%1646 = OpFMul %v3float %1643 %1645
%1647 = OpFSub %v3float %1640 %1646
%1648 = OpCompositeExtract %float %1647 0
%1649 = OpCompositeExtract %float %1647 1
%1650 = OpCompositeExtract %float %1647 2
%1651 = OpLoad %v4float %1333
%1652 = OpCompositeExtract %float %1651 3
%1653 = OpLoad %v4float %1333
%1654 = OpCompositeExtract %float %1653 3
%1655 = OpFSub %float %float_1 %1654
%1656 = OpLoad %v4float %1334
%1657 = OpCompositeExtract %float %1656 3
%1658 = OpFMul %float %1655 %1657
%1659 = OpFAdd %float %1652 %1658
%1660 = OpCompositeConstruct %v4float %1648 %1649 %1650 %1659
OpReturnValue %1660
%1362 = OpLabel
%1661 = OpLoad %v4float %1333
%1662 = OpCompositeExtract %float %1661 3
%1663 = OpFSub %float %float_1 %1662
%1664 = OpLoad %v4float %1334
%1665 = OpVectorShuffle %v3float %1664 %1664 0 1 2
%1666 = OpVectorTimesScalar %v3float %1665 %1663
%1667 = OpLoad %v4float %1334
%1668 = OpCompositeExtract %float %1667 3
%1669 = OpFSub %float %float_1 %1668
%1670 = OpLoad %v4float %1333
%1671 = OpVectorShuffle %v3float %1670 %1670 0 1 2
%1672 = OpVectorTimesScalar %v3float %1671 %1669
%1673 = OpFAdd %v3float %1666 %1672
%1674 = OpLoad %v4float %1333
%1675 = OpVectorShuffle %v3float %1674 %1674 0 1 2
%1676 = OpLoad %v4float %1334
%1677 = OpVectorShuffle %v3float %1676 %1676 0 1 2
%1678 = OpFMul %v3float %1675 %1677
%1679 = OpFAdd %v3float %1673 %1678
%1680 = OpCompositeExtract %float %1679 0
%1681 = OpCompositeExtract %float %1679 1
%1682 = OpCompositeExtract %float %1679 2
%1683 = OpLoad %v4float %1333
%1684 = OpCompositeExtract %float %1683 3
%1685 = OpLoad %v4float %1333
%1686 = OpCompositeExtract %float %1685 3
%1687 = OpFSub %float %float_1 %1686
%1688 = OpLoad %v4float %1334
%1689 = OpCompositeExtract %float %1688 3
%1690 = OpFMul %float %1687 %1689
%1691 = OpFAdd %float %1684 %1690
%1692 = OpCompositeConstruct %v4float %1680 %1681 %1682 %1691
OpReturnValue %1692
%1363 = OpLabel
%1694 = OpLoad %v4float %1334
%1695 = OpCompositeExtract %float %1694 3
%1696 = OpLoad %v4float %1333
%1697 = OpCompositeExtract %float %1696 3
%1698 = OpFMul %float %1695 %1697
OpStore %_44_alpha %1698
%1700 = OpLoad %v4float %1333
%1701 = OpVectorShuffle %v3float %1700 %1700 0 1 2
%1702 = OpLoad %v4float %1334
%1703 = OpCompositeExtract %float %1702 3
%1704 = OpVectorTimesScalar %v3float %1701 %1703
OpStore %_45_sda %1704
%1706 = OpLoad %v4float %1334
%1707 = OpVectorShuffle %v3float %1706 %1706 0 1 2
%1708 = OpLoad %v4float %1333
%1709 = OpCompositeExtract %float %1708 3
%1710 = OpVectorTimesScalar %v3float %1707 %1709
OpStore %_46_dsa %1710
%1711 = OpLoad %v3float %_45_sda
OpStore %1712 %1711
%1713 = OpLoad %v3float %_46_dsa
OpStore %1714 %1713
%1715 = OpFunctionCall %v3float %_blend_set_color_saturation %1712 %1714
OpStore %1716 %1715
%1717 = OpLoad %float %_44_alpha
OpStore %1718 %1717
%1719 = OpLoad %v3float %_46_dsa
OpStore %1720 %1719
%1721 = OpFunctionCall %v3float %_blend_set_color_luminance %1716 %1718 %1720
%1722 = OpLoad %v4float %1334
%1723 = OpVectorShuffle %v3float %1722 %1722 0 1 2
%1724 = OpFAdd %v3float %1721 %1723
%1725 = OpLoad %v3float %_46_dsa
%1726 = OpFSub %v3float %1724 %1725
%1727 = OpLoad %v4float %1333
%1728 = OpVectorShuffle %v3float %1727 %1727 0 1 2
%1729 = OpFAdd %v3float %1726 %1728
%1730 = OpLoad %v3float %_45_sda
%1731 = OpFSub %v3float %1729 %1730
%1732 = OpCompositeExtract %float %1731 0
%1733 = OpCompositeExtract %float %1731 1
%1734 = OpCompositeExtract %float %1731 2
%1735 = OpLoad %v4float %1333
%1736 = OpCompositeExtract %float %1735 3
%1737 = OpLoad %v4float %1334
%1738 = OpCompositeExtract %float %1737 3
%1739 = OpFAdd %float %1736 %1738
%1740 = OpLoad %float %_44_alpha
%1741 = OpFSub %float %1739 %1740
%1742 = OpCompositeConstruct %v4float %1732 %1733 %1734 %1741
OpReturnValue %1742
%1364 = OpLabel
%1744 = OpLoad %v4float %1334
%1745 = OpCompositeExtract %float %1744 3
%1746 = OpLoad %v4float %1333
%1747 = OpCompositeExtract %float %1746 3
%1748 = OpFMul %float %1745 %1747
OpStore %_48_alpha %1748
%1750 = OpLoad %v4float %1333
%1751 = OpVectorShuffle %v3float %1750 %1750 0 1 2
%1752 = OpLoad %v4float %1334
%1753 = OpCompositeExtract %float %1752 3
%1754 = OpVectorTimesScalar %v3float %1751 %1753
OpStore %_49_sda %1754
%1756 = OpLoad %v4float %1334
%1757 = OpVectorShuffle %v3float %1756 %1756 0 1 2
%1758 = OpLoad %v4float %1333
%1759 = OpCompositeExtract %float %1758 3
%1760 = OpVectorTimesScalar %v3float %1757 %1759
OpStore %_50_dsa %1760
%1761 = OpLoad %v3float %_50_dsa
OpStore %1762 %1761
%1763 = OpLoad %v3float %_49_sda
OpStore %1764 %1763
%1765 = OpFunctionCall %v3float %_blend_set_color_saturation %1762 %1764
OpStore %1766 %1765
%1767 = OpLoad %float %_48_alpha
OpStore %1768 %1767
%1769 = OpLoad %v3float %_50_dsa
OpStore %1770 %1769
%1771 = OpFunctionCall %v3float %_blend_set_color_luminance %1766 %1768 %1770
%1772 = OpLoad %v4float %1334
%1773 = OpVectorShuffle %v3float %1772 %1772 0 1 2
%1774 = OpFAdd %v3float %1771 %1773
%1775 = OpLoad %v3float %_50_dsa
%1776 = OpFSub %v3float %1774 %1775
%1777 = OpLoad %v4float %1333
%1778 = OpVectorShuffle %v3float %1777 %1777 0 1 2
%1779 = OpFAdd %v3float %1776 %1778
%1780 = OpLoad %v3float %_49_sda
%1781 = OpFSub %v3float %1779 %1780
%1782 = OpCompositeExtract %float %1781 0
%1783 = OpCompositeExtract %float %1781 1
%1784 = OpCompositeExtract %float %1781 2
%1785 = OpLoad %v4float %1333
%1786 = OpCompositeExtract %float %1785 3
%1787 = OpLoad %v4float %1334
%1788 = OpCompositeExtract %float %1787 3
%1789 = OpFAdd %float %1786 %1788
%1790 = OpLoad %float %_48_alpha
%1791 = OpFSub %float %1789 %1790
%1792 = OpCompositeConstruct %v4float %1782 %1783 %1784 %1791
OpReturnValue %1792
%1365 = OpLabel
%1794 = OpLoad %v4float %1334
%1795 = OpCompositeExtract %float %1794 3
%1796 = OpLoad %v4float %1333
%1797 = OpCompositeExtract %float %1796 3
%1798 = OpFMul %float %1795 %1797
OpStore %_52_alpha %1798
%1800 = OpLoad %v4float %1333
%1801 = OpVectorShuffle %v3float %1800 %1800 0 1 2
%1802 = OpLoad %v4float %1334
%1803 = OpCompositeExtract %float %1802 3
%1804 = OpVectorTimesScalar %v3float %1801 %1803
OpStore %_53_sda %1804
%1806 = OpLoad %v4float %1334
%1807 = OpVectorShuffle %v3float %1806 %1806 0 1 2
%1808 = OpLoad %v4float %1333
%1809 = OpCompositeExtract %float %1808 3
%1810 = OpVectorTimesScalar %v3float %1807 %1809
OpStore %_54_dsa %1810
%1811 = OpLoad %v3float %_53_sda
OpStore %1812 %1811
%1813 = OpLoad %float %_52_alpha
OpStore %1814 %1813
%1815 = OpLoad %v3float %_54_dsa
OpStore %1816 %1815
%1817 = OpFunctionCall %v3float %_blend_set_color_luminance %1812 %1814 %1816
%1818 = OpLoad %v4float %1334
%1819 = OpVectorShuffle %v3float %1818 %1818 0 1 2
%1820 = OpFAdd %v3float %1817 %1819
%1821 = OpLoad %v3float %_54_dsa
%1822 = OpFSub %v3float %1820 %1821
%1823 = OpLoad %v4float %1333
%1824 = OpVectorShuffle %v3float %1823 %1823 0 1 2
%1825 = OpFAdd %v3float %1822 %1824
%1826 = OpLoad %v3float %_53_sda
%1827 = OpFSub %v3float %1825 %1826
%1828 = OpCompositeExtract %float %1827 0
%1829 = OpCompositeExtract %float %1827 1
%1830 = OpCompositeExtract %float %1827 2
%1831 = OpLoad %v4float %1333
%1832 = OpCompositeExtract %float %1831 3
%1833 = OpLoad %v4float %1334
%1834 = OpCompositeExtract %float %1833 3
%1835 = OpFAdd %float %1832 %1834
%1836 = OpLoad %float %_52_alpha
%1837 = OpFSub %float %1835 %1836
%1838 = OpCompositeConstruct %v4float %1828 %1829 %1830 %1837
OpReturnValue %1838
%1366 = OpLabel
%1840 = OpLoad %v4float %1334
%1841 = OpCompositeExtract %float %1840 3
%1842 = OpLoad %v4float %1333
%1843 = OpCompositeExtract %float %1842 3
%1844 = OpFMul %float %1841 %1843
OpStore %_56_alpha %1844
%1846 = OpLoad %v4float %1333
%1847 = OpVectorShuffle %v3float %1846 %1846 0 1 2
%1848 = OpLoad %v4float %1334
%1849 = OpCompositeExtract %float %1848 3
%1850 = OpVectorTimesScalar %v3float %1847 %1849
OpStore %_57_sda %1850
%1852 = OpLoad %v4float %1334
%1853 = OpVectorShuffle %v3float %1852 %1852 0 1 2
%1854 = OpLoad %v4float %1333
%1855 = OpCompositeExtract %float %1854 3
%1856 = OpVectorTimesScalar %v3float %1853 %1855
OpStore %_58_dsa %1856
%1857 = OpLoad %v3float %_58_dsa
OpStore %1858 %1857
%1859 = OpLoad %float %_56_alpha
OpStore %1860 %1859
%1861 = OpLoad %v3float %_57_sda
OpStore %1862 %1861
%1863 = OpFunctionCall %v3float %_blend_set_color_luminance %1858 %1860 %1862
%1864 = OpLoad %v4float %1334
%1865 = OpVectorShuffle %v3float %1864 %1864 0 1 2
%1866 = OpFAdd %v3float %1863 %1865
%1867 = OpLoad %v3float %_58_dsa
%1868 = OpFSub %v3float %1866 %1867
%1869 = OpLoad %v4float %1333
%1870 = OpVectorShuffle %v3float %1869 %1869 0 1 2
%1871 = OpFAdd %v3float %1868 %1870
%1872 = OpLoad %v3float %_57_sda
%1873 = OpFSub %v3float %1871 %1872
%1874 = OpCompositeExtract %float %1873 0
%1875 = OpCompositeExtract %float %1873 1
%1876 = OpCompositeExtract %float %1873 2
%1877 = OpLoad %v4float %1333
%1878 = OpCompositeExtract %float %1877 3
%1879 = OpLoad %v4float %1334
%1880 = OpCompositeExtract %float %1879 3
%1881 = OpFAdd %float %1878 %1880
%1882 = OpLoad %float %_56_alpha
%1883 = OpFSub %float %1881 %1882
%1884 = OpCompositeConstruct %v4float %1874 %1875 %1876 %1883
OpReturnValue %1884
%1337 = OpLabel
OpReturnValue %59
OpFunctionEnd
%main = OpFunction %void None %1886
%1887 = OpLabel
%1889 = OpVariable %_ptr_Function_int Function
%1891 = OpVariable %_ptr_Function_v4float Function
%1893 = OpVariable %_ptr_Function_v4float Function
OpStore %1889 %int_13
%1890 = OpLoad %v4float %src
OpStore %1891 %1890
%1892 = OpLoad %v4float %dst
OpStore %1893 %1892
%1894 = OpFunctionCall %v4float %blend %1889 %1891 %1893
OpStore %sk_FragColor %1894
OpReturn
OpFunctionEnd
