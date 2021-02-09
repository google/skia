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
OpName %_1_coords "_1_coords"
OpName %_2_inCoord "_2_inCoord"
OpName %_3_subsetCoord "_3_subsetCoord"
OpName %_4_clampedCoord "_4_clampedCoord"
OpName %_5_textureColor "_5_textureColor"
OpName %_6_snappedX "_6_snappedX"
OpName %main "main"
OpName %output_Stage1 "output_Stage1"
OpName %_8_output "_8_output"
OpName %_9_coord "_9_coord"
OpName %_10_coordSampled "_10_coordSampled"
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
OpDecorate %67 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %432 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %486 RelaxedPrecision
OpDecorate %489 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %495 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%108 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_12 = OpConstant %float 12
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%118 = OpConstantComposite %v2float %float_0 %float_0
%120 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%MatrixEffect_Stage1_c0_c0 = OpFunction %v4float None %26
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%_1_coords = OpVariable %_ptr_Function_v2float Function
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
OpStore %_1_coords %43
%45 = OpLoad %v2float %_1_coords
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
%output_Stage1 = OpVariable %_ptr_Function_v4float Function
%_8_output = OpVariable %_ptr_Function_v4float Function
%_9_coord = OpVariable %_ptr_Function_v2float Function
%_10_coordSampled = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v4float Function
%123 = OpVariable %_ptr_Function_v2float Function
%136 = OpVariable %_ptr_Function_v4float Function
%138 = OpVariable %_ptr_Function_v2float Function
%151 = OpVariable %_ptr_Function_v4float Function
%153 = OpVariable %_ptr_Function_v2float Function
%166 = OpVariable %_ptr_Function_v4float Function
%168 = OpVariable %_ptr_Function_v2float Function
%181 = OpVariable %_ptr_Function_v4float Function
%183 = OpVariable %_ptr_Function_v2float Function
%196 = OpVariable %_ptr_Function_v4float Function
%198 = OpVariable %_ptr_Function_v2float Function
%211 = OpVariable %_ptr_Function_v4float Function
%213 = OpVariable %_ptr_Function_v2float Function
%226 = OpVariable %_ptr_Function_v4float Function
%228 = OpVariable %_ptr_Function_v2float Function
%241 = OpVariable %_ptr_Function_v4float Function
%243 = OpVariable %_ptr_Function_v2float Function
%256 = OpVariable %_ptr_Function_v4float Function
%258 = OpVariable %_ptr_Function_v2float Function
%271 = OpVariable %_ptr_Function_v4float Function
%273 = OpVariable %_ptr_Function_v2float Function
%286 = OpVariable %_ptr_Function_v4float Function
%288 = OpVariable %_ptr_Function_v2float Function
%301 = OpVariable %_ptr_Function_v4float Function
%303 = OpVariable %_ptr_Function_v2float Function
%316 = OpVariable %_ptr_Function_v4float Function
%318 = OpVariable %_ptr_Function_v2float Function
%331 = OpVariable %_ptr_Function_v4float Function
%333 = OpVariable %_ptr_Function_v2float Function
%346 = OpVariable %_ptr_Function_v4float Function
%348 = OpVariable %_ptr_Function_v2float Function
%361 = OpVariable %_ptr_Function_v4float Function
%363 = OpVariable %_ptr_Function_v2float Function
%376 = OpVariable %_ptr_Function_v4float Function
%378 = OpVariable %_ptr_Function_v2float Function
%391 = OpVariable %_ptr_Function_v4float Function
%393 = OpVariable %_ptr_Function_v2float Function
%406 = OpVariable %_ptr_Function_v4float Function
%408 = OpVariable %_ptr_Function_v2float Function
%421 = OpVariable %_ptr_Function_v4float Function
%423 = OpVariable %_ptr_Function_v2float Function
%436 = OpVariable %_ptr_Function_v4float Function
%438 = OpVariable %_ptr_Function_v2float Function
%451 = OpVariable %_ptr_Function_v4float Function
%453 = OpVariable %_ptr_Function_v2float Function
%466 = OpVariable %_ptr_Function_v4float Function
%468 = OpVariable %_ptr_Function_v2float Function
%481 = OpVariable %_ptr_Function_v4float Function
%483 = OpVariable %_ptr_Function_v2float Function
OpStore %_8_output %108
%110 = OpLoad %v2float %vLocalCoord_Stage0
%112 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%114 = OpLoad %v2float %112
%115 = OpVectorTimesScalar %v2float %114 %float_12
%116 = OpFSub %v2float %110 %115
OpStore %_9_coord %116
OpStore %_10_coordSampled %118
%119 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %119
OpStore %121 %120
%122 = OpLoad %v2float %_10_coordSampled
OpStore %123 %122
%124 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %121 %123
%126 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 0
%129 = OpVectorTimesScalar %v4float %124 %128
OpStore %_8_output %129
%130 = OpLoad %v2float %_9_coord
%131 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%132 = OpLoad %v2float %131
%133 = OpFAdd %v2float %130 %132
OpStore %_9_coord %133
%134 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %134
%135 = OpLoad %v4float %_8_output
OpStore %136 %120
%137 = OpLoad %v2float %_10_coordSampled
OpStore %138 %137
%139 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %136 %138
%140 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%141 = OpLoad %v4float %140
%142 = OpCompositeExtract %float %141 1
%143 = OpVectorTimesScalar %v4float %139 %142
%144 = OpFAdd %v4float %135 %143
OpStore %_8_output %144
%145 = OpLoad %v2float %_9_coord
%146 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%147 = OpLoad %v2float %146
%148 = OpFAdd %v2float %145 %147
OpStore %_9_coord %148
%149 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %149
%150 = OpLoad %v4float %_8_output
OpStore %151 %120
%152 = OpLoad %v2float %_10_coordSampled
OpStore %153 %152
%154 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %151 %153
%155 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%156 = OpLoad %v4float %155
%157 = OpCompositeExtract %float %156 2
%158 = OpVectorTimesScalar %v4float %154 %157
%159 = OpFAdd %v4float %150 %158
OpStore %_8_output %159
%160 = OpLoad %v2float %_9_coord
%161 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%162 = OpLoad %v2float %161
%163 = OpFAdd %v2float %160 %162
OpStore %_9_coord %163
%164 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %164
%165 = OpLoad %v4float %_8_output
OpStore %166 %120
%167 = OpLoad %v2float %_10_coordSampled
OpStore %168 %167
%169 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %166 %168
%170 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_0
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 3
%173 = OpVectorTimesScalar %v4float %169 %172
%174 = OpFAdd %v4float %165 %173
OpStore %_8_output %174
%175 = OpLoad %v2float %_9_coord
%176 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%177 = OpLoad %v2float %176
%178 = OpFAdd %v2float %175 %177
OpStore %_9_coord %178
%179 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %179
%180 = OpLoad %v4float %_8_output
OpStore %181 %120
%182 = OpLoad %v2float %_10_coordSampled
OpStore %183 %182
%184 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %181 %183
%185 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%186 = OpLoad %v4float %185
%187 = OpCompositeExtract %float %186 0
%188 = OpVectorTimesScalar %v4float %184 %187
%189 = OpFAdd %v4float %180 %188
OpStore %_8_output %189
%190 = OpLoad %v2float %_9_coord
%191 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%192 = OpLoad %v2float %191
%193 = OpFAdd %v2float %190 %192
OpStore %_9_coord %193
%194 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %194
%195 = OpLoad %v4float %_8_output
OpStore %196 %120
%197 = OpLoad %v2float %_10_coordSampled
OpStore %198 %197
%199 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %196 %198
%200 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%201 = OpLoad %v4float %200
%202 = OpCompositeExtract %float %201 1
%203 = OpVectorTimesScalar %v4float %199 %202
%204 = OpFAdd %v4float %195 %203
OpStore %_8_output %204
%205 = OpLoad %v2float %_9_coord
%206 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%207 = OpLoad %v2float %206
%208 = OpFAdd %v2float %205 %207
OpStore %_9_coord %208
%209 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %209
%210 = OpLoad %v4float %_8_output
OpStore %211 %120
%212 = OpLoad %v2float %_10_coordSampled
OpStore %213 %212
%214 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %211 %213
%215 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%216 = OpLoad %v4float %215
%217 = OpCompositeExtract %float %216 2
%218 = OpVectorTimesScalar %v4float %214 %217
%219 = OpFAdd %v4float %210 %218
OpStore %_8_output %219
%220 = OpLoad %v2float %_9_coord
%221 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%222 = OpLoad %v2float %221
%223 = OpFAdd %v2float %220 %222
OpStore %_9_coord %223
%224 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %224
%225 = OpLoad %v4float %_8_output
OpStore %226 %120
%227 = OpLoad %v2float %_10_coordSampled
OpStore %228 %227
%229 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %226 %228
%230 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_1
%231 = OpLoad %v4float %230
%232 = OpCompositeExtract %float %231 3
%233 = OpVectorTimesScalar %v4float %229 %232
%234 = OpFAdd %v4float %225 %233
OpStore %_8_output %234
%235 = OpLoad %v2float %_9_coord
%236 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%237 = OpLoad %v2float %236
%238 = OpFAdd %v2float %235 %237
OpStore %_9_coord %238
%239 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %239
%240 = OpLoad %v4float %_8_output
OpStore %241 %120
%242 = OpLoad %v2float %_10_coordSampled
OpStore %243 %242
%244 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %241 %243
%245 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%246 = OpLoad %v4float %245
%247 = OpCompositeExtract %float %246 0
%248 = OpVectorTimesScalar %v4float %244 %247
%249 = OpFAdd %v4float %240 %248
OpStore %_8_output %249
%250 = OpLoad %v2float %_9_coord
%251 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%252 = OpLoad %v2float %251
%253 = OpFAdd %v2float %250 %252
OpStore %_9_coord %253
%254 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %254
%255 = OpLoad %v4float %_8_output
OpStore %256 %120
%257 = OpLoad %v2float %_10_coordSampled
OpStore %258 %257
%259 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %256 %258
%260 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%261 = OpLoad %v4float %260
%262 = OpCompositeExtract %float %261 1
%263 = OpVectorTimesScalar %v4float %259 %262
%264 = OpFAdd %v4float %255 %263
OpStore %_8_output %264
%265 = OpLoad %v2float %_9_coord
%266 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%267 = OpLoad %v2float %266
%268 = OpFAdd %v2float %265 %267
OpStore %_9_coord %268
%269 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %269
%270 = OpLoad %v4float %_8_output
OpStore %271 %120
%272 = OpLoad %v2float %_10_coordSampled
OpStore %273 %272
%274 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %271 %273
%275 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%276 = OpLoad %v4float %275
%277 = OpCompositeExtract %float %276 2
%278 = OpVectorTimesScalar %v4float %274 %277
%279 = OpFAdd %v4float %270 %278
OpStore %_8_output %279
%280 = OpLoad %v2float %_9_coord
%281 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%282 = OpLoad %v2float %281
%283 = OpFAdd %v2float %280 %282
OpStore %_9_coord %283
%284 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %284
%285 = OpLoad %v4float %_8_output
OpStore %286 %120
%287 = OpLoad %v2float %_10_coordSampled
OpStore %288 %287
%289 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %286 %288
%290 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_2
%291 = OpLoad %v4float %290
%292 = OpCompositeExtract %float %291 3
%293 = OpVectorTimesScalar %v4float %289 %292
%294 = OpFAdd %v4float %285 %293
OpStore %_8_output %294
%295 = OpLoad %v2float %_9_coord
%296 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%297 = OpLoad %v2float %296
%298 = OpFAdd %v2float %295 %297
OpStore %_9_coord %298
%299 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %299
%300 = OpLoad %v4float %_8_output
OpStore %301 %120
%302 = OpLoad %v2float %_10_coordSampled
OpStore %303 %302
%304 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %301 %303
%305 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%306 = OpLoad %v4float %305
%307 = OpCompositeExtract %float %306 0
%308 = OpVectorTimesScalar %v4float %304 %307
%309 = OpFAdd %v4float %300 %308
OpStore %_8_output %309
%310 = OpLoad %v2float %_9_coord
%311 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%312 = OpLoad %v2float %311
%313 = OpFAdd %v2float %310 %312
OpStore %_9_coord %313
%314 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %314
%315 = OpLoad %v4float %_8_output
OpStore %316 %120
%317 = OpLoad %v2float %_10_coordSampled
OpStore %318 %317
%319 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %316 %318
%320 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%321 = OpLoad %v4float %320
%322 = OpCompositeExtract %float %321 1
%323 = OpVectorTimesScalar %v4float %319 %322
%324 = OpFAdd %v4float %315 %323
OpStore %_8_output %324
%325 = OpLoad %v2float %_9_coord
%326 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%327 = OpLoad %v2float %326
%328 = OpFAdd %v2float %325 %327
OpStore %_9_coord %328
%329 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %329
%330 = OpLoad %v4float %_8_output
OpStore %331 %120
%332 = OpLoad %v2float %_10_coordSampled
OpStore %333 %332
%334 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %331 %333
%335 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%336 = OpLoad %v4float %335
%337 = OpCompositeExtract %float %336 2
%338 = OpVectorTimesScalar %v4float %334 %337
%339 = OpFAdd %v4float %330 %338
OpStore %_8_output %339
%340 = OpLoad %v2float %_9_coord
%341 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%342 = OpLoad %v2float %341
%343 = OpFAdd %v2float %340 %342
OpStore %_9_coord %343
%344 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %344
%345 = OpLoad %v4float %_8_output
OpStore %346 %120
%347 = OpLoad %v2float %_10_coordSampled
OpStore %348 %347
%349 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %346 %348
%350 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_3
%351 = OpLoad %v4float %350
%352 = OpCompositeExtract %float %351 3
%353 = OpVectorTimesScalar %v4float %349 %352
%354 = OpFAdd %v4float %345 %353
OpStore %_8_output %354
%355 = OpLoad %v2float %_9_coord
%356 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%357 = OpLoad %v2float %356
%358 = OpFAdd %v2float %355 %357
OpStore %_9_coord %358
%359 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %359
%360 = OpLoad %v4float %_8_output
OpStore %361 %120
%362 = OpLoad %v2float %_10_coordSampled
OpStore %363 %362
%364 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %361 %363
%365 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%366 = OpLoad %v4float %365
%367 = OpCompositeExtract %float %366 0
%368 = OpVectorTimesScalar %v4float %364 %367
%369 = OpFAdd %v4float %360 %368
OpStore %_8_output %369
%370 = OpLoad %v2float %_9_coord
%371 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%372 = OpLoad %v2float %371
%373 = OpFAdd %v2float %370 %372
OpStore %_9_coord %373
%374 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %374
%375 = OpLoad %v4float %_8_output
OpStore %376 %120
%377 = OpLoad %v2float %_10_coordSampled
OpStore %378 %377
%379 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %376 %378
%380 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%381 = OpLoad %v4float %380
%382 = OpCompositeExtract %float %381 1
%383 = OpVectorTimesScalar %v4float %379 %382
%384 = OpFAdd %v4float %375 %383
OpStore %_8_output %384
%385 = OpLoad %v2float %_9_coord
%386 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%387 = OpLoad %v2float %386
%388 = OpFAdd %v2float %385 %387
OpStore %_9_coord %388
%389 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %389
%390 = OpLoad %v4float %_8_output
OpStore %391 %120
%392 = OpLoad %v2float %_10_coordSampled
OpStore %393 %392
%394 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %391 %393
%395 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%396 = OpLoad %v4float %395
%397 = OpCompositeExtract %float %396 2
%398 = OpVectorTimesScalar %v4float %394 %397
%399 = OpFAdd %v4float %390 %398
OpStore %_8_output %399
%400 = OpLoad %v2float %_9_coord
%401 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%402 = OpLoad %v2float %401
%403 = OpFAdd %v2float %400 %402
OpStore %_9_coord %403
%404 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %404
%405 = OpLoad %v4float %_8_output
OpStore %406 %120
%407 = OpLoad %v2float %_10_coordSampled
OpStore %408 %407
%409 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %406 %408
%410 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_4
%411 = OpLoad %v4float %410
%412 = OpCompositeExtract %float %411 3
%413 = OpVectorTimesScalar %v4float %409 %412
%414 = OpFAdd %v4float %405 %413
OpStore %_8_output %414
%415 = OpLoad %v2float %_9_coord
%416 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%417 = OpLoad %v2float %416
%418 = OpFAdd %v2float %415 %417
OpStore %_9_coord %418
%419 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %419
%420 = OpLoad %v4float %_8_output
OpStore %421 %120
%422 = OpLoad %v2float %_10_coordSampled
OpStore %423 %422
%424 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %421 %423
%425 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%426 = OpLoad %v4float %425
%427 = OpCompositeExtract %float %426 0
%428 = OpVectorTimesScalar %v4float %424 %427
%429 = OpFAdd %v4float %420 %428
OpStore %_8_output %429
%430 = OpLoad %v2float %_9_coord
%431 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%432 = OpLoad %v2float %431
%433 = OpFAdd %v2float %430 %432
OpStore %_9_coord %433
%434 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %434
%435 = OpLoad %v4float %_8_output
OpStore %436 %120
%437 = OpLoad %v2float %_10_coordSampled
OpStore %438 %437
%439 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %436 %438
%440 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%441 = OpLoad %v4float %440
%442 = OpCompositeExtract %float %441 1
%443 = OpVectorTimesScalar %v4float %439 %442
%444 = OpFAdd %v4float %435 %443
OpStore %_8_output %444
%445 = OpLoad %v2float %_9_coord
%446 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%447 = OpLoad %v2float %446
%448 = OpFAdd %v2float %445 %447
OpStore %_9_coord %448
%449 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %449
%450 = OpLoad %v4float %_8_output
OpStore %451 %120
%452 = OpLoad %v2float %_10_coordSampled
OpStore %453 %452
%454 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %451 %453
%455 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%456 = OpLoad %v4float %455
%457 = OpCompositeExtract %float %456 2
%458 = OpVectorTimesScalar %v4float %454 %457
%459 = OpFAdd %v4float %450 %458
OpStore %_8_output %459
%460 = OpLoad %v2float %_9_coord
%461 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%462 = OpLoad %v2float %461
%463 = OpFAdd %v2float %460 %462
OpStore %_9_coord %463
%464 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %464
%465 = OpLoad %v4float %_8_output
OpStore %466 %120
%467 = OpLoad %v2float %_10_coordSampled
OpStore %468 %467
%469 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %466 %468
%470 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_5
%471 = OpLoad %v4float %470
%472 = OpCompositeExtract %float %471 3
%473 = OpVectorTimesScalar %v4float %469 %472
%474 = OpFAdd %v4float %465 %473
OpStore %_8_output %474
%475 = OpLoad %v2float %_9_coord
%476 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%477 = OpLoad %v2float %476
%478 = OpFAdd %v2float %475 %477
OpStore %_9_coord %478
%479 = OpLoad %v2float %_9_coord
OpStore %_10_coordSampled %479
%480 = OpLoad %v4float %_8_output
OpStore %481 %120
%482 = OpLoad %v2float %_10_coordSampled
OpStore %483 %482
%484 = OpFunctionCall %v4float %MatrixEffect_Stage1_c0_c0 %481 %483
%485 = OpAccessChain %_ptr_Uniform_v4float %4 %int_2 %int_6
%486 = OpLoad %v4float %485
%487 = OpCompositeExtract %float %486 0
%488 = OpVectorTimesScalar %v4float %484 %487
%489 = OpFAdd %v4float %480 %488
OpStore %_8_output %489
%490 = OpLoad %v2float %_9_coord
%491 = OpAccessChain %_ptr_Uniform_v2float %4 %int_1
%492 = OpLoad %v2float %491
%493 = OpFAdd %v2float %490 %492
OpStore %_9_coord %493
%494 = OpLoad %v4float %_8_output
OpStore %output_Stage1 %494
%495 = OpLoad %v4float %output_Stage1
OpStore %sk_FragColor %495
OpReturn
OpFunctionEnd
