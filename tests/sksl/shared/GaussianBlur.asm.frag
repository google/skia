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
OpName %MatrixEffect_Stage1_c0_c0 "MatrixEffect_Stage1_c0_c0"
OpName %_output "_output"
OpName %_0_TextureEffect_Stage1_c0_c0_c0 "_0_TextureEffect_Stage1_c0_c0_c0"
OpName %_1_coords "_1_coords"
OpName %_2_output "_2_output"
OpName %_3_inCoord "_3_inCoord"
OpName %_4_subsetCoord "_4_subsetCoord"
OpName %_5_clampedCoord "_5_clampedCoord"
OpName %_6_textureColor "_6_textureColor"
OpName %_7_snappedX "_7_snappedX"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpName %_8_GaussianConvolution_Stage1_c0 "_8_GaussianConvolution_Stage1_c0"
OpName %_9_output "_9_output"
OpName %_10_coord "_10_coord"
OpName %_11_coordSampled "_11_coordSampled"
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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %70 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %503 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %522 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
OpDecorate %532 RelaxedPrecision
OpDecorate %533 RelaxedPrecision
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
%106 = OpTypeFunction %void
%110 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%115 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%125 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_output = OpVariable %_ptr_Function_v4float Function
%_0_TextureEffect_Stage1_c0_c0_c0 = OpVariable %_ptr_Function_v4float Function
%_1_coords = OpVariable %_ptr_Function_v2float Function
%_2_output = OpVariable %_ptr_Function_v4float Function
%_3_inCoord = OpVariable %_ptr_Function_v2float Function
%_4_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_5_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_6_textureColor = OpVariable %_ptr_Function_v4float Function
%_7_snappedX = OpVariable %_ptr_Function_float Function
%36 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%38 = OpLoad %mat3v3float %36
%39 = OpLoad %v2float %30
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%43 = OpCompositeConstruct %v3float %40 %41 %float_1
%44 = OpMatrixTimesVector %v3float %38 %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
OpStore %_1_coords %45
%48 = OpLoad %v2float %_1_coords
OpStore %_3_inCoord %48
%49 = OpLoad %v2float %_3_inCoord
%51 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%53 = OpLoad %v4float %51
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFMul %v2float %49 %54
OpStore %_3_inCoord %55
%57 = OpLoad %v2float %_3_inCoord
%58 = OpCompositeExtract %float %57 0
%59 = OpAccessChain %_ptr_Function_float %_4_subsetCoord %int_0
OpStore %59 %58
%62 = OpLoad %v2float %_3_inCoord
%63 = OpCompositeExtract %float %62 1
%64 = OpAccessChain %_ptr_Function_float %_4_subsetCoord %int_1
OpStore %64 %63
%67 = OpLoad %v2float %_4_subsetCoord
OpStore %_5_clampedCoord %67
%70 = OpLoad %22 %uTextureSampler_0_Stage1
%71 = OpLoad %v2float %_5_clampedCoord
%72 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%73 = OpLoad %v4float %72
%74 = OpVectorShuffle %v2float %73 %73 2 3
%75 = OpFMul %v2float %71 %74
%69 = OpImageSampleImplicitLod %v4float %70 %75
OpStore %_6_textureColor %69
%78 = OpLoad %v2float %_3_inCoord
%79 = OpCompositeExtract %float %78 0
%81 = OpFAdd %float %79 %float_0_00100000005
%77 = OpExtInst %float %1 Floor %81
%83 = OpFAdd %float %77 %float_0_5
OpStore %_7_snappedX %83
%85 = OpLoad %float %_7_snappedX
%87 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%88 = OpLoad %v4float %87
%89 = OpCompositeExtract %float %88 0
%90 = OpFOrdLessThan %bool %85 %89
OpSelectionMerge %92 None
OpBranchConditional %90 %92 %91
%91 = OpLabel
%93 = OpLoad %float %_7_snappedX
%94 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%95 = OpLoad %v4float %94
%96 = OpCompositeExtract %float %95 2
%97 = OpFOrdGreaterThan %bool %93 %96
OpBranch %92
%92 = OpLabel
%98 = OpPhi %bool %true %31 %97 %91
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%103 = OpLoad %v4float %102
OpStore %_6_textureColor %103
OpBranch %100
%100 = OpLabel
%104 = OpLoad %v4float %_6_textureColor
OpReturnValue %104
OpFunctionEnd
%main = OpFunction %void None %106
%107 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_8_GaussianConvolution_Stage1_c0 = OpVariable %_ptr_Function_v4float Function
%_9_output = OpVariable %_ptr_Function_v4float Function
%_10_coord = OpVariable %_ptr_Function_v2float Function
%_11_coordSampled = OpVariable %_ptr_Function_v2float Function
%129 = OpVariable %_ptr_Function_v4float Function
%131 = OpVariable %_ptr_Function_v2float Function
%146 = OpVariable %_ptr_Function_v4float Function
%148 = OpVariable %_ptr_Function_v2float Function
%162 = OpVariable %_ptr_Function_v4float Function
%164 = OpVariable %_ptr_Function_v2float Function
%178 = OpVariable %_ptr_Function_v4float Function
%180 = OpVariable %_ptr_Function_v2float Function
%194 = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v4float Function
%212 = OpVariable %_ptr_Function_v2float Function
%226 = OpVariable %_ptr_Function_v4float Function
%228 = OpVariable %_ptr_Function_v2float Function
%242 = OpVariable %_ptr_Function_v4float Function
%244 = OpVariable %_ptr_Function_v2float Function
%258 = OpVariable %_ptr_Function_v4float Function
%260 = OpVariable %_ptr_Function_v2float Function
%274 = OpVariable %_ptr_Function_v4float Function
%276 = OpVariable %_ptr_Function_v2float Function
%290 = OpVariable %_ptr_Function_v4float Function
%292 = OpVariable %_ptr_Function_v2float Function
%306 = OpVariable %_ptr_Function_v4float Function
%308 = OpVariable %_ptr_Function_v2float Function
%322 = OpVariable %_ptr_Function_v4float Function
%324 = OpVariable %_ptr_Function_v2float Function
%338 = OpVariable %_ptr_Function_v4float Function
%340 = OpVariable %_ptr_Function_v2float Function
%354 = OpVariable %_ptr_Function_v4float Function
%356 = OpVariable %_ptr_Function_v2float Function
%370 = OpVariable %_ptr_Function_v4float Function
%372 = OpVariable %_ptr_Function_v2float Function
%386 = OpVariable %_ptr_Function_v4float Function
%388 = OpVariable %_ptr_Function_v2float Function
%402 = OpVariable %_ptr_Function_v4float Function
%404 = OpVariable %_ptr_Function_v2float Function
%418 = OpVariable %_ptr_Function_v4float Function
%420 = OpVariable %_ptr_Function_v2float Function
%434 = OpVariable %_ptr_Function_v4float Function
%436 = OpVariable %_ptr_Function_v2float Function
%450 = OpVariable %_ptr_Function_v4float Function
%452 = OpVariable %_ptr_Function_v2float Function
%466 = OpVariable %_ptr_Function_v4float Function
%468 = OpVariable %_ptr_Function_v2float Function
%482 = OpVariable %_ptr_Function_v4float Function
%484 = OpVariable %_ptr_Function_v2float Function
%498 = OpVariable %_ptr_Function_v4float Function
%500 = OpVariable %_ptr_Function_v2float Function
%514 = OpVariable %_ptr_Function_v4float Function
%516 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %110
OpStore %outputCoverage_Stage0 %110
OpStore %_9_output %115
%117 = OpLoad %v2float %vLocalCoord_Stage0
%119 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%121 = OpLoad %v2float %119
%122 = OpVectorTimesScalar %v2float %121 %float_12
%123 = OpFSub %v2float %117 %122
OpStore %_10_coord %123
OpStore %_11_coordSampled %125
%126 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %126
%127 = OpLoad %v4float %_9_output
%128 = OpLoad %v4float %outputColor_Stage0
OpStore %129 %128
%130 = OpLoad %v2float %_11_coordSampled
OpStore %131 %130
%132 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %129 %131
%134 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%135 = OpLoad %v4float %134
%136 = OpCompositeExtract %float %135 0
%137 = OpVectorTimesScalar %v4float %132 %136
%138 = OpFAdd %v4float %127 %137
OpStore %_9_output %138
%139 = OpLoad %v2float %_10_coord
%140 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%141 = OpLoad %v2float %140
%142 = OpFAdd %v2float %139 %141
OpStore %_10_coord %142
%143 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %143
%144 = OpLoad %v4float %_9_output
%145 = OpLoad %v4float %outputColor_Stage0
OpStore %146 %145
%147 = OpLoad %v2float %_11_coordSampled
OpStore %148 %147
%149 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %146 %148
%150 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 1
%153 = OpVectorTimesScalar %v4float %149 %152
%154 = OpFAdd %v4float %144 %153
OpStore %_9_output %154
%155 = OpLoad %v2float %_10_coord
%156 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%157 = OpLoad %v2float %156
%158 = OpFAdd %v2float %155 %157
OpStore %_10_coord %158
%159 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %159
%160 = OpLoad %v4float %_9_output
%161 = OpLoad %v4float %outputColor_Stage0
OpStore %162 %161
%163 = OpLoad %v2float %_11_coordSampled
OpStore %164 %163
%165 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %162 %164
%166 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 2
%169 = OpVectorTimesScalar %v4float %165 %168
%170 = OpFAdd %v4float %160 %169
OpStore %_9_output %170
%171 = OpLoad %v2float %_10_coord
%172 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%173 = OpLoad %v2float %172
%174 = OpFAdd %v2float %171 %173
OpStore %_10_coord %174
%175 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %175
%176 = OpLoad %v4float %_9_output
%177 = OpLoad %v4float %outputColor_Stage0
OpStore %178 %177
%179 = OpLoad %v2float %_11_coordSampled
OpStore %180 %179
%181 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %178 %180
%182 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%183 = OpLoad %v4float %182
%184 = OpCompositeExtract %float %183 3
%185 = OpVectorTimesScalar %v4float %181 %184
%186 = OpFAdd %v4float %176 %185
OpStore %_9_output %186
%187 = OpLoad %v2float %_10_coord
%188 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%189 = OpLoad %v2float %188
%190 = OpFAdd %v2float %187 %189
OpStore %_10_coord %190
%191 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %191
%192 = OpLoad %v4float %_9_output
%193 = OpLoad %v4float %outputColor_Stage0
OpStore %194 %193
%195 = OpLoad %v2float %_11_coordSampled
OpStore %196 %195
%197 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %194 %196
%198 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%199 = OpLoad %v4float %198
%200 = OpCompositeExtract %float %199 0
%201 = OpVectorTimesScalar %v4float %197 %200
%202 = OpFAdd %v4float %192 %201
OpStore %_9_output %202
%203 = OpLoad %v2float %_10_coord
%204 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%205 = OpLoad %v2float %204
%206 = OpFAdd %v2float %203 %205
OpStore %_10_coord %206
%207 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %207
%208 = OpLoad %v4float %_9_output
%209 = OpLoad %v4float %outputColor_Stage0
OpStore %210 %209
%211 = OpLoad %v2float %_11_coordSampled
OpStore %212 %211
%213 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %210 %212
%214 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%215 = OpLoad %v4float %214
%216 = OpCompositeExtract %float %215 1
%217 = OpVectorTimesScalar %v4float %213 %216
%218 = OpFAdd %v4float %208 %217
OpStore %_9_output %218
%219 = OpLoad %v2float %_10_coord
%220 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%221 = OpLoad %v2float %220
%222 = OpFAdd %v2float %219 %221
OpStore %_10_coord %222
%223 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %223
%224 = OpLoad %v4float %_9_output
%225 = OpLoad %v4float %outputColor_Stage0
OpStore %226 %225
%227 = OpLoad %v2float %_11_coordSampled
OpStore %228 %227
%229 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %226 %228
%230 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%231 = OpLoad %v4float %230
%232 = OpCompositeExtract %float %231 2
%233 = OpVectorTimesScalar %v4float %229 %232
%234 = OpFAdd %v4float %224 %233
OpStore %_9_output %234
%235 = OpLoad %v2float %_10_coord
%236 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%237 = OpLoad %v2float %236
%238 = OpFAdd %v2float %235 %237
OpStore %_10_coord %238
%239 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %239
%240 = OpLoad %v4float %_9_output
%241 = OpLoad %v4float %outputColor_Stage0
OpStore %242 %241
%243 = OpLoad %v2float %_11_coordSampled
OpStore %244 %243
%245 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %242 %244
%246 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 3
%249 = OpVectorTimesScalar %v4float %245 %248
%250 = OpFAdd %v4float %240 %249
OpStore %_9_output %250
%251 = OpLoad %v2float %_10_coord
%252 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%253 = OpLoad %v2float %252
%254 = OpFAdd %v2float %251 %253
OpStore %_10_coord %254
%255 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %255
%256 = OpLoad %v4float %_9_output
%257 = OpLoad %v4float %outputColor_Stage0
OpStore %258 %257
%259 = OpLoad %v2float %_11_coordSampled
OpStore %260 %259
%261 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %258 %260
%262 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%263 = OpLoad %v4float %262
%264 = OpCompositeExtract %float %263 0
%265 = OpVectorTimesScalar %v4float %261 %264
%266 = OpFAdd %v4float %256 %265
OpStore %_9_output %266
%267 = OpLoad %v2float %_10_coord
%268 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%269 = OpLoad %v2float %268
%270 = OpFAdd %v2float %267 %269
OpStore %_10_coord %270
%271 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %271
%272 = OpLoad %v4float %_9_output
%273 = OpLoad %v4float %outputColor_Stage0
OpStore %274 %273
%275 = OpLoad %v2float %_11_coordSampled
OpStore %276 %275
%277 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %274 %276
%278 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%279 = OpLoad %v4float %278
%280 = OpCompositeExtract %float %279 1
%281 = OpVectorTimesScalar %v4float %277 %280
%282 = OpFAdd %v4float %272 %281
OpStore %_9_output %282
%283 = OpLoad %v2float %_10_coord
%284 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%285 = OpLoad %v2float %284
%286 = OpFAdd %v2float %283 %285
OpStore %_10_coord %286
%287 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %287
%288 = OpLoad %v4float %_9_output
%289 = OpLoad %v4float %outputColor_Stage0
OpStore %290 %289
%291 = OpLoad %v2float %_11_coordSampled
OpStore %292 %291
%293 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %290 %292
%294 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%295 = OpLoad %v4float %294
%296 = OpCompositeExtract %float %295 2
%297 = OpVectorTimesScalar %v4float %293 %296
%298 = OpFAdd %v4float %288 %297
OpStore %_9_output %298
%299 = OpLoad %v2float %_10_coord
%300 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%301 = OpLoad %v2float %300
%302 = OpFAdd %v2float %299 %301
OpStore %_10_coord %302
%303 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %303
%304 = OpLoad %v4float %_9_output
%305 = OpLoad %v4float %outputColor_Stage0
OpStore %306 %305
%307 = OpLoad %v2float %_11_coordSampled
OpStore %308 %307
%309 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %306 %308
%310 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%311 = OpLoad %v4float %310
%312 = OpCompositeExtract %float %311 3
%313 = OpVectorTimesScalar %v4float %309 %312
%314 = OpFAdd %v4float %304 %313
OpStore %_9_output %314
%315 = OpLoad %v2float %_10_coord
%316 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%317 = OpLoad %v2float %316
%318 = OpFAdd %v2float %315 %317
OpStore %_10_coord %318
%319 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %319
%320 = OpLoad %v4float %_9_output
%321 = OpLoad %v4float %outputColor_Stage0
OpStore %322 %321
%323 = OpLoad %v2float %_11_coordSampled
OpStore %324 %323
%325 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %322 %324
%326 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%327 = OpLoad %v4float %326
%328 = OpCompositeExtract %float %327 0
%329 = OpVectorTimesScalar %v4float %325 %328
%330 = OpFAdd %v4float %320 %329
OpStore %_9_output %330
%331 = OpLoad %v2float %_10_coord
%332 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%333 = OpLoad %v2float %332
%334 = OpFAdd %v2float %331 %333
OpStore %_10_coord %334
%335 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %335
%336 = OpLoad %v4float %_9_output
%337 = OpLoad %v4float %outputColor_Stage0
OpStore %338 %337
%339 = OpLoad %v2float %_11_coordSampled
OpStore %340 %339
%341 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %338 %340
%342 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%343 = OpLoad %v4float %342
%344 = OpCompositeExtract %float %343 1
%345 = OpVectorTimesScalar %v4float %341 %344
%346 = OpFAdd %v4float %336 %345
OpStore %_9_output %346
%347 = OpLoad %v2float %_10_coord
%348 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%349 = OpLoad %v2float %348
%350 = OpFAdd %v2float %347 %349
OpStore %_10_coord %350
%351 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %351
%352 = OpLoad %v4float %_9_output
%353 = OpLoad %v4float %outputColor_Stage0
OpStore %354 %353
%355 = OpLoad %v2float %_11_coordSampled
OpStore %356 %355
%357 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %354 %356
%358 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%359 = OpLoad %v4float %358
%360 = OpCompositeExtract %float %359 2
%361 = OpVectorTimesScalar %v4float %357 %360
%362 = OpFAdd %v4float %352 %361
OpStore %_9_output %362
%363 = OpLoad %v2float %_10_coord
%364 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%365 = OpLoad %v2float %364
%366 = OpFAdd %v2float %363 %365
OpStore %_10_coord %366
%367 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %367
%368 = OpLoad %v4float %_9_output
%369 = OpLoad %v4float %outputColor_Stage0
OpStore %370 %369
%371 = OpLoad %v2float %_11_coordSampled
OpStore %372 %371
%373 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %370 %372
%374 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%375 = OpLoad %v4float %374
%376 = OpCompositeExtract %float %375 3
%377 = OpVectorTimesScalar %v4float %373 %376
%378 = OpFAdd %v4float %368 %377
OpStore %_9_output %378
%379 = OpLoad %v2float %_10_coord
%380 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%381 = OpLoad %v2float %380
%382 = OpFAdd %v2float %379 %381
OpStore %_10_coord %382
%383 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %383
%384 = OpLoad %v4float %_9_output
%385 = OpLoad %v4float %outputColor_Stage0
OpStore %386 %385
%387 = OpLoad %v2float %_11_coordSampled
OpStore %388 %387
%389 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %386 %388
%390 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%391 = OpLoad %v4float %390
%392 = OpCompositeExtract %float %391 0
%393 = OpVectorTimesScalar %v4float %389 %392
%394 = OpFAdd %v4float %384 %393
OpStore %_9_output %394
%395 = OpLoad %v2float %_10_coord
%396 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%397 = OpLoad %v2float %396
%398 = OpFAdd %v2float %395 %397
OpStore %_10_coord %398
%399 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %399
%400 = OpLoad %v4float %_9_output
%401 = OpLoad %v4float %outputColor_Stage0
OpStore %402 %401
%403 = OpLoad %v2float %_11_coordSampled
OpStore %404 %403
%405 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %402 %404
%406 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%407 = OpLoad %v4float %406
%408 = OpCompositeExtract %float %407 1
%409 = OpVectorTimesScalar %v4float %405 %408
%410 = OpFAdd %v4float %400 %409
OpStore %_9_output %410
%411 = OpLoad %v2float %_10_coord
%412 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%413 = OpLoad %v2float %412
%414 = OpFAdd %v2float %411 %413
OpStore %_10_coord %414
%415 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %415
%416 = OpLoad %v4float %_9_output
%417 = OpLoad %v4float %outputColor_Stage0
OpStore %418 %417
%419 = OpLoad %v2float %_11_coordSampled
OpStore %420 %419
%421 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %418 %420
%422 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%423 = OpLoad %v4float %422
%424 = OpCompositeExtract %float %423 2
%425 = OpVectorTimesScalar %v4float %421 %424
%426 = OpFAdd %v4float %416 %425
OpStore %_9_output %426
%427 = OpLoad %v2float %_10_coord
%428 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%429 = OpLoad %v2float %428
%430 = OpFAdd %v2float %427 %429
OpStore %_10_coord %430
%431 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %431
%432 = OpLoad %v4float %_9_output
%433 = OpLoad %v4float %outputColor_Stage0
OpStore %434 %433
%435 = OpLoad %v2float %_11_coordSampled
OpStore %436 %435
%437 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %434 %436
%438 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%439 = OpLoad %v4float %438
%440 = OpCompositeExtract %float %439 3
%441 = OpVectorTimesScalar %v4float %437 %440
%442 = OpFAdd %v4float %432 %441
OpStore %_9_output %442
%443 = OpLoad %v2float %_10_coord
%444 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%445 = OpLoad %v2float %444
%446 = OpFAdd %v2float %443 %445
OpStore %_10_coord %446
%447 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %447
%448 = OpLoad %v4float %_9_output
%449 = OpLoad %v4float %outputColor_Stage0
OpStore %450 %449
%451 = OpLoad %v2float %_11_coordSampled
OpStore %452 %451
%453 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %450 %452
%454 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%455 = OpLoad %v4float %454
%456 = OpCompositeExtract %float %455 0
%457 = OpVectorTimesScalar %v4float %453 %456
%458 = OpFAdd %v4float %448 %457
OpStore %_9_output %458
%459 = OpLoad %v2float %_10_coord
%460 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%461 = OpLoad %v2float %460
%462 = OpFAdd %v2float %459 %461
OpStore %_10_coord %462
%463 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %463
%464 = OpLoad %v4float %_9_output
%465 = OpLoad %v4float %outputColor_Stage0
OpStore %466 %465
%467 = OpLoad %v2float %_11_coordSampled
OpStore %468 %467
%469 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %466 %468
%470 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%471 = OpLoad %v4float %470
%472 = OpCompositeExtract %float %471 1
%473 = OpVectorTimesScalar %v4float %469 %472
%474 = OpFAdd %v4float %464 %473
OpStore %_9_output %474
%475 = OpLoad %v2float %_10_coord
%476 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%477 = OpLoad %v2float %476
%478 = OpFAdd %v2float %475 %477
OpStore %_10_coord %478
%479 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %479
%480 = OpLoad %v4float %_9_output
%481 = OpLoad %v4float %outputColor_Stage0
OpStore %482 %481
%483 = OpLoad %v2float %_11_coordSampled
OpStore %484 %483
%485 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %482 %484
%486 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%487 = OpLoad %v4float %486
%488 = OpCompositeExtract %float %487 2
%489 = OpVectorTimesScalar %v4float %485 %488
%490 = OpFAdd %v4float %480 %489
OpStore %_9_output %490
%491 = OpLoad %v2float %_10_coord
%492 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%493 = OpLoad %v2float %492
%494 = OpFAdd %v2float %491 %493
OpStore %_10_coord %494
%495 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %495
%496 = OpLoad %v4float %_9_output
%497 = OpLoad %v4float %outputColor_Stage0
OpStore %498 %497
%499 = OpLoad %v2float %_11_coordSampled
OpStore %500 %499
%501 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %498 %500
%502 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%503 = OpLoad %v4float %502
%504 = OpCompositeExtract %float %503 3
%505 = OpVectorTimesScalar %v4float %501 %504
%506 = OpFAdd %v4float %496 %505
OpStore %_9_output %506
%507 = OpLoad %v2float %_10_coord
%508 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%509 = OpLoad %v2float %508
%510 = OpFAdd %v2float %507 %509
OpStore %_10_coord %510
%511 = OpLoad %v2float %_10_coord
OpStore %_11_coordSampled %511
%512 = OpLoad %v4float %_9_output
%513 = OpLoad %v4float %outputColor_Stage0
OpStore %514 %513
%515 = OpLoad %v2float %_11_coordSampled
OpStore %516 %515
%517 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %514 %516
%518 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%519 = OpLoad %v4float %518
%520 = OpCompositeExtract %float %519 0
%521 = OpVectorTimesScalar %v4float %517 %520
%522 = OpFAdd %v4float %512 %521
OpStore %_9_output %522
%523 = OpLoad %v2float %_10_coord
%524 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%525 = OpLoad %v2float %524
%526 = OpFAdd %v2float %523 %525
OpStore %_10_coord %526
%527 = OpLoad %v4float %_9_output
%528 = OpLoad %v4float %outputColor_Stage0
%529 = OpFMul %v4float %527 %528
OpStore %_9_output %529
%530 = OpLoad %v4float %_9_output
OpStore %output_Stage1 %530
%531 = OpLoad %v4float %output_Stage1
%532 = OpLoad %v4float %outputCoverage_Stage0
%533 = OpFMul %v4float %531 %532
OpStore %sk_FragColor %533
OpReturn
OpFunctionEnd
