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
OpDecorate %455 RelaxedPrecision
OpDecorate %minComp RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %maxComp RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
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
OpDecorate %499 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
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
OpDecorate %526 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %534 RelaxedPrecision
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
OpDecorate %554 RelaxedPrecision
OpDecorate %sat RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %565 RelaxedPrecision
OpDecorate %566 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
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
OpDecorate %580 RelaxedPrecision
OpDecorate %585 RelaxedPrecision
OpDecorate %586 RelaxedPrecision
OpDecorate %587 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %598 RelaxedPrecision
OpDecorate %599 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %607 RelaxedPrecision
OpDecorate %609 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
OpDecorate %613 RelaxedPrecision
OpDecorate %614 RelaxedPrecision
OpDecorate %616 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %621 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %629 RelaxedPrecision
OpDecorate %631 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
OpDecorate %635 RelaxedPrecision
OpDecorate %636 RelaxedPrecision
OpDecorate %637 RelaxedPrecision
OpDecorate %638 RelaxedPrecision
OpDecorate %643 RelaxedPrecision
OpDecorate %644 RelaxedPrecision
OpDecorate %646 RelaxedPrecision
OpDecorate %649 RelaxedPrecision
OpDecorate %650 RelaxedPrecision
OpDecorate %651 RelaxedPrecision
OpDecorate %653 RelaxedPrecision
OpDecorate %656 RelaxedPrecision
OpDecorate %664 RelaxedPrecision
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
OpDecorate %761 RelaxedPrecision
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
OpDecorate %777 RelaxedPrecision
OpDecorate %779 RelaxedPrecision
OpDecorate %_0_result RelaxedPrecision
OpDecorate %783 RelaxedPrecision
OpDecorate %784 RelaxedPrecision
OpDecorate %785 RelaxedPrecision
OpDecorate %786 RelaxedPrecision
OpDecorate %787 RelaxedPrecision
OpDecorate %788 RelaxedPrecision
OpDecorate %789 RelaxedPrecision
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
OpDecorate %804 RelaxedPrecision
OpDecorate %_1_result RelaxedPrecision
OpDecorate %806 RelaxedPrecision
OpDecorate %807 RelaxedPrecision
OpDecorate %808 RelaxedPrecision
OpDecorate %809 RelaxedPrecision
OpDecorate %810 RelaxedPrecision
OpDecorate %811 RelaxedPrecision
OpDecorate %812 RelaxedPrecision
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
OpDecorate %829 RelaxedPrecision
OpDecorate %831 RelaxedPrecision
OpDecorate %832 RelaxedPrecision
OpDecorate %835 RelaxedPrecision
OpDecorate %836 RelaxedPrecision
OpDecorate %838 RelaxedPrecision
OpDecorate %839 RelaxedPrecision
OpDecorate %842 RelaxedPrecision
OpDecorate %843 RelaxedPrecision
OpDecorate %845 RelaxedPrecision
OpDecorate %846 RelaxedPrecision
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
OpDecorate %860 RelaxedPrecision
OpDecorate %862 RelaxedPrecision
OpDecorate %863 RelaxedPrecision
OpDecorate %866 RelaxedPrecision
OpDecorate %867 RelaxedPrecision
OpDecorate %869 RelaxedPrecision
OpDecorate %870 RelaxedPrecision
OpDecorate %873 RelaxedPrecision
OpDecorate %874 RelaxedPrecision
OpDecorate %876 RelaxedPrecision
OpDecorate %877 RelaxedPrecision
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
OpDecorate %890 RelaxedPrecision
OpDecorate %892 RelaxedPrecision
OpDecorate %895 RelaxedPrecision
OpDecorate %896 RelaxedPrecision
OpDecorate %902 RelaxedPrecision
OpDecorate %903 RelaxedPrecision
OpDecorate %904 RelaxedPrecision
OpDecorate %906 RelaxedPrecision
OpDecorate %907 RelaxedPrecision
OpDecorate %910 RelaxedPrecision
OpDecorate %911 RelaxedPrecision
OpDecorate %913 RelaxedPrecision
OpDecorate %914 RelaxedPrecision
OpDecorate %917 RelaxedPrecision
OpDecorate %918 RelaxedPrecision
OpDecorate %920 RelaxedPrecision
OpDecorate %921 RelaxedPrecision
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
OpDecorate %939 RelaxedPrecision
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
OpDecorate %1022 RelaxedPrecision
OpDecorate %_2_alpha RelaxedPrecision
OpDecorate %1024 RelaxedPrecision
OpDecorate %1025 RelaxedPrecision
OpDecorate %1026 RelaxedPrecision
OpDecorate %1027 RelaxedPrecision
OpDecorate %1028 RelaxedPrecision
OpDecorate %_3_sda RelaxedPrecision
OpDecorate %1030 RelaxedPrecision
OpDecorate %1031 RelaxedPrecision
OpDecorate %1032 RelaxedPrecision
OpDecorate %1033 RelaxedPrecision
OpDecorate %1034 RelaxedPrecision
OpDecorate %_4_dsa RelaxedPrecision
OpDecorate %1036 RelaxedPrecision
OpDecorate %1037 RelaxedPrecision
OpDecorate %1038 RelaxedPrecision
OpDecorate %1039 RelaxedPrecision
OpDecorate %1040 RelaxedPrecision
OpDecorate %1041 RelaxedPrecision
OpDecorate %1043 RelaxedPrecision
OpDecorate %1047 RelaxedPrecision
OpDecorate %1049 RelaxedPrecision
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
OpDecorate %1072 RelaxedPrecision
OpDecorate %_5_alpha RelaxedPrecision
OpDecorate %1074 RelaxedPrecision
OpDecorate %1075 RelaxedPrecision
OpDecorate %1076 RelaxedPrecision
OpDecorate %1077 RelaxedPrecision
OpDecorate %1078 RelaxedPrecision
OpDecorate %_6_sda RelaxedPrecision
OpDecorate %1080 RelaxedPrecision
OpDecorate %1081 RelaxedPrecision
OpDecorate %1082 RelaxedPrecision
OpDecorate %1083 RelaxedPrecision
OpDecorate %1084 RelaxedPrecision
OpDecorate %_7_dsa RelaxedPrecision
OpDecorate %1086 RelaxedPrecision
OpDecorate %1087 RelaxedPrecision
OpDecorate %1088 RelaxedPrecision
OpDecorate %1089 RelaxedPrecision
OpDecorate %1090 RelaxedPrecision
OpDecorate %1091 RelaxedPrecision
OpDecorate %1093 RelaxedPrecision
OpDecorate %1097 RelaxedPrecision
OpDecorate %1099 RelaxedPrecision
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
OpDecorate %1122 RelaxedPrecision
OpDecorate %_8_alpha RelaxedPrecision
OpDecorate %1124 RelaxedPrecision
OpDecorate %1125 RelaxedPrecision
OpDecorate %1126 RelaxedPrecision
OpDecorate %1127 RelaxedPrecision
OpDecorate %1128 RelaxedPrecision
OpDecorate %_9_sda RelaxedPrecision
OpDecorate %1130 RelaxedPrecision
OpDecorate %1131 RelaxedPrecision
OpDecorate %1132 RelaxedPrecision
OpDecorate %1133 RelaxedPrecision
OpDecorate %1134 RelaxedPrecision
OpDecorate %_10_dsa RelaxedPrecision
OpDecorate %1136 RelaxedPrecision
OpDecorate %1137 RelaxedPrecision
OpDecorate %1138 RelaxedPrecision
OpDecorate %1139 RelaxedPrecision
OpDecorate %1140 RelaxedPrecision
OpDecorate %1141 RelaxedPrecision
OpDecorate %1143 RelaxedPrecision
OpDecorate %1145 RelaxedPrecision
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
OpDecorate %1168 RelaxedPrecision
OpDecorate %_11_alpha RelaxedPrecision
OpDecorate %1170 RelaxedPrecision
OpDecorate %1171 RelaxedPrecision
OpDecorate %1172 RelaxedPrecision
OpDecorate %1173 RelaxedPrecision
OpDecorate %1174 RelaxedPrecision
OpDecorate %_12_sda RelaxedPrecision
OpDecorate %1176 RelaxedPrecision
OpDecorate %1177 RelaxedPrecision
OpDecorate %1178 RelaxedPrecision
OpDecorate %1179 RelaxedPrecision
OpDecorate %1180 RelaxedPrecision
OpDecorate %_13_dsa RelaxedPrecision
OpDecorate %1182 RelaxedPrecision
OpDecorate %1183 RelaxedPrecision
OpDecorate %1184 RelaxedPrecision
OpDecorate %1185 RelaxedPrecision
OpDecorate %1186 RelaxedPrecision
OpDecorate %1187 RelaxedPrecision
OpDecorate %1189 RelaxedPrecision
OpDecorate %1191 RelaxedPrecision
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
OpDecorate %1214 RelaxedPrecision
OpDecorate %1223 RelaxedPrecision
OpDecorate %1227 RelaxedPrecision
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
%false = OpConstantFalse %bool
%527 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%554 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%555 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_v3float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%658 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_v4float %_ptr_Function_v4float
%696 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%1216 = OpTypeFunction %void
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
%445 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%446 = OpLoad %v3float %438
%441 = OpDot %float %445 %446
OpStore %lum %441
%448 = OpLoad %float %lum
%450 = OpCompositeConstruct %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%451 = OpLoad %v3float %436
%449 = OpDot %float %450 %451
%452 = OpFSub %float %448 %449
%453 = OpLoad %v3float %436
%454 = OpCompositeConstruct %v3float %452 %452 %452
%455 = OpFAdd %v3float %454 %453
OpStore %result_0 %455
%459 = OpLoad %v3float %result_0
%460 = OpCompositeExtract %float %459 0
%461 = OpLoad %v3float %result_0
%462 = OpCompositeExtract %float %461 1
%458 = OpExtInst %float %1 FMin %460 %462
%463 = OpLoad %v3float %result_0
%464 = OpCompositeExtract %float %463 2
%457 = OpExtInst %float %1 FMin %458 %464
OpStore %minComp %457
%468 = OpLoad %v3float %result_0
%469 = OpCompositeExtract %float %468 0
%470 = OpLoad %v3float %result_0
%471 = OpCompositeExtract %float %470 1
%467 = OpExtInst %float %1 FMax %469 %471
%472 = OpLoad %v3float %result_0
%473 = OpCompositeExtract %float %472 2
%466 = OpExtInst %float %1 FMax %467 %473
OpStore %maxComp %466
%475 = OpLoad %float %minComp
%476 = OpFOrdLessThan %bool %475 %float_0
OpSelectionMerge %478 None
OpBranchConditional %476 %477 %478
%477 = OpLabel
%479 = OpLoad %float %lum
%480 = OpLoad %float %minComp
%481 = OpFOrdNotEqual %bool %479 %480
OpBranch %478
%478 = OpLabel
%482 = OpPhi %bool %false %439 %481 %477
OpSelectionMerge %484 None
OpBranchConditional %482 %483 %484
%483 = OpLabel
%485 = OpLoad %float %lum
%486 = OpLoad %v3float %result_0
%487 = OpLoad %float %lum
%488 = OpCompositeConstruct %v3float %487 %487 %487
%489 = OpFSub %v3float %486 %488
%490 = OpLoad %float %lum
%491 = OpLoad %float %lum
%492 = OpLoad %float %minComp
%493 = OpFSub %float %491 %492
%494 = OpFDiv %float %490 %493
%495 = OpVectorTimesScalar %v3float %489 %494
%496 = OpCompositeConstruct %v3float %485 %485 %485
%497 = OpFAdd %v3float %496 %495
OpStore %result_0 %497
OpBranch %484
%484 = OpLabel
%498 = OpLoad %float %maxComp
%499 = OpLoad %float %437
%500 = OpFOrdGreaterThan %bool %498 %499
OpSelectionMerge %502 None
OpBranchConditional %500 %501 %502
%501 = OpLabel
%503 = OpLoad %float %maxComp
%504 = OpLoad %float %lum
%505 = OpFOrdNotEqual %bool %503 %504
OpBranch %502
%502 = OpLabel
%506 = OpPhi %bool %false %484 %505 %501
OpSelectionMerge %509 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%510 = OpLoad %float %lum
%511 = OpLoad %v3float %result_0
%512 = OpLoad %float %lum
%513 = OpCompositeConstruct %v3float %512 %512 %512
%514 = OpFSub %v3float %511 %513
%515 = OpLoad %float %437
%516 = OpLoad %float %lum
%517 = OpFSub %float %515 %516
%518 = OpVectorTimesScalar %v3float %514 %517
%519 = OpLoad %float %maxComp
%520 = OpLoad %float %lum
%521 = OpFSub %float %519 %520
%522 = OpFDiv %float %float_1 %521
%523 = OpVectorTimesScalar %v3float %518 %522
%524 = OpCompositeConstruct %v3float %510 %510 %510
%525 = OpFAdd %v3float %524 %523
OpReturnValue %525
%508 = OpLabel
%526 = OpLoad %v3float %result_0
OpReturnValue %526
%509 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_helper_h3h3h = OpFunction %v3float None %527
%528 = OpFunctionParameter %_ptr_Function_v3float
%529 = OpFunctionParameter %_ptr_Function_float
%530 = OpLabel
%531 = OpLoad %v3float %528
%532 = OpCompositeExtract %float %531 0
%533 = OpLoad %v3float %528
%534 = OpCompositeExtract %float %533 2
%535 = OpFOrdLessThan %bool %532 %534
OpSelectionMerge %538 None
OpBranchConditional %535 %536 %537
%536 = OpLabel
%539 = OpLoad %float %529
%540 = OpLoad %v3float %528
%541 = OpCompositeExtract %float %540 1
%542 = OpLoad %v3float %528
%543 = OpCompositeExtract %float %542 0
%544 = OpFSub %float %541 %543
%545 = OpFMul %float %539 %544
%546 = OpLoad %v3float %528
%547 = OpCompositeExtract %float %546 2
%548 = OpLoad %v3float %528
%549 = OpCompositeExtract %float %548 0
%550 = OpFSub %float %547 %549
%551 = OpFDiv %float %545 %550
%552 = OpLoad %float %529
%553 = OpCompositeConstruct %v3float %float_0 %551 %552
OpReturnValue %553
%537 = OpLabel
OpReturnValue %554
%538 = OpLabel
OpUnreachable
OpFunctionEnd
%_blend_set_color_saturation_h3h3h3 = OpFunction %v3float None %555
%556 = OpFunctionParameter %_ptr_Function_v3float
%557 = OpFunctionParameter %_ptr_Function_v3float
%558 = OpLabel
%sat = OpVariable %_ptr_Function_float Function
%594 = OpVariable %_ptr_Function_v3float Function
%596 = OpVariable %_ptr_Function_float Function
%608 = OpVariable %_ptr_Function_v3float Function
%610 = OpVariable %_ptr_Function_float Function
%615 = OpVariable %_ptr_Function_v3float Function
%617 = OpVariable %_ptr_Function_float Function
%630 = OpVariable %_ptr_Function_v3float Function
%632 = OpVariable %_ptr_Function_float Function
%645 = OpVariable %_ptr_Function_v3float Function
%647 = OpVariable %_ptr_Function_float Function
%652 = OpVariable %_ptr_Function_v3float Function
%654 = OpVariable %_ptr_Function_float Function
%562 = OpLoad %v3float %557
%563 = OpCompositeExtract %float %562 0
%564 = OpLoad %v3float %557
%565 = OpCompositeExtract %float %564 1
%561 = OpExtInst %float %1 FMax %563 %565
%566 = OpLoad %v3float %557
%567 = OpCompositeExtract %float %566 2
%560 = OpExtInst %float %1 FMax %561 %567
%570 = OpLoad %v3float %557
%571 = OpCompositeExtract %float %570 0
%572 = OpLoad %v3float %557
%573 = OpCompositeExtract %float %572 1
%569 = OpExtInst %float %1 FMin %571 %573
%574 = OpLoad %v3float %557
%575 = OpCompositeExtract %float %574 2
%568 = OpExtInst %float %1 FMin %569 %575
%576 = OpFSub %float %560 %568
OpStore %sat %576
%577 = OpLoad %v3float %556
%578 = OpCompositeExtract %float %577 0
%579 = OpLoad %v3float %556
%580 = OpCompositeExtract %float %579 1
%581 = OpFOrdLessThanEqual %bool %578 %580
OpSelectionMerge %584 None
OpBranchConditional %581 %582 %583
%582 = OpLabel
%585 = OpLoad %v3float %556
%586 = OpCompositeExtract %float %585 1
%587 = OpLoad %v3float %556
%588 = OpCompositeExtract %float %587 2
%589 = OpFOrdLessThanEqual %bool %586 %588
OpSelectionMerge %592 None
OpBranchConditional %589 %590 %591
%590 = OpLabel
%593 = OpLoad %v3float %556
OpStore %594 %593
%595 = OpLoad %float %sat
OpStore %596 %595
%597 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %594 %596
OpReturnValue %597
%591 = OpLabel
%598 = OpLoad %v3float %556
%599 = OpCompositeExtract %float %598 0
%600 = OpLoad %v3float %556
%601 = OpCompositeExtract %float %600 2
%602 = OpFOrdLessThanEqual %bool %599 %601
OpSelectionMerge %605 None
OpBranchConditional %602 %603 %604
%603 = OpLabel
%606 = OpLoad %v3float %556
%607 = OpVectorShuffle %v3float %606 %606 0 2 1
OpStore %608 %607
%609 = OpLoad %float %sat
OpStore %610 %609
%611 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %608 %610
%612 = OpVectorShuffle %v3float %611 %611 0 2 1
OpReturnValue %612
%604 = OpLabel
%613 = OpLoad %v3float %556
%614 = OpVectorShuffle %v3float %613 %613 2 0 1
OpStore %615 %614
%616 = OpLoad %float %sat
OpStore %617 %616
%618 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %615 %617
%619 = OpVectorShuffle %v3float %618 %618 1 2 0
OpReturnValue %619
%605 = OpLabel
OpBranch %592
%592 = OpLabel
OpBranch %584
%583 = OpLabel
%620 = OpLoad %v3float %556
%621 = OpCompositeExtract %float %620 0
%622 = OpLoad %v3float %556
%623 = OpCompositeExtract %float %622 2
%624 = OpFOrdLessThanEqual %bool %621 %623
OpSelectionMerge %627 None
OpBranchConditional %624 %625 %626
%625 = OpLabel
%628 = OpLoad %v3float %556
%629 = OpVectorShuffle %v3float %628 %628 1 0 2
OpStore %630 %629
%631 = OpLoad %float %sat
OpStore %632 %631
%633 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %630 %632
%634 = OpVectorShuffle %v3float %633 %633 1 0 2
OpReturnValue %634
%626 = OpLabel
%635 = OpLoad %v3float %556
%636 = OpCompositeExtract %float %635 1
%637 = OpLoad %v3float %556
%638 = OpCompositeExtract %float %637 2
%639 = OpFOrdLessThanEqual %bool %636 %638
OpSelectionMerge %642 None
OpBranchConditional %639 %640 %641
%640 = OpLabel
%643 = OpLoad %v3float %556
%644 = OpVectorShuffle %v3float %643 %643 1 2 0
OpStore %645 %644
%646 = OpLoad %float %sat
OpStore %647 %646
%648 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %645 %647
%649 = OpVectorShuffle %v3float %648 %648 2 0 1
OpReturnValue %649
%641 = OpLabel
%650 = OpLoad %v3float %556
%651 = OpVectorShuffle %v3float %650 %650 2 1 0
OpStore %652 %651
%653 = OpLoad %float %sat
OpStore %654 %653
%655 = OpFunctionCall %v3float %_blend_set_color_saturation_helper_h3h3h %652 %654
%656 = OpVectorShuffle %v3float %655 %655 2 1 0
OpReturnValue %656
%642 = OpLabel
OpBranch %627
%627 = OpLabel
OpBranch %584
%584 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_h4eh4h4 = OpFunction %v4float None %658
%660 = OpFunctionParameter %_ptr_Function_int
%661 = OpFunctionParameter %_ptr_Function_v4float
%662 = OpFunctionParameter %_ptr_Function_v4float
%663 = OpLabel
%778 = OpVariable %_ptr_Function_v4float Function
%780 = OpVariable %_ptr_Function_v4float Function
%_0_result = OpVariable %_ptr_Function_v4float Function
%_1_result = OpVariable %_ptr_Function_v4float Function
%830 = OpVariable %_ptr_Function_v2float Function
%833 = OpVariable %_ptr_Function_v2float Function
%837 = OpVariable %_ptr_Function_v2float Function
%840 = OpVariable %_ptr_Function_v2float Function
%844 = OpVariable %_ptr_Function_v2float Function
%847 = OpVariable %_ptr_Function_v2float Function
%861 = OpVariable %_ptr_Function_v2float Function
%864 = OpVariable %_ptr_Function_v2float Function
%868 = OpVariable %_ptr_Function_v2float Function
%871 = OpVariable %_ptr_Function_v2float Function
%875 = OpVariable %_ptr_Function_v2float Function
%878 = OpVariable %_ptr_Function_v2float Function
%891 = OpVariable %_ptr_Function_v4float Function
%893 = OpVariable %_ptr_Function_v4float Function
%898 = OpVariable %_ptr_Function_v4float Function
%905 = OpVariable %_ptr_Function_v2float Function
%908 = OpVariable %_ptr_Function_v2float Function
%912 = OpVariable %_ptr_Function_v2float Function
%915 = OpVariable %_ptr_Function_v2float Function
%919 = OpVariable %_ptr_Function_v2float Function
%922 = OpVariable %_ptr_Function_v2float Function
%_2_alpha = OpVariable %_ptr_Function_float Function
%_3_sda = OpVariable %_ptr_Function_v3float Function
%_4_dsa = OpVariable %_ptr_Function_v3float Function
%1042 = OpVariable %_ptr_Function_v3float Function
%1044 = OpVariable %_ptr_Function_v3float Function
%1046 = OpVariable %_ptr_Function_v3float Function
%1048 = OpVariable %_ptr_Function_float Function
%1050 = OpVariable %_ptr_Function_v3float Function
%_5_alpha = OpVariable %_ptr_Function_float Function
%_6_sda = OpVariable %_ptr_Function_v3float Function
%_7_dsa = OpVariable %_ptr_Function_v3float Function
%1092 = OpVariable %_ptr_Function_v3float Function
%1094 = OpVariable %_ptr_Function_v3float Function
%1096 = OpVariable %_ptr_Function_v3float Function
%1098 = OpVariable %_ptr_Function_float Function
%1100 = OpVariable %_ptr_Function_v3float Function
%_8_alpha = OpVariable %_ptr_Function_float Function
%_9_sda = OpVariable %_ptr_Function_v3float Function
%_10_dsa = OpVariable %_ptr_Function_v3float Function
%1142 = OpVariable %_ptr_Function_v3float Function
%1144 = OpVariable %_ptr_Function_float Function
%1146 = OpVariable %_ptr_Function_v3float Function
%_11_alpha = OpVariable %_ptr_Function_float Function
%_12_sda = OpVariable %_ptr_Function_v3float Function
%_13_dsa = OpVariable %_ptr_Function_v3float Function
%1188 = OpVariable %_ptr_Function_v3float Function
%1190 = OpVariable %_ptr_Function_float Function
%1192 = OpVariable %_ptr_Function_v3float Function
%664 = OpLoad %int %660
OpSelectionMerge %665 None
OpSwitch %664 %695 0 %666 1 %667 2 %668 3 %669 4 %670 5 %671 6 %672 7 %673 8 %674 9 %675 10 %676 11 %677 12 %678 13 %679 14 %680 15 %681 16 %682 17 %683 18 %684 19 %685 20 %686 21 %687 22 %688 23 %689 24 %690 25 %691 26 %692 27 %693 28 %694
%666 = OpLabel
OpReturnValue %696
%667 = OpLabel
%697 = OpLoad %v4float %661
OpReturnValue %697
%668 = OpLabel
%698 = OpLoad %v4float %662
OpReturnValue %698
%669 = OpLabel
%699 = OpLoad %v4float %661
%700 = OpLoad %v4float %661
%701 = OpCompositeExtract %float %700 3
%702 = OpFSub %float %float_1 %701
%703 = OpLoad %v4float %662
%704 = OpVectorTimesScalar %v4float %703 %702
%705 = OpFAdd %v4float %699 %704
OpReturnValue %705
%670 = OpLabel
%706 = OpLoad %v4float %662
%707 = OpCompositeExtract %float %706 3
%708 = OpFSub %float %float_1 %707
%709 = OpLoad %v4float %661
%710 = OpVectorTimesScalar %v4float %709 %708
%711 = OpLoad %v4float %662
%712 = OpFAdd %v4float %710 %711
OpReturnValue %712
%671 = OpLabel
%713 = OpLoad %v4float %661
%714 = OpLoad %v4float %662
%715 = OpCompositeExtract %float %714 3
%716 = OpVectorTimesScalar %v4float %713 %715
OpReturnValue %716
%672 = OpLabel
%717 = OpLoad %v4float %662
%718 = OpLoad %v4float %661
%719 = OpCompositeExtract %float %718 3
%720 = OpVectorTimesScalar %v4float %717 %719
OpReturnValue %720
%673 = OpLabel
%721 = OpLoad %v4float %662
%722 = OpCompositeExtract %float %721 3
%723 = OpFSub %float %float_1 %722
%724 = OpLoad %v4float %661
%725 = OpVectorTimesScalar %v4float %724 %723
OpReturnValue %725
%674 = OpLabel
%726 = OpLoad %v4float %661
%727 = OpCompositeExtract %float %726 3
%728 = OpFSub %float %float_1 %727
%729 = OpLoad %v4float %662
%730 = OpVectorTimesScalar %v4float %729 %728
OpReturnValue %730
%675 = OpLabel
%731 = OpLoad %v4float %662
%732 = OpCompositeExtract %float %731 3
%733 = OpLoad %v4float %661
%734 = OpVectorTimesScalar %v4float %733 %732
%735 = OpLoad %v4float %661
%736 = OpCompositeExtract %float %735 3
%737 = OpFSub %float %float_1 %736
%738 = OpLoad %v4float %662
%739 = OpVectorTimesScalar %v4float %738 %737
%740 = OpFAdd %v4float %734 %739
OpReturnValue %740
%676 = OpLabel
%741 = OpLoad %v4float %662
%742 = OpCompositeExtract %float %741 3
%743 = OpFSub %float %float_1 %742
%744 = OpLoad %v4float %661
%745 = OpVectorTimesScalar %v4float %744 %743
%746 = OpLoad %v4float %661
%747 = OpCompositeExtract %float %746 3
%748 = OpLoad %v4float %662
%749 = OpVectorTimesScalar %v4float %748 %747
%750 = OpFAdd %v4float %745 %749
OpReturnValue %750
%677 = OpLabel
%751 = OpLoad %v4float %662
%752 = OpCompositeExtract %float %751 3
%753 = OpFSub %float %float_1 %752
%754 = OpLoad %v4float %661
%755 = OpVectorTimesScalar %v4float %754 %753
%756 = OpLoad %v4float %661
%757 = OpCompositeExtract %float %756 3
%758 = OpFSub %float %float_1 %757
%759 = OpLoad %v4float %662
%760 = OpVectorTimesScalar %v4float %759 %758
%761 = OpFAdd %v4float %755 %760
OpReturnValue %761
%678 = OpLabel
%763 = OpLoad %v4float %661
%764 = OpLoad %v4float %662
%765 = OpFAdd %v4float %763 %764
%766 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%762 = OpExtInst %v4float %1 FMin %765 %766
OpReturnValue %762
%679 = OpLabel
%767 = OpLoad %v4float %661
%768 = OpLoad %v4float %662
%769 = OpFMul %v4float %767 %768
OpReturnValue %769
%680 = OpLabel
%770 = OpLoad %v4float %661
%771 = OpLoad %v4float %661
%772 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%773 = OpFSub %v4float %772 %771
%774 = OpLoad %v4float %662
%775 = OpFMul %v4float %773 %774
%776 = OpFAdd %v4float %770 %775
OpReturnValue %776
%681 = OpLabel
%777 = OpLoad %v4float %661
OpStore %778 %777
%779 = OpLoad %v4float %662
OpStore %780 %779
%781 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %778 %780
OpReturnValue %781
%682 = OpLabel
%783 = OpLoad %v4float %661
%784 = OpLoad %v4float %661
%785 = OpCompositeExtract %float %784 3
%786 = OpFSub %float %float_1 %785
%787 = OpLoad %v4float %662
%788 = OpVectorTimesScalar %v4float %787 %786
%789 = OpFAdd %v4float %783 %788
OpStore %_0_result %789
%791 = OpLoad %v4float %_0_result
%792 = OpVectorShuffle %v3float %791 %791 0 1 2
%793 = OpLoad %v4float %662
%794 = OpCompositeExtract %float %793 3
%795 = OpFSub %float %float_1 %794
%796 = OpLoad %v4float %661
%797 = OpVectorShuffle %v3float %796 %796 0 1 2
%798 = OpVectorTimesScalar %v3float %797 %795
%799 = OpLoad %v4float %662
%800 = OpVectorShuffle %v3float %799 %799 0 1 2
%801 = OpFAdd %v3float %798 %800
%790 = OpExtInst %v3float %1 FMin %792 %801
%802 = OpLoad %v4float %_0_result
%803 = OpVectorShuffle %v4float %802 %790 4 5 6 3
OpStore %_0_result %803
%804 = OpLoad %v4float %_0_result
OpReturnValue %804
%683 = OpLabel
%806 = OpLoad %v4float %661
%807 = OpLoad %v4float %661
%808 = OpCompositeExtract %float %807 3
%809 = OpFSub %float %float_1 %808
%810 = OpLoad %v4float %662
%811 = OpVectorTimesScalar %v4float %810 %809
%812 = OpFAdd %v4float %806 %811
OpStore %_1_result %812
%814 = OpLoad %v4float %_1_result
%815 = OpVectorShuffle %v3float %814 %814 0 1 2
%816 = OpLoad %v4float %662
%817 = OpCompositeExtract %float %816 3
%818 = OpFSub %float %float_1 %817
%819 = OpLoad %v4float %661
%820 = OpVectorShuffle %v3float %819 %819 0 1 2
%821 = OpVectorTimesScalar %v3float %820 %818
%822 = OpLoad %v4float %662
%823 = OpVectorShuffle %v3float %822 %822 0 1 2
%824 = OpFAdd %v3float %821 %823
%813 = OpExtInst %v3float %1 FMax %815 %824
%825 = OpLoad %v4float %_1_result
%826 = OpVectorShuffle %v4float %825 %813 4 5 6 3
OpStore %_1_result %826
%827 = OpLoad %v4float %_1_result
OpReturnValue %827
%684 = OpLabel
%828 = OpLoad %v4float %661
%829 = OpVectorShuffle %v2float %828 %828 0 3
OpStore %830 %829
%831 = OpLoad %v4float %662
%832 = OpVectorShuffle %v2float %831 %831 0 3
OpStore %833 %832
%834 = OpFunctionCall %float %_color_dodge_component_hh2h2 %830 %833
%835 = OpLoad %v4float %661
%836 = OpVectorShuffle %v2float %835 %835 1 3
OpStore %837 %836
%838 = OpLoad %v4float %662
%839 = OpVectorShuffle %v2float %838 %838 1 3
OpStore %840 %839
%841 = OpFunctionCall %float %_color_dodge_component_hh2h2 %837 %840
%842 = OpLoad %v4float %661
%843 = OpVectorShuffle %v2float %842 %842 2 3
OpStore %844 %843
%845 = OpLoad %v4float %662
%846 = OpVectorShuffle %v2float %845 %845 2 3
OpStore %847 %846
%848 = OpFunctionCall %float %_color_dodge_component_hh2h2 %844 %847
%849 = OpLoad %v4float %661
%850 = OpCompositeExtract %float %849 3
%851 = OpLoad %v4float %661
%852 = OpCompositeExtract %float %851 3
%853 = OpFSub %float %float_1 %852
%854 = OpLoad %v4float %662
%855 = OpCompositeExtract %float %854 3
%856 = OpFMul %float %853 %855
%857 = OpFAdd %float %850 %856
%858 = OpCompositeConstruct %v4float %834 %841 %848 %857
OpReturnValue %858
%685 = OpLabel
%859 = OpLoad %v4float %661
%860 = OpVectorShuffle %v2float %859 %859 0 3
OpStore %861 %860
%862 = OpLoad %v4float %662
%863 = OpVectorShuffle %v2float %862 %862 0 3
OpStore %864 %863
%865 = OpFunctionCall %float %_color_burn_component_hh2h2 %861 %864
%866 = OpLoad %v4float %661
%867 = OpVectorShuffle %v2float %866 %866 1 3
OpStore %868 %867
%869 = OpLoad %v4float %662
%870 = OpVectorShuffle %v2float %869 %869 1 3
OpStore %871 %870
%872 = OpFunctionCall %float %_color_burn_component_hh2h2 %868 %871
%873 = OpLoad %v4float %661
%874 = OpVectorShuffle %v2float %873 %873 2 3
OpStore %875 %874
%876 = OpLoad %v4float %662
%877 = OpVectorShuffle %v2float %876 %876 2 3
OpStore %878 %877
%879 = OpFunctionCall %float %_color_burn_component_hh2h2 %875 %878
%880 = OpLoad %v4float %661
%881 = OpCompositeExtract %float %880 3
%882 = OpLoad %v4float %661
%883 = OpCompositeExtract %float %882 3
%884 = OpFSub %float %float_1 %883
%885 = OpLoad %v4float %662
%886 = OpCompositeExtract %float %885 3
%887 = OpFMul %float %884 %886
%888 = OpFAdd %float %881 %887
%889 = OpCompositeConstruct %v4float %865 %872 %879 %888
OpReturnValue %889
%686 = OpLabel
%890 = OpLoad %v4float %662
OpStore %891 %890
%892 = OpLoad %v4float %661
OpStore %893 %892
%894 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %891 %893
OpReturnValue %894
%687 = OpLabel
%895 = OpLoad %v4float %662
%896 = OpCompositeExtract %float %895 3
%897 = OpFOrdEqual %bool %896 %float_0
OpSelectionMerge %901 None
OpBranchConditional %897 %899 %900
%899 = OpLabel
%902 = OpLoad %v4float %661
OpStore %898 %902
OpBranch %901
%900 = OpLabel
%903 = OpLoad %v4float %661
%904 = OpVectorShuffle %v2float %903 %903 0 3
OpStore %905 %904
%906 = OpLoad %v4float %662
%907 = OpVectorShuffle %v2float %906 %906 0 3
OpStore %908 %907
%909 = OpFunctionCall %float %_soft_light_component_hh2h2 %905 %908
%910 = OpLoad %v4float %661
%911 = OpVectorShuffle %v2float %910 %910 1 3
OpStore %912 %911
%913 = OpLoad %v4float %662
%914 = OpVectorShuffle %v2float %913 %913 1 3
OpStore %915 %914
%916 = OpFunctionCall %float %_soft_light_component_hh2h2 %912 %915
%917 = OpLoad %v4float %661
%918 = OpVectorShuffle %v2float %917 %917 2 3
OpStore %919 %918
%920 = OpLoad %v4float %662
%921 = OpVectorShuffle %v2float %920 %920 2 3
OpStore %922 %921
%923 = OpFunctionCall %float %_soft_light_component_hh2h2 %919 %922
%924 = OpLoad %v4float %661
%925 = OpCompositeExtract %float %924 3
%926 = OpLoad %v4float %661
%927 = OpCompositeExtract %float %926 3
%928 = OpFSub %float %float_1 %927
%929 = OpLoad %v4float %662
%930 = OpCompositeExtract %float %929 3
%931 = OpFMul %float %928 %930
%932 = OpFAdd %float %925 %931
%933 = OpCompositeConstruct %v4float %909 %916 %923 %932
OpStore %898 %933
OpBranch %901
%901 = OpLabel
%934 = OpLoad %v4float %898
OpReturnValue %934
%688 = OpLabel
%935 = OpLoad %v4float %661
%936 = OpVectorShuffle %v3float %935 %935 0 1 2
%937 = OpLoad %v4float %662
%938 = OpVectorShuffle %v3float %937 %937 0 1 2
%939 = OpFAdd %v3float %936 %938
%941 = OpLoad %v4float %661
%942 = OpVectorShuffle %v3float %941 %941 0 1 2
%943 = OpLoad %v4float %662
%944 = OpCompositeExtract %float %943 3
%945 = OpVectorTimesScalar %v3float %942 %944
%946 = OpLoad %v4float %662
%947 = OpVectorShuffle %v3float %946 %946 0 1 2
%948 = OpLoad %v4float %661
%949 = OpCompositeExtract %float %948 3
%950 = OpVectorTimesScalar %v3float %947 %949
%940 = OpExtInst %v3float %1 FMin %945 %950
%951 = OpVectorTimesScalar %v3float %940 %float_2
%952 = OpFSub %v3float %939 %951
%953 = OpCompositeExtract %float %952 0
%954 = OpCompositeExtract %float %952 1
%955 = OpCompositeExtract %float %952 2
%956 = OpLoad %v4float %661
%957 = OpCompositeExtract %float %956 3
%958 = OpLoad %v4float %661
%959 = OpCompositeExtract %float %958 3
%960 = OpFSub %float %float_1 %959
%961 = OpLoad %v4float %662
%962 = OpCompositeExtract %float %961 3
%963 = OpFMul %float %960 %962
%964 = OpFAdd %float %957 %963
%965 = OpCompositeConstruct %v4float %953 %954 %955 %964
OpReturnValue %965
%689 = OpLabel
%966 = OpLoad %v4float %662
%967 = OpVectorShuffle %v3float %966 %966 0 1 2
%968 = OpLoad %v4float %661
%969 = OpVectorShuffle %v3float %968 %968 0 1 2
%970 = OpFAdd %v3float %967 %969
%971 = OpLoad %v4float %662
%972 = OpVectorShuffle %v3float %971 %971 0 1 2
%973 = OpVectorTimesScalar %v3float %972 %float_2
%974 = OpLoad %v4float %661
%975 = OpVectorShuffle %v3float %974 %974 0 1 2
%976 = OpFMul %v3float %973 %975
%977 = OpFSub %v3float %970 %976
%978 = OpCompositeExtract %float %977 0
%979 = OpCompositeExtract %float %977 1
%980 = OpCompositeExtract %float %977 2
%981 = OpLoad %v4float %661
%982 = OpCompositeExtract %float %981 3
%983 = OpLoad %v4float %661
%984 = OpCompositeExtract %float %983 3
%985 = OpFSub %float %float_1 %984
%986 = OpLoad %v4float %662
%987 = OpCompositeExtract %float %986 3
%988 = OpFMul %float %985 %987
%989 = OpFAdd %float %982 %988
%990 = OpCompositeConstruct %v4float %978 %979 %980 %989
OpReturnValue %990
%690 = OpLabel
%991 = OpLoad %v4float %661
%992 = OpCompositeExtract %float %991 3
%993 = OpFSub %float %float_1 %992
%994 = OpLoad %v4float %662
%995 = OpVectorShuffle %v3float %994 %994 0 1 2
%996 = OpVectorTimesScalar %v3float %995 %993
%997 = OpLoad %v4float %662
%998 = OpCompositeExtract %float %997 3
%999 = OpFSub %float %float_1 %998
%1000 = OpLoad %v4float %661
%1001 = OpVectorShuffle %v3float %1000 %1000 0 1 2
%1002 = OpVectorTimesScalar %v3float %1001 %999
%1003 = OpFAdd %v3float %996 %1002
%1004 = OpLoad %v4float %661
%1005 = OpVectorShuffle %v3float %1004 %1004 0 1 2
%1006 = OpLoad %v4float %662
%1007 = OpVectorShuffle %v3float %1006 %1006 0 1 2
%1008 = OpFMul %v3float %1005 %1007
%1009 = OpFAdd %v3float %1003 %1008
%1010 = OpCompositeExtract %float %1009 0
%1011 = OpCompositeExtract %float %1009 1
%1012 = OpCompositeExtract %float %1009 2
%1013 = OpLoad %v4float %661
%1014 = OpCompositeExtract %float %1013 3
%1015 = OpLoad %v4float %661
%1016 = OpCompositeExtract %float %1015 3
%1017 = OpFSub %float %float_1 %1016
%1018 = OpLoad %v4float %662
%1019 = OpCompositeExtract %float %1018 3
%1020 = OpFMul %float %1017 %1019
%1021 = OpFAdd %float %1014 %1020
%1022 = OpCompositeConstruct %v4float %1010 %1011 %1012 %1021
OpReturnValue %1022
%691 = OpLabel
%1024 = OpLoad %v4float %662
%1025 = OpCompositeExtract %float %1024 3
%1026 = OpLoad %v4float %661
%1027 = OpCompositeExtract %float %1026 3
%1028 = OpFMul %float %1025 %1027
OpStore %_2_alpha %1028
%1030 = OpLoad %v4float %661
%1031 = OpVectorShuffle %v3float %1030 %1030 0 1 2
%1032 = OpLoad %v4float %662
%1033 = OpCompositeExtract %float %1032 3
%1034 = OpVectorTimesScalar %v3float %1031 %1033
OpStore %_3_sda %1034
%1036 = OpLoad %v4float %662
%1037 = OpVectorShuffle %v3float %1036 %1036 0 1 2
%1038 = OpLoad %v4float %661
%1039 = OpCompositeExtract %float %1038 3
%1040 = OpVectorTimesScalar %v3float %1037 %1039
OpStore %_4_dsa %1040
%1041 = OpLoad %v3float %_3_sda
OpStore %1042 %1041
%1043 = OpLoad %v3float %_4_dsa
OpStore %1044 %1043
%1045 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %1042 %1044
OpStore %1046 %1045
%1047 = OpLoad %float %_2_alpha
OpStore %1048 %1047
%1049 = OpLoad %v3float %_4_dsa
OpStore %1050 %1049
%1051 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1046 %1048 %1050
%1052 = OpLoad %v4float %662
%1053 = OpVectorShuffle %v3float %1052 %1052 0 1 2
%1054 = OpFAdd %v3float %1051 %1053
%1055 = OpLoad %v3float %_4_dsa
%1056 = OpFSub %v3float %1054 %1055
%1057 = OpLoad %v4float %661
%1058 = OpVectorShuffle %v3float %1057 %1057 0 1 2
%1059 = OpFAdd %v3float %1056 %1058
%1060 = OpLoad %v3float %_3_sda
%1061 = OpFSub %v3float %1059 %1060
%1062 = OpCompositeExtract %float %1061 0
%1063 = OpCompositeExtract %float %1061 1
%1064 = OpCompositeExtract %float %1061 2
%1065 = OpLoad %v4float %661
%1066 = OpCompositeExtract %float %1065 3
%1067 = OpLoad %v4float %662
%1068 = OpCompositeExtract %float %1067 3
%1069 = OpFAdd %float %1066 %1068
%1070 = OpLoad %float %_2_alpha
%1071 = OpFSub %float %1069 %1070
%1072 = OpCompositeConstruct %v4float %1062 %1063 %1064 %1071
OpReturnValue %1072
%692 = OpLabel
%1074 = OpLoad %v4float %662
%1075 = OpCompositeExtract %float %1074 3
%1076 = OpLoad %v4float %661
%1077 = OpCompositeExtract %float %1076 3
%1078 = OpFMul %float %1075 %1077
OpStore %_5_alpha %1078
%1080 = OpLoad %v4float %661
%1081 = OpVectorShuffle %v3float %1080 %1080 0 1 2
%1082 = OpLoad %v4float %662
%1083 = OpCompositeExtract %float %1082 3
%1084 = OpVectorTimesScalar %v3float %1081 %1083
OpStore %_6_sda %1084
%1086 = OpLoad %v4float %662
%1087 = OpVectorShuffle %v3float %1086 %1086 0 1 2
%1088 = OpLoad %v4float %661
%1089 = OpCompositeExtract %float %1088 3
%1090 = OpVectorTimesScalar %v3float %1087 %1089
OpStore %_7_dsa %1090
%1091 = OpLoad %v3float %_7_dsa
OpStore %1092 %1091
%1093 = OpLoad %v3float %_6_sda
OpStore %1094 %1093
%1095 = OpFunctionCall %v3float %_blend_set_color_saturation_h3h3h3 %1092 %1094
OpStore %1096 %1095
%1097 = OpLoad %float %_5_alpha
OpStore %1098 %1097
%1099 = OpLoad %v3float %_7_dsa
OpStore %1100 %1099
%1101 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1096 %1098 %1100
%1102 = OpLoad %v4float %662
%1103 = OpVectorShuffle %v3float %1102 %1102 0 1 2
%1104 = OpFAdd %v3float %1101 %1103
%1105 = OpLoad %v3float %_7_dsa
%1106 = OpFSub %v3float %1104 %1105
%1107 = OpLoad %v4float %661
%1108 = OpVectorShuffle %v3float %1107 %1107 0 1 2
%1109 = OpFAdd %v3float %1106 %1108
%1110 = OpLoad %v3float %_6_sda
%1111 = OpFSub %v3float %1109 %1110
%1112 = OpCompositeExtract %float %1111 0
%1113 = OpCompositeExtract %float %1111 1
%1114 = OpCompositeExtract %float %1111 2
%1115 = OpLoad %v4float %661
%1116 = OpCompositeExtract %float %1115 3
%1117 = OpLoad %v4float %662
%1118 = OpCompositeExtract %float %1117 3
%1119 = OpFAdd %float %1116 %1118
%1120 = OpLoad %float %_5_alpha
%1121 = OpFSub %float %1119 %1120
%1122 = OpCompositeConstruct %v4float %1112 %1113 %1114 %1121
OpReturnValue %1122
%693 = OpLabel
%1124 = OpLoad %v4float %662
%1125 = OpCompositeExtract %float %1124 3
%1126 = OpLoad %v4float %661
%1127 = OpCompositeExtract %float %1126 3
%1128 = OpFMul %float %1125 %1127
OpStore %_8_alpha %1128
%1130 = OpLoad %v4float %661
%1131 = OpVectorShuffle %v3float %1130 %1130 0 1 2
%1132 = OpLoad %v4float %662
%1133 = OpCompositeExtract %float %1132 3
%1134 = OpVectorTimesScalar %v3float %1131 %1133
OpStore %_9_sda %1134
%1136 = OpLoad %v4float %662
%1137 = OpVectorShuffle %v3float %1136 %1136 0 1 2
%1138 = OpLoad %v4float %661
%1139 = OpCompositeExtract %float %1138 3
%1140 = OpVectorTimesScalar %v3float %1137 %1139
OpStore %_10_dsa %1140
%1141 = OpLoad %v3float %_9_sda
OpStore %1142 %1141
%1143 = OpLoad %float %_8_alpha
OpStore %1144 %1143
%1145 = OpLoad %v3float %_10_dsa
OpStore %1146 %1145
%1147 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1142 %1144 %1146
%1148 = OpLoad %v4float %662
%1149 = OpVectorShuffle %v3float %1148 %1148 0 1 2
%1150 = OpFAdd %v3float %1147 %1149
%1151 = OpLoad %v3float %_10_dsa
%1152 = OpFSub %v3float %1150 %1151
%1153 = OpLoad %v4float %661
%1154 = OpVectorShuffle %v3float %1153 %1153 0 1 2
%1155 = OpFAdd %v3float %1152 %1154
%1156 = OpLoad %v3float %_9_sda
%1157 = OpFSub %v3float %1155 %1156
%1158 = OpCompositeExtract %float %1157 0
%1159 = OpCompositeExtract %float %1157 1
%1160 = OpCompositeExtract %float %1157 2
%1161 = OpLoad %v4float %661
%1162 = OpCompositeExtract %float %1161 3
%1163 = OpLoad %v4float %662
%1164 = OpCompositeExtract %float %1163 3
%1165 = OpFAdd %float %1162 %1164
%1166 = OpLoad %float %_8_alpha
%1167 = OpFSub %float %1165 %1166
%1168 = OpCompositeConstruct %v4float %1158 %1159 %1160 %1167
OpReturnValue %1168
%694 = OpLabel
%1170 = OpLoad %v4float %662
%1171 = OpCompositeExtract %float %1170 3
%1172 = OpLoad %v4float %661
%1173 = OpCompositeExtract %float %1172 3
%1174 = OpFMul %float %1171 %1173
OpStore %_11_alpha %1174
%1176 = OpLoad %v4float %661
%1177 = OpVectorShuffle %v3float %1176 %1176 0 1 2
%1178 = OpLoad %v4float %662
%1179 = OpCompositeExtract %float %1178 3
%1180 = OpVectorTimesScalar %v3float %1177 %1179
OpStore %_12_sda %1180
%1182 = OpLoad %v4float %662
%1183 = OpVectorShuffle %v3float %1182 %1182 0 1 2
%1184 = OpLoad %v4float %661
%1185 = OpCompositeExtract %float %1184 3
%1186 = OpVectorTimesScalar %v3float %1183 %1185
OpStore %_13_dsa %1186
%1187 = OpLoad %v3float %_13_dsa
OpStore %1188 %1187
%1189 = OpLoad %float %_11_alpha
OpStore %1190 %1189
%1191 = OpLoad %v3float %_12_sda
OpStore %1192 %1191
%1193 = OpFunctionCall %v3float %_blend_set_color_luminance_h3h3hh3 %1188 %1190 %1192
%1194 = OpLoad %v4float %662
%1195 = OpVectorShuffle %v3float %1194 %1194 0 1 2
%1196 = OpFAdd %v3float %1193 %1195
%1197 = OpLoad %v3float %_13_dsa
%1198 = OpFSub %v3float %1196 %1197
%1199 = OpLoad %v4float %661
%1200 = OpVectorShuffle %v3float %1199 %1199 0 1 2
%1201 = OpFAdd %v3float %1198 %1200
%1202 = OpLoad %v3float %_12_sda
%1203 = OpFSub %v3float %1201 %1202
%1204 = OpCompositeExtract %float %1203 0
%1205 = OpCompositeExtract %float %1203 1
%1206 = OpCompositeExtract %float %1203 2
%1207 = OpLoad %v4float %661
%1208 = OpCompositeExtract %float %1207 3
%1209 = OpLoad %v4float %662
%1210 = OpCompositeExtract %float %1209 3
%1211 = OpFAdd %float %1208 %1210
%1212 = OpLoad %float %_11_alpha
%1213 = OpFSub %float %1211 %1212
%1214 = OpCompositeConstruct %v4float %1204 %1205 %1206 %1213
OpReturnValue %1214
%695 = OpLabel
OpReturnValue %696
%665 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %1216
%1217 = OpLabel
%1219 = OpVariable %_ptr_Function_int Function
%1224 = OpVariable %_ptr_Function_v4float Function
%1228 = OpVariable %_ptr_Function_v4float Function
OpStore %1219 %int_13
%1220 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%1223 = OpLoad %v4float %1220
OpStore %1224 %1223
%1225 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%1227 = OpLoad %v4float %1225
OpStore %1228 %1227
%1229 = OpFunctionCall %v4float %blend_h4eh4h4 %1219 %1224 %1228
OpStore %sk_FragColor %1229
OpReturn
OpFunctionEnd
