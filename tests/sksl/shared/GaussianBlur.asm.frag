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
OpName %_0_coords "_0_coords"
OpName %_1_output "_1_output"
OpName %_2_inCoord "_2_inCoord"
OpName %_3_subsetCoord "_3_subsetCoord"
OpName %_4_clampedCoord "_4_clampedCoord"
OpName %_5_textureColor "_5_textureColor"
OpName %_6_snappedX "_6_snappedX"
OpName %main "main"
OpName %outputColor_Stage0 "outputColor_Stage0"
OpName %outputCoverage_Stage0 "outputCoverage_Stage0"
OpName %output_Stage1 "output_Stage1"
OpName %_7_output "_7_output"
OpName %_8_coord "_8_coord"
OpName %_9_coordSampled "_9_coordSampled"
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
OpDecorate %69 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
OpDecorate %485 RelaxedPrecision
OpDecorate %488 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
OpDecorate %501 RelaxedPrecision
OpDecorate %504 RelaxedPrecision
OpDecorate %507 RelaxedPrecision
OpDecorate %510 RelaxedPrecision
OpDecorate %511 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %531 RelaxedPrecision
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
%105 = OpTypeFunction %void
%109 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%113 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%123 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_output = OpVariable %_ptr_Function_v4float Function
%_0_coords = OpVariable %_ptr_Function_v2float Function
%_1_output = OpVariable %_ptr_Function_v4float Function
%_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_6_snappedX = OpVariable %_ptr_Function_float Function
%35 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%37 = OpLoad %mat3v3float %35
%38 = OpLoad %v2float %30
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%42 = OpCompositeConstruct %v3float %39 %40 %float_1
%43 = OpMatrixTimesVector %v3float %37 %42
%44 = OpVectorShuffle %v2float %43 %43 0 1
OpStore %_0_coords %44
%47 = OpLoad %v2float %_0_coords
OpStore %_2_inCoord %47
%48 = OpLoad %v2float %_2_inCoord
%50 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%52 = OpLoad %v4float %50
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpFMul %v2float %48 %53
OpStore %_2_inCoord %54
%56 = OpLoad %v2float %_2_inCoord
%57 = OpCompositeExtract %float %56 0
%58 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_0
OpStore %58 %57
%61 = OpLoad %v2float %_2_inCoord
%62 = OpCompositeExtract %float %61 1
%63 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_1
OpStore %63 %62
%66 = OpLoad %v2float %_3_subsetCoord
OpStore %_4_clampedCoord %66
%69 = OpLoad %22 %uTextureSampler_0_Stage1
%70 = OpLoad %v2float %_4_clampedCoord
%71 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v2float %72 %72 2 3
%74 = OpFMul %v2float %70 %73
%68 = OpImageSampleImplicitLod %v4float %69 %74
OpStore %_5_textureColor %68
%77 = OpLoad %v2float %_2_inCoord
%78 = OpCompositeExtract %float %77 0
%80 = OpFAdd %float %78 %float_0_00100000005
%76 = OpExtInst %float %1 Floor %80
%82 = OpFAdd %float %76 %float_0_5
OpStore %_6_snappedX %82
%84 = OpLoad %float %_6_snappedX
%86 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%87 = OpLoad %v4float %86
%88 = OpCompositeExtract %float %87 0
%89 = OpFOrdLessThan %bool %84 %88
OpSelectionMerge %91 None
OpBranchConditional %89 %91 %90
%90 = OpLabel
%92 = OpLoad %float %_6_snappedX
%93 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 2
%96 = OpFOrdGreaterThan %bool %92 %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %true %31 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%102 = OpLoad %v4float %101
OpStore %_5_textureColor %102
OpBranch %99
%99 = OpLabel
%103 = OpLoad %v4float %_5_textureColor
OpReturnValue %103
OpFunctionEnd
%main = OpFunction %void None %105
%106 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_7_output = OpVariable %_ptr_Function_v4float Function
%_8_coord = OpVariable %_ptr_Function_v2float Function
%_9_coordSampled = OpVariable %_ptr_Function_v2float Function
%127 = OpVariable %_ptr_Function_v4float Function
%129 = OpVariable %_ptr_Function_v2float Function
%144 = OpVariable %_ptr_Function_v4float Function
%146 = OpVariable %_ptr_Function_v2float Function
%160 = OpVariable %_ptr_Function_v4float Function
%162 = OpVariable %_ptr_Function_v2float Function
%176 = OpVariable %_ptr_Function_v4float Function
%178 = OpVariable %_ptr_Function_v2float Function
%192 = OpVariable %_ptr_Function_v4float Function
%194 = OpVariable %_ptr_Function_v2float Function
%208 = OpVariable %_ptr_Function_v4float Function
%210 = OpVariable %_ptr_Function_v2float Function
%224 = OpVariable %_ptr_Function_v4float Function
%226 = OpVariable %_ptr_Function_v2float Function
%240 = OpVariable %_ptr_Function_v4float Function
%242 = OpVariable %_ptr_Function_v2float Function
%256 = OpVariable %_ptr_Function_v4float Function
%258 = OpVariable %_ptr_Function_v2float Function
%272 = OpVariable %_ptr_Function_v4float Function
%274 = OpVariable %_ptr_Function_v2float Function
%288 = OpVariable %_ptr_Function_v4float Function
%290 = OpVariable %_ptr_Function_v2float Function
%304 = OpVariable %_ptr_Function_v4float Function
%306 = OpVariable %_ptr_Function_v2float Function
%320 = OpVariable %_ptr_Function_v4float Function
%322 = OpVariable %_ptr_Function_v2float Function
%336 = OpVariable %_ptr_Function_v4float Function
%338 = OpVariable %_ptr_Function_v2float Function
%352 = OpVariable %_ptr_Function_v4float Function
%354 = OpVariable %_ptr_Function_v2float Function
%368 = OpVariable %_ptr_Function_v4float Function
%370 = OpVariable %_ptr_Function_v2float Function
%384 = OpVariable %_ptr_Function_v4float Function
%386 = OpVariable %_ptr_Function_v2float Function
%400 = OpVariable %_ptr_Function_v4float Function
%402 = OpVariable %_ptr_Function_v2float Function
%416 = OpVariable %_ptr_Function_v4float Function
%418 = OpVariable %_ptr_Function_v2float Function
%432 = OpVariable %_ptr_Function_v4float Function
%434 = OpVariable %_ptr_Function_v2float Function
%448 = OpVariable %_ptr_Function_v4float Function
%450 = OpVariable %_ptr_Function_v2float Function
%464 = OpVariable %_ptr_Function_v4float Function
%466 = OpVariable %_ptr_Function_v2float Function
%480 = OpVariable %_ptr_Function_v4float Function
%482 = OpVariable %_ptr_Function_v2float Function
%496 = OpVariable %_ptr_Function_v4float Function
%498 = OpVariable %_ptr_Function_v2float Function
%512 = OpVariable %_ptr_Function_v4float Function
%514 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %109
OpStore %outputCoverage_Stage0 %109
OpStore %_7_output %113
%115 = OpLoad %v2float %vLocalCoord_Stage0
%117 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%119 = OpLoad %v2float %117
%120 = OpVectorTimesScalar %v2float %119 %float_12
%121 = OpFSub %v2float %115 %120
OpStore %_8_coord %121
OpStore %_9_coordSampled %123
%124 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %124
%125 = OpLoad %v4float %_7_output
%126 = OpLoad %v4float %outputColor_Stage0
OpStore %127 %126
%128 = OpLoad %v2float %_9_coordSampled
OpStore %129 %128
%130 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %127 %129
%132 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 0
%135 = OpVectorTimesScalar %v4float %130 %134
%136 = OpFAdd %v4float %125 %135
OpStore %_7_output %136
%137 = OpLoad %v2float %_8_coord
%138 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%139 = OpLoad %v2float %138
%140 = OpFAdd %v2float %137 %139
OpStore %_8_coord %140
%141 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %141
%142 = OpLoad %v4float %_7_output
%143 = OpLoad %v4float %outputColor_Stage0
OpStore %144 %143
%145 = OpLoad %v2float %_9_coordSampled
OpStore %146 %145
%147 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %144 %146
%148 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 1
%151 = OpVectorTimesScalar %v4float %147 %150
%152 = OpFAdd %v4float %142 %151
OpStore %_7_output %152
%153 = OpLoad %v2float %_8_coord
%154 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%155 = OpLoad %v2float %154
%156 = OpFAdd %v2float %153 %155
OpStore %_8_coord %156
%157 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %157
%158 = OpLoad %v4float %_7_output
%159 = OpLoad %v4float %outputColor_Stage0
OpStore %160 %159
%161 = OpLoad %v2float %_9_coordSampled
OpStore %162 %161
%163 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %160 %162
%164 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 2
%167 = OpVectorTimesScalar %v4float %163 %166
%168 = OpFAdd %v4float %158 %167
OpStore %_7_output %168
%169 = OpLoad %v2float %_8_coord
%170 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%171 = OpLoad %v2float %170
%172 = OpFAdd %v2float %169 %171
OpStore %_8_coord %172
%173 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %173
%174 = OpLoad %v4float %_7_output
%175 = OpLoad %v4float %outputColor_Stage0
OpStore %176 %175
%177 = OpLoad %v2float %_9_coordSampled
OpStore %178 %177
%179 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %176 %178
%180 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 3
%183 = OpVectorTimesScalar %v4float %179 %182
%184 = OpFAdd %v4float %174 %183
OpStore %_7_output %184
%185 = OpLoad %v2float %_8_coord
%186 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%187 = OpLoad %v2float %186
%188 = OpFAdd %v2float %185 %187
OpStore %_8_coord %188
%189 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %189
%190 = OpLoad %v4float %_7_output
%191 = OpLoad %v4float %outputColor_Stage0
OpStore %192 %191
%193 = OpLoad %v2float %_9_coordSampled
OpStore %194 %193
%195 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %192 %194
%196 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%197 = OpLoad %v4float %196
%198 = OpCompositeExtract %float %197 0
%199 = OpVectorTimesScalar %v4float %195 %198
%200 = OpFAdd %v4float %190 %199
OpStore %_7_output %200
%201 = OpLoad %v2float %_8_coord
%202 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%203 = OpLoad %v2float %202
%204 = OpFAdd %v2float %201 %203
OpStore %_8_coord %204
%205 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %205
%206 = OpLoad %v4float %_7_output
%207 = OpLoad %v4float %outputColor_Stage0
OpStore %208 %207
%209 = OpLoad %v2float %_9_coordSampled
OpStore %210 %209
%211 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %208 %210
%212 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%213 = OpLoad %v4float %212
%214 = OpCompositeExtract %float %213 1
%215 = OpVectorTimesScalar %v4float %211 %214
%216 = OpFAdd %v4float %206 %215
OpStore %_7_output %216
%217 = OpLoad %v2float %_8_coord
%218 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%219 = OpLoad %v2float %218
%220 = OpFAdd %v2float %217 %219
OpStore %_8_coord %220
%221 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %221
%222 = OpLoad %v4float %_7_output
%223 = OpLoad %v4float %outputColor_Stage0
OpStore %224 %223
%225 = OpLoad %v2float %_9_coordSampled
OpStore %226 %225
%227 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %224 %226
%228 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%229 = OpLoad %v4float %228
%230 = OpCompositeExtract %float %229 2
%231 = OpVectorTimesScalar %v4float %227 %230
%232 = OpFAdd %v4float %222 %231
OpStore %_7_output %232
%233 = OpLoad %v2float %_8_coord
%234 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%235 = OpLoad %v2float %234
%236 = OpFAdd %v2float %233 %235
OpStore %_8_coord %236
%237 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %237
%238 = OpLoad %v4float %_7_output
%239 = OpLoad %v4float %outputColor_Stage0
OpStore %240 %239
%241 = OpLoad %v2float %_9_coordSampled
OpStore %242 %241
%243 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %240 %242
%244 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%245 = OpLoad %v4float %244
%246 = OpCompositeExtract %float %245 3
%247 = OpVectorTimesScalar %v4float %243 %246
%248 = OpFAdd %v4float %238 %247
OpStore %_7_output %248
%249 = OpLoad %v2float %_8_coord
%250 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%251 = OpLoad %v2float %250
%252 = OpFAdd %v2float %249 %251
OpStore %_8_coord %252
%253 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %253
%254 = OpLoad %v4float %_7_output
%255 = OpLoad %v4float %outputColor_Stage0
OpStore %256 %255
%257 = OpLoad %v2float %_9_coordSampled
OpStore %258 %257
%259 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %256 %258
%260 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%261 = OpLoad %v4float %260
%262 = OpCompositeExtract %float %261 0
%263 = OpVectorTimesScalar %v4float %259 %262
%264 = OpFAdd %v4float %254 %263
OpStore %_7_output %264
%265 = OpLoad %v2float %_8_coord
%266 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%267 = OpLoad %v2float %266
%268 = OpFAdd %v2float %265 %267
OpStore %_8_coord %268
%269 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %269
%270 = OpLoad %v4float %_7_output
%271 = OpLoad %v4float %outputColor_Stage0
OpStore %272 %271
%273 = OpLoad %v2float %_9_coordSampled
OpStore %274 %273
%275 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %272 %274
%276 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%277 = OpLoad %v4float %276
%278 = OpCompositeExtract %float %277 1
%279 = OpVectorTimesScalar %v4float %275 %278
%280 = OpFAdd %v4float %270 %279
OpStore %_7_output %280
%281 = OpLoad %v2float %_8_coord
%282 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%283 = OpLoad %v2float %282
%284 = OpFAdd %v2float %281 %283
OpStore %_8_coord %284
%285 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %285
%286 = OpLoad %v4float %_7_output
%287 = OpLoad %v4float %outputColor_Stage0
OpStore %288 %287
%289 = OpLoad %v2float %_9_coordSampled
OpStore %290 %289
%291 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %288 %290
%292 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%293 = OpLoad %v4float %292
%294 = OpCompositeExtract %float %293 2
%295 = OpVectorTimesScalar %v4float %291 %294
%296 = OpFAdd %v4float %286 %295
OpStore %_7_output %296
%297 = OpLoad %v2float %_8_coord
%298 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%299 = OpLoad %v2float %298
%300 = OpFAdd %v2float %297 %299
OpStore %_8_coord %300
%301 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %301
%302 = OpLoad %v4float %_7_output
%303 = OpLoad %v4float %outputColor_Stage0
OpStore %304 %303
%305 = OpLoad %v2float %_9_coordSampled
OpStore %306 %305
%307 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %304 %306
%308 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%309 = OpLoad %v4float %308
%310 = OpCompositeExtract %float %309 3
%311 = OpVectorTimesScalar %v4float %307 %310
%312 = OpFAdd %v4float %302 %311
OpStore %_7_output %312
%313 = OpLoad %v2float %_8_coord
%314 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%315 = OpLoad %v2float %314
%316 = OpFAdd %v2float %313 %315
OpStore %_8_coord %316
%317 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %317
%318 = OpLoad %v4float %_7_output
%319 = OpLoad %v4float %outputColor_Stage0
OpStore %320 %319
%321 = OpLoad %v2float %_9_coordSampled
OpStore %322 %321
%323 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %320 %322
%324 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%325 = OpLoad %v4float %324
%326 = OpCompositeExtract %float %325 0
%327 = OpVectorTimesScalar %v4float %323 %326
%328 = OpFAdd %v4float %318 %327
OpStore %_7_output %328
%329 = OpLoad %v2float %_8_coord
%330 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%331 = OpLoad %v2float %330
%332 = OpFAdd %v2float %329 %331
OpStore %_8_coord %332
%333 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %333
%334 = OpLoad %v4float %_7_output
%335 = OpLoad %v4float %outputColor_Stage0
OpStore %336 %335
%337 = OpLoad %v2float %_9_coordSampled
OpStore %338 %337
%339 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %336 %338
%340 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%341 = OpLoad %v4float %340
%342 = OpCompositeExtract %float %341 1
%343 = OpVectorTimesScalar %v4float %339 %342
%344 = OpFAdd %v4float %334 %343
OpStore %_7_output %344
%345 = OpLoad %v2float %_8_coord
%346 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%347 = OpLoad %v2float %346
%348 = OpFAdd %v2float %345 %347
OpStore %_8_coord %348
%349 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %349
%350 = OpLoad %v4float %_7_output
%351 = OpLoad %v4float %outputColor_Stage0
OpStore %352 %351
%353 = OpLoad %v2float %_9_coordSampled
OpStore %354 %353
%355 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %352 %354
%356 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%357 = OpLoad %v4float %356
%358 = OpCompositeExtract %float %357 2
%359 = OpVectorTimesScalar %v4float %355 %358
%360 = OpFAdd %v4float %350 %359
OpStore %_7_output %360
%361 = OpLoad %v2float %_8_coord
%362 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%363 = OpLoad %v2float %362
%364 = OpFAdd %v2float %361 %363
OpStore %_8_coord %364
%365 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %365
%366 = OpLoad %v4float %_7_output
%367 = OpLoad %v4float %outputColor_Stage0
OpStore %368 %367
%369 = OpLoad %v2float %_9_coordSampled
OpStore %370 %369
%371 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %368 %370
%372 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%373 = OpLoad %v4float %372
%374 = OpCompositeExtract %float %373 3
%375 = OpVectorTimesScalar %v4float %371 %374
%376 = OpFAdd %v4float %366 %375
OpStore %_7_output %376
%377 = OpLoad %v2float %_8_coord
%378 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%379 = OpLoad %v2float %378
%380 = OpFAdd %v2float %377 %379
OpStore %_8_coord %380
%381 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %381
%382 = OpLoad %v4float %_7_output
%383 = OpLoad %v4float %outputColor_Stage0
OpStore %384 %383
%385 = OpLoad %v2float %_9_coordSampled
OpStore %386 %385
%387 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %384 %386
%388 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%389 = OpLoad %v4float %388
%390 = OpCompositeExtract %float %389 0
%391 = OpVectorTimesScalar %v4float %387 %390
%392 = OpFAdd %v4float %382 %391
OpStore %_7_output %392
%393 = OpLoad %v2float %_8_coord
%394 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%395 = OpLoad %v2float %394
%396 = OpFAdd %v2float %393 %395
OpStore %_8_coord %396
%397 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %397
%398 = OpLoad %v4float %_7_output
%399 = OpLoad %v4float %outputColor_Stage0
OpStore %400 %399
%401 = OpLoad %v2float %_9_coordSampled
OpStore %402 %401
%403 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %400 %402
%404 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%405 = OpLoad %v4float %404
%406 = OpCompositeExtract %float %405 1
%407 = OpVectorTimesScalar %v4float %403 %406
%408 = OpFAdd %v4float %398 %407
OpStore %_7_output %408
%409 = OpLoad %v2float %_8_coord
%410 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%411 = OpLoad %v2float %410
%412 = OpFAdd %v2float %409 %411
OpStore %_8_coord %412
%413 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %413
%414 = OpLoad %v4float %_7_output
%415 = OpLoad %v4float %outputColor_Stage0
OpStore %416 %415
%417 = OpLoad %v2float %_9_coordSampled
OpStore %418 %417
%419 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %416 %418
%420 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%421 = OpLoad %v4float %420
%422 = OpCompositeExtract %float %421 2
%423 = OpVectorTimesScalar %v4float %419 %422
%424 = OpFAdd %v4float %414 %423
OpStore %_7_output %424
%425 = OpLoad %v2float %_8_coord
%426 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%427 = OpLoad %v2float %426
%428 = OpFAdd %v2float %425 %427
OpStore %_8_coord %428
%429 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %429
%430 = OpLoad %v4float %_7_output
%431 = OpLoad %v4float %outputColor_Stage0
OpStore %432 %431
%433 = OpLoad %v2float %_9_coordSampled
OpStore %434 %433
%435 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %432 %434
%436 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%437 = OpLoad %v4float %436
%438 = OpCompositeExtract %float %437 3
%439 = OpVectorTimesScalar %v4float %435 %438
%440 = OpFAdd %v4float %430 %439
OpStore %_7_output %440
%441 = OpLoad %v2float %_8_coord
%442 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%443 = OpLoad %v2float %442
%444 = OpFAdd %v2float %441 %443
OpStore %_8_coord %444
%445 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %445
%446 = OpLoad %v4float %_7_output
%447 = OpLoad %v4float %outputColor_Stage0
OpStore %448 %447
%449 = OpLoad %v2float %_9_coordSampled
OpStore %450 %449
%451 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %448 %450
%452 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%453 = OpLoad %v4float %452
%454 = OpCompositeExtract %float %453 0
%455 = OpVectorTimesScalar %v4float %451 %454
%456 = OpFAdd %v4float %446 %455
OpStore %_7_output %456
%457 = OpLoad %v2float %_8_coord
%458 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%459 = OpLoad %v2float %458
%460 = OpFAdd %v2float %457 %459
OpStore %_8_coord %460
%461 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %461
%462 = OpLoad %v4float %_7_output
%463 = OpLoad %v4float %outputColor_Stage0
OpStore %464 %463
%465 = OpLoad %v2float %_9_coordSampled
OpStore %466 %465
%467 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %464 %466
%468 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%469 = OpLoad %v4float %468
%470 = OpCompositeExtract %float %469 1
%471 = OpVectorTimesScalar %v4float %467 %470
%472 = OpFAdd %v4float %462 %471
OpStore %_7_output %472
%473 = OpLoad %v2float %_8_coord
%474 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%475 = OpLoad %v2float %474
%476 = OpFAdd %v2float %473 %475
OpStore %_8_coord %476
%477 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %477
%478 = OpLoad %v4float %_7_output
%479 = OpLoad %v4float %outputColor_Stage0
OpStore %480 %479
%481 = OpLoad %v2float %_9_coordSampled
OpStore %482 %481
%483 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %480 %482
%484 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%485 = OpLoad %v4float %484
%486 = OpCompositeExtract %float %485 2
%487 = OpVectorTimesScalar %v4float %483 %486
%488 = OpFAdd %v4float %478 %487
OpStore %_7_output %488
%489 = OpLoad %v2float %_8_coord
%490 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%491 = OpLoad %v2float %490
%492 = OpFAdd %v2float %489 %491
OpStore %_8_coord %492
%493 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %493
%494 = OpLoad %v4float %_7_output
%495 = OpLoad %v4float %outputColor_Stage0
OpStore %496 %495
%497 = OpLoad %v2float %_9_coordSampled
OpStore %498 %497
%499 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %496 %498
%500 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%501 = OpLoad %v4float %500
%502 = OpCompositeExtract %float %501 3
%503 = OpVectorTimesScalar %v4float %499 %502
%504 = OpFAdd %v4float %494 %503
OpStore %_7_output %504
%505 = OpLoad %v2float %_8_coord
%506 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%507 = OpLoad %v2float %506
%508 = OpFAdd %v2float %505 %507
OpStore %_8_coord %508
%509 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %509
%510 = OpLoad %v4float %_7_output
%511 = OpLoad %v4float %outputColor_Stage0
OpStore %512 %511
%513 = OpLoad %v2float %_9_coordSampled
OpStore %514 %513
%515 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %512 %514
%516 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%517 = OpLoad %v4float %516
%518 = OpCompositeExtract %float %517 0
%519 = OpVectorTimesScalar %v4float %515 %518
%520 = OpFAdd %v4float %510 %519
OpStore %_7_output %520
%521 = OpLoad %v2float %_8_coord
%522 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%523 = OpLoad %v2float %522
%524 = OpFAdd %v2float %521 %523
OpStore %_8_coord %524
%525 = OpLoad %v4float %_7_output
%526 = OpLoad %v4float %outputColor_Stage0
%527 = OpFMul %v4float %525 %526
OpStore %_7_output %527
%528 = OpLoad %v4float %_7_output
OpStore %output_Stage1 %528
%529 = OpLoad %v4float %output_Stage1
%530 = OpLoad %v4float %outputCoverage_Stage0
%531 = OpFMul %v4float %529 %530
OpStore %sk_FragColor %531
OpReturn
OpFunctionEnd
