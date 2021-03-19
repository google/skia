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
OpName %_0_coords "_0_coords"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %uTextureSampler_0_Stage1 RelaxedPrecision
OpDecorate %uTextureSampler_0_Stage1 Binding 0
OpDecorate %uTextureSampler_0_Stage1 DescriptorSet 0
OpDecorate %vLocalCoord_Stage0 Location 0
OpDecorate %67 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %428 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %451 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %502 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
OpDecorate %515 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %521 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %529 RelaxedPrecision
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
%103 = OpTypeFunction %void
%107 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_0 = OpConstant %float 0
%111 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%121 = OpConstantComposite %v2float %float_0 %float_0
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_0_coords = OpVariable %_ptr_Function_v2float Function
%_2_inCoord = OpVariable %_ptr_Function_v2float Function
%_3_subsetCoord = OpVariable %_ptr_Function_v2float Function
%_4_clampedCoord = OpVariable %_ptr_Function_v2float Function
%_5_textureColor = OpVariable %_ptr_Function_v4float Function
%_6_snappedX = OpVariable %_ptr_Function_float Function
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %4 %int_3
%36 = OpLoad %mat3v3float %34
%37 = OpLoad %v2float %30
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%41 = OpCompositeConstruct %v3float %38 %39 %float_1
%42 = OpMatrixTimesVector %v3float %36 %41
%43 = OpVectorShuffle %v2float %42 %42 0 1
OpStore %_0_coords %43
%45 = OpLoad %v2float %_0_coords
OpStore %_2_inCoord %45
%46 = OpLoad %v2float %_2_inCoord
%48 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%50 = OpLoad %v4float %48
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFMul %v2float %46 %51
OpStore %_2_inCoord %52
%54 = OpLoad %v2float %_2_inCoord
%55 = OpCompositeExtract %float %54 0
%56 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_0
OpStore %56 %55
%59 = OpLoad %v2float %_2_inCoord
%60 = OpCompositeExtract %float %59 1
%61 = OpAccessChain %_ptr_Function_float %_3_subsetCoord %int_1
OpStore %61 %60
%64 = OpLoad %v2float %_3_subsetCoord
OpStore %_4_clampedCoord %64
%67 = OpLoad %22 %uTextureSampler_0_Stage1
%68 = OpLoad %v2float %_4_clampedCoord
%69 = OpAccessChain %_ptr_Uniform_v4float %4 %int_6
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 2 3
%72 = OpFMul %v2float %68 %71
%66 = OpImageSampleImplicitLod %v4float %67 %72
OpStore %_5_textureColor %66
%75 = OpLoad %v2float %_2_inCoord
%76 = OpCompositeExtract %float %75 0
%78 = OpFAdd %float %76 %float_0_00100000005
%74 = OpExtInst %float %1 Floor %78
%80 = OpFAdd %float %74 %float_0_5
OpStore %_6_snappedX %80
%82 = OpLoad %float %_6_snappedX
%84 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdLessThan %bool %82 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %89 %88
%88 = OpLabel
%90 = OpLoad %float %_6_snappedX
%91 = OpAccessChain %_ptr_Uniform_v4float %4 %int_5
%92 = OpLoad %v4float %91
%93 = OpCompositeExtract %float %92 2
%94 = OpFOrdGreaterThan %bool %90 %93
OpBranch %89
%89 = OpLabel
%95 = OpPhi %bool %true %31 %94 %88
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %4 %int_4
%100 = OpLoad %v4float %99
OpStore %_5_textureColor %100
OpBranch %97
%97 = OpLabel
%101 = OpLoad %v4float %_5_textureColor
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %void None %103
%104 = OpLabel
%outputColor_Stage0 = OpVariable %_ptr_Function_v4float Function
%outputCoverage_Stage0 = OpVariable %_ptr_Function_v4float Function
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_7_output = OpVariable %_ptr_Function_v4float Function
%_8_coord = OpVariable %_ptr_Function_v2float Function
%_9_coordSampled = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v4float Function
%127 = OpVariable %_ptr_Function_v2float Function
%142 = OpVariable %_ptr_Function_v4float Function
%144 = OpVariable %_ptr_Function_v2float Function
%158 = OpVariable %_ptr_Function_v4float Function
%160 = OpVariable %_ptr_Function_v2float Function
%174 = OpVariable %_ptr_Function_v4float Function
%176 = OpVariable %_ptr_Function_v2float Function
%190 = OpVariable %_ptr_Function_v4float Function
%192 = OpVariable %_ptr_Function_v2float Function
%206 = OpVariable %_ptr_Function_v4float Function
%208 = OpVariable %_ptr_Function_v2float Function
%222 = OpVariable %_ptr_Function_v4float Function
%224 = OpVariable %_ptr_Function_v2float Function
%238 = OpVariable %_ptr_Function_v4float Function
%240 = OpVariable %_ptr_Function_v2float Function
%254 = OpVariable %_ptr_Function_v4float Function
%256 = OpVariable %_ptr_Function_v2float Function
%270 = OpVariable %_ptr_Function_v4float Function
%272 = OpVariable %_ptr_Function_v2float Function
%286 = OpVariable %_ptr_Function_v4float Function
%288 = OpVariable %_ptr_Function_v2float Function
%302 = OpVariable %_ptr_Function_v4float Function
%304 = OpVariable %_ptr_Function_v2float Function
%318 = OpVariable %_ptr_Function_v4float Function
%320 = OpVariable %_ptr_Function_v2float Function
%334 = OpVariable %_ptr_Function_v4float Function
%336 = OpVariable %_ptr_Function_v2float Function
%350 = OpVariable %_ptr_Function_v4float Function
%352 = OpVariable %_ptr_Function_v2float Function
%366 = OpVariable %_ptr_Function_v4float Function
%368 = OpVariable %_ptr_Function_v2float Function
%382 = OpVariable %_ptr_Function_v4float Function
%384 = OpVariable %_ptr_Function_v2float Function
%398 = OpVariable %_ptr_Function_v4float Function
%400 = OpVariable %_ptr_Function_v2float Function
%414 = OpVariable %_ptr_Function_v4float Function
%416 = OpVariable %_ptr_Function_v2float Function
%430 = OpVariable %_ptr_Function_v4float Function
%432 = OpVariable %_ptr_Function_v2float Function
%446 = OpVariable %_ptr_Function_v4float Function
%448 = OpVariable %_ptr_Function_v2float Function
%462 = OpVariable %_ptr_Function_v4float Function
%464 = OpVariable %_ptr_Function_v2float Function
%478 = OpVariable %_ptr_Function_v4float Function
%480 = OpVariable %_ptr_Function_v2float Function
%494 = OpVariable %_ptr_Function_v4float Function
%496 = OpVariable %_ptr_Function_v2float Function
%510 = OpVariable %_ptr_Function_v4float Function
%512 = OpVariable %_ptr_Function_v2float Function
OpStore %outputColor_Stage0 %107
OpStore %outputCoverage_Stage0 %107
OpStore %_7_output %111
%113 = OpLoad %v2float %vLocalCoord_Stage0
%115 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%117 = OpLoad %v2float %115
%118 = OpVectorTimesScalar %v2float %117 %float_12
%119 = OpFSub %v2float %113 %118
OpStore %_8_coord %119
OpStore %_9_coordSampled %121
%122 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %122
%123 = OpLoad %v4float %_7_output
%124 = OpLoad %v4float %outputColor_Stage0
OpStore %125 %124
%126 = OpLoad %v2float %_9_coordSampled
OpStore %127 %126
%128 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %125 %127
%130 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 0
%133 = OpVectorTimesScalar %v4float %128 %132
%134 = OpFAdd %v4float %123 %133
OpStore %_7_output %134
%135 = OpLoad %v2float %_8_coord
%136 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%137 = OpLoad %v2float %136
%138 = OpFAdd %v2float %135 %137
OpStore %_8_coord %138
%139 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %139
%140 = OpLoad %v4float %_7_output
%141 = OpLoad %v4float %outputColor_Stage0
OpStore %142 %141
%143 = OpLoad %v2float %_9_coordSampled
OpStore %144 %143
%145 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %142 %144
%146 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 1
%149 = OpVectorTimesScalar %v4float %145 %148
%150 = OpFAdd %v4float %140 %149
OpStore %_7_output %150
%151 = OpLoad %v2float %_8_coord
%152 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%153 = OpLoad %v2float %152
%154 = OpFAdd %v2float %151 %153
OpStore %_8_coord %154
%155 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %155
%156 = OpLoad %v4float %_7_output
%157 = OpLoad %v4float %outputColor_Stage0
OpStore %158 %157
%159 = OpLoad %v2float %_9_coordSampled
OpStore %160 %159
%161 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %158 %160
%162 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 2
%165 = OpVectorTimesScalar %v4float %161 %164
%166 = OpFAdd %v4float %156 %165
OpStore %_7_output %166
%167 = OpLoad %v2float %_8_coord
%168 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%169 = OpLoad %v2float %168
%170 = OpFAdd %v2float %167 %169
OpStore %_8_coord %170
%171 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %171
%172 = OpLoad %v4float %_7_output
%173 = OpLoad %v4float %outputColor_Stage0
OpStore %174 %173
%175 = OpLoad %v2float %_9_coordSampled
OpStore %176 %175
%177 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %174 %176
%178 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%179 = OpLoad %v4float %178
%180 = OpCompositeExtract %float %179 3
%181 = OpVectorTimesScalar %v4float %177 %180
%182 = OpFAdd %v4float %172 %181
OpStore %_7_output %182
%183 = OpLoad %v2float %_8_coord
%184 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%185 = OpLoad %v2float %184
%186 = OpFAdd %v2float %183 %185
OpStore %_8_coord %186
%187 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %187
%188 = OpLoad %v4float %_7_output
%189 = OpLoad %v4float %outputColor_Stage0
OpStore %190 %189
%191 = OpLoad %v2float %_9_coordSampled
OpStore %192 %191
%193 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %190 %192
%194 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%195 = OpLoad %v4float %194
%196 = OpCompositeExtract %float %195 0
%197 = OpVectorTimesScalar %v4float %193 %196
%198 = OpFAdd %v4float %188 %197
OpStore %_7_output %198
%199 = OpLoad %v2float %_8_coord
%200 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%201 = OpLoad %v2float %200
%202 = OpFAdd %v2float %199 %201
OpStore %_8_coord %202
%203 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %203
%204 = OpLoad %v4float %_7_output
%205 = OpLoad %v4float %outputColor_Stage0
OpStore %206 %205
%207 = OpLoad %v2float %_9_coordSampled
OpStore %208 %207
%209 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %206 %208
%210 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%211 = OpLoad %v4float %210
%212 = OpCompositeExtract %float %211 1
%213 = OpVectorTimesScalar %v4float %209 %212
%214 = OpFAdd %v4float %204 %213
OpStore %_7_output %214
%215 = OpLoad %v2float %_8_coord
%216 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%217 = OpLoad %v2float %216
%218 = OpFAdd %v2float %215 %217
OpStore %_8_coord %218
%219 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %219
%220 = OpLoad %v4float %_7_output
%221 = OpLoad %v4float %outputColor_Stage0
OpStore %222 %221
%223 = OpLoad %v2float %_9_coordSampled
OpStore %224 %223
%225 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %222 %224
%226 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%227 = OpLoad %v4float %226
%228 = OpCompositeExtract %float %227 2
%229 = OpVectorTimesScalar %v4float %225 %228
%230 = OpFAdd %v4float %220 %229
OpStore %_7_output %230
%231 = OpLoad %v2float %_8_coord
%232 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%233 = OpLoad %v2float %232
%234 = OpFAdd %v2float %231 %233
OpStore %_8_coord %234
%235 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %235
%236 = OpLoad %v4float %_7_output
%237 = OpLoad %v4float %outputColor_Stage0
OpStore %238 %237
%239 = OpLoad %v2float %_9_coordSampled
OpStore %240 %239
%241 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %238 %240
%242 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%243 = OpLoad %v4float %242
%244 = OpCompositeExtract %float %243 3
%245 = OpVectorTimesScalar %v4float %241 %244
%246 = OpFAdd %v4float %236 %245
OpStore %_7_output %246
%247 = OpLoad %v2float %_8_coord
%248 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%249 = OpLoad %v2float %248
%250 = OpFAdd %v2float %247 %249
OpStore %_8_coord %250
%251 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %251
%252 = OpLoad %v4float %_7_output
%253 = OpLoad %v4float %outputColor_Stage0
OpStore %254 %253
%255 = OpLoad %v2float %_9_coordSampled
OpStore %256 %255
%257 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %254 %256
%258 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%259 = OpLoad %v4float %258
%260 = OpCompositeExtract %float %259 0
%261 = OpVectorTimesScalar %v4float %257 %260
%262 = OpFAdd %v4float %252 %261
OpStore %_7_output %262
%263 = OpLoad %v2float %_8_coord
%264 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%265 = OpLoad %v2float %264
%266 = OpFAdd %v2float %263 %265
OpStore %_8_coord %266
%267 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %267
%268 = OpLoad %v4float %_7_output
%269 = OpLoad %v4float %outputColor_Stage0
OpStore %270 %269
%271 = OpLoad %v2float %_9_coordSampled
OpStore %272 %271
%273 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %270 %272
%274 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%275 = OpLoad %v4float %274
%276 = OpCompositeExtract %float %275 1
%277 = OpVectorTimesScalar %v4float %273 %276
%278 = OpFAdd %v4float %268 %277
OpStore %_7_output %278
%279 = OpLoad %v2float %_8_coord
%280 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%281 = OpLoad %v2float %280
%282 = OpFAdd %v2float %279 %281
OpStore %_8_coord %282
%283 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %283
%284 = OpLoad %v4float %_7_output
%285 = OpLoad %v4float %outputColor_Stage0
OpStore %286 %285
%287 = OpLoad %v2float %_9_coordSampled
OpStore %288 %287
%289 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %286 %288
%290 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%291 = OpLoad %v4float %290
%292 = OpCompositeExtract %float %291 2
%293 = OpVectorTimesScalar %v4float %289 %292
%294 = OpFAdd %v4float %284 %293
OpStore %_7_output %294
%295 = OpLoad %v2float %_8_coord
%296 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%297 = OpLoad %v2float %296
%298 = OpFAdd %v2float %295 %297
OpStore %_8_coord %298
%299 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %299
%300 = OpLoad %v4float %_7_output
%301 = OpLoad %v4float %outputColor_Stage0
OpStore %302 %301
%303 = OpLoad %v2float %_9_coordSampled
OpStore %304 %303
%305 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %302 %304
%306 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%307 = OpLoad %v4float %306
%308 = OpCompositeExtract %float %307 3
%309 = OpVectorTimesScalar %v4float %305 %308
%310 = OpFAdd %v4float %300 %309
OpStore %_7_output %310
%311 = OpLoad %v2float %_8_coord
%312 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%313 = OpLoad %v2float %312
%314 = OpFAdd %v2float %311 %313
OpStore %_8_coord %314
%315 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %315
%316 = OpLoad %v4float %_7_output
%317 = OpLoad %v4float %outputColor_Stage0
OpStore %318 %317
%319 = OpLoad %v2float %_9_coordSampled
OpStore %320 %319
%321 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %318 %320
%322 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%323 = OpLoad %v4float %322
%324 = OpCompositeExtract %float %323 0
%325 = OpVectorTimesScalar %v4float %321 %324
%326 = OpFAdd %v4float %316 %325
OpStore %_7_output %326
%327 = OpLoad %v2float %_8_coord
%328 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%329 = OpLoad %v2float %328
%330 = OpFAdd %v2float %327 %329
OpStore %_8_coord %330
%331 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %331
%332 = OpLoad %v4float %_7_output
%333 = OpLoad %v4float %outputColor_Stage0
OpStore %334 %333
%335 = OpLoad %v2float %_9_coordSampled
OpStore %336 %335
%337 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %334 %336
%338 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 1
%341 = OpVectorTimesScalar %v4float %337 %340
%342 = OpFAdd %v4float %332 %341
OpStore %_7_output %342
%343 = OpLoad %v2float %_8_coord
%344 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%345 = OpLoad %v2float %344
%346 = OpFAdd %v2float %343 %345
OpStore %_8_coord %346
%347 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %347
%348 = OpLoad %v4float %_7_output
%349 = OpLoad %v4float %outputColor_Stage0
OpStore %350 %349
%351 = OpLoad %v2float %_9_coordSampled
OpStore %352 %351
%353 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %350 %352
%354 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%355 = OpLoad %v4float %354
%356 = OpCompositeExtract %float %355 2
%357 = OpVectorTimesScalar %v4float %353 %356
%358 = OpFAdd %v4float %348 %357
OpStore %_7_output %358
%359 = OpLoad %v2float %_8_coord
%360 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%361 = OpLoad %v2float %360
%362 = OpFAdd %v2float %359 %361
OpStore %_8_coord %362
%363 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %363
%364 = OpLoad %v4float %_7_output
%365 = OpLoad %v4float %outputColor_Stage0
OpStore %366 %365
%367 = OpLoad %v2float %_9_coordSampled
OpStore %368 %367
%369 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %366 %368
%370 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%371 = OpLoad %v4float %370
%372 = OpCompositeExtract %float %371 3
%373 = OpVectorTimesScalar %v4float %369 %372
%374 = OpFAdd %v4float %364 %373
OpStore %_7_output %374
%375 = OpLoad %v2float %_8_coord
%376 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%377 = OpLoad %v2float %376
%378 = OpFAdd %v2float %375 %377
OpStore %_8_coord %378
%379 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %379
%380 = OpLoad %v4float %_7_output
%381 = OpLoad %v4float %outputColor_Stage0
OpStore %382 %381
%383 = OpLoad %v2float %_9_coordSampled
OpStore %384 %383
%385 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %382 %384
%386 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%387 = OpLoad %v4float %386
%388 = OpCompositeExtract %float %387 0
%389 = OpVectorTimesScalar %v4float %385 %388
%390 = OpFAdd %v4float %380 %389
OpStore %_7_output %390
%391 = OpLoad %v2float %_8_coord
%392 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%393 = OpLoad %v2float %392
%394 = OpFAdd %v2float %391 %393
OpStore %_8_coord %394
%395 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %395
%396 = OpLoad %v4float %_7_output
%397 = OpLoad %v4float %outputColor_Stage0
OpStore %398 %397
%399 = OpLoad %v2float %_9_coordSampled
OpStore %400 %399
%401 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %398 %400
%402 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%403 = OpLoad %v4float %402
%404 = OpCompositeExtract %float %403 1
%405 = OpVectorTimesScalar %v4float %401 %404
%406 = OpFAdd %v4float %396 %405
OpStore %_7_output %406
%407 = OpLoad %v2float %_8_coord
%408 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%409 = OpLoad %v2float %408
%410 = OpFAdd %v2float %407 %409
OpStore %_8_coord %410
%411 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %411
%412 = OpLoad %v4float %_7_output
%413 = OpLoad %v4float %outputColor_Stage0
OpStore %414 %413
%415 = OpLoad %v2float %_9_coordSampled
OpStore %416 %415
%417 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %414 %416
%418 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%419 = OpLoad %v4float %418
%420 = OpCompositeExtract %float %419 2
%421 = OpVectorTimesScalar %v4float %417 %420
%422 = OpFAdd %v4float %412 %421
OpStore %_7_output %422
%423 = OpLoad %v2float %_8_coord
%424 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%425 = OpLoad %v2float %424
%426 = OpFAdd %v2float %423 %425
OpStore %_8_coord %426
%427 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %427
%428 = OpLoad %v4float %_7_output
%429 = OpLoad %v4float %outputColor_Stage0
OpStore %430 %429
%431 = OpLoad %v2float %_9_coordSampled
OpStore %432 %431
%433 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %430 %432
%434 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%435 = OpLoad %v4float %434
%436 = OpCompositeExtract %float %435 3
%437 = OpVectorTimesScalar %v4float %433 %436
%438 = OpFAdd %v4float %428 %437
OpStore %_7_output %438
%439 = OpLoad %v2float %_8_coord
%440 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%441 = OpLoad %v2float %440
%442 = OpFAdd %v2float %439 %441
OpStore %_8_coord %442
%443 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %443
%444 = OpLoad %v4float %_7_output
%445 = OpLoad %v4float %outputColor_Stage0
OpStore %446 %445
%447 = OpLoad %v2float %_9_coordSampled
OpStore %448 %447
%449 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %446 %448
%450 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%451 = OpLoad %v4float %450
%452 = OpCompositeExtract %float %451 0
%453 = OpVectorTimesScalar %v4float %449 %452
%454 = OpFAdd %v4float %444 %453
OpStore %_7_output %454
%455 = OpLoad %v2float %_8_coord
%456 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%457 = OpLoad %v2float %456
%458 = OpFAdd %v2float %455 %457
OpStore %_8_coord %458
%459 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %459
%460 = OpLoad %v4float %_7_output
%461 = OpLoad %v4float %outputColor_Stage0
OpStore %462 %461
%463 = OpLoad %v2float %_9_coordSampled
OpStore %464 %463
%465 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %462 %464
%466 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%467 = OpLoad %v4float %466
%468 = OpCompositeExtract %float %467 1
%469 = OpVectorTimesScalar %v4float %465 %468
%470 = OpFAdd %v4float %460 %469
OpStore %_7_output %470
%471 = OpLoad %v2float %_8_coord
%472 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%473 = OpLoad %v2float %472
%474 = OpFAdd %v2float %471 %473
OpStore %_8_coord %474
%475 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %475
%476 = OpLoad %v4float %_7_output
%477 = OpLoad %v4float %outputColor_Stage0
OpStore %478 %477
%479 = OpLoad %v2float %_9_coordSampled
OpStore %480 %479
%481 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %478 %480
%482 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%483 = OpLoad %v4float %482
%484 = OpCompositeExtract %float %483 2
%485 = OpVectorTimesScalar %v4float %481 %484
%486 = OpFAdd %v4float %476 %485
OpStore %_7_output %486
%487 = OpLoad %v2float %_8_coord
%488 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%489 = OpLoad %v2float %488
%490 = OpFAdd %v2float %487 %489
OpStore %_8_coord %490
%491 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %491
%492 = OpLoad %v4float %_7_output
%493 = OpLoad %v4float %outputColor_Stage0
OpStore %494 %493
%495 = OpLoad %v2float %_9_coordSampled
OpStore %496 %495
%497 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %494 %496
%498 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%499 = OpLoad %v4float %498
%500 = OpCompositeExtract %float %499 3
%501 = OpVectorTimesScalar %v4float %497 %500
%502 = OpFAdd %v4float %492 %501
OpStore %_7_output %502
%503 = OpLoad %v2float %_8_coord
%504 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%505 = OpLoad %v2float %504
%506 = OpFAdd %v2float %503 %505
OpStore %_8_coord %506
%507 = OpLoad %v2float %_8_coord
OpStore %_9_coordSampled %507
%508 = OpLoad %v4float %_7_output
%509 = OpLoad %v4float %outputColor_Stage0
OpStore %510 %509
%511 = OpLoad %v2float %_9_coordSampled
OpStore %512 %511
%513 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %510 %512
%514 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%515 = OpLoad %v4float %514
%516 = OpCompositeExtract %float %515 0
%517 = OpVectorTimesScalar %v4float %513 %516
%518 = OpFAdd %v4float %508 %517
OpStore %_7_output %518
%519 = OpLoad %v2float %_8_coord
%520 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%521 = OpLoad %v2float %520
%522 = OpFAdd %v2float %519 %521
OpStore %_8_coord %522
%523 = OpLoad %v4float %_7_output
%524 = OpLoad %v4float %outputColor_Stage0
%525 = OpFMul %v4float %523 %524
OpStore %_7_output %525
%526 = OpLoad %v4float %_7_output
OpStore %output_Stage1 %526
%527 = OpLoad %v4float %output_Stage1
%528 = OpLoad %v4float %outputCoverage_Stage0
%529 = OpFMul %v4float %527 %528
OpStore %sk_FragColor %529
OpReturn
OpFunctionEnd
