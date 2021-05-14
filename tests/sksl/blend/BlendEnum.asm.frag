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
OpName %_blend_overlay_component_hh2h2 "_blend_overlay_component_hh2h2"
OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"
OpName %result "result"
OpName %_color_dodge_component_hh2h2 "_color_dodge_component_hh2h2"
OpName %delta "delta"
OpName %_color_burn_component_hh2h2 "_color_burn_component_hh2h2"
OpName %delta_0 "delta"
OpName %_soft_light_component_hh2h2 "_soft_light_component_hh2h2"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %_blend_set_color_luminance_h3h3hh3 "_blend_set_color_luminance_h3h3hh3"
OpName %lum "lum"
OpName %result_0 "result"
OpName %minComp "minComp"
OpName %maxComp "maxComp"
OpName %_blend_set_color_saturation_helper_h3h3h "_blend_set_color_saturation_helper_h3h3h"
OpName %_blend_set_color_saturation_h3h3h3 "_blend_set_color_saturation_h3h3h3"
OpName %sat "sat"
OpName %blend_h4eh4h4 "blend_h4eh4h4"
OpName %_0_result "_0_result"
OpName %_1_result "_1_result"
OpName %_2_alpha "_2_alpha"
OpName %_3_sda "_3_sda"
OpName %_4_dsa "_4_dsa"
OpName %_5_alpha "_5_alpha"
OpName %_6_sda "_6_sda"
OpName %_7_dsa "_7_dsa"
OpName %_8_alpha "_8_alpha"
OpName %_9_sda "_9_sda"
OpName %_10_dsa "_10_dsa"
OpName %_11_alpha "_11_alpha"
OpName %_12_sda "_12_sda"
OpName %_13_dsa "_13_dsa"
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
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %delta_0 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %lum RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %result_0 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %510 RelaxedPrecision
OpDecorate %511 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %539 RelaxedPrecision
OpDecorate %540 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %542 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %545 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %548 RelaxedPrecision
OpDecorate %549 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %561 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %570 RelaxedPrecision
OpDecorate %571 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %573 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %577 RelaxedPrecision
OpDecorate %578 RelaxedPrecision
OpDecorate %579 RelaxedPrecision
OpDecorate %584 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %592 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %597 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %599 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %608 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %613 RelaxedPrecision
OpDecorate %615 RelaxedPrecision
OpDecorate %618 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %627 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %633 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %636 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %642 RelaxedPrecision
OpDecorate %643 RelaxedPrecision
OpDecorate %645 RelaxedPrecision
OpDecorate %648 RelaxedPrecision
OpDecorate %649 RelaxedPrecision
OpDecorate %650 RelaxedPrecision
OpDecorate %652 RelaxedPrecision
OpDecorate %655 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %695 RelaxedPrecision
OpDecorate %696 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
OpDecorate %699 RelaxedPrecision
OpDecorate %700 RelaxedPrecision
OpDecorate %701 RelaxedPrecision
OpDecorate %702 RelaxedPrecision
OpDecorate %703 RelaxedPrecision
OpDecorate %704 RelaxedPrecision
OpDecorate %705 RelaxedPrecision
OpDecorate %706 RelaxedPrecision
OpDecorate %707 RelaxedPrecision
OpDecorate %708 RelaxedPrecision
OpDecorate %709 RelaxedPrecision
OpDecorate %710 RelaxedPrecision
OpDecorate %711 RelaxedPrecision
OpDecorate %712 RelaxedPrecision
OpDecorate %713 RelaxedPrecision
OpDecorate %714 RelaxedPrecision
OpDecorate %715 RelaxedPrecision
OpDecorate %716 RelaxedPrecision
OpDecorate %717 RelaxedPrecision
OpDecorate %718 RelaxedPrecision
OpDecorate %719 RelaxedPrecision
OpDecorate %720 RelaxedPrecision
OpDecorate %721 RelaxedPrecision
OpDecorate %722 RelaxedPrecision
OpDecorate %723 RelaxedPrecision
OpDecorate %724 RelaxedPrecision
OpDecorate %725 RelaxedPrecision
OpDecorate %726 RelaxedPrecision
OpDecorate %727 RelaxedPrecision
OpDecorate %728 RelaxedPrecision
OpDecorate %729 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %731 RelaxedPrecision
OpDecorate %732 RelaxedPrecision
OpDecorate %733 RelaxedPrecision
OpDecorate %734 RelaxedPrecision
OpDecorate %735 RelaxedPrecision
OpDecorate %736 RelaxedPrecision
OpDecorate %737 RelaxedPrecision
OpDecorate %738 RelaxedPrecision
OpDecorate %739 RelaxedPrecision
OpDecorate %740 RelaxedPrecision
OpDecorate %741 RelaxedPrecision
OpDecorate %742 RelaxedPrecision
OpDecorate %743 RelaxedPrecision
OpDecorate %744 RelaxedPrecision
OpDecorate %745 RelaxedPrecision
OpDecorate %746 RelaxedPrecision
OpDecorate %747 RelaxedPrecision
OpDecorate %748 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %750 RelaxedPrecision
OpDecorate %751 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
OpDecorate %754 RelaxedPrecision
OpDecorate %755 RelaxedPrecision
OpDecorate %756 RelaxedPrecision
OpDecorate %757 RelaxedPrecision
OpDecorate %758 RelaxedPrecision
OpDecorate %759 RelaxedPrecision
OpDecorate %760 RelaxedPrecision
OpDecorate %762 RelaxedPrecision
OpDecorate %763 RelaxedPrecision
OpDecorate %764 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
OpDecorate %766 RelaxedPrecision
OpDecorate %767 RelaxedPrecision
OpDecorate %768 RelaxedPrecision
OpDecorate %769 RelaxedPrecision
OpDecorate %770 RelaxedPrecision
OpDecorate %771 RelaxedPrecision
OpDecorate %772 RelaxedPrecision
OpDecorate %773 RelaxedPrecision
OpDecorate %774 RelaxedPrecision
OpDecorate %775 RelaxedPrecision
OpDecorate %776 RelaxedPrecision
OpDecorate %778 RelaxedPrecision
OpDecorate %_0_result RelaxedPrecision
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
OpDecorate %795 RelaxedPrecision
OpDecorate %796 RelaxedPrecision
OpDecorate %797 RelaxedPrecision
OpDecorate %798 RelaxedPrecision
OpDecorate %799 RelaxedPrecision
OpDecorate %800 RelaxedPrecision
OpDecorate %801 RelaxedPrecision
OpDecorate %802 RelaxedPrecision
OpDecorate %803 RelaxedPrecision
OpDecorate %_1_result RelaxedPrecision
OpDecorate %805 RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %807 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %810 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %813 RelaxedPrecision
OpDecorate %814 RelaxedPrecision
OpDecorate %815 RelaxedPrecision
OpDecorate %816 RelaxedPrecision
OpDecorate %817 RelaxedPrecision
OpDecorate %818 RelaxedPrecision
OpDecorate %819 RelaxedPrecision
OpDecorate %820 RelaxedPrecision
OpDecorate %821 RelaxedPrecision
OpDecorate %822 RelaxedPrecision
OpDecorate %823 RelaxedPrecision
OpDecorate %824 RelaxedPrecision
OpDecorate %825 RelaxedPrecision
OpDecorate %826 RelaxedPrecision
OpDecorate %827 RelaxedPrecision
OpDecorate %828 RelaxedPrecision
OpDecorate %830 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %834 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %837 RelaxedPrecision
OpDecorate %838 RelaxedPrecision
OpDecorate %841 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %844 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %848 RelaxedPrecision
OpDecorate %849 RelaxedPrecision
OpDecorate %850 RelaxedPrecision
OpDecorate %851 RelaxedPrecision
OpDecorate %852 RelaxedPrecision
OpDecorate %853 RelaxedPrecision
OpDecorate %854 RelaxedPrecision
OpDecorate %855 RelaxedPrecision
OpDecorate %856 RelaxedPrecision
OpDecorate %857 RelaxedPrecision
OpDecorate %858 RelaxedPrecision
OpDecorate %859 RelaxedPrecision
OpDecorate %861 RelaxedPrecision
OpDecorate %862 RelaxedPrecision
OpDecorate %865 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %868 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %872 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %875 RelaxedPrecision
OpDecorate %876 RelaxedPrecision
OpDecorate %879 RelaxedPrecision
OpDecorate %880 RelaxedPrecision
OpDecorate %881 RelaxedPrecision
OpDecorate %882 RelaxedPrecision
OpDecorate %883 RelaxedPrecision
OpDecorate %884 RelaxedPrecision
OpDecorate %885 RelaxedPrecision
OpDecorate %886 RelaxedPrecision
OpDecorate %887 RelaxedPrecision
OpDecorate %888 RelaxedPrecision
OpDecorate %889 RelaxedPrecision
OpDecorate %891 RelaxedPrecision
OpDecorate %894 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %901 RelaxedPrecision
OpDecorate %902 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %905 RelaxedPrecision
OpDecorate %906 RelaxedPrecision
OpDecorate %909 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %912 RelaxedPrecision
OpDecorate %913 RelaxedPrecision
OpDecorate %916 RelaxedPrecision
OpDecorate %917 RelaxedPrecision
OpDecorate %919 RelaxedPrecision
OpDecorate %920 RelaxedPrecision
OpDecorate %923 RelaxedPrecision
OpDecorate %924 RelaxedPrecision
OpDecorate %925 RelaxedPrecision
OpDecorate %926 RelaxedPrecision
OpDecorate %927 RelaxedPrecision
OpDecorate %928 RelaxedPrecision
OpDecorate %929 RelaxedPrecision
OpDecorate %930 RelaxedPrecision
OpDecorate %931 RelaxedPrecision
OpDecorate %932 RelaxedPrecision
OpDecorate %933 RelaxedPrecision
OpDecorate %934 RelaxedPrecision
OpDecorate %935 RelaxedPrecision
OpDecorate %936 RelaxedPrecision
OpDecorate %937 RelaxedPrecision
OpDecorate %938 RelaxedPrecision
OpDecorate %940 RelaxedPrecision
OpDecorate %941 RelaxedPrecision
OpDecorate %942 RelaxedPrecision
OpDecorate %943 RelaxedPrecision
OpDecorate %944 RelaxedPrecision
OpDecorate %945 RelaxedPrecision
OpDecorate %946 RelaxedPrecision
OpDecorate %947 RelaxedPrecision
OpDecorate %948 RelaxedPrecision
OpDecorate %949 RelaxedPrecision
OpDecorate %950 RelaxedPrecision
OpDecorate %951 RelaxedPrecision
OpDecorate %952 RelaxedPrecision
OpDecorate %953 RelaxedPrecision
OpDecorate %954 RelaxedPrecision
OpDecorate %955 RelaxedPrecision
OpDecorate %956 RelaxedPrecision
OpDecorate %957 RelaxedPrecision
OpDecorate %958 RelaxedPrecision
OpDecorate %959 RelaxedPrecision
OpDecorate %960 RelaxedPrecision
OpDecorate %961 RelaxedPrecision
OpDecorate %962 RelaxedPrecision
OpDecorate %963 RelaxedPrecision
OpDecorate %964 RelaxedPrecision
OpDecorate %965 RelaxedPrecision
OpDecorate %966 RelaxedPrecision
OpDecorate %967 RelaxedPrecision
OpDecorate %968 RelaxedPrecision
OpDecorate %969 RelaxedPrecision
OpDecorate %970 RelaxedPrecision
OpDecorate %971 RelaxedPrecision
OpDecorate %972 RelaxedPrecision
OpDecorate %973 RelaxedPrecision
OpDecorate %974 RelaxedPrecision
OpDecorate %975 RelaxedPrecision
OpDecorate %976 RelaxedPrecision
OpDecorate %977 RelaxedPrecision
OpDecorate %978 RelaxedPrecision
OpDecorate %979 RelaxedPrecision
OpDecorate %980 RelaxedPrecision
OpDecorate %981 RelaxedPrecision
OpDecorate %982 RelaxedPrecision
OpDecorate %983 RelaxedPrecision
OpDecorate %984 RelaxedPrecision
OpDecorate %985 RelaxedPrecision
OpDecorate %986 RelaxedPrecision
OpDecorate %987 RelaxedPrecision
OpDecorate %988 RelaxedPrecision
OpDecorate %989 RelaxedPrecision
OpDecorate %990 RelaxedPrecision
OpDecorate %991 RelaxedPrecision
OpDecorate %992 RelaxedPrecision
OpDecorate %993 RelaxedPrecision
OpDecorate %994 RelaxedPrecision
OpDecorate %995 RelaxedPrecision
OpDecorate %996 RelaxedPrecision
OpDecorate %997 RelaxedPrecision
OpDecorate %998 RelaxedPrecision
OpDecorate %999 RelaxedPrecision
OpDecorate %1000 RelaxedPrecision
OpDecorate %1001 RelaxedPrecision
OpDecorate %1002 RelaxedPrecision
OpDecorate %1003 RelaxedPrecision
OpDecorate %1004 RelaxedPrecision
OpDecorate %1005 RelaxedPrecision
OpDecorate %1006 RelaxedPrecision
OpDecorate %1007 RelaxedPrecision
OpDecorate %1008 RelaxedPrecision
OpDecorate %1009 RelaxedPrecision
OpDecorate %1010 RelaxedPrecision
OpDecorate %1011 RelaxedPrecision
OpDecorate %1012 RelaxedPrecision
OpDecorate %1013 RelaxedPrecision
OpDecorate %1014 RelaxedPrecision
OpDecorate %1015 RelaxedPrecision
OpDecorate %1016 RelaxedPrecision
OpDecorate %1017 RelaxedPrecision
OpDecorate %1018 RelaxedPrecision
OpDecorate %1019 RelaxedPrecision
OpDecorate %1020 RelaxedPrecision
OpDecorate %1021 RelaxedPrecision
OpDecorate %_2_alpha RelaxedPrecision
OpDecorate %1023 RelaxedPrecision
OpDecorate %1024 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1026 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %_3_sda RelaxedPrecision
OpDecorate %1029 RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %_4_dsa RelaxedPrecision
OpDecorate %1035 RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1039 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1042 RelaxedPrecision
OpDecorate %1046 RelaxedPrecision
OpDecorate %1048 RelaxedPrecision
OpDecorate %1051 RelaxedPrecision
OpDecorate %1052 RelaxedPrecision
OpDecorate %1053 RelaxedPrecision
OpDecorate %1054 RelaxedPrecision
OpDecorate %1055 RelaxedPrecision
OpDecorate %1056 RelaxedPrecision
OpDecorate %1057 RelaxedPrecision
OpDecorate %1058 RelaxedPrecision
OpDecorate %1059 RelaxedPrecision
OpDecorate %1060 RelaxedPrecision
OpDecorate %1061 RelaxedPrecision
OpDecorate %1062 RelaxedPrecision
OpDecorate %1063 RelaxedPrecision
OpDecorate %1064 RelaxedPrecision
OpDecorate %1065 RelaxedPrecision
OpDecorate %1066 RelaxedPrecision
OpDecorate %1067 RelaxedPrecision
OpDecorate %1068 RelaxedPrecision
OpDecorate %1069 RelaxedPrecision
OpDecorate %1070 RelaxedPrecision
OpDecorate %1071 RelaxedPrecision
OpDecorate %_5_alpha RelaxedPrecision
OpDecorate %1073 RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1075 RelaxedPrecision
OpDecorate %1076 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %_6_sda RelaxedPrecision
OpDecorate %1079 RelaxedPrecision
OpDecorate %1080 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1083 RelaxedPrecision
OpDecorate %_7_dsa RelaxedPrecision
OpDecorate %1085 RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1087 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1089 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1092 RelaxedPrecision
OpDecorate %1096 RelaxedPrecision
OpDecorate %1098 RelaxedPrecision
OpDecorate %1101 RelaxedPrecision
OpDecorate %1102 RelaxedPrecision
OpDecorate %1103 RelaxedPrecision
OpDecorate %1104 RelaxedPrecision
OpDecorate %1105 RelaxedPrecision
OpDecorate %1106 RelaxedPrecision
OpDecorate %1107 RelaxedPrecision
OpDecorate %1108 RelaxedPrecision
OpDecorate %1109 RelaxedPrecision
OpDecorate %1110 RelaxedPrecision
OpDecorate %1111 RelaxedPrecision
OpDecorate %1112 RelaxedPrecision
OpDecorate %1113 RelaxedPrecision
OpDecorate %1114 RelaxedPrecision
OpDecorate %1115 RelaxedPrecision
OpDecorate %1116 RelaxedPrecision
OpDecorate %1117 RelaxedPrecision
OpDecorate %1118 RelaxedPrecision
OpDecorate %1119 RelaxedPrecision
OpDecorate %1120 RelaxedPrecision
OpDecorate %1121 RelaxedPrecision
OpDecorate %_8_alpha RelaxedPrecision
OpDecorate %1123 RelaxedPrecision
OpDecorate %1124 RelaxedPrecision
OpDecorate %1125 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %_9_sda RelaxedPrecision
OpDecorate %1129 RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %_10_dsa RelaxedPrecision
OpDecorate %1135 RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1139 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1142 RelaxedPrecision
OpDecorate %1144 RelaxedPrecision
OpDecorate %1147 RelaxedPrecision
OpDecorate %1148 RelaxedPrecision
OpDecorate %1149 RelaxedPrecision
OpDecorate %1150 RelaxedPrecision
OpDecorate %1151 RelaxedPrecision
OpDecorate %1152 RelaxedPrecision
OpDecorate %1153 RelaxedPrecision
OpDecorate %1154 RelaxedPrecision
OpDecorate %1155 RelaxedPrecision
OpDecorate %1156 RelaxedPrecision
OpDecorate %1157 RelaxedPrecision
OpDecorate %1158 RelaxedPrecision
OpDecorate %1159 RelaxedPrecision
OpDecorate %1160 RelaxedPrecision
OpDecorate %1161 RelaxedPrecision
OpDecorate %1162 RelaxedPrecision
OpDecorate %1163 RelaxedPrecision
OpDecorate %1164 RelaxedPrecision
OpDecorate %1165 RelaxedPrecision
OpDecorate %1166 RelaxedPrecision
OpDecorate %1167 RelaxedPrecision
OpDecorate %_11_alpha RelaxedPrecision
OpDecorate %1169 RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1171 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %_12_sda RelaxedPrecision
OpDecorate %1175 RelaxedPrecision
OpDecorate %1176 RelaxedPrecision
OpDecorate %1177 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1179 RelaxedPrecision
OpDecorate %_13_dsa RelaxedPrecision
OpDecorate %1181 RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1188 RelaxedPrecision
OpDecorate %1190 RelaxedPrecision
OpDecorate %1193 RelaxedPrecision
OpDecorate %1194 RelaxedPrecision
OpDecorate %1195 RelaxedPrecision
OpDecorate %1196 RelaxedPrecision
OpDecorate %1197 RelaxedPrecision
OpDecorate %1198 RelaxedPrecision
OpDecorate %1199 RelaxedPrecision
OpDecorate %1200 RelaxedPrecision
OpDecorate %1201 RelaxedPrecision
OpDecorate %1202 RelaxedPrecision
OpDecorate %1203 RelaxedPrecision
OpDecorate %1204 RelaxedPrecision
OpDecorate %1205 RelaxedPrecision
OpDecorate %1206 RelaxedPrecision
OpDecorate %1207 RelaxedPrecision
OpDecorate %1208 RelaxedPrecision
OpDecorate %1209 RelaxedPrecision
OpDecorate %1210 RelaxedPrecision
OpDecorate %1211 RelaxedPrecision
OpDecorate %1212 RelaxedPrecision
OpDecorate %1213 RelaxedPrecision
OpDecorate %1222 RelaxedPrecision
OpDecorate %1226 RelaxedPrecision
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
%434 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float %_ptr_Function_v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%445 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%526 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%553 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%554 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%657 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%695 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1215 = OpTypeFunction %void
%int_13 = OpConstant %int 13
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_blend_overlay_component_hh2h2 = OpFunction %float None %23
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
%blend_overlay_h4h4h4 = OpFunction %v4float None %65
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
%77 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %73 %76
%78 = OpLoad %v4float %67
%79 = OpVectorShuffle %v2float %78 %78 1 3
OpStore %80 %79
%81 = OpLoad %v4float %68
%82 = OpVectorShuffle %v2float %81 %81 1 3
OpStore %83 %82
%84 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %80 %83
%85 = OpLoad %v4float %67
%86 = OpVectorShuffle %v2float %85 %85 2 3
OpStore %87 %86
%88 = OpLoad %v4float %68
%89 = OpVectorShuffle %v2float %88 %88 2 3
OpStore %90 %89
%91 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %87 %90
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
%_color_dodge_component_hh2h2 = OpFunction %float None %23
%123 = OpFunctionParameter %_ptr_Function_v2float
%124 = OpFunctionParameter %_ptr_Function_v2float
%125 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
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
%171 = OpCompositeExtract %float %170 1
%172 = OpLoad %v2float %124
%173 = OpCompositeExtract %float %172 0
%174 = OpLoad %v2float %123
%175 = OpCompositeExtract %float %174 1
%176 = OpFMul %float %173 %175
%177 = OpLoad %float %delta
%178 = OpFDiv %float %176 %177
%169 = OpExtInst %float %1 FMin %171 %178
OpStore %delta %169
%179 = OpLoad %float %delta
%180 = OpLoad %v2float %123
%181 = OpCompositeExtract %float %180 1
%182 = OpFMul %float %179 %181
%183 = OpLoad %v2float %123
%184 = OpCompositeExtract %float %183 0
%185 = OpLoad %v2float %124
%186 = OpCompositeExtract %float %185 1
%187 = OpFSub %float %float_1 %186
%188 = OpFMul %float %184 %187
%189 = OpFAdd %float %182 %188
%190 = OpLoad %v2float %124
%191 = OpCompositeExtract %float %190 0
%192 = OpLoad %v2float %123
%193 = OpCompositeExtract %float %192 1
%194 = OpFSub %float %float_1 %193
%195 = OpFMul %float %191 %194
%196 = OpFAdd %float %189 %195
OpReturnValue %196
%149 = OpLabel
OpBranch %132
%132 = OpLabel
OpUnreachable
OpFunctionEnd
%_color_burn_component_hh2h2 = OpFunction %float None %23
%197 = OpFunctionParameter %_ptr_Function_v2float
%198 = OpFunctionParameter %_ptr_Function_v2float
%199 = OpLabel
%delta_0 = OpVariable %_ptr_Function_float Function
%200 = OpLoad %v2float %198
%201 = OpCompositeExtract %float %200 1
%202 = OpLoad %v2float %198
%203 = OpCompositeExtract %float %202 0
%204 = OpFOrdEqual %bool %201 %203
OpSelectionMerge %207 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%208 = OpLoad %v2float %197
%209 = OpCompositeExtract %float %208 1
%210 = OpLoad %v2float %198
%211 = OpCompositeExtract %float %210 1
%212 = OpFMul %float %209 %211
%213 = OpLoad %v2float %197
%214 = OpCompositeExtract %float %213 0
%215 = OpLoad %v2float %198
%216 = OpCompositeExtract %float %215 1
%217 = OpFSub %float %float_1 %216
%218 = OpFMul %float %214 %217
%219 = OpFAdd %float %212 %218
%220 = OpLoad %v2float %198
%221 = OpCompositeExtract %float %220 0
%222 = OpLoad %v2float %197
%223 = OpCompositeExtract %float %222 1
%224 = OpFSub %float %float_1 %223
%225 = OpFMul %float %221 %224
%226 = OpFAdd %float %219 %225
OpReturnValue %226
%206 = OpLabel
%227 = OpLoad %v2float %197
%228 = OpCompositeExtract %float %227 0
%229 = OpFOrdEqual %bool %228 %float_0
OpSelectionMerge %232 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%233 = OpLoad %v2float %198
%234 = OpCompositeExtract %float %233 0
%235 = OpLoad %v2float %197
%236 = OpCompositeExtract %float %235 1
%237 = OpFSub %float %float_1 %236
%238 = OpFMul %float %234 %237
OpReturnValue %238
%231 = OpLabel
%241 = OpLoad %v2float %198
%242 = OpCompositeExtract %float %241 1
%243 = OpLoad %v2float %198
%244 = OpCompositeExtract %float %243 1
%245 = OpLoad %v2float %198
%246 = OpCompositeExtract %float %245 0
%247 = OpFSub %float %244 %246
%248 = OpLoad %v2float %197
%249 = OpCompositeExtract %float %248 1
%250 = OpFMul %float %247 %249
%251 = OpLoad %v2float %197
%252 = OpCompositeExtract %float %251 0
%253 = OpFDiv %float %250 %252
%254 = OpFSub %float %242 %253
%240 = OpExtInst %float %1 FMax %float_0 %254
OpStore %delta_0 %240
%255 = OpLoad %float %delta_0
%256 = OpLoad %v2float %197
%257 = OpCompositeExtract %float %256 1
%258 = OpFMul %float %255 %257
%259 = OpLoad %v2float %197
%260 = OpCompositeExtract %float %259 0
%261 = OpLoad %v2float %198
%262 = OpCompositeExtract %float %261 1
%263 = OpFSub %float %float_1 %262
%264 = OpFMul %float %260 %263
%265 = OpFAdd %float %258 %264
%266 = OpLoad %v2float %198
%267 = OpCompositeExtract %float %266 0
%268 = OpLoad %v2float %197
%269 = OpCompositeExtract %float %268 1
%270 = OpFSub %float %float_1 %269
%271 = OpFMul %float %267 %270
%272 = OpFAdd %float %265 %271
OpReturnValue %272
%232 = OpLabel
OpBranch %207
%207 = OpLabel
OpUnreachable
OpFunctionEnd
%_soft_light_component_hh2h2 = OpFunction %float None %23
%273 = OpFunctionParameter %_ptr_Function_v2float
%274 = OpFunctionParameter %_ptr_Function_v2float
%275 = OpLabel
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%276 = OpLoad %v2float %273
%277 = OpCompositeExtract %float %276 0
%278 = OpFMul %float %float_2 %277
%279 = OpLoad %v2float %273
%280 = OpCompositeExtract %float %279 1
%281 = OpFOrdLessThanEqual %bool %278 %280
OpSelectionMerge %284 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%285 = OpLoad %v2float %274
%286 = OpCompositeExtract %float %285 0
%287 = OpLoad %v2float %274
%288 = OpCompositeExtract %float %287 0
%289 = OpFMul %float %286 %288
%290 = OpLoad %v2float %273
%291 = OpCompositeExtract %float %290 1
%292 = OpLoad %v2float %273
%293 = OpCompositeExtract %float %292 0
%294 = OpFMul %float %float_2 %293
%295 = OpFSub %float %291 %294
%296 = OpFMul %float %289 %295
%297 = OpLoad %v2float %274
%298 = OpCompositeExtract %float %297 1
%299 = OpFDiv %float %296 %298
%300 = OpLoad %v2float %274
%301 = OpCompositeExtract %float %300 1
%302 = OpFSub %float %float_1 %301
%303 = OpLoad %v2float %273
%304 = OpCompositeExtract %float %303 0
%305 = OpFMul %float %302 %304
%306 = OpFAdd %float %299 %305
%307 = OpLoad %v2float %274
%308 = OpCompositeExtract %float %307 0
%310 = OpLoad %v2float %273
%311 = OpCompositeExtract %float %310 1
%309 = OpFNegate %float %311
%312 = OpLoad %v2float %273
%313 = OpCompositeExtract %float %312 0
%314 = OpFMul %float %float_2 %313
%315 = OpFAdd %float %309 %314
%316 = OpFAdd %float %315 %float_1
%317 = OpFMul %float %308 %316
%318 = OpFAdd %float %306 %317
OpReturnValue %318
%283 = OpLabel
%320 = OpLoad %v2float %274
%321 = OpCompositeExtract %float %320 0
%322 = OpFMul %float %float_4 %321
%323 = OpLoad %v2float %274
%324 = OpCompositeExtract %float %323 1
%325 = OpFOrdLessThanEqual %bool %322 %324
OpSelectionMerge %328 None
OpBranchConditional %325 %326 %327
%326 = OpLabel
%330 = OpLoad %v2float %274
%331 = OpCompositeExtract %float %330 0
%332 = OpLoad %v2float %274
%333 = OpCompositeExtract %float %332 0
%334 = OpFMul %float %331 %333
OpStore %DSqd %334
%336 = OpLoad %float %DSqd
%337 = OpLoad %v2float %274
%338 = OpCompositeExtract %float %337 0
%339 = OpFMul %float %336 %338
OpStore %DCub %339
%341 = OpLoad %v2float %274
%342 = OpCompositeExtract %float %341 1
%343 = OpLoad %v2float %274
%344 = OpCompositeExtract %float %343 1
%345 = OpFMul %float %342 %344
OpStore %DaSqd %345
%347 = OpLoad %float %DaSqd
%348 = OpLoad %v2float %274
%349 = OpCompositeExtract %float %348 1
%350 = OpFMul %float %347 %349
OpStore %DaCub %350
%351 = OpLoad %float %DaSqd
%352 = OpLoad %v2float %273
%353 = OpCompositeExtract %float %352 0
%354 = OpLoad %v2float %274
%355 = OpCompositeExtract %float %354 0
%357 = OpLoad %v2float %273
%358 = OpCompositeExtract %float %357 1
%359 = OpFMul %float %float_3 %358
%361 = OpLoad %v2float %273
%362 = OpCompositeExtract %float %361 0
%363 = OpFMul %float %float_6 %362
%364 = OpFSub %float %359 %363
%365 = OpFSub %float %364 %float_1
%366 = OpFMul %float %355 %365
%367 = OpFSub %float %353 %366
%368 = OpFMul %float %351 %367
%370 = OpLoad %v2float %274
%371 = OpCompositeExtract %float %370 1
%372 = OpFMul %float %float_12 %371
%373 = OpLoad %float %DSqd
%374 = OpFMul %float %372 %373
%375 = OpLoad %v2float %273
%376 = OpCompositeExtract %float %375 1
%377 = OpLoad %v2float %273
%378 = OpCompositeExtract %float %377 0
%379 = OpFMul %float %float_2 %378
%380 = OpFSub %float %376 %379
%381 = OpFMul %float %374 %380
%382 = OpFAdd %float %368 %381
%384 = OpLoad %float %DCub
%385 = OpFMul %float %float_16 %384
%386 = OpLoad %v2float %273
%387 = OpCompositeExtract %float %386 1
%388 = OpLoad %v2float %273
%389 = OpCompositeExtract %float %388 0
%390 = OpFMul %float %float_2 %389
%391 = OpFSub %float %387 %390
%392 = OpFMul %float %385 %391
%393 = OpFSub %float %382 %392
%394 = OpLoad %float %DaCub
%395 = OpLoad %v2float %273
%396 = OpCompositeExtract %float %395 0
%397 = OpFMul %float %394 %396
%398 = OpFSub %float %393 %397
%399 = OpLoad %float %DaSqd
%400 = OpFDiv %float %398 %399
OpReturnValue %400
%327 = OpLabel
%401 = OpLoad %v2float %274
%402 = OpCompositeExtract %float %401 0
%403 = OpLoad %v2float %273
%404 = OpCompositeExtract %float %403 1
%405 = OpLoad %v2float %273
%406 = OpCompositeExtract %float %405 0
%407 = OpFMul %float %float_2 %406
%408 = OpFSub %float %404 %407
%409 = OpFAdd %float %408 %float_1
%410 = OpFMul %float %402 %409
%411 = OpLoad %v2float %273
%412 = OpCompositeExtract %float %411 0
%413 = OpFAdd %float %410 %412
%415 = OpLoad %v2float %274
%416 = OpCompositeExtract %float %415 1
%417 = OpLoad %v2float %274
%418 = OpCompositeExtract %float %417 0
%419 = OpFMul %float %416 %418
%414 = OpExtInst %float %1 Sqrt %419
%420 = OpLoad %v2float %273
%421 = OpCompositeExtract %float %420 1
%422 = OpLoad %v2float %273
%423 = OpCompositeExtract %float %422 0
%424 = OpFMul %float %float_2 %423
%425 = OpFSub %float %421 %424
%426 = OpFMul %float %414 %425
%427 = OpFSub %float %413 %426
%428 = OpLoad %v2float %274
%429 = OpCompositeExtract %float %428 1
%430 = OpLoad %v2float %273
%431 = OpCompositeExtract %float %430 0
%432 = OpFMul %float %429 %431
%433 = OpFSub %float %427 %432
OpReturnValue %433
%328 = OpLabel
OpBranch %284
%284 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_luminance_h3h3hh3 = OpFunction %v3float None %434
%436 = OpFunctionParameter %_ptr_Function_v3float
%437 = OpFunctionParameter %_ptr_Function_float
%438 = OpFunctionParameter %_ptr_Function_v3float
%439 = OpLabel
%lum = OpVariable %_ptr_Function_float Function
%result_0 = OpVariable %_ptr_Function_v3float Function
%minComp = OpVariable %_ptr_Function_float Function
%maxComp = OpVariable %_ptr_Function_float Function
%446 = OpLoad %v3float %438
%441 = OpDot %float %445 %446
OpStore %lum %441
%448 = OpLoad %float %lum
%450 = OpLoad %v3float %436
%449 = OpDot %float %445 %450
%451 = OpFSub %float %448 %449
%452 = OpLoad %v3float %436
%453 = OpCompositeConstruct %v3float %451 %451 %451
%454 = OpFAdd %v3float %453 %452
OpStore %result_0 %454
%458 = OpLoad %v3float %result_0
%459 = OpCompositeExtract %float %458 0
%460 = OpLoad %v3float %result_0
%461 = OpCompositeExtract %float %460 1
%457 = OpExtInst %float %1 FMin %459 %461
%462 = OpLoad %v3float %result_0
%463 = OpCompositeExtract %float %462 2
%456 = OpExtInst %float %1 FMin %457 %463
OpStore %minComp %456
%467 = OpLoad %v3float %result_0
%468 = OpCompositeExtract %float %467 0
%469 = OpLoad %v3float %result_0
%470 = OpCompositeExtract %float %469 1
%466 = OpExtInst %float %1 FMax %468 %470
%471 = OpLoad %v3float %result_0
%472 = OpCompositeExtract %float %471 2
%465 = OpExtInst %float %1 FMax %466 %472
OpStore %maxComp %465
%474 = OpLoad %float %minComp
%475 = OpFOrdLessThan %bool %474 %float_0
OpSelectionMerge %477 None
OpBranchConditional %475 %476 %477
%476 = OpLabel
%478 = OpLoad %float %lum
%479 = OpLoad %float %minComp
%480 = OpFOrdNotEqual %bool %478 %479
OpBranch %477
%477 = OpLabel
%481 = OpPhi %bool %false %439 %480 %476
OpSelectionMerge %483 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%484 = OpLoad %float %lum
%485 = OpLoad %v3float %result_0
%486 = OpLoad %float %lum
%487 = OpCompositeConstruct %v3float %486 %486 %486
%488 = OpFSub %v3float %485 %487
%489 = OpLoad %float %lum
%490 = OpLoad %float %lum
%491 = OpLoad %float %minComp
%492 = OpFSub %float %490 %491
%493 = OpFDiv %float %489 %492
%494 = OpVectorTimesScalar %v3float %488 %493
%495 = OpCompositeConstruct %v3float %484 %484 %484
%496 = OpFAdd %v3float %495 %494
OpStore %result_0 %496
OpBranch %483
%483 = OpLabel
%497 = OpLoad %float %maxComp
%498 = OpLoad %float %437
%499 = OpFOrdGreaterThan %bool %497 %498
OpSelectionMerge %501 None
OpBranchConditional %499 %500 %501
%500 = OpLabel
%502 = OpLoad %float %maxComp
%503 = OpLoad %float %lum
%504 = OpFOrdNotEqual %bool %502 %503
OpBranch %501
%501 = OpLabel
%505 = OpPhi %bool %false %483 %504 %500
OpSelectionMerge %508 None
OpBranchConditional %505 %506 %507
%506 = OpLabel
%509 = OpLoad %float %lum
%510 = OpLoad %v3float %result_0
%511 = OpLoad %float %lum
%512 = OpCompositeConstruct %v3float %511 %511 %511
%513 = OpFSub %v3float %510 %512
%514 = OpLoad %float %437
%515 = OpLoad %float %lum
%516 = OpFSub %float %514 %515
%517 = OpVectorTimesScalar %v3float %513 %516
%518 = OpLoad %float %maxComp
%519 = OpLoad %float %lum
%520 = OpFSub %float %518 %519
%521 = OpFDiv %float %float_1 %520
%522 = OpVectorTimesScalar %v3float %517 %521
%523 = OpCompositeConstruct %v3float %509 %509 %509
%524 = OpFAdd %v3float %523 %522
OpReturnValue %524
%507 = OpLabel
%525 = OpLoad %v3float %result_0
OpReturnValue %525
%508 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper_h3h3h = OpFunction %v3float None %526
%527 = OpFunctionParameter %_ptr_Function_v3float
%528 = OpFunctionParameter %_ptr_Function_float
%529 = OpLabel
%530 = OpLoad %v3float %527
%531 = OpCompositeExtract %float %530 0
%532 = OpLoad %v3float %527
%533 = OpCompositeExtract %float %532 2
%534 = OpFOrdLessThan %bool %531 %533
OpSelectionMerge %537 None
OpBranchConditional %534 %535 %536
%535 = OpLabel
%538 = OpLoad %float %528
%539 = OpLoad %v3float %527
%540 = OpCompositeExtract %float %539 1
%541 = OpLoad %v3float %527
%542 = OpCompositeExtract %float %541 0
%543 = OpFSub %float %540 %542
%544 = OpFMul %float %538 %543
%545 = OpLoad %v3float %527
%546 = OpCompositeExtract %float %545 2
%547 = OpLoad %v3float %527
%548 = OpCompositeExtract %float %547 0
%549 = OpFSub %float %546 %548
%550 = OpFDiv %float %544 %549
%551 = OpLoad %float %528
%552 = OpCompositeConstruct %v3float %float_0 %550 %551
OpReturnValue %552
%536 = OpLabel
OpReturnValue %553
%537 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_h3h3h3 = OpFunction %v3float None %554
%555 = OpFunctionParameter %_ptr_Function_v3float
%556 = OpFunctionParameter %_ptr_Function_v3float
%557 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%593 = OpVariable %_ptr_Function_v3float Function
%595 = OpVariable %_ptr_Function_float Function
%607 = OpVariable %_ptr_Function_v3float Function
%609 = OpVariable %_ptr_Function_float Function
%614 = OpVariable %_ptr_Function_v3float Function
%616 = OpVariable %_ptr_Function_float Function
%629 = OpVariable %_ptr_Function_v3float Function
%631 = OpVariable %_ptr_Function_float Function
%644 = OpVariable %_ptr_Function_v3float Function
%646 = OpVariable %_ptr_Function_float Function
%651 = OpVariable %_ptr_Function_v3float Function
%653 = OpVariable %_ptr_Function_float Function
%561 = OpLoad %v3float %556
%562 = OpCompositeExtract %float %561 0
%563 = OpLoad %v3float %556
%564 = OpCompositeExtract %float %563 1
%560 = OpExtInst %float %1 FMax %562 %564
%565 = OpLoad %v3float %556
%566 = OpCompositeExtract %float %565 2
%559 = OpExtInst %float %1 FMax %560 %566
%569 = OpLoad %v3float %556
%570 = OpCompositeExtract %float %569 0
%571 = OpLoad %v3float %556
%572 = OpCompositeExtract %float %571 1
%568 = OpExtInst %float %1 FMin %570 %572
%573 = OpLoad %v3float %556
%574 = OpCompositeExtract %float %573 2
%567 = OpExtInst %float %1 FMin %568 %574
%575 = OpFSub %float %559 %567
OpStore %sat %575
%576 = OpLoad %v3float %555
%577 = OpCompositeExtract %float %576 0
%578 = OpLoad %v3float %555
%579 = OpCompositeExtract %float %578 1
%580 = OpFOrdLessThanEqual %bool %577 %579
OpSelectionMerge %583 None
OpBranchConditional %580 %581 %582
%581 = OpLabel
%584 = OpLoad %v3float %555
%585 = OpCompositeExtract %float %584 1
%586 = OpLoad %v3float %555
%587 = OpCompositeExtract %float %586 2
%588 = OpFOrdLessThanEqual %bool %585 %587
OpSelectionMerge %591 None
OpBranchConditional %588 %589 %590
%589 = OpLabel
%592 = OpLoad %v3float %555
OpStore %593 %592
%594 = OpLoad %float %sat
OpStore %595 %594
%596 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %593 %595
OpReturnValue %596
%590 = OpLabel
%597 = OpLoad %v3float %555
%598 = OpCompositeExtract %float %597 0
%599 = OpLoad %v3float %555
%600 = OpCompositeExtract %float %599 2
%601 = OpFOrdLessThanEqual %bool %598 %600
OpSelectionMerge %604 None
OpBranchConditional %601 %602 %603
%602 = OpLabel
%605 = OpLoad %v3float %555
%606 = OpVectorShuffle %v3float %605 %605 0 2 1
OpStore %607 %606
%608 = OpLoad %float %sat
OpStore %609 %608
%610 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %607 %609
%611 = OpVectorShuffle %v3float %610 %610 0 2 1
OpReturnValue %611
%603 = OpLabel
%612 = OpLoad %v3float %555
%613 = OpVectorShuffle %v3float %612 %612 2 0 1
OpStore %614 %613
%615 = OpLoad %float %sat
OpStore %616 %615
%617 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %614 %616
%618 = OpVectorShuffle %v3float %617 %617 1 2 0
OpReturnValue %618
%604 = OpLabel
OpBranch %591
%591 = OpLabel
OpBranch %583
%582 = OpLabel
%619 = OpLoad %v3float %555
%620 = OpCompositeExtract %float %619 0
%621 = OpLoad %v3float %555
%622 = OpCompositeExtract %float %621 2
%623 = OpFOrdLessThanEqual %bool %620 %622
OpSelectionMerge %626 None
OpBranchConditional %623 %624 %625
%624 = OpLabel
%627 = OpLoad %v3float %555
%628 = OpVectorShuffle %v3float %627 %627 1 0 2
OpStore %629 %628
%630 = OpLoad %float %sat
OpStore %631 %630
%632 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %629 %631
%633 = OpVectorShuffle %v3float %632 %632 1 0 2
OpReturnValue %633
%625 = OpLabel
%634 = OpLoad %v3float %555
%635 = OpCompositeExtract %float %634 1
%636 = OpLoad %v3float %555
%637 = OpCompositeExtract %float %636 2
%638 = OpFOrdLessThanEqual %bool %635 %637
OpSelectionMerge %641 None
OpBranchConditional %638 %639 %640
%639 = OpLabel
%642 = OpLoad %v3float %555
%643 = OpVectorShuffle %v3float %642 %642 1 2 0
OpStore %644 %643
%645 = OpLoad %float %sat
OpStore %646 %645
%647 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %644 %646
%648 = OpVectorShuffle %v3float %647 %647 2 0 1
OpReturnValue %648
%640 = OpLabel
%649 = OpLoad %v3float %555
%650 = OpVectorShuffle %v3float %649 %649 2 1 0
OpStore %651 %650
%652 = OpLoad %float %sat
OpStore %653 %652
%654 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %651 %653
%655 = OpVectorShuffle %v3float %654 %654 2 1 0
OpReturnValue %655
%641 = OpLabel
OpBranch %626
%626 = OpLabel
OpBranch %583
%583 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_h4eh4h4 = OpFunction %v4float None %657
%659 = OpFunctionParameter %_ptr_Function_int
%660 = OpFunctionParameter %_ptr_Function_v4float
%661 = OpFunctionParameter %_ptr_Function_v4float
%662 = OpLabel
%777 = OpVariable %_ptr_Function_v4float Function
%779 = OpVariable %_ptr_Function_v4float Function
%_0_result = OpVariable %_ptr_Function_v4float Function
%_1_result = OpVariable %_ptr_Function_v4float Function
%829 = OpVariable %_ptr_Function_v2float Function
%832 = OpVariable %_ptr_Function_v2float Function
%836 = OpVariable %_ptr_Function_v2float Function
%839 = OpVariable %_ptr_Function_v2float Function
%843 = OpVariable %_ptr_Function_v2float Function
%846 = OpVariable %_ptr_Function_v2float Function
%860 = OpVariable %_ptr_Function_v2float Function
%863 = OpVariable %_ptr_Function_v2float Function
%867 = OpVariable %_ptr_Function_v2float Function
%870 = OpVariable %_ptr_Function_v2float Function
%874 = OpVariable %_ptr_Function_v2float Function
%877 = OpVariable %_ptr_Function_v2float Function
%890 = OpVariable %_ptr_Function_v4float Function
%892 = OpVariable %_ptr_Function_v4float Function
%897 = OpVariable %_ptr_Function_v4float Function
%904 = OpVariable %_ptr_Function_v2float Function
%907 = OpVariable %_ptr_Function_v2float Function
%911 = OpVariable %_ptr_Function_v2float Function
%914 = OpVariable %_ptr_Function_v2float Function
%918 = OpVariable %_ptr_Function_v2float Function
%921 = OpVariable %_ptr_Function_v2float Function
%_2_alpha = OpVariable %_ptr_Function_float Function
%_3_sda = OpVariable %_ptr_Function_v3float Function
%_4_dsa = OpVariable %_ptr_Function_v3float Function
%1041 = OpVariable %_ptr_Function_v3float Function
%1043 = OpVariable %_ptr_Function_v3float Function
%1045 = OpVariable %_ptr_Function_v3float Function
%1047 = OpVariable %_ptr_Function_float Function
%1049 = OpVariable %_ptr_Function_v3float Function
%_5_alpha = OpVariable %_ptr_Function_float Function
%_6_sda = OpVariable %_ptr_Function_v3float Function
%_7_dsa = OpVariable %_ptr_Function_v3float Function
%1091 = OpVariable %_ptr_Function_v3float Function
%1093 = OpVariable %_ptr_Function_v3float Function
%1095 = OpVariable %_ptr_Function_v3float Function
%1097 = OpVariable %_ptr_Function_float Function
%1099 = OpVariable %_ptr_Function_v3float Function
%_8_alpha = OpVariable %_ptr_Function_float Function
%_9_sda = OpVariable %_ptr_Function_v3float Function
%_10_dsa = OpVariable %_ptr_Function_v3float Function
%1141 = OpVariable %_ptr_Function_v3float Function
%1143 = OpVariable %_ptr_Function_float Function
%1145 = OpVariable %_ptr_Function_v3float Function
%_11_alpha = OpVariable %_ptr_Function_float Function
%_12_sda = OpVariable %_ptr_Function_v3float Function
%_13_dsa = OpVariable %_ptr_Function_v3float Function
%1187 = OpVariable %_ptr_Function_v3float Function
%1189 = OpVariable %_ptr_Function_float Function
%1191 = OpVariable %_ptr_Function_v3float Function
%663 = OpLoad %int %659
OpSelectionMerge %664 None
OpSwitch %663 %694 0 %665 1 %666 2 %667 3 %668 4 %669 5 %670 6 %671 7 %672 8 %673 9 %674 10 %675 11 %676 12 %677 13 %678 14 %679 15 %680 16 %681 17 %682 18 %683 19 %684 20 %685 21 %686 22 %687 23 %688 24 %689 25 %690 26 %691 27 %692 28 %693
%665 = OpLabel
OpReturnValue %695
%666 = OpLabel
%696 = OpLoad %v4float %660
OpReturnValue %696
%667 = OpLabel
%697 = OpLoad %v4float %661
OpReturnValue %697
%668 = OpLabel
%698 = OpLoad %v4float %660
%699 = OpLoad %v4float %660
%700 = OpCompositeExtract %float %699 3
%701 = OpFSub %float %float_1 %700
%702 = OpLoad %v4float %661
%703 = OpVectorTimesScalar %v4float %702 %701
%704 = OpFAdd %v4float %698 %703
OpReturnValue %704
%669 = OpLabel
%705 = OpLoad %v4float %661
%706 = OpCompositeExtract %float %705 3
%707 = OpFSub %float %float_1 %706
%708 = OpLoad %v4float %660
%709 = OpVectorTimesScalar %v4float %708 %707
%710 = OpLoad %v4float %661
%711 = OpFAdd %v4float %709 %710
OpReturnValue %711
%670 = OpLabel
%712 = OpLoad %v4float %660
%713 = OpLoad %v4float %661
%714 = OpCompositeExtract %float %713 3
%715 = OpVectorTimesScalar %v4float %712 %714
OpReturnValue %715
%671 = OpLabel
%716 = OpLoad %v4float %661
%717 = OpLoad %v4float %660
%718 = OpCompositeExtract %float %717 3
%719 = OpVectorTimesScalar %v4float %716 %718
OpReturnValue %719
%672 = OpLabel
%720 = OpLoad %v4float %661
%721 = OpCompositeExtract %float %720 3
%722 = OpFSub %float %float_1 %721
%723 = OpLoad %v4float %660
%724 = OpVectorTimesScalar %v4float %723 %722
OpReturnValue %724
%673 = OpLabel
%725 = OpLoad %v4float %660
%726 = OpCompositeExtract %float %725 3
%727 = OpFSub %float %float_1 %726
%728 = OpLoad %v4float %661
%729 = OpVectorTimesScalar %v4float %728 %727
OpReturnValue %729
%674 = OpLabel
%730 = OpLoad %v4float %661
%731 = OpCompositeExtract %float %730 3
%732 = OpLoad %v4float %660
%733 = OpVectorTimesScalar %v4float %732 %731
%734 = OpLoad %v4float %660
%735 = OpCompositeExtract %float %734 3
%736 = OpFSub %float %float_1 %735
%737 = OpLoad %v4float %661
%738 = OpVectorTimesScalar %v4float %737 %736
%739 = OpFAdd %v4float %733 %738
OpReturnValue %739
%675 = OpLabel
%740 = OpLoad %v4float %661
%741 = OpCompositeExtract %float %740 3
%742 = OpFSub %float %float_1 %741
%743 = OpLoad %v4float %660
%744 = OpVectorTimesScalar %v4float %743 %742
%745 = OpLoad %v4float %660
%746 = OpCompositeExtract %float %745 3
%747 = OpLoad %v4float %661
%748 = OpVectorTimesScalar %v4float %747 %746
%749 = OpFAdd %v4float %744 %748
OpReturnValue %749
%676 = OpLabel
%750 = OpLoad %v4float %661
%751 = OpCompositeExtract %float %750 3
%752 = OpFSub %float %float_1 %751
%753 = OpLoad %v4float %660
%754 = OpVectorTimesScalar %v4float %753 %752
%755 = OpLoad %v4float %660
%756 = OpCompositeExtract %float %755 3
%757 = OpFSub %float %float_1 %756
%758 = OpLoad %v4float %661
%759 = OpVectorTimesScalar %v4float %758 %757
%760 = OpFAdd %v4float %754 %759
OpReturnValue %760
%677 = OpLabel
%762 = OpLoad %v4float %660
%763 = OpLoad %v4float %661
%764 = OpFAdd %v4float %762 %763
%765 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%761 = OpExtInst %v4float %1 FMin %764 %765
OpReturnValue %761
%678 = OpLabel
%766 = OpLoad %v4float %660
%767 = OpLoad %v4float %661
%768 = OpFMul %v4float %766 %767
OpReturnValue %768
%679 = OpLabel
%769 = OpLoad %v4float %660
%770 = OpLoad %v4float %660
%771 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%772 = OpFSub %v4float %771 %770
%773 = OpLoad %v4float %661
%774 = OpFMul %v4float %772 %773
%775 = OpFAdd %v4float %769 %774
OpReturnValue %775
%680 = OpLabel
%776 = OpLoad %v4float %660
OpStore %777 %776
%778 = OpLoad %v4float %661
OpStore %779 %778
%780 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %777 %779
OpReturnValue %780
%681 = OpLabel
%782 = OpLoad %v4float %660
%783 = OpLoad %v4float %660
%784 = OpCompositeExtract %float %783 3
%785 = OpFSub %float %float_1 %784
%786 = OpLoad %v4float %661
%787 = OpVectorTimesScalar %v4float %786 %785
%788 = OpFAdd %v4float %782 %787
OpStore %_0_result %788
%790 = OpLoad %v4float %_0_result
%791 = OpVectorShuffle %v3float %790 %790 0 1 2
%792 = OpLoad %v4float %661
%793 = OpCompositeExtract %float %792 3
%794 = OpFSub %float %float_1 %793
%795 = OpLoad %v4float %660
%796 = OpVectorShuffle %v3float %795 %795 0 1 2
%797 = OpVectorTimesScalar %v3float %796 %794
%798 = OpLoad %v4float %661
%799 = OpVectorShuffle %v3float %798 %798 0 1 2
%800 = OpFAdd %v3float %797 %799
%789 = OpExtInst %v3float %1 FMin %791 %800
%801 = OpLoad %v4float %_0_result
%802 = OpVectorShuffle %v4float %801 %789 4 5 6 3
OpStore %_0_result %802
%803 = OpLoad %v4float %_0_result
OpReturnValue %803
%682 = OpLabel
%805 = OpLoad %v4float %660
%806 = OpLoad %v4float %660
%807 = OpCompositeExtract %float %806 3
%808 = OpFSub %float %float_1 %807
%809 = OpLoad %v4float %661
%810 = OpVectorTimesScalar %v4float %809 %808
%811 = OpFAdd %v4float %805 %810
OpStore %_1_result %811
%813 = OpLoad %v4float %_1_result
%814 = OpVectorShuffle %v3float %813 %813 0 1 2
%815 = OpLoad %v4float %661
%816 = OpCompositeExtract %float %815 3
%817 = OpFSub %float %float_1 %816
%818 = OpLoad %v4float %660
%819 = OpVectorShuffle %v3float %818 %818 0 1 2
%820 = OpVectorTimesScalar %v3float %819 %817
%821 = OpLoad %v4float %661
%822 = OpVectorShuffle %v3float %821 %821 0 1 2
%823 = OpFAdd %v3float %820 %822
%812 = OpExtInst %v3float %1 FMax %814 %823
%824 = OpLoad %v4float %_1_result
%825 = OpVectorShuffle %v4float %824 %812 4 5 6 3
OpStore %_1_result %825
%826 = OpLoad %v4float %_1_result
OpReturnValue %826
%683 = OpLabel
%827 = OpLoad %v4float %660
%828 = OpVectorShuffle %v2float %827 %827 0 3
OpStore %829 %828
%830 = OpLoad %v4float %661
%831 = OpVectorShuffle %v2float %830 %830 0 3
OpStore %832 %831
%833 = OpFunctionCall %float %_color_dodge_component_hh2h2 %829 %832
%834 = OpLoad %v4float %660
%835 = OpVectorShuffle %v2float %834 %834 1 3
OpStore %836 %835
%837 = OpLoad %v4float %661
%838 = OpVectorShuffle %v2float %837 %837 1 3
OpStore %839 %838
%840 = OpFunctionCall %float %_color_dodge_component_hh2h2 %836 %839
%841 = OpLoad %v4float %660
%842 = OpVectorShuffle %v2float %841 %841 2 3
OpStore %843 %842
%844 = OpLoad %v4float %661
%845 = OpVectorShuffle %v2float %844 %844 2 3
OpStore %846 %845
%847 = OpFunctionCall %float %_color_dodge_component_hh2h2 %843 %846
%848 = OpLoad %v4float %660
%849 = OpCompositeExtract %float %848 3
%850 = OpLoad %v4float %660
%851 = OpCompositeExtract %float %850 3
%852 = OpFSub %float %float_1 %851
%853 = OpLoad %v4float %661
%854 = OpCompositeExtract %float %853 3
%855 = OpFMul %float %852 %854
%856 = OpFAdd %float %849 %855
%857 = OpCompositeConstruct %v4float %833 %840 %847 %856
OpReturnValue %857
%684 = OpLabel
%858 = OpLoad %v4float %660
%859 = OpVectorShuffle %v2float %858 %858 0 3
OpStore %860 %859
%861 = OpLoad %v4float %661
%862 = OpVectorShuffle %v2float %861 %861 0 3
OpStore %863 %862
%864 = OpFunctionCall %float %_color_burn_component_hh2h2 %860 %863
%865 = OpLoad %v4float %660
%866 = OpVectorShuffle %v2float %865 %865 1 3
OpStore %867 %866
%868 = OpLoad %v4float %661
%869 = OpVectorShuffle %v2float %868 %868 1 3
OpStore %870 %869
%871 = OpFunctionCall %float %_color_burn_component_hh2h2 %867 %870
%872 = OpLoad %v4float %660
%873 = OpVectorShuffle %v2float %872 %872 2 3
OpStore %874 %873
%875 = OpLoad %v4float %661
%876 = OpVectorShuffle %v2float %875 %875 2 3
OpStore %877 %876
%878 = OpFunctionCall %float %_color_burn_component_hh2h2 %874 %877
%879 = OpLoad %v4float %660
%880 = OpCompositeExtract %float %879 3
%881 = OpLoad %v4float %660
%882 = OpCompositeExtract %float %881 3
%883 = OpFSub %float %float_1 %882
%884 = OpLoad %v4float %661
%885 = OpCompositeExtract %float %884 3
%886 = OpFMul %float %883 %885
%887 = OpFAdd %float %880 %886
%888 = OpCompositeConstruct %v4float %864 %871 %878 %887
OpReturnValue %888
%685 = OpLabel
%889 = OpLoad %v4float %661
OpStore %890 %889
%891 = OpLoad %v4float %660
OpStore %892 %891
%893 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %890 %892
OpReturnValue %893
%686 = OpLabel
%894 = OpLoad %v4float %661
%895 = OpCompositeExtract %float %894 3
%896 = OpFOrdEqual %bool %895 %float_0
OpSelectionMerge %900 None
OpBranchConditional %896 %898 %899
%898 = OpLabel
%901 = OpLoad %v4float %660
OpStore %897 %901
OpBranch %900
%899 = OpLabel
%902 = OpLoad %v4float %660
%903 = OpVectorShuffle %v2float %902 %902 0 3
OpStore %904 %903
%905 = OpLoad %v4float %661
%906 = OpVectorShuffle %v2float %905 %905 0 3
OpStore %907 %906
%908 = OpFunctionCall %float %_soft_light_component_hh2h2 %904 %907
%909 = OpLoad %v4float %660
%910 = OpVectorShuffle %v2float %909 %909 1 3
OpStore %911 %910
%912 = OpLoad %v4float %661
%913 = OpVectorShuffle %v2float %912 %912 1 3
OpStore %914 %913
%915 = OpFunctionCall %float %_soft_light_component_hh2h2 %911 %914
%916 = OpLoad %v4float %660
%917 = OpVectorShuffle %v2float %916 %916 2 3
OpStore %918 %917
%919 = OpLoad %v4float %661
%920 = OpVectorShuffle %v2float %919 %919 2 3
OpStore %921 %920
%922 = OpFunctionCall %float %_soft_light_component_hh2h2 %918 %921
%923 = OpLoad %v4float %660
%924 = OpCompositeExtract %float %923 3
%925 = OpLoad %v4float %660
%926 = OpCompositeExtract %float %925 3
%927 = OpFSub %float %float_1 %926
%928 = OpLoad %v4float %661
%929 = OpCompositeExtract %float %928 3
%930 = OpFMul %float %927 %929
%931 = OpFAdd %float %924 %930
%932 = OpCompositeConstruct %v4float %908 %915 %922 %931
OpStore %897 %932
OpBranch %900
%900 = OpLabel
%933 = OpLoad %v4float %897
OpReturnValue %933
%687 = OpLabel
%934 = OpLoad %v4float %660
%935 = OpVectorShuffle %v3float %934 %934 0 1 2
%936 = OpLoad %v4float %661
%937 = OpVectorShuffle %v3float %936 %936 0 1 2
%938 = OpFAdd %v3float %935 %937
%940 = OpLoad %v4float %660
%941 = OpVectorShuffle %v3float %940 %940 0 1 2
%942 = OpLoad %v4float %661
%943 = OpCompositeExtract %float %942 3
%944 = OpVectorTimesScalar %v3float %941 %943
%945 = OpLoad %v4float %661
%946 = OpVectorShuffle %v3float %945 %945 0 1 2
%947 = OpLoad %v4float %660
%948 = OpCompositeExtract %float %947 3
%949 = OpVectorTimesScalar %v3float %946 %948
%939 = OpExtInst %v3float %1 FMin %944 %949
%950 = OpVectorTimesScalar %v3float %939 %float_2
%951 = OpFSub %v3float %938 %950
%952 = OpCompositeExtract %float %951 0
%953 = OpCompositeExtract %float %951 1
%954 = OpCompositeExtract %float %951 2
%955 = OpLoad %v4float %660
%956 = OpCompositeExtract %float %955 3
%957 = OpLoad %v4float %660
%958 = OpCompositeExtract %float %957 3
%959 = OpFSub %float %float_1 %958
%960 = OpLoad %v4float %661
%961 = OpCompositeExtract %float %960 3
%962 = OpFMul %float %959 %961
%963 = OpFAdd %float %956 %962
%964 = OpCompositeConstruct %v4float %952 %953 %954 %963
OpReturnValue %964
%688 = OpLabel
%965 = OpLoad %v4float %661
%966 = OpVectorShuffle %v3float %965 %965 0 1 2
%967 = OpLoad %v4float %660
%968 = OpVectorShuffle %v3float %967 %967 0 1 2
%969 = OpFAdd %v3float %966 %968
%970 = OpLoad %v4float %661
%971 = OpVectorShuffle %v3float %970 %970 0 1 2
%972 = OpVectorTimesScalar %v3float %971 %float_2
%973 = OpLoad %v4float %660
%974 = OpVectorShuffle %v3float %973 %973 0 1 2
%975 = OpFMul %v3float %972 %974
%976 = OpFSub %v3float %969 %975
%977 = OpCompositeExtract %float %976 0
%978 = OpCompositeExtract %float %976 1
%979 = OpCompositeExtract %float %976 2
%980 = OpLoad %v4float %660
%981 = OpCompositeExtract %float %980 3
%982 = OpLoad %v4float %660
%983 = OpCompositeExtract %float %982 3
%984 = OpFSub %float %float_1 %983
%985 = OpLoad %v4float %661
%986 = OpCompositeExtract %float %985 3
%987 = OpFMul %float %984 %986
%988 = OpFAdd %float %981 %987
%989 = OpCompositeConstruct %v4float %977 %978 %979 %988
OpReturnValue %989
%689 = OpLabel
%990 = OpLoad %v4float %660
%991 = OpCompositeExtract %float %990 3
%992 = OpFSub %float %float_1 %991
%993 = OpLoad %v4float %661
%994 = OpVectorShuffle %v3float %993 %993 0 1 2
%995 = OpVectorTimesScalar %v3float %994 %992
%996 = OpLoad %v4float %661
%997 = OpCompositeExtract %float %996 3
%998 = OpFSub %float %float_1 %997
%999 = OpLoad %v4float %660
%1000 = OpVectorShuffle %v3float %999 %999 0 1 2
%1001 = OpVectorTimesScalar %v3float %1000 %998
%1002 = OpFAdd %v3float %995 %1001
%1003 = OpLoad %v4float %660
%1004 = OpVectorShuffle %v3float %1003 %1003 0 1 2
%1005 = OpLoad %v4float %661
%1006 = OpVectorShuffle %v3float %1005 %1005 0 1 2
%1007 = OpFMul %v3float %1004 %1006
%1008 = OpFAdd %v3float %1002 %1007
%1009 = OpCompositeExtract %float %1008 0
%1010 = OpCompositeExtract %float %1008 1
%1011 = OpCompositeExtract %float %1008 2
%1012 = OpLoad %v4float %660
%1013 = OpCompositeExtract %float %1012 3
%1014 = OpLoad %v4float %660
%1015 = OpCompositeExtract %float %1014 3
%1016 = OpFSub %float %float_1 %1015
%1017 = OpLoad %v4float %661
%1018 = OpCompositeExtract %float %1017 3
%1019 = OpFMul %float %1016 %1018
%1020 = OpFAdd %float %1013 %1019
%1021 = OpCompositeConstruct %v4float %1009 %1010 %1011 %1020
OpReturnValue %1021
%690 = OpLabel
%1023 = OpLoad %v4float %661
%1024 = OpCompositeExtract %float %1023 3
%1025 = OpLoad %v4float %660
%1026 = OpCompositeExtract %float %1025 3
%1027 = OpFMul %float %1024 %1026
OpStore %_2_alpha %1027
%1029 = OpLoad %v4float %660
%1030 = OpVectorShuffle %v3float %1029 %1029 0 1 2
%1031 = OpLoad %v4float %661
%1032 = OpCompositeExtract %float %1031 3
%1033 = OpVectorTimesScalar %v3float %1030 %1032
OpStore %_3_sda %1033
%1035 = OpLoad %v4float %661
%1036 = OpVectorShuffle %v3float %1035 %1035 0 1 2
%1037 = OpLoad %v4float %660
%1038 = OpCompositeExtract %float %1037 3
%1039 = OpVectorTimesScalar %v3float %1036 %1038
OpStore %_4_dsa %1039
%1040 = OpLoad %v3float %_3_sda
OpStore %1041 %1040
%1042 = OpLoad %v3float %_4_dsa
OpStore %1043 %1042
%1044 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %1041 %1043
OpStore %1045 %1044
%1046 = OpLoad %float %_2_alpha
OpStore %1047 %1046
%1048 = OpLoad %v3float %_4_dsa
OpStore %1049 %1048
%1050 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1045 %1047 %1049
%1051 = OpLoad %v4float %661
%1052 = OpVectorShuffle %v3float %1051 %1051 0 1 2
%1053 = OpFAdd %v3float %1050 %1052
%1054 = OpLoad %v3float %_4_dsa
%1055 = OpFSub %v3float %1053 %1054
%1056 = OpLoad %v4float %660
%1057 = OpVectorShuffle %v3float %1056 %1056 0 1 2
%1058 = OpFAdd %v3float %1055 %1057
%1059 = OpLoad %v3float %_3_sda
%1060 = OpFSub %v3float %1058 %1059
%1061 = OpCompositeExtract %float %1060 0
%1062 = OpCompositeExtract %float %1060 1
%1063 = OpCompositeExtract %float %1060 2
%1064 = OpLoad %v4float %660
%1065 = OpCompositeExtract %float %1064 3
%1066 = OpLoad %v4float %661
%1067 = OpCompositeExtract %float %1066 3
%1068 = OpFAdd %float %1065 %1067
%1069 = OpLoad %float %_2_alpha
%1070 = OpFSub %float %1068 %1069
%1071 = OpCompositeConstruct %v4float %1061 %1062 %1063 %1070
OpReturnValue %1071
%691 = OpLabel
%1073 = OpLoad %v4float %661
%1074 = OpCompositeExtract %float %1073 3
%1075 = OpLoad %v4float %660
%1076 = OpCompositeExtract %float %1075 3
%1077 = OpFMul %float %1074 %1076
OpStore %_5_alpha %1077
%1079 = OpLoad %v4float %660
%1080 = OpVectorShuffle %v3float %1079 %1079 0 1 2
%1081 = OpLoad %v4float %661
%1082 = OpCompositeExtract %float %1081 3
%1083 = OpVectorTimesScalar %v3float %1080 %1082
OpStore %_6_sda %1083
%1085 = OpLoad %v4float %661
%1086 = OpVectorShuffle %v3float %1085 %1085 0 1 2
%1087 = OpLoad %v4float %660
%1088 = OpCompositeExtract %float %1087 3
%1089 = OpVectorTimesScalar %v3float %1086 %1088
OpStore %_7_dsa %1089
%1090 = OpLoad %v3float %_7_dsa
OpStore %1091 %1090
%1092 = OpLoad %v3float %_6_sda
OpStore %1093 %1092
%1094 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %1091 %1093
OpStore %1095 %1094
%1096 = OpLoad %float %_5_alpha
OpStore %1097 %1096
%1098 = OpLoad %v3float %_7_dsa
OpStore %1099 %1098
%1100 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1095 %1097 %1099
%1101 = OpLoad %v4float %661
%1102 = OpVectorShuffle %v3float %1101 %1101 0 1 2
%1103 = OpFAdd %v3float %1100 %1102
%1104 = OpLoad %v3float %_7_dsa
%1105 = OpFSub %v3float %1103 %1104
%1106 = OpLoad %v4float %660
%1107 = OpVectorShuffle %v3float %1106 %1106 0 1 2
%1108 = OpFAdd %v3float %1105 %1107
%1109 = OpLoad %v3float %_6_sda
%1110 = OpFSub %v3float %1108 %1109
%1111 = OpCompositeExtract %float %1110 0
%1112 = OpCompositeExtract %float %1110 1
%1113 = OpCompositeExtract %float %1110 2
%1114 = OpLoad %v4float %660
%1115 = OpCompositeExtract %float %1114 3
%1116 = OpLoad %v4float %661
%1117 = OpCompositeExtract %float %1116 3
%1118 = OpFAdd %float %1115 %1117
%1119 = OpLoad %float %_5_alpha
%1120 = OpFSub %float %1118 %1119
%1121 = OpCompositeConstruct %v4float %1111 %1112 %1113 %1120
OpReturnValue %1121
%692 = OpLabel
%1123 = OpLoad %v4float %661
%1124 = OpCompositeExtract %float %1123 3
%1125 = OpLoad %v4float %660
%1126 = OpCompositeExtract %float %1125 3
%1127 = OpFMul %float %1124 %1126
OpStore %_8_alpha %1127
%1129 = OpLoad %v4float %660
%1130 = OpVectorShuffle %v3float %1129 %1129 0 1 2
%1131 = OpLoad %v4float %661
%1132 = OpCompositeExtract %float %1131 3
%1133 = OpVectorTimesScalar %v3float %1130 %1132
OpStore %_9_sda %1133
%1135 = OpLoad %v4float %661
%1136 = OpVectorShuffle %v3float %1135 %1135 0 1 2
%1137 = OpLoad %v4float %660
%1138 = OpCompositeExtract %float %1137 3
%1139 = OpVectorTimesScalar %v3float %1136 %1138
OpStore %_10_dsa %1139
%1140 = OpLoad %v3float %_9_sda
OpStore %1141 %1140
%1142 = OpLoad %float %_8_alpha
OpStore %1143 %1142
%1144 = OpLoad %v3float %_10_dsa
OpStore %1145 %1144
%1146 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1141 %1143 %1145
%1147 = OpLoad %v4float %661
%1148 = OpVectorShuffle %v3float %1147 %1147 0 1 2
%1149 = OpFAdd %v3float %1146 %1148
%1150 = OpLoad %v3float %_10_dsa
%1151 = OpFSub %v3float %1149 %1150
%1152 = OpLoad %v4float %660
%1153 = OpVectorShuffle %v3float %1152 %1152 0 1 2
%1154 = OpFAdd %v3float %1151 %1153
%1155 = OpLoad %v3float %_9_sda
%1156 = OpFSub %v3float %1154 %1155
%1157 = OpCompositeExtract %float %1156 0
%1158 = OpCompositeExtract %float %1156 1
%1159 = OpCompositeExtract %float %1156 2
%1160 = OpLoad %v4float %660
%1161 = OpCompositeExtract %float %1160 3
%1162 = OpLoad %v4float %661
%1163 = OpCompositeExtract %float %1162 3
%1164 = OpFAdd %float %1161 %1163
%1165 = OpLoad %float %_8_alpha
%1166 = OpFSub %float %1164 %1165
%1167 = OpCompositeConstruct %v4float %1157 %1158 %1159 %1166
OpReturnValue %1167
%693 = OpLabel
%1169 = OpLoad %v4float %661
%1170 = OpCompositeExtract %float %1169 3
%1171 = OpLoad %v4float %660
%1172 = OpCompositeExtract %float %1171 3
%1173 = OpFMul %float %1170 %1172
OpStore %_11_alpha %1173
%1175 = OpLoad %v4float %660
%1176 = OpVectorShuffle %v3float %1175 %1175 0 1 2
%1177 = OpLoad %v4float %661
%1178 = OpCompositeExtract %float %1177 3
%1179 = OpVectorTimesScalar %v3float %1176 %1178
OpStore %_12_sda %1179
%1181 = OpLoad %v4float %661
%1182 = OpVectorShuffle %v3float %1181 %1181 0 1 2
%1183 = OpLoad %v4float %660
%1184 = OpCompositeExtract %float %1183 3
%1185 = OpVectorTimesScalar %v3float %1182 %1184
OpStore %_13_dsa %1185
%1186 = OpLoad %v3float %_13_dsa
OpStore %1187 %1186
%1188 = OpLoad %float %_11_alpha
OpStore %1189 %1188
%1190 = OpLoad %v3float %_12_sda
OpStore %1191 %1190
%1192 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1187 %1189 %1191
%1193 = OpLoad %v4float %661
%1194 = OpVectorShuffle %v3float %1193 %1193 0 1 2
%1195 = OpFAdd %v3float %1192 %1194
%1196 = OpLoad %v3float %_13_dsa
%1197 = OpFSub %v3float %1195 %1196
%1198 = OpLoad %v4float %660
%1199 = OpVectorShuffle %v3float %1198 %1198 0 1 2
%1200 = OpFAdd %v3float %1197 %1199
%1201 = OpLoad %v3float %_12_sda
%1202 = OpFSub %v3float %1200 %1201
%1203 = OpCompositeExtract %float %1202 0
%1204 = OpCompositeExtract %float %1202 1
%1205 = OpCompositeExtract %float %1202 2
%1206 = OpLoad %v4float %660
%1207 = OpCompositeExtract %float %1206 3
%1208 = OpLoad %v4float %661
%1209 = OpCompositeExtract %float %1208 3
%1210 = OpFAdd %float %1207 %1209
%1211 = OpLoad %float %_11_alpha
%1212 = OpFSub %float %1210 %1211
%1213 = OpCompositeConstruct %v4float %1203 %1204 %1205 %1212
OpReturnValue %1213
%694 = OpLabel
OpReturnValue %695
%664 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %1215
%1216 = OpLabel
%1218 = OpVariable %_ptr_Function_int Function
%1223 = OpVariable %_ptr_Function_v4float Function
%1227 = OpVariable %_ptr_Function_v4float Function
OpStore %1218 %int_13
%1219 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%1222 = OpLoad %v4float %1219
OpStore %1223 %1222
%1224 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%1226 = OpLoad %v4float %1224
OpStore %1227 %1226
%1228 = OpFunctionCall %v4float %blend_h4eh4h4 %1218 %1223 %1227
OpStore %sk_FragColor %1228
OpReturn
OpFunctionEnd
