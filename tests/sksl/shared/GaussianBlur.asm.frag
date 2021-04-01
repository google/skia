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
OpDecorate %106 RelaxedPrecision
OpDecorate %output_Stage1 RelaxedPrecision
OpDecorate %_6_output RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %498 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %514 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %516 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%110 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%120 = OpConstantComposite %v2float %float_0 %float_0
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
%124 = OpVariable %_ptr_Function_v4float Function
%126 = OpVariable %_ptr_Function_v2float Function
%141 = OpVariable %_ptr_Function_v4float Function
%143 = OpVariable %_ptr_Function_v2float Function
%157 = OpVariable %_ptr_Function_v4float Function
%159 = OpVariable %_ptr_Function_v2float Function
%173 = OpVariable %_ptr_Function_v4float Function
%175 = OpVariable %_ptr_Function_v2float Function
%189 = OpVariable %_ptr_Function_v4float Function
%191 = OpVariable %_ptr_Function_v2float Function
%205 = OpVariable %_ptr_Function_v4float Function
%207 = OpVariable %_ptr_Function_v2float Function
%221 = OpVariable %_ptr_Function_v4float Function
%223 = OpVariable %_ptr_Function_v2float Function
%237 = OpVariable %_ptr_Function_v4float Function
%239 = OpVariable %_ptr_Function_v2float Function
%253 = OpVariable %_ptr_Function_v4float Function
%255 = OpVariable %_ptr_Function_v2float Function
%269 = OpVariable %_ptr_Function_v4float Function
%271 = OpVariable %_ptr_Function_v2float Function
%285 = OpVariable %_ptr_Function_v4float Function
%287 = OpVariable %_ptr_Function_v2float Function
%301 = OpVariable %_ptr_Function_v4float Function
%303 = OpVariable %_ptr_Function_v2float Function
%317 = OpVariable %_ptr_Function_v4float Function
%319 = OpVariable %_ptr_Function_v2float Function
%333 = OpVariable %_ptr_Function_v4float Function
%335 = OpVariable %_ptr_Function_v2float Function
%349 = OpVariable %_ptr_Function_v4float Function
%351 = OpVariable %_ptr_Function_v2float Function
%365 = OpVariable %_ptr_Function_v4float Function
%367 = OpVariable %_ptr_Function_v2float Function
%381 = OpVariable %_ptr_Function_v4float Function
%383 = OpVariable %_ptr_Function_v2float Function
%397 = OpVariable %_ptr_Function_v4float Function
%399 = OpVariable %_ptr_Function_v2float Function
%413 = OpVariable %_ptr_Function_v4float Function
%415 = OpVariable %_ptr_Function_v2float Function
%429 = OpVariable %_ptr_Function_v4float Function
%431 = OpVariable %_ptr_Function_v2float Function
%445 = OpVariable %_ptr_Function_v4float Function
%447 = OpVariable %_ptr_Function_v2float Function
%461 = OpVariable %_ptr_Function_v4float Function
%463 = OpVariable %_ptr_Function_v2float Function
%477 = OpVariable %_ptr_Function_v4float Function
%479 = OpVariable %_ptr_Function_v2float Function
%493 = OpVariable %_ptr_Function_v4float Function
%495 = OpVariable %_ptr_Function_v2float Function
%509 = OpVariable %_ptr_Function_v4float Function
%511 = OpVariable %_ptr_Function_v2float Function
%105 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
OpStore %outputColor_Stage0 %105
%106 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
OpStore %outputCoverage_Stage0 %106
OpStore %_6_output %110
%112 = OpLoad %v2float %vLocalCoord_Stage0
%114 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%116 = OpLoad %v2float %114
%117 = OpVectorTimesScalar %v2float %116 %float_12
%118 = OpFSub %v2float %112 %117
OpStore %_7_coord %118
OpStore %_8_coordSampled %120
%121 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %121
%122 = OpLoad %v4float %_6_output
%123 = OpLoad %v4float %outputColor_Stage0
OpStore %124 %123
%125 = OpLoad %v2float %_8_coordSampled
OpStore %126 %125
%127 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %124 %126
%129 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%130 = OpLoad %v4float %129
%131 = OpCompositeExtract %float %130 0
%132 = OpVectorTimesScalar %v4float %127 %131
%133 = OpFAdd %v4float %122 %132
OpStore %_6_output %133
%134 = OpLoad %v2float %_7_coord
%135 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%136 = OpLoad %v2float %135
%137 = OpFAdd %v2float %134 %136
OpStore %_7_coord %137
%138 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %138
%139 = OpLoad %v4float %_6_output
%140 = OpLoad %v4float %outputColor_Stage0
OpStore %141 %140
%142 = OpLoad %v2float %_8_coordSampled
OpStore %143 %142
%144 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %141 %143
%145 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 1
%148 = OpVectorTimesScalar %v4float %144 %147
%149 = OpFAdd %v4float %139 %148
OpStore %_6_output %149
%150 = OpLoad %v2float %_7_coord
%151 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%152 = OpLoad %v2float %151
%153 = OpFAdd %v2float %150 %152
OpStore %_7_coord %153
%154 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %154
%155 = OpLoad %v4float %_6_output
%156 = OpLoad %v4float %outputColor_Stage0
OpStore %157 %156
%158 = OpLoad %v2float %_8_coordSampled
OpStore %159 %158
%160 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %157 %159
%161 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%162 = OpLoad %v4float %161
%163 = OpCompositeExtract %float %162 2
%164 = OpVectorTimesScalar %v4float %160 %163
%165 = OpFAdd %v4float %155 %164
OpStore %_6_output %165
%166 = OpLoad %v2float %_7_coord
%167 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%168 = OpLoad %v2float %167
%169 = OpFAdd %v2float %166 %168
OpStore %_7_coord %169
%170 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %170
%171 = OpLoad %v4float %_6_output
%172 = OpLoad %v4float %outputColor_Stage0
OpStore %173 %172
%174 = OpLoad %v2float %_8_coordSampled
OpStore %175 %174
%176 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %173 %175
%177 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%178 = OpLoad %v4float %177
%179 = OpCompositeExtract %float %178 3
%180 = OpVectorTimesScalar %v4float %176 %179
%181 = OpFAdd %v4float %171 %180
OpStore %_6_output %181
%182 = OpLoad %v2float %_7_coord
%183 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%184 = OpLoad %v2float %183
%185 = OpFAdd %v2float %182 %184
OpStore %_7_coord %185
%186 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %186
%187 = OpLoad %v4float %_6_output
%188 = OpLoad %v4float %outputColor_Stage0
OpStore %189 %188
%190 = OpLoad %v2float %_8_coordSampled
OpStore %191 %190
%192 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %189 %191
%193 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 0
%196 = OpVectorTimesScalar %v4float %192 %195
%197 = OpFAdd %v4float %187 %196
OpStore %_6_output %197
%198 = OpLoad %v2float %_7_coord
%199 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%200 = OpLoad %v2float %199
%201 = OpFAdd %v2float %198 %200
OpStore %_7_coord %201
%202 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %202
%203 = OpLoad %v4float %_6_output
%204 = OpLoad %v4float %outputColor_Stage0
OpStore %205 %204
%206 = OpLoad %v2float %_8_coordSampled
OpStore %207 %206
%208 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %205 %207
%209 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%210 = OpLoad %v4float %209
%211 = OpCompositeExtract %float %210 1
%212 = OpVectorTimesScalar %v4float %208 %211
%213 = OpFAdd %v4float %203 %212
OpStore %_6_output %213
%214 = OpLoad %v2float %_7_coord
%215 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%216 = OpLoad %v2float %215
%217 = OpFAdd %v2float %214 %216
OpStore %_7_coord %217
%218 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %218
%219 = OpLoad %v4float %_6_output
%220 = OpLoad %v4float %outputColor_Stage0
OpStore %221 %220
%222 = OpLoad %v2float %_8_coordSampled
OpStore %223 %222
%224 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %221 %223
%225 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%226 = OpLoad %v4float %225
%227 = OpCompositeExtract %float %226 2
%228 = OpVectorTimesScalar %v4float %224 %227
%229 = OpFAdd %v4float %219 %228
OpStore %_6_output %229
%230 = OpLoad %v2float %_7_coord
%231 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%232 = OpLoad %v2float %231
%233 = OpFAdd %v2float %230 %232
OpStore %_7_coord %233
%234 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %234
%235 = OpLoad %v4float %_6_output
%236 = OpLoad %v4float %outputColor_Stage0
OpStore %237 %236
%238 = OpLoad %v2float %_8_coordSampled
OpStore %239 %238
%240 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %237 %239
%241 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%242 = OpLoad %v4float %241
%243 = OpCompositeExtract %float %242 3
%244 = OpVectorTimesScalar %v4float %240 %243
%245 = OpFAdd %v4float %235 %244
OpStore %_6_output %245
%246 = OpLoad %v2float %_7_coord
%247 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%248 = OpLoad %v2float %247
%249 = OpFAdd %v2float %246 %248
OpStore %_7_coord %249
%250 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %250
%251 = OpLoad %v4float %_6_output
%252 = OpLoad %v4float %outputColor_Stage0
OpStore %253 %252
%254 = OpLoad %v2float %_8_coordSampled
OpStore %255 %254
%256 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %253 %255
%257 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%258 = OpLoad %v4float %257
%259 = OpCompositeExtract %float %258 0
%260 = OpVectorTimesScalar %v4float %256 %259
%261 = OpFAdd %v4float %251 %260
OpStore %_6_output %261
%262 = OpLoad %v2float %_7_coord
%263 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%264 = OpLoad %v2float %263
%265 = OpFAdd %v2float %262 %264
OpStore %_7_coord %265
%266 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %266
%267 = OpLoad %v4float %_6_output
%268 = OpLoad %v4float %outputColor_Stage0
OpStore %269 %268
%270 = OpLoad %v2float %_8_coordSampled
OpStore %271 %270
%272 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %269 %271
%273 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%274 = OpLoad %v4float %273
%275 = OpCompositeExtract %float %274 1
%276 = OpVectorTimesScalar %v4float %272 %275
%277 = OpFAdd %v4float %267 %276
OpStore %_6_output %277
%278 = OpLoad %v2float %_7_coord
%279 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%280 = OpLoad %v2float %279
%281 = OpFAdd %v2float %278 %280
OpStore %_7_coord %281
%282 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %282
%283 = OpLoad %v4float %_6_output
%284 = OpLoad %v4float %outputColor_Stage0
OpStore %285 %284
%286 = OpLoad %v2float %_8_coordSampled
OpStore %287 %286
%288 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %285 %287
%289 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%290 = OpLoad %v4float %289
%291 = OpCompositeExtract %float %290 2
%292 = OpVectorTimesScalar %v4float %288 %291
%293 = OpFAdd %v4float %283 %292
OpStore %_6_output %293
%294 = OpLoad %v2float %_7_coord
%295 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%296 = OpLoad %v2float %295
%297 = OpFAdd %v2float %294 %296
OpStore %_7_coord %297
%298 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %298
%299 = OpLoad %v4float %_6_output
%300 = OpLoad %v4float %outputColor_Stage0
OpStore %301 %300
%302 = OpLoad %v2float %_8_coordSampled
OpStore %303 %302
%304 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %301 %303
%305 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%306 = OpLoad %v4float %305
%307 = OpCompositeExtract %float %306 3
%308 = OpVectorTimesScalar %v4float %304 %307
%309 = OpFAdd %v4float %299 %308
OpStore %_6_output %309
%310 = OpLoad %v2float %_7_coord
%311 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%312 = OpLoad %v2float %311
%313 = OpFAdd %v2float %310 %312
OpStore %_7_coord %313
%314 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %314
%315 = OpLoad %v4float %_6_output
%316 = OpLoad %v4float %outputColor_Stage0
OpStore %317 %316
%318 = OpLoad %v2float %_8_coordSampled
OpStore %319 %318
%320 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %317 %319
%321 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%322 = OpLoad %v4float %321
%323 = OpCompositeExtract %float %322 0
%324 = OpVectorTimesScalar %v4float %320 %323
%325 = OpFAdd %v4float %315 %324
OpStore %_6_output %325
%326 = OpLoad %v2float %_7_coord
%327 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%328 = OpLoad %v2float %327
%329 = OpFAdd %v2float %326 %328
OpStore %_7_coord %329
%330 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %330
%331 = OpLoad %v4float %_6_output
%332 = OpLoad %v4float %outputColor_Stage0
OpStore %333 %332
%334 = OpLoad %v2float %_8_coordSampled
OpStore %335 %334
%336 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %333 %335
%337 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%338 = OpLoad %v4float %337
%339 = OpCompositeExtract %float %338 1
%340 = OpVectorTimesScalar %v4float %336 %339
%341 = OpFAdd %v4float %331 %340
OpStore %_6_output %341
%342 = OpLoad %v2float %_7_coord
%343 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%344 = OpLoad %v2float %343
%345 = OpFAdd %v2float %342 %344
OpStore %_7_coord %345
%346 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %346
%347 = OpLoad %v4float %_6_output
%348 = OpLoad %v4float %outputColor_Stage0
OpStore %349 %348
%350 = OpLoad %v2float %_8_coordSampled
OpStore %351 %350
%352 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %349 %351
%353 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%354 = OpLoad %v4float %353
%355 = OpCompositeExtract %float %354 2
%356 = OpVectorTimesScalar %v4float %352 %355
%357 = OpFAdd %v4float %347 %356
OpStore %_6_output %357
%358 = OpLoad %v2float %_7_coord
%359 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%360 = OpLoad %v2float %359
%361 = OpFAdd %v2float %358 %360
OpStore %_7_coord %361
%362 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %362
%363 = OpLoad %v4float %_6_output
%364 = OpLoad %v4float %outputColor_Stage0
OpStore %365 %364
%366 = OpLoad %v2float %_8_coordSampled
OpStore %367 %366
%368 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %365 %367
%369 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%370 = OpLoad %v4float %369
%371 = OpCompositeExtract %float %370 3
%372 = OpVectorTimesScalar %v4float %368 %371
%373 = OpFAdd %v4float %363 %372
OpStore %_6_output %373
%374 = OpLoad %v2float %_7_coord
%375 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%376 = OpLoad %v2float %375
%377 = OpFAdd %v2float %374 %376
OpStore %_7_coord %377
%378 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %378
%379 = OpLoad %v4float %_6_output
%380 = OpLoad %v4float %outputColor_Stage0
OpStore %381 %380
%382 = OpLoad %v2float %_8_coordSampled
OpStore %383 %382
%384 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %381 %383
%385 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%386 = OpLoad %v4float %385
%387 = OpCompositeExtract %float %386 0
%388 = OpVectorTimesScalar %v4float %384 %387
%389 = OpFAdd %v4float %379 %388
OpStore %_6_output %389
%390 = OpLoad %v2float %_7_coord
%391 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%392 = OpLoad %v2float %391
%393 = OpFAdd %v2float %390 %392
OpStore %_7_coord %393
%394 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %394
%395 = OpLoad %v4float %_6_output
%396 = OpLoad %v4float %outputColor_Stage0
OpStore %397 %396
%398 = OpLoad %v2float %_8_coordSampled
OpStore %399 %398
%400 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %397 %399
%401 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%402 = OpLoad %v4float %401
%403 = OpCompositeExtract %float %402 1
%404 = OpVectorTimesScalar %v4float %400 %403
%405 = OpFAdd %v4float %395 %404
OpStore %_6_output %405
%406 = OpLoad %v2float %_7_coord
%407 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%408 = OpLoad %v2float %407
%409 = OpFAdd %v2float %406 %408
OpStore %_7_coord %409
%410 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %410
%411 = OpLoad %v4float %_6_output
%412 = OpLoad %v4float %outputColor_Stage0
OpStore %413 %412
%414 = OpLoad %v2float %_8_coordSampled
OpStore %415 %414
%416 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %413 %415
%417 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%418 = OpLoad %v4float %417
%419 = OpCompositeExtract %float %418 2
%420 = OpVectorTimesScalar %v4float %416 %419
%421 = OpFAdd %v4float %411 %420
OpStore %_6_output %421
%422 = OpLoad %v2float %_7_coord
%423 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%424 = OpLoad %v2float %423
%425 = OpFAdd %v2float %422 %424
OpStore %_7_coord %425
%426 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %426
%427 = OpLoad %v4float %_6_output
%428 = OpLoad %v4float %outputColor_Stage0
OpStore %429 %428
%430 = OpLoad %v2float %_8_coordSampled
OpStore %431 %430
%432 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %429 %431
%433 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%434 = OpLoad %v4float %433
%435 = OpCompositeExtract %float %434 3
%436 = OpVectorTimesScalar %v4float %432 %435
%437 = OpFAdd %v4float %427 %436
OpStore %_6_output %437
%438 = OpLoad %v2float %_7_coord
%439 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%440 = OpLoad %v2float %439
%441 = OpFAdd %v2float %438 %440
OpStore %_7_coord %441
%442 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %442
%443 = OpLoad %v4float %_6_output
%444 = OpLoad %v4float %outputColor_Stage0
OpStore %445 %444
%446 = OpLoad %v2float %_8_coordSampled
OpStore %447 %446
%448 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %445 %447
%449 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%450 = OpLoad %v4float %449
%451 = OpCompositeExtract %float %450 0
%452 = OpVectorTimesScalar %v4float %448 %451
%453 = OpFAdd %v4float %443 %452
OpStore %_6_output %453
%454 = OpLoad %v2float %_7_coord
%455 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%456 = OpLoad %v2float %455
%457 = OpFAdd %v2float %454 %456
OpStore %_7_coord %457
%458 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %458
%459 = OpLoad %v4float %_6_output
%460 = OpLoad %v4float %outputColor_Stage0
OpStore %461 %460
%462 = OpLoad %v2float %_8_coordSampled
OpStore %463 %462
%464 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %461 %463
%465 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%466 = OpLoad %v4float %465
%467 = OpCompositeExtract %float %466 1
%468 = OpVectorTimesScalar %v4float %464 %467
%469 = OpFAdd %v4float %459 %468
OpStore %_6_output %469
%470 = OpLoad %v2float %_7_coord
%471 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%472 = OpLoad %v2float %471
%473 = OpFAdd %v2float %470 %472
OpStore %_7_coord %473
%474 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %474
%475 = OpLoad %v4float %_6_output
%476 = OpLoad %v4float %outputColor_Stage0
OpStore %477 %476
%478 = OpLoad %v2float %_8_coordSampled
OpStore %479 %478
%480 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %477 %479
%481 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%482 = OpLoad %v4float %481
%483 = OpCompositeExtract %float %482 2
%484 = OpVectorTimesScalar %v4float %480 %483
%485 = OpFAdd %v4float %475 %484
OpStore %_6_output %485
%486 = OpLoad %v2float %_7_coord
%487 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%488 = OpLoad %v2float %487
%489 = OpFAdd %v2float %486 %488
OpStore %_7_coord %489
%490 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %490
%491 = OpLoad %v4float %_6_output
%492 = OpLoad %v4float %outputColor_Stage0
OpStore %493 %492
%494 = OpLoad %v2float %_8_coordSampled
OpStore %495 %494
%496 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %493 %495
%497 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%498 = OpLoad %v4float %497
%499 = OpCompositeExtract %float %498 3
%500 = OpVectorTimesScalar %v4float %496 %499
%501 = OpFAdd %v4float %491 %500
OpStore %_6_output %501
%502 = OpLoad %v2float %_7_coord
%503 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%504 = OpLoad %v2float %503
%505 = OpFAdd %v2float %502 %504
OpStore %_7_coord %505
%506 = OpLoad %v2float %_7_coord
OpStore %_8_coordSampled %506
%507 = OpLoad %v4float %_6_output
%508 = OpLoad %v4float %outputColor_Stage0
OpStore %509 %508
%510 = OpLoad %v2float %_8_coordSampled
OpStore %511 %510
%512 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0_h4h4f2 %509 %511
%513 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%514 = OpLoad %v4float %513
%515 = OpCompositeExtract %float %514 0
%516 = OpVectorTimesScalar %v4float %512 %515
%517 = OpFAdd %v4float %507 %516
OpStore %_6_output %517
%518 = OpLoad %v2float %_7_coord
%519 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%520 = OpLoad %v2float %519
%521 = OpFAdd %v2float %518 %520
OpStore %_7_coord %521
%522 = OpLoad %v4float %_6_output
%523 = OpLoad %v4float %outputColor_Stage0
%524 = OpFMul %v4float %522 %523
OpStore %_6_output %524
%525 = OpLoad %v4float %_6_output
OpStore %output_Stage1 %525
%526 = OpLoad %v4float %output_Stage1
%527 = OpLoad %v4float %outputCoverage_Stage0
%528 = OpFMul %v4float %526 %527
OpStore %sk_FragColor %528
OpReturn
OpFunctionEnd
