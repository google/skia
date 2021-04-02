OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %vLocalCoord_Stage0
OpExecutionMode %main OriginUpperLeft
OpName %uniformBuffer "uniformBuffer"
OpMemberName %uniformBuffer 0 "sk_RTAdjust"
OpMemberName %uniformBuffer 1 "uIncrement_Stage1_c0"
OpMemberName %uniformBuffer 2 "uKernel_Stage1_c0"
OpMemberName %uniformBuffer 3 "umatrix_Stage1_c0_c0"
OpMemberName %uniformBuffer 4 "uborder_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 5 "usubset_Stage1_c0_c0_c0"
OpMemberName %uniformBuffer 6 "unorm_Stage1_c0_c0_c0"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %uTextureSampler_0_Stage1 "uTextureSampler_0_Stage1"
OpName %vLocalCoord_Stage0 "vLocalCoord_Stage0"
OpName %MatrixEffect_Stage1_c0_c0_h4h4f2 "MatrixEffect_Stage1_c0_c0_h4h4f2"
OpName %_1_inCoord "_1_inCoord"
OpName %_2_subsetCoord "_2_subsetCoord"
OpName %_3_clampedCoord "_3_clampedCoord"
OpName %_4_textureColor "_4_textureColor"
OpName %_5_snappedX "_5_snappedX"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpName %_6_output "_6_output"
OpName %_7_coord "_7_coord"
OpName %_8_coordSampled "_8_coordSampled"
OpDecorate %_arr_v4float_int_7 ArrayStride 16
OpMemberDecorate %uniformBuffer 0 Offset 0
OpMemberDecorate %uniformBuffer 1 Offset 16
OpMemberDecorate %uniformBuffer 1 RelaxedPrecision
OpMemberDecorate %uniformBuffer 2 Offset 32
OpMemberDecorate %uniformBuffer 2 RelaxedPrecision
OpMemberDecorate %uniformBuffer 3 Offset 144
OpMemberDecorate %uniformBuffer 3 ColMajor
OpMemberDecorate %uniformBuffer 3 MatrixStride 16
OpMemberDecorate %uniformBuffer 4 Offset 192
OpMemberDecorate %uniformBuffer 4 RelaxedPrecision
OpMemberDecorate %uniformBuffer 5 Offset 208
OpMemberDecorate %uniformBuffer 6 Offset 224
OpDecorate %uniformBuffer Block
OpDecorate %4 Binding 0
OpDecorate %4 DescriptorSet 0
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %_4_textureColor RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %outputColor_Stage0 RelaxedPrecision
OpDecorate %outputCoverage_Stage0 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_6_output RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
OpDecorate %534 RelaxedPrecision
OpDecorate %537 RelaxedPrecision
OpDecorate %538 RelaxedPrecision
OpDecorate %539 RelaxedPrecision
OpDecorate %543 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %551 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %556 RelaxedPrecision
OpDecorate %557 RelaxedPrecision
OpDecorate %558 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %563 RelaxedPrecision
OpDecorate %569 RelaxedPrecision
OpDecorate %570 RelaxedPrecision
OpDecorate %571 RelaxedPrecision
OpDecorate %572 RelaxedPrecision
OpDecorate %575 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %577 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %589 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %596 RelaxedPrecision
OpDecorate %599 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %602 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_7 = OpConstant %int 7
%_arr_v4float_int_7 = OpTypeArray %v4float %int_7
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%uniformBuffer = OpTypeStruct %v4float %v2float %_arr_v4float_int_7 %mat3v3float %v4float %v4float %v4float
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
%4 = OpVariable %_ptr_Uniform_uniformBuffer Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%23 = OpTypeImage %float 2D 0 0 0 1 Unknown
%22 = OpTypeSampledImage %23
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%uTextureSampler_0_Stage1 = OpVariable %_ptr_UniformConstant_22 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
%vLocalCoord_Stage0 = OpVariable %_ptr_Input_v2float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v2float
%int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%float_1 = OpConstant %float 1
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_0_00100000005 = OpConstant %float 0.00100000005
%float_0_5 = OpConstant %float 0.5
%true = OpConstantTrue %bool
%int_5 = OpConstant %int 5
%int_4 = OpConstant %int 4
%void = OpTypeVoid
%101 = OpTypeFunction %void
%105 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%109 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%122 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0_h4h4f2 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_1_inCoord = OpVariable %_ptr_Function_v2float Function
%_2_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_3_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_4_textureColor = OpVariable %_ptr_Function_v4float Function
%_5_snappedX = OpVariable %_ptr_Function_float Function
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%36 = OpLoad %mat3v3float %34
%37 = OpLoad %v2float %30
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpCompositeConstruct %v3float %38 %39 %float_1
%42 = OpMatrixTimesVector %v3float %36 %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
OpStore %_1_inCoord %43
%44 = OpLoad %v2float %_1_inCoord
%46 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%48 = OpLoad %v4float %46
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFMul %v2float %44 %49
OpStore %_1_inCoord %50
%52 = OpLoad %v2float %_1_inCoord
%53 = OpCompositeExtract %float %52 0
%54 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_0
OpStore %54 %53
%57 = OpLoad %v2float %_1_inCoord
%58 = OpCompositeExtract %float %57 1
%59 = OpAccessChain %_ptr_Function_float %_2_subsetCoord %int_1
OpStore %59 %58
%62 = OpLoad %v2float %_2_subsetCoord
OpStore %_3_clampedCoord %62
%65 = OpLoad %22 %uTextureSampler_0_Stage1
%66 = OpLoad %v2float %_3_clampedCoord
%67 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%68 = OpLoad %v4float %67
%69 = OpVectorShuffle %v2float %68 %68 2 3
%70 = OpFMul %v2float %66 %69
%64 = OpImageSampleImplicitLod %v4float %65 %70
OpStore %_4_textureColor %64
%73 = OpLoad %v2float %_1_inCoord
%74 = OpCompositeExtract %float %73 0
%76 = OpFAdd %float %74 %float_0_00100000005
%72 = OpExtInst %float %1 Floor %76
%78 = OpFAdd %float %72 %float_0_5
OpStore %_5_snappedX %78
%80 = OpLoad %float %_5_snappedX
%82 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%83 = OpLoad %v4float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpFOrdLessThan %bool %80 %84
OpSelectionMerge %87 None
OpBranchConditional %85 %87 %86
%86 = OpLabel
%88 = OpLoad %float %_5_snappedX
%89 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 2
%92 = OpFOrdGreaterThan %bool %88 %91
OpBranch %87
%87 = OpLabel
%93 = OpPhi %bool %true %31 %92 %86
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%98 = OpLoad %v4float %97
OpStore %_4_textureColor %98
OpBranch %95
%95 = OpLabel
%99 = OpLoad %v4float %_4_textureColor
OpReturnValue %99
OpFunctionEnd
%main = OpFunction %void None %101
%102 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_6_output = OpVariable %_ptr_Function_v4float Function
%_7_coord = OpVariable %_ptr_Function_v2float Function
%_8_coordSampled = OpVariable %_ptr_Function_v2float Function
%126 = OpVariable %_ptr_Function_v4float Function
%128 = OpVariable %_ptr_Function_v2float Function
%146 = OpVariable %_ptr_Function_v4float Function
%148 = OpVariable %_ptr_Function_v2float Function
%165 = OpVariable %_ptr_Function_v4float Function
%167 = OpVariable %_ptr_Function_v2float Function
%184 = OpVariable %_ptr_Function_v4float Function
%186 = OpVariable %_ptr_Function_v2float Function
%203 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_v2float Function
%222 = OpVariable %_ptr_Function_v4float Function
%224 = OpVariable %_ptr_Function_v2float Function
%241 = OpVariable %_ptr_Function_v4float Function
%243 = OpVariable %_ptr_Function_v2float Function
%260 = OpVariable %_ptr_Function_v4float Function
%262 = OpVariable %_ptr_Function_v2float Function
%279 = OpVariable %_ptr_Function_v4float Function
%281 = OpVariable %_ptr_Function_v2float Function
%298 = OpVariable %_ptr_Function_v4float Function
%300 = OpVariable %_ptr_Function_v2float Function
%317 = OpVariable %_ptr_Function_v4float Function
%319 = OpVariable %_ptr_Function_v2float Function
%336 = OpVariable %_ptr_Function_v4float Function
%338 = OpVariable %_ptr_Function_v2float Function
%355 = OpVariable %_ptr_Function_v4float Function
%357 = OpVariable %_ptr_Function_v2float Function
%374 = OpVariable %_ptr_Function_v4float Function
%376 = OpVariable %_ptr_Function_v2float Function
%393 = OpVariable %_ptr_Function_v4float Function
%395 = OpVariable %_ptr_Function_v2float Function
%412 = OpVariable %_ptr_Function_v4float Function
%414 = OpVariable %_ptr_Function_v2float Function
%431 = OpVariable %_ptr_Function_v4float Function
%433 = OpVariable %_ptr_Function_v2float Function
%450 = OpVariable %_ptr_Function_v4float Function
%452 = OpVariable %_ptr_Function_v2float Function
%469 = OpVariable %_ptr_Function_v4float Function
%471 = OpVariable %_ptr_Function_v2float Function
%488 = OpVariable %_ptr_Function_v4float Function
%490 = OpVariable %_ptr_Function_v2float Function
%507 = OpVariable %_ptr_Function_v4float Function
%509 = OpVariable %_ptr_Function_v2float Function
%526 = OpVariable %_ptr_Function_v4float Function
%528 = OpVariable %_ptr_Function_v2float Function
%545 = OpVariable %_ptr_Function_v4float Function
%547 = OpVariable %_ptr_Function_v2float Function
%564 = OpVariable %_ptr_Function_v4float Function
%566 = OpVariable %_ptr_Function_v2float Function
%583 = OpVariable %_ptr_Function_v4float Function
%585 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %105
OpStore %outputCoverage_Stage0 %105
OpStore %_6_output %109
%111 = OpLoad %v2float %vLocalCoord_Stage0
%113 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%115 = OpLoad %v2float %113
%116 = OpVectorTimesScalar %v2float %115 %float_12
%117 = OpCompositeExtract %float %116 0
%118 = OpCompositeExtract %float %116 1
%119 = OpCompositeConstruct %v2float %117 %118
%120 = OpFSub %v2float %111 %119
OpStore %_7_coord %120
OpStore %_8_coordSampled %122
%123 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %123
%124 = OpLoad %v4float %_6_output
%125 = OpLoad %v4float %outputColor_Stage0
OpStore %126 %125
%127 = OpLoad %v2float %_8_coordSampled
OpStore %128 %127
%129 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %126 %128
%131 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%132 = OpLoad %v4float %131
%133 = OpCompositeExtract %float %132 0
%134 = OpVectorTimesScalar %v4float %129 %133
%135 = OpFAdd %v4float %124 %134
OpStore %_6_output %135
%136 = OpLoad %v2float %_7_coord
%137 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%138 = OpLoad %v2float %137
%139 = OpCompositeExtract %float %138 0
%140 = OpCompositeExtract %float %138 1
%141 = OpCompositeConstruct %v2float %139 %140
%142 = OpFAdd %v2float %136 %141
OpStore %_7_coord %142
%143 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %143
%144 = OpLoad %v4float %_6_output
%145 = OpLoad %v4float %outputColor_Stage0
OpStore %146 %145
%147 = OpLoad %v2float %_8_coordSampled
OpStore %148 %147
%149 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %146 %148
%150 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 1
%153 = OpVectorTimesScalar %v4float %149 %152
%154 = OpFAdd %v4float %144 %153
OpStore %_6_output %154
%155 = OpLoad %v2float %_7_coord
%156 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%157 = OpLoad %v2float %156
%158 = OpCompositeExtract %float %157 0
%159 = OpCompositeExtract %float %157 1
%160 = OpCompositeConstruct %v2float %158 %159
%161 = OpFAdd %v2float %155 %160
OpStore %_7_coord %161
%162 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %162
%163 = OpLoad %v4float %_6_output
%164 = OpLoad %v4float %outputColor_Stage0
OpStore %165 %164
%166 = OpLoad %v2float %_8_coordSampled
OpStore %167 %166
%168 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %165 %167
%169 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 2
%172 = OpVectorTimesScalar %v4float %168 %171
%173 = OpFAdd %v4float %163 %172
OpStore %_6_output %173
%174 = OpLoad %v2float %_7_coord
%175 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%176 = OpLoad %v2float %175
%177 = OpCompositeExtract %float %176 0
%178 = OpCompositeExtract %float %176 1
%179 = OpCompositeConstruct %v2float %177 %178
%180 = OpFAdd %v2float %174 %179
OpStore %_7_coord %180
%181 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %181
%182 = OpLoad %v4float %_6_output
%183 = OpLoad %v4float %outputColor_Stage0
OpStore %184 %183
%185 = OpLoad %v2float %_8_coordSampled
OpStore %186 %185
%187 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %184 %186
%188 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%189 = OpLoad %v4float %188
%190 = OpCompositeExtract %float %189 3
%191 = OpVectorTimesScalar %v4float %187 %190
%192 = OpFAdd %v4float %182 %191
OpStore %_6_output %192
%193 = OpLoad %v2float %_7_coord
%194 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%195 = OpLoad %v2float %194
%196 = OpCompositeExtract %float %195 0
%197 = OpCompositeExtract %float %195 1
%198 = OpCompositeConstruct %v2float %196 %197
%199 = OpFAdd %v2float %193 %198
OpStore %_7_coord %199
%200 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %200
%201 = OpLoad %v4float %_6_output
%202 = OpLoad %v4float %outputColor_Stage0
OpStore %203 %202
%204 = OpLoad %v2float %_8_coordSampled
OpStore %205 %204
%206 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %203 %205
%207 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpVectorTimesScalar %v4float %206 %209
%211 = OpFAdd %v4float %201 %210
OpStore %_6_output %211
%212 = OpLoad %v2float %_7_coord
%213 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%214 = OpLoad %v2float %213
%215 = OpCompositeExtract %float %214 0
%216 = OpCompositeExtract %float %214 1
%217 = OpCompositeConstruct %v2float %215 %216
%218 = OpFAdd %v2float %212 %217
OpStore %_7_coord %218
%219 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %219
%220 = OpLoad %v4float %_6_output
%221 = OpLoad %v4float %outputColor_Stage0
OpStore %222 %221
%223 = OpLoad %v2float %_8_coordSampled
OpStore %224 %223
%225 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %222 %224
%226 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%227 = OpLoad %v4float %226
%228 = OpCompositeExtract %float %227 1
%229 = OpVectorTimesScalar %v4float %225 %228
%230 = OpFAdd %v4float %220 %229
OpStore %_6_output %230
%231 = OpLoad %v2float %_7_coord
%232 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%233 = OpLoad %v2float %232
%234 = OpCompositeExtract %float %233 0
%235 = OpCompositeExtract %float %233 1
%236 = OpCompositeConstruct %v2float %234 %235
%237 = OpFAdd %v2float %231 %236
OpStore %_7_coord %237
%238 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %238
%239 = OpLoad %v4float %_6_output
%240 = OpLoad %v4float %outputColor_Stage0
OpStore %241 %240
%242 = OpLoad %v2float %_8_coordSampled
OpStore %243 %242
%244 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %241 %243
%245 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%246 = OpLoad %v4float %245
%247 = OpCompositeExtract %float %246 2
%248 = OpVectorTimesScalar %v4float %244 %247
%249 = OpFAdd %v4float %239 %248
OpStore %_6_output %249
%250 = OpLoad %v2float %_7_coord
%251 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%252 = OpLoad %v2float %251
%253 = OpCompositeExtract %float %252 0
%254 = OpCompositeExtract %float %252 1
%255 = OpCompositeConstruct %v2float %253 %254
%256 = OpFAdd %v2float %250 %255
OpStore %_7_coord %256
%257 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %257
%258 = OpLoad %v4float %_6_output
%259 = OpLoad %v4float %outputColor_Stage0
OpStore %260 %259
%261 = OpLoad %v2float %_8_coordSampled
OpStore %262 %261
%263 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %260 %262
%264 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%265 = OpLoad %v4float %264
%266 = OpCompositeExtract %float %265 3
%267 = OpVectorTimesScalar %v4float %263 %266
%268 = OpFAdd %v4float %258 %267
OpStore %_6_output %268
%269 = OpLoad %v2float %_7_coord
%270 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%271 = OpLoad %v2float %270
%272 = OpCompositeExtract %float %271 0
%273 = OpCompositeExtract %float %271 1
%274 = OpCompositeConstruct %v2float %272 %273
%275 = OpFAdd %v2float %269 %274
OpStore %_7_coord %275
%276 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %276
%277 = OpLoad %v4float %_6_output
%278 = OpLoad %v4float %outputColor_Stage0
OpStore %279 %278
%280 = OpLoad %v2float %_8_coordSampled
OpStore %281 %280
%282 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %279 %281
%283 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%284 = OpLoad %v4float %283
%285 = OpCompositeExtract %float %284 0
%286 = OpVectorTimesScalar %v4float %282 %285
%287 = OpFAdd %v4float %277 %286
OpStore %_6_output %287
%288 = OpLoad %v2float %_7_coord
%289 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%290 = OpLoad %v2float %289
%291 = OpCompositeExtract %float %290 0
%292 = OpCompositeExtract %float %290 1
%293 = OpCompositeConstruct %v2float %291 %292
%294 = OpFAdd %v2float %288 %293
OpStore %_7_coord %294
%295 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %295
%296 = OpLoad %v4float %_6_output
%297 = OpLoad %v4float %outputColor_Stage0
OpStore %298 %297
%299 = OpLoad %v2float %_8_coordSampled
OpStore %300 %299
%301 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %298 %300
%302 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%303 = OpLoad %v4float %302
%304 = OpCompositeExtract %float %303 1
%305 = OpVectorTimesScalar %v4float %301 %304
%306 = OpFAdd %v4float %296 %305
OpStore %_6_output %306
%307 = OpLoad %v2float %_7_coord
%308 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%309 = OpLoad %v2float %308
%310 = OpCompositeExtract %float %309 0
%311 = OpCompositeExtract %float %309 1
%312 = OpCompositeConstruct %v2float %310 %311
%313 = OpFAdd %v2float %307 %312
OpStore %_7_coord %313
%314 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %314
%315 = OpLoad %v4float %_6_output
%316 = OpLoad %v4float %outputColor_Stage0
OpStore %317 %316
%318 = OpLoad %v2float %_8_coordSampled
OpStore %319 %318
%320 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %317 %319
%321 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%322 = OpLoad %v4float %321
%323 = OpCompositeExtract %float %322 2
%324 = OpVectorTimesScalar %v4float %320 %323
%325 = OpFAdd %v4float %315 %324
OpStore %_6_output %325
%326 = OpLoad %v2float %_7_coord
%327 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%328 = OpLoad %v2float %327
%329 = OpCompositeExtract %float %328 0
%330 = OpCompositeExtract %float %328 1
%331 = OpCompositeConstruct %v2float %329 %330
%332 = OpFAdd %v2float %326 %331
OpStore %_7_coord %332
%333 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %333
%334 = OpLoad %v4float %_6_output
%335 = OpLoad %v4float %outputColor_Stage0
OpStore %336 %335
%337 = OpLoad %v2float %_8_coordSampled
OpStore %338 %337
%339 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %336 %338
%340 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%341 = OpLoad %v4float %340
%342 = OpCompositeExtract %float %341 3
%343 = OpVectorTimesScalar %v4float %339 %342
%344 = OpFAdd %v4float %334 %343
OpStore %_6_output %344
%345 = OpLoad %v2float %_7_coord
%346 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%347 = OpLoad %v2float %346
%348 = OpCompositeExtract %float %347 0
%349 = OpCompositeExtract %float %347 1
%350 = OpCompositeConstruct %v2float %348 %349
%351 = OpFAdd %v2float %345 %350
OpStore %_7_coord %351
%352 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %352
%353 = OpLoad %v4float %_6_output
%354 = OpLoad %v4float %outputColor_Stage0
OpStore %355 %354
%356 = OpLoad %v2float %_8_coordSampled
OpStore %357 %356
%358 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %355 %357
%359 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%360 = OpLoad %v4float %359
%361 = OpCompositeExtract %float %360 0
%362 = OpVectorTimesScalar %v4float %358 %361
%363 = OpFAdd %v4float %353 %362
OpStore %_6_output %363
%364 = OpLoad %v2float %_7_coord
%365 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%366 = OpLoad %v2float %365
%367 = OpCompositeExtract %float %366 0
%368 = OpCompositeExtract %float %366 1
%369 = OpCompositeConstruct %v2float %367 %368
%370 = OpFAdd %v2float %364 %369
OpStore %_7_coord %370
%371 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %371
%372 = OpLoad %v4float %_6_output
%373 = OpLoad %v4float %outputColor_Stage0
OpStore %374 %373
%375 = OpLoad %v2float %_8_coordSampled
OpStore %376 %375
%377 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %374 %376
%378 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%379 = OpLoad %v4float %378
%380 = OpCompositeExtract %float %379 1
%381 = OpVectorTimesScalar %v4float %377 %380
%382 = OpFAdd %v4float %372 %381
OpStore %_6_output %382
%383 = OpLoad %v2float %_7_coord
%384 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%385 = OpLoad %v2float %384
%386 = OpCompositeExtract %float %385 0
%387 = OpCompositeExtract %float %385 1
%388 = OpCompositeConstruct %v2float %386 %387
%389 = OpFAdd %v2float %383 %388
OpStore %_7_coord %389
%390 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %390
%391 = OpLoad %v4float %_6_output
%392 = OpLoad %v4float %outputColor_Stage0
OpStore %393 %392
%394 = OpLoad %v2float %_8_coordSampled
OpStore %395 %394
%396 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %393 %395
%397 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%398 = OpLoad %v4float %397
%399 = OpCompositeExtract %float %398 2
%400 = OpVectorTimesScalar %v4float %396 %399
%401 = OpFAdd %v4float %391 %400
OpStore %_6_output %401
%402 = OpLoad %v2float %_7_coord
%403 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%404 = OpLoad %v2float %403
%405 = OpCompositeExtract %float %404 0
%406 = OpCompositeExtract %float %404 1
%407 = OpCompositeConstruct %v2float %405 %406
%408 = OpFAdd %v2float %402 %407
OpStore %_7_coord %408
%409 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %409
%410 = OpLoad %v4float %_6_output
%411 = OpLoad %v4float %outputColor_Stage0
OpStore %412 %411
%413 = OpLoad %v2float %_8_coordSampled
OpStore %414 %413
%415 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %412 %414
%416 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%417 = OpLoad %v4float %416
%418 = OpCompositeExtract %float %417 3
%419 = OpVectorTimesScalar %v4float %415 %418
%420 = OpFAdd %v4float %410 %419
OpStore %_6_output %420
%421 = OpLoad %v2float %_7_coord
%422 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%423 = OpLoad %v2float %422
%424 = OpCompositeExtract %float %423 0
%425 = OpCompositeExtract %float %423 1
%426 = OpCompositeConstruct %v2float %424 %425
%427 = OpFAdd %v2float %421 %426
OpStore %_7_coord %427
%428 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %428
%429 = OpLoad %v4float %_6_output
%430 = OpLoad %v4float %outputColor_Stage0
OpStore %431 %430
%432 = OpLoad %v2float %_8_coordSampled
OpStore %433 %432
%434 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %431 %433
%435 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%436 = OpLoad %v4float %435
%437 = OpCompositeExtract %float %436 0
%438 = OpVectorTimesScalar %v4float %434 %437
%439 = OpFAdd %v4float %429 %438
OpStore %_6_output %439
%440 = OpLoad %v2float %_7_coord
%441 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%442 = OpLoad %v2float %441
%443 = OpCompositeExtract %float %442 0
%444 = OpCompositeExtract %float %442 1
%445 = OpCompositeConstruct %v2float %443 %444
%446 = OpFAdd %v2float %440 %445
OpStore %_7_coord %446
%447 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %447
%448 = OpLoad %v4float %_6_output
%449 = OpLoad %v4float %outputColor_Stage0
OpStore %450 %449
%451 = OpLoad %v2float %_8_coordSampled
OpStore %452 %451
%453 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %450 %452
%454 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%455 = OpLoad %v4float %454
%456 = OpCompositeExtract %float %455 1
%457 = OpVectorTimesScalar %v4float %453 %456
%458 = OpFAdd %v4float %448 %457
OpStore %_6_output %458
%459 = OpLoad %v2float %_7_coord
%460 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%461 = OpLoad %v2float %460
%462 = OpCompositeExtract %float %461 0
%463 = OpCompositeExtract %float %461 1
%464 = OpCompositeConstruct %v2float %462 %463
%465 = OpFAdd %v2float %459 %464
OpStore %_7_coord %465
%466 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %466
%467 = OpLoad %v4float %_6_output
%468 = OpLoad %v4float %outputColor_Stage0
OpStore %469 %468
%470 = OpLoad %v2float %_8_coordSampled
OpStore %471 %470
%472 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %469 %471
%473 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%474 = OpLoad %v4float %473
%475 = OpCompositeExtract %float %474 2
%476 = OpVectorTimesScalar %v4float %472 %475
%477 = OpFAdd %v4float %467 %476
OpStore %_6_output %477
%478 = OpLoad %v2float %_7_coord
%479 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%480 = OpLoad %v2float %479
%481 = OpCompositeExtract %float %480 0
%482 = OpCompositeExtract %float %480 1
%483 = OpCompositeConstruct %v2float %481 %482
%484 = OpFAdd %v2float %478 %483
OpStore %_7_coord %484
%485 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %485
%486 = OpLoad %v4float %_6_output
%487 = OpLoad %v4float %outputColor_Stage0
OpStore %488 %487
%489 = OpLoad %v2float %_8_coordSampled
OpStore %490 %489
%491 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %488 %490
%492 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%493 = OpLoad %v4float %492
%494 = OpCompositeExtract %float %493 3
%495 = OpVectorTimesScalar %v4float %491 %494
%496 = OpFAdd %v4float %486 %495
OpStore %_6_output %496
%497 = OpLoad %v2float %_7_coord
%498 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%499 = OpLoad %v2float %498
%500 = OpCompositeExtract %float %499 0
%501 = OpCompositeExtract %float %499 1
%502 = OpCompositeConstruct %v2float %500 %501
%503 = OpFAdd %v2float %497 %502
OpStore %_7_coord %503
%504 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %504
%505 = OpLoad %v4float %_6_output
%506 = OpLoad %v4float %outputColor_Stage0
OpStore %507 %506
%508 = OpLoad %v2float %_8_coordSampled
OpStore %509 %508
%510 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %507 %509
%511 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%512 = OpLoad %v4float %511
%513 = OpCompositeExtract %float %512 0
%514 = OpVectorTimesScalar %v4float %510 %513
%515 = OpFAdd %v4float %505 %514
OpStore %_6_output %515
%516 = OpLoad %v2float %_7_coord
%517 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%518 = OpLoad %v2float %517
%519 = OpCompositeExtract %float %518 0
%520 = OpCompositeExtract %float %518 1
%521 = OpCompositeConstruct %v2float %519 %520
%522 = OpFAdd %v2float %516 %521
OpStore %_7_coord %522
%523 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %523
%524 = OpLoad %v4float %_6_output
%525 = OpLoad %v4float %outputColor_Stage0
OpStore %526 %525
%527 = OpLoad %v2float %_8_coordSampled
OpStore %528 %527
%529 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %526 %528
%530 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%531 = OpLoad %v4float %530
%532 = OpCompositeExtract %float %531 1
%533 = OpVectorTimesScalar %v4float %529 %532
%534 = OpFAdd %v4float %524 %533
OpStore %_6_output %534
%535 = OpLoad %v2float %_7_coord
%536 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%537 = OpLoad %v2float %536
%538 = OpCompositeExtract %float %537 0
%539 = OpCompositeExtract %float %537 1
%540 = OpCompositeConstruct %v2float %538 %539
%541 = OpFAdd %v2float %535 %540
OpStore %_7_coord %541
%542 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %542
%543 = OpLoad %v4float %_6_output
%544 = OpLoad %v4float %outputColor_Stage0
OpStore %545 %544
%546 = OpLoad %v2float %_8_coordSampled
OpStore %547 %546
%548 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %545 %547
%549 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%550 = OpLoad %v4float %549
%551 = OpCompositeExtract %float %550 2
%552 = OpVectorTimesScalar %v4float %548 %551
%553 = OpFAdd %v4float %543 %552
OpStore %_6_output %553
%554 = OpLoad %v2float %_7_coord
%555 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%556 = OpLoad %v2float %555
%557 = OpCompositeExtract %float %556 0
%558 = OpCompositeExtract %float %556 1
%559 = OpCompositeConstruct %v2float %557 %558
%560 = OpFAdd %v2float %554 %559
OpStore %_7_coord %560
%561 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %561
%562 = OpLoad %v4float %_6_output
%563 = OpLoad %v4float %outputColor_Stage0
OpStore %564 %563
%565 = OpLoad %v2float %_8_coordSampled
OpStore %566 %565
%567 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %564 %566
%568 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%569 = OpLoad %v4float %568
%570 = OpCompositeExtract %float %569 3
%571 = OpVectorTimesScalar %v4float %567 %570
%572 = OpFAdd %v4float %562 %571
OpStore %_6_output %572
%573 = OpLoad %v2float %_7_coord
%574 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%575 = OpLoad %v2float %574
%576 = OpCompositeExtract %float %575 0
%577 = OpCompositeExtract %float %575 1
%578 = OpCompositeConstruct %v2float %576 %577
%579 = OpFAdd %v2float %573 %578
OpStore %_7_coord %579
%580 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %580
%581 = OpLoad %v4float %_6_output
%582 = OpLoad %v4float %outputColor_Stage0
OpStore %583 %582
%584 = OpLoad %v2float %_8_coordSampled
OpStore %585 %584
%586 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %583 %585
%587 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%588 = OpLoad %v4float %587
%589 = OpCompositeExtract %float %588 0
%590 = OpVectorTimesScalar %v4float %586 %589
%591 = OpFAdd %v4float %581 %590
OpStore %_6_output %591
%592 = OpLoad %v2float %_7_coord
%593 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%594 = OpLoad %v2float %593
%595 = OpCompositeExtract %float %594 0
%596 = OpCompositeExtract %float %594 1
%597 = OpCompositeConstruct %v2float %595 %596
%598 = OpFAdd %v2float %592 %597
OpStore %_7_coord %598
%599 = OpLoad %v4float %_6_output
%600 = OpLoad %v4float %outputColor_Stage0
%601 = OpFMul %v4float %599 %600
OpStore %_6_output %601
%602 = OpLoad %v4float %_6_output
OpStore %output_Stage1 %602
%603 = OpLoad %v4float %output_Stage1
%604 = OpLoad %v4float %outputCoverage_Stage0
%605 = OpFMul %v4float %603 %604
OpStore %sk_FragColor %605
OpReturn
OpFunctionEnd
